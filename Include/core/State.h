#pragma once
#include <SFML/Graphics.hpp>

class App; // Khai báo trước để tránh lỗi include vòng lặp

class State {
public:
    virtual ~State() = default;

    virtual void handleEvent(sf::Event& event) = 0;
    virtual void update(sf::Time dt) = 0;
    virtual void draw() = 0;
};