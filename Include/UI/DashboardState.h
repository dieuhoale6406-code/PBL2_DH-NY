// DashboardState.h
#pragma once
#include "core/State.h"
#include "UI/App.h"
#include "UI/RoundedRectangleShape.h"
#include <vector>

class DashboardState : public State {
public:
    DashboardState(App* app);
    void handleEvent(sf::Event& event) override;
    void update(sf::Time dt) override;
    void draw() override;

private:
    void drawChartPlaceholder(); // Hàm để vẽ biểu đồ giả

    App* mApp;
    sf::RectangleShape mContentArea;
    sf::RectangleShape mTopBar;

    sf::Text mHeaderTitle;
    sf::Text mAdminLabel;

    // Các thành phần cho nút Đăng Xuất
    sf::RoundedRectangleShape mLogoutButtonBg;
    sf::Text mLogoutButtonText;

    // Thanh điều hướng dưới cùng
    std::vector<sf::RoundedRectangleShape> mNavButtons;
    std::vector<sf::Text> mNavLabels;
};