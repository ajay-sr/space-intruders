cmake_minimum_required(VERSION 3.16)
project(space_intruders LANGUAGES CXX)

if(MSVC)
    # Keep the Windows ZIP portable without a separate Visual C++ runtime install.
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

option(SPACE_INTRUDERS_FETCH_SDL2 "Download and build the pinned SDL2 dependency" OFF)
if(SPACE_INTRUDERS_FETCH_SDL2)
    include(FetchContent)
    if(POLICY CMP0135)
        cmake_policy(SET CMP0135 NEW)
    endif()
    set(SDL_SHARED ON CACHE BOOL "" FORCE)
    set(SDL_STATIC OFF CACHE BOOL "" FORCE)
    set(SDL_TEST OFF CACHE BOOL "" FORCE)
    set(SDL2_DISABLE_INSTALL ON CACHE BOOL "" FORCE)
    set(SDL2_DISABLE_SDL2MAIN ON CACHE BOOL "" FORCE)
    if(MSVC)
        set(SDL_FORCE_STATIC_VCRT ON CACHE BOOL "" FORCE)
    endif()
    FetchContent_Declare(SDL2
        URL https://github.com/libsdl-org/SDL/releases/download/release-2.32.10/SDL2-2.32.10.tar.gz
        URL_HASH SHA256=5f5993c530f084535c65a6879e9b26ad441169b3e25d789d83287040a9ca5165
    )
    FetchContent_MakeAvailable(SDL2)
else()
    find_package(SDL2 REQUIRED CONFIG)
endif()

# Require the shared library even if SDL2's package offers a static fallback.
get_target_property(SDL2_LIBRARY_TYPE SDL2::SDL2 TYPE)
if(NOT SDL2_LIBRARY_TYPE STREQUAL "SHARED_LIBRARY")
    message(FATAL_ERROR "Install a shared SDL2 library; this game links SDL2 dynamically.")
endif()

add_executable(space_intruders
    src/main.cpp
    src/StateManager.cpp
    src/TitleScreen.cpp
    src/GameScreen.cpp
    src/PlayerShip.cpp
    src/Enemy.cpp
    src/PixelText.cpp
)
target_compile_features(space_intruders PRIVATE cxx_std_17)
set_target_properties(space_intruders PROPERTIES CXX_EXTENSIONS OFF)
target_compile_definitions(space_intruders PRIVATE SDL_MAIN_HANDLED)
target_link_libraries(space_intruders PRIVATE SDL2::SDL2)

include(CTest)
if(BUILD_TESTING)
    add_executable(game_screen_tests
        tests/GameScreenTests.cpp
        src/StateManager.cpp
        src/TitleScreen.cpp
        src/GameScreen.cpp
        src/PlayerShip.cpp
        src/Enemy.cpp
        src/PixelText.cpp
    )
    target_compile_features(game_screen_tests PRIVATE cxx_std_17)
    target_compile_definitions(game_screen_tests PRIVATE SDL_MAIN_HANDLED)
    target_include_directories(game_screen_tests PRIVATE src)
    target_link_libraries(game_screen_tests PRIVATE SDL2::SDL2)
    add_test(NAME game_screen_tests COMMAND game_screen_tests)
    set_tests_properties(game_screen_tests PROPERTIES ENVIRONMENT "SDL_VIDEODRIVER=dummy")
endif()

if(WIN32)
    set(runtime_targets space_intruders)
    if(BUILD_TESTING)
        list(APPEND runtime_targets game_screen_tests)
    endif()
    foreach(runtime_target IN LISTS runtime_targets)
        add_custom_command(TARGET ${runtime_target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:SDL2::SDL2> $<TARGET_FILE_DIR:${runtime_target}>
            VERBATIM
        )
    endforeach()
endif()

install(TARGETS space_intruders RUNTIME DESTINATION .)
install(FILES LICENSE README.md DESTINATION .)
if(WIN32)
    install(FILES $<TARGET_FILE:SDL2::SDL2> DESTINATION .)
endif()
if(SPACE_INTRUDERS_FETCH_SDL2)
    install(FILES "${sdl2_SOURCE_DIR}/LICENSE.txt" DESTINATION . RENAME SDL2-LICENSE.txt)
endif()
