// Example low level rendering Unity plugin


#include "UnityPluginInterface.h"

#include <math.h>
#include <stdio.h>
#include <d3d11.h>


// --------------------------------------------------------------------------
// SetTextureFromUnity, an example function we export which is called by one of the scripts.

static void* g_TexturePointer;

extern "C" void EXPORT_API SetTextureFromUnity (void* texturePtr)
{
	// A script calls this at initialization time; just remember the texture pointer here.
	// Will update texture pixels each frame from the plugin rendering event (texture update
	// needs to happen on the rendering thread).
	g_TexturePointer = texturePtr;
}



// --------------------------------------------------------------------------
// UnitySetGraphicsDevice

static int g_DeviceType = -1;
static ID3D11Device* g_D3D11Device;
extern unsigned char *gTextureData;

extern "C" void EXPORT_API UnitySetGraphicsDevice (void* device, int deviceType, int eventType)
{
	// Set device type to -1, i.e. "not recognized by our plugin"
	g_DeviceType = -1;
	
	// D3D11 device, remember device pointer and device type.
	// The pointer we get is ID3D11Device.
	if (deviceType == kGfxRendererD3D11)
	{		
		g_DeviceType = deviceType;
		g_D3D11Device = (ID3D11Device*)device;		
	}
}

extern "C" void EXPORT_API UnityRenderEvent(int eventID)
{
	// Unknown graphics device type? Do nothing.
	if (g_DeviceType == -1)
		return;

	// Actual functions defined below
	//SetDefaultGraphicsState();

	if (g_DeviceType == kGfxRendererD3D11)
	{
		ID3D11DeviceContext* ctx = NULL;
		g_D3D11Device->GetImmediateContext(&ctx);
		// update native texture from code
		if (g_TexturePointer)
		{
			ID3D11Texture2D* d3dtex = (ID3D11Texture2D*)g_TexturePointer;
			D3D11_TEXTURE2D_DESC desc;
			d3dtex->GetDesc(&desc);

			if (gTextureData != 0)
				ctx->UpdateSubresource(d3dtex, 0, NULL, gTextureData, desc.Width * 4, 0);

		}

		ctx->Release();
	}
}


