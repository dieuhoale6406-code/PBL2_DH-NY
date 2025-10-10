#pragma once
#include <SFML/Graphics.hpp>
#include <stack>
#include "core/State.h"

class App {
public:
    App();
    ~App();
    void run();

    void pushState(State* state);
    void popState();
    void changeState(State* state);

    sf::RenderWindow& getWindow();
    sf::Font& getFont();

private:
    void handleEvents();
    void update(sf::Time dt);
    void draw();

    sf::RenderWindow mWindow;
    sf::Font mFont;
    std::stack<State*> mStates;
};