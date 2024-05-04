#include "renderer\renderer.h"
#include "rendererD3D11.h"

// platform specific
IDXGIAdapter *rendererD3D11FindBestAdapter(void)
{
    IDXGIFactory *factory = null;
    HRESULT hr = CreateDXGIFactory(&IID_IDXGIFactory, &factory);

    if (HRFAILURE(hr))
    {
        logProgErrorFmt("Failed to create dxgi factory, error '%ld'\n", (long)hr);
        return null;
    }

    IDXGIAdapter *adapter = null;
    for(int i = 0; factory->lpVtbl->EnumAdapters(factory, i, &adapter) != DXGI_ERROR_NOT_FOUND; i++)
    {
        DXGI_ADAPTER_DESC desc = {0};
        hr = adapter->lpVtbl->GetDesc(adapter, &desc);

        if(HRFAILURE(hr))
        {
            logProgErrorFmt("Failed to get description while enumerating adapters, error '%ld'\n", (long)hr);
            return null;
        }

        // 0x10de = nvidia vender id
        if(desc.VendorId == 0x10de)
        {
            break;
        }
    }

    factory->lpVtbl->Release(factory);

    return adapter;
}

// core
RendererState *rendererInit(BaseArena *arena, OSGfxState *gfxState)
{
    if(gfxState == null)
    {
        return null;
    }
    
    RendererStateD3D11 *state = baseArenaPush(arena, sizeof(RendererStateD3D11));
    state->gfxState = gfxState;

    IDXGIAdapter *adapter = rendererD3D11FindBestAdapter();
    if (adapter == null)
    {
        return null;
    }

    D3D_FEATURE_LEVEL featureLevels[] = 
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };

    HRESULT hr = D3D11CreateDevice(adapter, 
                                   D3D_DRIVER_TYPE_UNKNOWN,
                                   null,
                                   D3D11_CREATE_DEVICE_DEBUG,
                                   featureLevels,
                                   BASE_ARRAY_SIZE(featureLevels),
                                   D3D11_SDK_VERSION,
                                   &state->device,
                                   null,
                                   &state->deviceContext);
    
    if (HRFAILURE(hr))
    {
        logProgErrorFmt("{r}Failed to create d3d11 device, error: %ld\n", hr);
        return null;
    }

    // todo: gate some of this behind debug flags
    // get debug interface
    {
        hr = state->device->lpVtbl->QueryInterface(state->device, &IID_ID3D11Debug, &state->debug);

        if (state->debug == null)
        {
            logProgWarningFmt("{b}Failed to get info queue interface from device, hr: %ld\n", hr);
            return null;
        }
    }

    // set up debug break pointing on certain messages
    {
        hr = state->device->lpVtbl->QueryInterface(state->device, &IID_ID3D11InfoQueue, &state->infoQueue);

        if (state->infoQueue != null)
        {
            state->infoQueue->lpVtbl->SetBreakOnSeverity(state->infoQueue, D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
            state->infoQueue->lpVtbl->SetBreakOnSeverity(state->infoQueue, D3D11_MESSAGE_SEVERITY_ERROR, true);
        }
        else
        {
            logProgErrorFmt("{b}Failed to get info queue interface from device, hr: %ld\n", hr);
            return null;
        }
    }

    //get factory
    {
        adapter->lpVtbl->GetParent(adapter, &IID_IDXGIFactory2, &state->factory2);
    }

    logProgInfoFmt("Successfully initialized renderer - D3D11");
    adapter->lpVtbl->Release(adapter);

    return (RendererState *) state;
}

RendererWindowState *rendererAttachToWindow(RendererState *rs, BaseArena *arena, OSHandle window)
{
    RendererWindowState *wndState = baseArenaPush(arena, sizeof(RendererWindowState));
    RendererWindowStatePlatformD3D11 *d3dWndState = baseArenaPush(arena, sizeof(RendererWindowStatePlatformD3D11));
    HWND wndHandle = (HWND) window._u64;

    wndState->platformSpecific = (RendererWindowStatePlatform *) d3dWndState;

    RendererStateD3D11 *rsD3D = (RendererStateD3D11 *)rs;

    DXGI_SWAP_CHAIN_DESC1 desc = 
    {
        .Width = 0,
        .Height = 0,
        .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
        .Stereo = false,
        .SampleDesc = {.Count = 1, .Quality = 0},
        .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
        .BufferCount = 2,
        .Scaling = DXGI_SCALING_STRETCH,
        .SwapEffect = DXGI_SWAP_EFFECT_DISCARD,
        .AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
        .Flags = 0,
    };

    IUnknown *deviceBase = null;
    rsD3D->device->lpVtbl->QueryInterface(rsD3D->device, &IID_IUnknown, &deviceBase);

    HRESULT hr = rsD3D->factory2->lpVtbl->CreateSwapChainForHwnd(rsD3D->factory2, 
                                                                 deviceBase, wndHandle, 
                                                                 &desc, 
                                                                 null, null, 
                                                                 &d3dWndState->swapChain);

    if(HRFAILURE(hr))
    {
        logProgErrorFmt("Failed to create swap chain, error %ld. \n", hr);
        return null;
    }

    hr = d3dWndState->swapChain->lpVtbl->GetBuffer(d3dWndState->swapChain, 0, &IID_ID3D11Texture2D, &d3dWndState->framebuffer);
    if(HRFAILURE(hr))
    {
        logProgErrorFmt("Failed to get swap chain frame buffer. \n");
        return null;
    }

    logProgInfoFmt("Successfully attached renderer to window with handle '%llu' - D3D11", (u64)wndHandle);
    deviceBase->lpVtbl->Release(deviceBase);
    return wndState;
}

void rendererWindowResizeBuffers(RendererState *rs, RendererWindowState *wndState, vec2i newResolution)
{
    if(newResolution.width <= 0 || newResolution.height <= 0)
    {
        return;
    }
    
    RendererWindowStatePlatformD3D11 *d3dWndState = (RendererWindowStatePlatformD3D11 *) wndState->platformSpecific;
    RendererStateD3D11 *d3dRS = (RendererStateD3D11 *) rs;
    d3dRS->deviceContext->lpVtbl->OMSetRenderTargets(d3dRS->deviceContext, 0, null, null);

    str8 frameBufferDebugName = STR8_LIT("FRAME_BUFFER");
    str8 depthBufferDebugName = STR8_LIT("DEPTH_BUFFER");

    HRESULT hr = 0;
    if (d3dWndState->framebufferRTV != null)
    {
        d3dWndState->framebufferRTV->lpVtbl->Release(d3dWndState->framebufferRTV);
    }

    d3dWndState->framebuffer->lpVtbl->Release(d3dWndState->framebuffer);
    hr = d3dWndState->swapChain->lpVtbl->ResizeBuffers(d3dWndState->swapChain, 0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

    if(HRFAILURE(hr))
    {
        logProgErrorFmt("Failed to resize swapchain buffers to (%lld x %lld).", newResolution.w, newResolution.h);
        return;
    }

    hr = d3dWndState->swapChain->lpVtbl->GetBuffer(d3dWndState->swapChain, 0, &IID_ID3D11Texture2D, &d3dWndState->framebuffer);
    d3dWndState->framebuffer->lpVtbl->SetPrivateData(d3dWndState->framebuffer, &WKPDID_D3DDebugObjectName, (UINT) frameBufferDebugName.len, frameBufferDebugName.data);

    if(HRFAILURE(hr))
    {
        logProgErrorFmt("Failed to get framebuffer from swapchain");
        return;
    }

    if (d3dWndState->depthBufferDSV != null)
    {
        d3dWndState->depthBufferDSV->lpVtbl->Release(d3dWndState->depthBufferDSV);
        d3dWndState->depthBuffer->lpVtbl->Release(d3dWndState->depthBuffer);
    }

    D3D11_TEXTURE2D_DESC depthBufferDesc = 
    {
        .Width = (UINT)newResolution.width,
        .Height = (UINT)newResolution.height,
        .MipLevels = 1,
        .ArraySize = 1,
        .Format = DXGI_FORMAT_D32_FLOAT,
        .SampleDesc.Count = 1,
        .SampleDesc.Quality = 0,
        .Usage = D3D11_USAGE_DEFAULT,
        .BindFlags = D3D11_BIND_DEPTH_STENCIL,
        .CPUAccessFlags = 0,
        .MiscFlags = 0,
    };

    hr = d3dRS->device->lpVtbl->CreateTexture2D(d3dRS->device, &depthBufferDesc, null, &d3dWndState->depthBuffer);
    d3dWndState->depthBuffer->lpVtbl->SetPrivateData(d3dWndState->depthBuffer, &WKPDID_D3DDebugObjectName, (UINT)depthBufferDebugName.len, depthBufferDebugName.data);

    if(HRFAILURE(hr))
    {
        logProgErrorFmt("Failed to create depth buffer texture");
        return;
    }

    hr = d3dRS->device->lpVtbl->CreateDepthStencilView(d3dRS->device, (ID3D11Resource *)d3dWndState->depthBuffer, null, &d3dWndState->depthBufferDSV);
    if(HRFAILURE(hr))
    {
        logProgErrorFmt("Failed to create depth buffer view");
        return;
    }

    hr = d3dRS->device->lpVtbl->CreateRenderTargetView(d3dRS->device, (ID3D11Resource *)d3dWndState->framebuffer, null, &d3dWndState->framebufferRTV);
    if(HRFAILURE(hr))
    {
        logProgErrorFmt("Failed to create render target view for framebuffer.");
        return;
    }

    d3dRS->deviceContext->lpVtbl->OMSetRenderTargets(d3dRS->deviceContext, 1, &d3dWndState->framebufferRTV, d3dWndState->depthBufferDSV);

    logProgInfoFmt("Successfully resized swapchain buffers to (%lld x %lld).", newResolution.w, newResolution.h);
}
void rendererWindowBegin(RendererState *rs, RendererWindowState *wndState, vec2i resolution)
{
    bool resolutionChanged = (resolution.h != wndState->lastResolution.h 
                           && resolution.w != wndState->lastResolution.w);

    RendererWindowStatePlatformD3D11 *d3dWndState = (RendererWindowStatePlatformD3D11 *) wndState->platformSpecific;
    RendererStateD3D11 *d3dRS = (RendererStateD3D11 *) rs;

    if(resolutionChanged || (d3dWndState->framebufferRTV == null || d3dWndState->depthBufferDSV == null))
    {
        rendererWindowResizeBuffers(rs, wndState, resolution);
    }
    wndState->lastResolution = resolution;

    d3dRS->deviceContext->lpVtbl->ClearRenderTargetView(d3dRS->deviceContext, d3dWndState->framebufferRTV, Vec4f(0.5f, 0.5f, 0.5f, 1.0f).v);
}
void rendererWindowEnd(RendererState *rs, RendererWindowState *wndState)
{
    RendererWindowStatePlatformD3D11 *d3dWndState = (RendererWindowStatePlatformD3D11 *) wndState->platformSpecific;
    RendererStateD3D11 *d3dRS = (RendererStateD3D11 *) rs;

    d3dWndState->swapChain->lpVtbl->Present(d3dWndState->swapChain, 1, 0);
    d3dRS->deviceContext->lpVtbl->ClearState(d3dRS->deviceContext);
}

void rendererOutputFinalDebugReport(BaseArena *arena, RendererState *rs)
{
    logProgInfoFmt("Printing renderer final debug report.");

    RendererStateD3D11 *d3dRS = (RendererStateD3D11 *) rs;
    ID3D11InfoQueue *infoQueue = d3dRS->infoQueue;

    logProgInfoFmt("Reporting All Live Device Objects");
    HRESULT hr = d3dRS->debug->lpVtbl->ReportLiveDeviceObjects(d3dRS->debug, D3D11_RLDO_IGNORE_INTERNAL | D3D11_RLDO_DETAIL);
    if(HRFAILURE(hr))
    {
        logProgErrorFmt("Failed to report live device objects, error: %d", hr);
        return;
    }

    // output all messages to log
    u64 nuMessages = infoQueue->lpVtbl->GetNumStoredMessages(infoQueue);
    for(u64 i = 0; i < nuMessages; i++)
    {
        u64 messageByteLength = 0;
        hr = infoQueue->lpVtbl->GetMessage(infoQueue, i, null, &messageByteLength);
        if(HRFAILURE(hr))
        {
            logProgErrorFmt("Failed to get size for message from infoqueue at index '%llu', error code '%d' please debug the program.", i, hr);
            return;
        }

        D3D11_MESSAGE *message = baseArenaPush(arena, messageByteLength);
        hr = infoQueue->lpVtbl->GetMessage(infoQueue, i, message, &messageByteLength);
        if(HRFAILURE(hr))
        {
            logProgErrorFmt("Failed to message from infoqueue at index '%llu', error code '%d' please debug the program.", i, hr);
            return;
        }

        str8 msg = baseStr8((u8*)message->pDescription, message->DescriptionByteLength - 1);

        switch(message->Severity)
        {
            case D3D11_MESSAGE_SEVERITY_MESSAGE:
            case D3D11_MESSAGE_SEVERITY_INFO:
            {
                logProgInfoFmt("%S", msg);
            }break;
            case D3D11_MESSAGE_SEVERITY_CORRUPTION:
            case D3D11_MESSAGE_SEVERITY_ERROR:
            {
                logProgErrorFmt("%S", msg);
            }break;
            case D3D11_MESSAGE_SEVERITY_WARNING:
            {
                logProgWarningFmt("%S", msg);
            }break;
        }
    }
}