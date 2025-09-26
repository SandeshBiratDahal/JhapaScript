//JHAPASCRIPT: 1.0
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <conio.h>
#include <ctime>
#include <string.h>
#include "tokens.cpp"
#include "utilities.cpp"
#include "variables_storage.cpp"

using namespace std;

//DATA TYPES
string NUM = "num", STR = "str";
string DECIMAL_SUFFIX = ".000000";

// Used to translate from high level syntax to assembly-like code
class Interpreter{
    string raw_code;
    vector<string> code_lines;

    vector<string> ass_code;

    string alphas = "qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM_",
    nums = "1234567890", specials = "!@#$%^&*()+{}[]:;'<>,.?/|=-~`", space = " ";

    vector<string> keywords = 
    {
        "num", "str", "for", "loop", "if", "end", "else", "print", "input", "while", "endif", "break", "continue", "clear", "num_array", "str_array"
    };

    VariableStorage vars;

    vector<int> if_tracker, while_break_tracker, while_continue_tracker, for_break_tracker, for_continue_tracker;
    vector<char> loops;
    vector<vector<int>> if_elif_tracker;
    vector<vector<string>> last_conditional;

    string last_loop;
    public:
        string remove_whitespace(string s) {
            string new_string = "";
            for (char c: s) {
                if (c != ' ' && c != '\t' && c != '\n') {
                    new_string += c;
                }
            }
            return new_string;
        }
        // Reads the source code and divides it into different lines
        void read(string file_path){
            ifstream source_file(file_path);
            string line;
            while (getline(source_file, line)) {
                raw_code += line + "\n";

                if (remove_whitespace(line) != "") code_lines.push_back(line);
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
        vector<Token> tokenize_line(string line, int index = -1) {
            if (index >= 0) line = code_lines[index];
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

                // for interges and doubles
                else if (nums.find(letter) != string::npos) {
                    bool has_decimal = false;
                    current_token += letter;

                    j = i + 1;

                    while ((nums.find(line[j]) != string::npos || line[j] == '.') && j < line.length()) {
                        if (line[j] == '.') {
                            if (!has_decimal) has_decimal = true;
                            else cout << "Multiple decimal symbols found in the double literal!" << endl;
                        }
                        current_token += line[j];
                        j++;
                    }
                    
                    if (tokens.size() >= 2 && tokens.back().get_value() == "-" && (tokens[tokens.size() - 2].get_type() == NUM || tokens[tokens.size() - 2].get_type() == "identifier")) {
                        if (has_decimal) tokens.emplace_back(NUM, current_token);
                        else tokens.emplace_back(NUM, current_token + DECIMAL_SUFFIX);
                    }
                    else if (tokens.size() >= 1 && tokens.back().get_value() == "-") {
                        tokens.erase(tokens.begin() + tokens.size() - 1);
                        if (has_decimal) tokens.emplace_back(NUM, "-" + current_token);
                        else tokens.emplace_back(NUM, "-" + current_token + DECIMAL_SUFFIX);
                    }
                    else {
                        if (has_decimal) tokens.emplace_back(NUM, current_token);
                        else tokens.emplace_back(NUM, current_token + DECIMAL_SUFFIX);
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
                    //cout << (line[i + 1]);
                    if (line[i + 1] == '/') {
                        return tokens;
                    }
                    else if (line[i + 1] == '*') {
                        tokens.emplace_back("operator:integerdivide", "/*");
                        i++;
                    }
                    else tokens.emplace_back("operator:divide", "/");
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
                    tokens.emplace_back("operator:dot", ".");
                }

                else if (letter == '%') {
                    tokens.emplace_back("operator:modulus", "%");
                }

                else if (letter == '|') {
                    if (line[i + 1] == '|') {
                        tokens.emplace_back("operator:or", "||");
                        i++;
                    }
                }

                else if (letter == '&') {
                    if (line[i + 1] == '&') {
                        tokens.emplace_back("operator:and", "&&");
                        i++;
                    }
                }

                else if (letter == '{') {
                    tokens.emplace_back("leftbraces", "{");
                }

                else if (letter == '}') {
                    tokens.emplace_back("rightbraces", "}");
                }

                else if (letter == '[') {
                    tokens.emplace_back("leftsquares", "[");
                }

                else if (letter == ']') {
                    tokens.emplace_back("rightsquares", "]");
                }

                i++;
            }
            //show_tokens(tokens);
            return tokens;
        }

        Token evaluate(vector<Token> expression, int depth = 0, bool input_var = false) {

            //CONTAINS_BOOLS
            bool ASSIGNMENT = false, COMMA = false, AND = false, OR = false, NOT = false, MODULUS = false, LEFTPARANTHESIS = false,
            DOT = false, LESSTHAN = false, GREATERTHAN = false, LESSTHANEQ = false, GREATERTHANEQ = false, EQUALSTO = false, PLUS = false,
            MINUS = false, MULTIPLY = false, DIVIDE = false, INTEGERDIVIDE = false, LEFTSQUARES = false, NOTEQUALSTO = false;
            //cout << "Depth: " << depth << endl;
            //vector<string> present_operators;
            string _type;
            for (int i = 0; i < expression.size(); i++) {
                _type = expression[i].get_type();
                if (_type == "operator:assignment") ASSIGNMENT = true;
                else if (_type == "operator:comma") COMMA = true;
                else if (_type == "operator:and") AND = true;
                else if (_type == "operator:or") OR = true;
                else if (_type == "operator:not") NOT = true;
                else if (_type == "operator:modulus") MODULUS = true;
                else if (_type == "leftparanthesis") LEFTPARANTHESIS = true;
                else if (_type == "operator:dot") DOT = true;
                else if (_type == "operator:lesser") LESSTHAN = true;
                else if (_type == "operator:greater") GREATERTHAN= true;
                else if (_type == "operator:lesserorequal") LESSTHANEQ = true;
                else if (_type == "operator:greaterorequal") GREATERTHANEQ = true;
                else if (_type == "operator:equal") EQUALSTO = true;
                else if (_type == "operator:plus") PLUS = true;
                else if (_type == "operator:minus") MINUS = true;
                else if (_type == "operator:multiply") MULTIPLY = true;
                else if (_type == "operator:divide") DIVIDE = true;
                else if (_type == "operator:integerdivide") INTEGERDIVIDE = true;
                else if (_type == "leftsquares") LEFTSQUARES = true;
                else if (_type == "operator:notequal") NOTEQUALSTO = true;
                // if (expression[i].get_type().substr(0, 8) == "operator" || expression[i].get_type() == "leftparanthesis" || expression[i].get_type() == "leftsquares") {
                //     present_operators.push_back(expression[i].get_value());
                //     //cout << expression[i].get_value() << endl;
                // }
            }

            bool has_paranthesis = false;
            int i = 0;
            Token current_token;
            if (LEFTPARANTHESIS || LEFTSQUARES) {
                while (i < expression.size()){
                    current_token = expression[i];
                    //cout << current_token.get_value() << endl;
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
                            if (j >= expression.size()) break;
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
                    if (current_token.get_value() == "[") {
                        has_paranthesis = true;
                        int no_of_open_paranthesis = 1, j = i + 1;
                        vector<Token> sub_tokens;
                        while (true) {
                            if (expression[j].get_value() == "[") no_of_open_paranthesis++;
                            else if (expression[j].get_value() == "]") no_of_open_paranthesis--;
                            if (expression[j].get_value() == "]" && no_of_open_paranthesis == 0) break;
                            sub_tokens.push_back(expression[j]);
                            j++;
                            if (j >= expression.size()) break;
                        }
                        expression.erase(expression.begin() + i, expression.begin() + j + 1);
                        //show_tokens(sub_tokens);
                        expression[i - 1].set_value(
                            expression[i - 1].get_value() +"___" + evaluate(sub_tokens, depth + 1).get_value()
                        );
                        has_paranthesis = false;
                        //cout << "__NEW__" << endl;
                        //show_tokens(expression);
                        i--;
                    }
                    i++; 
                }
            }

            if (input_var) 
            {
                //cout << expression[0].get_value() << endl;
                return expression[0];
            }
            

            if (!has_paranthesis) {
                //show_tokens(expression);
                i = 0;
                int j = 0;
                //WORKED FIRST TRY WTFF
                while (i < expression.size()) {
                    current_token = expression[i];
                    if (current_token.get_type() == "identifier") {
                        if (i + 1 >= expression.size()) {
                            Variable var = vars.get(current_token.get_value());
                            expression[i] = Token(var.get_data_type(), var.get_value());
                        }
                        else if (expression[i + 1].get_value() != "=") {
                            Variable var = vars.get(current_token.get_value());
                            expression[i] = Token(var.get_data_type(), var.get_value());
                        }
                    }
                    //show_tokens(expression);
                    i++;
                }

                i = 0;
                //For assignment operator
                // I AM GOING TO DIE FIXING THIS AAAAAAAAAAAAAAAAAAAAA
                if (ASSIGNMENT) {
                    while (i < expression.size()) {
                        current_token = expression[i];
                        if (current_token.get_value() == "=" && expression[i - 1].get_type() == "identifier") {
                            j = i + 1;
                            //show_tokens(expression);
                            vector<Token> sub_tokens;
                            while (j < expression.size() && expression[j].get_value() != ","){
                                sub_tokens.push_back(expression[j]);
                                j++;
                            }
                            //show_tokens(sub_tokens);
                            //cout << evaluate(sub_tokens).get_value();
                            vars.get(expression[i - 1].get_value()).set_value(
                                evaluate(sub_tokens).get_value()
                            );
                            //vars.show_all();
                            expression.erase(
                                expression.begin() + i - 1, expression.begin() + j - 1
                            );
                            //cout << "__NEW__" << endl;
                            //show_tokens(expression);
                            //vars.show_all();  
                            i = i - 1;                  
                        }
                        i++;
                        //cout << i << endl;
                    }
                }

                //for !
                i = expression.size() - 1;
                if (NOT) {
                    while (i >= 0) {
                        current_token = expression[i];
                        double right_operand_double;
                        if (current_token.get_type() == "operator:not") {
                            //cout << expression[i + 1].get_value();
                            right_operand_double = stod(expression[i + 1].get_value());
                            if (right_operand_double == 0) expression[i + 1].set_value("1.000000");
                            else expression[i + 1].set_value("0.000000");

                            expression.erase(expression.end() - i - 2);
                            //show_tokens(expression);
                        } 
                        i--;
                    }
                }
                i = 0;
                //Highest Priority for * and /
                if (MULTIPLY || DIVIDE || PLUS || MODULUS || INTEGERDIVIDE) {
                    //cout << "Yes";
                    //show_tokens(expression);
                    while (i < expression.size()) {
                        //cout << "Yes";
                        current_token = expression[i];
                        //cout <<current_token.get_value() << endl << expression[i - 1].get_type() << endl <<  expression[i + 1].get_type() << endl;
                        //cout << current_token.get_value();
                        if (current_token.get_type().substr(0, 8) == "operator" && (current_token.get_value() == "*" || current_token.get_value() == "/" || current_token.get_value() == "%" || current_token.get_value() == "/*")) {
                            double left_operand_double, right_operand_double;
                            left_operand_double = stod(expression[i - 1].get_value());
                            right_operand_double = stod(expression[i + 1].get_value());
                            //cout << (left_operand_double / right_operand_double);
                            expression.erase(
                                expression.begin() + i - 1, expression.begin() + i + 2
                            );
                            Token ans;
                            if (current_token.get_value() == "*") {
                                ans.set_type(NUM);
                                ans.set_value(to_string(left_operand_double * right_operand_double));
                                //cout << "AJADGKJA" << endl;
                                //show_tokens(expression);
                            //ans(NUM, to_string(left_operand_double * right_operand_double));
                            }
                            else if (current_token.get_value() == "/"){
                                ans.set_type(NUM);
                                ans.set_value(to_string(left_operand_double / right_operand_double));
                                //ans(NUM, to_string(left_operand_double / right_operand_double));
                            }
                            else if(current_token.get_value() == "%"){
                                ans.set_type(NUM);
                                ans.set_value(to_string((int)left_operand_double % (int)right_operand_double));
                                //cout << ans.get_value() << endl;
                            }
                            else if (current_token.get_value() == "/*") {
                                ans.set_type(NUM);
                                //cout << "Yes";
                                ans.set_value(to_string((int) (left_operand_double / right_operand_double)));
                            }
                            expression.insert(
                                expression.begin() + i - 1, ans
                            );
                            i -= 1;
                        }
                        else if (current_token.get_type().substr(0, 8) == "operator" && (current_token.get_value() == "+" && expression[i - 1].get_type() == STR && expression[i + 1].get_type() == STR)) {
                            //cout << expression[i - 1].get_type() <<endl << expression[i + 1].get_type() << endl;
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
                }

                i = 0;
                // FOr + and -
                if (PLUS || MINUS) {
                    while (i < expression.size()) {
                        current_token = expression[i];
                        double left_operand_double, right_operand_double;
                        if (current_token.get_value() == "+" || current_token.get_value() == "-") {
                            //cout << "Yes";
                            //show_tokens(expression);
                            //cout << expression[i + 1].get_value()<< endl << expression[i - 1].get_value()<<endl;
                            left_operand_double = stod(expression[i - 1].get_value());
                            right_operand_double = stod(expression[i + 1].get_value());

                            expression.erase(
                                expression.begin() + i - 1, expression.begin() + i + 2
                            );
                            Token ans;
                            if (current_token.get_value() == "+") {
                                ans.set_type(NUM);
                                ans.set_value(to_string(left_operand_double + right_operand_double));
                            //ans(NUM, to_string(left_operand_double * right_operand_double));
                            }
                            else if (current_token.get_value() == "-") {
                                ans.set_type(NUM);
                                ans.set_value(to_string(left_operand_double - right_operand_double));
                                //ans(NUM, to_string(left_operand_double / right_operand_double));
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

                i = 0;
                // for <,>, <=, >=
                if (LESSTHAN || LESSTHANEQ || GREATERTHAN || GREATERTHANEQ) {
                    while (i < expression.size()) {
                        current_token = expression[i];
                        double left_operand_double, right_operand_double;
                        if (current_token.get_value() == "<" || current_token.get_value() == "<=" || current_token.get_value() == ">" || current_token.get_value() == ">=") {
                            //show_tokens(expression);
                            //cout << expression[i + 1].get_value()<< endl << expression[i - 1].get_value()<<endl;
                            left_operand_double = stod(expression[i - 1].get_value());
                            right_operand_double = stod(expression[i + 1].get_value());

                            expression.erase(
                                expression.begin() + i - 1, expression.begin() + i + 2
                            );
                            Token ans;
                            if (current_token.get_value() == "<") {
                                ans.set_type(NUM);
                                ans.set_value(to_string((int)(left_operand_double < right_operand_double)));
                            //ans(NUM, to_string(left_operand_double * right_operand_double));
                            }
                            else if (current_token.get_value() == "<=") {
                                ans.set_type(NUM);
                                ans.set_value(to_string((int)(left_operand_double <= right_operand_double)));
                                //ans(NUM, to_string(left_operand_double / right_operand_double));
                            }
                            else if (current_token.get_value() == ">") {
                                ans.set_type(NUM);
                                ans.set_value(to_string((int)(left_operand_double > right_operand_double)));
                                //ans(NUM, to_string(left_operand_double / right_operand_double));
                            }
                            else if (current_token.get_value() == ">=") {
                                ans.set_type(NUM);
                                ans.set_value(to_string((int)(left_operand_double >= right_operand_double)));
                                //ans(NUM, to_string(left_operand_double / right_operand_double));
                            }
                            //cout << ans.repr()<< endl << i << endl;
                            expression.insert(
                                expression.begin() + i - 1, ans
                            );
                            //show_tokens(expression);
                            i -= 1;
                        }
                        i++;
                    }
                }

                i = 0;
                // for != and ==
                if (EQUALSTO || NOTEQUALSTO) {
                    while (i < expression.size()) {
                        current_token = expression[i];
                        double left_operand_double, right_operand_double;
                        string right_operand_string, left_operand_string;
                        if (current_token.get_value() == "==" || current_token.get_value() == "!=") {
                            //cout << "New" << endl;
                            //show_tokens(expression);
                            //cout << expression[i + 1].get_value()<< endl << expression[i - 1].get_value()<<endl;
                            if (expression[i - 1].get_type() == NUM && expression[i + 1].get_type() == NUM) {
                                left_operand_double = stod(expression[i - 1].get_value());
                                right_operand_double = stod(expression[i + 1].get_value());
                            }
                            else if (expression[i - 1].get_type() == STR && expression[i + 1].get_type() == STR){
                                left_operand_string = expression[i - 1].get_value();
                                right_operand_string = expression[i + 1].get_value();
                            }

                            Token ans;
                            if (current_token.get_value() == "==") {
                                ans.set_type(NUM);
                                if (expression[i - 1].get_type() == NUM && expression[i + 1].get_type() == NUM)
                                    ans.set_value(to_string((int)(left_operand_double == right_operand_double)));
                                else if ((expression[i - 1].get_type() == STR && expression[i + 1].get_type() == STR))
                                    ans.set_value(to_string((int)(left_operand_string == right_operand_string)));
                            //ans(NUM, to_string(left_operand_double * right_operand_double));
                            }
                            else if (current_token.get_value() == "!=") {
                                ans.set_type(NUM);
                                if (expression[i - 1].get_type() == NUM && expression[i + 1].get_type() == NUM)
                                    ans.set_value(to_string((int)(left_operand_double != right_operand_double)));
                                else if ((expression[i - 1].get_type() == STR && expression[i + 1].get_type() == STR))
                                    ans.set_value(to_string((int)(left_operand_string != right_operand_string)));
                                //ans(NUM, to_string(left_operand_double / right_operand_double));
                            }
                            expression.erase(
                                expression.begin() + i - 1, expression.begin() + i + 2
                            );
                            //cout << ans.repr()<< endl << i << endl;
                            expression.insert(
                                expression.begin() + i - 1, ans
                            );
                            //show_tokens(expression);
                            i -= 1;
                            //show_tokens(expression);
                        }
                        i++;
                    }
                }
                i = 0;
                if (AND || OR) {
                    //show_tokens(expression);
                    while (i < expression.size()) {
                        current_token = expression[i];
                        //cout << current_token.get_value() << " ";
                        if (current_token.get_type().substr(0, 8) == "operator" && (current_token.get_value() == "&&" || current_token.get_value() == "||")) {
                            double left_operand, right_operand;
                            left_operand = stod(expression[i - 1].get_value());
                            right_operand = stod(expression[i + 1].get_value());
                            //cout << left_operand << endl << right_operand;
                            //show_tokens(expression);
                            Token ans;
                            if (current_token.get_value() == "&&") {
                                //cout << left_operand << " " << right_operand;
                                ans.set_type(NUM);
                                ans.set_value(to_string((int)left_operand && (int)right_operand));
                            }
                            else if (current_token.get_value() == "||") {
                                ans.set_type(NUM);
                                ans.set_value(to_string((int)left_operand || (int)right_operand));
                            }
                            expression.erase(
                                expression.begin() + i - 1, expression.begin() + i + 2
                            );
                            expression.insert(
                                expression.begin() + i - 1, ans
                            );
                            i -= 1;
                            //show_tokens(expression);
                        }
                        i++;
                    }
                }

            }
            if (expression.size() == 0) expression.emplace_back("num", "0.000000");
            return expression[0];
        }

        void interpret(vector<string> ass_code) {
            //cout << "Hello" << endl;
            string print_buffer;
            int max_buffer_size = 100;
            vars.store("endl", STR, "\n");
            //vars.show_all();
            //ifstream in("program.nv");
            string line;
            //while(getline(in, line)) ass_code.push_back(line);
            
            int line_no = 0, sub_line_no = 0;
            int no_of_steps = 0;

            while (line_no < ass_code.size() && line_no >= 0) {
                //vars.show_all();
                //cout << line_no << endl;
                no_of_steps++;
                line = ass_code[line_no];

                if (line == "ext") break;

                else if (line == "prt") {
                    sub_line_no = line_no + 1;
                    while (ass_code[sub_line_no] != ";") {
                        //cout << evaluate(tokenize_line(ass_code[sub_line_no])).get_value();
                        //cout << "Hello";
                        print_buffer += evaluate(tokenize_line(ass_code[sub_line_no])).get_value();
                        
                        sub_line_no++;
                    }
                    line_no = sub_line_no;
                }

                else if (line == "num") {
                    sub_line_no = line_no + 1;
                    while (ass_code[sub_line_no] != ";") {
                        vars.store(ass_code[sub_line_no], NUM, "0.000000");
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

                /*else if (line == "ld") {
                    sub_line_no = line_no + 1;
                    while (ass_code[sub_line_no] != ";") {
                        vars.edit(
                            ass_code[sub_line_no],
                            evaluate(tokenize_line(ass_code[sub_line_no + 1])).get_value()
                        );
                        sub_line_no += 2;
                    }
                    line_no = sub_line_no;
                }*/

                else if (line == "ipt") {
                    printf("%s", print_buffer.c_str());
                    print_buffer = "";
                    sub_line_no = line_no + 1;
                    //vars.show_all();
                    while (ass_code[sub_line_no] != ";") {
                        //show_tokens(tokenize_line(ass_code[sub_line_no]));
                        //cout << evaluate(tokenize_line(ass_code[sub_line_no])).get_value() << endl;
                        string identifier = evaluate(tokenize_line(ass_code[sub_line_no]), 0, true).get_value();
                        Variable &var = vars.get(identifier);
                        //cout << identifier << endl;
                        if (var.get_data_type() == NUM) {
                            double num;
                            cin >> num;
                            //vars.show_all();
                            vars.edit(identifier, to_string(num));
                            //var.set_value(to_string(num));
                            //vars.show_all();
                        }

                        else if (var.get_data_type() == STR) {
                            string str;
                            cin >> str;
                            vars.edit(identifier, str);
                        }
                        sub_line_no++;
                    }
                    line_no = sub_line_no;
                    //vars.show_all();
                }

                else if (line == "expr") {
                    sub_line_no = line_no + 1;
                    while (sub_line_no < ass_code.size() && ass_code[sub_line_no] != ";") {
                        vector<Token> t = (tokenize_line(ass_code[sub_line_no]));
                        //cout << "--NEW--" << endl;
                        //show_tokens(t);
                        evaluate(t);
                        sub_line_no++;
                    }
                    line_no = sub_line_no;
                }

                else if (line == "if") {
                    //show_tokens(tokenize_line(ass_code[line_no + 1]));
                    if (evaluate(tokenize_line(ass_code[line_no + 1])).get_value() == "0") {
                        line_no = stod(ass_code[line_no + 2]) - 1;
                    } else {
                        line_no += 3;
                    }
                }
                //else if (line == "nl") cout << endl;
                else if (line == "goto") line_no = stod(ass_code[line_no + 1]) - 1;

                else if(line == "cls") {
                    system("cls");
                    line_no++;
                }

                if (print_buffer.size() >= max_buffer_size) {
                    printf("%s", print_buffer.c_str());
                    print_buffer = "";
                }

                line_no++;
            }
            //cout << print_buffer.size();
            printf("%s", print_buffer.c_str());
            cout << endl << endl << "Number of steps: " << no_of_steps;
        }

        vector<string> translate_to_ass_code() {
            vector<string> ass_code;
            int line_no = 0, sub_line_no = 0;
            vector<int> trackers;
            int endable_keywords_tracker = 0;
            
            while (line_no < code_lines.size()) {
                //cout << line_no << endl;
                vector<Token> tokens = tokenize_line("", line_no);
                if (tokens.size()) {
                    string line = code_lines[line_no], type = tokens[0].get_type(), value = tokens[0].get_value();
                    string current_expression = "";

                    if ((type == "keyword" && value == "print") || type != "keyword") {

                        if (type == "keyword") ass_code.push_back("prt");
                        else ass_code.push_back("expr");
                        int i = 0;

                        for (i = type == "keyword" ? 1 : 0; i < tokens.size(); i++) {
                            if (tokens[i].get_value() == "," && tokens[i].get_type() == "operator:comma"){
                                ass_code.push_back(current_expression);
                                current_expression = "";
                                continue;
                            }
                            if (tokens[i].get_type() == STR) {
                                current_expression += "\""+ tokens[i].get_value() + "\"" + " ";
                            }
                            else {
                                current_expression += tokens[i].get_value() + " ";
                            }
                        }

                        if (current_expression != "") {
                            ass_code.push_back(current_expression);
                        }
                        ass_code.push_back(";");
                    }

                    else if (type == "keyword" && value == "input") {
                        //show_tokens(tokens);
                        ass_code.push_back("ipt");

                        for (int i = 1; i < tokens.size(); i++) {
                            if (tokens[i].get_type() == "operator:comma" && tokens[i].get_value() == ",") {
                                ass_code.push_back(current_expression);
                                current_expression = "";
                                continue;
                            }
                            else {
                                current_expression += tokens[i].get_value() + " ";
                            } 
                        }
                        ass_code.push_back(current_expression);
                        ass_code.push_back(";");
                    }

                    else if (type == "keyword" && (value == STR || value == NUM)) {
                        tokens.emplace_back("operator:comma", ",");

                        ass_code.push_back(value);
                        //show_tokens(tokens);
                        vector<string> declared;
                        vector<string> initialized;
                        vector<string> expressions;

                        for (int i = 1; i < tokens.size(); i++) {
                            if (tokens[i].get_type() == "identifier" && tokens[i + 1].get_value() != "=") {
                                declared.push_back(tokens[i].get_value());
                            }
                            else if(tokens[i].get_type() == "identifier" && tokens[i + 1].get_value() == "=") {
                                declared.push_back(tokens[i].get_value());
                                initialized.push_back(tokens[i].get_value());
                                sub_line_no = i + 2;
                                current_expression = "";
                                while (tokens[sub_line_no].get_type() != "operator:comma") {
                                    if (tokens[sub_line_no].get_type() != STR) {
                                        current_expression += tokens[sub_line_no].get_value() + " ";
                                    }
                                    else if (tokens[sub_line_no].get_type() == STR){
                                        current_expression += "\"" + tokens[sub_line_no].get_value() + "\" ";
                                    }
                                    sub_line_no++;
                                }
                                expressions.push_back(current_expression);
                                current_expression = "";
                                i = sub_line_no;
                            }
                        }
                        for (int i = 0; i < declared.size(); i++) {
                            ass_code.push_back(declared[i]);
                        }
                        ass_code.push_back(";");
                        
                        if (initialized.size() != 0){
                            ass_code.push_back("expr");
                            for (int i = 0; i < initialized.size(); i++) {
                                //cout << expressions[i];
                                if (value == NUM)
                                    ass_code.push_back(
                                        initialized[i] + " = " + expressions[i]
                                    );
                                else
                                {
                                    if (!expressions[i].empty()) {
                                        expressions[i].pop_back();
                                    }
                                    //cout << expressions[i];
                                    ass_code.push_back(
                                        initialized[i] + " = " + expressions[i]
                                    );
                                }   
                            }
                            ass_code.push_back(";");
                        }
                    }

                    else if(type == "keyword" && value == "while") {
                        loops.push_back('w');
                        last_loop = "while";
                        endable_keywords_tracker++;

                        ass_code.push_back("if");

                        current_expression = "";
                        for (int i = 1; i < tokens.size(); i++) {
                            if (tokens[i].get_type() == STR) current_expression += "\"" + tokens[i].get_value() + "\"" + " ";
                            else current_expression += tokens[i].get_value() + " ";
                        }
                        ass_code.push_back(current_expression);
                        ass_code.push_back("PLC");
                        ass_code.push_back(";");

                        trackers.push_back(ass_code.size() - 2);
                    }

                    else if (type == "keyword" && value == "for") {
                        loops.push_back('f');
                        last_loop = "for";
                        endable_keywords_tracker++;
                        vector<string> expressions;
                        current_expression = "";
                        string loop_var = tokens[1].get_value();

                        for (int i = 1; i < tokens.size(); i++) {
                            if (tokens[i].get_type() == "operator:comma") {
                                expressions.push_back(current_expression);
                                current_expression = "";
                                continue;
                            }
                            if (tokens[i].get_type() == STR) current_expression += "\"" + tokens[i].get_value() + "\"" + " ";
                            else current_expression += tokens[i].get_value() + " ";
                        }
                        expressions.push_back(current_expression);

                        if (expressions.size() == 2) expressions.push_back("1");
                        ass_code.push_back("expr");
                        ass_code.push_back(expressions[0]);
                        ass_code.push_back(";");

                        ass_code.push_back("if");
                        ass_code.push_back(expressions[1]);
                        ass_code.push_back(loop_var);
                        ass_code.push_back(expressions[2]);
                        trackers.push_back(ass_code.size() - 2);
                    }

                    else if (type == "keyword" && value == "loop") {
                        if (for_continue_tracker.size() != 0) {
                            ass_code[for_continue_tracker.back()] = to_string(ass_code.size());
                            for_continue_tracker.pop_back();
                        }
                        string loop_var = ass_code[trackers.back()];
                        string increment = ass_code[trackers.back() + 1];
                        ass_code.push_back("expr");
                        ass_code.push_back(loop_var + " = " + loop_var + " + " + increment);
                        ass_code.push_back(";");
                        endable_keywords_tracker--;
                        ass_code.push_back("goto");
                        ass_code.push_back(to_string(trackers.back() - 2));
                        if (for_break_tracker.size() != 0) {
                            ass_code[for_break_tracker.back()] = to_string(ass_code.size());
                            for_break_tracker.pop_back();
                        }
                        ass_code.push_back(";");
                        ass_code[trackers.back()] = to_string(ass_code.size());
                        ass_code[trackers.back() + 1] = ";";
                        trackers.pop_back();
                    }

                    else if (type == "keyword" && value == "end") {
                        if (while_continue_tracker.size() != 0) {
                            ass_code[while_continue_tracker.back()] = to_string(ass_code.size());
                            while_continue_tracker.pop_back();
                        }
                        endable_keywords_tracker--;
                        ass_code.push_back("goto");
                        ass_code.push_back(to_string(trackers.back() - 2));
                        //cout << while_break_tracker.size() << "x" << endl;
                        if (while_break_tracker.size() != 0) {
                            //cout << ass_code[while_break_tracker[while_break_tracker.size() - 1]] << "  " << ass_code.size() << endl;
                            ass_code[while_break_tracker.back()] = to_string(ass_code.size());
                            while_break_tracker.pop_back();
                        }
                        ass_code.push_back(";");
                        ass_code[trackers.back()] = to_string(ass_code.size());
                        trackers.pop_back();
                    }

                    else if(type == "keyword" && value == "if") {
                        last_conditional.push_back({"if"});
                        if_elif_tracker.push_back({});
                        if_tracker.push_back(ass_code.size() + 2);
                        ass_code.push_back("if");
                        current_expression = "";
                        for (int i = 1; i < tokens.size(); i++) {
                            if (tokens[i].get_type() == STR) current_expression += "\"" + tokens[i].get_value() + "\"" + " ";
                            else current_expression += tokens[i].get_value() + " ";
                        }
                        ass_code.push_back(current_expression);
                        ass_code.push_back("PLC");
                        ass_code.push_back(";");
                    }

                    else if (tokens.size() > 1 && type == "keyword" && value == "else" && tokens[1].get_value() == "if") {
                        last_conditional.back().push_back("elif");
                        ass_code.push_back("goto");
                        ass_code.push_back("PLC");
                        ass_code.push_back(";");
                        ass_code[if_tracker.back()] = to_string(ass_code.size());
                        if_elif_tracker.back().push_back(ass_code.size() - 2);
                        if_tracker.pop_back();
                        if_tracker.push_back(ass_code.size() + 2);
                        ass_code.push_back("if");
                        current_expression = "";
                        for (int i = 2; i < tokens.size(); i++) {
                            if (tokens[i].get_type() == STR) current_expression += "\"" + tokens[i].get_value() + "\"" + " ";
                            else current_expression += tokens[i].get_value() + " ";
                        }
                        ass_code.push_back(current_expression);
                        ass_code.push_back("PLC");
                        ass_code.push_back(";");
                    }
                    else if (type == "keyword" && value == "else") {
                        last_conditional.back().push_back("else");
                        ass_code.push_back("goto");
                        ass_code.push_back("PLC");
                        ass_code[if_tracker.back()] = to_string(ass_code.size());
                        if_elif_tracker.back().push_back(ass_code.size() - 1);
                        //cout << if_elif_tracker.back().back()<< endl;
                        if_tracker.pop_back();
                        ass_code.push_back(";");
                    }

                    else if (type == "keyword" && value == "endif") {
                        vector<string> last_if = last_conditional.back();
                        //cout << if_elif_tracker.back().size() << endl;
                        if (last_if.back() != "if") {
                            while (if_elif_tracker.back().size() != 0) {
                                //cout << ass_code[if_elif_tracker.back().back()] << endl;
                                ass_code[if_elif_tracker.back().back()] = to_string(ass_code.size());
                                if_elif_tracker.back().pop_back();
                            }
                            if_elif_tracker.pop_back();
                        }
                        else {
                            ass_code[if_tracker.back()] = to_string(ass_code.size());
                            if_tracker.pop_back();
                            if_elif_tracker.pop_back();
                        }
                        last_conditional.pop_back();
                    }

                    else if (type == "keyword" && value == "break") {
                        ass_code.push_back("goto");
                        if (loops.back() == 'f') for_break_tracker.push_back(ass_code.size());
                        else while_break_tracker.push_back(ass_code.size());
                        ass_code.push_back("PLC");
                        ass_code.push_back(";");
                    }

                    else if (type == "keyword" && value == "continue") {
                        ass_code.push_back("goto");
                        if (loops.back() == 'f') for_continue_tracker.push_back(ass_code.size());
                        else while_continue_tracker.push_back(ass_code.size());
                        ass_code.push_back("PLC");
                        ass_code.push_back(";");
                    }  

                    else if (type == "keyword" && value == "clear") {
                        ass_code.push_back("cls");
                        ass_code.push_back(";");
                    }

                    else if (type == "keyword" && (value == "num_array" || value == "str_array")) {
                        tokens.emplace_back("operator:comma", ",");
                        vector<string> declaration_ass_code;
                        vector<string> initialization_ass_code;

                        int depth = 0;
                        vector<int> depth_count;
                        string current_array = "";
                        current_expression = "";
                        int i = 1;
                        //show_tokens(tokens);
                        while (i < tokens.size()) {
                            //cout << i << endl;
                            if (tokens[i].get_type() == "identifier" && depth == 0 && tokens[i + 1].get_type() == "operator:assignment") {
                                current_array = tokens[i].get_value();
                                i++;
                                //continue;
                            }
                            if (tokens[i].get_type() == "identifier" && depth == 0 && tokens[i + 1].get_type() == "leftsquares")
                            {
                                int j = i + 1;
                                current_array = tokens[i].get_value();
                                //cout << current_array << endl;
                                vector<int> sizes;
                                
                                while (tokens[j].get_type() != "operator:comma" && j < tokens.size()) {
                                    //cout << tokens[j].get_value() << endl;
                                    if (tokens[j].get_type() == NUM) {
                                        sizes.push_back(stod(tokens[j].get_value()) - 1);
                                    }
                                    j++;
                                }
                                vector<string> combinations = generateCombinations(sizes);

                                for (int k = 0; k < combinations.size(); k++) {
                                    declaration_ass_code.push_back(current_array + combinations[k]);
                                    //initialization_ass_code.push_back(current_array + combinations[k] + " = 0.000000");
                                }
                                
                                i = j + 1;
                                //cout << i << endl;
                                //cout << tokens[i].get_value() << endl;
                                continue;

                            }
                            if (tokens[i].get_type() == "leftbraces") {
                                depth++;
                                depth_count.push_back(0);
                                //cout << depth_count[0];
                            }
                            else if (tokens[i].get_type() == "rightbraces") {
                                if (current_expression != "") {
                                    string cur_var = "";
                                    //cout << depth_count[0] << endl;
                                    for (int j = 0; j < depth_count.size(); j++) {
                                        cur_var +="___" + to_string(depth_count[j]);
                                        //cout << cur_var << endl;
                                    }
                                    declaration_ass_code.push_back(
                                        current_array + cur_var
                                    );
                                    //cout << current_expression << endl;
                                    current_expression.pop_back();
                                    if (value == "num_array")
                                        initialization_ass_code.push_back(current_array + cur_var + " = " + current_expression);
                                    else
                                        initialization_ass_code.push_back(current_array + cur_var + " = \"" + current_expression + "\"");
                                    current_expression = "";
                                }
                                depth--;
                                //cout << depth << endl;
                                depth_count.pop_back();
                                if (!depth_count.empty()) {
                                    depth_count.back()++;
                                }
                            }
                            else if (tokens[i].get_type() == "operator:comma" && depth != 0) {
                                //cout << depth << endl;
                                if (current_expression != "") {
                                    string cur_var = "";
                                    //cout << depth_count[0] << endl;
                                    for (int j = 0; j < depth_count.size(); j++) {
                                        cur_var +="___" + to_string(depth_count[j]);
                                        //cout << cur_var << endl;
                                    }
                                    declaration_ass_code.push_back(
                                        current_array + cur_var
                                    );
                                    current_expression.pop_back();
                                    if (value == "num_array")
                                        initialization_ass_code.push_back(current_array + cur_var + " = " + current_expression);
                                    else
                                        initialization_ass_code.push_back(current_array + cur_var + " = \"" + current_expression + "\"");
                                    current_expression = "";

                                    //cout << cur_var << endl;
                                    if (!depth_count.empty()) {
                                        depth_count.back()++;
                                    }
                                }
                            } 
                            else if (tokens[i].get_type() == "operator:comma") {
                                //cout << depth << endl;
                                if (i + 1 < tokens.size()) current_array = tokens[i + 1].get_value();
                                //cout << current_array << endl;
                            }
                            else {
                                current_expression += tokens[i].get_value() + " ";
                            }

                            if (tokens[i].get_type() == "operator:assignment" && depth == 0) {
                                current_expression.pop_back();
                                current_expression.pop_back();
                            } 
                            //cout << i << endl;                            
                            i++;
                            
                            //cout << current_expression << endl;
                        }
                        if (declaration_ass_code.size()) {
                            if (value == "num_array")
                                ass_code.push_back(NUM);
                            else
                                ass_code.push_back(STR);
                            for (int i = 0; i < declaration_ass_code.size(); i++) ass_code.push_back(declaration_ass_code[i]);
                            ass_code.push_back(";");
                        }

                        if (initialization_ass_code.size()) {
                            ass_code.push_back("expr");
                            for (int i = 0; i < initialization_ass_code.size(); i++) ass_code.push_back(initialization_ass_code[i]);
                            ass_code.push_back(";");
                        }

                    }
                    //cout << if_elif_tracker.size() << "x" << line_no<< endl;
                }
                
                line_no++;
                //for (int k = 0; k < ass_code.size(); k++) cout << ass_code[k] << endl;

            }
            ass_code.push_back("ext");
            ass_code.push_back(";");

            ofstream out("program.ac");
            for (int i = 0; i < ass_code.size(); i++) {
                if (i == ass_code.size() - 1) {
                    out << ass_code[i];
                    break;
                }
                out << ass_code[i] << endl;
            }
            
            return ass_code;
        }

        VariableStorage get_vars() {
            return vars;
        }
};

int main(int argc, char* argv[]) { 
    string file_path = "program.jhs";
    if (argc > 1) file_path = argv[1];
    Interpreter jhapascript;
    jhapascript.read(file_path);
    //jhapascript.show_raw_code();
    vector<string> ass_code = jhapascript.translate_to_ass_code();
    //for (int i = 0; i < ass_code.size(); i++) cout << ass_code[i] << endl;
    clock_t start = clock();   // Start timing
    jhapascript.interpret(ass_code);
    clock_t end = clock();     // End timing
    double time_taken = double(end - start) / CLOCKS_PER_SEC; // Convert to seconds
    cout << endl << "Time taken: " << time_taken << " seconds" << endl;
    //jhapascript.get_vars().show_all();
    return 0;
}

/*
Optimizations that could be made:
1. Instead of looping over and over for all the present operators, we make separate variables for each operator and check by looping only once |||||||||||||||||||||||--DONE
2. Instead of writing the raw expression to the assembly-like code, we just save the token objects and there is no need for tokenization again
3. Using printf instead of cout as printf prints using output buffer unlike cout ||||||||||||||||||||||||||||||||||||||--DONE
*/

/*
TODO:
1. Add &&, || and /* operators |||||||||||||||||||||--Done
2. Add proper error handling
3. Add arrays for str and num  |||||||||||||||||||||--DONE
4. Add functions
5. Make it able to evaluate /n's entered by user in strings
6. Add comments |||||||||||||||||||||||||||||--Done
7. Add a way to index individual elements of a string
*/