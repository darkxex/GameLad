#pragma once

#include <cstring>
#include <string>
#include <memory>
#include <cstdlib>

#if WINDOWS
    #include <SDL.h>
  #include <SDL_ttf.h>
#else
    #include "SDL2/SDL.h"
  #include "SDL2/SDL_ttf.h"
#endif

#include "Logger.hpp"

typedef unsigned char byte;
