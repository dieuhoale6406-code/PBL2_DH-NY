#include "Application.h"
#include "UIManager.h"
#include "EventHandler.h"

Application::Application()
    : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Warehouse Dashboard Refactored") 
{
    window.setFramerateLimit(60);
    
    // Tải dữ liệu trước
    dataManager.loadAllData();
    
    // Khởi tạo các thành phần quản lý
    uiManager = std::make_unique<UIManager>(*this);
    eventHandler = std::make_unique<EventHandler>(*this);
}

void Application::run() {
    while (window.isOpen()) {
        // 1. Xử lý sự kiện
        eventHandler->processEvents();
        
        // 2. Cập nhật logic (nếu có, ví dụ animation)
        
        // 3. Vẽ lại giao diện
        window.clear(sf::Color(245, 230, 140));
        uiManager->draw();
        window.display();
    }
}