#ifndef QUEUE_H
#define QUEUE_H

#include <iostream>
using namespace std;

class Queue {
private:
    struct Node {
        int data;
        Node* next;
        Node(int v);
    };

    Node* head;
    Node* tail;
    unsigned int qsize;

public:
    Queue();            // constructor
    ~Queue();           // destructor

    void push(int value);
    bool pop();
    int front() const;
    bool empty() const;
    unsigned int size() const;
    void clear();
    void show() const;
};

#endif