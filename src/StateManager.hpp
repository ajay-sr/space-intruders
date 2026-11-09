#pragma once

#include "Screen.hpp"

#include <SDL.h>
#include <memory>

enum class ScreenId {
    Title,
    Game,
};

class StateManager {
public:
    StateManager();

    void setScreen(ScreenId screen);
    void handleEvent(const SDL_Event& event);
    void update(double deltaSeconds);
    void render(SDL_Renderer* renderer) const;

private:
    ScreenId currentScreen_ = ScreenId::Title;
    std::unique_ptr<Screen> screen_;
};
