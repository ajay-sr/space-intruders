cmake_minimum_required(VERSION 3.16)
project(space_intruders LANGUAGES CXX)

find_package(SDL2 REQUIRED CONFIG)

# Require the shared library even if SDL2's package offers a static fallback.
get_target_property(SDL2_LIBRARY_TYPE SDL2::SDL2 TYPE)
if(NOT SDL2_LIBRARY_TYPE STREQUAL "SHARED_LIBRARY")
    message(FATAL_ERROR "Install a shared SDL2 library; this game links SDL2 dynamically.")
endif()

add_executable(space_intruders
    src/main.cpp
    src/StateManager.cpp
)
target_compile_features(space_intruders PRIVATE cxx_std_17)
set_target_properties(space_intruders PROPERTIES CXX_EXTENSIONS OFF)
target_compile_definitions(space_intruders PRIVATE SDL_MAIN_HANDLED)
target_link_libraries(space_intruders PRIVATE SDL2::SDL2)
