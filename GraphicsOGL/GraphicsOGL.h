/* QuestDesigner - Open Zelda's Project
   Copyright (C) 2003. System Failure
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
/*! \file		GraphicsOGL.h
	\brief		Interface for the OpenGL 2D Graphics plugin for Open Zelda.
	\date		Nov. 10, 2003
				
	\author		System Failure
*/

#pragma once

#include "../IGraphics.h"

//////////////////////////////////////////////////////////////////////////////
// Needed Headers:
#include <GL.H>
#include <GLAUX.H>
#include <GLU.H>

BOOL WINAPI DllEntryPoint(HINSTANCE hDll, DWORD fdwReason, LPVOID lpvRserved);

class CTextureOGL:public ITexture
{
	~CTextureOGL() {};

	//! Gets a void pointer to the texture. (the pointer must be casted)
	void* GetTexture();
	bool SetTexture(void *pTexture);

	void SetScale(float fScale); //!< Obtains the texture's scale.

	float GetScale(); //!< Obtains the texture's scale.
	int GetWidth(); //!< Obtains the texture's width.
	int GetHeight(); //!< Obtains the texture's height.
	void* GetSrcInfo(); //!< Obtains the texture's source file information.

	//! Invalidates the data in the texture. It releases or deletes the contained texture.
	void Invalidate();

	//! Gets the ID of the device that created this texture.
	DWORD GetDeviceID();

	ITexture* AddRef();
	ITexture* Release();
};

class CBufferOGL:public IBuffer
{
	~CBufferOGL() {};
	//! Get a void pointer to the buffer (the pointer must be casted)
	void* GetBuffer();
	bool SetBuffer(void *pBuffer);

	//! Invalidates the data in the buffer. This function releases or deletes the contained buffer.
	void Invalidate();

	//! This touches the buffer, making its content dirty. A dirty buffer is updated in the next render.
	void Touch();

	//! Gets the ID of the device that created this buffer.
	DWORD GetDeviceID();

	IBuffer* AddRef();
	IBuffer* Release();
};

class CGraphicsOGL:public IGraphics
{
	~CGraphicsOGL() {};

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
	bool Initialize(HWND hWnd, bool bWindowed=true, int nScreenWidth=0, int nScreenHeight=0);

	bool SetMode(HWND hWnd, bool bWindowed=true, int nScreenWidth=0, int nScreenHeight=0);

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
	int Finalize();

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
	bool SetWindowView(HWND hWnd, float fZoom, const RECT *pClient, const RECT *pWorld);

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
	void GetWorldRect(RECT *Rect_);
	void GetWorldPosition(POINT *Point_);
	void SetWorldPosition(POINT &Point_);

	void WorldToView(POINT *Point_);
	void WorldToView(RECT *Rect_);
	void ViewToWorld(POINT *Point_);
	void ViewToWorld(RECT *Rect_);

	/*! \brief Returns what zone of the world is visible at the time of the call.

		\return Returns the world's visible zone on the screen in world coordinates.

		\remarks 
		The visible zone is determinated by the actual world view and zoom. It represents
		the exact bounding coordinates of the visible pixels.

		\sa SetWindowView()
	*/
	void GetVisibleRect(RECT *Rect_);

	float GetCurrentZoom();

	/*! \brief Begins painting.

		\return Returns true if the it was possible to set up the device for painting.

		\remarks 
		This function must be called before drawing anything to the graphics device,
		and an EndPaint() must be issued after all painting has been done for the current
		frame. Before any drawing can take place, the device must be initialized.

		\sa Initialize(), EndPaint()
	*/
	bool BeginPaint();

	//! Draws the frame for all objects in the world (current clipping)
	bool DrawFrame();

	//! Draws a clipping frame with the given clipping rectangle and color
	bool DrawFrame(const RECT &RectClip, ARGBCOLOR rgbColor, ARGBCOLOR rgbBoundaries);

	//! Draws the grid with the given size and color
	bool DrawGrid(int nGridSize, ARGBCOLOR rgbColor);

	/*! \brief Ends painting and presents the results on the screen.

		\return Returns true if the device could be closed and the painting has been presented.

		\remarks 
		This function must be called after the painting has been done. No other painting
		functions should be called after this method. Before any drawing can take place,
		a function call to BeginPaint() must be issued.

		\sa BeginPaint()
	*/
	bool EndPaint();

	bool BeginCapture(RECT *rectDesired, float zoom);
	bool EndCapture(WORD *pDest, RECT &rcPortion, RECT &rcFull);

	void SetClearColor(ARGBCOLOR rgbColor);

	void SetFont(LPCSTR lpFont, int CharHeight, ARGBCOLOR rgbColor, LONG Weight);
	void DrawText(const POINT &pointDest, LPCSTR lpString, ...);
	void DrawText(const POINT &pointDest, ARGBCOLOR rgbColor, LPCSTR lpString, ...);

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
	void Render(
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
	);

	//! Draws a filled rectangle
	void FillRect(const RECT &rectDest, ARGBCOLOR rgbColor);

	//! Draws a rectangle with no fill.
	void DrawRect(const RECT &rectDest, ARGBCOLOR rgbColor, int bordersize=1);

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
	void BoundingBox(const RECT &rectDest, ARGBCOLOR rgbColor);
	
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
	void SelectingBox(const RECT &rectDest, ARGBCOLOR rgbColor);

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
	void SelectionBox(const RECT &rectDest, ARGBCOLOR rgbColor);

	// scale tells if the texture must be rescaled from the source file, either at load time or at render time.
	bool CreateTextureFromFile(LPCSTR filename, ITexture **texture, float scale);
	bool CreateTextureFromFileInMemory(LPCSTR filename, LPCVOID pSrcData, UINT SrcData, ITexture **texture, float scale);

	// Sets the console for the interface. All messages will be sent to that console.
	void SetConsole(IConsole *pConsole);

	DWORD GetDeviceID();

};