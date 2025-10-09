#include "ui/Button.h"

Button::Button(const std::string& text, sf::Vector2f position, sf::Vector2f size, sf::Font& font) {
    mShape.setPosition(position);
    mShape.setSize(size);
    mShape.setFillColor(sf::Color(100, 100, 100)); // Màu xám

    mText.setFont(font);
    mText.setString(sf::String::fromUtf8(text.c_str()));
    mText.setCharacterSize(24);
    mText.setFillColor(sf::Color::White);

    // Căn chữ ra giữa nút
    sf::FloatRect textRect = mText.getLocalBounds();
    mText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    mText.setPosition(position.x + size.x / 2.0f, position.y + size.y / 2.0f);
}

void Button::draw(sf::RenderWindow& window) {
    window.draw(mShape);
    window.draw(mText);
}

bool Button::isClicked(sf::Vector2i mousePosition) {
    // Chuyển đổi tọa độ chuột thành float
    sf::Vector2f mousePosFloat(static_cast<float>(mousePosition.x), static_cast<float>(mousePosition.y));
    return mShape.getGlobalBounds().contains(mousePosFloat);
}