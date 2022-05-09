cmake_minimum_required(VERSION 3.22.1)

target_sources(
    Pong
    PRIVATE
    Pong.cpp
    Pong.h
    UiManager.h
    UiManager.cpp
    main.cpp
)

install(
    FILES
    Pong.h
    UiManager.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Pong
)