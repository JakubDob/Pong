#include <iostream>

#include "Pong.h"

int main(int argc, char* args[]) {
    try {
        Pong::loadAssetsFromFilesystem("assets");
        Pong pong;
        pong.run();
    } catch (std::exception& e) {
        std::cerr << e.what() << '\n';
    }

    return 0;
}