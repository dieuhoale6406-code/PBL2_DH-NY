#include "core/Trie.h"

// TrieNode
TrieNode::TrieNode():end(false) {
    // quản lý nút
    for (int i = 0; i < 26; i++) {
        children[i] = nullptr;
    }
}

TrieNode::~TrieNode() {} // destructor

TrieNode* TrieNode::getChild(char c) const {
    // c-'a' ---> chỉ số mảng của ký tự c
    return children[c - 'a'];
}

void TrieNode::setChild(char c, TrieNode* node) {
    // gắn nút node và vị trí ký tự c
    children[c - 'a'] = node;
}

bool TrieNode::hasChild(char c) const {
    // kiểm tra nút ở vị trí c có nhánh con không
    return children[c - 'a'] != nullptr;
}

bool TrieNode::isEnd() const {
    // kiểm tra kết thúc từ
    return this->end;
}

void TrieNode::setEnd(bool value) {
    // đặt trạng thái kết thúc
    this->end = value;
}

// Trie
Trie::Trie() {
    // tạo cây Trie mới
    root = new TrieNode();
}

Trie::~Trie() {
    // giải phóng cây Trie
    deleteNode(root);
}

void Trie::deleteNode(TrieNode* node) {
    // giải phóng các nút con
    if (!node) return;
    for (int i = 0; i < 26; i++) {
        if (node->getChild('a' + i)) {
            deleteNode(node->getChild('a' + i));
        }
    }
    delete node;
}

void Trie::insert(const string& word) {
    //chèn từ mới vào Trie
    TrieNode* node = root;
    for (char c : word) {
        if (!node->hasChild(c)) {
            node->setChild(c, new TrieNode());
        }
        node = node->getChild(c);
    }
    node->setEnd(true);
}

bool Trie::search(const string& word) const {
    // tìm từ hoàn chỉnh trong Trie
    TrieNode* node = root;
    for (char c : word) {
        if (!node->hasChild(c)) return false;
        node = node->getChild(c);
    }
    return node->isEnd();
}

bool Trie::startsWith(const string& prefix) const {
    // xem có từ bắt đầu bằng chuỗi prefix không
    TrieNode* node = root;
    for (char c : prefix) {
        if (!node->hasChild(c)) return false;
        node = node->getChild(c);
    }
    return true;
}
