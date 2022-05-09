cmake_minimum_required(VERSION 3.22.1)

project(Pong)

if(BUILD_EXE)
	add_executable(Pong)
else()
	add_library(Pong STATIC)
endif()

target_compile_features(Pong PUBLIC cxx_std_23)

add_subdirectory(src)

find_package(
    SDL2_Sandbox REQUIRED
)

target_link_libraries(
    Pong 
    PRIVATE
    SDL2_Sandbox::SDL2_Sandbox
)

include(GNUInstallDirs)
target_include_directories(
    Pong
    PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_CURRENT_INCLUDEDIR}/Pong>
)

install(
    TARGETS Pong
    EXPORT PongTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_DIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    EXPORT PongTargets
    FILE PongTargets.cmake
    NAMESPACE Pong::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Pong
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/PongConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Pong
)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/PongConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Pong
)