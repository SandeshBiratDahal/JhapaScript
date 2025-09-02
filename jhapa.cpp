//JHAPASCRIPT

#include<iostream>
#include<fstream>
#include<vector>
#include<map>
#include<string>
#include<conio.h>
#include <ctime>
#include<string.h>

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
        "num", "str", "for", "loop", "if", "end", "else", "print", "input", "while", "endif"
    };

    VariableStorage vars;

    vector<int> if_tracker;
    vector<vector<int>> if_elif_tracker;
    string last_conditional;

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

                else if (letter == '%') {
                    tokens.emplace_back("operator:modulus", "%");
                }

                i++;
            }
            //show_tokens(tokens);
            return tokens;
        }

        Token evaluate(vector<Token> expression, int depth = 0) {
            //cout << "Depth: " << depth << endl;

            vector<string> present_operators;

            for (int i = 0; i < expression.size(); i++) {
                if (expression[i].get_type().substr(0, 8) == "operator" || expression[i].get_type() == "leftparanthesis") {
                    present_operators.push_back(expression[i].get_value());
                    //cout << expression[i].get_value() << endl;
                }
            }

            bool has_paranthesis = false;
            int i = 0;
            Token current_token;

            if (contains(present_operators, "(")) {
                //cout << "Hello" << endl;
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
                if (contains(present_operators, "=")) {
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
                if (contains(present_operators, "!")) {
                    while (i >= 0) {
                        current_token = expression[i];
                        float right_operand_float;
                        if (current_token.get_type() == "operator:not") {
                            //cout << expression[i + 1].get_value();
                            right_operand_float = stod(expression[i + 1].get_value());
                            if (right_operand_float == 0) expression[i + 1].set_value("1.000000");
                            else expression[i + 1].set_value("0.000000");

                            expression.erase(expression.end() - i - 2);
                            //show_tokens(expression);
                        } 
                        i--;
                    }
                }
                i = 0;
                //Highest Priority for * and /
                if (contains(present_operators, "*", "/", "&")) {
                    while (i < expression.size()) {
                        current_token = expression[i];
                        float left_operand_float, right_operand_float;
                        if (current_token.get_value() == "*" || current_token.get_value() == "/" || current_token.get_value() == "%") {
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
                            else if (current_token.get_value() == "/"){
                                ans.set_type(NUM);
                                ans.set_value(to_string(left_operand_float / right_operand_float));
                                //ans(NUM, to_string(left_operand_float / right_operand_float));
                            }
                            else if(current_token.get_value() == "%"){
                                ans.set_type(NUM);
                                ans.set_value(to_string((int)left_operand_float % (int)right_operand_float));
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
                }

                i = 0;
                // FOr + and -
                if (contains(present_operators, "+", "-")) {
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
                            else if (current_token.get_value() == "-") {
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

                i = 0;
                // for <,>, <=, >=
                if (contains(present_operators, "<", "<=", ">", ">=")) {
                    while (i < expression.size()) {
                        current_token = expression[i];
                        double left_operand_float, right_operand_float;
                        if (current_token.get_value() == "<" || current_token.get_value() == "<=" || current_token.get_value() == ">" || current_token.get_value() == ">=") {
                            //show_tokens(expression);
                            //cout << expression[i + 1].get_value()<< endl << expression[i - 1].get_value()<<endl;
                            left_operand_float = stod(expression[i - 1].get_value());
                            right_operand_float = stod(expression[i + 1].get_value());

                            expression.erase(
                                expression.begin() + i - 1, expression.begin() + i + 2
                            );
                            Token ans;
                            if (current_token.get_value() == "<") {
                                ans.set_type(NUM);
                                ans.set_value(to_string((int)(left_operand_float < right_operand_float)));
                            //ans(NUM, to_string(left_operand_float * right_operand_float));
                            }
                            else if (current_token.get_value() == "<=") {
                                ans.set_type(NUM);
                                ans.set_value(to_string((int)(left_operand_float <= right_operand_float)));
                                //ans(NUM, to_string(left_operand_float / right_operand_float));
                            }
                            else if (current_token.get_value() == ">") {
                                ans.set_type(NUM);
                                ans.set_value(to_string((int)(left_operand_float > right_operand_float)));
                                //ans(NUM, to_string(left_operand_float / right_operand_float));
                            }
                            else if (current_token.get_value() == ">=") {
                                ans.set_type(NUM);
                                ans.set_value(to_string((int)(left_operand_float >= right_operand_float)));
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
                    }
                }

                i = 0;
                // for != and ==
                if (contains(present_operators, "==", "!=")) {
                    while (i < expression.size()) {
                        current_token = expression[i];
                        double left_operand_float, right_operand_float;
                        string right_operand_string, left_operand_string;
                        if (current_token.get_value() == "==" || current_token.get_value() == "!=") {
                            //cout << "New" << endl;
                            //show_tokens(expression);
                            //cout << expression[i + 1].get_value()<< endl << expression[i - 1].get_value()<<endl;
                            if (expression[i - 1].get_type() == NUM && expression[i + 1].get_type() == NUM) {
                                left_operand_float = stod(expression[i - 1].get_value());
                                right_operand_float = stod(expression[i + 1].get_value());
                            }
                            else if (expression[i - 1].get_type() == STR && expression[i + 1].get_type() == STR){
                                left_operand_string = expression[i - 1].get_value();
                                right_operand_string = expression[i + 1].get_value();
                            }

                            Token ans;
                            if (current_token.get_value() == "==") {
                                ans.set_type(NUM);
                                if (expression[i - 1].get_type() == NUM && expression[i + 1].get_type() == NUM)
                                    ans.set_value(to_string((int)(left_operand_float == right_operand_float)));
                                else if ((expression[i - 1].get_type() == STR && expression[i + 1].get_type() == STR))
                                    ans.set_value(to_string((int)(left_operand_string == right_operand_string)));
                            //ans(NUM, to_string(left_operand_float * right_operand_float));
                            }
                            else if (current_token.get_value() == "!=") {
                                ans.set_type(NUM);
                                if (expression[i - 1].get_type() == NUM && expression[i + 1].get_type() == NUM)
                                    ans.set_value(to_string((int)(left_operand_float != right_operand_float)));
                                else if ((expression[i - 1].get_type() == STR && expression[i + 1].get_type() == STR))
                                    ans.set_value(to_string((int)(left_operand_string != right_operand_string)));
                                //ans(NUM, to_string(left_operand_float / right_operand_float));
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

            }
            if (expression.size() == 0) expression.emplace_back("num", "0.000000");
            return expression[0];
        }

        void interpret(vector<string> ass_code) {
            vars.store("endl", STR, "\n");
            //ifstream in("program.nv");
            string line;
            //while(getline(in, line)) ass_code.push_back(line);
            
            int line_no = 0, sub_line_no = 0;
            int no_of_steps = 0;

            while (line_no < ass_code.size() && line_no >= 0) {
                no_of_steps++;
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
                    sub_line_no = line_no + 1;
                    //vars.show_all();
                    while (ass_code[sub_line_no] != ";") {
                        Variable &var = vars.get(ass_code[sub_line_no]);
                        //cout << var.get_value();
                        if (var.get_data_type() == NUM) {
                            float num;
                            cin >> num;
                            //vars.show_all();
                            vars.edit(ass_code[sub_line_no], to_string(num));
                            //var.set_value(to_string(num));
                            //vars.show_all();
                        }

                        else if (var.get_data_type() == STR) {
                            string str;
                            cin >> str;
                            vars.edit(ass_code[sub_line_no], str);
                        }
                        sub_line_no++;
                    }
                    line_no = sub_line_no;
                }

                else if (line == "expr") {
                    sub_line_no = line_no + 1;
                    while (ass_code[sub_line_no] != ";") {
                        //cout << ass_code[sub_line_no]<<endl;
                        //vars.show_all();
                        //show_tokens(tokenize_line(ass_code[sub_line_no]));
                        evaluate(tokenize_line(ass_code[sub_line_no])).get_value();
                        sub_line_no++;
                        //cout << sub_line_no << endl;
                    }
                    line_no = sub_line_no;
                    //vars.show_all();
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

                line_no++;
            }
            
            cout << endl << "Number of steps: " << no_of_steps << endl;
        }

        vector<string> translate_to_ass_code() {
            vector<string> ass_code;
            int line_no = 0, sub_line_no = 0;
            vector<int> trackers;
            int endable_keywords_tracker = 0;

            while (line_no < code_lines.size()) {
                vector<Token> tokens = tokenize_line("", line_no);
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
                        ass_code.push_back(";");
                    }
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

                        if (tokens[i].get_type() == "identifier") {
                            current_expression += tokens[i].get_value();
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
                    

                    for (int i = 0; i < initialized.size(); i++) {
                        //cout << expressions[i];
                        ass_code.push_back("expr");
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
                        ass_code.push_back(";");
                    }
                }

                else if(type == "keyword" && value == "while") {
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
                    string loop_var = ass_code[trackers[trackers.size() - 1]];
                    string increment = ass_code[trackers[trackers.size() - 1] + 1];
                    ass_code.push_back("expr");
                    ass_code.push_back(loop_var + " = " + loop_var + " + " + increment);
                    ass_code.push_back(";");
                    endable_keywords_tracker--;
                    ass_code.push_back("goto");
                    ass_code.push_back(to_string(trackers[trackers.size() - 1] - 2));
                    ass_code.push_back(";");
                    ass_code[trackers[trackers.size() - 1]] = to_string(ass_code.size());
                    ass_code[trackers[trackers.size() - 1] + 1] = ";";
                    trackers.pop_back();
                }

                else if (type == "keyword" && value == "end") {
                    endable_keywords_tracker--;
                    ass_code.push_back("goto");
                    ass_code.push_back(to_string(trackers[trackers.size() - 1] - 2));
                    ass_code.push_back(";");
                    ass_code[trackers[trackers.size() - 1]] = to_string(ass_code.size());
                    trackers.pop_back();
                }

                else if(type == "keyword" && value == "if") {
                    last_conditional = "if";
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
                    last_conditional = "elif";
                    ass_code.push_back("goto");
                    ass_code.push_back("PLC");
                    ass_code.push_back(";");
                    ass_code[if_tracker[if_tracker.size() - 1]] = to_string(ass_code.size());
                    if_elif_tracker[if_elif_tracker.size() - 1].push_back(ass_code.size() - 2);
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
                    last_conditional = "else";
                    ass_code.push_back("goto");
                    ass_code.push_back("PLC");
                    ass_code[if_tracker[if_tracker.size() - 1]] = to_string(ass_code.size());
                    if_elif_tracker[if_elif_tracker.size() - 1].push_back(ass_code.size() - 1);
                    if_tracker.pop_back();
                    ass_code.push_back(";");
                }

                else if (type == "keyword" && value == "endif") {
                    if (last_conditional != "if") {
                        while (if_elif_tracker[if_elif_tracker.size() - 1].size() > 0) {
                            ass_code[if_elif_tracker[if_elif_tracker.size() - 1][if_elif_tracker[if_elif_tracker.size() - 1].size() - 1]] = to_string(ass_code.size());
                            if_elif_tracker[if_elif_tracker.size() - 1].pop_back();
                        }
                        if_elif_tracker.pop_back();
                    }
                    else {
                        ass_code[if_tracker[if_tracker.size() - 1]] = to_string(ass_code.size());
                        if_tracker.pop_back();
                    }
                }

                line_no++;
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
    vector<string> ass_code = jhapascript.translate_to_ass_code();
    clock_t start = clock();   // Start timing
    jhapascript.interpret(ass_code);
    clock_t end = clock();     // End timing
    double time_taken = double(end - start) / CLOCKS_PER_SEC; // Convert to seconds
    cout << endl << "Time taken: " << time_taken << " seconds" << endl;
    //getch();
    return 0;
}