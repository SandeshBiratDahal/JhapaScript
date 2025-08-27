#include<iostream>
#include<vector>
using namespace std;

int find(vector<string> elements, string to_find) {
    for (int i = 0; i < elements.size(); i++) {
        if (elements[i] == to_find) return i;
    }
    return -1;
}