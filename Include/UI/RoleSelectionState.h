#pragma once
#include "core/State.h"
#include "UI/Button.h"

class RoleSelectionState : public State {
public:
    RoleSelectionState(App* app);
    void handleEvent(sf::Event& event) override;
    void update(sf::Time dt) override;
    void draw() override;
private:
    App* mApp;
    sf::Text mTitle;
    Button mAdminButton;
    Button mStaffButton;
};