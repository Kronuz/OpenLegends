
#pragma once

#include "../IConsole.h"

#define GFX_NORMAL		0x00
#define GFX_MIRRORED	0x01
#define GFX_FLIPPED		0x02

#define GFX_ROTATE_0	0x00
#define GFX_ROTATE_90	0x01
#define GFX_ROTATE_180	0x02
#define GFX_ROTATE_270	0x03

typedef union tagARGBCOLOR {
	DWORD dwColor;
	struct {
		BYTE    rgbBlue; 
		BYTE    rgbGreen; 
		BYTE    rgbRed; 
		BYTE    rgbAlpha; 
	};
	tagARGBCOLOR(DWORD dwARGB) : dwColor(dwARGB) {}
	tagARGBCOLOR() : dwColor(0) {}
} ARGBCOLOR;

#define COLOR_ARGB(a,r,g,b) \
	((ARGBCOLOR)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))

/////////////////////////////////////////////////////////////////////////////
/*! \interface	IBuffer
	\brief		Interface for the textures.
	\author		Kronuz
	\version	1.0
	\date		May 27, 2003

	This interface is to keep all the information that might be used to render the 
	sprites	faster or to keep cached data for latter use. 
	
	\remarks
	Buffers can chenge as the device that created them die or is changed. So a method 
	to invalidate the buffer is provided. The device that created the buffer must check 
	every time if the buffer is still valid (by getting a not NULL pointer), and also 
	checking if the buffer's device ID is the same as the current device ID. In case 
	the buffer is not valid or its device is different from the current active device, 
	a new buffer should be created.

	Buffers generally contain cached data related to how an object is drawn on the 
	screen, so buffers should be invalidated every time an object changes, to reflect
	its changes at the rendering time.

	Many classes are needed to maintain control of their lives. This interface
	creates classes that need to have a life span control. This is handled using
	the AddRef() and Release() methods of the interface.
*/
interface IBuffer
{
	virtual ~IBuffer() {}
	//! Get a void pointer to the buffer (the pointer must be casted)
	virtual void* GetBuffer() const = 0;
	virtual bool SetBuffer(void *pBuffer) = 0;

	//! Invalidates the data in the buffer. This function releases or deletes the contained buffer.
	virtual void Invalidate() = 0;

	//! This touches the buffer, making its content dirty. A dirty buffer is updated in the next render.
	virtual void Touch() = 0;

	//! Gets the ID of the device that created this buffer.
	virtual DWORD GetDeviceID() const = 0;

	virtual IBuffer* AddRef() = 0;
	virtual IBuffer* Release() = 0;
};

/////////////////////////////////////////////////////////////////////////////
/*! \interface	ITexture
	\brief		Interface for the textures.
	\author		Kronuz
	\version	1.0
	\date		May 19, 2003

	This interface is to keep a virtual texture. 
	
	\remarks
	Textures can chenge as the device that created them die or is changed. So
	a method to invalidate the texture is provided. The device that created the 
	texture must issue an Invalidate() call to each of its textures before it 
	stops working, released or deleted.

	Many classes are needed to maintain control of their lives. This interface
	creates classes that need to have a life span control. This is handled using
	the AddRef() and Release() methods of the interface.
*/
interface ITexture
{
	virtual ~ITexture() {}

	//! Gets a void pointer to the texture. (the pointer must be casted)
	virtual void* GetTexture() const = 0;
	virtual bool SetTexture(void *pTexture) = 0;

	virtual void SetScale(float fScale) = 0; //!< Obtains the texture's scale.

	virtual float GetScale() const = 0; //!< Obtains the texture's scale.
	virtual int GetWidth() const = 0; //!< Obtains the texture's width.
	virtual int GetHeight() const = 0; //!< Obtains the texture's height.
	virtual void* GetSrcInfo() const = 0; //!< Obtains the texture's source file information.

	//! Invalidates the data in the texture. It releases or deletes the contained texture.
	virtual void Invalidate() = 0;

	//! Gets the ID of the device that created this texture.
	virtual DWORD GetDeviceID() const = 0;

	virtual ITexture* AddRef() = 0;
	virtual ITexture* Release() = 0;
};

/////////////////////////////////////////////////////////////////////////////
/*! \interface	IGraphics
	\brief		Interface the graphics devices.
	\author		Kronuz
	\version	1.0
	\date		May 19, 2003

	This interface contains all the methods to handle the graphics engine in
	a consistent, reliable and easy way. It contains methods to load graphics
	from multiple file formats, and to easily render sprites on the screen.
*/
interface IGraphics
{
	virtual ~IGraphics() {}

	static const WORD Version;

	/*! \brief Initialization of the interface.

		\param hWnd Handle to the parent window.
		\param bWindowed Sets the interface to be in windowed mode if true.
		\param nWindowWidth Sets the screen width for not windowed modes
		\param nWindowHeight Sets the screen height for not windowed modes
		\return Returns true if the interface was initialized or no major failures occurred.

		\remarks 
		This function must be called prior to any use of the interface,
		and should be called only once and as soon as the interface has
		been chosen. Before start drawing BeginPaint() must be called.
		
		If the interface needs to be switched, as if one needs to change
		from D3D to DD, the current graphics interface must be finalized
		before initializing the new graphics interface.

		\sa BeginPaint(), Finalize()
	*/
	virtual bool Initialize(HWND hWnd, bool bWindowed=true, int nScreenWidth=0, int nScreenHeight=0) = 0;

	/*! \brief Finalization of the interface.

		\return Returns true if the interface was finalized or no major failures occurred.

		\remarks 
		This function must be called just when the graphics interface is
		of no use anymore, and should be called only once after the
		initializacion of the interface.

		If the interface needs to be switched, as if one needs to change
		from D3D to DD, the current graphics interface must be finalized
		before initializing the new graphics interface.

		\sa Initialize()
	*/
	virtual int Finalize() = 0;

	/*! \brief Set the new window view position and size.

		\param hWnd Handle to the parent window.
		\param rect New position (and size) of the window in the graphics world.
		\param zoom states if the window view is zoomed, and how much the zoom is. 
			A value of 1 indicates no zoom while a bigger value indicates a zoom in 
			and a smaller (from ~0 to 1) indicates a zoom out. Values should be between
			0.25 to 4.

		\return Returns true if the window view could be changed or no major failures occurred.

		\remarks 
		This method must be called after any changes on the user's view. Changes
		that affect the user's view are window resize, or view scrolling.
		This method should be called just before rendering the changes.

		If this method is not used, and the view's size is changed, all graphics
		and sprites will be stretched and probably distorted. The method must be called
		before BeginPaint().

		\sa BeginPaint()
	*/
	virtual bool SetWindowView(HWND hWnd, const RECT &client, const RECT &clipping, float zoom=1.0) = 0;

	/*! \brief Returns the world position at a given point.

		\param point Actual point in the current view.
		\return Returns the world position point.

		\remarks 
		This method returns the world position relative to a view point.
		To achieve this, it must take into account the current zoom of the view
		and the size of the client area. This values are set using the function
		IGraphics::SetWindowView().

		\sa SetWindowView()
	*/
	virtual void GetWorldPosition(POINT *Point_) const = 0;
	virtual void GetWorldRect(RECT *Rect_) const = 0;

	virtual void GetViewPosition(POINT *Point_) const = 0;
	virtual void GetViewRect(RECT *Rect_) const = 0;

	/*! \brief Returns what zone of the world is visible at the time of the call.

		\return Returns the world's visible zone on the screen in world coordinates.

		\remarks 
		The visible zone is determinated by the actual world view and zoom. It represents
		the exact bounding coordinates of the visible pixels.

		\sa SetWindowView()
	*/
	virtual RECT GetVisibleRect() const = 0;

	virtual float GetCurrentZoom() const = 0;

	/*! \brief Begins painting.

		\return Returns true if the it was possible to set up the device for painting.

		\remarks 
		This function must be called before drawing anything to the graphics device,
		and an EndPaint() must be issued after all painting has been done for the current
		frame. Before any drawing can take place, the device must be initialized.

		\sa Initialize(), EndPaint()
	*/
	virtual bool BeginPaint() = 0;

	//! Draws the frame for all objects in the world
	virtual bool DrawFrame() = 0;

	//! Draws the grid with the given size and color
	virtual bool DrawGrid(int nGridSize, ARGBCOLOR rgbColor) = 0;

	/*! \brief Ends painting and presents the results on the screen.

		\return Returns true if the device could be closed and the painting has been presented.

		\remarks 
		This function must be called after the painting has been done. No other painting
		functions should be called after this method. Before any drawing can take place,
		a function call to BeginPaint() must be issued.

		\sa BeginPaint()
	*/
	virtual bool EndPaint() = 0;

	virtual bool BeginCapture(RECT *rectDesired, float zoom) = 0;
	virtual bool EndCapture(WORD *pDest, RECT &rcPortion, RECT &rcFull) = 0;

	virtual void SetClearColor(ARGBCOLOR rgbColor) = 0;

	virtual void SetFont(LPCSTR lpFont, const SIZE &sizeChar) = 0;
	virtual void DrawText(const POINT &pointDest, LPCSTR lpString, ...) const = 0;
	virtual void DrawText(const POINT &pointDest, ARGBCOLOR rgbColor, LPCSTR lpString, ...) const = 0;

	// Primitives:
	/*! \brief Used to render tiled sprites.

		\param texture		Texture or "sprite sheet" to be used.
		\param rectSrc		Part of the texture to be drawn (the actual sprite in the sprite sheet).
		\param rectDest		Location on the world where the sprite is to be drawn, and the size of the tile.
		\param rotate		Rotation of the sprite. 
			This can be	GFX_ROTATE_0, GFX_ROTATE_90, GFX_ROTATE_180, or GFX_ROTATE_270
		\param transform	Transformation applied to the sprite.
			This can be GFX_NORMAL, GFX_MIRRORED, GFX_FLIPPED, or GFX_MIRRORED | GFX_FLIPPED
		\param rgbColor Alpha, red, green, and blue components of the sprite color. 
			The Alpha value is probably the most important of all, because it changes the global
			alpha blending of the sprite.
		\param buffer		Pointer to a buffer to store temporary data. This buffer is used
			to speed up the rendering proces whenever is possible, and should not be changed
			outside the interface exept for invalidation and touching.

		\remarks 
		All rectangles are of inclusive-exlusive type.
		Before any drawing can take place, a function call to BeginPaint() must be issued.

		\sa IBuffer
	*/
	virtual void Render(
		const ITexture *texture, 
		const RECT &rectSrc, 
		const RECT &rectDest, 
		int rotate = GFX_ROTATE_0, 
		int transform = GFX_NORMAL, 
		ARGBCOLOR rgbColor = COLOR_ARGB(255,255,255,255), 
		IBuffer **buffer=NULL
	) const = 0;

	//! Draws a filled rectangle
	virtual void FillRect(const RECT &rectDest, ARGBCOLOR rgbColor) const = 0;

	//! Draws a rectangle with no fill.
	virtual void DrawRect(const RECT &rectDest, ARGBCOLOR rgbColor, int bordersize=1) const = 0;

	/*! \brief Draws a bounding box on the screen at a given place in the world.

		\param rectDest	Where the bounding box is going to be drew.
		\param rgbColor Alpha, red, green, and blue components of the sprite color. 

		\remarks 
		rectDest is a bounding rectangle giving the bounding box. All rectangles are
		of inclusive-exlusive type. A bounding is a simple rectangle drew on the screen
		with a one pixel line-thikness. Any zoom in the view should not affect the
		thickness of the lines composing the bounding box.
		Before any drawing can take place, a function call to BeginPaint() must be issued.

		This functionality is implemented at this level because optimizatinos can be 
		done easily.

		\sa BeginPaint()
	*/
	virtual void BoundingBox(const RECT &rectDest, ARGBCOLOR rgbColor) const = 0;
	
	/*! \brief Draws a selecting box on the screen at a given place in the world.

		\param rectDest	Where the selecting box is going to be drew.
		\param rgbColor Alpha, red, green, and blue components of the sprite color. 

		\remarks 
		This is pretty much the same as BoundingBox, with the exeption that this draws a box in the way
		of a selection (i.e. a box with dotted lines).
		Don't confuse SelectingBox with SelectionBox. SelectionBox is to mark the currently selected objects,
		while SelectingBox is used to show the boundaries of the objects to be selected.

		This functionality is implemented at this level because optimizatinos can be 
		done easily.

		\sa BoundingBox(), SelectingBox()
	*/
	virtual void SelectingBox(const RECT &rectDest, ARGBCOLOR rgbColor) const = 0;

	/*! \brief Draws a selection box on the screen at a given place in the world.

		\param rectDest	Where the selection box is going to be drew.
		\param rgbColor Alpha, red, green, and blue components of the sprite color. 

		\remarks 
		rectDest is a bounding rectangle giving the selction box. All rectangles are
		of inclusive-exlusive type. A selection box contains is in essence a bounding box 
		with small selection squares in the corners and in the middles to give the sense
		that the selection can be resized.
		Before any drawing can take place, a function call to BeginPaint() must be issued.

		This functionality is implemented at this level because optimizatinos can be 
		done easily.

		\sa BoundingBox(), BeginPaint()
	*/
	virtual void SelectionBox(const RECT &rectDest, ARGBCOLOR rgbColor) const = 0;

	// scale tells if the texture must be rescaled from the source file, either at load time or at render time.
	virtual bool CreateTextureFromFile(LPCSTR filename, ITexture **texture, float scale) const = 0;
	virtual bool CreateTextureFromFileInMemory(LPCSTR filename, LPCVOID pSrcData, UINT SrcData, ITexture **texture, float scale) const = 0;

	// Sets the console for the interface. All messages will be sent to that console.
	virtual void SetConsole(IConsole *pConsole) = 0;

	virtual DWORD GetDeviceID() const = 0;

};

extern "C"
{
	HRESULT QueryGfxInterface(WORD Version, IGraphics **pInterface, IConsole *Output);
	typedef HRESULT (*QUERYGFXINTERFACE)(WORD Version, IGraphics **pInterface, IConsole *Output);

	HRESULT ReleaseGfxInterface(IGraphics **pInterface);
	typedef HRESULT (*RELEASEGFXINTERFACE)(IGraphics **pInterface);
}
