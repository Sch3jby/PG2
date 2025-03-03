#include "app.hpp"
#include <iostream>

int main() {
    try {
        App app;

        if (!app.init()) {
            std::cerr << "Application initialization failed" << std::endl;
            return -1;
        }

        app.run();

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return -1;
    }
}