/* QuestDesigner - Open Zelda's Project
   Copyright (C) 2003. Kronuz (Germán Méndez Bravo)
   Copyright (C) 2001-2003. Open Zelda's Project
 
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
/*! \file		GraphicsD3D8.cpp
	\brief		Implementation of the DirectGraphics 2D Graphics plugin for Open Zelda.
	\date		May 19, 2003
				June 9, 2003
	\author		Kronuz
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

*/

#include "stdafx.h"
#include "GraphicsD3D8.h"

//////////////////////////////////////////////////////////////////////////////
// Interface Version Definition:
const WORD IGraphics::Version = 0x0200;

//////////////////////////////////////////////////////////////////////////////
// Needed Libraries:
#pragma comment(lib, "d3d8.lib")
#pragma comment(lib, "d3dx8.lib")

//////////////////////////////////////////////////////////////////////////////
// Exported functions:

HRESULT QueryGfxInterface(WORD Version, IGraphics **pInterface, IConsole *Output)
{
	if( HIBYTE(Version) != HIBYTE(IGraphics::Version) ) return E_FAIL;
	if(	LOBYTE(Version) > LOBYTE(IGraphics::Version) ) return E_FAIL;

	if(!*pInterface) {
		// Convert our interface pointer to a CGraphicsD3D8 object
		*pInterface = new CGraphicsD3D8;
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
	if((*pInterface)->GetDeviceID() != D3D8_DEVICE_ID) {
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
// Interface implementation begins here:
D3DDISPLAYMODE CGraphicsD3D8::ms_PreferredMode;
D3DPRESENT_PARAMETERS CGraphicsD3D8::ms_d3dpp;
int CGraphicsD3D8::ms_nCount = 0;
int CGraphicsD3D8::ms_nScreenWidth = 0;
int CGraphicsD3D8::ms_nScreenHeight = 0;
bool CGraphicsD3D8::ms_bWindowed = true;
IDirect3D8 *CGraphicsD3D8::ms_pD3D = NULL;
IDirect3DDevice8 *CGraphicsD3D8::ms_pD3DDevice = NULL;
std::map<std::string, CTextureD3D8*> CGraphicsD3D8::ms_Textures;

#ifdef _USE_HWVB
std::vector<CBufferD3D8*> CGraphicsD3D8::ms_Buffers;
#endif

#ifdef _USE_SWAPCHAINS
HWND CGraphicsD3D8::ms_hWnd = 0;
std::map<IDirect3DSwapChain8**, D3DPRESENT_PARAMETERS*> CGraphicsD3D8::ms_SwapChains;
#endif

CBufferD3D8::CBufferD3D8(SVertexBuffer *pBuffer) : 
	m_pBuffer(pBuffer),
	m_nCount(0)
{
	CONSOLE_DEBUG4("DEBUG: IBuffer created.\n");
	AddRef();
}
CBufferD3D8::~CBufferD3D8() 
{
	CONSOLE_DEBUG4("DEBUG: IBuffer deleted.\n"); 
}

void CBufferD3D8::Touch() 
{
	m_bDirty = true;
}

void CBufferD3D8::Invalidate() 
{
	if(m_pBuffer) {
		delete m_pBuffer;
		CONSOLE_DEBUG4("DEBUG: Invalidating a buffer.\n"); 
	} else {
		CONSOLE_DEBUG4("DEBUG: Invalidating an invalid buffer!\n"); 
	}
	m_pBuffer = NULL; 
}
IBuffer* CBufferD3D8::AddRef() { 
	m_nCount++;
	CONSOLE_DEBUG3("DEBUG: IBuffer reference added. (%d references)\n", m_nCount); 
	return this; 
}
IBuffer* CBufferD3D8::Release() { 
	ASSERT(m_nCount>0); 
	CONSOLE_DEBUG3("DEBUG: IBuffer reference removed. (%d references)\n", m_nCount-1); 
	if(--m_nCount==0) { 
		CONSOLE_DEBUG4("DEBUG: Deleting texture.\n"); 
		delete this; 
	} 
	return NULL; 
}	

//////////////////////////////////////////////////////////////////////////////

CTextureD3D8::CTextureD3D8(IDirect3DTexture8 *pTexture, D3DXIMAGE_INFO imageInfo) : 
	m_pTexture(pTexture),
	m_ImageInfo(imageInfo),
	m_fScale(1.0f),
	m_nCount(0)
{
	D3DSURFACE_DESC Desc;
	m_pTexture->GetLevelDesc(0, &Desc);

	m_Width = Desc.Width;
	m_Height = Desc.Height;

	CONSOLE_DEBUG4("DEBUG: ITexture created.\n");
	AddRef();
}
CTextureD3D8::~CTextureD3D8() {
	CONSOLE_DEBUG4("DEBUG: ITexture deleted.\n"); 
}
void CTextureD3D8::Invalidate() { 
	if(m_pTexture) {
		m_pTexture->Release(); 
		CONSOLE_DEBUG3("DEBUG: Invalidating a texture.\n"); 
	} else {
		CONSOLE_DEBUG3("DEBUG: Invalidating an invalid texture!\n"); 
	}
	m_pTexture = NULL; 
}
ITexture* CTextureD3D8::AddRef() { 
	m_nCount++;
	CONSOLE_DEBUG3("DEBUG: ITexture reference added. (%d references)\n", m_nCount); 
	return this; 
}
ITexture* CTextureD3D8::Release() { 
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
inline void CGraphicsD3D8::Clear(const RECT *rect, ARGBCOLOR rgbColor) const
{
	if(!m_bInitialized) return;
	ASSERT(ms_pD3DDevice);

	D3DVERIFY(ms_pD3DDevice->SetTransform(D3DTS_WORLD, &m_WorldMatrix));
	if(rect) {
		D3DVERIFY(ms_pD3DDevice->Clear(1, (D3DRECT*)rect, D3DCLEAR_TARGET, rgbColor.dwColor, 1.0f, 0));
	} else {
		D3DVERIFY(ms_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, rgbColor.dwColor, 1.0f, 0));
	}
}

inline WORD CGraphicsD3D8::GetBitCount()
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

// using the current format, get the equivalent 16 bits ARGB values from a buffer in the 
// current format, and increment the pointer of the buffer to the next ARGB color.
inline WORD CGraphicsD3D8::GetNext16ARGB(BYTE **Data)
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
inline DWORD CGraphicsD3D8::FigureOutVertexProcessing()
{
	ASSERT(ms_pD3D);

	D3DCAPS8 D3DCaps;
	ms_pD3D->GetDeviceCaps(D3DADAPTER_DEFAULT, m_devType, &D3DCaps );

	// Set the device/format for SW vertex processing
	DWORD dwBehavior = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	// Confirm the device/format for HW vertex processing
	if(D3DCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) {
		if(D3DCaps.DevCaps & D3DDEVCAPS_PUREDEVICE ) {
			dwBehavior = D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE;
		} else {
			dwBehavior = D3DCREATE_HARDWARE_VERTEXPROCESSING;
		}
	}
	return dwBehavior;
}

// FigureOutDisplayMode Direct3D initialization
inline void CGraphicsD3D8::FigureOutDisplayMode(D3DFORMAT currentFormat, UINT width, UINT height) 
{
	UINT numModes = ms_pD3D->GetAdapterModeCount(D3DADAPTER_DEFAULT);

	D3DDISPLAYMODE bestMode;
	ZeroMemory(&bestMode,sizeof(D3DDISPLAYMODE));
	D3DDISPLAYMODE aMode;
	ZeroMemory(&aMode,sizeof(D3DDISPLAYMODE));

	D3DFORMAT fsFormats[8] = {
		D3DFMT_A8R8G8B8,	// 32 bit color with alpha
		D3DFMT_X8R8G8B8,	// 32 bit color without alpha
		D3DFMT_R8G8B8,		// 24 bit color
		D3DFMT_A4R4G4B4,	// 16 bit color with 4 bit alpha
		D3DFMT_A1R5G5B5,	// 16 bit color with 1 bit alpha
		D3DFMT_X1R5G5B5,	// 16 bit color without alpha
		D3DFMT_R5G6B5,		// 16 bit color with extra green
		currentFormat		// a good default because it should work
	};

	int aModeFormatNumber = 7;
	int bestModeFormatNumber = 7;

	for(UINT i=0; i<numModes; i++) {
		if(FAILED(ms_pD3D->EnumAdapterModes(D3DADAPTER_DEFAULT, i, &aMode))) return;

		if((aMode.Width == width) && (aMode.Height == height)) {
			for(aModeFormatNumber=0; aModeFormatNumber<8; aModeFormatNumber++) {
				if(aMode.Format == fsFormats[aModeFormatNumber]) break;
			}
			
			if(aModeFormatNumber <= bestModeFormatNumber) {
				if((aMode.RefreshRate > bestMode.RefreshRate) && (aMode.RefreshRate <= 120)){
					bestMode = aMode;
					bestModeFormatNumber = aModeFormatNumber;
				}
			}
		}
	}
	ms_PreferredMode = bestMode;
}
inline void CGraphicsD3D8::PostInitialize(UINT WindowWidth, UINT WindowHeight)
{
	D3DXMATRIX Ortho2D;	
	
	D3DXMatrixOrthoOffCenterLH(&Ortho2D, 0.0f, (float)WindowWidth, (float)WindowHeight, 0.0f, 0.0f , 1.0f);
	D3DXMatrixIdentity(&m_IdentityMatrix);

	D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_LIGHTING, FALSE)); // Disable Lighting
	D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE)); // Disable Culling

	D3DVERIFY(ms_pD3DDevice->SetTransform(D3DTS_PROJECTION, &Ortho2D));
	D3DVERIFY(ms_pD3DDevice->SetTransform(D3DTS_WORLD, &m_IdentityMatrix));
	D3DVERIFY(ms_pD3DDevice->SetTransform(D3DTS_VIEW, &m_IdentityMatrix));

	D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_LOCALVIEWER, FALSE));
	D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_ZENABLE, FALSE)); // Disable Z-Buffer

	D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,  TRUE));
	D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
	D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));
	D3DVERIFY(ms_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE));
/**/
	D3DVERIFY(ms_pD3DDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_POINT));
	D3DVERIFY(ms_pD3DDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_POINT));
	D3DVERIFY(ms_pD3DDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3));
}
// Receives inclusive-exclusive coordinates
inline void CGraphicsD3D8::RenderRect(const RECT &rectDest, ARGBCOLOR rgbColor) const
{
	float l = (float)(rectDest.left);
	float t = (float)(rectDest.top);
	float r = (float)(rectDest.right) - 1;
	float b = (float)(rectDest.bottom) - 1;

	D3DCDVERTEX rect[5] = {
		{ l, t, 0.0f, rgbColor.dwColor },
		{ l, b, 0.0f, rgbColor.dwColor },
		{ r, b, 0.0f, rgbColor.dwColor },
		{ r, t, 0.0f, rgbColor.dwColor },
		{ l, t, 0.0f, rgbColor.dwColor }
	};

	D3DVERIFY(ms_pD3DDevice->SetTexture(0, NULL));
	D3DVERIFY(ms_pD3DDevice->SetVertexShader(D3DFVF_XYZ | D3DFVF_DIFFUSE));

	D3DVERIFY(ms_pD3DDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, rect, sizeof(D3DCDVERTEX)));
}

// Receives inclusive-exclusive coordinates
inline void CGraphicsD3D8::RenderFill(const RECT &rectDest, ARGBCOLOR rgbColor) const
{
	float l = (float)(rectDest.left);
	float t = (float)(rectDest.top);
	float r = (float)(rectDest.right);
	float b = (float)(rectDest.bottom);

	D3DCDVERTEX rect[4] = {
		{ l, t, 0.0f, rgbColor.dwColor },
		{ l, b, 0.0f, rgbColor.dwColor },
		{ r, t, 0.0f, rgbColor.dwColor },
		{ r, b, 0.0f, rgbColor.dwColor }
	};

	D3DVERIFY(ms_pD3DDevice->SetTexture(0, NULL));
	D3DVERIFY(ms_pD3DDevice->SetVertexShader(D3DFVF_XYZ | D3DFVF_DIFFUSE));
	D3DVERIFY(ms_pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, rect, sizeof(D3DCDVERTEX)));
}
inline void CGraphicsD3D8::UpdateVertexBuffer(SVertexBuffer **vbuffer, const ITexture *texture, const RECT &rectSrc, const RECT &rectDest, int rotate, int transform, ARGBCOLOR rgbColor) const
{
	float invW = 1.0f/(float)texture->GetWidth();
	float invH = 1.0f/(float)texture->GetHeight();
	float udiff = (float)(rectSrc.left-(*vbuffer)->m_texLeft) * invW;;
	float vdiff = (float)(rectSrc.top-(*vbuffer)->m_texTop) * invH;

	D3DCDTVERTEX *tile = (*vbuffer)->m_pVertices;
	for(int offset=0; offset<(*vbuffer)->m_nVertices; offset++) {
		tile[offset].u += udiff;
		tile[offset].v += vdiff;
		tile[offset].dwColor = (D3DCOLOR)(rgbColor.dwColor);
	}
	(*vbuffer)->m_texTop = rectSrc.top;
	(*vbuffer)->m_texLeft = rectSrc.left;

#ifdef _USE_HWVB
	if(!(*vbuffer)->m_pD3DVB) {
		BuildHWVertexBuffer(*vbuffer);
	} else {
		BYTE *Ptr;
		if(SUCCEEDED((*vbuffer)->m_pD3DVB->Lock(0, 0, (BYTE**)&Ptr, 0))) {
			memcpy(Ptr, (*vbuffer)->m_pVertices, sizeof(D3DCDTVERTEX) * (*vbuffer)->m_nVertices);
			(*vbuffer)->m_pD3DVB->Unlock();
		}
	}
#endif

}

inline void CGraphicsD3D8::CreateVertexBuffer(SVertexBuffer **vbuffer, const ITexture *texture, const RECT &rectSrc, const RECT &rectDest, int rotate, int transform, ARGBCOLOR rgbColor) const
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
	int lastWidth = destWidth%srcWidth;
	int lastHeight = destHeight%srcHeight;

	if(lastWidth) xVert++;
	else lastWidth = srcWidth;
	if(lastHeight) yVert++;
	else lastHeight = srcHeight;

	// Calculate the exact bounds of the sprite inside the texture
	float invW = 1.0f/(float)texture->GetWidth();
	float invH = 1.0f/(float)texture->GetHeight();
	float tl = (float)rectSrc.left * invW;
	float tt = (float)rectSrc.top * invH;
	float tr = (float)rectSrc.right * invW;
	float tb = (float)rectSrc.bottom * invH;
	
	// Calculate the exact bounds for the last row and column on the mesh in the texture
	float tlr = (float)(rectSrc.left+lastWidth) * invW;
	float tlb = (float)(rectSrc.top+lastHeight) * invH;

	float vcx[6] = {0, 0, (float)srcWidth, (float)srcWidth, 0, (float)srcWidth};
	float vcy[6] = {0, (float)srcHeight, 0, 0, (float)srcHeight, (float)srcHeight};
	float vtx[6] = {tl,tl,tr,tr,tl,tr};
	float vty[6] = {tt,tb,tt,tt,tb,tb};

	D3DCDTVERTEX *tile = new D3DCDTVERTEX[(xVert)*(yVert)*6];
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
				tile[offset].dwColor = (D3DCOLOR)(rgbColor.dwColor);
				offset++;
			}
		}
	}

	*vbuffer = new SVertexBuffer(tile, (xVert)*(yVert)*6, xVert*yVert*2, rectSrc.top, rectSrc.left);
}

inline bool CGraphicsD3D8::Recover()
{
	CONSOLE_DEBUG1("DEBUG: Recovering device...\n"); 

#ifdef _USE_SWAPCHAINS
	HWND m_hWnd = ms_hWnd;
#endif
	if(!SetMode(m_hWnd, ms_bWindowed, ms_nScreenWidth, ms_nScreenHeight)) return false;

	return true;
}
inline void CGraphicsD3D8::Invalidate()
{
	// Invalidate and release all textures
	ITexture *pTexture;
	std::map<std::string, CTextureD3D8*>::const_iterator texIter = ms_Textures.begin();
	while(texIter != ms_Textures.end()) {
		pTexture = texIter->second;
		pTexture->Invalidate();
		pTexture->Release();
		texIter++;
	}
	ms_Textures.clear();

#ifdef _USE_HWVB
	// Release all hardware vertex buffers
	std::vector<CBufferD3D8*>::const_iterator buffIter = ms_Buffers.begin();
	while(buffIter != ms_Buffers.end()) {
		SVertexBuffer *VB = static_cast<SVertexBuffer*>((*buffIter)->GetBuffer());
		if(VB->m_pD3DVB) {
			VB->m_pD3DVB->Release();
			VB->m_pD3DVB = NULL;
		}
		buffIter++;
	}
#endif

	if(m_pD3DFont) {
		m_pD3DFont->Release();
		m_pD3DFont = NULL;
	}

#ifdef _USE_SWAPCHAINS
	// Release all swap chains:
	std::map<IDirect3DSwapChain8**, D3DPRESENT_PARAMETERS*>::const_iterator scIter = ms_SwapChains.begin();
	while(scIter != ms_SwapChains.end()) {
		if((*scIter->first)) {
			CONSOLE_DEBUG2("DEBUG: Invalidating a SwapChain for hWnd: %d.\n", m_d3dpp.hDeviceWindow);
			(*scIter->first)->Release();
			(*scIter->first) = NULL;
		}
		scIter++;
	}
#endif
}

CGraphicsD3D8::CGraphicsD3D8() :
	m_bInitialized(false),
	m_bCapture(false),
#ifdef _USE_SWAPCHAINS
	m_pSwapChain(NULL),
#else
	m_hWnd(NULL),
#endif
	m_Zoom(0.0f),
	m_rgbClearColor(0),
	m_rgbFontColor(0),
	m_pD3DFont(NULL)
{
	m_pGrid[0] = NULL;
	m_pGrid[1] = NULL;

	::SetRect(&m_RectView, 0, 0, 0, 0);
	::SetRect(&m_RectClip, 0, 0, 0, 0);
	::SetRect(&m_RectWorld, 0, 0, 0, 0);
	::SetRect(&m_RectOldClip, 0, 0, 0, 0);

#ifdef _USE_SWAPCHAINS
	ms_SwapChains.insert(pairSwapChain(&m_pSwapChain, &m_d3dpp));
#endif
}
CGraphicsD3D8::~CGraphicsD3D8()
{
	delete []m_pGrid[0];
	delete []m_pGrid[1];
	Finalize();
#ifdef _USE_SWAPCHAINS
	ms_SwapChains.erase(&m_pSwapChain);
#endif
}

bool CGraphicsD3D8::SetMode(HWND hWnd, bool bWindowed, int nScreenWidth, int nScreenHeight)
{
	Invalidate();

	if(FAILED(ms_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &ms_PreferredMode))) {
		CONSOLE_PRINTF("ERROR (D3D8): Couldn't receive the current display mode.\n");
		return false;
	}

	if(bWindowed) {
		ms_d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
		//ms_PreferredMode.Width = nScreenWidth;
		//ms_PreferredMode.Height = nScreenHeight;
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
	ms_d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER; // for screenshots

	if(ms_pD3DDevice) {
		// Reset the device
		if(FAILED(ms_pD3DDevice->Reset(&ms_d3dpp))) {
				CONSOLE_PRINTF("ERROR (D3D8): Couldn't reset the device.\n");
				return false;
		}
	} else {
		// Create the device
		if(FAILED( ms_pD3D->CreateDevice(D3DADAPTER_DEFAULT, m_devType, hWnd,
				FigureOutVertexProcessing(), &ms_d3dpp, &ms_pD3DDevice) )) { 
			CONSOLE_PRINTF("ERROR (D3D8): Couldn't create the device.\n");
			return false;
		}
	}
	
	SetFont("Arial", 12, COLOR_ARGB(255,255,255,255), FW_BOLD); // initialize a default font.

	ms_bWindowed = bWindowed;
#ifndef _USE_SWAPCHAINS
	ms_nScreenWidth = ms_PreferredMode.Width;
	ms_nScreenHeight = ms_PreferredMode.Height;
#else
	ms_hWnd = hWnd;
	ms_nScreenWidth = nScreenWidth;
	ms_nScreenHeight = nScreenHeight;
#endif

#ifndef _USE_SWAPCHAINS
	PostInitialize(ms_nScreenWidth, ms_nScreenHeight);
#endif
	return true;
}

// On windowed evironments, this function must receive a handler to the parent window (main frame) 
// and not to the child view that is requesting the graphics device.
bool CGraphicsD3D8::Initialize(HWND hWnd, bool bWindowed, int nScreenWidth, int nScreenHeight)
{
	m_devType = D3DDEVTYPE_HAL; // Hardware acceleration
	if(m_bInitialized) return true;

	if(ms_pD3D == NULL) {
		ms_pD3D = Direct3DCreate8(D3D_SDK_VERSION); // Create Direct3D
		if(ms_pD3D == NULL) {
			// This will display a error message in the console
			CONSOLE_PRINTF("ERROR (D3D8): Couldn't initialize Direct3D.\n");
			return false;
		}
	}
	if(ms_pD3DDevice == NULL) {
		if(!SetMode(hWnd, bWindowed, nScreenWidth, nScreenHeight)) return false;
	}

	ms_nCount++;
	m_bInitialized = true;
#ifndef _USE_SWAPCHAINS
	PostInitialize(ms_nScreenWidth, ms_nScreenHeight);
#endif

	if(ms_nCount==1) {
		// Get some information about the D3D Driver
		D3DCAPS8 D3DCaps;
		ZeroMemory(&D3DCaps, sizeof(D3DCAPS8)); // clear it
		D3DVERIFY(ms_pD3D->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &D3DCaps));

		D3DADAPTER_IDENTIFIER8 D3DAdapterID;
		D3DVERIFY(ms_pD3D->GetAdapterIdentifier(D3DADAPTER_DEFAULT, NULL, &D3DAdapterID));

		// Output driver info
		CONSOLE_PRINTF("Using Microsoft Direct3D Version 8.0 '%s'\n", D3DAdapterID.Driver);
		CONSOLE_PRINTF(" | Description: %s\n", D3DAdapterID.Description);
		CONSOLE_PRINTF(" | Version: %d.%d\n", LOWORD(D3DAdapterID.DriverVersion.HighPart), HIWORD(D3DAdapterID.DriverVersion.LowPart));
		CONSOLE_PRINTF(" | Acceleration: %s rasterization\n", (m_devType==D3DDEVTYPE_HAL)?"Hardware":"Software");
		CONSOLE_PRINTF(" | Max Texture Size: %dx%d\n", D3DCaps.MaxTextureWidth, D3DCaps.MaxTextureHeight);
		CONSOLE_DEBUG(" | Max Primitives & Vertices: %d/%d\n", D3DCaps.MaxVertexIndex, D3DCaps.MaxPrimitiveCount);
#ifdef _USE_SWAPCHAINS
		CONSOLE_PRINTF(" | Using SwapChains!\n");
#endif	

		if((D3DCaps.RasterCaps&D3DPRASTERCAPS_ANTIALIASEDGES)) {
			CONSOLE_PRINTF(" | Support for antialias on lines forming the convex outline of an object is available.\n");
		}
		if((D3DCaps.RasterCaps&D3DPRASTERCAPS_PAT)) {
			CONSOLE_PRINTF(" | Support for patterned drawing is available for this driver.\n");
		}

	}
	CONSOLE_DEBUG("DEBUG: IGraphics initialized. (%d references)\n", ms_nCount);

	return true;
}
void CGraphicsD3D8::GetWorldRect(RECT *Rect_) const
{
	::SetRect(Rect_,
		(int)((float)(Rect_->left+m_RectView.left)/m_Zoom),
		(int)((float)(Rect_->top+m_RectView.top)/m_Zoom),
		(int)((float)(Rect_->right+m_RectView.left)/m_Zoom),
		(int)((float)(Rect_->bottom+m_RectView.top)/m_Zoom)
	);
}

void CGraphicsD3D8::GetWorldPosition(POINT *Point_) const
{
	Point_->x = (int)((float)(Point_->x+m_RectView.left)/m_Zoom);
	Point_->y = (int)((float)(Point_->y+m_RectView.top)/m_Zoom);
}

void CGraphicsD3D8::GetViewPosition(POINT *Point_) const
{
	Point_->x = (int)((float)Point_->x*m_Zoom)-m_RectView.left;
	Point_->y = (int)((float)Point_->y*m_Zoom)-m_RectView.top;
}
void CGraphicsD3D8::GetViewRect(RECT *Rect_) const
{
	::SetRect(Rect_,
		(int)((float)Rect_->left*m_Zoom)-m_RectView.left,
		(int)((float)Rect_->top*m_Zoom)-m_RectView.top,
		(int)((float)Rect_->right*m_Zoom)-m_RectView.left,
		(int)((float)Rect_->bottom*m_Zoom)-m_RectView.top
	);
}

RECT CGraphicsD3D8::GetVisibleRect() const
{
	return m_RectClip;
}
float CGraphicsD3D8::GetCurrentZoom() const
{
	return m_Zoom;
}
#ifdef _USE_SWAPCHAINS
bool CGraphicsD3D8::BuildSwapChain()
{
	m_d3dpp.BackBufferFormat = ms_PreferredMode.Format; 

	if(FAILED(ms_pD3DDevice->CreateAdditionalSwapChain(&m_d3dpp, &m_pSwapChain))) {
		CONSOLE_PRINTF("ERROR (D3D8): Couldn't create additional swap chain (2).\n");
		return false;
	}
	CONSOLE_DEBUG2("DEBUG: SwapChain created for hWnd: %d.\n", m_d3dpp.hDeviceWindow);

	PostInitialize(m_d3dpp.BackBufferWidth, m_d3dpp.BackBufferHeight);
	return true;
}
#endif

bool CGraphicsD3D8::SetWindowView(HWND hWnd, const RECT &client, const RECT &world, float zoom)
{
	if(!m_bInitialized) return true;
	if(client.right-client.left == 0 || client.bottom-client.top == 0) return true;

	D3DXMATRIX matTmp;
	D3DXMatrixScaling(&m_WorldMatrix, zoom, zoom, 1.0);
	D3DXMatrixTranslation(&matTmp, -0.5f-(float)client.left, -0.5f-(float)client.top, 0.0);
	m_WorldMatrix *= matTmp;

	if(!::EqualRect(&m_RectWorld, &world) || m_Zoom!=zoom) {
		m_Zoom = zoom;
		m_RectWorld = world;
		delete [](m_pGrid[0]); m_pGrid[0] = NULL;
		delete [](m_pGrid[1]); m_pGrid[1] = NULL;
	}

	RECT rcWorldView;
	::SetRect(&rcWorldView, 
		(int)((float)client.left / m_Zoom), 
		(int)((float)client.top / m_Zoom), 
		(int)(((float)client.right + m_Zoom) / m_Zoom), 
		(int)(((float)client.bottom + m_Zoom) / m_Zoom));

	// We need the intersecting rectangle:
	::IntersectRect(&m_RectClip, &world, &rcWorldView);

	if( ms_bWindowed==false ||
		m_RectView.right-m_RectView.left == client.right-client.left &&
		m_RectView.bottom-m_RectView.top == client.bottom-client.top ) {
			m_RectView = client;
			return true;
	}

	m_RectView = client;

#ifdef _USE_SWAPCHAINS
	if(m_pSwapChain) {
		CONSOLE_DEBUG2("DEBUG: Invalidating a SwapChain for hWnd: %d.\n", m_d3dpp.hDeviceWindow);
		m_pSwapChain->Release(); 
		m_pSwapChain = NULL;
	}

	ZeroMemory(&m_d3dpp, sizeof(D3DPRESENT_PARAMETERS)); // clear it
	
	m_d3dpp.BackBufferWidth  = m_RectView.right-m_RectView.left;	// The back buffer width
	m_d3dpp.BackBufferHeight = m_RectView.bottom-m_RectView.top;	// The back buffer height

	m_d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;

	m_d3dpp.hDeviceWindow = hWnd;
	m_d3dpp.Windowed = ms_bWindowed;						// Set windowed mode
	m_d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;		// for screenshots

	return BuildSwapChain();
#else
	m_hWnd = hWnd;
	return true;
#endif
}

bool CGraphicsD3D8::CreateTextureFromFile(LPCSTR filename, ITexture **texture, float scale) const
{
	if(!m_bInitialized) return false;
	ASSERT(ms_pD3DDevice);

	std::map<std::string, CTextureD3D8*> :: const_iterator texIter;
	texIter = ms_Textures.find(filename);
	if(texIter == ms_Textures.end()) {
		IDirect3DTexture8 *pTexture;
		D3DXIMAGE_INFO	imageInfo;
		if(FAILED( D3DXCreateTextureFromFileEx(ms_pD3DDevice, filename, 
				0, 0, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_DEFAULT, 
				D3DCOLOR_ARGB(255,0,255,0), &imageInfo, NULL, &pTexture) )) {
			CONSOLE_PRINTF("WARNING (D3D8): Couldn't create texture from '%s'.\n", filename);
			return false;
		}

		CTextureD3D8 *pCTextureD3D8 = new CTextureD3D8(pTexture, imageInfo);
		pCTextureD3D8->SetScale(scale);
		ms_Textures.insert( pairTexture(filename, pCTextureD3D8) );
		*texture = pCTextureD3D8;
	} else {
		*texture = texIter->second;
	}
	return true;
}
bool CGraphicsD3D8::CreateTextureFromFileInMemory(LPCSTR filename, LPCVOID pSrcData, UINT SrcData, ITexture **texture, float scale) const
{
	if(!m_bInitialized) return false;
	ASSERT(ms_pD3DDevice);

	std::map<std::string, CTextureD3D8*> :: const_iterator texIter;
	texIter = ms_Textures.find(filename);
	if(texIter == ms_Textures.end()) {
		IDirect3DTexture8 *pTexture;
		D3DXIMAGE_INFO	imageInfo;
		if(FAILED( D3DXCreateTextureFromFileInMemoryEx(ms_pD3DDevice, pSrcData, SrcData, 
				0, 0, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_DEFAULT, 
				D3DCOLOR_ARGB(255,0,255,0), &imageInfo, NULL, &pTexture) )) {
			CONSOLE_PRINTF("WARNING (D3D8): Couldn't create texture from memory.\n");
			return false;
		}

		CTextureD3D8 *pCTextureD3D8 = new CTextureD3D8(pTexture, imageInfo);
		pCTextureD3D8->SetScale(scale);
		ms_Textures.insert( pairTexture(filename, pCTextureD3D8) );
		*texture = pCTextureD3D8;
	} else {
		*texture = texIter->second;
	}
	return true;
}
int CGraphicsD3D8::Finalize()
{
	if(m_bInitialized) {
		ms_nCount--;
		m_bInitialized = false;
		CONSOLE_DEBUG("DEBUG: IGraphics finalized. (%d references)\n", ms_nCount);
	}
#ifdef _USE_SWAPCHAINS
	if(m_pSwapChain) { m_pSwapChain->Release(); m_pSwapChain = NULL; }
#endif
	if(ms_nCount == 0) {
		Invalidate();

		if(ms_pD3DDevice!=NULL) { ms_pD3DDevice->Release(); ms_pD3DDevice = NULL; }
		if(ms_pD3D!=NULL) { ms_pD3D->Release(); ms_pD3D = NULL; }
	}
	return ms_nCount;
}
bool CGraphicsD3D8::BeginPaint()
{
	if(!m_bInitialized) return false;
	if(m_bCapture) {
		m_WorldMatrix = m_OldWorldMatrix;
		m_RectClip = m_RectOldClip;
		m_bCapture = false;
	}

	HRESULT hr;
    if(FAILED(hr=ms_pD3DDevice->TestCooperativeLevel())) {
        // If the device was lost, do not render until we get it back
		if(hr == D3DERR_DEVICELOST) { return true; }

		// Check if the device needs to be reset.
		if(hr == D3DERR_DEVICENOTRESET) { if(Recover() == false) return false; }
	}

#ifdef _USE_SWAPCHAINS
	if(ms_bWindowed) {
		if(m_pSwapChain == NULL) { if(!BuildSwapChain()) return false; }

		LPDIRECT3DSURFACE8 pBack = NULL;
		if(FAILED(m_pSwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pBack))) return false;

		ASSERT(pBack);
		if(FAILED(ms_pD3DDevice->SetRenderTarget(pBack, NULL))) {
			pBack->Release();
			return false;
		}

		pBack->Release();
	}
#endif
	Clear(NULL, m_rgbClearColor);

	if(FAILED(ms_pD3DDevice->BeginScene())) return false;

	return true;
}
bool CGraphicsD3D8::DrawFrame()
{
	if(!m_bInitialized) return false;
	ASSERT(ms_pD3DDevice);
#ifdef _USE_SWAPCHAINS
	ASSERT((m_pSwapChain || !ms_bWindowed));
#endif
	// Now we hide what's not needed
	DWORD color = GetSysColor(COLOR_APPWORKSPACE);
	// we don't want zoom or translation for this
	D3DVERIFY(ms_pD3DDevice->SetTransform(D3DTS_WORLD, &m_IdentityMatrix));


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
	RECT Rect;
	::SetRect(&Rect, (int)((float)m_RectClip.left*m_Zoom), (int)((float)m_RectClip.bottom*m_Zoom), (int)((float)m_RectClip.right*m_Zoom), m_RectView.bottom);
	::IntersectRect(&Rect, &Rect, &m_RectView);
	::OffsetRect(&Rect, -m_RectView.left, -m_RectView.top);
	if(!::IsRectEmpty(&Rect)) // if it isn't empty, then we actually clear it
		RenderFill(Rect, COLOR_ARGB(255, GetRValue(color), GetGValue(color), GetBValue(color)));

	// Next the rectangle at the right of the view is cleared (R2):
	::SetRect(&Rect, (int)((float)m_RectClip.right*m_Zoom), (int)((float)m_RectClip.top*m_Zoom), m_RectView.right, m_RectView.bottom);
	::IntersectRect(&Rect, &Rect, &m_RectView);
	::OffsetRect(&Rect, -m_RectView.left, -m_RectView.top);
	if(!::IsRectEmpty(&Rect)) 
		RenderFill(Rect, COLOR_ARGB(255, GetRValue(color), GetGValue(color), GetBValue(color)));

	return true;
}
bool CGraphicsD3D8::EndPaint()
{
	if(!m_bInitialized) return false;
	ASSERT(ms_pD3DDevice);
#ifdef _USE_SWAPCHAINS
	ASSERT((m_pSwapChain || !ms_bWindowed));
#endif

	if(FAILED(ms_pD3DDevice->EndScene())) return false;

	if(ms_bWindowed) {
#ifdef _USE_SWAPCHAINS
		if(FAILED(m_pSwapChain->Present(NULL, NULL, NULL, NULL))) return false;
#else
		RECT Rect;
		Rect.top = Rect.left = 0;
		Rect.right = m_RectView.right-m_RectView.left;
		Rect.bottom = m_RectView.bottom-m_RectView.top;
		if(FAILED(ms_pD3DDevice->Present(&Rect, &Rect, m_hWnd, NULL))) return false;
#endif
	} else {
		if(FAILED(ms_pD3DDevice->Present(NULL, NULL, NULL, NULL))) return false;
	}
	return true;
}

bool CGraphicsD3D8::BeginCapture(RECT *rectDesired, float zoom)
{
	if(!m_bInitialized) return false;
	if(zoom == 0) return false;
	if(m_bCapture==true) return false;

	HRESULT hr;
    if(FAILED(hr=ms_pD3DDevice->TestCooperativeLevel())) {
        // If the device was lost, do not render until we get it back
		if(hr == D3DERR_DEVICELOST) { return true; }

		// Check if the device needs to be reset.
		if(hr == D3DERR_DEVICENOTRESET) { if(Recover() == false) return false; }
	}

	int width = rectDesired->right - rectDesired->left;
	int height = rectDesired->bottom - rectDesired->top;

	if(width > ms_nScreenWidth)
		rectDesired->right = rectDesired->left + (int)((float)ms_nScreenWidth);

	if(height > ms_nScreenHeight)
		rectDesired->bottom = rectDesired->top + (int)((float)ms_nScreenHeight);

	Clear(NULL, m_rgbClearColor); // white background
	if(FAILED(ms_pD3DDevice->BeginScene())) return false;

	m_bCapture = true;
	m_OldWorldMatrix = m_WorldMatrix;
	m_RectOldClip = m_RectClip;

	m_RectClip = *rectDesired;
	m_RectClip.top = (int)((float)m_RectClip.top/zoom);
	m_RectClip.bottom = (int)((float)m_RectClip.bottom/zoom);
	m_RectClip.left = (int)((float)m_RectClip.left/zoom);
	m_RectClip.right = (int)((float)m_RectClip.right/zoom);

	D3DXMATRIX matTmp;
	D3DXMatrixScaling(&m_WorldMatrix, zoom, zoom, 1.0);
	D3DXMatrixTranslation(&matTmp, -0.5f-(float)(*rectDesired).left, -0.5f-(float)(*rectDesired).top, 0.0);
	m_WorldMatrix *= matTmp;

	return true;
}
// pDest must be on the last (lower) row. The data is filled from bottom to top (inverted bitmap)
bool CGraphicsD3D8::EndCapture(WORD *pData, RECT &rcPortion, RECT &rcFull)
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
	IDirect3DSurface8 *pBack;
	D3DVERIFY(ms_pD3DDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pBack));

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

	//if(FAILED(ms_pD3DDevice->Present(NULL, NULL, m_hWnd, NULL))) return false;

	return true;
}

void CGraphicsD3D8::SetFont(LPCSTR lpFont, int CharHeight, ARGBCOLOR rgbColor, LONG Weight)
{
	ASSERT(ms_pD3DDevice);
	LOGFONT lf;

	if(m_pD3DFont) {
		m_pD3DFont->Release();
		m_pD3DFont = NULL;
	}

	ZeroMemory(&lf, sizeof(LOGFONT));
	strcpy(lf.lfFaceName, lpFont);
	lf.lfHeight = -CharHeight;
	lf.lfWeight = Weight;

	D3DXCreateFontIndirect(ms_pD3DDevice, &lf, &m_pD3DFont);
	m_rgbFontColor = rgbColor;
}
void CGraphicsD3D8::DrawText(const POINT &pointDest, LPCSTR lpString, ...) const
{
	ASSERT(m_pD3DFont);

	char lpBuffer[200];
	va_list argptr;
	va_start(argptr, lpString);
	vsprintf(lpBuffer, lpString, argptr);
	va_end(argptr);

	RECT rect;
	::SetRect(&rect, pointDest.x, pointDest.y, pointDest.x+300, pointDest.y+300);
	m_pD3DFont->DrawTextA(lpBuffer, -1, &rect, 0, m_rgbFontColor.dwColor);
}
void CGraphicsD3D8::DrawText(const POINT &pointDest, ARGBCOLOR rgbColor, LPCSTR lpString, ...) const
{
	ASSERT(m_pD3DFont);

	char lpBuffer[200];
	va_list argptr;
	va_start(argptr, lpString);
	vsprintf(lpBuffer, lpString, argptr);
	va_end(argptr);

	RECT rect;
	::SetRect(&rect, pointDest.x, pointDest.y, pointDest.x+300, pointDest.y+300);
	m_pD3DFont->DrawTextA(lpBuffer, -1, &rect, 0, rgbColor.dwColor);
}

void CGraphicsD3D8::SetClearColor(ARGBCOLOR rgbColor)
{
	m_rgbClearColor = rgbColor;
}

void CGraphicsD3D8::FillRect(const RECT &rectDest, ARGBCOLOR rgbColor) const
{
	D3DVERIFY(ms_pD3DDevice->SetTransform(D3DTS_WORLD, &m_WorldMatrix));
	RenderFill(rectDest, rgbColor);
}

void CGraphicsD3D8::DrawRect(const RECT &rectDest, ARGBCOLOR rgbColor, int bordersize) const
{
	RECT brc;

	D3DVERIFY(ms_pD3DDevice->SetTransform(D3DTS_WORLD, &m_WorldMatrix));

	// Draw top and left
	SetRect(&brc, rectDest.left+bordersize, rectDest.top, rectDest.right-bordersize, rectDest.top + bordersize);
	RenderFill(brc, rgbColor);
	SetRect(&brc, rectDest.left, rectDest.top, rectDest.left+bordersize, rectDest.bottom);
	RenderFill(brc, rgbColor);

	// Draw bottom and right
	SetRect(&brc, rectDest.left+bordersize, rectDest.bottom-bordersize, rectDest.right-bordersize, rectDest.bottom);
	RenderFill(brc, rgbColor);
	SetRect(&brc, rectDest.right-bordersize, rectDest.top, rectDest.right, rectDest.bottom);
	RenderFill(brc, rgbColor);
}

void CGraphicsD3D8::SelectingBox(const RECT &rectDest, ARGBCOLOR rgbColor) const
{
	// For now, this does the same as BoundingBox. (should be changed to dotted lines)
	RECT Rects;
	::SetRect(&Rects,
		(int)((float)rectDest.left * m_Zoom),
		(int)((float)rectDest.top * m_Zoom),
		(int)((float)rectDest.right * m_Zoom),
		(int)((float)rectDest.bottom * m_Zoom)
	);

	// Offset the rectangle from world to view
	::OffsetRect(&Rects, -m_RectView.left, -m_RectView.top);

	if(m_Zoom<2) rgbColor.rgbAlpha = (int)((float)rgbColor.rgbAlpha * m_Zoom * 0.8f);

	// we don't want zoom or translation for this
	D3DVERIFY(ms_pD3DDevice->SetTransform(D3DTS_WORLD, &m_IdentityMatrix));

	D3DLINEPATTERN Pattern;
	Pattern.wRepeatFactor=1;
	Pattern.wLinePattern=0xcccc;
	D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_LINEPATTERN , *((DWORD*)&Pattern)));

	D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_INVDESTCOLOR));
	D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_DESTCOLOR));
	D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_SUBTRACT));

	RenderRect(Rects, rgbColor);

	D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
	D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));
	D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD));

	Pattern.wRepeatFactor=0;
	Pattern.wLinePattern=0;
	D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_LINEPATTERN , *((DWORD*)&Pattern)));

}
void CGraphicsD3D8::UpdateGrid(int nGridSize, ARGBCOLOR rgbColor)
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
	if( s_rgbColor.dwColor == rgbColor.dwColor) return;

	for(int lines=0; lines<m_nGridLines[0]; lines++) {
		m_pGrid[0][lines].dwColor = rgbColor.dwColor;
		m_pGrid[0][lines+1].dwColor = rgbColor.dwColor;
	}

	for(int lines=0; lines<m_nGridLines[1]; lines++) {
		m_pGrid[1][lines].dwColor = rgbColor2.dwColor;
		m_pGrid[1][lines+1].dwColor = rgbColor2.dwColor;
	}

	s_rgbColor = rgbColor;
}
void CGraphicsD3D8::BuildGrid(int nGridIdx, int nGridSize, ARGBCOLOR rgbColor)
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
		m_pGrid[nGridIdx][offset].dwColor = rgbColor.dwColor;
		m_pGrid[nGridIdx][offset+1].x = (float)(m_RectWorld.right-m_RectWorld.left)*m_Zoom;
		m_pGrid[nGridIdx][offset+1].y = (float)(y*nGridSize)*m_Zoom;
		m_pGrid[nGridIdx][offset+1].z = 1;
		m_pGrid[nGridIdx][offset+1].dwColor = rgbColor.dwColor;
		offset += 2;
	}
	for(int x=0; x<cols; x++) {
		m_pGrid[nGridIdx][offset].x = (float)(x*nGridSize)*m_Zoom;
		m_pGrid[nGridIdx][offset].y = 0;
		m_pGrid[nGridIdx][offset].z = 1;
		m_pGrid[nGridIdx][offset].dwColor = rgbColor.dwColor;
		m_pGrid[nGridIdx][offset+1].x = (float)(x*nGridSize)*m_Zoom;
		m_pGrid[nGridIdx][offset+1].y = (float)(m_RectWorld.bottom-m_RectWorld.top)*m_Zoom;
		m_pGrid[nGridIdx][offset+1].z = 1;
		m_pGrid[nGridIdx][offset+1].dwColor = rgbColor.dwColor;
		offset += 2;
	}

}

bool CGraphicsD3D8::DrawGrid(int size, ARGBCOLOR rgbColor)
{
	if(!m_bInitialized) return false;
	ASSERT(ms_pD3DDevice);
#ifdef _USE_SWAPCHAINS
	ASSERT((m_pSwapChain || !ms_bWindowed));
#endif

	UpdateGrid(size, rgbColor);

	D3DVERIFY(ms_pD3DDevice->SetTexture(0, NULL));
	D3DVERIFY(ms_pD3DDevice->SetVertexShader(D3DFVF_XYZ | D3DFVF_DIFFUSE));

	// we don't want zoom for this
	D3DXMATRIX matTmp;
	D3DXMatrixTranslation(&matTmp, -(float)m_RectView.left, -(float)m_RectView.top, 0.0);
	D3DVERIFY(ms_pD3DDevice->SetTransform(D3DTS_WORLD, &matTmp));

	D3DLINEPATTERN Pattern;
	Pattern.wRepeatFactor=2;
	Pattern.wLinePattern=0xcccc;
	D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_LINEPATTERN , *((DWORD*)&Pattern)));

	D3DVERIFY(ms_pD3DDevice->DrawPrimitiveUP(D3DPT_LINELIST, m_nGridLines[0], m_pGrid[0], sizeof(D3DCDVERTEX)));

	Pattern.wRepeatFactor=1;
	D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_LINEPATTERN , *((DWORD*)&Pattern)));

	D3DVERIFY(ms_pD3DDevice->DrawPrimitiveUP(D3DPT_LINELIST, m_nGridLines[1], m_pGrid[1], sizeof(D3DCDVERTEX)));

	Pattern.wRepeatFactor=0;
	Pattern.wLinePattern=0;
	D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_LINEPATTERN , *((DWORD*)&Pattern)));

	return true;
}

void CGraphicsD3D8::BoundingBox(const RECT &rectDest, ARGBCOLOR rgbColor) const
{
	RECT Rects;
	::SetRect(&Rects,
		(int)((float)rectDest.left * m_Zoom),
		(int)((float)rectDest.top * m_Zoom),
		(int)((float)rectDest.right * m_Zoom),
		(int)((float)rectDest.bottom * m_Zoom)
	);

	// Offset the rectangle from world to view
	::OffsetRect(&Rects, -m_RectView.left, -m_RectView.top);

	if(m_Zoom<2) rgbColor.rgbAlpha = (int)((float)rgbColor.rgbAlpha * m_Zoom * 0.8f);

	// we don't want zoom or translation for this
	D3DVERIFY(ms_pD3DDevice->SetTransform(D3DTS_WORLD, &m_IdentityMatrix));

	RenderRect(Rects, rgbColor);
}
void CGraphicsD3D8::SelectionBox(const RECT &rectDest, ARGBCOLOR rgbColor) const
{
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
	}

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
void CGraphicsD3D8::Render(
	const ITexture *texture, const RECT &rectSrc, 
	const RECT &rectDest, 
	int rotate, 
	int transform, 
	ARGBCOLOR rgbColor, 
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
			if((*buffer)->GetDeviceID() != GetDeviceID()) (*buffer)->Invalidate();
			VertexBuffer = retVertexBuffer = static_cast<SVertexBuffer *>((*buffer)->GetBuffer());
		}
	}

	// create or update the vertex buffers:
	if(!VertexBuffer) CreateVertexBuffer(&VertexBuffer, texture, rectSrc, rectDest, rotate, transform, rgbColor);
	else if((static_cast<CBufferD3D8*>(*buffer))->isDirty()) UpdateVertexBuffer(&VertexBuffer, texture, rectSrc, rectDest, rotate, transform, rgbColor);
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

	if(VertexBuffer->m_pVertices && VertexBuffer->m_nPrimitives) {
		ASSERT((IDirect3DBaseTexture8*)texture->GetTexture());
		D3DVERIFY(ms_pD3DDevice->SetTexture(0, (IDirect3DBaseTexture8*)texture->GetTexture()));
		D3DVERIFY(ms_pD3DDevice->SetVertexShader(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1));
#ifdef _USE_HWVB
		if(!VertexBuffer->m_pD3DVB) BuildHWVertexBuffer(VertexBuffer);
		if(VertexBuffer->m_pD3DVB) {
			D3DVERIFY(ms_pD3DDevice->SetStreamSource(0, VertexBuffer->m_pD3DVB, sizeof(D3DCDTVERTEX)));
			D3DVERIFY(ms_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, VertexBuffer->m_nPrimitives));
		}
#else
		D3DVERIFY(ms_pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, VertexBuffer->m_nPrimitives, VertexBuffer->m_pVertices, sizeof(D3DCDTVERTEX)));
#endif
	}

	if(buffer) {
		if(!(*buffer)) {
			(*buffer) = new CBufferD3D8(VertexBuffer);
#ifdef _USE_HWVB
			ms_Buffers.push_back(static_cast<CBufferD3D8*>(*buffer));
#endif
		} else if(!retVertexBuffer) (*buffer)->SetBuffer(VertexBuffer);
	} else {
		delete VertexBuffer;
	}
}

#ifdef _USE_HWVB
void CGraphicsD3D8::BuildHWVertexBuffer(SVertexBuffer *pVertexBuffer) const
{
	if(FAILED(ms_pD3DDevice->CreateVertexBuffer(
		sizeof(D3DCDTVERTEX) * pVertexBuffer->m_nVertices,
		D3DUSAGE_WRITEONLY,
		D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1,
		D3DPOOL_MANAGED,
		&(pVertexBuffer->m_pD3DVB) ))) return;

	BYTE *Ptr;
	if(SUCCEEDED(pVertexBuffer->m_pD3DVB->Lock(0, 0, (BYTE**)&Ptr, 0))) {
		memcpy(Ptr, pVertexBuffer->m_pVertices, sizeof(D3DCDTVERTEX) * pVertexBuffer->m_nVertices);
		pVertexBuffer->m_pD3DVB->Unlock();
	}
}
#endif