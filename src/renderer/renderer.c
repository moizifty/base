#include "renderer.h"

#ifdef RENDERER_BACKEND == RENDERER_D3D11
#include "d3d11\rendererD3D11.c"
#else
#error Platform not defined
#endif