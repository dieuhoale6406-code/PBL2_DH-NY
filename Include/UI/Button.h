#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class Button {
public:
    Button(const std::string& text, sf::Vector2f position, sf::Vector2f size, sf::Font& font);

    void draw(sf::RenderWindow& window);
    bool isClicked(sf::Vector2i mousePosition);

private:
    sf::RectangleShape mShape;
    sf::Text mText;
};