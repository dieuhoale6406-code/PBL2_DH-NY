#ifndef STAFF_H
#define STAFF_H

#include "Account.h"
#include <iostream>

using namespace std;

class Staff : public Account {
public:
    Staff(const string&, const string&, const string&, const string&);

    void showInfo() const override;

    // Các quyền riêng của Staff
    void viewWarehouse();
    void exportGoods();
};

#endif