// deque.h - Deque (2 đầu) tự cài đặt, không dùng STL container
#pragma once
#include <cstddef>
#include <stdexcept>

template <typename T>
class Deque {
private:
    struct Node {
        T data;
        Node* prev;
        Node* next;
        Node(const T& v);
        Node(T&& v);
    };

    Node* _head;
    Node* _tail;
    std::size_t _size;

    void copy_from(const Deque& other);

public:
    Deque() noexcept;
    Deque(const Deque& other);
    Deque(Deque&& other) noexcept;
    Deque& operator=(const Deque& other);
    Deque& operator=(Deque&& other) noexcept;
    ~Deque();

    std::size_t size() const noexcept;
    bool empty() const noexcept;
    void clear() noexcept;

    void push_front(const T& v);
    void push_front(T&& v);
    void push_back(const T& v);
    void push_back(T&& v);

    bool pop_front(); 
    bool pop_back();

    T& front();               
    const T& front() const;   
    T& back();                
    const T& back() const; 
};

#define DEQUE_DECLARE_INSTANTIATIONS(Typ) \
    extern template class Deque<Typ>;
