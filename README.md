# Space Intruders

A minimal C++17 game using SDL2. The state manager starts on the title screen;
the title displays retro pixel lettering and a "Press any key to start" prompt.
Press any key to switch to the game and Escape to return to the title. Each new
game shows "Level 1" for one second, then spawns a green bitmap player ship at the
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
Clearing every enemy advances the level and displays its number for one second,
then resets the player, enemy formation, movement direction, and bullet.
Enemies start at 48 pixels per second and gain 12 pixels per second each level.
Bullets travel upward at 960 pixels per second.
The ship uses a 3-row, 5-column bitmap uploaded once per game to a transparent
SDL texture, then reused across frames and levels with 6 by 6 pixel blocks.
`PlayerShip` owns the ship's position, movement, bitmap, texture, and rendering;
`GameScreen` coordinates firing, enemy collisions, and level transitions.
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
path. Local builds use your installed SDL2 by default. Set
`-DSPACE_INTRUDERS_FETCH_SDL2=ON` to download and build the pinned SDL2 version.

## Windows builds and GitHub Releases

The [Windows workflow](.github/workflows/windows-release.yml) builds and tests
Windows x64 on pushes to `main`, pull requests, and manual runs from the Actions
tab. Download `space-intruders-windows-x64` from a successful run's artifacts,
extract the ZIP inside it, and run `space_intruders.exe`. Keep `SDL2.dll` beside
the executable. The package also includes the game and SDL2 licenses.

To publish a release, commit and push the changes, then push a version tag:

```sh
git tag v0.1.0
git push origin v0.1.0
```

Tags beginning with `v` trigger the same build and tests, then create a GitHub
Release with generated notes and `space-intruders-windows-x64.zip` attached.
Publishing uses GitHub's built-in token; no additional secrets are needed.

To build the package locally on Windows, install Visual Studio 2022 with
Desktop development with C++, CMake, and PowerShell, then run:

```powershell
cmake -S . -B build -A x64 -DSPACE_INTRUDERS_FETCH_SDL2=ON -DBUILD_TESTING=ON
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
cmake --install build --config Release --prefix package/space-intruders
Compress-Archive -Path package/space-intruders -DestinationPath space-intruders-windows-x64.zip
```

The Windows build links the C++ runtime statically and SDL2 dynamically, so the
ZIP includes `SDL2.dll` and needs no separate Visual C++ runtime installation.
