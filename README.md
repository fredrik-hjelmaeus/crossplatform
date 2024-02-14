### Develop on macOS (^Catalina)

- git clone this repo
- install gcc/clang
- install homebrew
- brew install sdl2 sdl2_image sdl2_mixer sdl2_net sdl2_ttf
- clang -std=c18 -Wall -pedantic \*.c -lSDL2

### Develop on windows (^10)

- Download SDL from https://www.libsdl.org/
  probably named SDL2-devel-2.30.0-VC.zip.
- Install Visual Studio 2019 & Desktop development with C++ package.
- Create new Empty C++ Console Project
- In VS Project Property Pages/Configuration Properties/:
- VC++ Directories:
- add include directories folder to the include folder of SDL
- add Library directories folder to the lib folder of SDL (x86)
  take the dll file in the sdl2/lib folder and copy it to your project folder.
- Manually copy all c & h files to Source Files.

### Develop on linux(ubuntu)

- git clone this repo
- install gcc?
- sudo apt install libsdl2-dev
- gcc -std=c18 -Wall -pedantic \*.c -lSDL2

### Develop for Webassembly (wasm)

- install emscripten/emsdk
- activate emsdk in project by typing emsdk activate
- emcc -v to verify installation
- include <emscripten.h>
- setup ifdef for emscripten include & emscripten_set_main_loop
- compile:
  emcc main.c -c -o output.o -s USE_SDL=2
  emcc output.o -o index.html -s USE_SDL=2
  open index.html with liveserver.

## Deploy
