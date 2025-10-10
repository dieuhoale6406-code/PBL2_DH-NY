#include "UI/TextBox.h"

static inline sf::String utf8(const std::string& s) {
    return sf::String::fromUtf8(s.begin(), s.end());
}

TextBox::TextBox() {}

TextBox::TextBox(sf::Vector2f size, sf::Font& font) {
    mShape.setSize(size);
    mShape.setFillColor(sf::Color::White);
    mShape.setOutlineThickness(2.f);
    mShape.setOutlineColor(sf::Color(180, 180, 180));

    mText.setFont(font);
    mText.setCharacterSize(24);
    mText.setFillColor(sf::Color::Black);

    mPlaceholder.setFont(font);
    mPlaceholder.setCharacterSize(24);
    mPlaceholder.setFillColor(sf::Color(150,150,150));
}

void TextBox::setSize(sf::Vector2f size) {
    mShape.setSize(size);
    // cập nhật lại vị trí text theo kích thước mới
    setPosition(mShape.getPosition());
}

sf::Vector2f TextBox::getSize() const {
    return mShape.getSize();
}

sf::FloatRect TextBox::getBounds() const {
    return mShape.getGlobalBounds();
}

void TextBox::setPosition(sf::Vector2f position) {
    mShape.setPosition(position);
    const float px = position.x + 10.f;
    const float py = position.y + (mShape.getSize().y / 2.f) - 15.f; // căn giữa thô
    mText.setPosition(px, py);
    mPlaceholder.setPosition(px, py);
}

void TextBox::setPlaceholder(const std::string& text, sf::Color color) {
    mPlaceholder.setString(utf8(text));
    mPlaceholder.setFillColor(color);
}

void TextBox::setMasked(bool masked) {
    mMasked = masked;
}

void TextBox::handleEvent(sf::Event& event) {
    if (!mIsSelected || event.type != sf::Event::TextEntered) return;

    // Nhập ASCII cơ bản (đủ dùng cho username/password)
    if (event.text.unicode < 128) {
        if (event.text.unicode == 8) { // Backspace
            if (!mInputString.empty()) mInputString.pop_back();
        } else if (event.text.unicode >= 32 && event.text.unicode != 127) {
            mInputString += static_cast<char>(event.text.unicode);
        }

        // Cập nhật chuỗi hiển thị (mask nếu là password)
        if (mMasked) {
            mText.setString(std::string(mInputString.size(), '*'));
        } else {
            mText.setString(mInputString);
        }
    }
}

void TextBox::draw(sf::RenderWindow& window) {
    window.draw(mShape);
    if (mInputString.empty()) {
        window.draw(mPlaceholder);
    } else {
        window.draw(mText);
    }
}

void TextBox::setSelected(bool selected) {
    mIsSelected = selected;
    mShape.setOutlineColor(mIsSelected ? sf::Color(66, 133, 244)   // xanh focus
                                       : sf::Color(180, 180, 180)); // xám bình thường
}

std::string TextBox::getText() const {
    return mInputString;
}

bool TextBox::isClicked(sf::Vector2i mousePosition) {
    sf::Vector2f p(static_cast<float>(mousePosition.x), static_cast<float>(mousePosition.y));
    return mShape.getGlobalBounds().contains(p);
}
