#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>
#include "ui/Button.h"

class App {
public:
    App();
    void run();

private:
    void processEvents();
    void update(sf::Time dt);
    void render();

    sf::RenderWindow mWindow;
    sf::Font mFont;
    sf::Text mWelcomeText;

    Button mLoginButton;
    Button mExitButton;
};