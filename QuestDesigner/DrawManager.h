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
/*! \file		DrawManager.h 
	\brief		Interface for CDrawableObject and CDrawableContext.
	\date		April 28, 2003

	The interface CDrawableObject represents an object that can, somehow, be draw on
	the screen. This drawing is done using a flyweight pool of "drawable" objects to save
	memory. For example, many objects in the world may exist that represent an wood chair,
	but only one picture or sprite of that chair is acctually needed to show them all.
	CDrawableObject defines the interface to access the common part of the objects
	that can be drawn, while CDrawableContext defines the interface to handle each instance
	of the object acctually drawn on the screen; It is keeped a list of the "common" parts 
	of all drawable objects, we call pool to this list, and it is a flyweight pool because 
	it only	keeps enough information about each drawable object as needed, but still makes the
	real instances of the objects smaller, for they no longer need to keep information that is 
	common for all of them. Real objects are managed by the CDrawableContext interface, and as 
	its name suggests, it contains only information about the context of the drawable object 
	with all of its properties (location, size, display behavior, etc.)

	CDrawableContext and CDrawableObject are the basic implementation of the explained interfaces.
*/

#pragma once
#include "Interfaces.h"
#include "Console.h"

#define MAX_SUBLAYERS 6

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
class CDrawableObject;

enum DRAWTYPE { birthOrder, yOrder, leftIso, rightIso, noOrder };

/////////////////////////////////////////////////////////////////////////////
/*! \class		CDrawableContext
	\brief		Flyweight drawable-objects context class.
	\author		Kronuz
	\version	1.0
	\date		April 28, 2003

	This class maintains the extrinsic status of flyweight drawable-objects. 
	It is an abstract class that must be implemented in any derivated class.
*/
class CDrawableContext
{
	const struct ContextSubLayerCompare {
		bool operator()(CDrawableContext* a, CDrawableContext* b);
	};
	const struct ContextYCompare {
		bool operator()(CDrawableContext* a, CDrawableContext* b);
	};
	const struct ContextYXCompare {
		bool operator()(CDrawableContext* a, CDrawableContext* b);
	};
	const struct ContextYiXCompare {
		bool operator()(CDrawableContext* a, CDrawableContext* b);
	};
	const struct ContextOrderCompare {
		bool operator()(CDrawableContext* a, CDrawableContext* b);
	};
	friend ContextSubLayerCompare;
	friend ContextYXCompare;
	friend ContextYiXCompare;
	friend ContextOrderCompare;

private:
	IGraphics *m_pIGraphics;

	vector<CDrawableContext*> m_Children;
	vector<CDrawableContext *>::iterator m_LayersMap[MAX_SUBLAYERS+2];
	vector<CDrawableContext *>::reverse_iterator m_LayersRMap[MAX_SUBLAYERS+2];

	vector<CDrawableContext *>::reverse_iterator m_ChildIterator;
	vector<CDrawableContext *>::reverse_iterator m_LastChildIteratorUsed;
	int m_nChildren;

	CDrawableObject *m_pDrawableObj;

	DRAWTYPE m_eDrawType[MAX_SUBLAYERS];	//!< Ordering type for child sprites.
	DRAWTYPE m_eSorted[MAX_SUBLAYERS];		//!< The current sort of the children.
	bool m_bValidMap;						//!< Indicates if the layers map is valid.
	int m_nOrder;							//!< Number of siblings at the time of the creation.

protected:
	int m_nSubLayer;						//!< Object's current sub layer (relative to the layer)
	CPoint m_Position;						//!< Object's position.

	mutable CSize m_Size;			//!< If the context's size is 0,0 then the size is obtained from the drawable object.
	CDrawableContext *m_pParent;	//! parent drawable object
	DWORD m_dwStatus;

	bool AddSibling(CDrawableContext *object);
	bool AddChild(CDrawableContext *object);
	void PreSort();
	void Sort(int nSubLayer);

	bool Draw(int nSubLayer, IGraphics *pIGraphics=NULL);

	bool GetFirstChildAt(int nSubLayer, const CPoint &point_, CDrawableContext **ppDrawableContext_);
	bool GetNextChildAt(int nSubLayer, const CPoint &point_, CDrawableContext **ppDrawableContext_);

	bool GetFirstChildIn(int nSubLayer, const RECT &rect_, CDrawableContext **ppDrawableContext_);
	bool GetNextChildIn(int nSubLayer, const RECT &rect_, CDrawableContext **ppDrawableContext_);

public:
	IBuffer *m_pBuffer; //!< Buffer for the drawable context (to use as needed)

	CDrawableContext();
	~CDrawableContext();

	void SetSize(const SIZE &_size);
	void SetSize(int x, int y);
	void GetSize(CSize &_Size) const;

	void SetSubLayer(int layer);

	void MoveTo(int x, int y);
	void MoveTo(const CPoint &_point);

	void GetPosition(CPoint &_Point) const;
	void GetAbsPosition(CPoint &_Point) const;

	void GetRect(CRect &_Rect) const;
	void GetAbsRect(CRect &_Rect) const;
	void GetAbsFinalRect(CRect &_Rect) const;

	/*! \brief Changes the current relative position and relative size of the object.
		\param rect_ Receives the rect marking the new boundaries of the object.
		The <B>SetRect</B> function sets the new object's boundaries, changing the object's
		position and size as necessary. The position of the object, and the boundaries received
		by this function are relative to its parent object.

		\remarks This function expects a normalized RECT.
		\sa GetRect(), GetPosition(), SetPosition(), GetSize(), SetSize(), 
			GetAbsRect(), GetAbsPosition(), GetAbsFinalRect(), MoveTo()
	*/
	void SetRect(const RECT &_rect);
	void SetAbsRect(const CRect &_Rect);
	void SetAbsFinalRect(const CRect &_Rect);

	bool isAt(int x, int y) const;
	bool isAt(const CPoint &_point) const;
	bool isAt(const RECT &_rect) const;

	bool isIn(const RECT &_rect) const;

	IGraphics* GetGraphicsDevice() const;
	void SetGraphicsDevice(IGraphics *pIGraphics);

	void SetStatus(DWORD dwStatus);
	DWORD GetStatus() const;

	void ShowSprite(bool bShow = true);
	bool isVisible() const;

	void Rotate(bool bRotate = true);
	bool isRotated() const;

	bool Draw(IGraphics *pIGraphics=NULL);

	bool GetFirstChildAt(const CPoint &point_, CDrawableContext **ppDrawableContext_);
	bool GetNextChildAt(const CPoint &point_, CDrawableContext **ppDrawableContext_);

	bool GetFirstChildIn(const RECT &rect_, CDrawableContext **ppDrawableContext_);
	bool GetNextChildIn(const RECT &rect_, CDrawableContext **ppDrawableContext_);

	void SetDrawableObj(CDrawableObject *pDrawableObj);
	CDrawableObject* GetDrawableObj();
};

/////////////////////////////////////////////////////////////////////////////
/*! \interface	CDrawableObject
	\brief		Interface for drawable objects.
	\author		Kronuz
	\version	1.0
	\date		May 31, 2003

	CDrawableObject is used to provide an interface that allows a flyweight
	object to be drawn. It is an abstract class that must be implemented in any
	derivated class, and must receive a drawing context containing the extrinsic
	data to the flyweight object.

	\sa CDrawableContext
*/
class CDrawableObject
{
public:
	/*! \brief Draw the object on the screen.
	
		Draw the object to the screen in its current state
		returns true if the object was drawn, or false if it
		was not drawn (this is not necessarily an error - the
		object may have determinated that it was obscured by others)
	*/
	virtual bool Draw(CDrawableContext &) {
		return false;
	}

	/*! \brief Determinates if an object needs to be drawn.
		
		Returns false if the object does not need to be drawn (i.e. off screen)
	*/
	virtual bool NeedToDraw(const CDrawableContext &) {
		return false;
	}

	/*! \brief Obtains the drawable object size

		The reference CSize sended is modified in this function.

		\remarks
		If the drawable object represents a tile, CDrawableObject::GetSize()
		obtains only the size of a single tile.
	*/
	virtual void GetSize(CSize &Size) { 
		Size.SetSize(0, 0);
	}

	/*! \brief Obtains a rect represening the objects base rect or point.

		The reference CRect sended is modified in this function.
		
		\remarks
		This is used for drawing order. If, for instance, isometric or
		Y-Ordered drawing order is selected, the location of the base points
		(probably where the object touches or should touch the ground) is needed.
	*/
	virtual void GetBaseRect(CRect &Rect) {
		Rect.SetRectEmpty();
	}

};

/////////////////////////////////////////////////////////////////////////////
/*! \interface	CDrawableSelection
	\brief		Interface for selected objects.
	\author		Kronuz
	\version	1.0
	\date		May 31, 2003

	CDrawableSelection is used to provide a selection class. This class
	manages selections of the objects, as well as changes in their size
	and location. Many objects can be selected at the same time,
	and this class can maintain each of them.

	\sa CDrawableContext, CDrawableObject
*/
enum CURSOR {
	eIDC_ARROW,
	eIDC_CROSS,
	eIDC_NO,
	eIDC_SIZEALL,
	eIDC_SIZENESW,
	eIDC_SIZENS,
	eIDC_SIZENWSE,
	eIDC_SIZEWE,
	eIDC_ARROWADD,
	eIDC_ARROWDEL,
	eIDC_ARROWSELECT
};

class CDrawableSelection :
	public CConsole
{
	void SelPointAdd(const CPoint &point_);
	void SelPointRemove(const CPoint &point_);

protected:
	CURSOR m_CurrentCursor;
	enum _CurrentState { eNone, eSelecting, eMoving, eResizing } 
		m_eCurrentState;

	bool m_bCursorLeft;
	bool m_bCursorTop;
	bool m_bCursorRight;
	bool m_bCursorBottom;

	int m_nSnapSize;

	bool m_bCanResize;
	bool m_bCanMove;

	CRect m_rcSelection;
	CPoint m_ptInitialPoint;
	CDrawableContext **m_ppMainDrawable;

	CSimpleMap<CDrawableContext *, CRect> m_Objects; //!< Sprites in the selection.
	int GetBoundingRect(CRect &Rect_);

	virtual void ResizeObject(CDrawableContext *Object, const CRect &rcObject_, const CRect &rcOldBounds_, const CRect &rcNewBounds_, bool bAllowResize_) = 0;
	virtual void BuildRealSelectionBounds() = 0;
public:
	CDrawableSelection(CDrawableContext **ppDrawableContext_);

	void StartResizing(const CPoint &point_);
	void ResizeTo(const CPoint &point_);
	void EndResizing(const CPoint &point_);

	void StartMoving(const CPoint &point_);
	void MoveTo(const CPoint &point_);
	void EndMoving(const CPoint &point_);

	void StartSelBox(const CPoint &point_);
	void CancelSelBox();
	void SizeSelBox(const CPoint &point_);
	void EndSelBoxAdd(const CPoint &point_);
	void EndSelBoxRemove(const CPoint &point_);

	void CleanSelection();

	void SetSnapSize(int nSnapSize_);

	bool isResizing();
	bool isMoving();
	bool isSelecting();
	 
	CURSOR GetMouseStateAt(const IGraphics *pGraphics_, const CPoint &point_);
	virtual void Draw(const IGraphics *pGraphics_) = 0;
};