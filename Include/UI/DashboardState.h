#pragma once
#include "core/State.h"
#include <vector>

class DashboardState : public State {
public:
    DashboardState(App* app);

    void handleEvent(sf::Event& event) override;
    void update(sf::Time dt) override;
    void draw() override;

private:
    App* mApp;
    
    // Vùng màu trắng để hiển thị nội dung chính
    sf::RectangleShape mContentArea; 
    
    // Thanh điều hướng dưới cùng
    sf::RectangleShape mNavBar;
    std::vector<sf::Text> mNavLabels;
    std::vector<sf::CircleShape> mNavIcons;
};