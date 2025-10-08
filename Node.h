#ifndef node_h
#define node_h

#include <iostream>

class Node {
    protected:
        int data; // giá trị
        Node *next;

    public:
        Node(int = -9999); // default
        Node(int*, int); // constructor từ list và size
        Node(Node*); // copy constructor
        ~Node(); // destructor

        int getData();
        int operator[](int);

        void display();
        void displayData();
        int getTail();
        Node* middle(Node*, Node*);
        void push_back(int);
        void add_head(int);
        void add(int, int);
        void update(int);
        int pop_back();
        int pop_head();
        void remove(int); 
        Node* search(int);
};
 
#endif