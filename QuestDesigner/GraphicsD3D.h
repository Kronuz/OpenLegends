

#include "Interfaces.h"
#include "Console.h"

#include <D3d8.h>
#include <D3dx8core.h>
#include <D3dx8tex.h>

#define D3D8_DEVICE_ID 0xca96d82f

#ifdef _DEBUG
#define D3DVERIFY(expr) ASSERT(SUCCEEDED(expr))
//#define D3DTRACE printf
#define D3DTRACE __noop
#else
#define D3DVERIFY(expr) (expr)
#define D3DTRACE __noop
#endif

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
	This struct contains the vertex buffer (precalculated cache) used by the D3D8 2D engine to
	draw the sprites faster on the screen.
*/
struct SVertexBuffer {
	D3DCDTVERTEX *m_pVertices;
	int m_nVertices;
	int m_nPrimitives;
	SVertexBuffer(D3DCDTVERTEX *pVertices_, int nVertices_, int nPrmitives_) : 
		m_pVertices(pVertices_), m_nVertices(nVertices_), m_nPrimitives(nPrmitives_) {}
	~SVertexBuffer() {
		delete []m_pVertices;
	}
};

class CBufferD3D8 :
	public IBuffer,
	public CConsole
{
	int m_nCount;
	SVertexBuffer *m_pBuffer;
public:
	CBufferD3D8(SVertexBuffer *pBuffer);
	~CBufferD3D8();

	void *GetBuffer() const { return (void*)m_pBuffer; }
	bool SetBuffer(void *pBuffer) { if(!m_pBuffer) { m_pBuffer = (SVertexBuffer*)pBuffer; return true; } return false; }

	DWORD GetDeviceID() const { return D3D8_DEVICE_ID; }

	void Invalidate();
	IBuffer* AddRef();
	IBuffer* Release();
};

class CTextureD3D8 :
	public ITexture,
	public CConsole
{
	int m_nCount;
	IDirect3DTexture8 *m_pTexture;
	D3DXIMAGE_INFO	m_ImageInfo;
	UINT m_Width;
	UINT m_Height;
public:
	CTextureD3D8(IDirect3DTexture8 *pTexture, D3DXIMAGE_INFO imageInfo);
	~CTextureD3D8();

	void *GetTexture() const { return (void*)m_pTexture; }
	bool SetTexture(void *pTexture) { if(!m_pTexture) { m_pTexture = (IDirect3DTexture8*)pTexture; return true; } return false; }

	DWORD GetDeviceID() const { return D3D8_DEVICE_ID; }

	int GetWidth() const { return m_Width; }
	int GetHeight() const { return m_Height; }
	void* GetSrcInfo() const { return (void*)&m_ImageInfo; }

	void Invalidate();
	ITexture* AddRef();
	ITexture* Release();
};
class CGraphicsD3D8 :
	public IGraphics,
	public CConsole
{
	friend CTextureD3D8;

	static int ms_nCount;
	static bool ms_bWindowed;
	static IDirect3D8 *ms_pD3D;
	static IDirect3DDevice8 *ms_pD3DDevice;
	static CSimpleMap<CString, CTextureD3D8*> ms_Textures;

	float m_Zoom;
	bool m_bInitialized;

	int m_nScreenWidth;
	int m_nScreenHeight;

	_D3DDEVTYPE m_devType;
	D3DXMATRIX WorldMatrix, IdentityMatrix;
	IDirect3DSwapChain8 *m_pSwapChain;

	D3DDISPLAYMODE m_PreferredMode;

	DWORD FigureOutVertexProcessing();
	void FigureOutDisplayMode(D3DFORMAT currentFormat, UINT width, UINT height);

	CRect m_RectView;
	CRect m_RectClip;

	void PostInitialize(float WindowWidth, float WindowHeight);
	void RenderRect(const RECT &rectDest, BYTE alpha, BYTE red, BYTE green, BYTE blue) const;
	void RenderFill(const RECT &rectDest, BYTE alpha, BYTE red, BYTE green, BYTE blue) const;

	void CreateVertexBuffer(SVertexBuffer **vbuffer, const ITexture *texture, const RECT &rectSrc, const RECT &rectDest, int rotate, int transform, int alpha) const;
	void UpdateVertexBuffer(SVertexBuffer **vbuffer, const ITexture *texture, const RECT &rectSrc, const RECT &rectDest, int rotate, int transform, int alpha) const;

public:
	CGraphicsD3D8();
	~CGraphicsD3D8();

	bool SetWindowView(HWND hWnd, const RECT &client, const RECT &clipping, float zoom);
	void GetWorldPosition(CPoint &Point_) const;
	void GetWorldRect(CRect &Rect_) const;
	void GetViewPosition(CPoint &Point_) const;
	void GetViewRect(CRect &Rect_) const;

	RECT GetVisibleRect();
	float GetCurrentZoom();

	bool Initialize(HWND hWnd, bool bWindowed, int nScreenWidth, int nScreenHeight);
	void Finalize();
	bool BeginPaint();
	bool EndPaint();

	void Render(const ITexture *texture, const RECT &rectSrc, const POINT &pointDest, int rotate, int transform, int alpha, IBuffer **buffer) const;
	void Render(const ITexture *texture, const RECT &rectSrc, const RECT &rectDest, int rotate, int transform, int alpha, IBuffer **buffer) const;
	void Clear(const RECT *rectDest, BYTE alpha, BYTE red, BYTE green, BYTE blue) const;
	void FillRect(const RECT &rectDest, BYTE alpha, BYTE red, BYTE green, BYTE blue) const;
	void DrawRect(const RECT &rectDest, BYTE alpha, BYTE red, BYTE green, BYTE blue, int bordersize) const;
	void SelectionBox(const RECT &rectDest, BYTE alpha, BYTE red, BYTE green, BYTE blue) const;
	void BoundingBox(const RECT &rectDest, BYTE alpha, BYTE red, BYTE green, BYTE blue) const;

	bool CreateTextureFromFile(LPCSTR filename, ITexture **texture);

	DWORD GetDeviceID() const { return D3D8_DEVICE_ID; }

};
