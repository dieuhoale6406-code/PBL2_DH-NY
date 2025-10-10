#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class TextBox {
public:
    TextBox();
    TextBox(sf::Vector2f size, sf::Font& font);

    // Layout
    void setPosition(sf::Vector2f position);
    void setSize(sf::Vector2f size);
    sf::Vector2f getSize() const;
    sf::FloatRect getBounds() const;

    // State
    void setSelected(bool selected);
    bool isSelected() const { return mIsSelected; }

    // Features cần cho LoginState
    void setPlaceholder(const std::string& text,
                        sf::Color color = sf::Color(150,150,150));
    void setMasked(bool masked);

    // I/O
    void handleEvent(sf::Event& event);
    void draw(sf::RenderWindow& window);
    std::string getText() const;

    // (đang dùng ở nơi khác)
    bool isClicked(sf::Vector2i mousePosition);

private:
    sf::RectangleShape mShape;
    sf::Text  mText;            // nội dung nhập
    sf::Text  mPlaceholder;     // placeholder khi trống
    bool mIsSelected = false;
    bool mMasked     = false;
    std::string mInputString;
};
