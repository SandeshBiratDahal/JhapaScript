#include<iostream>
#include<vector>
using namespace std;

//searches for a specific string among a vector of strings
//Used mainly to identify if a word is keyword or not
int find(vector<string> elements, string to_find) {
    for (int i = 0; i < elements.size(); i++) {
        if (elements[i] == to_find) return i;
    }
    return -1;
}