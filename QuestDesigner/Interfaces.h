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
/*! \file		interfaces.h 
	\brief		Interfaces and abstract classes used by the core of OZ.
	\date		April 16, 2003

	This file defines the interfaces needed to manage objects with names,
	flyweight drawable objects, document management (file saving and loading), 
	and scriptable objects.
*/

#pragma once

#include <algorithm>
#include <vector>
using namespace std;

// Flags for the sprites and their transformations (higher byte of the status reserved):
#define SNORMAL				0x00
#define SMIRRORED			0x01
#define SFLIPPED			0x02
#define _SPT_TRANSFORM		0
#define SPT_TRANSFORM		((SNORMAL<<_SPT_TRANSFORM) | (SMIRRORED<<_SPT_TRANSFORM) | (SFLIPPED<<_SPT_TRANSFORM))

#define SNTILED				0x04
#define _SPT_INFO			0
#define SPT_INFO			(SNTILED<<_SPT_INFO)

#define SROTATE_0			0x00
#define SROTATE_90			0x01
#define SROTATE_180			0x02
#define SROTATE_270			0x03
#define _SPT_ROT			4
#define SPT_ROT				((SROTATE_0<<_SPT_ROT) | (SROTATE_90<<_SPT_ROT) | (SROTATE_180<<_SPT_ROT) | (SROTATE_270<<_SPT_ROT))

#define SPT_ALPHA			0x00ff00
#define _SPT_ALPHA			8

// Flags for the drawable objects and their transformations:
#define DVISIBLE			0x01
#define _DRW_SHFT			24
#define DROTATE				((SROTATE_90<<_SPT_ROT))

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
interface IBuffer;
interface IGraphics;

/////////////////////////////////////////////////////////////////////////////
/*! \class	CNamedObj
	\brief		Class to manage the name of objects.
	\author		Kronuz
	\version	1.0
	\date		April 28, 2003

	Many classes are needed to maintain its name, and common ways to
	get and manage it in a simple way. This is a self contained class
	and no implementation is required in any derived class, but a
	call to its constructor in the base class own constructor passing 
	the name (or perhaps class) of the object being created.
*/
class CNamedObj 
{
	CString m_sName;
protected:
	CNamedObj(LPCSTR szName) : m_sName(szName) { }
public:
	CString& GetName() { return m_sName; }
	void SetName(LPCSTR szName)  { m_sName = szName; }
};

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
	//! Get a void pointer to the buffer (the pointer must be casted)
	virtual void* GetBuffer() const = 0;
	virtual bool SetBuffer(void *pBuffer) = 0;

	//! Gets the ID of the device that created this buffer.
	virtual DWORD GetDeviceID() const = 0;

	//! Invalidates the data in the buffer. This function releases or deletes the contained buffer.
	virtual void Invalidate() = 0;

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
	//! Gets a void pointer to the texture. (the pointer must be casted)
	virtual void* GetTexture() const = 0;
	virtual bool SetTexture(void *pTexture) = 0;

	//! Gets the ID of the device that created this texture.
	virtual DWORD GetDeviceID() const = 0;

	virtual int GetWidth() const = 0; //!< Obtains the texture's width.
	virtual int GetHeight() const = 0; //!< Obtains the texture's height.
	virtual void* GetSrcInfo() const = 0; //!< Obtains the texture's source file information.

	//! Invalidates the data in the texture. It releases or deletes the contained texture.
	virtual void Invalidate() = 0;

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
	virtual void Finalize() = 0;

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
	virtual void GetWorldPosition(CPoint &Point_) const = 0;
	virtual void GetWorldRect(CRect &Rect_) const = 0;

	virtual void GetViewPosition(CPoint &Point_) const = 0;
	virtual void GetViewRect(CRect &Rect_) const = 0;

	/*! \brief Returns what zone of the world is visible at the time of the call.

		\return Returns the world's visible zone on the screen in world coordinates.

		\remarks 
		The visible zone is determinated by the actual world view and zoom. It represents
		the exact bounding coordinates of the visible pixels.

		\sa SetWindowView()
	*/
	virtual RECT GetVisibleRect() = 0;

	virtual float GetCurrentZoom() = 0;

	/*! \brief Begins painting.

		\return Returns true if the it was possible to set up the device for painting.

		\remarks 
		This function must be called before drawing anything to the graphics device,
		and an EndPaint() must be issued after all painting has been done for the current
		frame. Before any drawing can take place, the device must be initialized.

		\sa Initialize(), EndPaint()
	*/
	virtual bool BeginPaint() = 0;

	/*! \brief Ends painting and presents the results on the screen.

		\return Returns true if the device could be closed and the painting has been presented.

		\remarks 
		This function must be called after the painting has been done. No other painting
		functions should be called after this method. Before any drawing can take place,
		a function call to BeginPaint() must be issued.

		\sa BeginPaint()
	*/
	virtual bool EndPaint() = 0;

	//! Used to render common sprites
	virtual void Render(const ITexture *texture, const RECT &rectSrc, const CPoint &pointDest, int rotate=SROTATE_0, int transform=0, int alpha=255, IBuffer **buffer=NULL) const = 0;

	//! Used to render tiled sprites
	virtual void Render(const ITexture *texture, const RECT &rectSrc, const RECT &rectDest, int rotate=SROTATE_0, int transform=0, int alpha=255, IBuffer **buffer=NULL) const = 0;

	virtual void Clear(const RECT *rectDest, BYTE alpha, BYTE red, BYTE green, BYTE blue) const = 0;

	virtual void FillRect(const RECT &rectDest, BYTE alpha, BYTE red, BYTE green, BYTE blue) const = 0;
	virtual void DrawRect(const RECT &rectDest, BYTE alpha, BYTE red, BYTE green, BYTE blue, int bordersize=1) const = 0;

	/*! \brief Draws a bounding box on the screen at a given place in the world.

		\param rectDest	Where the bounding box is going to be drew.
		\param alpha, red, green, and blue Alpha, red, green, and blue components of the color. 
			These values must be in the range 0 to 255. 

		\remarks 
		rectDest is a bounding rectangle giving the bounding box. All rectangles are
		of inclusive-exlusive type. A bounding is a simple rectangle drew on the screen
		with a one pixel line-thikness. Any zoom in the view should not affect the
		thickness of the lines compssing the bounding box.
		Before any drawing can take place, a function call to BeginPaint() must be issued.

		\sa BeginPaint()
	*/
	virtual void BoundingBox(const RECT &rectDest, BYTE alpha, BYTE red, BYTE green, BYTE blue) const = 0;

	/*! \brief Draws a selection box on the screen at a given place in the world.

		\param rectDest	Where the selection box is going to be drew.
		\param alpha, red, green, and blue Alpha, red, green, and blue components of the color. 
			These values must be in the range 0 to 255. 

		\remarks 
		rectDest is a bounding rectangle giving the selction box. All rectangles are
		of inclusive-exlusive type. A selection box contains is in essence a bounding box 
		with small selection squares in the corners and in the middles to give the sense
		that the selection can be resized.
		Before any drawing can take place, a function call to BeginPaint() must be issued.

		\sa BoundingBox(), BeginPaint()
	*/
	virtual void SelectionBox(const RECT &rectDest, BYTE alpha, BYTE red, BYTE green, BYTE blue) const = 0;

	virtual bool CreateTextureFromFile(LPCSTR filename, ITexture **texture) = 0;

	virtual DWORD GetDeviceID() const = 0;
};
/////////////////////////////////////////////////////////////////////////////
/*! \interface	IScriptableObject
	\brief		Interface for scriptable objects.
	\author		Kronuz
	\version	1.0
	\date		April 16, 2003

	IScriptableObject allows the object to be driven by by a script. It provides a
	default implementation for the ExecuteScript member which logs the 
	unexecuted script directly to a logging file.
*/
interface IScriptableObject 
{
	/*! \brief Enumaration of all sprite types.
	
		Override this and handle object specific scripts here. If your class
		handless the script then return true. else delegate the script to
		the base class, returning the result.
	*/
	virtual bool ExecuteScript(CString sCommand);
};

/////////////////////////////////////////////////////////////////////////////
/*! \interface	IArchive
	\brief		Interface for concrete implementation disk IO objects.
	\author		Kronuz
	\version	1.0
	\date		April 28, 2003

	IArchive Interface to implement disk read/write operations of specific
	objects. It is an abstract class that must be implemented in any
	derivated class.
*/
interface IArchive
{
	/*! \brief Reads the object from the disk.
	
		Reads the object from the disk or input device. This method fills all necesary
		information and attributes from the object in a file
		\sa Save()
	*/
	virtual bool ReadObject(LPCSTR szFile) = 0;

	/*! \brief Writes the object to the disk.
	
		Writes the object to the disk or output device. This method takes all necesary
		information and attributes from the object and puts them in the file
		for latter load and use of the object.
		\sa Load()
	*/
	virtual bool WriteObject(LPCSTR szFile) = 0;

};

/////////////////////////////////////////////////////////////////////////////
/*! \interface	IDocumentObject
	\brief		Interface for objects with on-disk read write operations.
	\author		Kronuz
	\version	1.0
	\date		April 16, 2003

	IDocumentObject allows the object to be saved or loaded from the 
	Archive interface. It is an abstract class that must be 
	implemented in any derivated class.
*/
interface IDocumentObject 
{
	/*! \brief Reads the object from the IArchive.
	
		Loads or reads the object using an Archive interface derivated object. 
		\sa Save()
	*/
	virtual bool Load(LPCSTR szFile) {
		ASSERT(m_ArchiveIn);
		return m_ArchiveIn->ReadObject(szFile);
	}

	/*! \brief Saves the object to the IArchive.
	
		Saves or writes the object using an Archive interface derivated object. 
		\sa Load()
	*/
	virtual bool Save(LPCSTR szFile) {
		ASSERT(m_ArchiveOut);
		return m_ArchiveOut->WriteObject(szFile);
	}

protected:
	IDocumentObject() : m_ArchiveIn(NULL), m_ArchiveOut(NULL) {}
	IArchive *m_ArchiveIn;
	IArchive *m_ArchiveOut;
};

