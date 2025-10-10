#pragma once
#include "core/DataModels.h"
#include <vector>
#include <string>

// Lớp này dùng để đọc và quản lý tài khoản từ file CSV
class AccountManager {
public:
    AccountManager(const std::string& filename);
    
    // Kiểm tra username và password, trả về Role nếu thành công
    Role authenticate(const std::string& username, const std::string& password, Role requestedRole);

private:
    struct AccountData {
        std::string username;
        std::string password;
        Role role;
    };
    std::vector<AccountData> mAccounts;

    void loadAccounts(const std::string& filename);
    Role roleFromString(const std::string& roleStr);
};