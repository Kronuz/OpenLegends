/* QuestDesigner - Open Legends's Project
   Copyright (C) 2003-2004. Germán Méndez Bravo (Kronuz)
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
/*! \file		GraphicsD3D9.h
	\brief		Interface for the DirectGraphics 2D Graphics plugin for Open Legends.
	\date		May 19, 2003
				June 9, 2003
	\author		Kronuz
*/

#pragma once

#include <map>
#include <vector>
#include <string>

#include "../IGraphics.h"

#include <D3d9.h>
#include <D3d9types.h>
#include <D3dx9core.h>
#include <D3dx9tex.h>

#define D3D9_DEVICE_ID 0x95a67b92

BOOL WINAPI DllEntryPoint(HINSTANCE hDll, DWORD fdwReason, LPVOID lpvRserved);

typedef struct _D3DCDTVERTEX {
    FLOAT x, y, z;
    DWORD dwColor;
    FLOAT u, v;
} D3DCDTVERTEX, *LPD3DCDTVERTEX;

typedef struct _D3DCDVERTEX {
	FLOAT x, y, z;
	DWORD dwColor;
} D3DCDVERTEX, *LPD3DCDVERTEX;

/*! 
	This struct contains the vertex buffer (precalculated cache) used by the D3D9 2D engine to
	draw the sprites faster on the screen.
*/
struct SVertexBuffer {
	bool m_bOverflow;
	ARGBCOLOR m_rgbColor; // rgb color used for the vertices
	D3DCDTVERTEX *m_pVertices;
	DWORD m_dwVertices;
	DWORD m_dwPrimitives;
	int m_texTop;
	int m_texLeft;
	SVertexBuffer(ARGBCOLOR rgbColor_, D3DCDTVERTEX *pVertices_, DWORD dwVertices_, DWORD dwPrmitives_, int texTop_, int texLeft_, bool bOverflow_) : 
		m_bOverflow(bOverflow_),
		m_rgbColor(rgbColor_),
		m_pVertices(pVertices_), m_dwVertices(dwVertices_), m_dwPrimitives(dwPrmitives_),
		m_texTop(texTop_), m_texLeft(texLeft_) {}
	~SVertexBuffer() {
		delete []m_pVertices;
	}
};

class CBufferD3D9 :
	public IBuffer
{
	int m_nCount;
	bool m_bDirty;
	SVertexBuffer *m_pBuffer;
public:
	CBufferD3D9(SVertexBuffer *pBuffer);
	~CBufferD3D9();

	void *GetBuffer() const { return (void*)m_pBuffer; }
	bool SetBuffer(void *pBuffer) { if(!m_pBuffer) { m_pBuffer = (SVertexBuffer*)pBuffer; return true; } return false; }

	DWORD GetDeviceID() const { return D3D9_DEVICE_ID; }

	void Invalidate(bool full);
	void Touch();

	IBuffer* AddRef();
	IBuffer* Release();

	inline bool isDirty() { return m_bDirty; }
};

class CTextureD3D9 :
	public ITexture
{
	int m_nCount;
	IDirect3DTexture9 *m_pTexture;
	D3DXIMAGE_INFO	m_ImageInfo;
	UINT m_Width;
	UINT m_Height;
	float m_fScale;
public:

	CTextureD3D9(IDirect3DTexture9 *pTexture, D3DXIMAGE_INFO &imageInfo, D3DSURFACE_DESC &surfaceDesc);
	~CTextureD3D9();

	void *GetTexture() const { return (void*)m_pTexture; }
	bool SetTexture(void *pTexture) { if(!m_pTexture) { m_pTexture = (IDirect3DTexture9*)pTexture; return true; } return false; }

	DWORD GetDeviceID() const { return D3D9_DEVICE_ID; }

	void SetScale(float fScale) { m_fScale = fScale; }
	float GetScale() const { return m_fScale; }
	int GetWidth() const { return m_Width; }
	int GetHeight() const { return m_Height; }
	void* GetSrcInfo() const { return (void*)&m_ImageInfo; }

	void Invalidate(bool full);
	ITexture* AddRef();
	ITexture* Release();
};

class CGraphicsD3D9 :
	public IGraphics
{
	friend CTextureD3D9;

	// Everything that's needed to Render the Scene to a Texture:
	// The old rendering surface which must be set back then released after we're done with the new one.
	static IDirect3DSurface9 *ms_pBackBufferSurface;
	// The texture we will be Rendering our Scene to.
	static IDirect3DTexture9 *ms_pHelperTexture;
	// The surface that is part of the texture we actually render to.
	static IDirect3DSurface9 *ms_pHelperSurface;
	// The texture we will be Rendering our Scene to.
	static IDirect3DTexture9 *ms_pHelperTexture2;
	// The surface that is part of the texture we actually render to.
	static IDirect3DSurface9 *ms_pHelperSurface2;

	static int ms_nHelperWidth;
	static int ms_nHelperHeight;
	static D3DFORMAT ms_TextureFormat;

	DWORD m_dwFilterColor;
	int m_nFilterHorzMove;
	int m_nFilterVertMove;
	float m_fFilterPixelate;
	bool m_bFilterPixelate;
	D3DCDTVERTEX m_FiltersOverlay[4];

	static HWND ms_hWnd;
	static D3DPRESENT_PARAMETERS ms_d3dpp; // Used to explain to Direct3D how it will present things on the screen
	static int ms_nCount;
	static bool ms_bWindowed;
	static IDirect3D9 *ms_pD3D;
	static IDirect3DDevice9 *ms_pD3DDevice;
	static D3DDISPLAYMODE ms_PreferredMode;
	static D3DDISPLAYMODE ms_WindowedMode;
	static D3DCAPS9 ms_D3DCaps;

	static int ms_nScreenWidth;
	static int ms_nScreenHeight;
	static bool ms_bLastRendered;

	float m_Zoom;
	bool m_bInitialized;
	ARGBCOLOR m_rgbClearColor;
	ARGBCOLOR m_rgbFilterBkColor;

	RECT m_RectView;
	RECT m_RectClip;
	RECT m_RectWorld;

	bool m_bFilters;
	bool m_bCapture;
	RECT m_RectOldClip;

	_D3DDEVTYPE m_devType;
	D3DXMATRIX m_OldWorldMatrix, m_WorldMatrix, m_IdentityMatrix;

	// map of created textures:
	typedef std::pair<const std::string, CTextureD3D9*> pairTexture;
	static std::map<std::string, CTextureD3D9*> ms_Textures;

	HWND m_hWnd;
	ID3DXFont *m_pD3DFont;
	ARGBCOLOR m_rgbFontColor;

	D3DCDVERTEX *m_pGrid[2];
	int m_nGridLines[2];
	void BuildGrid(int nGridIdx, int nGridSize, ARGBCOLOR rgbColor);
	void UpdateGrid(int nGridSize, ARGBCOLOR rgbColor);

	DWORD FigureOutVertexProcessing();
	void FigureOutDisplayMode(D3DFORMAT currentFormat, UINT width, UINT height);

	void PostInitialize(UINT WindowWidth, UINT WindowHeight);
	void RenderRect(const RECT &rectDest, ARGBCOLOR rgbColor) const;
	void RenderFill(const RECT &rectDest, ARGBCOLOR rgbColor) const;

	void CreateVertexBuffer(SVertexBuffer **vbuffer, const ITexture *texture, const RECT &rectSrc, const RECT &rectDest, int rotate, int transform, ARGBCOLOR rgbColor) const;
	void UpdateVertexBuffer(SVertexBuffer **vbuffer, const ITexture *texture, const RECT &rectSrc, const RECT &rectDest, int rotate, int transform, ARGBCOLOR rgbColor) const;

	bool Recover();
	void Invalidate(bool full);

	LPCSTR GetFormat(D3DFORMAT Format) const;
	WORD GetBitCount() const;
	WORD GetNext16ARGB(BYTE **Data) const;
	void Clear(const RECT *rectDest, ARGBCOLOR rgbColor) const;


public:
	CGraphicsD3D9();
	~CGraphicsD3D9();

	bool SetWindowView(HWND hWnd, float fZoom, const RECT *pClient, const RECT *pWorld);
	void GetWorldRect(RECT *Rect_) const;
	void GetWorldPosition(POINT *Point_) const;
	void SetWorldPosition(const POINT *Point_);

	void ViewToWorld(POINT *Point_) const;
	void ViewToWorld(RECT *Rect_) const;
	void WorldToView(POINT *Point_) const;
	void WorldToView(RECT *Rect_) const;

	void GetVisibleRect(RECT *Rect_) const;
	float GetCurrentZoom() const;

	bool SetMode(HWND hWnd, bool bWindowed, int nScreenWidth, int nScreenHeight);
	bool SetModeA(HWND hWnd, bool bWindowed, int nScreenWidth, int nScreenHeight);
	bool SetModeB(HWND hWnd, bool bWindowed, int nScreenWidth, int nScreenHeight);

	bool Initialize(HWND hWnd, bool bWindowed, int nScreenWidth, int nScreenHeight);
	int  Finalize();
	bool BeginPaint();
	bool DrawFrame(const RECT &RectClip, ARGBCOLOR rgbColor, ARGBCOLOR rgbBoundaries);
	bool EndPaint();
	bool DrawGrid(int nGridSize, ARGBCOLOR rgbColor);

	bool BeginCapture(RECT *rectDesired, float zoom);
	bool EndCapture(WORD *pDest, RECT &rcPortion, RECT &rcFull);

	void SetClearColor(ARGBCOLOR rgbColor);
	void SetFilterBkColor(ARGBCOLOR rgbColor);

	void SetFont(LPCSTR lpFont, int CharHeight, ARGBCOLOR rgbColor, LONG Weight);
	void DrawText(const POINT &pointDest, LPCSTR lpString, ...) const;
	void DrawText(const POINT &pointDest, ARGBCOLOR rgbColor, LPCSTR lpString, ...) const;

	void Render(
		const ITexture *texture, const RECT &rectSrc, 
		const RECT &rectDest, 
		int rotate = GFX_ROTATE_0, 
		int transform = GFX_NORMAL, 
		ARGBCOLOR rgbColor = COLOR_ARGB(255,255,255,255), 
		float lightness = 0.5f,
		IBuffer **buffer = NULL,
		float rotation = 0.0f,
		float scale = 1.0f
	) const;

	void FillRect(const RECT &rectDest, ARGBCOLOR rgbColor) const;
	void DrawRect(const RECT &rectDest, ARGBCOLOR rgbColor, int bordersize) const;
	void BoundingBox(const RECT &rectDest, ARGBCOLOR rgbColor) const;
	void SelectingBox(const RECT &rectDest, ARGBCOLOR rgbColor) const;
	void SelectionBox(const RECT &rectDest, ARGBCOLOR rgbColor) const;

	bool CreateTextureFromFile(LPCSTR filename, ITexture **texture, float scale) const;
	bool CreateTextureFromFileInMemory(LPCSTR filename, LPCVOID pSrcData, UINT SrcData, ITexture **texture, float scale) const;

	bool SetFilter(GpxFilters eFilter, void *vParam);
	bool FlushFilters(bool bClear);
	void UpdateFilters();

	void SetConsole(IConsole *pConsole) { g_pConsole = pConsole; }

	DWORD GetDeviceID() const { return D3D9_DEVICE_ID; }
};
