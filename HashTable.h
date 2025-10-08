#include <iostream>

#ifndef HashTable_h
#define HashTable_h

using namespace std;

class HashNode {
    public:
        string key;
        int index;
        HashNode* next;
        HashNode(string,int);
};

class HashTable {
    private:
        int capacity;
        HashNode** Hash;
        int HashFunc(const string&) const;
    public:
        HashTable(int = 101);
        ~HashTable();
        void insert(const string&, int);
        int find(const string&) const;
        void remove(const string&);
        void show() const;
};

#endif