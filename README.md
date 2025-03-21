Note we compile to native OpenGL, except for webassembly,where we compile OpenGL ES 3.0
Dependencies:
-SDL
-opengl
-emscripten
-linmath.h
-math.h
-string.h
-stdio.h
-time.h
-stdlib.h
-freetype

Below a list of things needed to be setup for development. However note that this is incomplete as of now.

### Develop on macOS (^Catalina) native OpenGL

- git clone this repo
- install gcc/clang
- install homebrew
- brew install sdl2 sdl2_image sdl2_mixer sdl2_net sdl2_ttf freetype
- clang -std=c18 -Wall -pedantic \*.c -lSDL2 -I/usr/local/include/freetype2 -L/usr/local/lib -lfreetype -framework OpenGL -DGL_SILENCE_DEPRECATION && ./a.out

### Develop on windows (^10) native OpenGL

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

### Develop on linux(ubuntu) native OpenGL

- git clone this repo
- install gcc?
- sudo apt install libsdl2-dev
- sudo apt-get install libgles2-mesa-dev
- sudo apt-get update
- sudo apt-get install libfreetype6-dev
- gcc -std=c18 -Wall -pedantic *.c -I/usr/include/freetype2 -lfreetype -lSDL2 -lGLESv2 -lm -DDEV_MODE && ./a.out
- to debug: gcc -std=c18 -Wall -pedantic \*.c -lSDL2 -lGLESv2 -lm -g

### Develop for Webassembly (wasm) OpenGL ES 3.0

- install emscripten/emsdk
- activate emsdk in project by typing emsdk activate
- emcc -v to verify installation
- include <emscripten.h>
- setup ifdef for emscripten include & emscripten_set_main_loop
- install live-server
- compile:
  - emcc \*.c -o index.html -s USE_SDL=2 -s USE_WEBGL2=1 --preload-file shaders/wasm/ -s INITIAL_MEMORY=33554432 -s ALLOW_MEMORY_GROWTH=1 --preload-file container.jpg -s USE_FREETYPE=1 --preload-file ARIAL.TTF
- run:
  - live-server

## Deploy
