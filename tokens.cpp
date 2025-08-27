#include<iostream>
using namespace std;

class Token{
    string type;
    string value;
    public:
    Token(string type, string value){
        this -> type = type;
        this -> value = value;
    }

    string get_type() {
        return type;
    }

    string get_value() {
        return value;
    }
};