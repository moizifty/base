#pragma comment(lib, "d3d11")
#pragma comment(lib, "dxguid")
#pragma comment(lib, "dxgi")

#ifndef RENDERER_D3D11_H
#define RENDERER_D3D11_H

#include "base\baseCore.h"
#include "os\core\win32\osCoreWin32.h"
#include "os\gfx\osGfx.h"

#include <d3d11.h>

typedef struct RendererStateD3D11
{
    ID3D11Device *device;
    ID3D11DeviceContext *deviceContext;

    OSGfxState *gfxState;
}RendererStateD3D11;

// each window that is being rendered to will have
// a differemt state,
// eg a backbuffer and swapchain and stuff
typedef struct RendererWindowStateD3D11
{
    IDXGISwapChain *swapChain;
    ID3D11Texture2D *framebuffer;
}RendererWindowStateD3D11;

// get the best adapter to use, eg nvidia, amd etc, instead of it possibly using dedicated gpu 
IDXGIAdapter *rendererD3D11FindBestAdapter(void);

#endif