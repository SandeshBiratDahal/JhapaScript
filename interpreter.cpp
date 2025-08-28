#include<iostream>
#include<vector>
#include<fstream>

using namespace std;

class Interpreter{
    vector<string> ass_code;

    public:
        Interpreter(string file_path) {
            string line;
            ifstream file_content(file_path);

            while (getline(file_content, line)) {
                if (line != "") ass_code.push_back(line);
            }
        }

        Interpreter (vector<string> ass_code) {
            this -> ass_code = ass_code;
        }

        void show_ass_code(){
            for (int i = 0; i < ass_code.size(); i++) {
                cout << ass_code[i] << endl;
            }
        }
};


int main() {
    Interpreter nova("program.nv");
    nova.show_ass_code();
    return 0;
}