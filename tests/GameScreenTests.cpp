#include "Enemy.hpp"
#include "GameScreen.hpp"
#include "StateManager.hpp"

#include <SDL.h>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {
void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

using Frame = std::vector<Uint32>;
constexpr Uint32 Red = 0xff5050ff;
constexpr Uint32 White = 0xffffffff;
constexpr Uint32 Black = 0x000000ff;

Frame capture(SDL_Renderer* renderer, const Screen& screen) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    screen.render(renderer);
    Frame frame(960 * 540);
    require(SDL_RenderReadPixels(renderer, nullptr, SDL_PIXELFORMAT_RGBA8888,
                                 frame.data(), 960 * sizeof(Uint32)) == 0,
            "Could not capture frame");
    return frame;
}

SDL_Event keyEvent(Uint32 type, SDL_Keycode key, bool repeat = false) {
    SDL_Event event{};
    event.type = type;
    event.key.keysym.sym = key;
    event.key.repeat = repeat;
    return event;
}

void checkPlayerX(SDL_Renderer* renderer, const GameScreen& game, int x) {
    const auto frame = capture(renderer, game);
    for (int column = 0; column < 960; ++column) {
        require(frame[492 * 960 + column] ==
                    (column >= x && column < x + 32 ? White : Black),
                "Player horizontal movement mismatch");
    }
}

void runCombatTests(SDL_Renderer* renderer) {
    GameScreen movement;
    movement.handleEvent(keyEvent(SDL_KEYDOWN, SDLK_UP));
    require(!movement.getBullet(), "Fired during the level intro");
    movement.update(1.0);
    movement.handleEvent(keyEvent(SDL_KEYDOWN, SDLK_LEFT));
    movement.update(0.5);
    checkPlayerX(renderer, movement, 304);
    movement.handleEvent(keyEvent(SDL_KEYUP, SDLK_LEFT));
    movement.update(0.25);
    checkPlayerX(renderer, movement, 304);
    movement.handleEvent(keyEvent(SDL_KEYDOWN, SDLK_RIGHT));
    movement.update(0.5);
    checkPlayerX(renderer, movement, 464);
    movement.handleEvent(keyEvent(SDL_KEYDOWN, SDLK_LEFT));
    movement.update(0.5);
    checkPlayerX(renderer, movement, 464);
    movement.handleEvent(keyEvent(SDL_KEYUP, SDLK_RIGHT));
    movement.update(10.0);
    checkPlayerX(renderer, movement, 0);
    movement.handleEvent(keyEvent(SDL_KEYUP, SDLK_LEFT));
    movement.handleEvent(keyEvent(SDL_KEYDOWN, SDLK_RIGHT));
    movement.update(10.0);
    checkPlayerX(renderer, movement, 928);
    SDL_Event focusLost{};
    focusLost.type = SDL_WINDOWEVENT;
    focusLost.window.event = SDL_WINDOWEVENT_FOCUS_LOST;
    movement.handleEvent(focusLost);
    movement.handleEvent(keyEvent(SDL_KEYDOWN, SDLK_LEFT));
    movement.handleEvent(focusLost);
    movement.update(0.1);
    checkPlayerX(renderer, movement, 928);

    GameScreen shooting;
    shooting.update(1.0);
    shooting.handleEvent(keyEvent(SDL_KEYDOWN, SDLK_UP));
    require(shooting.getBullet().has_value(), "Up arrow did not fire");
    require(shooting.getBullet()->x == 478.0 && shooting.getBullet()->y == 464.0,
            "Bullet did not spawn above the player's center");
    require(capture(renderer, shooting)[464 * 960 + 478] == White,
            "Bullet was not rendered");
    shooting.update(0.1);
    const auto firstBullet = *shooting.getBullet();
    require(firstBullet.y < 417.0 && firstBullet.y > 415.0,
            "Bullet did not move upward");
    shooting.handleEvent(keyEvent(SDL_KEYDOWN, SDLK_UP));
    require(shooting.getBullet()->y == firstBullet.y,
            "Firing replaced an active bullet");
    shooting.update(0.5);
    require(!shooting.getBullet(), "Bullet survived an enemy hit");
    require(shooting.isEnemyDead(3, 7), "Bullet did not kill the nearest enemy");
    std::size_t kills = 0;
    for (std::size_t row = 0; row < GameScreen::Rows; ++row) {
        for (std::size_t column = 0; column < GameScreen::Columns; ++column) {
            kills += shooting.isEnemyDead(row, column);
        }
    }
    require(kills == 1, "One bullet killed multiple enemies");
    const auto killedPosition = shooting.getEnemyGrid()[3][7].getCoordinates();
    shooting.update(0.1);
    require(shooting.getEnemyGrid()[3][7].getCoordinates().x == killedPosition.x,
            "Shot enemy continued moving");
    shooting.handleEvent(keyEvent(SDL_KEYDOWN, SDLK_UP, true));
    require(!shooting.getBullet(), "Key repeat fired another bullet");
    shooting.handleEvent(keyEvent(SDL_KEYDOWN, SDLK_UP));
    require(shooting.getBullet().has_value(), "Could not fire again after a hit");

    GameScreen missedShot;
    missedShot.update(1.0);
    for (std::size_t row = 0; row < GameScreen::Rows; ++row) {
        for (std::size_t column = 0; column < GameScreen::Columns; ++column) {
            missedShot.markEnemyDead(row, column);
        }
    }
    missedShot.handleEvent(keyEvent(SDL_KEYDOWN, SDLK_UP));
    missedShot.update(0.95);
    require(missedShot.getBullet().has_value(), "Bullet expired before leaving the screen");
    missedShot.update(0.05);
    require(!missedShot.getBullet(), "Bullet remained beyond the top of the screen");
    missedShot.handleEvent(keyEvent(SDL_KEYDOWN, SDLK_UP));
    require(missedShot.getBullet().has_value(), "Could not fire again after missing");

    GameScreen slowFrame;
    slowFrame.update(1.0);
    slowFrame.handleEvent(keyEvent(SDL_KEYDOWN, SDLK_UP));
    slowFrame.update(1.0);
    require(slowFrame.isEnemyDead(3, 7) && !slowFrame.getBullet(),
            "Fast bullet skipped an enemy during a slow frame");

    GameScreen lost;
    lost.update(30.0);
    lost.handleEvent(keyEvent(SDL_KEYDOWN, SDLK_UP));
    require(!lost.getBullet(), "Fired after Game Over");
    lost.handleEvent(keyEvent(SDL_KEYDOWN, SDLK_RIGHT));
    lost.update(1.0);
    checkPlayerX(renderer, lost, 464);

    StateManager states;
    states.handleEvent(keyEvent(SDL_KEYDOWN, SDLK_SPACE));
    states.update(1.0);
    states.handleEvent(keyEvent(SDL_KEYDOWN, SDLK_RIGHT));
    states.update(0.25);
    states.handleEvent(keyEvent(SDL_KEYUP, SDLK_RIGHT));
    states.update(0.25);
    states.handleEvent(keyEvent(SDL_KEYDOWN, SDLK_UP));
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    states.render(renderer);
    Uint32 pixel = 0;
    SDL_Rect bulletLocation{558, 464, 1, 1};
    require(SDL_RenderReadPixels(renderer, &bulletLocation, SDL_PIXELFORMAT_RGBA8888,
                                 &pixel, sizeof(pixel)) == 0 && pixel == White,
            "State manager did not forward movement, key release, or fire input");
}

void checkGrid(const Frame& frame, int expectedX, int expectedY) {
    require(std::count(frame.begin(), frame.end(), Red) == 32 * 32 * 32,
            "Expected 32 square enemies");
    for (int row = 0; row < 4; ++row) {
        for (int column = 0; column < 8; ++column) {
            const int x = expectedX + column * 56;
            const int y = expectedY + row * 48;
            require(frame[y * 960 + x] == Red, "Enemy top-left position mismatch");
            require(frame[(y + 31) * 960 + x + 31] == Red,
                    "Enemy bottom-right position mismatch");
        }
    }
}

void runTests(SDL_Renderer* renderer) {
    Enemy enemy(10.0, 20.0);
    require(enemy.getCoordinates().x == 10.0 && enemy.getCoordinates().y == 20.0,
            "Enemy constructor coordinates mismatch");
    enemy.updateCoordinates(120.25, 64.5);
    require(enemy.getCoordinates().x == 120.25 && enemy.getCoordinates().y == 64.5,
            "Enemy coordinate update mismatch");
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    enemy.render(renderer);
    Uint32 enemyPixel = 0;
    SDL_Rect location{120, 65, 1, 1};
    require(SDL_RenderReadPixels(renderer, &location, SDL_PIXELFORMAT_RGBA8888,
                                 &enemyPixel, sizeof(enemyPixel)) == 0,
            "Could not read enemy pixel");
    require(enemyPixel == Red, "Enemy did not render at updated coordinates");

    GameScreen game;
    auto frame = capture(renderer, game);
    require(frame[250 * 960 + 357] == White, "Missing level intro lettering");
    game.update(0.999);
    frame = capture(renderer, game);
    require(std::count(frame.begin(), frame.end(), Red) == 0,
            "Enemies spawned before one second");
    require(frame[405 * 960] == Black, "Gameplay appeared during intro");
    game.update(0.001);
    frame = capture(renderer, game);
    checkGrid(frame, 16, 32);
    require(frame[492 * 960 + 480] == White, "Missing player");
    for (int x = 0; x < 960; ++x) {
        require(frame[405 * 960 + x] == White, "Missing white loss line");
    }

    game.update(1.0);
    checkGrid(capture(renderer, game), 112, 32);
    game.update(4.25);
    checkGrid(capture(renderer, game), 520, 80);
    game.update(0.5);
    checkGrid(capture(renderer, game), 472, 80);
    game.update(4.75);
    checkGrid(capture(renderer, game), 16, 128);

    game.update(10.5);
    frame = capture(renderer, game);
    checkGrid(frame, 16, 224);
    require(!game.hasLost(), "Lost before enemies crossed the line");
    game.update(5.25);
    require(game.hasLost(), "Crossing the line did not cause a loss");
    const auto lostFrame = capture(renderer, game);
    require(lostFrame[253 * 960 + 352] == White, "Missing Game Over lettering");
    for (int x = 0; x < 960; ++x) {
        require(lostFrame[405 * 960 + x] == White, "Enemies obscured the loss line");
    }
    game.update(60.0);
    require(capture(renderer, game) == lostFrame, "Enemies kept moving after loss");

    GameScreen largeStep;
    GameScreen smallSteps;
    largeStep.update(1.0);
    smallSteps.update(1.0);
    largeStep.update(20.0);
    for (int step = 0; step < 1200; ++step) {
        smallSteps.update(1.0 / 60.0);
    }
    require(capture(renderer, largeStep) == capture(renderer, smallSteps),
            "Movement depends on update frequency");
    largeStep.update(100.0);
    smallSteps.update(100.0);
    require(largeStep.hasLost() && smallSteps.hasLost(),
            "Long updates skipped the loss condition");
    require(capture(renderer, largeStep) == capture(renderer, smallSteps),
            "Long updates moved enemies beyond the loss point");

    GameScreen freshGame;
    require(!freshGame.hasLost(), "New game retained previous loss state");
    freshGame.update(1.0);
    checkGrid(capture(renderer, freshGame), 16, 32);

    GameScreen withDeadEnemy;
    withDeadEnemy.update(1.0);
    withDeadEnemy.markEnemyDead(0, 0);
    require(withDeadEnemy.isEnemyDead(0, 0) && !withDeadEnemy.isEnemyDead(0, 1),
            "Dead flag does not correspond to its matrix cell");
    frame = capture(renderer, withDeadEnemy);
    require(std::count(frame.begin(), frame.end(), Red) == 31 * 32 * 32,
            "Dead enemy was rendered");
    require(frame[32 * 960 + 16] == Black, "Dead enemy remained visible");
    withDeadEnemy.update(5.75);
    const auto deadPosition = withDeadEnemy.getEnemyGrid()[0][0].getCoordinates();
    require(deadPosition.x == 16.0 && deadPosition.y == 32.0,
            "Dead enemy moved horizontally or descended");
    const auto livePosition = withDeadEnemy.getEnemyGrid()[0][1].getCoordinates();
    require(livePosition.x == 528.0 && livePosition.y == 80.0,
            "Living enemies stopped moving after a death");

    GameScreen deadEdge;
    deadEdge.update(1.0);
    for (std::size_t row = 0; row < GameScreen::Rows; ++row) {
        deadEdge.markEnemyDead(row, GameScreen::Columns - 1);
    }
    deadEdge.update(5.25);
    require(deadEdge.getEnemyGrid()[0][0].getCoordinates().y == 32.0,
            "Dead edge column caused an early turn");
    deadEdge.update(0.75);
    const auto afterTurn = deadEdge.getEnemyGrid()[0][0].getCoordinates();
    require(afterTurn.x == 560.0 && afterTurn.y == 80.0,
            "Formation did not turn at its living edge");

    GameScreen deadBottom;
    deadBottom.update(1.0);
    for (std::size_t column = 0; column < GameScreen::Columns; ++column) {
        deadBottom.markEnemyDead(GameScreen::Rows - 1, column);
    }
    deadBottom.update(26.25);
    require(!deadBottom.hasLost(), "Dead bottom row caused a loss");
    require(deadBottom.getEnemyGrid()[3][0].getCoordinates().y == 176.0,
            "Dead bottom row descended");
    require(deadBottom.getEnemyGrid()[2][0].getCoordinates().y == 368.0,
            "Living bottom row did not descend");
    deadBottom.update(5.25);
    require(deadBottom.hasLost(), "Living enemy crossing the line did not cause loss");

    GameScreen allDead;
    allDead.update(1.0);
    for (std::size_t row = 0; row < GameScreen::Rows; ++row) {
        for (std::size_t column = 0; column < GameScreen::Columns; ++column) {
            allDead.markEnemyDead(row, column);
        }
    }
    const auto emptyFormation = capture(renderer, allDead);
    allDead.update(1000.0);
    require(!allDead.hasLost(), "Empty formation caused a loss");
    require(capture(renderer, allDead) == emptyFormation,
            "Empty formation moved or rendered dead enemies");
    for (std::size_t row = 0; row < GameScreen::Rows; ++row) {
        for (std::size_t column = 0; column < GameScreen::Columns; ++column) {
            const auto position = allDead.getEnemyGrid()[row][column].getCoordinates();
            require(position.x == 16 + column * 56 && position.y == 32 + row * 48,
                    "Dead matrix coordinates were changed");
        }
    }
    bool invalidIndexRejected = false;
    try {
        allDead.markEnemyDead(0, GameScreen::Columns);
    } catch (const std::out_of_range&) {
        invalidIndexRejected = true;
    }
    require(invalidIndexRejected, "Invalid column aliased a different enemy flag");
}
} // namespace

int main() {
    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << SDL_GetError() << '\n';
        return 1;
    }
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(
        0, 960, 540, 32, SDL_PIXELFORMAT_RGBA8888);
    SDL_Renderer* renderer = surface ? SDL_CreateSoftwareRenderer(surface) : nullptr;
    int result = 0;
    try {
        require(renderer != nullptr, SDL_GetError());
        runTests(renderer);
        runCombatTests(renderer);
        std::cout << "Gameplay, movement, firing, collision, and rendering checks passed.\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        result = 1;
    }
    SDL_DestroyRenderer(renderer);
    SDL_FreeSurface(surface);
    SDL_Quit();
    return result;
}
