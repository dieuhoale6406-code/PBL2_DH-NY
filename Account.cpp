#include "Account.h"
#include <string>
#include <iostream>

using namespace std;

Account::Account(const string& i, const string& pwd, const string& fullname, const string& r)
    : id(i), password(pwd), fullName(fullname), role(r) {}

bool Account::Login(const string& i, const string& pwd) const {
    return i == id && pwd == password;
}

void Account::ResetPassword(const string& newpwd) {
    this->password = newpwd;
}

string Account::getRole() const {
    return this->role;
}

void Account::showInfo() const {
    cout << "USER ID:" << this->id << endl;
    cout << "NAME:" << this->fullName << endl;
    cout << "PHONE.NO:" << this->phone << endl;
    cout << "ROLE:" << this->role << endl;
}

string Account::getId() const {
    return this->id;
}