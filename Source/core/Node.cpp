#include "Node.h"
#include <iostream>

using namespace std;


Node::Node(int data)
    : data(data) {
    this->next = NULL;
}
Node::Node(int* list, int size) {
    Node *first_data = this;
    first_data->data = list[0];
    first_data->next = NULL;
    for (int i = 1; i < size; i++) {
        Node *new_data = new Node(list[i]);
        first_data->next = new_data;
        first_data = first_data->next;
    }
}


Node::Node(Node* node) {
    Node *tmp = this;
    Node *op = node;
    tmp = new Node;
    tmp->data = op->data;
    tmp->next = op->next;
    while (op) {
        op = op->next;
        Node *new_data = new Node;
        tmp->data = op->data;
        tmp->next = op->next;
        tmp->next = new_data;
        tmp = tmp->next;
    }
}


Node::~Node() {
    delete this->next;
    delete this;
}


int Node::operator[](int index) {
    Node *tmp = this;
    while (index > 0 && tmp != NULL) {
        tmp = tmp->next;
        index--;
    }
    if (tmp != NULL) 
        return tmp->data;
    return -9999;
}

int Node::getData() {
    return this->data;
}

void Node::display() {
    int i = 0;
    Node *tmp = this;
    cout << "Linked Node: " << endl;
        while (tmp) {
            cout << i << ": " << tmp->data << endl;
            tmp = tmp->next;
            i++;
        }
}

void Node::displayData() {
    cout << this->data << endl;
}

void Node::push_back(int data) {
    Node *tp = this;
    while (tp->next != NULL) 
        tp = tp->next;
    Node *new_data = new Node(data);
    tp->next = new_data;
}


void Node::add_head(int data) {
    Node *new_data = new Node(this->data);
    this->data = data;
    this->next = new_data;
}


void Node::add(int data, int index) {
    Node *tp = this;
    Node *new_data = new Node(data);
    while (index > 1 && tp != NULL) {
        tp = tp->next;
        index--;
    }

    if (tp != NULL) {
        Node *temp = tp->next;
        tp->next = new_data;
        new_data->next = temp;
    }
}


void Node::update(int index) {
    Node *tmp = this;
    while (tmp && index > 0) {
        tmp = tmp->next;
        index--;
    }
        
    if (tmp != NULL) {
        cout << "New value: " << endl;
        cin >> tmp->data;
    }
    else cout << "Empty index" << endl;
}


int Node::pop_back() {
    int result = -9999;
    Node *tmp = this;
    while (tmp->next->next != NULL) 
        tmp = tmp->next;
    Node* temp = tmp->next;
    result = tmp->next->data;
    tmp->next = NULL;
    return result;
}


int Node::pop_head() {
    int result = -9999;
    Node *tmp = this->next;
    result = this->data;
    this->data = tmp->data;
    this->next = tmp->next;
    return result;
    delete tmp->next;
    delete tmp;
    return result;
}


void Node::remove(int index) {
    Node *tmp = this;
    while (index > 1 && tmp != NULL) {
        tmp = tmp->next;
        index--;
    }

    if (tmp != NULL) {
        Node *temp = tmp->next;
        tmp->next = temp->next;
        tmp->data = temp->data;
    }
}


Node* Node::middle(Node* start, Node* end){
    if (start == NULL)
        return NULL;
    Node* slow = start;
    Node* fast = start -> next;
 
    while (fast != end) {
        fast = fast -> next;
        if (fast != end) {
            slow = slow -> next;
            fast = fast -> next;
        }
    }
    return slow;
}

Node* Node::search(int value) {
    Node* start = this;
    Node* end = NULL;
    do {
        Node* mid = middle(start, end);
        if (mid == NULL)
            return NULL;
        if (mid->data == value)
            return mid;
        else if (mid->data < value)
            start = mid->next;
        else end = mid;
 
    } while (end == NULL || end != start);
    return NULL;
}


int Node::getTail() {
    Node *tmp = this;
    while (tmp != NULL && tmp->next != NULL)
        tmp = tmp->next;
    return tmp->data;
}