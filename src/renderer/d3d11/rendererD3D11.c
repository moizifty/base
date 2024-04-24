#include "renderer\renderer.h"
#include "rendererD3D11.h"

IDXGIAdapter *rendererD3D11FindBestAdapter(void)
{
    IDXGIFactory *factory = null;
    HRESULT hr = CreateDXGIFactory(&IID_IDXGIFactory, &factory);

    if (HRFAILURE(hr))
    {
        baseColPrintf("Failed to create dxgi factory, error '%ld'\n", (long)hr);
        return null;
    }

    IDXGIAdapter *adapter = null;
    for(int i = 0; factory->lpVtbl->EnumAdapters(factory, i, &adapter) != DXGI_ERROR_NOT_FOUND;)
    {
        DXGI_ADAPTER_DESC desc = {0};
        hr = adapter->lpVtbl->GetDesc(adapter, &desc);

        if(HRFAILURE(hr))
        {
            baseColPrintf("Failed to get description while enumerating adapters, error '%ld'\n", (long)hr);
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

RendererState *RendererInit(BaseArena *arena, OSGfxState *gfxState)
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
        baseColPrintf("{r}Failed to create d3d11 device, error: %ld\n", hr);
        return null;
    }

    ID3D11InfoQueue *infoQueue = null;
    state->device->lpVtbl->QueryInterface(state->device, &IID_ID3D11InfoQueue, &infoQueue);

    if (infoQueue != null)
    {
        infoQueue->lpVtbl->SetBreakOnSeverity(infoQueue, D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
        infoQueue->lpVtbl->SetBreakOnSeverity(infoQueue, D3D11_MESSAGE_SEVERITY_ERROR, true);
    }
    else
    {
        baseColPrintf("{b}Failed to get info queue interface from device, hr: %ld\n", hr);
        return null;
    }

    return (RendererState *) state;
}