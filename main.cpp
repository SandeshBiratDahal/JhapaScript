#include<iostream>
#include<fstream>
#include<vector>
#include<map>
#include<string>

#include "tokens.cpp"
#include "utilities.cpp"
#include "variables_storage.cpp"

using namespace std;

//DATA TYPES
string NUM = "num", STR = "str";

// Used to translate from high level syntax to assembly-like code
class Interpreter{
    string raw_code;
    vector<string> code_lines;

    vector<string> ass_code;

    string alphas = "qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM_",
    nums = "1234567890", specials = "!@#$%^&*()+{}[]:;'<>,.?/|=-~`", space = " ";

    vector<string> keywords = 
    {
        "num", "str", "for", "loop", "if", "then", "endif", "else", "elif"
    };

    VariableStorage vars;

    public:
        // Reads the source code and divides it into different lines
        void read(string file_path){
            ifstream source_file(file_path);
            string line;
            while (getline(source_file, line)) {
                raw_code += line + "\n";

                if (line != "") code_lines.push_back(line);
            }
        }

        void show_raw_code() {
            cout << raw_code;
        }

        void show_code_lines() {
            for(int i = 0; i < code_lines.size(); i++) cout << code_lines[i] << endl;
        }

        void show_tokens(vector<Token> tokens) {
            for (int i = 0; i < tokens.size(); i++) cout << tokens[i].repr() << endl << endl;
        }

        // returns all the tokens as objects of class 'Token' in a given line of code
        vector<Token> tokenize_line(string line) {

            vector<Token> tokens;
            string current_token;
            int i = 0, j;

            while (i < line.length()) {
                current_token = "";
                char letter = line[i];


                // FOr identifiers and keywords
                if (alphas.find(letter) != string::npos) {
                    current_token += letter;
                    j = i + 1;

                    while ((alphas.find(line[j]) != string::npos || nums.find(line[j]) != string::npos) && j < line.length()) {
                        current_token += line[j];
                        j++;
                    }

                    if (find(keywords, current_token) != -1) tokens.emplace_back("keyword", current_token);
                    else tokens.emplace_back("identifier", current_token);

                    i = j - 1;
                }

                // for interges and floats
                else if (nums.find(letter) != string::npos) {
                    bool has_decimal = false;
                    current_token += letter;

                    j = i + 1;

                    while ((nums.find(line[j]) != string::npos || line[j] == '.') && j < line.length()) {
                        if (line[j] == '.') {
                            if (!has_decimal) has_decimal = true;
                            else cout << "Multiple decimal symbols found in the float literal!" << endl;
                        }
                        current_token += line[j];
                        j++;
                    }
                    
                    if (tokens.size() >= 2 && tokens[tokens.size() - 1].get_value() == "-" && tokens[tokens.size() - 2].get_type() != NUM) {
                        tokens.erase(
                            tokens.begin() + tokens.size() - 1
                        );
                        if (has_decimal) tokens.emplace_back(NUM, "-" + current_token);
                        else tokens.emplace_back(NUM, "-" + current_token + ".000000");
                    }
                    else {
                        if (has_decimal) tokens.emplace_back(NUM, current_token);
                        else tokens.emplace_back(NUM, current_token + ".000000");
                    }

                    i = j - 1;
                }
                // for strings
                else if (letter == '"') {
                    j = i + 1;
                    while (line[j] != '"' && j < line.length()) {
                        current_token += line[j];
                        j++;
                    }
                    tokens.emplace_back(STR, current_token);

                    i = j;
                }

                // NOW ALL FOR OPERATORS AND SYMBOLS

                else if (letter == '=') {
                    if (line[i + 1] == '=') {
                        tokens.emplace_back("operator:equal", "==");
                        i++;
                    }
                    else tokens.emplace_back("operator:assignment", "=");
                }

                else if (letter == '+') {
                    tokens.emplace_back("operator:plus", "+");
                }

                else if (letter == '-') {
                    tokens.emplace_back("operator:minus", "-");
                }

                else if (letter == '*') {
                    tokens.emplace_back("operator:multiply", "*");
                }

                else if (letter == '/') {
                    tokens.emplace_back("operator:divide", "/");
                }

                else if (letter == '>') {
                    if (line[i + 1] == '=') {
                        tokens.emplace_back("operator:greaterorequal", ">=");
                        i++;
                    }
                    else tokens.emplace_back("operator:greater", ">");
                }

                else if (letter == '<') {
                    if (line[i + 1] == '=') {
                        tokens.emplace_back("operator:lesserorequal", "<=");
                        i++;
                    }
                    else tokens.emplace_back("operator:lesser", "<");
                }

                else if (letter == '!') {
                    if (line[i + 1] == '=') {
                        tokens.emplace_back("operator:notequal", "!=");
                        i++;
                    }
                    else tokens.emplace_back("operator:not", "!");
                }

                else if (letter == ',') {
                    tokens.emplace_back("operator:comma", ",");
                }

                else if (letter == '(') {
                    tokens.emplace_back("leftparanthesis", "(");
                }

                else if (letter == ')') {
                    tokens.emplace_back("rightparanthesis", ")");
                }

                else if (letter == '.') {
                    if (line[i + 1] == '.') {
                        tokens.emplace_back("operator:concat", "..");
                        i++;
                    }
                    else tokens.emplace_back("operator:dot", ".");
                }

                i++;
            }
            return tokens;
        }

        Token evaluate(vector<Token> expression, int depth = 0) {
            //cout << "Depth: " << depth << endl;
            bool has_paranthesis = false;
            int i = 0;
            Token current_token;
            while (i < expression.size()){
                current_token = expression[i];
                
                if (current_token.get_value() == "(") {
                    has_paranthesis = true;
                    int no_of_open_paranthesis = 1, j = i + 1;
                    vector<Token> sub_tokens;
                    while (true) {
                        if (expression[j].get_value() == "(") no_of_open_paranthesis++;
                        else if (expression[j].get_value() == ")") no_of_open_paranthesis--;
                        if (expression[j].get_value() == ")" && no_of_open_paranthesis == 0) break;
                        sub_tokens.push_back(expression[j]);
                        j++;
                        if (j > expression.size()) break;
                    }

                    // I need to replace the sub tokens with the evaluated value which can be achieved by using recursion:: DONE

                    //show_tokens(sub_tokens);
                    expression.erase(expression.begin() + i, expression.begin() + j + 1);
                    expression.insert(
                        expression.begin() + i, evaluate(sub_tokens, depth + 1)
                    );
                    has_paranthesis = false;
                    //show_tokens(expression);
                }
                i++; 
            }

            if (!has_paranthesis) {
                //show_tokens(expression);
                i = 0;

                //Highest Priority for * and /
                while (i < expression.size()) {
                    current_token = expression[i];
                    float left_operand_float, right_operand_float;
                    if (current_token.get_value() == "*" || current_token.get_value() == "/") {
                        left_operand_float = stod(expression[i - 1].get_value());
                        right_operand_float = stod(expression[i + 1].get_value());

                        expression.erase(
                            expression.begin() + i - 1, expression.begin() + i + 2
                        );
                        Token ans;
                        if (current_token.get_value() == "*") {
                            ans.set_type(NUM);
                            ans.set_value(to_string(left_operand_float * right_operand_float));
                            //cout << "AJADGKJA" << endl;
                            //show_tokens(expression);
                           //ans(NUM, to_string(left_operand_float * right_operand_float));
                        }
                        else {
                            ans.set_type(NUM);
                            ans.set_value(to_string(left_operand_float / right_operand_float));
                            //ans(NUM, to_string(left_operand_float / right_operand_float));
                        }
                        expression.insert(
                            expression.begin() + i - 1, ans
                        );
                        i -= 1;
                    }
                    else if (current_token.get_value() == "+" && expression[i - 1].get_type() == "str" && expression[i + 1].get_type() == "str") {
                        string left_operand, right_operand;
                        left_operand = (expression[i - 1].get_value());
                        right_operand = (expression[i + 1].get_value());

                        expression.erase(
                            expression.begin() + i - 1, expression.begin() + i + 2
                        );

                        Token ans;

                        ans.set_type(STR);
                        ans.set_value(left_operand + right_operand);

                        expression.insert(
                            expression.begin() + i - 1, ans
                        );
                        i -= 1;
                    }
                    //cout << "___NEW___ pr" << endl;
                    //show_tokens(expression);
                    i++;
                }

                i = 0;
                // FOr + and -
                while (i < expression.size()) {
                    current_token = expression[i];
                    double left_operand_float, right_operand_float;
                    if (current_token.get_value() == "+" || current_token.get_value() == "-") {
                        //show_tokens(expression);
                        //cout << expression[i + 1].get_value()<< endl << expression[i - 1].get_value()<<endl;
                        left_operand_float = stod(expression[i - 1].get_value());
                        right_operand_float = stod(expression[i + 1].get_value());

                        expression.erase(
                            expression.begin() + i - 1, expression.begin() + i + 2
                        );
                        Token ans;
                        if (current_token.get_value() == "+") {
                            ans.set_type(NUM);
                            ans.set_value(to_string(left_operand_float + right_operand_float));
                           //ans(NUM, to_string(left_operand_float * right_operand_float));
                        }
                        else {
                            ans.set_type(NUM);
                            ans.set_value(to_string(left_operand_float - right_operand_float));
                            //ans(NUM, to_string(left_operand_float / right_operand_float));
                        }
                        //cout << ans.repr()<< endl << i << endl;
                        expression.insert(
                            expression.begin() + i - 1, ans
                        );
                        //show_tokens(expression);
                        i -= 1;
                    }
                    i++;
                    //cout << "___NEW___" << endl;
                    //show_tokens(expression);
                }

            }
            return expression[0];
        }

        void interpret() {
            ifstream in("program.nv");
            string line;
            while(getline(in, line)) ass_code.push_back(line);
            
            int line_no = 0, sub_line_no = 0;

            while (line_no < ass_code.size() && line_no >= 0) {
                line = ass_code[line_no];

                if (line == "ext") break;

                else if (line == "prt") {
                    sub_line_no = line_no + 1;
                    while (ass_code[sub_line_no] != ";") {
                        cout << evaluate(tokenize_line(ass_code[sub_line_no])).get_value();
                        sub_line_no++;
                    }
                    line_no = sub_line_no;
                }

                else if (line == "num") {
                    sub_line_no = line_no + 1;
                    while (ass_code[sub_line_no] != ";") {
                        vars.store(ass_code[sub_line_no], NUM, "0");
                        sub_line_no++;
                    }
                    line_no = sub_line_no;
                }

                else if (line == "str") {
                    sub_line_no = line_no + 1;
                    while (ass_code[sub_line_no] != ";") {
                        vars.store(ass_code[sub_line_no], STR, "");
                        sub_line_no++;
                    }
                    line_no = sub_line_no;
                }

                else if (line == "ld") {
                    sub_line_no = line_no + 1;
                    while (ass_code[sub_line_no] != ";") {
                        vars.edit(
                            ass_code[sub_line_no],
                            evaluate(tokenize_line(ass_code[sub_line_no + 1])).get_value()
                        );
                        sub_line_no += 2;
                    }
                    line_no = sub_line_no;
                    vars.show_all();
                }

                else if (line == "nl") cout << endl;

                else if (line == "goto") line_no = stod(ass_code[line_no + 1]);

                line_no++;
            }
        }
};

int main() {
    Interpreter nova;
    nova.read("program.nv");
    nova.interpret();
    //cout << translate.evaluate(n).repr() << endl;
    return 0;
}