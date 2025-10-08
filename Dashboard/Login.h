#ifndef LOGIN_H
#define LOGIN_H

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

struct Account {
    std::string user, pass, role;
};

/// Hiển thị màn hình đăng nhập.
/// @param window  cửa sổ SFML đã mở sẵn
/// @param font    font chữ đã load
/// @param accounts danh sách tài khoản (user, pass, role)
/// @param isAdmin true nếu đăng nhập với role Admin, false nếu Staff
/// @return true nếu login thành công, false nếu user đóng cửa sổ
bool showLoginScreen(sf::RenderWindow& window,
                     sf::Font& font,
                     const std::vector<Account>& accounts,
                     bool isAdmin);

#endif
