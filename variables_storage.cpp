#include<iostream>
#include<map>

using namespace std;

class Variable{
    string data_type;
    string value;

    public:
        Variable() {}
        Variable(string data_type, string value) {
            this -> data_type = data_type;
            this -> value = value;
        }

        string get_value() {
            return value;
        }

        string get_data_type() {
            return data_type;
        }

        string set_value(string value) {
            this -> value = value;
        }
};

class VariableStorage{
    map<string,Variable> storage;

    public:
        void store(string identifier, string data_type, string value) {
            Variable n(data_type, value);
            storage[identifier] = n;
        }

        string get_value(string identifier) {
            return storage[identifier].get_value();
        }

        void show_all() {
            for (auto &pair : storage) {
                cout << pair.first << " -> " << pair.second.get_value() << endl;
            }
        }

        void edit(string identifier, string value) {
            storage[identifier].set_value(value);
        }
};