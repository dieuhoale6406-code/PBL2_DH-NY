// Source/UI/LoginState.cpp
#include "UI/LoginState.h"
#include "UI/RoleSelectionState.h"
#include "UI/DashboardState.h"
#include "UI/App.h"
#include <iostream>

LoginState::LoginState(App* app, Role role) : mApp(app), mSelectedRole(role) {
    sf::Font& font = mApp->getFont();
    
    // 1. Tiêu đề
    mTitle.setFont(font);
    mTitle.setString(L"ĐĂNG NHẬP");
    mTitle.setCharacterSize(40);
    mTitle.setFillColor(mTextColor);
    mTitle.setStyle(sf::Text::Bold);

    // 2. Form nền trắng
    mFormBg.setSize({600, 400});
    mFormBg.setCornersRadius(20.f);
    mFormBg.setFillColor(sf::Color::White);
    mFormBg.setOutlineThickness(1.f);
    mFormBg.setOutlineColor(sf::Color(220, 220, 220));

    // 3. Các ô nhập liệu
    mUsernameBox = TextBox({500, 60}, font);
    mPasswordBox = TextBox({500, 60}, font);
    mUsernameBox.setPlaceholder("Username");
    mPasswordBox.setPlaceholder("Password");
    mPasswordBox.setMasked(true); // Che mật khẩu bằng dấu *

    // Mặc định chọn ô username lúc bắt đầu
    mUsernameBox.setSelected(true);
    mPasswordBox.setSelected(false);
    
    // 4. Nút "Xác Nhận"
    mConfirmButtonBg.setSize({250, 70});
    mConfirmButtonBg.setCornersRadius(15.f);
    mConfirmButtonBg.setFillColor(mButtonBgColor);
    mConfirmButtonText.setFont(font);
    mConfirmButtonText.setString(L"Xác Nhận");
    mConfirmButtonText.setCharacterSize(24);
    mConfirmButtonText.setFillColor(mTextColor);
    mConfirmButtonText.setStyle(sf::Text::Bold);

    // 5. Nút "Quay Lại"
    mBackButtonBg.setSize({150, 40});
    mBackButtonBg.setCornersRadius(10.f);
    mBackButtonBg.setFillColor(mButtonBgColor);
    mBackButtonText.setFont(font);
    mBackButtonText.setString(L"Quay Lại");
    mBackButtonText.setCharacterSize(16);
    mBackButtonText.setFillColor(mTextColor);
}

void LoginState::handleEvent(sf::Event& event) {
    // Xử lý click chuột
    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f mousePos = {static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y)};

            if (mUsernameBox.getBounds().contains(mousePos)) {
                mActiveBox = 0;
                mUsernameBox.setSelected(true);
                mPasswordBox.setSelected(false);
            } else if (mPasswordBox.getBounds().contains(mousePos)) {
                mActiveBox = 1;
                mUsernameBox.setSelected(false);
                mPasswordBox.setSelected(true);
            } else if (mConfirmButtonBg.getGlobalBounds().contains(mousePos)) {
                submitLogin();
            } else if (mBackButtonBg.getGlobalBounds().contains(mousePos)) {
                mApp->changeState(new RoleSelectionState(mApp));
            } else {
                // Click ra ngoài, bỏ chọn cả hai
                mUsernameBox.setSelected(false);
                mPasswordBox.setSelected(false);
            }
        }
    }

    // Xử lý sự kiện bàn phím (TAB và ENTER)
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Tab) {
            mActiveBox = 1 - mActiveBox; // Chuyển đổi giữa 0 và 1
            if (mActiveBox == 0) {
                mUsernameBox.setSelected(true);
                mPasswordBox.setSelected(false);
            } else {
                mUsernameBox.setSelected(false);
                mPasswordBox.setSelected(true);
            }
        } else if (event.key.code == sf::Keyboard::Enter) {
            submitLogin();
        }
    }
    
    // Chỉ gửi sự kiện gõ chữ cho ô đang được chọn
    if (mActiveBox == 0) {
        mUsernameBox.handleEvent(event);
    } else {
        mPasswordBox.handleEvent(event);
    }
}

void LoginState::submitLogin() {
    std::cout << "Attempting login..." << std::endl;
    std::cout << "Username: " << mUsernameBox.getText() << std::endl;
    // std::cout << "Password: " << mPasswordBox.getText() << std::endl; // Không nên in password ra console

    // Tạm thời giả định đăng nhập Admin luôn thành công để test
    if (mSelectedRole == Role::ADMIN && mUsernameBox.getText() == "admin" && mPasswordBox.getText() == "123") {
        mApp->changeState(new DashboardState(mApp));
    } else {
        // Có thể thêm thông báo lỗi ở đây
        std::cout << "Login failed or function for Staff is not implemented." << std::endl;
    }
}

void LoginState::update(sf::Time dt) {
    // Không cần update hiệu ứng hover cho các nút tự vẽ,
    // nhưng nếu bạn muốn có thể thêm logic ở đây.
}

void LoginState::draw() {
    sf::RenderWindow& window = mApp->getWindow();
    float winX = static_cast<float>(window.getSize().x);
    float winY = static_cast<float>(window.getSize().y);

    window.clear(mPurpleBgColor);

    // Căn chỉnh và vẽ Title
    sf::FloatRect textRect = mTitle.getLocalBounds();
    mTitle.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    mTitle.setPosition(winX / 2.f, winY * 0.15f);
    window.draw(mTitle);

    // Căn chỉnh và vẽ Form
    mFormBg.setOrigin(mFormBg.getSize().x / 2.f, mFormBg.getSize().y / 2.f);
    mFormBg.setPosition(winX / 2.f, winY / 2.f + 50.f);
    window.draw(mFormBg);

    // Căn chỉnh vị trí các thành phần bên trong Form
    sf::Vector2f formCenter = mFormBg.getPosition();
    mUsernameBox.setPosition({formCenter.x - mUsernameBox.getSize().x / 2.f, formCenter.y - 120.f});
    mPasswordBox.setPosition({formCenter.x - mPasswordBox.getSize().x / 2.f, formCenter.y - 40.f});
    
    mConfirmButtonBg.setOrigin(mConfirmButtonBg.getSize() / 2.f);
    mConfirmButtonBg.setPosition(formCenter.x, formCenter.y + 80.f);

    mBackButtonBg.setPosition({20, 20});

    // Vẽ các ô nhập liệu
    mUsernameBox.draw(window);
    mPasswordBox.draw(window);
    
    // Vẽ nút "Xác nhận"
    window.draw(mConfirmButtonBg);
    sf::FloatRect confirmRect = mConfirmButtonText.getLocalBounds();
    mConfirmButtonText.setOrigin(confirmRect.left + confirmRect.width / 2.f, confirmRect.top + confirmRect.height / 2.f);
    mConfirmButtonText.setPosition(mConfirmButtonBg.getPosition());
    window.draw(mConfirmButtonText);

    // Vẽ nút "Quay lại"
    window.draw(mBackButtonBg);
    sf::FloatRect backRect = mBackButtonText.getLocalBounds();
    mBackButtonText.setOrigin(backRect.left + backRect.width / 2.f, backRect.top + backRect.height / 2.f);
    mBackButtonText.setPosition(mBackButtonBg.getPosition() + mBackButtonBg.getSize() / 2.f);
    window.draw(mBackButtonText);
}