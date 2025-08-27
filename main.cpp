#include<iostream>
#include<fstream>
#include<vector>

#include "tokens.cpp"
using namespace std;

class Interpreter{

    string raw_code;
    vector<string> code_lines;

    public:
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

        vector<Token> tokenize_line(int index) {

            vector<Token> tokens;

            cout << code_lines[index] << endl;

            return tokens;
        } 

};


int main() {
    Interpreter nova;
    nova.read("program.nv");
    nova.show_code_lines();

    nova.tokenize_line(0);
    return 0;
}