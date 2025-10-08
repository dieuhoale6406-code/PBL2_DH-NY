#include <iostream>
#include "LinkedList.h"

using namespace std;

template<typename T>
node<T>::node() {
    this->prev = NULL;
    this->next = NULL;
}

template<typename T>
node<T>::node(const node<T>& a) {
    this->data = a.data;
    this->prev = NULL;
    this->next = NULL;
}

template<typename T>
node<T>::node(const T &data){
    this->data = data;
    this->prev = NULL;
    this->next = NULL;
}