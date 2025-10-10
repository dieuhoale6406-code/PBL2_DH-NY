#include "UI/App.h"
#include "UI/RoleSelectionState.h"
#include <iostream>

App::App()
    : mWindow(sf::VideoMode::getDesktopMode(), "Quan Ly Kho", sf::Style::Fullscreen)
{
    mWindow.setFramerateLimit(60);
    if (!mFont.loadFromFile("Assets/fonts/Roboto_Condensed-Bold.ttf")) {
        throw std::runtime_error("Loi: Khong the tai font!");
    }
    pushState(new RoleSelectionState(this));
}

App::~App() {
    while (!mStates.empty()) {
        popState();
    }
}

void App::run() {
    sf::Clock clock;
    while (mWindow.isOpen()) {
        sf::Time deltaTime = clock.restart();
        handleEvents();
        update(deltaTime);
        draw();
    }
}

void App::handleEvents() {
    sf::Event event;
    while (mWindow.pollEvent(event)) {
        if (event.type == sf::Event::Closed || (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)) {
            mWindow.close();
        }
        if (!mStates.empty()) {
            mStates.top()->handleEvent(event);
        }
    }
}

void App::update(sf::Time dt) {
    if (!mStates.empty()) {
        mStates.top()->update(dt);
    }
}

void App::draw() {
    mWindow.clear(sf::Color(204, 204, 255)); // Màu nền tím nhạt
    if (!mStates.empty()) {
        mStates.top()->draw();
    }
    mWindow.display();
}

void App::pushState(State* state) { mStates.push(state); }

void App::popState() {
    if (!mStates.empty()) {
        delete mStates.top();
        mStates.pop();
    }
}

void App::changeState(State* state) {
    popState();
    pushState(state);
}

sf::RenderWindow& App::getWindow() { return mWindow; }
sf::Font& App::getFont() { return mFont; }