#ifndef ADMIN_H
#define ADMIN_H

#include "Account.h"
#include <iostream>

using namespace std;

class Admin : public Account {
public:
    Admin(const string&, const string&,const string&, const string&);
    void showInfo() const override;

    // Các quyền riêng của Admin
    void createWarehouse();
    void deleteWarehouse();
    void assignStaff();
};

#endif