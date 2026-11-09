# Space Intruders

A minimal C++17 game using SDL2. The state manager starts on the title screen;
the title displays retro pixel lettering and a "Press any key to start" prompt.
Press any key to switch to the game and Escape to return to the title. Each new
game shows "Level 1" for one second, then spawns a white square player at the
bottom and an 8 by 4 grid of red square enemies at the top. Enemies sweep across
the screen, reverse at each edge, and descend one row. If any enemy extends past
the white line three-quarters down the screen, movement stops and "Game Over"
appears. Press Escape to return to the title and start again.
Enemy positions live in a 4 by 8 matrix of `Enemy` objects, with a matching
row-major boolean array of dead flags. Dead enemies keep their last coordinates
and are skipped during movement, edge checks, loss checks, and rendering.
Hold Left or Right to move the player; press Up to fire. Only one bullet can be
active at a time. It travels upward and disappears when it hits a living enemy
or fully leaves the top of the screen. A hit marks that enemy dead.
The loop targets 60 FPS, with elapsed time used for the intro and movement.
Close the window to quit.

The window starts at 1280 by 720 and can be resized or maximized. Rendering uses
a fixed 960 by 540 coordinate system with nearest-neighbor scaling. SDL preserves
the aspect ratio and adds black bars when the window has a different shape.

## Build and run

Install a C++ compiler, CMake, and the SDL2 development package (for example,
`sudo apt install build-essential cmake libsdl2-dev` on Debian/Ubuntu), then run:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/space_intruders
```

Run the headless gameplay checks with `ctest --test-dir build --output-on-failure`.

SDL2 is dynamically linked in all build configurations. When distributing a
release, provide the matching SDL2 shared library alongside the game or require
it to be installed on the target system. On Windows, put `SDL2.dll` next to the
executable; on Linux/macOS, ensure the shared library is on the loader's search
path. SDL2 itself is not downloaded or bundled by this project.
