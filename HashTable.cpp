#include "Hashtable.h"
#include <iostream>

using namespace std;

HashNode::HashNode(string k, int i)
    : key(k), index(i) {
        this->next = NULL;
}

HashTable::HashTable(int cap)
    : capacity(cap) {
        this->Hash = new HashNode*[this->capacity]; // khởi tạo node
        for(int i = 0; i < this->capacity; i++) *(this->Hash + i) = NULL;
}

HashTable::~HashTable() {
    for(int i = 0; i < this->capacity; i++) { // giải phóng từng bucket
        HashNode* current = *(this->Hash + i);
        while(current) { // duyệt và xóa từng node trong bucket i
            HashNode *tmp = current->next;
            delete current;
            current = tmp;
        }
    }
    delete [] this->Hash;
}

int HashTable::HashFunc(const string& key) const {
    unsigned long h = 0;
    for(char c : key)
        h = h * 31 + c; //chuyển string key thành số
    return h % this->capacity;
}

void HashTable::insert(const string& key, int index) {
    int pos = HashFunc(key);
    HashNode* current = *(this->Hash + pos);
    while(current) { // nếu tồn tại key thì upd index
        if(current->key == key) {
            current->index = index;
            return;
        }
        current = current->next;
    }
    HashNode* node = new HashNode(key,index); // chưa có key thì chèn node mới vào đầu bucket
    node->next = *(this->Hash + pos);
    *(this->Hash + pos) = node;
}

int HashTable::find(const string& key) const {
    int pos = HashFunc(key);
    HashNode* current = *(this->Hash + pos);
    while(current) { // có key trong bucket thì trả về giá trị
        if(current->key == key) return current->index;
        current = current->next;
    }
    return -1; // không thấy trả về -1
}

void HashTable::remove(const string& key) { //xóa key khỏi hash
    int pos = HashFunc(key);
    HashNode* current = *(this->Hash + pos);
    HashNode* prev = NULL;
    while(current) {
        if(current->key == key) {
            if(prev) prev->next = current->next; // trước key có số thì lk sau prev là next của key đó
            else *(this->Hash + pos) = current->next; // key ở đầu bucket thì next của key thành đầu bucket
            delete current;
            return;
        }
        prev = current; //không phải key thì qua node tiếp theo
        current = current->next;
    }
}

void HashTable::show() const {
    for(int i = 0; i < this->capacity; i++) {
        if(*(this->Hash + i)) {
            cout << i << ": ";
            HashNode* current = *(this->Hash + i);
            while(current) {
                cout << "       " << current->key << " - " << current->index << endl;
                current = current->next;
            }
            cout << endl;
        }
    }
}