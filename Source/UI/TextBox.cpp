#include "UI/TextBox.h"

TextBox::TextBox() {}

TextBox::TextBox(sf::Vector2f size, sf::Font& font) {
    mShape.setSize(size);
    mShape.setFillColor(sf::Color::White);
    mShape.setOutlineThickness(2.f);
    mShape.setOutlineColor(sf::Color(180, 180, 180));

    mText.setFont(font);
    mText.setCharacterSize(24);
    mText.setFillColor(sf::Color::Black);
}

void TextBox::setPosition(sf::Vector2f position) {
    mShape.setPosition(position);
    mText.setPosition(position.x + 10.f, position.y + (mShape.getSize().y / 2.f) - 15.f);
}

void TextBox::handleEvent(sf::Event& event) {
    if (!mIsSelected || event.type != sf::Event::TextEntered) return;
    if (event.text.unicode < 128) {
        if (event.text.unicode == 8 && !mInputString.empty()) { // Backspace
            mInputString.pop_back();
        } else if (event.text.unicode >= 32) {
            mInputString += static_cast<char>(event.text.unicode);
        }
        mText.setString(mInputString);
    }
}

void TextBox::draw(sf::RenderWindow& window) {
    window.draw(mShape);
    window.draw(mText);
}

void TextBox::setSelected(bool selected) {
    mIsSelected = selected;
    mShape.setOutlineColor(mIsSelected ? sf::Color::Blue : sf::Color(180, 180, 180));
}

std::string TextBox::getText() const {
    return mInputString;
}

bool TextBox::isClicked(sf::Vector2i mousePosition) {
    sf::Vector2f mousePosFloat(static_cast<float>(mousePosition.x), static_cast<float>(mousePosition.y));
    return mShape.getGlobalBounds().contains(mousePosFloat);
}