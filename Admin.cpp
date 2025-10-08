#include <iostream>
#include "Admin.h"

using namespace std;

Admin::Admin(const string& i, const string& pwd, const string& fullname, const string& phone)
        : Account(i, pwd, fullname, "Admin") {
        this->phone = phone;
}

void Admin::showInfo() const {
    cout << "[ADMIN]\n";
    Account::showInfo();
}

void Admin::createWarehouse() {
    cout << "Admin tao kho moi" << endl;
}

void Admin::deleteWarehouse() {
    cout << "Admin xoa kho" << endl;
}

void Admin::assignStaff() {
    cout << "Admin gan staff" << endl;
}