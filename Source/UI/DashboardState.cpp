#include "UI/DashboardState.h"
#include "UI/App.h"
#include <iostream>

DashboardState::DashboardState(App* app) : mApp(app) {
    // Vùng nội dung
    mContentArea.setFillColor(sf::Color::White);
    mContentArea.setOutlineThickness(1.f);
    mContentArea.setOutlineColor(sf::Color(220, 220, 220));

    // Thanh điều hướng
    mNavBar.setFillColor(sf::Color(4, 28, 55)); // Màu xanh navy đậm

    // Các nhãn cho thanh điều hướng
    const std::vector<std::string> labels = {
        u8"Kho Hàng", u8"Phiếu Nhập", u8"Phiếu Xuất", u8"Tồn Kho", u8"Lịch Sử"
    };
    
    for (const auto& label : labels) {
        sf::Text text;
        text.setFont(mApp->getFont());
        text.setString(label.c_str());
        text.setCharacterSize(16);
        text.setFillColor(sf::Color(210, 210, 210));
        mNavLabels.push_back(text);

        // Tạo icon placeholder
        sf::CircleShape icon(16.f);
        icon.setOrigin(16.f, 16.f);
        icon.setFillColor(sf::Color::Transparent);
        icon.setOutlineThickness(2.5f);
        icon.setOutlineColor(sf::Color(220, 220, 220));
        mNavIcons.push_back(icon);
    }
}

void DashboardState::handleEvent(sf::Event& event) {
    // Logic xử lý click vào các nút điều hướng sẽ được thêm ở đây
}

void DashboardState::update(sf::Time dt) {
    // Logic cập nhật (nếu có)
}

void DashboardState::draw() {
    sf::RenderWindow& window = mApp->getWindow();
    float winX = static_cast<float>(window.getSize().x);
    float winY = static_cast<float>(window.getSize().y);

    // 1. Vẽ thanh điều hướng dưới cùng
    float navHeight = 90.f;
    mNavBar.setSize({winX - 80.f, navHeight});
    mNavBar.setPosition(40.f, winY - navHeight - 20.f);
    window.draw(mNavBar);

    // 2. Vẽ các icon và nhãn trên thanh điều hướng
    int navCount = mNavLabels.size();
    float cellWidth = mNavBar.getSize().x / navCount;

    for (int i = 0; i < navCount; ++i) {
        float cellCenterX = mNavBar.getPosition().x + i * cellWidth + cellWidth * 0.5f;
        float cellCenterY = mNavBar.getPosition().y + navHeight * 0.55f;

        // Căn vị trí icon
        mNavIcons[i].setPosition(cellCenterX, cellCenterY);
        window.draw(mNavIcons[i]);

        // Căn vị trí nhãn chữ
        sf::FloatRect textRect = mNavLabels[i].getLocalBounds();
        mNavLabels[i].setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
        mNavLabels[i].setPosition(cellCenterX, cellCenterY + 25.f);
        window.draw(mNavLabels[i]);
    }
    
    // 3. Vẽ vùng nội dung màu trắng
    mContentArea.setSize({winX - 40.f, winY - navHeight - 40.f - 20.f}); // 20f là khoảng cách trên, 20f dưới
    mContentArea.setPosition(20.f, 20.f);
    window.draw(mContentArea);
}