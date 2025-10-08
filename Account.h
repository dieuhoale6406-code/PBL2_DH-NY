#include <string>

using namespace std;

class Account {
    protected:
        string id;
        string password;
        string fullName;
        string phone;
        string role;
    public:
        Account();
        Account(const string&, const string&, const string&, const string&);
        ~Account();
        virtual bool Login(const string&, const string&) const;
        void ResetPassword(const string&);
        string getRole() const;
        virtual void showInfo() const;
        string getId() const;
};