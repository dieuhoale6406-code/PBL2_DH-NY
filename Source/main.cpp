#include "UI/App.h"
#include <iostream>

int main() {
    try {
        App myApp;
        myApp.run();
    } catch (const std::exception& e) {
        std::cerr << "Da xay ra loi: " << e.what() << std::endl;
        std::cout << "Nhan Enter de thoat...";
        std::cin.get();
        return 1;
    }
    return 0;
}