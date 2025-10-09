#include "core/Queue.h"

Queue::Node::Node(int v) : data(v), next(nullptr) {}

Queue::Queue() : head(nullptr), tail(nullptr), qsize(0) {}

Queue::~Queue() {
    clear();
}

void Queue::push(int value) {
    Node* n = new Node(value);
    if (!tail) {
        head = tail = n;
    } else {
        tail->next = n;
        tail = n;
    }
    qsize++;
}

bool Queue::pop() {
    if (!head) return false;
    Node* tmp = head;
    head = head->next;
    if (!head) tail = nullptr;
    delete tmp;
    qsize--;
    return true;
}

int Queue::front() const {
    if (!head) {
        cout << "Queue underflow!" << endl;
        return 0;
    }
    return head->data;
}

bool Queue::empty() const {
    return qsize == 0;
}

unsigned int Queue::size() const {
    return qsize;
}

void Queue::clear() {
    while (head) {
        Node* tmp = head;
        head = head->next;
        delete tmp;
    }
    tail = nullptr;
    qsize = 0;
}

void Queue::show() const {
    Node* p = head;
    while (p) {
        cout << p->data << " ";
        p = p->next;
    }
    cout << endl;
}