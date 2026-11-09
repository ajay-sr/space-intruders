#include "StateManager.hpp"

#include "GameScreen.hpp"
#include "TitleScreen.hpp"

StateManager::StateManager() {
    setScreen(ScreenId::Title);
}

void StateManager::setScreen(ScreenId screen) {
    switch (screen) {
    case ScreenId::Title:
        screen_ = std::make_unique<TitleScreen>();
        break;
    case ScreenId::Game:
        screen_ = std::make_unique<GameScreen>();
        break;
    }
    currentScreen_ = screen;
}

void StateManager::handleEvent(const SDL_Event& event) {
    if (event.type == SDL_KEYDOWN && !event.key.repeat) {
        if (currentScreen_ == ScreenId::Title) {
            setScreen(ScreenId::Game);
            return;
        }
        if (event.key.keysym.sym == SDLK_ESCAPE) {
            setScreen(ScreenId::Title);
            return;
        }
    }
    screen_->handleEvent(event);
}

void StateManager::update(double deltaSeconds) {
    screen_->update(deltaSeconds);
}

void StateManager::render(SDL_Renderer* renderer) const {
    screen_->render(renderer);
}
