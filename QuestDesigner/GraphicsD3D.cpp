/* QuestDesigner - Open Zelda's Project
   Copyright (C) 2003 Kronuz
   Copyright (C) 2001/2003 Open Zelda's Project
 
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
/*! \file		GraphicsD3D.cpp
	\brief		Implementation of the Direct3D 8 graphics interface.
	\date		May 19, 2003

*/
#include "stdafx.h"

#include "GraphicsD3D.h"

int CGraphicsD3D8::ms_nCount = 0;
bool CGraphicsD3D8::ms_bWindowed = true;
IDirect3D8 *CGraphicsD3D8::ms_pD3D = NULL;
IDirect3DDevice8 *CGraphicsD3D8::ms_pD3DDevice = NULL;
CSimpleMap<CString, CTextureD3D8*> CGraphicsD3D8::ms_Textures;

CBufferD3D8::CBufferD3D8(SVertexBuffer *pBuffer) : 
	m_pBuffer(pBuffer),
	m_nCount(0) 
{
	D3DTRACE("DEBUG: IBuffer created.\n");
	AddRef();
}
CBufferD3D8::~CBufferD3D8() 
{
	D3DTRACE("DEBUG: IBuffer deleted.\n"); 
}

void CBufferD3D8::Invalidate() {
	if(m_pBuffer) {
		delete m_pBuffer;
		D3DTRACE("DEBUG: Invalidating a buffer.\n"); 
	} else {
		D3DTRACE("DEBUG: Invalidating an invalid buffer!\n"); 
	}
	m_pBuffer = NULL; 
}
IBuffer* CBufferD3D8::AddRef() { 
	m_nCount++;
	D3DTRACE("DEBUG: IBuffer reference added. (%d references)\n", m_nCount); 
	return this; 
}
IBuffer* CBufferD3D8::Release() { 
	ASSERT(m_nCount>0); 
	D3DTRACE("DEBUG: IBuffer reference removed. (%d references)\n", m_nCount-1); 
	if(--m_nCount==0) { 
		D3DTRACE("DEBUG: Deleting texture.\n"); 
		delete this; 
	} 
	return NULL; 
}	

//////////////////////////////////////////////////////////////////////////////

CTextureD3D8::CTextureD3D8(IDirect3DTexture8 *pTexture, D3DXIMAGE_INFO imageInfo) : 
	m_pTexture(pTexture),
	m_ImageInfo(imageInfo),
	m_nCount(0)
{
	D3DSURFACE_DESC Desc;
	m_pTexture->GetLevelDesc(0, &Desc);

	m_Width = Desc.Width;
	m_Height = Desc.Height;

	D3DTRACE("DEBUG: ITexture created.\n");
	AddRef();
}
CTextureD3D8::~CTextureD3D8() {
	D3DTRACE("DEBUG: ITexture deleted.\n"); 
}

void CTextureD3D8::Invalidate() { 
	if(m_pTexture) {
		m_pTexture->Release(); 
		D3DTRACE("DEBUG: Invalidating a texture.\n"); 
	} else {
		D3DTRACE("DEBUG: Invalidating an invalid texture!\n"); 
	}
	m_pTexture = NULL; 
}
ITexture* CTextureD3D8::AddRef() { 
	m_nCount++;
	D3DTRACE("DEBUG: ITexture reference added. (%d references)\n", m_nCount); 
	return this; 
}
ITexture* CTextureD3D8::Release() { 
	ASSERT(m_nCount>0); 
	D3DTRACE("DEBUG: ITexture reference removed. (%d references)\n", m_nCount-1); 
	if(--m_nCount==0) { 
		D3DTRACE("DEBUG: Deleting texture.\n"); 
		delete this; 
	} 
	return NULL; 
}

//////////////////////////////////////////////////////////////////////////////

CGraphicsD3D8::CGraphicsD3D8() :
	m_bInitialized(false),
	m_pSwapChain(NULL),
	m_RectView(0,0,0,0),
	m_RectClip(0,0,0,0),
	m_Zoom(0.0f)
{
}
CGraphicsD3D8::~CGraphicsD3D8()
{
	Finalize();
}

// Figure out vertex processing for Direct3D initialization
DWORD CGraphicsD3D8::FigureOutVertexProcessing()
{
	ASSERT(ms_pD3D);

	D3DCAPS8 d3dCaps;
	ms_pD3D->GetDeviceCaps(D3DADAPTER_DEFAULT, m_devType, &d3dCaps );

	// Set the device/format for SW vertex processing
	DWORD dwBehavior = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	// Confirm the device/format for HW vertex processing
	if(d3dCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) {
		if(d3dCaps.DevCaps & D3DDEVCAPS_PUREDEVICE ) {
			dwBehavior = D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE;
		} else {
			dwBehavior = D3DCREATE_HARDWARE_VERTEXPROCESSING;
		}
	}
	return dwBehavior;
}

// FigureOutDisplayMode Direct3D initialization
void CGraphicsD3D8::FigureOutDisplayMode(D3DFORMAT currentFormat, UINT width, UINT height) 
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
	m_PreferredMode = bestMode;
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
			printf("ERROR (D3D8): Couldn't initialize Direct3D.\n");
			return false;
		}
	}
	if(FAILED(ms_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &m_PreferredMode))) {
		printf("ERROR (D3D8): Couldn't receive the current display mode.\n");
		return false;
	}

	D3DPRESENT_PARAMETERS d3dpp; // Used to explain to Direct3D how it will present things on the screen
	ZeroMemory(&d3dpp, sizeof(D3DPRESENT_PARAMETERS)); // clear it
	
	if(bWindowed) {
	    d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
	} else {
		ASSERT(nScreenWidth && nScreenHeight);
		FigureOutDisplayMode(m_PreferredMode.Format, nScreenWidth, nScreenHeight);

		d3dpp.BackBufferCount = 1;
	    d3dpp.SwapEffect = D3DSWAPEFFECT_FLIP;
		d3dpp.FullScreen_RefreshRateInHz = m_PreferredMode.RefreshRate;
	}
	d3dpp.BackBufferWidth  = m_PreferredMode.Width;		// The back buffer width
	d3dpp.BackBufferHeight = m_PreferredMode.Height;	// The back buffer height
	d3dpp.BackBufferFormat = m_PreferredMode.Format; 

	d3dpp.hDeviceWindow = hWnd;							// Handle to the parent window
	d3dpp.Windowed = bWindowed;							// Set windowed mode
	d3dpp.EnableAutoDepthStencil = FALSE;

	if(ms_pD3DDevice == NULL) {
		// Create the device
		if(FAILED( ms_pD3D->CreateDevice(D3DADAPTER_DEFAULT, m_devType, hWnd,
				FigureOutVertexProcessing(), &d3dpp, &ms_pD3DDevice) )) { 
			printf("ERROR (D3D8): Couldn't create the device.\n");
			return false;
		}
	}

	ms_nCount++;
	m_bInitialized = true;
	ms_bWindowed = bWindowed;
	m_nScreenWidth = nScreenWidth;
	m_nScreenHeight = nScreenHeight;

	if(ms_nCount==1) {
		// Get some information about the D3D Driver
		D3DCAPS8 D3DCaps;
		D3DVERIFY(ms_pD3D->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &D3DCaps));

		D3DADAPTER_IDENTIFIER8 D3DAdapterID;
		D3DVERIFY(ms_pD3D->GetAdapterIdentifier(D3DADAPTER_DEFAULT, NULL, &D3DAdapterID));

		// Output driver info
		printf("Using Microsoft Direct3D Version 8.0 '%s'\n", D3DAdapterID.Driver);
	
		printf(" | Description: %s\n", D3DAdapterID.Description);
		printf(" | Version: %d.%d\n", LOWORD(D3DAdapterID.DriverVersion.HighPart), HIWORD(D3DAdapterID.DriverVersion.LowPart));
		printf(" | Acceleration: %s rasterization\n", (m_devType==D3DDEVTYPE_HAL)?"Hardware":"Software");
		printf(" | Max Texture Size: %dx%d\n", D3DCaps.MaxTextureWidth, D3DCaps.MaxTextureHeight);
	}
	D3DTRACE("DEBUG: IGraphics initialized. (%d references)\n", ms_nCount);

	return true;
}
void CGraphicsD3D8::PostInitialize(float WindowWidth, float WindowHeight)
{
	D3DXMATRIX Ortho2D;	
	
	D3DXMatrixOrthoOffCenterLH(&Ortho2D, 0.0f, WindowWidth, WindowHeight, 0.0f, 0.0f , 1.0f);
	D3DXMatrixIdentity(&IdentityMatrix);

	D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_LIGHTING, FALSE)); // Disable Lighting
	D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE)); // Disable Culling

	D3DVERIFY(ms_pD3DDevice->SetTransform(D3DTS_PROJECTION, &Ortho2D));
	D3DVERIFY(ms_pD3DDevice->SetTransform(D3DTS_WORLD, &IdentityMatrix));
	D3DVERIFY(ms_pD3DDevice->SetTransform(D3DTS_VIEW, &IdentityMatrix));

	D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_LOCALVIEWER, FALSE));
	D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_ZENABLE, FALSE)); // Disable Z-Buffer

	D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,  TRUE));
	D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
	D3DVERIFY(ms_pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));
	D3DVERIFY(ms_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE));
/**/
	D3DVERIFY(ms_pD3DDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_POINT));
	D3DVERIFY(ms_pD3DDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_POINT));
}
void CGraphicsD3D8::GetWorldRect(CRect &Rect_) const
{
	Rect_.SetRect(
		(int)((float)(Rect_.left+m_RectView.left)/m_Zoom),
		(int)((float)(Rect_.top+m_RectView.top)/m_Zoom),
		(int)((float)(Rect_.right+m_RectView.left)/m_Zoom),
		(int)((float)(Rect_.bottom+m_RectView.top)/m_Zoom) 
	);
}

void CGraphicsD3D8::GetWorldPosition(CPoint &Point_) const
{
	Point_.SetPoint(
		(int)((float)(Point_.x+m_RectView.left)/m_Zoom),
		(int)((float)(Point_.y+m_RectView.top)/m_Zoom) );
}

void CGraphicsD3D8::GetViewPosition(CPoint &Point_) const
{
	Point_.SetPoint(
		(int)((float)Point_.x*m_Zoom)-m_RectView.left,
		(int)((float)Point_.y*m_Zoom)-m_RectView.top );
}
void CGraphicsD3D8::GetViewRect(CRect &Rect_) const
{
	Rect_.SetRect(
		(int)((float)Rect_.left*m_Zoom)-m_RectView.left,
		(int)((float)Rect_.top*m_Zoom)-m_RectView.top,
		(int)((float)Rect_.right*m_Zoom)-m_RectView.left,
		(int)((float)Rect_.bottom*m_Zoom)-m_RectView.top
	);
}

RECT CGraphicsD3D8::GetVisibleRect()
{
	return this->m_RectClip;
}
float CGraphicsD3D8::GetCurrentZoom()
{
	return m_Zoom;
}
bool CGraphicsD3D8::SetWindowView(HWND hWnd, const RECT &client, const RECT &clipping, float zoom)
{
	if(!m_bInitialized) return true;
	if(client.right-client.left == 0 || client.bottom-client.top == 0) return true;

	D3DXMATRIX matTmp;
	D3DXMatrixScaling(&WorldMatrix, zoom, zoom, 1.0);
	D3DXMatrixTranslation(&matTmp, -0.5f-(float)client.left, -0.5f-(float)client.top, 0.0);
	WorldMatrix*=matTmp;

	m_RectClip = clipping;
	m_Zoom = zoom;

	CRect rcWorldView = client;
	rcWorldView.left /= m_Zoom;
	rcWorldView.top /= m_Zoom;
	rcWorldView.right = (int)(((float)rcWorldView.right + m_Zoom) / m_Zoom);
	rcWorldView.bottom = (int)(((float)rcWorldView.bottom + m_Zoom) / m_Zoom);
	m_RectClip.IntersectRect(m_RectClip, rcWorldView);

	if( !ms_bWindowed ||
		m_RectView.Width() == client.right-client.left &&
		m_RectView.Height() == client.bottom-client.top ) {
			m_RectView = client;
			return true;
	}

	if(m_pSwapChain) { m_pSwapChain->Release(); m_pSwapChain = NULL; }

	D3DDISPLAYMODE displayMode; // Structure to hold display mode info, like the resolution & refresh rate
	ms_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displayMode); 

	D3DPRESENT_PARAMETERS d3dpp; // Used to explain to Direct3D how it will present things on the screen
	ZeroMemory(&d3dpp, sizeof(D3DPRESENT_PARAMETERS)); // clear it
	
	d3dpp.BackBufferWidth  = client.right-client.left;	// The back buffer width
	d3dpp.BackBufferHeight = client.bottom-client.top;	// The back buffer height
	d3dpp.BackBufferFormat = displayMode.Format; 

	d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
	d3dpp.hDeviceWindow = hWnd;
	d3dpp.Windowed   = ms_bWindowed;						// Set windowed mode

	if(FAILED(ms_pD3DDevice->CreateAdditionalSwapChain(&d3dpp, &m_pSwapChain))) {
		printf("ERROR (D3D8): Couldn't create additional swap chain (2).\n");
		Finalize();
		return false;
	}

	PostInitialize((float)d3dpp.BackBufferWidth, (float)d3dpp.BackBufferHeight);

	m_RectView = client;
	return true;
}

bool CGraphicsD3D8::CreateTextureFromFile(LPCSTR filename, ITexture **texture)
{
	if(!m_bInitialized) return false;
	ASSERT(ms_pD3DDevice);

	int idx = ms_Textures.FindKey(filename);
	if(idx==-1) {
		IDirect3DTexture8 *pTexture;
		D3DXIMAGE_INFO	imageInfo;
		if(FAILED( D3DXCreateTextureFromFileEx(ms_pD3DDevice, filename, 
				0, 0, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_DEFAULT, 
				D3DCOLOR_ARGB(255,0,255,0), &imageInfo, NULL, &pTexture) )) {
			printf("ERROR (D3D8): Couldn't create texture from '%s'.\n", filename);
			return false;
		}

		CTextureD3D8 *pCTextureD3D8 = new CTextureD3D8(pTexture, imageInfo);
		ms_Textures.Add(filename, pCTextureD3D8);
		*texture = pCTextureD3D8;
	} else {
		*texture = ms_Textures.GetValueAt(idx);
	}
	return true;
}

void CGraphicsD3D8::Finalize()
{
	if(m_bInitialized) {
		ms_nCount--;
		m_bInitialized = false;
		D3DTRACE("DEBUG: IGraphics finalized. (%d references)\n", ms_nCount);
	}
	if(m_pSwapChain) { m_pSwapChain->Release(); m_pSwapChain = NULL; }
	if(ms_nCount == 0) {
		ITexture *pTexture;
		while(ms_Textures.GetSize()) {
			pTexture = ms_Textures.GetValueAt(0);
			ms_Textures.RemoveAt(0);
			pTexture->Invalidate();
			pTexture->Release();
		}
		if(ms_pD3DDevice!=NULL) { ms_pD3DDevice->Release(); ms_pD3DDevice = NULL; }
		if(ms_pD3D!=NULL) { ms_pD3D->Release(); ms_pD3D = NULL; }
	}
}

bool CGraphicsD3D8::BeginPaint()
{
	if(!m_bInitialized) return false;
	//ASSERT(m_pSwapChain || !ms_bWindowed);
	if(!m_pSwapChain) return false;

	if(ms_bWindowed) {
		LPDIRECT3DSURFACE8 pBack=NULL;
		if(FAILED(m_pSwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pBack))) return false;
		ASSERT(pBack);
		if(FAILED(ms_pD3DDevice->SetRenderTarget(pBack, NULL))) return false;
		pBack->Release();
	}

//	DWORD color = GetSysColor(COLOR_APPWORKSPACE);
//	Clear(NULL, 255, GetRValue(color), GetGValue(color), GetBValue(color));
	Clear(NULL, 255, 255, 0, 0);
	if(FAILED(ms_pD3DDevice->BeginScene())) return false;

	return true;
}
bool CGraphicsD3D8::EndPaint()
{
	if(!m_bInitialized) return false;
	ASSERT((m_pSwapChain || !ms_bWindowed) && ms_pD3DDevice);

	// Now we hide what's not needed
	DWORD color = GetSysColor(COLOR_APPWORKSPACE);
	// we don't want zoom or translation for this
	D3DVERIFY(ms_pD3DDevice->SetTransform(D3DTS_WORLD, &IdentityMatrix));

	CRect Rect(m_RectClip.left*m_Zoom, m_RectClip.bottom*m_Zoom, m_RectClip.right*m_Zoom, m_RectView.bottom);
	Rect.IntersectRect(Rect, m_RectView);
	Rect.OffsetRect(-m_RectView.left, -m_RectView.top);
	if(!Rect.IsRectEmpty()) 
		RenderFill(Rect, 255, GetRValue(color), GetGValue(color), GetBValue(color));

	Rect.SetRect(m_RectClip.right*m_Zoom, m_RectClip.top*m_Zoom, m_RectView.right, m_RectView.bottom);
	Rect.IntersectRect(Rect, m_RectView);
	Rect.OffsetRect(-m_RectView.left, -m_RectView.top);
	if(!Rect.IsRectEmpty()) 
		RenderFill(Rect, 255, GetRValue(color), GetGValue(color), GetBValue(color));

	if(FAILED(ms_pD3DDevice->EndScene())) return false;

	if(ms_bWindowed) {
		if(FAILED(m_pSwapChain->Present(NULL, NULL, NULL, NULL))) return false;
	} else {
		if(FAILED(ms_pD3DDevice->Present(NULL, NULL, NULL, NULL))) return false;
	}
	return true;
}

// Receives inclusive-exclusive coordinates
void CGraphicsD3D8::RenderRect(const RECT &rectDest, BYTE alpha, BYTE red, BYTE green, BYTE blue) const
{
	D3DCOLOR color = D3DCOLOR_ARGB(alpha, red, green, blue);
	float l = (float)(rectDest.left);
	float t = (float)(rectDest.top);
	float r = (float)(rectDest.right) - 1;
	float b = (float)(rectDest.bottom) - 1;

	D3DCDVERTEX rect[5] = {
		{ l, t, 0.0f, color },
		{ l, b, 0.0f, color },
		{ r, b, 0.0f, color },
		{ r, t, 0.0f, color },
		{ l, t, 0.0f, color }
	};

	D3DVERIFY(ms_pD3DDevice->SetTexture(0, NULL));
	D3DVERIFY(ms_pD3DDevice->SetVertexShader(D3DFVF_XYZ | D3DFVF_DIFFUSE));
	D3DVERIFY(ms_pD3DDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, rect, sizeof(D3DCDVERTEX)));
}

// Receives inclusive-exclusive coordinates
void CGraphicsD3D8::RenderFill(const RECT &rectDest, BYTE alpha, BYTE red, BYTE green, BYTE blue) const
{
	D3DCOLOR color = D3DCOLOR_ARGB(alpha, red, green, blue);
	float l = (float)(rectDest.left);
	float t = (float)(rectDest.top);
	float r = (float)(rectDest.right);
	float b = (float)(rectDest.bottom);

	D3DCDVERTEX rect[4] = {
		{ l, t, 0.0f, color },
		{ l, b, 0.0f, color },
		{ r, t, 0.0f, color },
		{ r, b, 0.0f, color }
	};

	D3DVERIFY(ms_pD3DDevice->SetTexture(0, NULL));
	D3DVERIFY(ms_pD3DDevice->SetVertexShader(D3DFVF_XYZ | D3DFVF_DIFFUSE));
	D3DVERIFY(ms_pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, rect, sizeof(D3DCDVERTEX)));
}

void CGraphicsD3D8::FillRect(const RECT &rectDest, BYTE alpha, BYTE red, BYTE green, BYTE blue) const
{
	D3DVERIFY(ms_pD3DDevice->SetTransform(D3DTS_WORLD, &WorldMatrix));
	RenderFill(rectDest, alpha,red,green,blue);
}

void CGraphicsD3D8::Clear(const RECT *rect, BYTE alpha, BYTE red, BYTE green, BYTE blue) const
{
	if(!m_bInitialized) return;
	ASSERT(ms_pD3DDevice);

	D3DVERIFY(ms_pD3DDevice->SetTransform(D3DTS_WORLD, &WorldMatrix));
	if(rect) {
		D3DVERIFY(ms_pD3DDevice->Clear(1, (D3DRECT*)rect, D3DCLEAR_TARGET, D3DCOLOR_ARGB(alpha, red, green, blue), 1.0f, 0));
	} else {
		D3DVERIFY(ms_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(alpha, red, green, blue), 1.0f, 0));
	}
}

void CGraphicsD3D8::DrawRect(const RECT &rectDest, BYTE alpha, BYTE red, BYTE green, BYTE blue, int bordersize) const
{
	RECT brc;

	D3DVERIFY(ms_pD3DDevice->SetTransform(D3DTS_WORLD, &WorldMatrix));

	// Draw top and left
	SetRect(&brc, rectDest.left+bordersize, rectDest.top, rectDest.right-bordersize, rectDest.top + bordersize);
	RenderFill(brc, alpha,red,green,blue);
	SetRect(&brc, rectDest.left, rectDest.top, rectDest.left+bordersize, rectDest.bottom);
	RenderFill(brc, alpha,red,green,blue);

	// Draw bottom and right
	SetRect(&brc, rectDest.left+bordersize, rectDest.bottom-bordersize, rectDest.right-bordersize, rectDest.bottom);
	RenderFill(brc, alpha,red,green,blue);
	SetRect(&brc, rectDest.right-bordersize, rectDest.top, rectDest.right, rectDest.bottom);
	RenderFill(brc, alpha,red,green,blue);
}

void CGraphicsD3D8::BoundingBox(const RECT &rectDest, BYTE alpha, BYTE red, BYTE green, BYTE blue) const
{
	CRect Rects = rectDest;
	Rects.left *= m_Zoom;
	Rects.right *= m_Zoom;
	Rects.top *= m_Zoom;
	Rects.bottom *= m_Zoom;

	// Offset the rectangle from world to view
	Rects.OffsetRect(-m_RectView.left, -m_RectView.top);

	if(m_Zoom<2) alpha *= ((float)m_Zoom * 0.8f);

	// we don't want zoom or translation for this
	D3DVERIFY(ms_pD3DDevice->SetTransform(D3DTS_WORLD, &IdentityMatrix));

	RenderRect(Rects, alpha, red, green, blue);
}
void CGraphicsD3D8::SelectionBox(const RECT &rectDest, BYTE alpha, BYTE red, BYTE green, BYTE blue) const
{
	CRect Rects = rectDest;
	
	// Convert to inclusive-inclusive; inclusive-exclusive is keeped for any zoom beyond 100% since 
	// we want the bounding box on the lower-right side of the pixels. For those, we convert latter.
	//if(m_Zoom<=1) Rects.right--, Rects.bottom--; 

	Rects.right--, Rects.bottom--; 

	Rects.left *= m_Zoom;
	Rects.right *= m_Zoom;
	Rects.top *= m_Zoom;
	Rects.bottom *= m_Zoom;

	if(m_Zoom<1) {
		Rects.right++, Rects.bottom++; // Now we do the inclusive-inclusive conversion.
	} else {
		Rects.right += m_Zoom;
		Rects.bottom += m_Zoom;
	}

	// Offset the rectangle from world to view
	Rects.OffsetRect(-m_RectView.left, -m_RectView.top);

	// This is just enhance the selecting box (cosmetics)
	int cbsz = 2;
	if(m_Zoom<1) {
		if(m_Zoom<1) alpha *= ((float)m_Zoom / 0.5f);
		cbsz -= ( 2.0f / (m_Zoom * 10.0f));
	} else 	if(m_Zoom>1) {
		cbsz += ( 2.0f * (m_Zoom / 20.0f));
	}

	// we don't want zoom or translation for this
	D3DVERIFY(ms_pD3DDevice->SetTransform(D3DTS_WORLD, &IdentityMatrix));

	CRect rcb = Rects;

	// Bounding box:
	RenderRect(Rects, alpha, red, green, blue);

	// Corners:
	rcb.SetRect(Rects.left-cbsz, Rects.top-cbsz, Rects.left+cbsz+1, Rects.top+cbsz+1);
	RenderFill(rcb, alpha, red, green, blue);

	rcb.SetRect(Rects.left-cbsz, Rects.bottom-1-cbsz, Rects.left+cbsz+1, Rects.bottom-1+cbsz+1);
	RenderFill(rcb, alpha, red, green, blue);

	rcb.SetRect(Rects.right-1-cbsz, Rects.top-cbsz, Rects.right-1+cbsz+1, Rects.top+cbsz+1);
	RenderFill(rcb, alpha, red, green, blue);

	rcb.SetRect(Rects.right-1-cbsz, Rects.bottom-1-cbsz, Rects.right-1+cbsz+1, Rects.bottom-1+cbsz+1);
	RenderFill(rcb, alpha, red, green, blue);

	// Middle:
	int my = Rects.top + Rects.Height()/2;
	int mx = Rects.left + Rects.Width()/2;

	rcb.SetRect(mx-cbsz, Rects.top-cbsz, mx+cbsz+1, Rects.top+cbsz+1);
	RenderFill(rcb, alpha, red, green, blue);

	rcb.SetRect(mx-cbsz, Rects.bottom-1-cbsz, mx+cbsz+1, Rects.bottom-1+cbsz+1);
	RenderFill(rcb, alpha, red, green, blue);

	rcb.SetRect(Rects.left-cbsz, my-cbsz, Rects.left+cbsz+1, my+cbsz+1);
	RenderFill(rcb, alpha, red, green, blue);

	rcb.SetRect(Rects.right-1-cbsz, my-cbsz, Rects.right-1+cbsz+1, my+cbsz+1);
	RenderFill(rcb, alpha, red, green, blue);
}
// ToDo: Probably this method needs to be rewriten to avoid the costs of validating tiles on the called Render
void CGraphicsD3D8::Render(const ITexture *texture, const RECT &rectSrc, const POINT &pointDest, int rotate, int transform, int alpha, IBuffer **buffer) const
{
	ASSERT(texture);

	RECT rectDest;
	rectDest.top = rectDest.bottom = pointDest.y;
	rectDest.left = rectDest.right = pointDest.x;
	Render(texture, rectSrc, rectDest, rotate, transform, alpha, buffer);
}

// This is the transformation table for the render function.
const struct STrans {
	float angle;
	bool startTop;
	bool startLeft;
} cRenderTable[4][4] = { // [transform][rotate]
	////////////////////////////////////////////////////////////////////////
	//   angle			 ST      SL
 	// Normal:
	{ {0.0f,			true,	true},		// SROTATE_0
	  {-D3DX_PI/2.0f,	false,	true},		// SROTATE_90
	  {-D3DX_PI,		false,	false},		// SROTATE_180
	  {D3DX_PI/2.0f,	true,	false} },	// SROTATE_270
	// Mirrored:
	{ {0.0f,			true,	false},		// SROTATE_0
	  {-D3DX_PI/2.0f,	true,	true},		// SROTATE_90
	  {-D3DX_PI,		false,	true},		// SROTATE_180
	  {D3DX_PI/2.0f,	false,	false} },	// SROTATE_270
	// Flipped:
	{ {0.0f,			false,	true},		// SROTATE_0
	  {-D3DX_PI/2.0f,	false,	false},		// SROTATE_90
	  {-D3DX_PI,		true,	false},		// SROTATE_180
	  {D3DX_PI/2.0f,	true,	true} },	// SROTATE_270
	// Mirrored/Flipped:
	{ {0.0f,			false,	false},		// SROTATE_0
	  {-D3DX_PI/2.0f,	true,	false},		// SROTATE_90
	  {-D3DX_PI,		true,	true},		// SROTATE_180
	  {D3DX_PI/2.0f,	false,	true} }		// SROTATE_270
};
void CGraphicsD3D8::UpdateVertexBuffer(SVertexBuffer **vbuffer, const ITexture *texture, const RECT &rectSrc, const RECT &rectDest, int rotate, int transform, int alpha) const
{
}
void CGraphicsD3D8::CreateVertexBuffer(SVertexBuffer **vbuffer, const ITexture *texture, const RECT &rectSrc, const RECT &rectDest, int rotate, int transform, int alpha) const
{
	ASSERT(vbuffer);

	int srcWidth, srcHeight;
	if(rotate == SROTATE_90 || rotate == SROTATE_270) {
		srcWidth = rectSrc.bottom - rectSrc.top;
		srcHeight = rectSrc.right - rectSrc.left;
	}  else {
		srcWidth = rectSrc.right - rectSrc.left;
		srcHeight = rectSrc.bottom - rectSrc.top;
	}

	int xVert, yVert, lastWidth, lastHeight;
	if(rectDest.left == rectDest.right || rectDest.top == rectDest.bottom) {
		xVert = 1;
		yVert = 1;
		lastWidth = srcWidth;
		lastHeight = srcHeight;
	} else {
		xVert = (rectDest.right-rectDest.left)/srcWidth;
		yVert = (rectDest.bottom-rectDest.top)/srcHeight;
		lastWidth = (rectDest.right-rectDest.left)-(xVert*srcWidth);
		lastHeight = (rectDest.bottom-rectDest.top)-(yVert*srcHeight);
		if(lastWidth) xVert++;
		else lastWidth = srcWidth;
		if(lastHeight) yVert++;
		else lastHeight = srcHeight;
	}

	float invW = 1.0f/(float)texture->GetWidth();
	float invH = 1.0f/(float)texture->GetHeight();
	float tl = (float)rectSrc.left * invW;
	float tt = (float)rectSrc.top * invH;
	float tr = (float)rectSrc.right * invW;
	float tb = (float)rectSrc.bottom * invH;
	
	float tlr, tlb;
	if(rotate == SROTATE_90 || rotate == SROTATE_270) {
		tlr = (float)(rectSrc.left+lastHeight) * invW;
		tlb = (float)(rectSrc.top+lastWidth) * invH;
	} else {
		tlr = (float)(rectSrc.left+lastWidth) * invW;
		tlb = (float)(rectSrc.top+lastHeight) * invH;
	}
	if(rectDest.left == rectDest.right || rectDest.top == rectDest.bottom) {
		lastWidth = rectSrc.right - rectSrc.left;
		lastHeight = rectSrc.bottom - rectSrc.top;
	}

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
				tile[offset].dwColor = D3DCOLOR_ARGB(alpha, 255, 255, 255);
				offset++;
			}
		}
	}

	*vbuffer = new SVertexBuffer(tile, (xVert)*(yVert)*6, xVert*yVert*2);
}

/*!
	\param texture Texture from which render the sprite.
	\param rectSrc Sprite's location in the texture (part of the texture to be rendered).
	\param rectDest Desired location of the rendered the sprite. 
		(this can be bigger than the sprite to render a tile)
	\param rotate Rotation of the sprite. 
		It can be SROTATE_0, SROTATE_90, SROTATE_180, or SROTATE_270.
	\param transform Transformations to apply to the sprite. 
		It can be SNORMAL, SMIRRORED, SFLIPPED, or SMIRRORED|SFLIPPED.
	\param alpha Overall alpha blending value to render the sprite.
		The value can be between 0 (transparent) to 255 (solid).
	\param vuffer Address of a virtual buffer to contain precalculated vertices.

	\remarks This method renders a sprite or a sprites tile to the graphics
		device using Direct3D8. Since it does clipping on the
		sprites, manages transformations and rotations, and also has to manage 
		the tiling for tiled sprites, this method may be expensive to use.

*/
void CGraphicsD3D8::Render(const ITexture *texture, const RECT &rectSrc, const RECT &rectDest, int rotate, int transform, int alpha, IBuffer **buffer) const
{
	ASSERT(texture);
	if(!m_bInitialized) return;

	int srcWidth, srcHeight;
	int destWidth, destHeight;
	if(rotate == SROTATE_90 || rotate == SROTATE_270) {
		srcWidth = rectSrc.bottom - rectSrc.top;
		srcHeight = rectSrc.right - rectSrc.left;
	}  else {
		srcWidth = rectSrc.right - rectSrc.left;
		srcHeight = rectSrc.bottom - rectSrc.top;
	}
	if(rectDest.left == rectDest.right || rectDest.top == rectDest.bottom) {
		destWidth = srcWidth;
		destHeight = srcHeight;
	} else {
		destWidth = rectDest.right - rectDest.left;
		destHeight = rectDest.bottom - rectDest.top;
	}

	/////////////////////////////////////////////////////////
	SVertexBuffer *VertexBuffer = NULL;
	SVertexBuffer *retVertexBuffer = NULL;
	if(buffer) {
		if(*buffer) {
			if((*buffer)->GetDeviceID() != GetDeviceID()) (*buffer)->Invalidate();
			VertexBuffer = retVertexBuffer = static_cast<SVertexBuffer *>((*buffer)->GetBuffer());
		}
	}
	if(!VertexBuffer) CreateVertexBuffer(&VertexBuffer, texture, rectSrc, rectDest, rotate, transform, alpha);
	else UpdateVertexBuffer(&VertexBuffer, texture, rectSrc, rectDest, rotate, transform, alpha);
	/////////////////////////////////////////////////////////

	float trX = (float)((cRenderTable[transform][rotate].startLeft)?rectDest.left:rectDest.left+destWidth);
	float trY = (float)((cRenderTable[transform][rotate].startTop)?rectDest.top:rectDest.top+destHeight);

	// Draw the sprite
	D3DXMATRIX matrix, matRo, matTr, matSc;
	D3DXMatrixScaling(&matSc, (transform&SMIRRORED)?-1.0f:1.0f, (transform&SFLIPPED)?-1.0f:1.0f, 1.0f);
	D3DXMatrixTranslation(&matTr, trX, trY, 0);
	D3DXMatrixRotationZ(&matRo, cRenderTable[transform][rotate].angle);

	matrix = matSc * matRo *  matTr * WorldMatrix;
	D3DVERIFY(ms_pD3DDevice->SetTransform(D3DTS_WORLD, &matrix));

	D3DVERIFY(ms_pD3DDevice->SetTexture(0, (IDirect3DBaseTexture8*)texture->GetTexture()));
	D3DVERIFY(ms_pD3DDevice->SetVertexShader(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1));
	D3DVERIFY(ms_pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, VertexBuffer->m_nPrimitives, VertexBuffer->m_pVertices, sizeof(D3DCDTVERTEX)));

	if(buffer) {
		if(!(*buffer)) (*buffer) = new CBufferD3D8(VertexBuffer);
		else if(!retVertexBuffer) (*buffer)->SetBuffer(VertexBuffer);
	} else {
		delete VertexBuffer;
	}
}
