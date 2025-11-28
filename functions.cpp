#include <vector>
#include <string>

using namespace std;

class Param {
    public:
        string type, name;
};

class Function {
    public:
        string name;
        vector<Param> params;
        vector<string> ass_code;
};