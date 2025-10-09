#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include "Globals.h"
#include "DataManager.h"
#include "UIManager.h"
#include "EventHandler.h"
#include "Account.h"

class UIManager;
class EventHandler;

class Application {
public:
    Application();
    void run();

    // === Quản lý trạng thái ===
    sf::RenderWindow window;
    View currentView = View::Login;
    Tab currentTab = Tab::Kho;
    Account* loggedInUser = nullptr;
    // ... các biến trạng thái khác (searchText, scrollOffset, selWhId, etc.)

    // === Components ===
    DataManager dataManager;
    std::unique_ptr<UIManager> uiManager;
    std::unique_ptr<EventHandler> eventHandler;
};