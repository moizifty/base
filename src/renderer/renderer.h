#ifndef RENDERER_H
#define RENDERER_H

#include "base\baseCore.h"
#include "base\baseMemory.h"

#ifdef RENDERER_BACKEND == RENDERER_D3D11
#include "d3d11\rendererD3D11.h"
#else
#error Platform not defined
#endif

typedef struct RendererState
{
    u8 _opaque;
}RendererState;

// each window that is being rendered to will have
// a differemt state,
// eg a backbuffer and swapchain and stuff
typedef struct RendererWindowState
{
    u8 _opaque;
}RendererWindowState;

RendererState *RendererInit(BaseArena *arena, OSGfxState *gfxState);
#endif