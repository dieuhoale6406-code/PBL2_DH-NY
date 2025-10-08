#include "Heap.h"
#include <climits>

Heap::Heap(unsigned int cap, const int arr[])
    : capacity(cap), heap_size(cap) {
    this->data = new int[capacity + 1];
    for (unsigned int i = 1; i <= capacity; i++)
        this->data[i] = arr[i];
}

Heap::~Heap() {
    delete[] this->data;
}

void Heap::maxHeapify(unsigned int i) {
    unsigned int l = 2 * i;
    unsigned int r = 2 * i + 1;
    unsigned int largest = i;
    if (l <= heap_size && data[l] > data[largest])
        largest = l;
    if (r <= heap_size && data[r] > data[largest])
        largest = r;
    if (largest != i) {
        int temp = data[i];
        data[i] = data[largest];
        data[largest] = temp;
        maxHeapify(largest);
    }
}

void Heap::buildMaxHeap() {
    heap_size = capacity;
    for (int i = capacity / 2; i >= 1; i--)
        maxHeapify(i);
}

void Heap::show() const {
    for (unsigned int i = 1; i <= capacity; i++)
        cout << data[i] << " ";
    cout << endl;
}

void Heap::heapSort() {
    buildMaxHeap();
    for (int i = capacity; i >= 2; i--) {
        int temp = data[1];
        data[1] = data[i];
        data[i] = temp;
        heap_size--;
        maxHeapify(1);
    }
}

int Heap::heapMaximum() const {
    return data[1];
}

int Heap::heapExtractMax() {
    if (heap_size < 1) {
        cout << "Heap underflow!" << endl;
        return 0;
    }
    int max = data[1];
    data[1] = data[heap_size];
    heap_size--;
    maxHeapify(1);
    return max;
}

void Heap::heapIncreaseKey(unsigned int i, int key) {
    if (key < data[i]) {
        cout << "The new key is smaller than current key!" << endl;
        return;
    }
    data[i] = key;
    while (i > 1 && data[i] > data[i / 2]) {
        int temp = data[i];
        data[i] = data[i / 2];
        data[i / 2] = temp;
        i /= 2;
    }
}

void Heap::heapInsert(int key) {
    heap_size++;
    capacity++;
    int* newData = new int[capacity + 1];
    for (unsigned int i = 1; i < capacity; i++)
        newData[i] = data[i];
    delete[] data;
    data = newData;

    data[heap_size] = INT_MIN;
    heapIncreaseKey(heap_size, key);
}