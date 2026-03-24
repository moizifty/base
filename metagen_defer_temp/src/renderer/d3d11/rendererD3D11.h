#pragma comment(lib, "d3d11")
#pragma comment(lib, "dxguid")
#pragma comment(lib, "dxgi")

#ifndef RENDERER_D3D11_H
#define RENDERER_D3D11_H

#include "base/baseCore.h"
#include "base/baseStrings.h"
#include "os/core/win32\osCoreWin32.h"
#include "os/gfx\osGfx.h"

#include <d3d11.h>
#include <d3d11_1.h>
#include <dxgi.h>
#include <dxgi1_2.h>

typedef struct RendererStateD3D11
{
    ID3D11Device *device;
    ID3D11DeviceContext *deviceContext;
    IDXGIFactory2 *factory2;

    ID3D11Debug *debug;
    ID3D11InfoQueue *infoQueue;
    OSGfxState *gfxState;
}RendererStateD3D11;

// each window that is being rendered to will have
// a differemt state,
// eg a backbuffer and swapchain and stuff
typedef struct RendererWindowStatePlatformD3D11
{
    IDXGISwapChain1 *swapChain;

    ID3D11Texture2D *framebuffer;
    ID3D11RenderTargetView *framebufferRTV;

    ID3D11Texture2D *depthBuffer;
    ID3D11DepthStencilView *depthBufferDSV;
}RendererWindowStatePlatformD3D11;

// get the best adapter to use, eg nvidia, amd etc, instead of it possibly using dedicated gpu 
IDXGIAdapter *rendererD3D11FindBestAdapter(void);

#endif