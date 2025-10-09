#ifndef set_h
#define set_h

#include "Node.h"

class Set : public Node {
    public:
        Set(int = -9999);
        Set(int*, int);
        Set(Set&);
        ~Set();
        void insert(int);
        void display();
        // Set* operator=(Set* other);
        Set* operator+(Set*);
        Set* operator*(Set*);
        Set* operator-(Set*);
        const Set& operator=(Set&);
};

#endif