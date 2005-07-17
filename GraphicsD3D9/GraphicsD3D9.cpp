/* QuestDesigner - Open Legends's Project
   Copyright (C) 2003-2005. Germán Méndez Bravo (Kronuz)
   Copyright (C) 2001-2003. Open Legends's Project
 
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
/////////////////////////////////////////////////////////////////////////////
/*! \file		GraphicsD3D9.cpp
	\author		Germán Méndez Bravo (Kronuz)
	\brief		Implementation of the DirectGraphics 2D Graphics plugin for Open Legends.
	\date		May 19, 2003: 
					* Creation date.
				June 9, 2003
				September 10, 2003: 
					- Bug Fix: The Alpha and RGB values are now updated in the vertex buffer
					  in case a change has been made (different values). Fixed!
				September 17, 2003
					+ Optimization. The vertex buffer is no longer needed to be updated
					  neither it needs to contain the ARGB values of the vertices.
					  (The vertices color and alpha value are set ar Draw time using
					   two stages for the color (aditive blending) and modulation for
					   alpha blending)
				October 8, 2003
					+ Improvement. All methods converting from float to int, missed
					  the +0.5f adjustment, now added.
				    + Optimization. Some methods used SetRect() instead of setting the
					  values on RECT creation. SetRect is now only used when needed.
			    November 18, 2003
					+ Improvement. Filters added: Pixelate, Alpha, and Panning.
			    December 5, 2003
					- Bug fix: Using filters, Zooming was messing up with the final
					  flipped texture. Showing 2x, 4x, etc. parts of the texture, 
					  instead of just 0x-1x parts of it. Fixed!
			    September 10, 2004
					- Bug fix: Some cards didn't work... now it has 
					  partial support for those cards and doesn't crash.
			    September 13, 2004
					- Improvement: Branched GraphicsD3D9.dll from GraphicsD3D8.dll
			    September 14, 2004
					- Bug Fix:  Added only square textures and power of 2 textures support.
			    Jul 6, 2005
					- Improvement: Increased the precision on the SetWorldPosition for screen resizing a little.
	\remarks	Known bugs:
				Take the next scenario. There is an open map (being displayed), the
				textures were loaded from png (or bmp) files.
				In this state, if you delete the png, or create one if there was 
				only a bmp, and change the resolution of the screen, or change 
				full-screen/windowed status, the texture source will change to bmp or
				png respecively (bmp have a scaling factor of 2, while png doesn't)
				this provokes wrong scaling for the sprites in that sprite-sheet.
				Texture transformation is changed as it should, but the textures
				still show wrong. ??
				There's a strange flickering when there are two 
				running GraphicsD3D engines (i.e. QD and OL debugger)

*/

#include "stdafx.h"
#include "GraphicsD3D9.h"

//////////////////////////////////////////////////////////////////////////////
// Interface Version Definition:
const WORD IGraphics::Version = GRAPHICS_INTVER;

//////////////////////////////////////////////////////////////////////////////
// Needed Libraries:
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

//////////////////////////////////////////////////////////////////////////////
// Exported functions:

HRESULT QueryGfxInterface(WORD Version, IGraphics **pInterface, IConsole *Output)
{
	if( HIBYTE(Version) != HIBYTE(IGraphics::Version) ) return E_FAIL;
	if(	LOBYTE(Version) > LOBYTE(IGraphics::Version) ) return E_FAIL;

	if(!*pInterface) {
		// Convert our interface pointer to a CGraphicsD3D9 object
		*pInterface = new CGraphicsD3D9;
		(*pInterface)->SetConsole(Output);
		return S_OK;
	}

	return E_FAIL;
}
// Release our Handle to the class
HRESULT ReleaseGfxInterface(IGraphics **pInterface)
{
	if(!*pInterface) {
		return E_FAIL;
	}

	// Not our device:
	if((*pInterface)->GetDeviceID() != D3D9_DEVICE_ID) {
		return E_FAIL;
	}

	delete *pInterface;
	*pInterface = NULL;
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
// Entry point:
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call) {
		// called when we attach to the DLL
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		// called when we free or detach from the DLL
		case DLL_PROCESS_DETACH:
			break;
	}

	//  If we had run into an error (like not being able to allocate memory
	//  or something else) we would return FALSE.  This would tell Windows
	//  that we were unnable to succesfully initialize.  If this DLL was
	//  being loaded by a program as it started, the program would fail to
	//  load.  If this DLL was being loaded by a running program via a call
	//  to LoadLibrary(), LoadLibrary would return NULL to the calling program.
    return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
// Misc functions:
static int int_log2(int val) 
{
    int log = 0;
    while ((val >>= 1) != 0)
		log++;
    return log;
}

//////////////////////////////////////////////////////////////////////////////
// Interface implementation begins here:
D3DDISPLAYMODE CGraphicsD3D9::ms_PreferredMode;
D3DDISPLAYMODE CGraphicsD3D9::ms_WindowedMode;
D3DCAPS9 CGraphicsD3D9::ms_D3DCaps;
D3DPRESENT_PARAMETERS CGraphicsD3D9::ms_d3dpp;
int CGraphicsD3D9::ms_nCount = 0;
int CGraphicsD3D9::ms_nScreenWidth = 0;
int CGraphicsD3D9::ms_nScreenHeight = 0;
bool CGraphicsD3D9::ms_bWindowed = true;
bool CGraphicsD3D9::ms_bLastRendered = false;
IDirect3D9 *CGraphicsD3D9::ms_pD3D = NULL;
IDirect3DDevice9 *CGraphicsD3D9::ms_pD3DDevice = NULL;
std::map<std::string, CTextureD3D9*> CGraphicsD3D9::ms_Textures;

IDirect3DSurface9 *CGraphicsD3D9::ms_pBackBufferSurface = NULL;
IDirect3DTexture9 *CGraphicsD3D9::ms_pHelperTexture = NULL;
IDirect3DSurface9 *CGraphicsD3D9::ms_pHelperSurface = NULL;
IDirect3DTexture9 *CGraphicsD3D9::ms_pHelperTexture2 = NULL;
IDirect3DSurface9 *CGraphicsD3D9::ms_pHelperSurface2 = NULL;
int CGraphicsD3D9::ms_nHelperWidth;
int CGraphicsD3D9::ms_nHelperHeight;
D3DFORMAT CGraphicsD3D9::ms_TextureFormat;

CBufferD3D9::CBufferD3D9(SVertexBuffer *pBuffer) : 
	m_pBuffer(pBuffer),
	m_nCount(0)
{
	CONSOLE_DEBUG4("DEBUG: IBuffer created.\n");
	AddRef();
}
CBufferD3D9::~CBufferD3D9() 
{
	BEGIN_DESTRUCTOR
	CONSOLE_DEBUG4("DEBUG: IBuffer deleted.\n"); 
	END_DESTRUCTOR
}

void CBufferD3D9::Touch() 
{
	m_bDirty = true;
}

void CBufferD3D9::Invalidate(bool full) 
{
	if(m_pBuffer) {
		delete m_pBuffer;
		CONSOLE_DEBUG4("DEBUG: Invalidating a buffer.\n"); 
	} else {
		CONSOLE_DEBUG4("DEBUG: Invalidating an invalid buffer!\n"); 
	}
	m_pBuffer = NULL; 
}
IBuffer* CBufferD3D9::AddRef() { 
	m_nCount++;
	CONSOLE_DEBUG3("DEBUG: IBuffer reference added. (%d references)\n", m_nCount); 
	return this; 
}
IBuffer* CBufferD3D9::Release() { 
	ASSERT(m_nCount>0); 
	CONSOLE_DEBUG3("DEBUG: IBuffer reference removed. (%d references)\n", m_nCount-1); 
	if(--m_nCount==0) { 
		CONSOLE_DEBUG4("DEBUG: Deleting texture.\n"); 
		delete this; 
	} 
	return NULL; 
}	

//////////////////////////////////////////////////////////////////////////////

CTextureD3D9::CTextureD3D9(IDirect3DTexture9 *pTexture, D3DXIMAGE_INFO &imageInfo, D3DSURFACE_DESC &surfaceDesc) : 
	m_pTexture(pTexture),
	m_ImageInfo(imageInfo),
	m_fScale(1.0f),
	m_nCount(0)
{
	m_Width = surfaceDesc.Width;
	m_Height = surfaceDesc.Height;

	CONSOLE_DEBUG4("DEBUG: ITexture created.\n");
	AddRef();
}
CTextureD3D9::~CTextureD3D9() {
	BEGIN_DESTRUCTOR
	CONSOLE_DEBUG4("DEBUG: ITexture deleted.\n"); 
	END_DESTRUCTOR
}

void CTextureD3D9::Invalidate(bool full) { 
	if(m_pTexture) {
		m_pTexture->Release(); 
		CONSOLE_DEBUG3("DEBUG: Invalidating a texture.\n"); 
	} else {
		CONSOLE_DEBUG3("DEBUG: Invalidating an invalid texture!\n"); 
	}
	m_pTexture = NULL; 
}
ITexture* CTextureD3D9::AddRef() { 
	m_nCount++;
	CONSOLE_DEBUG3("DEBUG: ITexture reference added. (%d references)\n", m_nCount); 
	return this; 
}
ITexture* CTextureD3D9::Release() { 
	ASSERT(m_nCount>0); 
	CONSOLE_DEBUG3("DEBUG: ITexture reference removed. (%d references)\n", m_nCount-1); 
	if(--m_nCount==0) { 
		CONSOLE_DEBUG4("DEBUG: Deleting texture.\n"); 
		delete this; 
	} 
	return NULL; 
}

//////////////////////////////////////////////////////////////////////////////
// This is the transformation table for the render function.
const struct STrans {
	float angle;
	bool startTop;
	bool startLeft;
} cRenderTable[4][4] = { // [transform][rotate]
	////////////////////////////////////////////////////////////////////////
	//   angle			 ST      SL
 	// GFX_NORMAL:
	{ {0.0f,			true,	true},		// GFX_ROTATE_0
	  {-D3DX_PI/2.0f,	false,	true},		// GFX_ROTATE_90
	  {-D3DX_PI,		false,	false},		// GFX_ROTATE_180
	  {D3DX_PI/2.0f,	true,	false} },	// GFX_ROTATE_270
	// GFX_MIRRORED:
	{ {0.0f,			true,	false},		// GFX_ROTATE_0
	  {-D3DX_PI/2.0f,	false,	false},		// GFX_ROTATE_90
	  {-D3DX_PI,		false,	true},		// GFX_ROTATE_180
	  {D3DX_PI/2.0f,	true,	true} },	// GFX_ROTATE_270
	// GFX_FLIPPED:
	{ {0.0f,			false,	true},		// GFX_ROTATE_0
	  {-D3DX_PI/2.0f,	true,	true},		// GFX_ROTATE_90
	  {-D3DX_PI,		true,	false},		// GFX_ROTATE_180
	  {D3DX_PI/2.0f,	false,	false} },	// GFX_ROTATE_270
	// GFX_MIRRORED | GFX_FLIPPED:
	{ {0.0f,			false,	false},		// GFX_ROTATE_0
	  {-D3DX_PI/2.0f,	true,	false},		// GFX_ROTATE_90
	  {-D3DX_PI,		true,	true},		// GFX_ROTATE_180
	  {D3DX_PI/2.0f,	false,	true} }		// GFX_ROTATE_270
};
inline void CGraphicsD3D9::Clear(const RECT *rect, ARGBCOLOR rgbColor) const
{
	if(!m_bInitialized) return;
	ASSERT(ms_pD3DDevice);

	D3DVERIFY(ms_pD3DDevice->SetTransform(D3DTS_WORLD, &m_WorldMatrix));
	if(rect) {
		D3DVERIFY(ms_pD3DDevice->Clear(1, (D3DRECT*)rect, D3DCLEAR_TARGET, rgbColor, 1.0f, 0));
	} else {
		D3DVERIFY(ms_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, rgbColor, 1.0f, 0));
	}
}

inline LPCSTR CGraphicsD3D9::GetFormat(D3DFORMAT Format) const
{
	switch(Format) {
		case D3DFMT_R8G8B8: return "D3DFMT_R8G8B8";
		case D3DFMT_A8R8G8B8: return "D3DFMT_A8R8G8B8";
		case D3DFMT_X8R8G8B8: return "D3DFMT_X8R8G8B8";
		case D3DFMT_R5G6B5: return "D3DFMT_R5G6B5";
		case D3DFMT_X1R5G5B5: return "D3DFMT_X1R5G5B5";
		case D3DFMT_A1R5G5B5: return "D3DFMT_A1R5G5B5";
		case D3DFMT_A4R4G4B4: return "D3DFMT_A4R4G4B4";
		case D3DFMT_A8: return "D3DFMT_A8";
		case D3DFMT_R3G3B2: return "D3DFMT_R3G3B2";
		case D3DFMT_A8R3G3B2: return "D3DFMT_A8R3G3B2";
		case D3DFMT_X4R4G4B4: return "D3DFMT_X4R4G4B4";
		case D3DFMT_A2B10G10R10: return "D3DFMT_A2B10G10R10";
		case D3DFMT_G16R16: return "D3DFMT_G16R16";
		case D3DFMT_A8P8: return "D3DFMT_A8P8";
		case D3DFMT_P8: return "D3DFMT_P8";
		case D3DFMT_L8: return "D3DFMT_L8";
		case D3DFMT_A8L8: return "D3DFMT_A8L8";
		case D3DFMT_A4L4: return "D3DFMT_A4L4";
		case D3DFMT_V8U8: return "D3DFMT_V8U8";
		case D3DFMT_Q8W8V8U8: return "D3DFMT_Q8W8V8U8";
		case D3DFMT_V16U16: return "D3DFMT_V16U16";
		//case D3DFMT_W11V11U10: return "D3DFMT_W11V11U10";
		case D3DFMT_L6V5U5: return "D3DFMT_L6V5U5";
		case D3DFMT_X8L8V8U8: return "D3DFMT_X8L8V8U8";
		case D3DFMT_A2W10V10U10: return "D3DFMT_A2W10V10U10";
		case D3DFMT_UYVY: return "D3DFMT_UYVY";
		case D3DFMT_YUY2: return "D3DFMT_YUY2";
		case D3DFMT_DXT1: return "D3DFMT_DXT1";
		case D3DFMT_DXT2: return "D3DFMT_DXT2";
		case D3DFMT_DXT3: return "D3DFMT_DXT3";
		case D3DFMT_DXT4: return "D3DFMT_DXT4";
		case D3DFMT_DXT5: return "D3DFMT_DXT5";
		case D3DFMT_VERTEXDATA: return "D3DFMT_VERTEXDATA";
		case D3DFMT_INDEX16: return "D3DFMT_INDEX16";
		case D3DFMT_INDEX32: return "D3DFMT_INDEX32";
		case D3DFMT_D16_LOCKABLE: return "D3DFMT_D16_LOCKABLE";
		case D3DFMT_D32: return "D3DFMT_D32";
		case D3DFMT_D15S1: return "D3DFMT_D15S1";
		case D3DFMT_D24S8: return "D3DFMT_D24S8";
		case D3DFMT_D16: return "D3DFMT_D16";
		case D3DFMT_D24X8: return "D3DFMT_D24X8";
		case D3DFMT_D24X4S4: return "D3DFMT_D24X4S4";
	}
	return "D3DFMT_UNKNOWN";
}
inline WORD CGraphicsD3D9::GetBitCount() const
{
	switch(ms_PreferredMode.Format) {
		case D3DFMT_A8R8G8B8:				// 32 bit color with alpha
		case D3DFMT_X8R8G8B8:	return 32;	// 32 bit color without alpha
		case D3DFMT_R8G8B8:		return 24;	// 24 bit color
		case D3DFMT_A4R4G4B4:				// 16 bit color with 4 bit alpha
		case D3DFMT_A1R5G5B5:				// 16 bit color with 1 bit alpha
		case D3DFMT_X1R5G5B5:				// 16 bit color without alpha
		case D3DFMT_R5G6B5:		return 16;	// 16 bit color with extra green
		case D3DFMT_R3G3B2:		return 8;
		default:				return 0;
	}
}

// Using the current format, get the equivalent 16 bits ARGB values from a buffer in the 
// current format, and increment the pointer of the buffer to the next ARGB color.
inline WORD CGraphicsD3D9::GetNext16ARGB(BYTE **Data) const
{
	WORD rgb555 = -1;
	switch(ms_PreferredMode.Format) {
		case D3DFMT_R8G8B8   :
			rgb555 = ( 0x8000 | ((((*Data)[2])>>3)<<10) | ((((*Data)[1])>>3)<<5) | ((((*Data)[0])>>3)) );
			(*Data)+=3;
			return rgb555;
		case D3DFMT_A8R8G8B8 :
			rgb555 = ( ((((*Data)[3])>>7)<<11) | ((((*Data)[2])>>3)<<10) | ((((*Data)[1])>>3)<<5) | ((((*Data)[0])>>3)) );
			(*Data)+=4;
			return rgb555;
		case D3DFMT_X8R8G8B8 :
			rgb555 = ( 0x8000 | ((((*Data)[2])>>3)<<10) | ((((*Data)[1])>>3)<<5) | ((((*Data)[0])>>3)) );
			(*Data)+=4;
			return rgb555;
		case D3DFMT_R5G6B5   :
			rgb555 = ( 0x8000 | (((*(WORD*)(*Data))>>1)&0x7fe0) | ((*(WORD*)(*Data))&0x1f) );
			(*Data)+=2;
			return rgb555;
		case D3DFMT_X1R5G5B5 :
			rgb555 = ( 0x8000 | (*(WORD*)(*Data)) );
			(*Data)+=2;
			return rgb555;
		case D3DFMT_A1R5G5B5 :
			rgb555 = (*(WORD*)(*Data));
			(*Data)+=2;
			return rgb555;
	}
	return rgb555;
}
// Figure out vertex processing for Direct3D initialization
inline DWORD CGraphicsD3D9::FigureOutVertexProcessing()
{
	ASSERT(ms_pD3D);

	// Set the device/format for SW vertex processing
	DWORD dwBehavior = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	// Confirm the device/format for HW vertex processing
	if(ms_D3DCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) {
		if(ms_D3DCaps.DevCaps & D3DDEVCAPS_PUREDEVICE ) {
			dwBehavior = D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE;
		} else {
			dwBehavior = D3DCREATE_HARDWARE_VERTEXPROCESSING;
		}
	}
	return dwBehavior;
}

// FigureOutDisplayMode Direct3D initialization
inline void CGraphicsD3D9::FigureOutDisplayMode(D3DFORMAT currentFormat, UINT width, UINT height) 
{
	HRESULT hr;
	UINT numModes;

	D3DDISPLAYMODE bestMode;
	ZeroMemory(&bestMode,sizeof(D3DDISPLAYMODE));
	D3DDISPLAYMODE aMode;
	ZeroMemory(&aMode,sizeof(D3DDISPLAYMODE));

	// from the worse to the best...
    D3DFORMAT validformats[6] = { 
		D3DFMT_R5G6B5 ,
		D3DFMT_X1R5G5B5,
		D3DFMT_A1R5G5B5, 
		D3DFMT_A2R10G10B10, 
		D3DFMT_X8R8G8B8, 
		D3DFMT_A8R8G8B8
	};

	// try the valid adapter modes:
	for(UINT f=0; f<6; f++) {
		numModes = ms_pD3D->GetAdapterModeCount(D3DADAPTER_DEFAULT, validformats[f]);
		if(numModes == 0) continue;
		for(UINT i=0; i<numModes; i++) {
			if(FAILED(hr=ms_pD3D->EnumAdapterModes(D3DADAPTER_DEFAULT, validformats[f], i, &aMode))) {
				if(hr != D3DERR_NOTAVAILABLE) return;
			} else {
				if((aMode.Width == width) && (aMode.Height == height)) {
					if( (aMode.RefreshRate > bestMode.RefreshRate) && 
						(aMode.RefreshRate <= ms_PreferredMode.RefreshRate) ) {
						bestMode = aMode;
					}
				}
			}
		}
	}
	// save the new preferred mode for the adapter:
	ms_PreferredMode = bestMode;
}

inline void CGraphicsD3D9::PostInitialize(UINT WindowWidth, UINT WindowHeight)
{
	D3DXMATRIX Ortho2D;	
	
	D3DXMatrixOrthoOffCenterLH(&Ortho2D, 0.0f, (float)WindowWidth, (float)WindowHeight, 0.0f, 0.0f , 1.0f);
	D3DXMatrixIdentity(&m_IdentityMatrix);

	D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_LIGHTING, FALSE)); // Disable Lighting
	D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE/*/D3DCULL_CW/**/)); // Turn on Culling (nice speed boost)
	

	D3DVERIFY(ms_pD3DDevice->SetTransform(D3DTS_PROJECTION, &Ortho2D));
	D3DVERIFY(ms_pD3DDevice->SetTransform(D3DTS_WORLD, &m_IdentityMatrix));
	D3DVERIFY(ms_pD3DDevice->SetTransform(D3DTS_VIEW, &m_IdentityMatrix));

	D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_LOCALVIEWER, FALSE));
	D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_ZENABLE, FALSE)); // Disable Z-Buffer

	D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,  TRUE));
	D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
	D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));
	D3DVERIFY(ms_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE));

	ms_bLastRendered = false;
	D3DVERIFY(ms_pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE));
	D3DVERIFY(ms_pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT));
	D3DVERIFY(ms_pD3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE));
	D3DVERIFY(ms_pD3DDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE));
	D3DVERIFY(ms_pD3DDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT));

	D3DVERIFY(ms_pD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
	D3DVERIFY(ms_pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT));
	D3DVERIFY(ms_pD3DDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3));

}

// Receives inclusive-exclusive coordinates
inline void CGraphicsD3D9::RenderRect(const RECT &rectDest, ARGBCOLOR rgbColor) const
{
	float l = (float)(rectDest.left);
	float t = (float)(rectDest.top);
	float r = (float)(rectDest.right) - 1.0f;
	float b = (float)(rectDest.bottom) - 1.0f;

	D3DCDVERTEX rect[5] = {
		{ l, t, 0.0f, rgbColor },
		{ l, b, 0.0f, rgbColor },
		{ r, b, 0.0f, rgbColor },
		{ r, t, 0.0f, rgbColor },
		{ l, t, 0.0f, rgbColor }
	};

	D3DVERIFY(ms_pD3DDevice->SetTexture(0, NULL));
//	D3DVERIFY(ms_pD3DDevice->SetVertexShader(D3DFVF_XYZ | D3DFVF_DIFFUSE));
	D3DVERIFY(ms_pD3DDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE));

	if(ms_bLastRendered == true) {
		ms_bLastRendered = false;
		D3DVERIFY(ms_pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE));
		D3DVERIFY(ms_pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT));
		D3DVERIFY(ms_pD3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE));
		D3DVERIFY(ms_pD3DDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE));
		D3DVERIFY(ms_pD3DDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT));
	}

	D3DVERIFY(ms_pD3DDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, rect, sizeof(D3DCDVERTEX)));
}

// Receives inclusive-exclusive coordinates
inline void CGraphicsD3D9::RenderFill(const RECT &rectDest, ARGBCOLOR rgbColor) const
{
	float l = (float)(rectDest.left);
	float t = (float)(rectDest.top);
	float r = (float)(rectDest.right);
	float b = (float)(rectDest.bottom);

	D3DCDVERTEX rect[4] = {
		{ l, t, 0.0f, rgbColor },
		{ l, b, 0.0f, rgbColor },
		{ r, t, 0.0f, rgbColor },
		{ r, b, 0.0f, rgbColor }
	};

	D3DVERIFY(ms_pD3DDevice->SetTexture(0, NULL));
//	D3DVERIFY(ms_pD3DDevice->SetVertexShader(D3DFVF_XYZ | D3DFVF_DIFFUSE));
	D3DVERIFY(ms_pD3DDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE));
	if(ms_bLastRendered == true) {
		ms_bLastRendered = false;
		D3DVERIFY(ms_pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE));
		D3DVERIFY(ms_pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT));
		D3DVERIFY(ms_pD3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE));
		D3DVERIFY(ms_pD3DDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE));
		D3DVERIFY(ms_pD3DDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT));
	}

	D3DVERIFY(ms_pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, rect, sizeof(D3DCDVERTEX)));
}
inline void CGraphicsD3D9::UpdateVertexBuffer(SVertexBuffer **vbuffer, const ITexture *texture, const RECT &rectSrc, const RECT &rectDest, int rotate, int transform, ARGBCOLOR rgbColor) const
{
	float invW = 1.0f/(float)texture->GetWidth();
	float invH = 1.0f/(float)texture->GetHeight();
	float udiff = (float)(rectSrc.left-(*vbuffer)->m_texLeft) * invW;
	float vdiff = (float)(rectSrc.top-(*vbuffer)->m_texTop) * invH;
	if((*vbuffer)->m_bOverflow) {
		rgbColor = COLOR_ARGB(128,255,0,0);
	}

	D3DCDTVERTEX *tile = (*vbuffer)->m_pVertices;
	for(DWORD offset=0; offset < (*vbuffer)->m_dwVertices; offset++) {
		tile[offset].u += udiff;
		tile[offset].v += vdiff;
		tile[offset].dwColor = (D3DCOLOR)rgbColor;
	}
	(*vbuffer)->m_texTop = rectSrc.top;
	(*vbuffer)->m_texLeft = rectSrc.left;
	(*vbuffer)->m_rgbColor = rgbColor;

}

inline void CGraphicsD3D9::CreateVertexBuffer(SVertexBuffer **vbuffer, const ITexture *texture, const RECT &rectSrc, const RECT &rectDest, int rotate, int transform, ARGBCOLOR rgbColor) const
{
	ASSERT(vbuffer);

	int destWidth, destHeight;
	if(rotate == GFX_ROTATE_90 || rotate == GFX_ROTATE_270) {
		destWidth = (rectDest.bottom-rectDest.top);
		destHeight = (rectDest.right-rectDest.left);
	}  else {
		destWidth = (rectDest.right-rectDest.left);
		destHeight = (rectDest.bottom-rectDest.top);
	}
	int srcWidth = rectSrc.right - rectSrc.left;
	int srcHeight = rectSrc.bottom - rectSrc.top;

	// Calculate the number of vertex needed on the mesh
	int xVert = destWidth/srcWidth;
	int yVert = destHeight/srcHeight;

	// Calculate the width and height of the last column and row of the mesh
	int lastWidth = destWidth % srcWidth;
	int lastHeight = destHeight % srcHeight;

	if(lastWidth) xVert++;
	else lastWidth = srcWidth;
	if(lastHeight) yVert++;
	else lastHeight = srcHeight;

	DWORD dwVertices = xVert * yVert * 6;
	DWORD dwPrimitives = xVert * yVert * 2;
	bool bOverflow = false;
	D3DCDTVERTEX *tile = NULL;

	// Calculate the exact bounds of the sprite inside the texture
	float invW = 1.0f/(float)texture->GetWidth();
	float invH = 1.0f/(float)texture->GetHeight();
	float tl = (float)rectSrc.left * invW;
	float tt = (float)rectSrc.top * invH;
	float tr = (float)rectSrc.right * invW;
	float tb = (float)rectSrc.bottom * invH;

	if( dwVertices > ms_D3DCaps.MaxVertexIndex || dwPrimitives > ms_D3DCaps.MaxPrimitiveCount ) {
		// If there are too many vertices (for the current video card):
		dwVertices = 6;
		dwPrimitives = 2;

		rgbColor = COLOR_ARGB(128,255,0,0);

		float l = 0.0f;
		float t = 0.0f;
		float r = (float)(destWidth);
		float b = (float)(destHeight);
		D3DCDTVERTEX tmp[6] = {
			{ l, t, 0.0f, rgbColor, tl, tt},
			{ l, b, 0.0f, rgbColor, tl, tb},
			{ r, t, 0.0f, rgbColor, tr, tt},
			{ l, b, 0.0f, rgbColor, tl, tb},
			{ r, b, 0.0f, rgbColor, tr, tb},
			{ r, t, 0.0f, rgbColor, tr, tt}
		};
		tile = new D3DCDTVERTEX[dwVertices];
		memcpy(tile, &tmp, sizeof(tmp));

		bOverflow = true;
	} else {
		// Calculate the exact bounds for the last row and column on the mesh in the texture
		float tlr = (float)(rectSrc.left+lastWidth) * invW;
		float tlb = (float)(rectSrc.top+lastHeight) * invH;

		float vcx[6] = {0, 0, (float)srcWidth, (float)srcWidth, 0, (float)srcWidth};
		float vcy[6] = {0, (float)srcHeight, 0, 0, (float)srcHeight, (float)srcHeight};
		float vtx[6] = {tl,tl,tr,tr,tl,tr};
		float vty[6] = {tt,tb,tt,tt,tb,tb};

		tile = new D3DCDTVERTEX[dwVertices];

		int offset = 0;
		for(int y=0; y<yVert; y++) {
			for(int x=0; x<xVert; x++) {
				for(int i=0; i<6; i++) {
					if(x==xVert-1 && (i==2||i==3||i==5) ) {
						tile[offset].x = vcx[i] + (x-1)*srcWidth + lastWidth;
						tile[offset].u = tlr;
					} else {
						tile[offset].x = vcx[i] + x*srcWidth;
						tile[offset].u = vtx[i];
					}
					if(y==yVert-1 && (i==1||i==4||i==5) ) {
						tile[offset].y = vcy[i] + (y-1)*srcHeight + lastHeight;
						tile[offset].v = tlb;
					} else {
						tile[offset].y = vcy[i] + y*srcHeight;
						tile[offset].v = vty[i];
					}

					tile[offset].z = 0.0f;
					tile[offset].dwColor = rgbColor;
					offset++;
				}
			}
		}
	}
	*vbuffer = new SVertexBuffer(rgbColor, tile, dwVertices, dwPrimitives, rectSrc.top, rectSrc.left, bOverflow);
}

inline bool CGraphicsD3D9::Recover()
{
	CONSOLE_DEBUG1("DEBUG: Recovering device...\n"); 

	if(!SetMode(m_hWnd, ms_bWindowed, ms_nScreenWidth, ms_nScreenHeight)) return false;

	return true;
}
inline void CGraphicsD3D9::Invalidate(bool full)
{
	// Release indirectly created references: 
	if(ms_pD3DDevice && ms_pBackBufferSurface) {
		D3DVERIFY(ms_pD3DDevice->SetRenderTarget(0, ms_pBackBufferSurface));
		D3DVERIFY(ms_pD3DDevice->SetTexture(0, NULL));
	}

	// Release the helper textures: 
	if(ms_pHelperSurface) {
		ms_pHelperSurface->Release();
		ms_pHelperSurface = NULL;
	}
	if(ms_pHelperSurface2) {
		ms_pHelperSurface2->Release();
		ms_pHelperSurface2 = NULL;
	}
	if(ms_pHelperTexture) {
		ms_pHelperTexture->Release();
		ms_pHelperTexture = NULL;
	}
	if(ms_pHelperTexture2) {
		ms_pHelperTexture2->Release();
		ms_pHelperTexture2 = NULL;
	}
	if(ms_pBackBufferSurface) {
		ms_pBackBufferSurface->Release();
		ms_pBackBufferSurface = NULL;
	}
	if(m_pD3DFont) {
		m_pD3DFont->Release();
		m_pD3DFont = NULL;
	}

	// Not needed for recovery, since textures are managed:
	// Invalidate and release all textures
	if(full) {
		ITexture *pTexture;
		std::map<std::string, CTextureD3D9*>::const_iterator texIter = ms_Textures.begin();
		while(texIter != ms_Textures.end()) {
			pTexture = texIter->second;
			pTexture->Invalidate(full);
			pTexture->Release();
			texIter++;
		}
		ms_Textures.clear();
	}
}

CGraphicsD3D9::CGraphicsD3D9() :
	m_bInitialized(false),
	m_bCapture(false),
	m_bFilters(false),
	m_bFilterPixelate(false),
	m_hWnd(NULL),
	m_Zoom(0.0f),
	m_rgbFilterBkColor(0),
	m_rgbClearColor(0),
	m_rgbFontColor(0),
	m_pD3DFont(NULL)
{
	m_pGrid[0] = NULL;
	m_pGrid[1] = NULL;

	SetFilter(ClearFilters, NULL);

	::SetRect(&m_RectView, 0, 0, 0, 0);
	::SetRect(&m_RectClip, 0, 0, 0, 0);
	::SetRect(&m_RectWorld, 0, 0, 0, 0);
	::SetRect(&m_RectOldClip, 0, 0, 0, 0);
}
CGraphicsD3D9::~CGraphicsD3D9()
{
	BEGIN_DESTRUCTOR
	delete []m_pGrid[0];
	delete []m_pGrid[1];
	Finalize();
	END_DESTRUCTOR
}

bool CGraphicsD3D9::SetMode(HWND hWnd, bool bWindowed, int nScreenWidth, int nScreenHeight)
{
	if(SetModeA(hWnd, bWindowed, nScreenWidth, nScreenHeight)) {
		return SetModeB(hWnd, bWindowed, nScreenWidth, nScreenHeight);
	}
	return false;
}
bool CGraphicsD3D9::SetModeA(HWND hWnd, bool bWindowed, int nScreenWidth, int nScreenHeight)
{
	HRESULT hr;

	Invalidate(false);

	if(FAILED(hr=ms_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &ms_PreferredMode))) {
		CONSOLE_PRINTF("ERROR (D3D9:%s): Couldn't receive the current display mode.\n", DXGetErrorString(hr));
		return false;
	}
	if(ms_pD3DDevice == NULL) ms_WindowedMode = ms_PreferredMode;

	if(bWindowed) {
		ms_d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
		ms_PreferredMode = ms_WindowedMode;
	} else {
		ASSERT(nScreenWidth && nScreenHeight);
		FigureOutDisplayMode(ms_PreferredMode.Format, nScreenWidth, nScreenHeight);

		ms_d3dpp.SwapEffect = D3DSWAPEFFECT_FLIP;
		ms_d3dpp.FullScreen_RefreshRateInHz = ms_PreferredMode.RefreshRate;
	}
	ms_d3dpp.BackBufferWidth  = ms_PreferredMode.Width;		// The back buffer width
	ms_d3dpp.BackBufferHeight = ms_PreferredMode.Height;	// The back buffer height
	ms_d3dpp.BackBufferFormat = ms_PreferredMode.Format; 

	ms_d3dpp.hDeviceWindow = hWnd;							// Handle to the parent window
	ms_d3dpp.Windowed = bWindowed;							// Set windowed mode
	ms_d3dpp.EnableAutoDepthStencil = FALSE;
	ms_d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;	// for screenshots

	// Find out the best supported texture format for the device preferred format:
	ms_TextureFormat = D3DFMT_UNKNOWN;
	D3DFORMAT fsFormats[8] = {
		D3DFMT_A8R8G8B8,
		D3DFMT_X8R8G8B8,
		D3DFMT_R8G8B8,
		D3DFMT_A4R4G4B4,
		D3DFMT_A1R5G5B5,
		D3DFMT_X1R5G5B5,
		D3DFMT_R5G6B5,
		ms_PreferredMode.Format
	};
	for(UINT i=0; i<8; i++) {
		if(SUCCEEDED(ms_pD3D->CheckDeviceFormat(D3DADAPTER_DEFAULT, m_devType, ms_PreferredMode.Format, 0, 
			D3DRTYPE_TEXTURE, 
			fsFormats[i]))) { 
				ms_TextureFormat = fsFormats[i];
				break;
			}
	}

	return true;
}

bool CGraphicsD3D9::SetModeB(HWND hWnd, bool bWindowed, int nScreenWidth, int nScreenHeight)
{
	HRESULT hr;

	if(ms_pD3DDevice) {
		// Reset the device
		if(FAILED(hr = ms_pD3DDevice->Reset(&ms_d3dpp))) {
			CONSOLE_PRINTF("ERROR (D3D9:%s): Couldn't reset the device.\n", DXGetErrorString(hr));
			return false;
		}
	} else {
		// Create the device
		if(FAILED(hr = ms_pD3D->CreateDevice(D3DADAPTER_DEFAULT, m_devType, hWnd,
				FigureOutVertexProcessing(), &ms_d3dpp, &ms_pD3DDevice) )) { 
			CONSOLE_PRINTF("ERROR (D3D9:%s): Couldn't create the device.\n", DXGetErrorString(hr));
			return false;
		}
	}
	SetFont("Arial", 12, COLOR_ARGB(255,255,255,255), FW_BOLD); // initialize a default font.

	ms_bWindowed = bWindowed;

	ms_nScreenWidth = ms_PreferredMode.Width;
	ms_nScreenHeight = ms_PreferredMode.Height;

	// KingOfHeart repored that Intel 82815 doesn't create these helper textures (at 16 bits?):

	// Now we create a helper surface we'll be needing later for the some effects (pixelate and others)
	// Keep which surface is the old Render Target.
	D3DVERIFY(ms_pD3DDevice->GetRenderTarget(0, &ms_pBackBufferSurface));

	// Get the Description of the Primary Render Surface.
	D3DSURFACE_DESC SurfaceDescription;
	D3DVERIFY(ms_pBackBufferSurface->GetDesc(&SurfaceDescription));

	// If the device only supports sqare textures:
	if(!(ms_D3DCaps.TextureOpCaps & D3DPTEXTURECAPS_SQUAREONLY)) {
		SurfaceDescription.Width = SurfaceDescription.Height = max(SurfaceDescription.Width, SurfaceDescription.Height);
	}

	// If the device only supports textures in powers of two:
	if((ms_D3DCaps.TextureOpCaps & D3DPTEXTURECAPS_POW2)) {
		if(!(ms_D3DCaps.TextureOpCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL)) {
			SurfaceDescription.Width = 2 << int_log2(SurfaceDescription.Width);
			SurfaceDescription.Height = 2 << int_log2(SurfaceDescription.Height);
		}
	}

	// CreateTexture Fails on some video cards with some screen depths (i.e. 16 bits)
	// Create the Texture we will Render our scene to (for Filters in general), 
	// which is essentialy a Back Buffer duplicate.
	if(FAILED(hr=ms_pD3DDevice->CreateTexture(
		SurfaceDescription.Width, 
		SurfaceDescription.Height, 
		1, 
		SurfaceDescription.Usage, // should always be D3DUSAGE_RENDERTARGET
		ms_TextureFormat, 
		SurfaceDescription.Pool, // should always be D3DPOOL_DEFAULT
		&ms_pHelperTexture, 
		NULL
	))) {
		CONSOLE_PRINTF("ERROR (D3D9:%s): Video card only partially supported. No filters available (cropping, etc.)\n", DXGetErrorString(hr));
	} else {
		// Get the surface from this texture.
		D3DVERIFY(ms_pHelperTexture->GetSurfaceLevel(0, &ms_pHelperSurface));
	}

	// Create a second Texture we will Render our scene to (for Pixelate effect), 
	// which is essentialy a Back Buffer duplicate.
	if(FAILED(hr=ms_pD3DDevice->CreateTexture(
		SurfaceDescription.Width, 
		SurfaceDescription.Height, 
		1, 
		SurfaceDescription.Usage, // should always be D3DUSAGE_RENDERTARGET
		ms_TextureFormat, 
		SurfaceDescription.Pool, // should always be D3DPOOL_DEFAULT
		&ms_pHelperTexture2,
		NULL
	))) {
		CONSOLE_PRINTF("ERROR (D3D9:%s): Video card only partially supported. No special effects available (pixelate, etc.)\n", DXGetErrorString(hr));
	} else {
		// Get the surface from this texture.
		D3DVERIFY(ms_pHelperTexture2->GetSurfaceLevel(0, &ms_pHelperSurface2));
	}

	ms_nHelperWidth = SurfaceDescription.Width;
	ms_nHelperHeight = SurfaceDescription.Height;

	PostInitialize(ms_nScreenWidth, ms_nScreenHeight);
	return true;
}

// On windowed evironments, this function must receive a handler to the parent window (main frame) 
// and not to the child view that is requesting the graphics device.
bool CGraphicsD3D9::Initialize(HWND hWnd, bool bWindowed, int nScreenWidth, int nScreenHeight)
{
	m_devType = D3DDEVTYPE_HAL; // Hardware acceleration
//	m_devType = D3DDEVTYPE_REF; // 
//	m_devType = D3DDEVTYPE_SW;  // 

	if(m_bInitialized) return true;

	if(ms_pD3D == NULL) {
		ms_pD3D = Direct3DCreate9(D3D_SDK_VERSION); // Create Direct3D
		if(ms_pD3D == NULL) {
			// This will display a error message in the console
			CONSOLE_PRINTF("ERROR (D3D9): Couldn't initialize Direct3D.\n");
			return false;
		}
	}

	if(ms_pD3DDevice == NULL) {
		// Start Setting the new mode (just get information this time)
		if(!SetModeA(hWnd, bWindowed, nScreenWidth, nScreenHeight)) return false;
	}

	if(ms_nCount == 0) { // on the first run only...
		// get some information about the D3D Driver for later use:
		ZeroMemory(&ms_D3DCaps, sizeof(D3DCAPS9)); // clear it
		D3DVERIFY(ms_pD3D->GetDeviceCaps(D3DADAPTER_DEFAULT, m_devType, &ms_D3DCaps));
	}

	bool bStatus = true;

	if(ms_nCount++ == 0) {
		D3DADAPTER_IDENTIFIER9 D3DAdapterID;
		D3DVERIFY(ms_pD3D->GetAdapterIdentifier(D3DADAPTER_DEFAULT, NULL, &D3DAdapterID));

		// Output driver info
		CONSOLE_PRINTF("Using the " D3D9_FULLNAME " - '%s' (v%d.%d)\n", D3DAdapterID.Driver, HIBYTE(Version), LOBYTE(Version));
		CONSOLE_PRINTF(" | Description: %s (%d)\n", D3DAdapterID.Description, HIWORD(D3DAdapterID.DriverVersion.HighPart));
		CONSOLE_PRINTF(" | Version: %d.%d build %d\n", LOWORD(D3DAdapterID.DriverVersion.HighPart), HIWORD(D3DAdapterID.DriverVersion.LowPart), LOWORD(D3DAdapterID.DriverVersion.LowPart));
		CONSOLE_LOG   (" | dwVendorId: 0x%08X\n", D3DAdapterID.VendorId);
		CONSOLE_LOG   (" | dwDeviceId: 0x%08X\n", D3DAdapterID.DeviceId);
		CONSOLE_LOG   (" | dwSubSysId: 0x%08X\n", D3DAdapterID.SubSysId);
		CONSOLE_LOG   (" | dwRevision: 0x%08X\n", D3DAdapterID.VendorId);
		CONSOLE_LOG   (" | Mode: %dx%d at %dHz (%s)\n", ms_PreferredMode.Width, ms_PreferredMode.Height, ms_PreferredMode.RefreshRate, GetFormat(ms_PreferredMode.Format));
		CONSOLE_PRINTF(" | Acceleration: %s rasterization\n", (m_devType==D3DDEVTYPE_HAL)?"Hardware":"Software");
		CONSOLE_PRINTF(" | Max Texture Size: %dx%d\n", ms_D3DCaps.MaxTextureWidth, ms_D3DCaps.MaxTextureHeight);
		CONSOLE_LOG   (" | Max Texture Blend Stages: %d\n", ms_D3DCaps.MaxTextureBlendStages);
		CONSOLE_LOG   (" | Max Primitive Count: %d\n", ms_D3DCaps.MaxPrimitiveCount);
		CONSOLE_LOG   (" | Max Vertex Index: %d\n", ms_D3DCaps.MaxVertexIndex);
		CONSOLE_LOG   (" | Max Simultaneous Textures: %d\n", ms_D3DCaps.MaxSimultaneousTextures);
		CONSOLE_LOG   (" | Max Streams: %d\n", ms_D3DCaps.MaxStreams);
		CONSOLE_LOG   (" | Max Stream Stride: %d\n", ms_D3DCaps.MaxStreamStride);
		CONSOLE_LOG   (" | Vertex Shader Version: %d.%d\n", HIBYTE(LOWORD(ms_D3DCaps.VertexShaderVersion)), LOBYTE(LOWORD(ms_D3DCaps.VertexShaderVersion)));
		CONSOLE_LOG   (" | Max Vertex Shader Const: %d\n", ms_D3DCaps.MaxVertexShaderConst);
		CONSOLE_LOG   (" | Pixel Shader Version: %d.%d\n", HIBYTE(LOWORD(ms_D3DCaps.PixelShaderVersion)), LOBYTE(LOWORD(ms_D3DCaps.PixelShaderVersion)));

		// Print caps (do not change order, add at the end if needed):
		CONSOLE_LOG   (" | Device Caps: ");
		CONSOLE_LOG   ("0x%08X ", ms_D3DCaps.DevCaps);
		CONSOLE_LOG   ("0x%08X ", ms_D3DCaps.PrimitiveMiscCaps);
		CONSOLE_LOG   ("0x%08X ", ms_D3DCaps.RasterCaps);
		CONSOLE_LOG   ("0x%08X ", ms_D3DCaps.ZCmpCaps);
		CONSOLE_LOG   ("0x%08X ", ms_D3DCaps.SrcBlendCaps);
		CONSOLE_LOG   ("0x%08X ", ms_D3DCaps.DestBlendCaps);
		CONSOLE_LOG   ("0x%08X ", ms_D3DCaps.AlphaCmpCaps);
		CONSOLE_LOG   ("\n |              ");
		CONSOLE_LOG   ("0x%08X ", ms_D3DCaps.ShadeCaps);
		CONSOLE_LOG   ("0x%08X ", ms_D3DCaps.TextureCaps);
		CONSOLE_LOG   ("0x%08X ", ms_D3DCaps.TextureFilterCaps);
		CONSOLE_LOG   ("0x%08X ", ms_D3DCaps.TextureAddressCaps);
		CONSOLE_LOG   ("0x%08X ", ms_D3DCaps.TextureOpCaps);
		CONSOLE_LOG   ("0x%08X ", ms_D3DCaps.LineCaps);
		CONSOLE_LOG   ("\n");

		CONSOLE_LOG   (" | Support for temporary register: %s\n", ((ms_D3DCaps.PrimitiveMiscCaps & D3DPMISCCAPS_TSSARGTEMP) == D3DPMISCCAPS_TSSARGTEMP)?"Yes":"No");

		if(!(ms_D3DCaps.DevCaps & D3DPMISCCAPS_BLENDOP)) {
			CONSOLE_PRINTF(" | Device does not support the alpha-blending operations. *\n");
			bStatus = false;
		} 
		if(!(ms_D3DCaps.LineCaps & D3DLINECAPS_ANTIALIAS)) {
			CONSOLE_LOG(" | Device does not support antialiasing on lines.\n");
		}
//		if(!(ms_D3DCaps.RasterCaps & D3DPRASTERCAPS_PAT)) {
//			CONSOLE_PRINTF(" | Device does not support patterned drawing.\n");
//		}
		if(!(ms_D3DCaps.TextureCaps & D3DPTEXTURECAPS_ALPHA)) {
			CONSOLE_PRINTF(" | Device does not support alpha channel in texture pixels. *\n");
			bStatus = false;
		}
		if(!(ms_D3DCaps.TextureCaps & D3DPTEXTURECAPS_ALPHA) || !(ms_D3DCaps.TextureCaps & D3DPMISCCAPS_BLENDOP)) {
			CONSOLE_PRINTF(" | Device does not support full alpha channel in textures.\n");
		}
		if(ms_D3DCaps.MaxTextureWidth < 512 || ms_D3DCaps.MaxTextureHeight < 512) {
			CONSOLE_PRINTF(" | Device only supports exceptionally small textures (smaller than 512x512.)\n");
		}

		if((ms_D3DCaps.TextureOpCaps & D3DPTEXTURECAPS_SQUAREONLY)) {
			CONSOLE_LOG(" | Square-only textures.\n");
		} else {
			CONSOLE_LOG(" | Non-square textures.\n");
		}
		if((ms_D3DCaps.TextureOpCaps & D3DPTEXTURECAPS_POW2)) {
			if((ms_D3DCaps.TextureOpCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL)) {
				CONSOLE_LOG(" | Conditional non-pow2 texture size support.\n");
			} else  {
				CONSOLE_LOG(" | Textures must be a power of 2 in size.\n");
			}
		} else {
			CONSOLE_LOG(" | Textures do not need to be a power of 2 in size.\n");
		}

		// Extra requirements:
		if(  (ms_D3DCaps.MaxTextureBlendStages < 2) || 
			!(ms_D3DCaps.TextureOpCaps & D3DTOP_MODULATE) ||
			!(ms_D3DCaps.TextureOpCaps & D3DTOP_MODULATE2X) ) {
			bStatus = false;
		}
		if(ms_TextureFormat == D3DFMT_UNKNOWN) {
			CONSOLE_LOG(" | Couldn't find a valid texture format. *\n");
			bStatus = false;
		}
	}

	if(!bStatus) {
		CONSOLE_PRINTF(" | Video card not supported.\n");
		CONSOLE_PRINTF(" +-----------------------------------------------------------------------------\n");
		CONSOLE_PRINTF("FATAL ERROR (D3D9): Video card not supported, sorry.\n");
	} else {
		if(ms_nCount == 1) {
			CONSOLE_PRINTF(" | Running in %s color mode\n", GetFormat(ms_TextureFormat));
			CONSOLE_PRINTF(" | Supported video card.\n");
			CONSOLE_PRINTF(" +-----------------------------------------------------------------------------\n");
			if(!(ms_D3DCaps.TextureOpCaps & D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR)) {
				CONSOLE_PRINTF("ERROR (D3D9): Video card only partially supported. Only partial lightness is supported.\n");
			}
		}
		if(ms_pD3DDevice == NULL) {
			// Set the new screen mode:
			if(!SetModeB(hWnd, bWindowed, nScreenWidth, nScreenHeight)) return false;
		}

		m_bInitialized = true;
		PostInitialize(ms_nScreenWidth, ms_nScreenHeight);
	}

	return bStatus;
}

void CGraphicsD3D9::GetWorldRect(RECT *Rect_) const
{
	*Rect_ = m_RectWorld;
}
void CGraphicsD3D9::GetWorldPosition(POINT *Point_) const
{
	Point_->x = m_RectClip.left;
	Point_->y = m_RectClip.top;
}
void CGraphicsD3D9::SetWorldPosition(const POINT *Point_)
{
	HWND hWnd = m_hWnd;
	if(Point_) {
		OffsetRect(&m_RectView, -m_RectView.left, -m_RectView.top);
		//This offset is NOT exact, however this will probably not be noticable as long as SetWorldPosition is regularly updated.
		OffsetRect(&m_RectView, (int)((float)Point_->x*m_Zoom + 0.5f), (int)((float)Point_->y*m_Zoom + 0.4925f));

		RECT rcWorldView = {
			(int)((float)m_RectView.left / m_Zoom + 0.5f), 
			(int)((float)m_RectView.top / m_Zoom + 0.5f), 
			(int)((float)m_RectView.right / m_Zoom + 0.5f), 
			(int)((float)m_RectView.bottom / m_Zoom + 0.5f)
		};
		// We need the intersecting rectangle:
		::IntersectRect(&m_RectClip, &m_RectWorld, &rcWorldView);
	}

	D3DXMATRIX matTmp;
	D3DXMatrixScaling(&m_WorldMatrix, m_Zoom, m_Zoom, 1.0);
	D3DXMatrixTranslation(&matTmp, 
		-0.5f - (float)m_RectView.left + (float)(m_nFilterHorzMove<0?m_nFilterHorzMove:0) * m_Zoom, 
		-0.5f - (float)m_RectView.top + (float)(m_nFilterVertMove<0?m_nFilterVertMove:0) * m_Zoom, 
		0.0);
	m_WorldMatrix *= matTmp;
}

void CGraphicsD3D9::ViewToWorld(RECT *Rect_) const
{
	// This needs +0.5f adjustment??!??:
	::SetRect(Rect_,
		(int)((float)(Rect_->left + m_RectView.left) / m_Zoom + 0.5f),
		(int)((float)(Rect_->top + m_RectView.top) / m_Zoom + 0.5f),
		(int)((float)(Rect_->right + m_RectView.left) / m_Zoom + 0.5f),
		(int)((float)(Rect_->bottom + m_RectView.top) / m_Zoom + 0.5f)
	);
}

void CGraphicsD3D9::ViewToWorld(POINT *Point_) const
{
	// This needs +0.5f adjustment??!??:
	Point_->x = (int)((float)(Point_->x + m_RectView.left) / m_Zoom + 0.5f);
	Point_->y = (int)((float)(Point_->y + m_RectView.top) / m_Zoom + 0.5f);
}

void CGraphicsD3D9::WorldToView(POINT *Point_) const
{
	Point_->x = (int)((float)Point_->x * m_Zoom + 0.5f) - m_RectView.left;
	Point_->y = (int)((float)Point_->y * m_Zoom + 0.5f) - m_RectView.top;
	/*/
	Point_->x = (int)((float)Point_->x*m_Zoom)-m_RectView.left;
	Point_->y = (int)((float)Point_->y*m_Zoom)-m_RectView.top;
	/**/
}
void CGraphicsD3D9::WorldToView(RECT *Rect_) const
{
	::SetRect(Rect_,
		(int)((float)Rect_->left * m_Zoom + 0.5f) - m_RectView.left,
		(int)((float)Rect_->top * m_Zoom + 0.5f) - m_RectView.top,
		(int)((float)Rect_->right * m_Zoom + 0.5f) - m_RectView.left,
		(int)((float)Rect_->bottom * m_Zoom + 0.5f) - m_RectView.top
	);
	/*/
	::SetRect(Rect_,
		(int)((float)Rect_->left*m_Zoom)-m_RectView.left,
		(int)((float)Rect_->top*m_Zoom)-m_RectView.top,
		(int)((float)Rect_->right*m_Zoom)-m_RectView.left,
		(int)((float)Rect_->bottom*m_Zoom)-m_RectView.top
	);
	/**/
}

void CGraphicsD3D9::GetVisibleRect(RECT *Rect_) const
{
	*Rect_ = m_RectClip;

	Rect_->left -= m_nFilterHorzMove;
	Rect_->right -= m_nFilterHorzMove;
	Rect_->top -= m_nFilterVertMove;
	Rect_->bottom -= m_nFilterVertMove;
	::IntersectRect(Rect_, Rect_, &m_RectWorld);
}
float CGraphicsD3D9::GetCurrentZoom() const
{
	return m_Zoom;
}
bool CGraphicsD3D9::SetWindowView(HWND hWnd, float fZoom, const RECT *pClient, const RECT *pWorld)
{
	if(!m_bInitialized) return true;

	if(pClient) m_RectView = *pClient;

	if(pWorld) {
		if(!::EqualRect(&m_RectWorld, pWorld)) {
			m_RectWorld = *pWorld;
			delete [](m_pGrid[0]); m_pGrid[0] = NULL;
			delete [](m_pGrid[1]); m_pGrid[1] = NULL;
		}
	}
	if(m_Zoom != fZoom) {
		m_fFilterPixelate = m_fFilterPixelate * m_Zoom;
		m_Zoom = fZoom;
		m_fFilterPixelate = m_fFilterPixelate / m_Zoom;
		UpdateFilters();

		delete [](m_pGrid[0]); m_pGrid[0] = NULL;
		delete [](m_pGrid[1]); m_pGrid[1] = NULL;
	}

	SetWorldPosition(NULL);

	RECT rcWorldView = {
		(int)((float)m_RectView.left / m_Zoom + 0.5f), 
		(int)((float)m_RectView.top / m_Zoom + 0.5f), 
		(int)((float)m_RectView.right / m_Zoom + 0.5f), 
		(int)((float)m_RectView.bottom / m_Zoom + 0.5f)
	};
	// We need the intersecting rectangle:
	::IntersectRect(&m_RectClip, &m_RectWorld, &rcWorldView);

	m_hWnd = hWnd;
	return true;
}

bool CGraphicsD3D9::CreateTextureFromFile(LPCSTR filename, ITexture **texture, float scale) const
{
	HRESULT hr;

	if(!m_bInitialized) return false;
	ASSERT(ms_pD3DDevice);

	std::map<std::string, CTextureD3D9*> :: const_iterator texIter;
	texIter = ms_Textures.find(filename);
	if(texIter == ms_Textures.end()) {
		IDirect3DTexture9 *pTexture;
		D3DXIMAGE_INFO	imageInfo;
		if(FAILED(hr=D3DXCreateTextureFromFileEx(ms_pD3DDevice, filename, 
				0, 0, 1, 0, ms_TextureFormat, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_DEFAULT, 
				D3DCOLOR_ARGB(255,0,255,0), &imageInfo, NULL, &pTexture) )) {
			CONSOLE_PRINTF("WARNING (D3D9:%s): Couldn't create texture from '%s'.\n", DXGetErrorString(hr), filename);
			return false;
		}
		D3DSURFACE_DESC surfaceDesc;
		pTexture->GetLevelDesc(0, &surfaceDesc);
		if(surfaceDesc.Format != ms_TextureFormat) {
			CONSOLE_LOG("WARNING (D3D9): Texture created with a different format (%s)\n", GetFormat(surfaceDesc.Format));
			CONSOLE_LOG("imageInfo: %d x %d (%s)\n", imageInfo.Width, imageInfo.Height, GetFormat(imageInfo.Format));
			CONSOLE_LOG("surfaceDesc: %d x %d (%s)\n", surfaceDesc.Width, surfaceDesc.Height, GetFormat(surfaceDesc.Format));
		}

		CTextureD3D9 *pCTextureD3D9 = new CTextureD3D9(pTexture, imageInfo, surfaceDesc);
		pCTextureD3D9->SetScale(scale);
		ms_Textures.insert( pairTexture(filename, pCTextureD3D9) );
		*texture = pCTextureD3D9;
	} else {
		*texture = texIter->second;
	}
	return true;
}
bool CGraphicsD3D9::CreateTextureFromFileInMemory(LPCSTR filename, LPCVOID pSrcData, UINT SrcData, ITexture **texture, float scale) const
{
	HRESULT hr;

	if(!m_bInitialized) return false;
	ASSERT(ms_pD3DDevice);

	std::map<std::string, CTextureD3D9*> :: const_iterator texIter;
	texIter = ms_Textures.find(filename);
	if(texIter == ms_Textures.end()) {
		IDirect3DTexture9 *pTexture;
		D3DXIMAGE_INFO	imageInfo;
		if(FAILED(hr=D3DXCreateTextureFromFileInMemoryEx(ms_pD3DDevice, pSrcData, SrcData, 
				0, 0, 1, 0, ms_TextureFormat, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_DEFAULT, 
				D3DCOLOR_ARGB(255,0,255,0), &imageInfo, NULL, &pTexture) )) {
			CONSOLE_PRINTF("WARNING (D3D9:%s): Couldn't create texture from memory.\n", DXGetErrorString(hr));
			return false;
		}

		D3DSURFACE_DESC surfaceDesc;
		pTexture->GetLevelDesc(0, &surfaceDesc);
		if(surfaceDesc.Format != ms_TextureFormat) {
			CONSOLE_LOG("WARNING (D3D9): Texture created with an invalid format (%s)\n", GetFormat(surfaceDesc.Format));
			CONSOLE_LOG("imageInfo: %d x %d (%s)\n", imageInfo.Width, imageInfo.Height, GetFormat(imageInfo.Format));
			CONSOLE_LOG("surfaceDesc: %d x %d (%s)\n", surfaceDesc.Width, surfaceDesc.Height, GetFormat(surfaceDesc.Format));
		}
		CONSOLE_DEBUG("imageInfo: %d x %d (%s)\n", imageInfo.Width, imageInfo.Height, GetFormat(imageInfo.Format));
		CONSOLE_DEBUG("surfaceDesc: %d x %d (%s)\n", surfaceDesc.Width, surfaceDesc.Height, GetFormat(surfaceDesc.Format));

		CTextureD3D9 *pCTextureD3D9 = new CTextureD3D9(pTexture, imageInfo, surfaceDesc);
		pCTextureD3D9->SetScale(scale);
		ms_Textures.insert( pairTexture(filename, pCTextureD3D9) );
		*texture = pCTextureD3D9;
	} else {
		*texture = texIter->second;
	}
	return true;
}
bool CGraphicsD3D9::SetFilter(GpxFilters eFilter, void *vParam)
{
	float fAux;
	bool bRet = false;
	switch(eFilter) {
		case ClearFilters:
			// Filters initialization:
			m_fFilterPixelate = 1.0f / m_Zoom;
			m_bFilterPixelate = false;

			m_nFilterVertMove = 0;
			m_nFilterHorzMove = 0;
			m_dwFilterColor = COLOR_ARGB(255,128,128,128);
		case EnableFilters:
			bRet = m_bFilters;
			m_bFilters = (vParam!=NULL);
			UpdateFilters();
			return bRet;
		case Pixelate:
			fAux = *((float*)vParam);
			if(fAux > ms_nHelperHeight) fAux = (float)ms_nHelperHeight;
			else if(fAux<1) fAux = 1.0f;
			m_fFilterPixelate = 1.0f / (fAux * m_Zoom);
			if(fAux != 1.0f) m_bFilterPixelate = true;
			else m_bFilterPixelate = false;
			bRet = true;
			break;
		case Colorize:
		case Alpha:
			m_dwFilterColor = *((ARGBCOLOR*)vParam);
			bRet = true;
			break;
		case HorzMove:
			m_nFilterHorzMove = *((int*)vParam);
			bRet = true;
			break;
		case VertMove:
			m_nFilterVertMove = *((int*)vParam);
			bRet = true;
			break;
	}

	UpdateFilters();

	m_bFilters |= bRet;
	return bRet;
}

void CGraphicsD3D9::UpdateFilters()
{
	SetWorldPosition(NULL);

	RECT rcVisible;
	GetVisibleRect(&rcVisible);

	if(!m_bFilterPixelate) m_fFilterPixelate = 1.0f;
	else if(m_fFilterPixelate <= 0.0f) m_fFilterPixelate = 0.0f;
	else if(m_fFilterPixelate > 1.0f) m_fFilterPixelate = 1.0f;

	float x = (float)(rcVisible.right - rcVisible.left + 1) * m_Zoom;
	float y = (float)(rcVisible.bottom - rcVisible.top + 1) * m_Zoom;
	float u = m_fFilterPixelate * x / (float)(ms_nHelperWidth);
	float v = m_fFilterPixelate * y / (float)(ms_nHelperHeight);

	float fXDisp = ((float)(m_nFilterHorzMove>0?m_nFilterHorzMove:0) * m_Zoom);
	float fYDisp = ((float)(m_nFilterVertMove>0?m_nFilterVertMove:0) * m_Zoom);

	m_FiltersOverlay[0].x = fXDisp;
	m_FiltersOverlay[0].y = fYDisp;
	m_FiltersOverlay[0].z = 0.0f;
	m_FiltersOverlay[0].dwColor = m_dwFilterColor;
	m_FiltersOverlay[0].u = 0.0f;
	m_FiltersOverlay[0].v = 0.0f;

	m_FiltersOverlay[1].x = fXDisp;
	m_FiltersOverlay[1].y = y + fYDisp;
	m_FiltersOverlay[1].z = 0.0f;
	m_FiltersOverlay[1].dwColor = m_dwFilterColor;
	m_FiltersOverlay[1].u = 0.0f;
	m_FiltersOverlay[1].v = v;

	m_FiltersOverlay[2].x = x + fXDisp;
	m_FiltersOverlay[2].y = fYDisp;
	m_FiltersOverlay[2].z = 0.0f;
	m_FiltersOverlay[2].dwColor = m_dwFilterColor;
	m_FiltersOverlay[2].u = u;
	m_FiltersOverlay[2].v = 0.0f;

	m_FiltersOverlay[3].x = x + fXDisp;
	m_FiltersOverlay[3].y = y + fYDisp;
	m_FiltersOverlay[3].z = 0.0f;
	m_FiltersOverlay[3].dwColor = m_dwFilterColor;
	m_FiltersOverlay[3].u = u;
	m_FiltersOverlay[3].v = v;
}

bool CGraphicsD3D9::FlushFilters(bool bClear)
{
	if(!m_bFilters) return false;

	if(ms_bLastRendered == true) {
		ms_bLastRendered = false;
/*
		D3DVERIFY(ms_pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE));
		D3DVERIFY(ms_pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE));
		D3DVERIFY(ms_pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT));
/*/
		D3DVERIFY(ms_pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE));
		D3DVERIFY(ms_pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT));
		D3DVERIFY(ms_pD3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE));
		D3DVERIFY(ms_pD3DDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE));
		D3DVERIFY(ms_pD3DDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT));
/**/
	}
//	D3DVERIFY(ms_pD3DDevice->SetVertexShader(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1));
	D3DVERIFY(ms_pD3DDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1));

	if(ms_pHelperSurface && ms_pHelperSurface2 && m_fFilterPixelate != 1.0f) {
		D3DCDTVERTEX Pixelate[4] = {
			{0.0f, 0.0f, 0.0f, COLOR_ARGB(255,255,255,255), 0.0f, 0.0f},
			{0.0f, (float)(ms_nHelperHeight)*m_fFilterPixelate, 0.0f, COLOR_ARGB(255,255,255,255), 0.0f, 1.0f},
			{(float)(ms_nHelperWidth)*m_fFilterPixelate, 0.0f, 0.0f, COLOR_ARGB(255,255,255,255), 1.0f, 0.0f},
			{(float)(ms_nHelperWidth)*m_fFilterPixelate, (float)(ms_nHelperHeight)*m_fFilterPixelate, 0.0f, COLOR_ARGB(255,255,255,255), 1.0f, 1.0f}
		};

		// Set the Render Target back to the default one.
		if(FAILED(ms_pD3DDevice->SetRenderTarget(0, ms_pHelperSurface2))) {
			m_bFilters = false; // deactivate filters on error
			return false;
		}

		D3DVERIFY(ms_pD3DDevice->SetTransform(D3DTS_WORLD, &m_IdentityMatrix));
		D3DVERIFY(ms_pD3DDevice->SetTransform(D3DTS_TEXTURE0, &m_IdentityMatrix));

		D3DVERIFY(ms_pD3DDevice->SetTexture(0, ms_pHelperTexture));

		D3DVERIFY(ms_pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR));
		D3DVERIFY(ms_pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, (PVOID)Pixelate, sizeof(D3DCDTVERTEX)));
		D3DVERIFY(ms_pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT));

		D3DVERIFY(ms_pD3DDevice->SetTexture(0, ms_pHelperTexture2));
	} else {
		if(ms_pHelperSurface) {
			D3DVERIFY(ms_pD3DDevice->SetTexture(0, ms_pHelperTexture));
		} else {
			m_bFilters = false; // deactivate filters on error
			return false;
		}
	}

	// Set the Render Target back to the default one.
	if(FAILED(ms_pD3DDevice->SetRenderTarget(0, ms_pBackBufferSurface))) {
		m_bFilters = false; // deactivate filters on error
		return false;
	}

	// Clear the current target surface with the Filter Background Clear Color set by a SetFilterBkColor() call.
	if(bClear) Clear(NULL, m_rgbFilterBkColor);

	D3DXMATRIX matTmp;
	D3DXMatrixTranslation(&matTmp, -0.5f, -0.5f, 0.0);
	D3DVERIFY(ms_pD3DDevice->SetTransform(D3DTS_WORLD, &matTmp));
	D3DVERIFY(ms_pD3DDevice->SetTransform(D3DTS_TEXTURE0, &m_IdentityMatrix));

	D3DVERIFY(ms_pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE2X));
	D3DVERIFY(ms_pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, (PVOID)m_FiltersOverlay, sizeof(D3DCDTVERTEX)));
	D3DVERIFY(ms_pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE));

	return true;
}

int CGraphicsD3D9::Finalize()
{
	if(m_bInitialized) {
		ms_nCount--;
		m_bInitialized = false;
		CONSOLE_DEBUG("DEBUG: IGraphics finalized. (%d references)\n", ms_nCount);
	}
	if(ms_nCount == 0) {
		Invalidate(true);

		if(ms_pD3DDevice!=NULL) { ms_pD3DDevice->Release(); ms_pD3DDevice = NULL; }
		if(ms_pD3D!=NULL) { ms_pD3D->Release(); ms_pD3D = NULL; }
	}
	return ms_nCount;
}
bool CGraphicsD3D9::BeginPaint()
{
	HRESULT hr;

	if(!m_bInitialized) return false;
	if(m_bCapture) {
		m_WorldMatrix = m_OldWorldMatrix;
		m_RectClip = m_RectOldClip;
		m_bCapture = false;
	}

    if(FAILED(hr=ms_pD3DDevice->TestCooperativeLevel())) {
        // If the device was lost, do not render until we get it back
		if(hr == D3DERR_DEVICELOST) { return true; }

		// Check if the device needs to be reset.
		if(hr == D3DERR_DEVICENOTRESET) { if(Recover() == false) return false; }
	}

	// if there are any filters active:
	if(m_bFilters) {
		// Set the Render Target to the helper texture/surface.
		if(!ms_pHelperSurface || FAILED(ms_pD3DDevice->SetRenderTarget(0, ms_pHelperSurface))) {
			m_bFilters = false; // deactivate filters on error
		}
	}

	// Clear the current target surface with the Clear Color set by a SetClearColor() call.
	Clear(NULL, m_rgbClearColor);

	// Begin the scene:
	if(FAILED(ms_pD3DDevice->BeginScene())) return false;

	return true;
}

bool CGraphicsD3D9::DrawFrame(const RECT &RectClip, ARGBCOLOR rgbColor, ARGBCOLOR rgbBoundaries)
{
	if(!m_bInitialized) return false;
	ASSERT(ms_pD3DDevice);
	// we don't want zoom or translation for this
	D3DVERIFY(ms_pD3DDevice->SetTransform(D3DTS_WORLD, &m_IdentityMatrix));

	RECT Rect;
	if(rgbColor.rgbAlpha != 0) {
		//  We now need to clear the parts surroundint the view...
		//  +-------------+------+
		//  +             +      +
		//  +    View     +      +
		//  +             +  R2  +
		//  +-------------+      +
		//  +     R1      +      +
		//  +-------------+------+
		//  (see bellow)
		
		// First the rectangle exactly dow the view is cleared (R1):
		::SetRect(&Rect, 
			(int)((float)RectClip.left * m_Zoom + 0.5f), 
			(int)((float)RectClip.bottom * m_Zoom + 0.5f), 
			(int)((float)RectClip.right * m_Zoom + 0.5f), 
			m_RectView.bottom
		);
		/*/
		::SetRect(&Rect, (int)((float)RectClip.left*m_Zoom), (int)((float)RectClip.bottom*m_Zoom), (int)((float)RectClip.right*m_Zoom), m_RectView.bottom);
		/**/

		::IntersectRect(&Rect, &Rect, &m_RectView);
		::OffsetRect(&Rect, -m_RectView.left, -m_RectView.top);
		// if it isn't empty, then we actually clear it:
		if(!::IsRectEmpty(&Rect)) RenderFill(Rect, rgbColor); 

		// Next the rectangle at the right of the view is cleared (R2):
		::SetRect(&Rect, 
			(int)((float)RectClip.right * m_Zoom + 0.5f), 
			(int)((float)RectClip.top * m_Zoom + 0.5f), 
			m_RectView.right, 
			m_RectView.bottom
		);
		/*/
		::SetRect(&Rect, (int)((float)RectClip.right*m_Zoom), (int)((float)RectClip.top*m_Zoom), m_RectView.right, m_RectView.bottom);
		/**/

		::IntersectRect(&Rect, &Rect, &m_RectView);
		::OffsetRect(&Rect, -m_RectView.left, -m_RectView.top);
		// if it isn't empty, then we actually clear it:
		if(!::IsRectEmpty(&Rect)) RenderFill(Rect, rgbColor);
	}

	if(rgbBoundaries.rgbAlpha != 0) {
		::SetRect(&Rect, 
			(int)((float)RectClip.left * m_Zoom + 0.5f), 
			(int)((float)RectClip.top * m_Zoom + 0.5f), 
			(int)((float)RectClip.right * m_Zoom + 0.5f), 
			(int)((float)RectClip.bottom * m_Zoom + 0.5f)
		);
		/*/
		::SetRect(&Rect, (int)((float)RectClip.left*m_Zoom), (int)((float)RectClip.top*m_Zoom), (int)((float)RectClip.right*m_Zoom), (int)((float)RectClip.bottom*m_Zoom));
		/**/

		::OffsetRect(&Rect, -m_RectView.left, -m_RectView.top);
		if(!::IsRectEmpty(&Rect)) RenderRect(Rect, rgbBoundaries);
	}

	return true;
}
bool CGraphicsD3D9::EndPaint()
{
	if(!m_bInitialized) return false;
	ASSERT(ms_pD3DDevice);

	if(FAILED(ms_pD3DDevice->EndScene())) return false;

	if(ms_bWindowed) {
		RECT Rect;
		Rect.top = Rect.left = 0;
		Rect.right = m_RectView.right-m_RectView.left;
		Rect.bottom = m_RectView.bottom-m_RectView.top;
		if(FAILED(ms_pD3DDevice->Present(&Rect, &Rect, m_hWnd, NULL))) return false;
	} else {
		if(FAILED(ms_pD3DDevice->Present(NULL, NULL, NULL, NULL))) return false;
	}
	return true;
}

bool CGraphicsD3D9::BeginCapture(RECT *rectDesired, float zoom)
{
	HRESULT hr;

	if(!m_bInitialized) return false;
	if(zoom == 0) return false;
	if(m_bCapture==true) return false;

    if(FAILED(hr=ms_pD3DDevice->TestCooperativeLevel())) {
        // If the device was lost, do not render until we get it back
		if(hr == D3DERR_DEVICELOST) { return true; }

		// Check if the device needs to be reset.
		if(hr == D3DERR_DEVICENOTRESET) { if(Recover() == false) return false; }
	}

	int width = rectDesired->right - rectDesired->left;
	int height = rectDesired->bottom - rectDesired->top;

	if(width > ms_nScreenWidth)
		rectDesired->right = rectDesired->left + ms_nScreenWidth;

	if(height > ms_nScreenHeight)
		rectDesired->bottom = rectDesired->top + ms_nScreenHeight;

	Clear(NULL, m_rgbClearColor); // white background
	if(FAILED(ms_pD3DDevice->BeginScene())) return false;

	m_bCapture = true;
	m_OldWorldMatrix = m_WorldMatrix;
	m_RectOldClip = m_RectClip;

	m_RectClip = *rectDesired;
	m_RectClip.top = (int)((float)m_RectClip.top / zoom + 0.5f);
	m_RectClip.bottom = (int)((float)m_RectClip.bottom / zoom + 0.5f);
	m_RectClip.left = (int)((float)m_RectClip.left / zoom + 0.5f);
	m_RectClip.right = (int)((float)m_RectClip.right / zoom + 0.5f);

	D3DXMATRIX matTmp;
	D3DXMatrixScaling(&m_WorldMatrix, zoom, zoom, 1.0);
	D3DXMatrixTranslation(&matTmp, -0.5f-(float)(*rectDesired).left, -0.5f-(float)(*rectDesired).top, 0.0);
	m_WorldMatrix *= matTmp;

	return true;
}
// pDest must be on the last (lower) row. The data is filled from bottom to top (inverted bitmap)
bool CGraphicsD3D9::EndCapture(WORD *pData, RECT &rcPortion, RECT &rcFull)
{
	if(!m_bInitialized) return false;
	if(!m_bCapture) return false;
	ASSERT(ms_pD3DDevice);

	m_bCapture = false;

	if(FAILED(ms_pD3DDevice->EndScene())) {
		m_RectClip = m_RectOldClip;
		return false;
	}

	WORD biBitCount = GetBitCount();
	int nHeight = (rcPortion.bottom - rcPortion.top);
	int nWidth = (rcPortion.right - rcPortion.left);
	int nWidthBytes = nWidth * (biBitCount/8);

	BYTE *pTmpMem = (BYTE*)new char[nHeight*nWidthBytes]; // temporary memory

	// Capture "copy" to memory:
	IDirect3DSurface9 *pBack;
	D3DVERIFY(ms_pD3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBack));

	D3DLOCKED_RECT lockrect;
	D3DVERIFY(pBack->LockRect(&lockrect, NULL, D3DLOCK_READONLY));

	// copy the surface to a temporary memory (as fast as possible);
	BYTE *pSrc = (BYTE*)lockrect.pBits;
	if(lockrect.Pitch == nWidthBytes) {
		CopyMemory(pTmpMem, pSrc, nHeight*nWidthBytes);
	} else {
		BYTE *pDest = pTmpMem;
		for(int y=0; y<nHeight; y++) {
			CopyMemory(pDest, pSrc, nWidthBytes);
			pDest += nWidthBytes;
			pSrc += lockrect.Pitch;
		}
	}
	D3DVERIFY(pBack->UnlockRect());
	pBack->Release();

	// Convert captured memory to 16 bits and copy to the client's memory
	int nFullWidth = (rcFull.right - rcFull.left);

	// client's memory is to hold 16 bits bitmaps.
	WORD *pDest = pData + nFullWidth * (rcFull.bottom - rcPortion.top - 1) + (rcPortion.left - rcFull.left);
	pSrc = pTmpMem;

	for(int y=0; y<nHeight; y++) {
		for(int x=0; x<nWidth; x++) {
			pDest[x] = GetNext16ARGB(&pSrc);
		}
		pDest -= nFullWidth;
	}

	delete []pTmpMem;

	m_RectClip = m_RectOldClip;
	m_WorldMatrix = m_OldWorldMatrix;

	// This simply shows what's been captured on the screen (for testing purposes):
//	if(FAILED(ms_pD3DDevice->Present(NULL, NULL, m_hWnd, NULL))) return false;

	return true;
}

void CGraphicsD3D9::SetFont(LPCSTR lpFont, int CharHeight, ARGBCOLOR rgbColor, LONG Weight)
{
	ASSERT(ms_pD3DDevice);
	D3DXFONT_DESC font;

	if(m_pD3DFont) {
		m_pD3DFont->Release();
		m_pD3DFont = NULL;
	}
	ZeroMemory(&font, sizeof(D3DXFONT_DESC));
	strcpy(font.FaceName, lpFont);
	font.Height = -CharHeight;
	font.Weight = Weight;


	D3DXCreateFontIndirect(ms_pD3DDevice, &font, &m_pD3DFont);
	m_rgbFontColor = rgbColor;
}
void CGraphicsD3D9::DrawText(const POINT &pointDest, LPCSTR lpString, ...) const
{
	ASSERT(m_pD3DFont);

	char lpBuffer[200];
	va_list argptr;
	va_start(argptr, lpString);
	vsprintf(lpBuffer, lpString, argptr);
	va_end(argptr);

	RECT rect = { pointDest.x, pointDest.y, pointDest.x+300, pointDest.y+300 };
	m_pD3DFont->DrawTextA(NULL, lpBuffer, -1, &rect, 0, m_rgbFontColor);
}
void CGraphicsD3D9::DrawText(const POINT &pointDest, ARGBCOLOR rgbColor, LPCSTR lpString, ...) const
{
	ASSERT(m_pD3DFont);

	char lpBuffer[200];
	va_list argptr;
	va_start(argptr, lpString);
	vsprintf(lpBuffer, lpString, argptr);
	va_end(argptr);

	RECT rect = { pointDest.x, pointDest.y, pointDest.x+300, pointDest.y+300 };
	m_pD3DFont->DrawTextA(NULL, lpBuffer, -1, &rect, 0, rgbColor);
}

void CGraphicsD3D9::SetFilterBkColor(ARGBCOLOR rgbColor)
{
	m_rgbFilterBkColor = rgbColor;
	m_rgbClearColor = rgbColor;
}
void CGraphicsD3D9::SetClearColor(ARGBCOLOR rgbColor)
{
	m_rgbClearColor = rgbColor;
}

void CGraphicsD3D9::FillRect(const RECT &rectDest, ARGBCOLOR rgbColor) const
{
	D3DVERIFY(ms_pD3DDevice->SetTransform(D3DTS_WORLD, &m_WorldMatrix));
	RenderFill(rectDest, rgbColor);
}

void CGraphicsD3D9::DrawRect(const RECT &rectDest, ARGBCOLOR rgbColor, int bordersize) const
{
	D3DVERIFY(ms_pD3DDevice->SetTransform(D3DTS_WORLD, &m_WorldMatrix));

	RECT brc;
	// Draw top and left
	::SetRect(&brc, rectDest.left+bordersize, rectDest.top, rectDest.right-bordersize, rectDest.top + bordersize);
	RenderFill(brc, rgbColor);
	::SetRect(&brc, rectDest.left, rectDest.top, rectDest.left+bordersize, rectDest.bottom);
	RenderFill(brc, rgbColor);

	// Draw bottom and right
	::SetRect(&brc, rectDest.left+bordersize, rectDest.bottom-bordersize, rectDest.right-bordersize, rectDest.bottom);
	RenderFill(brc, rgbColor);
	::SetRect(&brc, rectDest.right-bordersize, rectDest.top, rectDest.right, rectDest.bottom);
	RenderFill(brc, rgbColor);
}

void CGraphicsD3D9::SelectingBox(const RECT &rectDest, ARGBCOLOR rgbColor) const
{
	// For now, this does the same as BoundingBox. (should be changed to dotted lines)
	RECT Rects = {
		(int)((float)rectDest.left * m_Zoom + 0.5f),
		(int)((float)rectDest.top * m_Zoom + 0.5f),
		(int)((float)rectDest.right * m_Zoom + 0.5f),
		(int)((float)rectDest.bottom * m_Zoom + 0.5f)
	};
	/*/
	RECT Rects;
	::SetRect(&Rects,
		(int)((float)rectDest.left * m_Zoom),
		(int)((float)rectDest.top * m_Zoom),
		(int)((float)rectDest.right * m_Zoom),
		(int)((float)rectDest.bottom * m_Zoom)
	);
	/**/

	// Offset the rectangle from world to view
	::OffsetRect(&Rects, -m_RectView.left, -m_RectView.top);

	if(m_Zoom<2) rgbColor.rgbAlpha = (int)((float)rgbColor.rgbAlpha * m_Zoom * 0.8f);

	// we don't want zoom or translation for this
	D3DVERIFY(ms_pD3DDevice->SetTransform(D3DTS_WORLD, &m_IdentityMatrix));

	//D3DLINEPATTERN Pattern;
	//Pattern.wRepeatFactor=1;
	//Pattern.wLinePattern=0xcccc;
	//D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_LINEPATTERN , *((DWORD*)&Pattern)));

	D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_INVDESTCOLOR));
	D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_DESTCOLOR));
	D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_SUBTRACT));

	RenderRect(Rects, rgbColor);

	D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
	D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));
	D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD));

//	Pattern.wRepeatFactor=0;
//	Pattern.wLinePattern=0;
//	D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_LINEPATTERN , *((DWORD*)&Pattern)));

}
void CGraphicsD3D9::UpdateGrid(int nGridSize, ARGBCOLOR rgbColor)
{
	static int s_nGridSize;
	static ARGBCOLOR s_rgbColor;

	if(m_Zoom<2) rgbColor.rgbAlpha = (int)((float)rgbColor.rgbAlpha * m_Zoom * 0.8f);
	int newAlpha = (int)((float)rgbColor.rgbAlpha * 1.8f);
	ARGBCOLOR rgbColor2 = rgbColor;
	rgbColor2.rgbAlpha = (newAlpha>255) ? 255 : newAlpha;

	// If either the grid does not exist or the grid size is different, the grid buffers are invalid:
	if(m_pGrid[0] == NULL || s_nGridSize != nGridSize) {
		BuildGrid(0, nGridSize, rgbColor);
		BuildGrid(1, nGridSize*2, rgbColor2); // second grid (lloks nicer with it)
		s_nGridSize = nGridSize;
		s_rgbColor = rgbColor;
		return;
	}

	// the grid buffer is still valid, so we check if it needs a color change:
	if( s_rgbColor == rgbColor ) return;

	for(int lines=0; lines<m_nGridLines[0]; lines++) {
		m_pGrid[0][lines].dwColor = rgbColor;
		m_pGrid[0][lines+1].dwColor = rgbColor;
	}

	for(int lines=0; lines<m_nGridLines[1]; lines++) {
		m_pGrid[1][lines].dwColor = rgbColor2;
		m_pGrid[1][lines+1].dwColor = rgbColor2;
	}

	s_rgbColor = rgbColor;
}
void CGraphicsD3D9::BuildGrid(int nGridIdx, int nGridSize, ARGBCOLOR rgbColor)
{
	int cols = 1 + (m_RectWorld.right - m_RectWorld.left + nGridSize - 1) / nGridSize;
	int rows = 1 + (m_RectWorld.bottom - m_RectWorld.top + nGridSize - 1) / nGridSize;
	m_nGridLines[nGridIdx] = rows+cols;

	delete [](m_pGrid[nGridIdx]);
	m_pGrid[nGridIdx] = new D3DCDVERTEX[m_nGridLines[nGridIdx]*2];

	int offset = 0;
	for(int y=0; y<rows; y++) {
		m_pGrid[nGridIdx][offset].x = 0;
		m_pGrid[nGridIdx][offset].y = (float)(y*nGridSize)*m_Zoom;
		m_pGrid[nGridIdx][offset].z = 1;
		m_pGrid[nGridIdx][offset].dwColor = rgbColor;
		m_pGrid[nGridIdx][offset+1].x = (float)(m_RectWorld.right-m_RectWorld.left)*m_Zoom;
		m_pGrid[nGridIdx][offset+1].y = (float)(y*nGridSize)*m_Zoom;
		m_pGrid[nGridIdx][offset+1].z = 1;
		m_pGrid[nGridIdx][offset+1].dwColor = rgbColor;
		offset += 2;
	}
	for(int x=0; x<cols; x++) {
		m_pGrid[nGridIdx][offset].x = (float)(x*nGridSize)*m_Zoom;
		m_pGrid[nGridIdx][offset].y = 0;
		m_pGrid[nGridIdx][offset].z = 1;
		m_pGrid[nGridIdx][offset].dwColor = rgbColor;
		m_pGrid[nGridIdx][offset+1].x = (float)(x*nGridSize)*m_Zoom;
		m_pGrid[nGridIdx][offset+1].y = (float)(m_RectWorld.bottom-m_RectWorld.top)*m_Zoom;
		m_pGrid[nGridIdx][offset+1].z = 1;
		m_pGrid[nGridIdx][offset+1].dwColor = rgbColor;
		offset += 2;
	}

}

bool CGraphicsD3D9::DrawGrid(int size, ARGBCOLOR rgbColor)
{
	if(!m_bInitialized) return false;
	ASSERT(ms_pD3DDevice);

	UpdateGrid(size, rgbColor);

	D3DVERIFY(ms_pD3DDevice->SetTexture(0, NULL));
//	D3DVERIFY(ms_pD3DDevice->SetVertexShader(D3DFVF_XYZ | D3DFVF_DIFFUSE));
	D3DVERIFY(ms_pD3DDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE));

	// we don't want zoom for this
	D3DXMATRIX matTmp;
	D3DXMatrixTranslation(&matTmp, -(float)m_RectView.left, -(float)m_RectView.top, 0.0);
	D3DVERIFY(ms_pD3DDevice->SetTransform(D3DTS_WORLD, &matTmp));

	if(ms_bLastRendered == true) {
		ms_bLastRendered = false;
		D3DVERIFY(ms_pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE));
		D3DVERIFY(ms_pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT));
		D3DVERIFY(ms_pD3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE));
		D3DVERIFY(ms_pD3DDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE));
		D3DVERIFY(ms_pD3DDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT));
	}

	//D3DLINEPATTERN Pattern;
	//Pattern.wRepeatFactor=2;
	//Pattern.wLinePattern=0xcccc;
	//D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_LINEPATTERN , *((DWORD*)&Pattern)));

	D3DVERIFY(ms_pD3DDevice->DrawPrimitiveUP(D3DPT_LINELIST, m_nGridLines[0], m_pGrid[0], sizeof(D3DCDVERTEX)));

	//Pattern.wRepeatFactor=1;
	//D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_LINEPATTERN , *((DWORD*)&Pattern)));

	D3DVERIFY(ms_pD3DDevice->DrawPrimitiveUP(D3DPT_LINELIST, m_nGridLines[1], m_pGrid[1], sizeof(D3DCDVERTEX)));

	//Pattern.wRepeatFactor=0;
	//Pattern.wLinePattern=0;
	//D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_LINEPATTERN , *((DWORD*)&Pattern)));

	return true;
}

void CGraphicsD3D9::BoundingBox(const RECT &rectDest, ARGBCOLOR rgbColor) const
{
	RECT Rects = {
		(int)((float)rectDest.left * m_Zoom + 0.5f),
		(int)((float)rectDest.top * m_Zoom + 0.5f),
		(int)((float)rectDest.right * m_Zoom + 0.5f),
		(int)((float)rectDest.bottom * m_Zoom + 0.5f)
	};
	/*/
	RECT Rects;
	::SetRect(&Rects,
		(int)((float)rectDest.left * m_Zoom),
		(int)((float)rectDest.top * m_Zoom),
		(int)((float)rectDest.right * m_Zoom),
		(int)((float)rectDest.bottom * m_Zoom)
	);
	/**/

	// Offset the rectangle from world to view
	::OffsetRect(&Rects, -m_RectView.left, -m_RectView.top);

	if(m_Zoom<2) rgbColor.rgbAlpha = (int)((float)rgbColor.rgbAlpha * m_Zoom * 0.8f);

	// we don't want zoom or translation for this
	D3DVERIFY(ms_pD3DDevice->SetTransform(D3DTS_WORLD, &m_IdentityMatrix));

	RenderRect(Rects, rgbColor);
}
void CGraphicsD3D9::SelectionBox(const RECT &rectDest, ARGBCOLOR rgbColor) const
{
	RECT Rects = {
		(int)((float)rectDest.left * m_Zoom + 0.5f),
		(int)((float)rectDest.top * m_Zoom + 0.5f),
		(int)((float)rectDest.right * m_Zoom + 0.5f),
		(int)((float)rectDest.bottom * m_Zoom + 0.5f)
	};

	/*/ // The method bellow (used before) was slower, and the new one seems a lot better:
	// Convert to inclusive-inclusive; inclusive-exclusive is keeped for any zoom beyond 100% since 
	// we want the bounding box on the lower-right side of the pixels. For those, we convert latter.
	RECT Rects;
	::SetRect(&Rects,
		(int)((float)rectDest.left * m_Zoom),
		(int)((float)rectDest.top * m_Zoom),
		(int)((float)(rectDest.right-1) * m_Zoom),
		(int)((float)(rectDest.bottom-1) * m_Zoom)
	);

	if(m_Zoom<1) {
		Rects.right++, Rects.bottom++; // Now we do the inclusive-inclusive conversion.
	} else {
		Rects.right += (int)m_Zoom;
		Rects.bottom += (int)m_Zoom;
	}/**/

	// Offset the rectangle from world to view
	::OffsetRect(&Rects, -m_RectView.left, -m_RectView.top);

	// This is just enhance the selecting box (cosmetics)
	int cbsz = 2;
	if(m_Zoom<1) {
		if(m_Zoom<1) rgbColor.rgbAlpha = (int)((float)rgbColor.rgbAlpha * m_Zoom / 0.5f);
		cbsz -= (int)( 2.0f / (m_Zoom * 10.0f));
	} else 	if(m_Zoom>1) {
		cbsz += (int)( 2.0f * (m_Zoom / 20.0f));
	}

	// we don't want zoom or translation for this
	D3DVERIFY(ms_pD3DDevice->SetTransform(D3DTS_WORLD, &m_IdentityMatrix));

	// Bounding box:
	RECT rcb = Rects;
	RenderRect(Rects, rgbColor);

	// rubber bands (Small boxes at the corners):
	::SetRect(&rcb, Rects.left-cbsz, Rects.top-cbsz, Rects.left+cbsz+1, Rects.top+cbsz+1);
	RenderFill(rcb, rgbColor);

	::SetRect(&rcb, Rects.left-cbsz, Rects.bottom-1-cbsz, Rects.left+cbsz+1, Rects.bottom-1+cbsz+1);
	RenderFill(rcb, rgbColor);

	::SetRect(&rcb, Rects.right-1-cbsz, Rects.top-cbsz, Rects.right-1+cbsz+1, Rects.top+cbsz+1);
	RenderFill(rcb, rgbColor);

	::SetRect(&rcb, Rects.right-1-cbsz, Rects.bottom-1-cbsz, Rects.right-1+cbsz+1, Rects.bottom-1+cbsz+1);
	RenderFill(rcb, rgbColor);

	// rubber bands (Small boxes in the middles):
	int my = Rects.top + (Rects.bottom-Rects.top)/2;
	int mx = Rects.left + (Rects.right-Rects.left)/2;

	::SetRect(&rcb, mx-cbsz, Rects.top-cbsz, mx+cbsz+1, Rects.top+cbsz+1);
	RenderFill(rcb, rgbColor);

	::SetRect(&rcb, mx-cbsz, Rects.bottom-1-cbsz, mx+cbsz+1, Rects.bottom-1+cbsz+1);
	RenderFill(rcb, rgbColor);

	::SetRect(&rcb, Rects.left-cbsz, my-cbsz, Rects.left+cbsz+1, my+cbsz+1);
	RenderFill(rcb, rgbColor);

	::SetRect(&rcb, Rects.right-1-cbsz, my-cbsz, Rects.right-1+cbsz+1, my+cbsz+1);
	RenderFill(rcb, rgbColor);
}


/*!
	\param texture Texture from which render the sprite.
	\param rectSrc Sprite's location in the texture (part of the texture to be rendered).
	\param rectDest Desired location of the rendered the sprite. 
		(this can be bigger than the sprite to render a tile)
	\param rotate Initial basic rotation of the sprite. 
		It can be GFX_ROTATE_0, GFX_ROTATE_90, GFX_ROTATE_180, or GFX_ROTATE_270.
	\param transform Transformations to apply to the sprite. 
		It can be GFX_NORMAL, GFX_MIRRORED, GFX_FLIPPED, or GFX_MIRRORED|GFX_FLIPPED.
	\param alpha Overall alpha blending value to render the sprite.
		The value can be between 0 (transparent) to 255 (solid).
	\param buffer Address of a virtual buffer to contain precalculated vertices.
	\param rotation Relative extra rotation of the sprite. 
		Initial basic rotation + relative rotation = Final rotation of the sprite.
	\param scale Relative extra scale factor for the sprite.

	\remarks This method renders a sprite or a sprites tile to the graphics
		device using Direct3D8. Since it does clipping on the
		sprites, manages transformations and rotations, and also has to manage 
		the tiling for tiled sprites, this method may be expensive to use.

*/
void CGraphicsD3D9::Render(
	const ITexture *texture, const RECT &rectSrc, 
	const RECT &rectDest, 
	int rotate, 
	int transform, 
	ARGBCOLOR rgbColor, 
	float lightness,
	IBuffer **buffer,
	float rotation,
	float scale
) const
{
	ASSERT(texture);
	if(!m_bInitialized) return;

	int srcWidth, srcHeight;
	if(rotate == GFX_ROTATE_90 || rotate == GFX_ROTATE_270) {
		srcWidth = rectSrc.bottom - rectSrc.top;
		srcHeight = rectSrc.right - rectSrc.left;
	}  else {
		srcWidth = rectSrc.right - rectSrc.left;
		srcHeight = rectSrc.bottom - rectSrc.top;
	}
	int destWidth = (rectDest.right-rectDest.left);
	int destHeight = (rectDest.bottom-rectDest.top);


	/////////////////////////////////////////////////////////
	SVertexBuffer *VertexBuffer = NULL;
	SVertexBuffer *retVertexBuffer = NULL;
	if(buffer) {
		if(*buffer) {
			if((*buffer)->GetDeviceID() != GetDeviceID()) (*buffer)->Invalidate(false);
			VertexBuffer = retVertexBuffer = static_cast<SVertexBuffer *>((*buffer)->GetBuffer());
		}
	}

	// create or update the vertex buffers:
	if(!VertexBuffer) {
		CreateVertexBuffer(&VertexBuffer, texture, rectSrc, rectDest, rotate, transform, rgbColor);
	} else if((static_cast<CBufferD3D9*>(*buffer))->isDirty() || VertexBuffer->m_rgbColor != rgbColor) {
		UpdateVertexBuffer(&VertexBuffer, texture, rectSrc, rectDest, rotate, transform, rgbColor);
	}
	/////////////////////////////////////////////////////////

	float trX = (float)((cRenderTable[transform][rotate].startLeft)?rectDest.left:rectDest.left+destWidth);
	float trY = (float)((cRenderTable[transform][rotate].startTop)?rectDest.top:rectDest.top+destHeight);

	// Draw the sprite
	D3DXMATRIX matrix, matCenter, matCenter2, matRo, matTr, matSc, matTexSc;

	D3DXVECTOR2 center;
	center.x = (float)(rectDest.right - rectDest.left) / 2;
	center.y = (float)(rectDest.bottom - rectDest.top) / 2;

	D3DXMatrixTranslation(&matCenter, -center.x, -center.y, 0);
	D3DXMatrixRotationZ(&matRo, -rotation);
	D3DXMatrixScaling(&matSc, scale, scale, 1.0f);
	D3DXMatrixTranslation(&matCenter2, center.x, center.y, 0);
	matrix = matCenter * matRo * matSc * matCenter2;

	D3DXMatrixRotationZ(&matRo, cRenderTable[transform][rotate].angle);
	D3DXMatrixScaling(&matSc, (transform&GFX_MIRRORED)?-1.0f:1.0f, (transform&GFX_FLIPPED)?-1.0f:1.0f, 1.0f);
	D3DXMatrixTranslation(&matTr, trX, trY, 0);
	matrix = matrix * matRo * matSc *  matTr * m_WorldMatrix;

	D3DVERIFY(ms_pD3DDevice->SetTransform(D3DTS_WORLD, &matrix));

	float fScale = 1/texture->GetScale();
	D3DXMatrixScaling(&matSc, fScale, fScale, 1.0f);
	D3DVERIFY(ms_pD3DDevice->SetTransform(D3DTS_TEXTURE0, &matSc));

	if( VertexBuffer->m_pVertices && VertexBuffer->m_dwVertices && VertexBuffer->m_dwPrimitives) {
		ASSERT(VertexBuffer->m_dwVertices <= ms_D3DCaps.MaxVertexIndex);
		ASSERT(VertexBuffer->m_dwPrimitives <= ms_D3DCaps.MaxPrimitiveCount);
		ASSERT((IDirect3DBaseTexture9*)texture->GetTexture());

		if(VertexBuffer->m_bOverflow) {
			D3DVERIFY(ms_pD3DDevice->SetTexture(0, NULL));
		} else {
			D3DVERIFY(ms_pD3DDevice->SetTexture(0, (IDirect3DBaseTexture9*)texture->GetTexture()));
		}

//		D3DVERIFY(ms_pD3DDevice->SetVertexShader(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1));
		D3DVERIFY(ms_pD3DDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1));

		// full lightness support:
		if(ms_D3DCaps.TextureOpCaps & D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR) {
			// Strider asked for this black to full white lightness system, but it doesn't work 
			// on all the video cards (doesn't work on the Intel 82815) why? 
			// > It can't do D3DTOP_MODULATEALPHA_ADDCOLOR
			// KingOfHeart repored the Intel 82815 bug on this (all sprites are shown grey)
			float lightnessAlpha;
			BYTE lightnessValue;
			if(lightness<0.5f) {
				lightnessAlpha = (2.0f * lightness);
				lightnessValue = 0;
			} else {
				lightnessAlpha = (2.0f - 2.0f * lightness);
				lightnessValue = (BYTE)((1.0f - lightnessAlpha) * 255.0f + 0.5f);
			}
			ARGBCOLOR rgbLightness = COLOR_ARGB(
				(BYTE)(lightnessAlpha * 255.0f + 0.5f),
				lightnessValue, lightnessValue, lightnessValue );

			D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_TEXTUREFACTOR, rgbLightness));
			if(ms_bLastRendered == false) {
				ms_bLastRendered = true;
				D3DVERIFY(ms_pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE2X));
				D3DVERIFY(ms_pD3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATEALPHA_ADDCOLOR));
				D3DVERIFY(ms_pD3DDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TFACTOR));
				D3DVERIFY(ms_pD3DDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT));
			}
		} else {
			// Alternative to the lightness (from black to lighter sprites)
			if(ms_bLastRendered == false) {
				ms_bLastRendered = true;
				D3DVERIFY(ms_pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE2X));
				D3DVERIFY(ms_pD3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE));
			}
		}
		D3DVERIFY(ms_pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, VertexBuffer->m_dwPrimitives, VertexBuffer->m_pVertices, sizeof(D3DCDTVERTEX)));
	}

	if(buffer) {
		if(!(*buffer)) {
			(*buffer) = new CBufferD3D9(VertexBuffer);
		} else if(!retVertexBuffer) (*buffer)->SetBuffer(VertexBuffer);
	} else {
		delete VertexBuffer;
	}
}
