#include "UI/App.h"

App::App()
    : mWindow(sf::VideoMode::getDesktopMode(), "Phan Mem Quan Ly Kho", sf::Style::Fullscreen)
    , mLoginButton(u8"Đăng Nhập", {540, 300}, {200, 50}, mFont)
    , mExitButton(u8"Thoát", {540, 400}, {200, 50}, mFont){
    // 1. Tải font chữ (quan trọng!)
    if (!mFont.loadFromFile("Assets/fonts/Roboto_Condensed-Bold.ttf")) {
        std::cerr << "Error: Khong the tai font Assets/fonts/Roboto_Condensed-Bold.ttf" << std::endl;
        throw std::runtime_error("Failed to load font!");
    }

    // 2. Thiết lập đoạn text chào mừng
    // Dùng sf::String::fromUtf8 để đảm bảo hiển thị đúng tiếng Việt
    mWelcomeText.setFont(mFont);
    mWelcomeText.setString(u8"Chào mừng đến Hệ Thống Quản Lý Kho");
    mWelcomeText.setCharacterSize(30);
    mWelcomeText.setFillColor(sf::Color::White);
    // Căn giữa đoạn text
    sf::FloatRect textRect = mWelcomeText.getLocalBounds();
    mWelcomeText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    mWelcomeText.setPosition(sf::Vector2f(mWindow.getSize().x / 2.0f, mWindow.getSize().y / 2.0f));
}

void App::run() {
    sf::Clock clock;
    while (mWindow.isOpen()) {
        sf::Time deltaTime = clock.restart();
        processEvents();
        update(deltaTime);
        render();
    }
}

void App::processEvents() {
    sf::Event event;
    while (mWindow.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            mWindow.close();
        }
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) {
                mWindow.close();
            }
        }
    }
}

void App::update(sf::Time dt) {
    // Hiện tại chưa có logic gì để cập nhật
}

void App::render() {
    mWindow.clear(sf::Color(40, 42, 54)); // Xóa màn hình với màu nền tối
    mWindow.draw(mWelcomeText); // Vẽ đoạn text
    mWindow.display(); // Hiển thị những gì đã vẽ
}