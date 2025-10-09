#include "core/Set.h"

Set::Set(int data)
    : Node(data) {}

void Set::insert(int data) {
    Set* tmp = this;
    Set* prev = NULL;
    while (tmp != NULL && tmp->data < data) {
        prev = tmp;
        tmp = (Set*)tmp->next;
    }
    if (tmp == NULL) {
        Set* new_data = new Set(data);
        prev->next = new_data;
    }    
    else {
        Set* new_data = new Set(tmp->data);
        new_data->next = tmp->next;
        tmp->data = data;
        tmp->next = new_data;
    }
}

Set::Set(int* array, int size) {
    if (size > 0) {
        this->data = array[0];
        for (int i = 1; i < size; i++) {
            this->insert(array[i]);
        }
    }
} 

Set::Set(Set& other) {
    if (this != &other) {
        Set *temp = this;
        Set *p = &other;
        while (p != NULL) {
            if (temp != this) {
                Set *new_data = new Set(p->data);
                temp = new_data;
            }
            else {
                Set *new_data = new Set(p->data);
                temp->next = new_data;
            }
            p = (Set*)p->next;
        }
    }
}

Set::~Set() {
    delete this;
}

void Set::display() {
    int i = 0;
    Set *tmp = this;
    while (tmp) {
        std::cout << i << ": " << tmp->data << std::endl;
        tmp = (Set*)tmp->next;
        i++;   
    }
}

const Set &Set::operator=( Set& other){
    if (this != &other) {
        Set *temp = this;
        Set *p = &other;
        while (p != NULL) {
            if (temp != this){
                Set *new_data = new Set(p->data);
                temp = new_data;
            }
            else{
                Set *new_data = new Set(p->data);
                temp->next = new_data;
            }
            p = (Set*)p->next;
    }
    }
    return *this;
}
Set* Set::operator+(Set* other) {
    Set *result = new Set(-9999);
    Set *tmp = this;
    Set *op = other;
    Set *rp = result;
    while (tmp != NULL && op != NULL) {
        if (tmp->data < op->data) {
            if (rp->data == -9999) {
                rp->data = tmp->data;
            }
            else {
                Set *new_data = new Set(tmp->data);
                rp->next = new_data;
                rp = (Set*)rp->next;
            }
            tmp = (Set*)tmp->next;
        }
        else if (tmp->data > op->data) {
            if (rp->data == -9999) {
                rp->data = op->data;
            }
            else {
                Set *new_data = new Set(op->data);
                rp->next = new_data;
                rp = (Set*)rp->next;
            }
            op = (Set*)op->next;
        }
        else {
            if (rp->data == -9999) {
                rp->data = tmp->data;
            }
            else {
                Set *new_data = new Set(op->data);
                rp->next = new_data;
                rp = (Set*)rp->next;
            }
            tmp = (Set*)tmp->next;
            op = (Set*)op->next;
        }
    }

    while (tmp != NULL) {
        if (rp->data == -9999) {
            rp->data = tmp->data;
        }
        else {
            Set *new_data = new Set(tmp->data);
            rp->next = new_data;
            rp = (Set*)rp->next;
        }
        tmp = (Set*)tmp->next;
    }

    while (op != NULL) {
        if (rp->data == -9999) {
            rp->data = op->data;
        }
        else {
            Set *new_data = new Set(op->data);
            rp->next = new_data;
            rp = (Set*)rp->next;
        }
        op = (Set*)op->next;
    }

    return result;
}

Set* Set::operator*(Set* other) {
    Set *result = new Set(-9999);
    Set *tmp = this;
    Set *op = other;
    Set *rp = result;
    while (tmp != NULL && op != NULL) {
        if (tmp->data < op->data) {
            tmp = (Set*)tmp->next;
        }
        else if (tmp->data > op->data) {
            op = (Set*)op->next;
        }
        else {
            if (rp->data == -9999) {
                rp->data = tmp->data;
            }
            else {
                Set *new_data = new Set(op->data);
                rp->next = new_data;
                rp = (Set*)rp->next;
            }
            tmp = (Set*)tmp->next;
            op = (Set*)op->next;
        }
    }
    return result;
}

Set* Set::operator-(Set* other) {
    Set *result = new Set(-9999);
    Set *tmp = this;
    Set *op = other;
    Set *rp = result;
    while (tmp != NULL && op != NULL) {
        if (tmp->data < op->data) {
            if (rp->data == -9999) {
                rp->data = tmp->data;
            }
            else {
                Set *new_data = new Set(tmp->data);
                rp->next = new_data;
                rp = (Set*)rp->next;
            }
            tmp = (Set*)tmp->next;
        }
        else if (tmp->data > op->data) {
            op = (Set*)op->next;
        }
        else {
            tmp = (Set*)tmp->next;
            op = (Set*)op->next;
        }
    }

    while (tmp != NULL) {
        if (rp->data == -9999) {
            rp->data = tmp->data;
        }
        else {
            Set *new_data = new Set(tmp->data);
            rp->next = new_data;
            rp = (Set*)rp->next;
        }
        tmp = (Set*)tmp->next;
    }

    return result;
}