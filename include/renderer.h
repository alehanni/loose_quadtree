#ifndef RENDERER_H
#define RENDERER_H

#define R_RAYLIB 0
#define R_SOKOL 1

#ifndef RENDERER
#define RENDERER R_RAYLIB
#endif

#if (RENDERER == R_RAYLIB)
#include "raylib_renderer.h"
#elif (RENDERER == R_SOKOL)
// include sokol
#endif

#endif