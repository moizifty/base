#ifndef RENDERER_H
#define RENDERER_H

#include "base\baseCore.h"
#include "base\baseMemory.h"

#if RENDERER_BACKEND == RENDERER_D3D11
#include "d3d11\rendererD3D11.h"
#else
#error Platform not defined
#endif

typedef struct RendererState
{
    u8 _opaque;
}RendererState;

typedef struct RendererWindowStatePlatform
{
    u64 _opaque;
}RendererWindowStatePlatform;
// each window that is being rendered to will have
// a differemt state,
// eg a backbuffer and swapchain and stuff
typedef struct RendererWindowState
{
    RendererWindowStatePlatform *platformSpecific;
    vec2i lastResolution;
    bool preformedFirstPaint;

    vec4f clearColor;
}RendererWindowState;

RendererState *rendererInit(BaseArena *arena, OSGfxState *gfxState);
RendererWindowState *rendererAttachToWindow(RendererState *rs, BaseArena *arena, OSHandle window);

void rendererWindowResizeBuffers(RendererState *rs, RendererWindowState *wndState, vec2i newResolution);
void rendererWindowBegin(RendererState *rs, RendererWindowState *wndState, vec2i resolution);
void rendererWindowEnd(RendererState *rs, RendererWindowState *wndState);
void rendererOutputFinalDebugReport(BaseArena *arena, RendererState *rs);
#endif