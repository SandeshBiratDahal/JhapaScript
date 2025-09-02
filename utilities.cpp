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

bool contains(vector<string> a, string target1, string target2 = "X", string target3 = "X", string target4 = "X") {
    for (int i = 0; i < a.size(); i++) {
        if (a[i] == target1 || a[i] == target2 || a[i] == target3 || a[i] == target4) return true;
    }
    return false;
}