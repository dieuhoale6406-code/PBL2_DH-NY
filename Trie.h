#include <iostream>
#include <string>

using namespace std;

class TrieNode {
    private:
        TrieNode* children[26];
        bool end;
    public:
        TrieNode();
        ~TrieNode();

        TrieNode* getChild(char) const;
        void setChild(char, TrieNode*);
        bool hasChild(char) const;
        bool isEnd() const;
        void setEnd(bool);
};

class Trie {
    private:
        TrieNode* root;
    public:
        Trie();
        ~Trie();

        void insert(const string&);
        bool search(const string&) const;
        bool startsWith(const string&) const;
    private:
        void deleteNode(TrieNode*);
};
