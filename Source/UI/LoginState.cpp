#include "UI/LoginState.h"
#include "UI/RoleSelectionState.h"
#include "UI/DashboardState.h"
#include "UI/App.h"
#include <iostream>

LoginState::LoginState(App* app, Role role) : mApp(app), mSelectedRole(role) {
    mTitle.setFont(mApp->getFont());
    mTitle.setString(u8"ĐĂNG NHẬP");
    mTitle.setCharacterSize(40);
    mTitle.setFillColor(sf::Color::Blue);
    mTitle.setStyle(sf::Text::Bold);

    mUsernameBox = TextBox({500, 60}, mApp->getFont());
    mPasswordBox = TextBox({500, 60}, mApp->getFont());
    mConfirmButton = Button(u8"Xác Nhận", {250, 70}, mApp->getFont());
    mBackButton = Button(u8"Quay Lại", {150, 40}, mApp->getFont());
}

void LoginState::handleEvent(sf::Event& event) {
    // Chỉ xử lý sự kiện click chuột
    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2i mousePos = {event.mouseButton.x, event.mouseButton.y};

            // Kiểm tra xem có click vào các ô textbox không
            mUsernameBox.setSelected(mUsernameBox.isClicked(mousePos));
            mPasswordBox.setSelected(mPasswordBox.isClicked(mousePos));

            // Kiểm tra click nút "Xác Nhận"
            if (mConfirmButton.isClicked(mousePos)) {
                std::cout << "Username: " << mUsernameBox.getText() << std::endl;
                
                // Tạm thời giả định đăng nhập Admin luôn thành công để test
                if (mSelectedRole == Role::ADMIN) {
                    mApp->changeState(new DashboardState(mApp));
                } else {
                    std::cout << "Chuc nang cho Staff se duoc them sau." << std::endl;
                }
            } 
            // Kiểm tra click nút "Quay Lại"
            else if (mBackButton.isClicked(mousePos)) {
                mApp->changeState(new RoleSelectionState(mApp));
            }
        }
    }

    // Luôn gửi sự kiện gõ phím cho các textbox để chúng có thể nhận chữ
    mUsernameBox.handleEvent(event);
    mPasswordBox.handleEvent(event);
}

void LoginState::update(sf::Time dt) {
    // Cập nhật hiệu ứng hover cho các nút
    sf::Vector2i mousePos = sf::Mouse::getPosition(mApp->getWindow());
    mConfirmButton.update(mousePos);
    mBackButton.update(mousePos);
}

void LoginState::draw() {
    sf::RenderWindow& window = mApp->getWindow();
    float winX = static_cast<float>(window.getSize().x);
    float winY = static_cast<float>(window.getSize().y);

    // Căn chỉnh và vẽ Title
    sf::FloatRect textRect = mTitle.getLocalBounds();
    mTitle.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    mTitle.setPosition(winX / 2.f, winY * 0.2f);

    // Vẽ nền cho form đăng nhập
    sf::RectangleShape formBg({600, 400});
    formBg.setFillColor(sf::Color::White);
    formBg.setOrigin(formBg.getSize().x / 2.f, formBg.getSize().y / 2.f);
    formBg.setPosition(winX / 2.f, winY / 2.f + 50.f);

    // Căn chỉnh vị trí các thành phần
    mUsernameBox.setPosition({winX / 2.f - 250.f, winY * 0.4f});
    mPasswordBox.setPosition({winX / 2.f - 250.f, winY * 0.5f});
    mConfirmButton.setPosition({winX / 2.f - 125.f, winY * 0.65f});
    mBackButton.setPosition({10, 10});

    // Vẽ mọi thứ lên cửa sổ
    window.draw(mTitle);
    window.draw(formBg);
    mUsernameBox.draw(window);
    mPasswordBox.draw(window);
    mConfirmButton.draw(window);
    mBackButton.draw(window);
}