#include "UI/Button.h"

Button::Button() {}

Button::Button(const std::string& text, sf::Vector2f size, sf::Font& font) {
    mBaseColor = sf::Color(255, 224, 130);
    mHoverColor = sf::Color(255, 255, 150);

    mShape.setSize(size);
    mShape.setFillColor(mBaseColor);

    mText.setFont(font);
    mText.setString(text.c_str());
    mText.setCharacterSize(30);
    mText.setFillColor(sf::Color::Blue);
    mText.setStyle(sf::Text::Bold);
}

void Button::setPosition(sf::Vector2f position) {
    mShape.setPosition(position);

    sf::FloatRect textRect = mText.getLocalBounds();
    mText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    mText.setPosition(position.x + mShape.getSize().x / 2.0f, position.y + mShape.getSize().y / 2.0f);
}

void Button::update(sf::Vector2i mousePosition) {
    if (mShape.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePosition))) {
        mShape.setFillColor(mHoverColor);
    } else {
        mShape.setFillColor(mBaseColor);
    }
}

void Button::draw(sf::RenderWindow& window) {
    window.draw(mShape);
    window.draw(mText);
}

bool Button::isClicked(sf::Vector2i mousePosition) {
    return mShape.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePosition));
}