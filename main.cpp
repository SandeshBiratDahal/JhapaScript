#include<iostream>
#include<fstream>
#include<vector>
#include<string>

#include "tokens.cpp"
#include "utilities.cpp"

using namespace std;

class Interpreter{

    string raw_code;
    vector<string> code_lines;

    string alphas = "qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM_",
    nums = "1234567890", specials = "!@#$%^&*()+{}[]:;'<>,.?/|=-~`", space = " ";

    vector<string> keywords = 
    {
        "int", "float", "str", "for", "loop", "if", "then", "endif", "else", "elif"
    };

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
            for(int i = 0; i < code_lines.size(); i++) {
                cout << code_lines[i] << endl;
            }
        }

        // returns all the tokens as objects of class 'Token' in a given line of code
        vector<Token> tokenize_line(int index) {

            vector<Token> tokens;
            string line = code_lines[index], current_token;
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
                    
                    if (has_decimal) tokens.emplace_back("float", current_token);
                    else tokens.emplace_back("integer", current_token);

                    i = j - 1;

                }

                i++;
            }

            return tokens;
        } 

};


int main() {
    Interpreter nova;
    nova.read("program.nv");
    vector<Token> n = nova.tokenize_line(0);

    cout << n[2].repr();
    return 0;
}