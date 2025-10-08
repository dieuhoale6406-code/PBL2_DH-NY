#ifndef HEAP_H
#define HEAP_H

#include <iostream>
using namespace std;

class Heap {
private:
    unsigned int capacity;
    unsigned int heap_size;
    int* data;

    void maxHeapify(unsigned int i);
    void buildMaxHeap();

public:
    Heap(unsigned int capacity, const int arr[]);  // constructor
    ~Heap();                                       // destructor

    void show() const;
    void heapSort();
    int heapMaximum() const;
    int heapExtractMax();
    void heapIncreaseKey(unsigned int i, int key);
    void heapInsert(int key);
};

#endif