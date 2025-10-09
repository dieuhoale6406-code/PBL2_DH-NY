#include "ui/App.h" // Include file header của lớp App
#include <iostream>

int main()
{
    try {
        App myApp;   // Tạo một đối tượng của lớp App
        myApp.run(); // Chạy vòng lặp chính của ứng dụng
    }
    catch (const std::exception& e) {
        std::cerr << "Mot loi da xay ra: " << e.what() << std::endl;
        return 1; // Thoát với mã lỗi
    }

    return 0; // Thoát thành công
}