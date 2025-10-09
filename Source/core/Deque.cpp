#include "core/queue.h"
#include <utility> 

// ---- Node ----
template <typename T>
Queue<T>::Node::Node(const T& v) : data(v), next(nullptr) {}

template <typename T>
Queue<T>::Node::Node(T&& v) : data(std::move(v)), next(nullptr) {}

// ---- Core ----
template <typename T>
Queue<T>::Queue() noexcept : _head(nullptr), _tail(nullptr), _size(0) {}

template <typename T>
void Queue<T>::copy_from(const Queue& other){
    for (Node* p = other._head; p; p = p->next) push(p->data);
}

template <typename T>
Queue<T>::Queue(const Queue& other) : Queue() { copy_from(other); }

template <typename T>
Queue<T>::Queue(Queue&& other) noexcept
: _head(other._head), _tail(other._tail), _size(other._size) {
    other._head = other._tail = nullptr;
    other._size = 0;
}

template <typename T>
Queue<T>& Queue<T>::operator=(const Queue& other){
    if (this != &other) { clear(); copy_from(other); }
    return *this;
}

template <typename T>
Queue<T>& Queue<T>::operator=(Queue&& other) noexcept{
    if (this != &other){
        clear();
        _head = other._head; _tail = other._tail; _size = other._size;
        other._head = other._tail = nullptr; other._size = 0;
    }
    return *this;
}

template <typename T>
Queue<T>::~Queue(){ clear(); }

template <typename T>
std::size_t Queue<T>::size() const noexcept { return _size; }

template <typename T>
bool Queue<T>::empty() const noexcept { return _size == 0; }

template <typename T>
void Queue<T>::clear() noexcept {
    Node* p = _head;
    while (p){
        Node* nx = p->next;
        delete p;
        p = nx;
    }
    _head = _tail = nullptr;
    _size = 0;
}

template <typename T>
void Queue<T>::push(const T& value){
    Node* n = new Node(value);
    if (!_tail) { _head = _tail = n; }
    else { _tail->next = n; _tail = n; }
    ++_size;
}

template <typename T>
void Queue<T>::push(T&& value){
    Node* n = new Node(std::move(value));
    if (!_tail) { _head = _tail = n; }
    else { _tail->next = n; _tail = n; }
    ++_size;
}

template <typename T>
bool Queue<T>::pop(){
    if (!_head) return false;
    Node* old = _head;
    _head = _head->next;
    if (!_head) _tail = nullptr;
    delete old;
    --_size;
    return true;
}

template <typename T>
T& Queue<T>::front(){
    if (!_head) throw std::runtime_error("Queue::front() on empty queue");
    return _head->data;
}

template <typename T>
const T& Queue<T>::front() const{
    if (!_head) throw std::runtime_error("Queue::front() on empty queue");
    return _head->data;
}

#include <string>

#define QUEUE_INSTANTIATE(Typ) \
    template class Queue<Typ>;

QUEUE_INSTANTIATE(int)
QUEUE_INSTANTIATE(double)
QUEUE_INSTANTIATE(std::string)
