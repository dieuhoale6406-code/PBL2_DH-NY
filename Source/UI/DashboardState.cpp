// DashboardState.cpp
#include "UI/DashboardState.h"
#include "UI/App.h"
#include <iostream>
#include <vector>

// Hàm khởi tạo - Nơi chúng ta thiết lập mọi thứ
DashboardState::DashboardState(App* app) : mApp(app) {
    sf::Font& font = mApp->getFont();
    
    // 1. Thanh Header trên cùng
    mTopBar.setFillColor(sf::Color(245, 245, 245)); // Màu xám rất nhạt

    // Tiêu đề chính
    mHeaderTitle.setFont(font);
    mHeaderTitle.setString(L"HỆ THỐNG THEO DÕI NHẬP XUẤT KHO HÀNG");
    mHeaderTitle.setCharacterSize(24);
    mHeaderTitle.setFillColor(sf::Color(4, 28, 55)); // Màu xanh navy đậm
    mHeaderTitle.setStyle(sf::Text::Bold);

    // Nhãn Admin
    mAdminLabel.setFont(font);
    mAdminLabel.setString("ADMIN");
    mAdminLabel.setCharacterSize(16);
    mAdminLabel.setFillColor(sf::Color(4, 28, 55));

    // Nút Đăng Xuất
    mLogoutButtonBg.setSize({120.f, 40.f});
    mLogoutButtonBg.setCornersRadius(10.f);
    mLogoutButtonBg.setFillColor(sf::Color(255, 223, 211)); // Màu hồng cam nhạt
    mLogoutButtonText.setFont(font);
    mLogoutButtonText.setString(L"ĐĂNG XUẤT");
    mLogoutButtonText.setCharacterSize(16);
    mLogoutButtonText.setFillColor(sf::Color(211, 84, 0)); // Màu cam đậm

    // 2. Vùng nội dung chính
    mContentArea.setFillColor(sf::Color::White);
    mContentArea.setOutlineThickness(1.f);
    mContentArea.setOutlineColor(sf::Color(220, 220, 220));

    // 3. Thanh điều hướng dưới cùng
    const std::vector<std::wstring> labels = {
        L"DASHBOARD", L"QUẢN LÝ STAFF", L"QUẢN LÝ KHO", L"CHUNG", L"TÙY CHỌN"
    };
    
    for (const auto& label : labels) {
        // Nút bấm
        sf::RoundedRectangleShape button;
        button.setSize({180.f, 50.f}); // Kích thước sẽ được điều chỉnh trong hàm draw
        button.setCornersRadius(15.f);
        button.setFillColor(sf::Color(128, 118, 241)); // Màu tím
        mNavButtons.push_back(button);

        // Nhãn chữ
        sf::Text text;
        text.setFont(font);
        text.setString(label);
        text.setCharacterSize(16);
        text.setFillColor(sf::Color::White);
        mNavLabels.push_back(text);
    }
}

void DashboardState::handleEvent(sf::Event& event) {
    // Logic xử lý sự kiện
}

void DashboardState::update(sf::Time dt) {
    // Logic cập nhật
}

// Hàm vẽ - Nơi chúng ta hiển thị mọi thứ lên màn hình
void DashboardState::draw() {
    sf::RenderWindow& window = mApp->getWindow();
    float winX = static_cast<float>(window.getSize().x);
    float winY = static_cast<float>(window.getSize().y);

    // Xóa màn hình với màu nền
    window.clear(sf::Color(230, 230, 250)); // Màu tím rất nhạt

    // 1. Vẽ thanh Header
    float topBarHeight = 80.f;
    mTopBar.setSize({winX, topBarHeight});
    mTopBar.setPosition(0.f, 0.f);
    window.draw(mTopBar);

    // Căn chỉnh và vẽ các thành phần trên Header
    sf::FloatRect titleRect = mHeaderTitle.getLocalBounds();
    mHeaderTitle.setOrigin(titleRect.left + titleRect.width / 2.0f, titleRect.top + titleRect.height / 2.0f);
    mHeaderTitle.setPosition(winX / 2.0f, topBarHeight / 2.0f);
    window.draw(mHeaderTitle);

    mLogoutButtonBg.setPosition(40.f, (topBarHeight - 40.f) / 2.f);
    window.draw(mLogoutButtonBg);
    sf::FloatRect logoutTextRect = mLogoutButtonText.getLocalBounds();
    mLogoutButtonText.setOrigin(logoutTextRect.left + logoutTextRect.width / 2.0f, logoutTextRect.top + logoutTextRect.height / 2.0f);
    mLogoutButtonText.setPosition(mLogoutButtonBg.getPosition().x + mLogoutButtonBg.getSize().x / 2.f, mLogoutButtonBg.getPosition().y + mLogoutButtonBg.getSize().y / 2.f);
    window.draw(mLogoutButtonText);

    sf::FloatRect adminRect = mAdminLabel.getLocalBounds();
    mAdminLabel.setOrigin(adminRect.left + adminRect.width, 0); // Căn phải
    mAdminLabel.setPosition(winX - 40.f, (topBarHeight - adminRect.height) / 2.f - 5.f);
    window.draw(mAdminLabel);

    // 2. Vẽ vùng nội dung
    float navBarHeight = 80.f;
    float contentMargin = 20.f;
    mContentArea.setSize({winX - contentMargin * 2, winY - topBarHeight - navBarHeight - contentMargin * 3});
    mContentArea.setPosition(contentMargin, topBarHeight + contentMargin);
    window.draw(mContentArea);
    
    // Vẽ biểu đồ (placeholder)
    drawChartPlaceholder();

    // 3. Vẽ các nút điều hướng dưới cùng
    int navCount = mNavLabels.size();
    float totalNavWidth = winX - contentMargin * 2;
    float navSpacing = 20.f;
    float buttonWidth = (totalNavWidth - (navSpacing * (navCount - 1))) / navCount;

    for (int i = 0; i < navCount; ++i) {
        float buttonX = contentMargin + i * (buttonWidth + navSpacing);
        float buttonY = winY - navBarHeight - contentMargin;
        
        mNavButtons[i].setSize({buttonWidth, 50.f});
        mNavButtons[i].setPosition(buttonX, buttonY);
        window.draw(mNavButtons[i]);

        sf::FloatRect textRect = mNavLabels[i].getLocalBounds();
        mNavLabels[i].setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
        mNavLabels[i].setPosition(buttonX + buttonWidth / 2.0f, buttonY + 25.f);
        window.draw(mNavLabels[i]);
    }
}

// Hàm này chỉ vẽ một biểu đồ giả để trông giống hình ảnh
void DashboardState::drawChartPlaceholder() {
    sf::RenderWindow& window = mApp->getWindow();

    // Dữ liệu giả
    std::vector<std::pair<std::string, int>> data = {
        {"Camera", 16}, {"Desktop", 35}, {"Headphones", 3}, {"Laptop HP", 16},
        {"Mouse", 20}, {"Rgb Keyboard", 13}, {"Smart Watch", 36}, {"Speakers", 3},
        {"Tablets", 32}, {"Wireless Pr", 40}
    };
    int maxValue = 40;

    sf::FloatRect area = mContentArea.getGlobalBounds();
    float chartAreaWidth = area.width - 100;
    float chartAreaHeight = area.height - 100;
    float barWidth = chartAreaWidth / (data.size() * 1.5f);
    float barSpacing = barWidth * 0.5f;

    sf::Text chartTitle("Stock Available", mApp->getFont(), 22);
    chartTitle.setFillColor(sf::Color::Black);
    sf::FloatRect titleRect = chartTitle.getLocalBounds();
    chartTitle.setOrigin(titleRect.left + titleRect.width / 2.0f, 0);
    chartTitle.setPosition(area.left + area.width / 2.0f, area.top + 20);
    window.draw(chartTitle);

    for (size_t i = 0; i < data.size(); ++i) {
        float barHeight = (static_cast<float>(data[i].second) / maxValue) * chartAreaHeight;
        sf::RectangleShape bar({barWidth, barHeight});
        bar.setFillColor(sf::Color(22, 160, 133)); // Màu xanh teal
        
        float barX = area.left + 50 + i * (barWidth + barSpacing);
        float barY = area.top + area.height - 50 - barHeight;
        bar.setPosition(barX, barY);
        window.draw(bar);

        // Nhãn giá trị trên cột
        sf::Text valueLabel(std::to_string(data[i].second), mApp->getFont(), 14);
        valueLabel.setFillColor(sf::Color::Black);
        sf::FloatRect valueRect = valueLabel.getLocalBounds();
        valueLabel.setOrigin(valueRect.left + valueRect.width/2.f, 0);
        valueLabel.setPosition(barX + barWidth / 2.f, barY - 20);
        window.draw(valueLabel);

        // Nhãn tên sản phẩm dưới cột
        sf::Text nameLabel(data[i].first, mApp->getFont(), 12);
        nameLabel.setFillColor(sf::Color(100, 100, 100));
        sf::FloatRect nameRect = nameLabel.getLocalBounds();
        nameLabel.setOrigin(nameRect.left + nameRect.width/2.f, 0);
        nameLabel.setPosition(barX + barWidth / 2.f, area.top + area.height - 45);
        window.draw(nameLabel);
    }
}