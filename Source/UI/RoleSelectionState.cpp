// Source/UI/RoleSelectionState.cpp
#include "UI/RoleSelectionState.h"
#include "UI/LoginState.h" 
#include "UI/App.h"
#include "core/DataModels.h" // Đảm bảo Role::ADMIN, Role::STAFF được định nghĩa ở đây

RoleSelectionState::RoleSelectionState(App* app) : mApp(app) {
    sf::Font& font = mApp->getFont(); // Lấy font từ App

    // 1. Tiêu đề "VUI LÒNG CHỌN QUYỀN"
    mTitleBackground.setCornersRadius(10.f);
    mTitleBackground.setFillColor(mPinkTitleBg); // Màu hồng nhạt
    
    mTitle.setFont(font);
    mTitle.setString(L"VUI LÒNG CHỌN QUYỀN"); // Sử dụng L"" cho UTF-8
    mTitle.setCharacterSize(36);
    mTitle.setFillColor(mTextColor); // Màu xanh navy đậm
    mTitle.setStyle(sf::Text::Bold);

    // 2. Nút ADMIN
    mAdminButtonBg.setSize({300, 150});
    mAdminButtonBg.setCornersRadius(15.f);
    mAdminButtonBg.setFillColor(mButtonBgColor); // Màu vàng nhạt

    mAdminButtonText.setFont(font);
    mAdminButtonText.setString("ADMIN");
    mAdminButtonText.setCharacterSize(30);
    mAdminButtonText.setFillColor(mTextColor); // Màu xanh navy đậm
    mAdminButtonText.setStyle(sf::Text::Bold);

    // Icon ADMIN (placeholder: vòng tròn với chấm check)
    mAdminIcon.setRadius(20.f);
    mAdminIcon.setFillColor(sf::Color::Transparent);
    mAdminIcon.setOutlineThickness(3.f);
    mAdminIcon.setOutlineColor(mTextColor);

    // 3. Nút STAFF
    mStaffButtonBg.setSize({300, 150});
    mStaffButtonBg.setCornersRadius(15.f);
    mStaffButtonBg.setFillColor(mButtonBgColor); // Màu vàng nhạt

    mStaffButtonText.setFont(font);
    mStaffButtonText.setString("STAFF");
    mStaffButtonText.setCharacterSize(30);
    mStaffButtonText.setFillColor(mTextColor); // Màu xanh navy đậm
    mStaffButtonText.setStyle(sf::Text::Bold);

    // Icon STAFF (placeholder: nhóm 3 vòng tròn và chấm xanh)
    mStaffIconGroup.setRadius(20.f);
    mStaffIconGroup.setFillColor(sf::Color::Transparent);
    mStaffIconGroup.setOutlineThickness(3.f);
    mStaffIconGroup.setOutlineColor(mTextColor);
    
    mStaffIconDot.setRadius(5.f);
    mStaffIconDot.setFillColor(sf::Color(66, 133, 244)); // Màu xanh Google
}

void RoleSelectionState::handleEvent(sf::Event& event) {
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f mousePos = {static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y)};
        
        if (mAdminButtonBg.getGlobalBounds().contains(mousePos)) {
            mApp->changeState(new LoginState(mApp, Role::ADMIN)); 
        } else if (mStaffButtonBg.getGlobalBounds().contains(mousePos)) {
            mApp->changeState(new LoginState(mApp, Role::STAFF));
        }
    }
}

void RoleSelectionState::update(sf::Time dt) {
    // Không cần cập nhật trạng thái nút bằng lớp Button nữa vì ta vẽ trực tiếp
    // (Nếu bạn muốn hiệu ứng hover, sẽ cần thêm logic ở đây)
}

void RoleSelectionState::draw() {
    sf::RenderWindow& window = mApp->getWindow();
    float winX = static_cast<float>(window.getSize().x);
    float winY = static_cast<float>(window.getSize().y);

    window.clear(mPurpleBgColor); // Đặt màu nền chính cho cửa sổ

    // 1. Vẽ nền và chữ cho tiêu đề
    float titleBgWidth = mTitle.getGlobalBounds().width + 80.f; // Thêm padding
    float titleBgHeight = mTitle.getGlobalBounds().height + 40.f;
    mTitleBackground.setSize({titleBgWidth, titleBgHeight});
    
    // Căn giữa tiêu đề
    sf::FloatRect titleTextRect = mTitle.getLocalBounds();
    mTitle.setOrigin(titleTextRect.left + titleTextRect.width / 2.0f, titleTextRect.top + titleTextRect.height / 2.0f);
    mTitle.setPosition(winX / 2.f, winY * 0.2f); // Vị trí cao hơn một chút

    mTitleBackground.setOrigin(mTitleBackground.getSize().x / 2.f, mTitleBackground.getSize().y / 2.f);
    mTitleBackground.setPosition(mTitle.getPosition());
    window.draw(mTitleBackground);
    window.draw(mTitle);

    // 2. Đặt vị trí và vẽ nút ADMIN
    mAdminButtonBg.setPosition({winX / 2.f - 320.f, winY / 2.f + 50.f}); // Điều chỉnh vị trí y
    window.draw(mAdminButtonBg);
    
    sf::FloatRect adminTextRect = mAdminButtonText.getLocalBounds();
    mAdminButtonText.setOrigin(adminTextRect.left + adminTextRect.width / 2.0f, adminTextRect.top + adminTextRect.height / 2.0f);
    mAdminButtonText.setPosition(mAdminButtonBg.getPosition().x + mAdminButtonBg.getSize().x / 2.f, mAdminButtonBg.getPosition().y + mAdminButtonBg.getSize().y * 0.7f);
    window.draw(mAdminButtonText);

    // Icon ADMIN
    mAdminIcon.setOrigin(mAdminIcon.getRadius(), mAdminIcon.getRadius());
    mAdminIcon.setPosition(mAdminButtonBg.getPosition().x + mAdminButtonBg.getSize().x / 2.f, mAdminButtonBg.getPosition().y + mAdminButtonBg.getSize().y * 0.35f);
    window.draw(mAdminIcon);
    
    // Icon dấu tích nhỏ (vẽ tạm bằng 2 đường thẳng)
    sf::RectangleShape checkPart1({15.f, 3.f});
    checkPart1.setFillColor(mTextColor);
    checkPart1.rotate(45);
    checkPart1.setPosition(mAdminIcon.getPosition().x - 10, mAdminIcon.getPosition().y + 5);
    window.draw(checkPart1);

    sf::RectangleShape checkPart2({25.f, 3.f});
    checkPart2.setFillColor(mTextColor);
    checkPart2.rotate(-45);
    checkPart2.setPosition(mAdminIcon.getPosition().x - 5, mAdminIcon.getPosition().y);
    window.draw(checkPart2);


    // 3. Đặt vị trí và vẽ nút STAFF
    mStaffButtonBg.setPosition({winX / 2.f + 20.f, winY / 2.f + 50.f}); // Điều chỉnh vị trí y
    window.draw(mStaffButtonBg);

    sf::FloatRect staffTextRect = mStaffButtonText.getLocalBounds();
    mStaffButtonText.setOrigin(staffTextRect.left + staffTextRect.width / 2.0f, staffTextRect.top + staffTextRect.height / 2.0f);
    mStaffButtonText.setPosition(mStaffButtonBg.getPosition().x + mStaffButtonBg.getSize().x / 2.f, mStaffButtonBg.getPosition().y + mStaffButtonBg.getSize().y * 0.7f);
    window.draw(mStaffButtonText);

    // Icon STAFF (vẽ 3 vòng tròn nhỏ giả lập nhóm người)
    // Vị trí trung tâm cho cả nhóm icon
    sf::Vector2f staffIconCenter = {mStaffButtonBg.getPosition().x + mStaffButtonBg.getSize().x / 2.f, mStaffButtonBg.getPosition().y + mStaffButtonBg.getSize().y * 0.35f};

    // Vòng tròn trung tâm
    mStaffIconGroup.setRadius(18.f); // Kích thước lớn hơn một chút
    mStaffIconGroup.setOrigin(mStaffIconGroup.getRadius(), mStaffIconGroup.getRadius());
    mStaffIconGroup.setPosition(staffIconCenter.x, staffIconCenter.y - 5); // Đẩy lên trên một chút
    window.draw(mStaffIconGroup);

    // Hai vòng tròn nhỏ hơn hai bên
    sf::CircleShape sideIcon(12.f);
    sideIcon.setFillColor(sf::Color::Transparent);
    sideIcon.setOutlineThickness(3.f);
    sideIcon.setOutlineColor(mTextColor);
    sideIcon.setOrigin(sideIcon.getRadius(), sideIcon.getRadius());

    sideIcon.setPosition(staffIconCenter.x - 28, staffIconCenter.y + 10);
    window.draw(sideIcon);
    sideIcon.setPosition(staffIconCenter.x + 28, staffIconCenter.y + 10);
    window.draw(sideIcon);
    
    // Chấm xanh nhỏ
    mStaffIconDot.setOrigin(mStaffIconDot.getRadius(), mStaffIconDot.getRadius());
    mStaffIconDot.setPosition(mStaffButtonBg.getPosition().x + mStaffButtonBg.getSize().x - 40.f, mStaffButtonBg.getPosition().y + 40.f);
    window.draw(mStaffIconDot);
}