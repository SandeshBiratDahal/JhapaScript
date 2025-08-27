#include<iostream>
#include<fstream>
#include<vector>
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
            }
        }

        void show_raw() {
            cout << raw_code;
        }

};


int main() {
    Interpreter nova;
    nova.read("program.nv");
    nova.show_raw();
    return 0;
}