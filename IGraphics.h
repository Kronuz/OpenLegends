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

#pragma once

#include <IConsole.h>

#define GFX_NORMAL		0x00
#define GFX_MIRRORED	0x01
#define GFX_FLIPPED		0x02

#define GFX_ROTATE_0	0x00
#define GFX_ROTATE_90	0x01
#define GFX_ROTATE_180	0x02
#define GFX_ROTATE_270	0x03

#define COLOR_ARGB(a,r,g,b) \
	((ARGBCOLOR)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))

#define COLOR_RGB(r,g,b) \
	((ARGBCOLOR)((((255)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))

#define COLOR_AHSL(a,h,s,l) \
	((ARGBCOLOR)((((a)&0xff)<<24)|(((h)&0xff)<<16)|(((s)&0xff)<<8)|((l)&0xff)))

#define COLOR_HSL(h,s,l) \
	((AHSLCOLOR)((((255)&0xff)<<24)|(((h)&0xff)<<16)|(((s)&0xff)<<8)|((l)&0xff)))

typedef struct tagCOLOR {
	union {
		DWORD dwColor;
		struct {
			BYTE    rgbBlue; 
			BYTE    rgbGreen; 
			BYTE    rgbRed; 
			BYTE    rgbAlpha; 
		};
		struct {
			DWORD	hslLightness	: 8;
			DWORD	hslSaturation	: 7;
			DWORD	hslHue			: 9;
			DWORD	hslAlpha		: 8; 
		};
	};
	tagCOLOR(BYTE Alpha, BYTE Red, BYTE Green, BYTE Blue) : dwColor(COLOR_ARGB(Alpha, Red, Green, Blue)) {}
	tagCOLOR(BYTE Red, BYTE Green, BYTE Blue) : dwColor(COLOR_RGB(Red, Green, Blue)) {}
	tagCOLOR(DWORD _dwColor) : dwColor(_dwColor) {}
	tagCOLOR(RGBQUAD _Color) : rgbRed(_Color.rgbRed), rgbGreen(_Color.rgbGreen), rgbBlue(_Color.rgbBlue), rgbAlpha(_Color.rgbReserved) {}
	tagCOLOR() : dwColor(0) {}
	bool operator==(DWORD _dwColor) const { return (dwColor == _dwColor); }
	bool operator==(tagCOLOR _Color) const { return (dwColor == _Color.dwColor); }
	bool operator!=(DWORD _dwColor) const { return (dwColor != _dwColor); }
	bool operator!=(tagCOLOR _Color) const { return (dwColor != _Color.dwColor); }
	tagCOLOR operator=(DWORD _dwColor) { dwColor = _dwColor; return *this; }
	operator DWORD() const { return dwColor; }
	operator RGBQUAD() const { 
		RGBQUAD ret; 
		ret.rgbReserved = rgbAlpha; 
		ret.rgbRed = rgbRed; 
		ret.rgbGreen = rgbGreen; 
		ret.rgbBlue = rgbBlue; 
		return ret; 
	}
} ARGBCOLOR, AHSLCOLOR;

/*
Calculate HSL from RGB
Hue is in degrees (from 0 to 360)
Lightness is between 0 and 255
Saturation is between 0 and 100 (percentil)
*/
inline AHSLCOLOR RGB2HSL(ARGBCOLOR rgbColor)
{
	AHSLCOLOR hslColor;

	double themin, themax, delta;
	double c1r, c1g, c1b;
	double c2h, c2l, c2s;

	c1r = (double)rgbColor.rgbRed / 255.0f;
	c1g = (double)rgbColor.rgbGreen / 255.0f;
	c1b = (double)rgbColor.rgbBlue / 255.0f;

	themin = min(c1r,min(c1g,c1b));
	themax = max(c1r,max(c1g,c1b));
	delta = themax - themin;
	c2l = (themin + themax) / 2;
	c2s = 0;
	if (c2l > 0 && c2l < 1)
		c2s = delta / (c2l < 0.5 ? (2*c2l) : (2-2*c2l));
	c2h = 0;
	if (delta > 0) {
		if (themax == c1r && themax != c1g)
			c2h += (c1g - c1b) / delta;
		if (themax == c1g && themax != c1b)
			c2h += (2 + (c1b - c1r) / delta);
		if (themax == c1b && themax != c1r)
			c2h += (4 + (c1r - c1g) / delta);
		c2h *= 60;
	}

	hslColor.hslAlpha = rgbColor.hslAlpha;
	hslColor.hslHue = (int)(c2h + 0.5f);
	hslColor.hslSaturation = (int)(c2s * 100.0f + 0.5f);
	hslColor.hslLightness = (int)(c2l * 255.0f + 0.5f);

	return hslColor;
}

/*
Calculate HSL from RGB
Hue is in degrees (from 0 to 360)
Lightness is between 0 and 255
Saturation is between 0 and 100 (percentil)
*/
inline ARGBCOLOR HSL2RGB(AHSLCOLOR hslColor)
{
	ARGBCOLOR rgbColor;

	double c2r, c2g, c2b;
	double c1h, c1l, c1s;

	double ctmpr, ctmpg, ctmpb;
	double satr, satg, satb;

	c1h = (double)hslColor.hslHue;					// Hue in degrees
	c1l = (double)hslColor.hslLightness / 255.0f;	// Lightness betweeen 0 and 1
	c1s = (double)hslColor.hslSaturation / 100.0f;	// Saturation between 0 and 1

	while (c1h < 0)
		c1h += 360;
	while (c1h > 360)
		c1h -= 360;

	if (c1h < 120) {
		satr = (120 - c1h) / 60.0;
		satg = c1h / 60.0;
		satb = 0;
	} else if (c1h < 240) {
		satr = 0;
		satg = (240 - c1h) / 60.0;
		satb = (c1h - 120) / 60.0;
	} else {
		satr = (c1h - 240) / 60.0;
		satg = 0;
		satb = (360 - c1h) / 60.0;
	}
	satr = min(satr, 1);
	satg = min(satg, 1);
	satb = min(satb, 1);

	ctmpr = 2 * c1s * satr + (1 - c1s);
	ctmpg = 2 * c1s * satg + (1 - c1s);
	ctmpb = 2 * c1s * satb + (1 - c1s);

	if (c1l < 0.5) {
		c2r = c1l * ctmpr;
		c2g = c1l * ctmpg;
		c2b = c1l * ctmpb;
	} else {
		c2r = (1 - c1l) * ctmpr + 2 * c1l - 1;
		c2g = (1 - c1l) * ctmpg + 2 * c1l - 1;
		c2b = (1 - c1l) * ctmpb + 2 * c1l - 1;
	}

	rgbColor.hslAlpha = hslColor.hslAlpha;
	rgbColor.rgbRed = (int)(c2r * 255.0f + 0.5f);
	rgbColor.rgbGreen = (int)(c2g * 255.0f + 0.5f);
	rgbColor.rgbBlue = (int)(c2b * 255.0f + 0.5f);

	return rgbColor;
}

enum GpxFilters { ClearFilters, EnableFilters, Pixelate, Colorize, Alpha, HorzMove, VertMove };

/////////////////////////////////////////////////////////////////////////////
/*! \interface	IBuffer
	\brief		Interface for the textures.
	\author		Germán Méndez Bravo (Kronuz)
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
	virtual void Invalidate(bool full) = 0;

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
	\author		Germán Méndez Bravo (Kronuz)
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
	virtual void Invalidate(bool full) = 0;

	//! Gets the ID of the device that created this texture.
	virtual DWORD GetDeviceID() const = 0;

	virtual ITexture* AddRef() = 0;
	virtual ITexture* Release() = 0;
};

/////////////////////////////////////////////////////////////////////////////
/*! \interface	IGraphics
	\brief		Interface the graphics devices.
	\author		Germán Méndez Bravo (Kronuz)
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

	virtual bool SetMode(HWND hWnd, bool bWindowed=true, int nScreenWidth=0, int nScreenHeight=0) = 0;

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
		\param fZoom states if the window view is zoomed, and how much the zoom is. 
			A value of 1 indicates no zoom while a bigger value indicates a zoom in 
			and a smaller (from ~0 to 1) indicates a zoom out. Values should be between
			0.25 to 4.
		\param pClient The new client RECT of the window, to find out what parts 
			of the world are visible in the screen (in view coordinates).
		\param pWorld The new World or clipping size (in world coordinates).

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
	virtual bool SetWindowView(HWND hWnd, float fZoom, const RECT *pClient, const RECT *pWorld) = 0;

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
	virtual void GetWorldRect(RECT *Rect_) const = 0;
	virtual void GetWorldPosition(POINT *Point_) const = 0;
	virtual void SetWorldPosition(const POINT *Point_) = 0;

	virtual void WorldToView(POINT *Point_) const = 0;
	virtual void WorldToView(RECT *Rect_) const = 0;
	virtual void ViewToWorld(POINT *Point_) const = 0;
	virtual void ViewToWorld(RECT *Rect_) const = 0;

	/*! \brief Returns what zone of the world is visible at the time of the call.

		\return Returns the world's visible zone on the screen in world coordinates.

		\remarks 
		The visible zone is determinated by the actual world view and zoom. It represents
		the exact bounding coordinates of the visible pixels.

		\sa SetWindowView()
	*/
	virtual void GetVisibleRect(RECT *Rect_) const = 0;

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

	//! Draws a clipping frame with the given clipping rectangle and color
	virtual bool DrawFrame(const RECT &RectClip, ARGBCOLOR rgbColor, ARGBCOLOR rgbBoundaries) = 0;

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

	// Sets the background color.
	virtual void SetClearColor(ARGBCOLOR rgbColor) = 0;

	// Sets the Filter Background clear color *and* the Clear color to the desired rgbColor:
	// SetClearColor Should be used again after this call to change the Clear Color value.
	virtual void SetFilterBkColor(ARGBCOLOR rgbColor) = 0;

	virtual void SetFont(LPCSTR lpFont, int CharHeight, ARGBCOLOR rgbColor, LONG Weight) = 0;
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
		\param rotation		Relative extra rotation of the sprite. 
			Initial basic rotation + relative rotation = Final rotation of the sprite.
		\param scale		Relative extra scale factor for the sprite.

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
		float lightness = 0.5f,
		IBuffer **buffer = NULL,
		float rotation = 0.0f,
		float scale = 1.0f
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
		with rubber bands (small selection squares in the corners and in the middles to 
		give the sense that the selection can be resized.)
		Before any drawing can take place, a function call to BeginPaint() must be issued.

		This functionality is implemented at this level because optimizatinos can be 
		done easily.

		\sa BoundingBox(), BeginPaint()
	*/
	virtual void SelectionBox(const RECT &rectDest, ARGBCOLOR rgbColor) const = 0;

	// scale tells if the texture must be rescaled from the source file, either at load time or at render time.
	virtual bool CreateTextureFromFile(LPCSTR filename, ITexture **texture, float scale) const = 0;
	virtual bool CreateTextureFromFileInMemory(LPCSTR filename, LPCVOID pSrcData, UINT SrcData, ITexture **texture, float scale) const = 0;

	// 
	/*! \brief Sets and configures the filters for the next render.

		\param eFilter Filter to use.
		\param vParam Parameter of the filter. 

		\return Returns true if the filter was successfully applied.

		\remarks 
		eFilter can be:
			* Clear			To clear all filters (vParam is not used and must be set to NULL)

			* Pixelate		Where vParam is an integer stating the width or height 
							of the resulting pixels in the pixelated scene.

			* Alpha or		Where vParam is an ARGB color satating the desired Alpha and the RGB
			  Colorize		values of the colorization. if the RGB components are (128, 128, 128)
							no colorization is applied and only the alpha is used.

			* HorzMove		Where vParam is an integer stating the horizontal displacement of
							everything to be drawn before painting it to the screen.

			* VertMove		Where vParam is an integer stating the Vertical displacement of
							everything to be drawn before painting it to the screen.

		If any filter is going to be used, at least the first filter should be set before
		the call to BeginPaint(). Further filters can be set any time after BeginPaint() call.
		If a filter is set, that filter is kept until it is overwritten or the filters are cleared.

		\sa SetClearColor(), SetFilterBkColor(), BeginPaint()
	*/
	virtual bool SetFilter(GpxFilters eFilter, void *vParam) = 0;

	// Flushes the filters, drawing everything to the back buffer. Returns false if no filters
	// where set before the call to BeginPaint(). If bClear is true, the background is cleared before
	// painting using the color set by the call to SetFilterBkColor() or (0,0,0)
	// bClear should only be set to true on the first flush, otherwise the clear would cover 
	// previously flushed filters.
	virtual bool FlushFilters(bool bClear) = 0;

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
