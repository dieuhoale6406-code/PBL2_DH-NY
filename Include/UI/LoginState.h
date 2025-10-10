// Include/UI/LoginState.h
#pragma once
#include "core/State.h"
#include "UI/App.h"
#include "UI/RoundedRectangleShape.h"
#include "UI/TextBox.h" // Giả định bạn có lớp TextBox
#include "core/DataModels.h"
#include <SFML/Graphics.hpp>

class LoginState : public State {
public:
    LoginState(App* app, Role role);
    void handleEvent(sf::Event& event) override;
    void update(sf::Time dt) override;
    void draw() override;

private:
    void submitLogin(); // Hàm xử lý logic đăng nhập

    App* mApp;
    Role mSelectedRole;

    // Các thành phần giao diện
    sf::Text mTitle;
    sf::RoundedRectangleShape mFormBg; // Nền trắng cho form

    TextBox mUsernameBox;
    TextBox mPasswordBox;

    sf::RoundedRectangleShape mConfirmButtonBg;
    sf::Text mConfirmButtonText;

    sf::RoundedRectangleShape mBackButtonBg;
    sf::Text mBackButtonText;

    // Biến theo dõi ô nhập liệu đang được chọn (0: username, 1: password)
    int mActiveBox = 0; 
    
    // Bảng màu
    sf::Color mPurpleBgColor = sf::Color(220, 210, 250);
    sf::Color mButtonBgColor = sf::Color(255, 255, 204); // Vàng nhạt
    sf::Color mTextColor = sf::Color(0, 0, 204);         // Xanh đậm
};