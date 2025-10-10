// Include/UI/RoleSelectionState.h
#pragma once
#include "core/State.h"
#include "UI/App.h"
#include "UI/RoundedRectangleShape.h" // Thêm thư viện bo góc
#include "core/DataModels.h"          // Để sử dụng Role::ADMIN, Role::STAFF
#include <SFML/Graphics.hpp>          // Đảm bảo đã include

// Nếu bạn đang sử dụng lớp Button tùy chỉnh, hãy cân nhắc có cần
// chỉnh sửa lớp đó để hỗ trợ RoundedRectangleShape hoặc
// vẽ trực tiếp trong RoleSelectionState như cách dưới đây.
// #include "UI/Button.h" 

class RoleSelectionState : public State {
public:
    RoleSelectionState(App* app);
    void handleEvent(sf::Event& event) override;
    void update(sf::Time dt) override;
    void draw() override;

private:
    App* mApp;

    // Tiêu đề "VUI LÒNG CHỌN QUYỀN"
    sf::RoundedRectangleShape mTitleBackground; // Nền hồng của tiêu đề
    sf::Text mTitle;

    // Nút ADMIN
    sf::RoundedRectangleShape mAdminButtonBg; // Nền vàng của nút
    sf::Text mAdminButtonText;
    sf::CircleShape mAdminIcon; // Icon người có dấu tích

    // Nút STAFF
    sf::RoundedRectangleShape mStaffButtonBg; // Nền vàng của nút
    sf::Text mStaffButtonText;
    sf::CircleShape mStaffIconGroup; // Icon nhóm người
    sf::CircleShape mStaffIconDot; // Chấm xanh nhỏ

    // Placeholder cho font và màu sắc
    sf::Color mPurpleBgColor = sf::Color(220, 210, 250); // Màu nền tím nhạt
    sf::Color mPinkTitleBg = sf::Color(255, 204, 229);   // Màu hồng nhạt cho nền tiêu đề
    sf::Color mButtonBgColor = sf::Color(255, 255, 204); // Màu vàng nhạt cho nút
    sf::Color mTextColor = sf::Color(4, 28, 55);         // Màu xanh navy đậm cho chữ
};