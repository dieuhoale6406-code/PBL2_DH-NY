#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class TextBox {
public:
    TextBox();
    TextBox(sf::Vector2f size, sf::Font& font);

    void setPosition(sf::Vector2f position);
    void handleEvent(sf::Event& event);
    void draw(sf::RenderWindow& window);
    void setSelected(bool selected);
    std::string getText() const;
    bool isClicked(sf::Vector2i mousePosition);

private:
    sf::RectangleShape mShape;
    sf::Text mText;
    std::string mInputString;
    bool mIsSelected = false;
};