#include<iostream>
using namespace std;


// Just a place to store tokens for tokenization
class Token{
    string type;
    string value;
    public:
    Token(string type, string value){
        this -> type = type;
        this -> value = value;
    }

    Token() {
        type = "";
        value = "";
    }

    string get_type() {
        return type;
    }

    string get_value() {
        return value;
    }

    string repr() {
        return "<Token \ntype=" + type + ",\nvalue=" + value + ">";
    }
};