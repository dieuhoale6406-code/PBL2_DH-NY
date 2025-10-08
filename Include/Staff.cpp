#include "Staff.h"

using namespace std;

Staff::Staff(const string& i, const string& pwd, const string& fullname, const string& phone)
    : Account(i, pwd, fullname, "Staff") {
        this->phone = phone;
}

void Staff::showInfo() const {
    cout << "[STAFF]" << endl;
    Account::showInfo();
}

// Các quyền riêng của Staff
void Staff::viewWarehouse() {
    cout << "Staff xem danh sach kho" << endl;
}
void Staff::exportGoods() {
    cout << "Staff xuat hang" << endl;
}