/* QuestDesigner - Open Legends's Project
   Copyright (C) 2003. System_Failure 
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
/*! \file		GraphicsOGL.cpp
	\brief		Implementation of the OpenGL 2D Graphics plugin for Open Legends.
	\date		Nov. 10, 2003: 
					* Initial Release.
				
	\author		System Failure
	\remarks	
				Starting this puppy. Got the framework layed out just gotta fill in the blanks ;)
				

			Known bugs:
				

*/

#include "stdafx.h"
#include "GraphicsOGL.h"

//////////////////////////////////////////////////////////////////////////////
// Interface Version Definition:
const WORD IGraphics::Version = 0x0014;//version 1.4, same for a while. hard to improve a good thing ;)

//////////////////////////////////////////////////////////////////////////////
// Needed Libraries:
#pragma comment(lib, "GLU32.lib")
#pragma comment(lib, "GLAUX.lib")
#pragma comment(lib, "OPENGL32.lib")

//////////////////////////////////////////////////////////////////////////////
// Exported functions:

HRESULT QueryGfxInterface(WORD Version, IGraphics **pInterface, IConsole *Output)
{
	if( HIBYTE(Version) != HIBYTE(IGraphics::Version) ) return E_FAIL;
	if(	LOBYTE(Version) > LOBYTE(IGraphics::Version) ) return E_FAIL;

	if(!*pInterface) {
		// Convert our interface pointer to a CGraphicsOGL object
		*pInterface = new CGraphicsOGL;
		(*pInterface)->SetConsole(Output);
		return S_OK;
	}

	return E_FAIL;
}//thanks Kronuz for this function ;)

// Release our Handle to the class
HRESULT ReleaseGfxInterface(IGraphics **pInterface)
{
	if(!*pInterface) {
		return E_FAIL;
	}

	// Not our device:
	if((*pInterface)->GetDeviceID() != OGL_DEVICE_ID) {
		return E_FAIL;
	}

	delete *pInterface;
	*pInterface = NULL;
	return S_OK;
}

/***********CTextureOGL***************/

CTextureOGL::~CTextureOGL() 
	{
		
	}

	//! Gets a void pointer to the texture. (the pointer must be casted)
void* CTextureOGL::GetTexture()
	{
		
	}

bool CTextureOGL::SetTexture(void *pTexture)
	{
		
	}

void CTextureOGL::SetScale(float fScale) //!< Obtains the texture's scale.
	{
		
	}

float CTextureOGL::GetScale() //!< Obtains the texture's scale.
	{
		
	}

int CTextureOGL::GetWidth() //!< Obtains the texture's width.
	{
		
	}

int CTextureOGL::GetHeight() //!< Obtains the texture's height.
	{
		
	}

void* CTextureOGL::GetSrcInfo() //!< Obtains the texture's source file information.
	{
		
	}

	//! Invalidates the data in the texture. It releases or deletes the contained texture.
void CTextureOGL::Invalidate()
	{
		
	}

	//! Gets the ID of the device that created this texture.
DWORD CTextureOGL::GetDeviceID()
	{
		
	}

ITexture* CTextureOGL::AddRef()
	{
		
	}	

ITexture* CTextureOGL::Release()
	{
		
	}

/****************CBuufferOGL********************/
CBufferOGL::~CBufferOGL() 
	{
		
	}

	//! Get a void pointer to the buffer (the pointer must be casted)
void* CBufferOGL::GetBuffer()
	{
		
	}

bool CBufferOGL::SetBuffer(void *pBuffer)
	{
		
	}

	//! Invalidates the data in the buffer. This function releases or deletes the contained buffer.
void CBufferOGL::Invalidate()
	{
		
	}

	//! This touches the buffer, making its content dirty. A dirty buffer is updated in the next render.
void CBufferOGL::Touch()
	{
		
	}

	//! Gets the ID of the device that created this buffer.
DWORD CBufferOGL::GetDeviceID()
	{
		
	}

IBuffer* CBufferOGL::AddRef()
	{
		
	}

IBuffer* CBufferOGL::Release()
	{
		
	}

/**************CGraphicsOGL***************/
CGraphicsOGL::~CGraphicsOGL() 
	{
		
	}

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
bool CGraphicsOGL::Initialize(HWND hWnd, bool bWindowed=true, int nScreenWidth=0, int nScreenHeight=0)
	{
		
	}

bool CGraphicsOGL::SetMode(HWND hWnd, bool bWindowed=true, int nScreenWidth=0, int nScreenHeight=0)
	{
		
	}

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
int CGraphicsOGL::Finalize()
	{
		
	}

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
bool CGraphicsOGL::SetWindowView(HWND hWnd, float fZoom, const RECT *pClient, const RECT *pWorld)
	{
		
	}

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
void CGraphicsOGL::GetWorldRect(RECT *Rect_)
	{
		
	}

void CGraphicsOGL::GetWorldPosition(POINT *Point_)
	{
		
	}

void CGraphicsOGL::SetWorldPosition(POINT &Point_)
	{
		
	}

void CGraphicsOGL::WorldToView(POINT *Point_)
	{
		
	}

void CGraphicsOGL::WorldToView(RECT *Rect_)
	{
		
	}

void CGraphicsOGL::ViewToWorld(POINT *Point_)
	{
		
	}

void CGraphicsOGL::ViewToWorld(RECT *Rect_)
	{
		
	}

	/*! \brief Returns what zone of the world is visible at the time of the call.

		\return Returns the world's visible zone on the screen in world coordinates.

		\remarks 
		The visible zone is determinated by the actual world view and zoom. It represents
		the exact bounding coordinates of the visible pixels.

		\sa SetWindowView()
	*/
void CGraphicsOGL::GetVisibleRect(RECT *Rect_)
	{
		
	}

float CGraphicsOGL::GetCurrentZoom()
	{
		
	}

	/*! \brief Begins painting.

		\return Returns true if the it was possible to set up the device for painting.

		\remarks 
		This function must be called before drawing anything to the graphics device,
		and an EndPaint() must be issued after all painting has been done for the current
		frame. Before any drawing can take place, the device must be initialized.

		\sa Initialize(), EndPaint()
	*/
bool CGraphicsOGL::BeginPaint()
	{
		
	}

	//! Draws a clipping frame with the given clipping rectangle and color
bool CGraphicsOGL::DrawFrame(const RECT &RectClip, ARGBCOLOR rgbColor, ARGBCOLOR rgbBoundaries)
	{
		
	}

	//! Draws the grid with the given size and color
bool CGraphicsOGL::DrawGrid(int nGridSize, ARGBCOLOR rgbColor)
	{
		
	}

	/*! \brief Ends painting and presents the results on the screen.

		\return Returns true if the device could be closed and the painting has been presented.

		\remarks 
		This function must be called after the painting has been done. No other painting
		functions should be called after this method. Before any drawing can take place,
		a function call to BeginPaint() must be issued.

		\sa BeginPaint()
	*/
bool CGraphicsOGL::EndPaint()
	{
		
	}

bool CGraphicsOGL::BeginCapture(RECT *rectDesired, float zoom)
	{
		
	}

bool CGraphicsOGL::EndCapture(WORD *pDest, RECT &rcPortion, RECT &rcFull)
	{
		
	}

void CGraphicsOGL::SetClearColor(ARGBCOLOR rgbColor)
	{
		
	}

void CGraphicsOGL::SetFont(LPCSTR lpFont, int CharHeight, ARGBCOLOR rgbColor, LONG Weight)
	{
		
	}

void CGraphicsOGL::DrawText(const POINT &pointDest, LPCSTR lpString, ...)
	{
		
	}

void CGraphicsOGL::DrawText(const POINT &pointDest, ARGBCOLOR rgbColor, LPCSTR lpString, ...)
	{
		
	}

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

	//! Draws a filled rectangle
void CGraphicsOGL::FillRect(const RECT &rectDest, ARGBCOLOR rgbColor)
	{
		
	}

	//! Draws a rectangle with no fill.
void CGraphicsOGL::DrawRect(const RECT &rectDest, ARGBCOLOR rgbColor, int bordersize=1)
	{
		
	}

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
void CGraphicsOGL::BoundingBox(const RECT &rectDest, ARGBCOLOR rgbColor)
	{
		
	}
	
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
void CGraphicsOGL::SelectingBox(const RECT &rectDest, ARGBCOLOR rgbColor)
	{
		
	}

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
void CGraphicsOGL::SelectionBox(const RECT &rectDest, ARGBCOLOR rgbColor)
	{
		
	}

	// scale tells if the texture must be rescaled from the source file, either at load time or at render time.
bool CGraphicsOGL::CreateTextureFromFile(LPCSTR filename, ITexture **texture, float scale)
	{
		
	}

bool CGraphicsOGL::CreateTextureFromFileInMemory(LPCSTR filename, LPCVOID pSrcData, UINT SrcData, ITexture **texture, float scale)
	{
		
	}

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
bool CGraphicsOGL::SetFilter(GpxFilters eFilter, void *vParam)
	{
		
	}

	// Flushes the filters, drawing everything to the back buffer. Returns false if no filters
	// where set before the call to BeginPaint(). If bClear is true, the background is cleared before
	// painting using the color set by the call to SetFilterBkColor() or (0,0,0)
	// bClear should only be set to true on the first flush, otherwise the clear would cover 
	// previously flushed filters.
bool CGraphicsOGL::FlushFilters(bool bClear)
	{
		
	}

	// Sets the console for the interface. All messages will be sent to that console.
void CGraphicsOGL::SetConsole(IConsole *pConsole)
	{
		
	}

DWORD CGraphicsOGL::GetDeviceID()
	{
		
	}
