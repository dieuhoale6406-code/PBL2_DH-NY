#pragma once
#include "core/State.h"
#include "core/DataModels.h"
#include "UI/Button.h"
#include "UI/TextBox.h"

class LoginState : public State {
public:
    LoginState(App* app, Role role);
    void handleEvent(sf::Event& event) override;
    void update(sf::Time dt) override;
    void draw() override;
private:
    App* mApp;
    Role mSelectedRole;
    sf::Text mTitle;
    TextBox mUsernameBox;
    TextBox mPasswordBox;
    Button mConfirmButton;
    Button mBackButton;
};