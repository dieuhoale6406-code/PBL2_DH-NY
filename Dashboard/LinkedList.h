#include <iostream>

template<typename T>

class node {
    public:
        T data;
        node<T> *prev;
        node<T> *next;
        node();
        node(const node<T>& n);
        node(const T&);
};