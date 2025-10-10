#include "UI/RoleSelectionState.h"
#include "UI/LoginState.h" // Cần để chuyển State
#include "UI/App.h"
#include "core/DataModels.h"

RoleSelectionState::RoleSelectionState(App* app) : mApp(app) {
    mTitle.setFont(mApp->getFont());
    mTitle.setString(u8"VUI LÒNG CHỌN QUYỀN");
    mTitle.setCharacterSize(40);
    mTitle.setFillColor(sf::Color::Blue);
    mTitle.setStyle(sf::Text::Bold);

    mAdminButton = Button("ADMIN", {300, 150}, mApp->getFont());
    mStaffButton = Button("STAFF", {300, 150}, mApp->getFont());
}

void RoleSelectionState::handleEvent(sf::Event& event) {
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i mousePos = {event.mouseButton.x, event.mouseButton.y};
        
        if (mAdminButton.isClicked(mousePos)) {
            // <<--- KHI CLICK NÚT ADMIN, CHỌN "Role::ADMIN"
            // và yêu cầu App chuyển sang màn hình LoginState,
            // mang theo giá trị Role::ADMIN
            mApp->changeState(new LoginState(mApp, Role::ADMIN)); 

        } else if (mStaffButton.isClicked(mousePos)) {
            // <<--- TƯƠNG TỰ, KHI CLICK NÚT STAFF, CHỌN "Role::STAFF"
            mApp->changeState(new LoginState(mApp, Role::STAFF));
        }
    }
}

void RoleSelectionState::update(sf::Time dt) {
    sf::Vector2i mousePos = sf::Mouse::getPosition(mApp->getWindow());
    mAdminButton.update(mousePos);
    mStaffButton.update(mousePos);
}

void RoleSelectionState::draw() {
    sf::RenderWindow& window = mApp->getWindow();
    float winX = static_cast<float>(window.getSize().x);
    float winY = static_cast<float>(window.getSize().y);

    sf::FloatRect textRect = mTitle.getLocalBounds();
    mTitle.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    mTitle.setPosition(winX / 2.f, winY * 0.25f);

    mAdminButton.setPosition({winX / 2.f - 320.f, winY / 2.f - 75.f});
    mStaffButton.setPosition({winX / 2.f + 20.f, winY / 2.f - 75.f});

    window.draw(mTitle);
    mAdminButton.draw(window);
    mStaffButton.draw(window);
}