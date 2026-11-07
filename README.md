# Space Intruders

A minimal C++17 game using SDL2. The state manager starts on the title screen;
press Enter to switch to the game and Escape to return to the title. Both screens
are empty for now. Close the window to quit.

## Build and run

Install a C++ compiler, CMake, and the SDL2 development package (for example,
`sudo apt install build-essential cmake libsdl2-dev` on Debian/Ubuntu), then run:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/space_intruders
```

SDL2 is dynamically linked in all build configurations. When distributing a
release, provide the matching SDL2 shared library alongside the game or require
it to be installed on the target system. On Windows, put `SDL2.dll` next to the
executable; on Linux/macOS, ensure the shared library is on the loader's search
path. SDL2 itself is not downloaded or bundled by this project.
