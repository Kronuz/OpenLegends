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

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
class CDrawableObject;

enum DRAWTYPE { topView, yOrder, leftIso, rightIso };

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
	const struct ContextZYXCompare {
		bool operator()(CDrawableContext* a, CDrawableContext* b) {
			if(a->m_nSubLayer < b->m_nSubLayer) return true;
			if(a->m_nSubLayer > b->m_nSubLayer) return false;
			if(a->m_Position.y < b->m_Position.y) return true;
			if(a->m_Position.y > b->m_Position.y) return false;
			if(a->m_Position.x < b->m_Position.x) return true;
			return false;
		}
	};
	const struct ContextZYiXCompare {
		bool operator()(CDrawableContext* a, CDrawableContext* b) {
			if(a->m_nSubLayer < b->m_nSubLayer) return true;
			if(a->m_nSubLayer > b->m_nSubLayer) return false;
			if(a->m_Position.y < b->m_Position.y) return true;
			if(a->m_Position.y > b->m_Position.y) return false;
			if(a->m_Position.x > b->m_Position.x) return true;
			return false;
		}
	};
	const struct ContextOrderCompare {
		bool operator()(CDrawableContext* a, CDrawableContext* b) {
			if(a->m_nSubLayer < b->m_nSubLayer) return true;
			if(a->m_nSubLayer > b->m_nSubLayer) return false;
			if(a->m_nOrder < b->m_nOrder) return true;
			return false;
		}
	};
	friend ContextZYXCompare;
	friend ContextZYiXCompare;
	friend ContextOrderCompare;

private:
	IGraphics *m_pIGraphics;

	CDrawableContext *m_pParent;	//! parent drawable object
	vector<CDrawableContext*> m_Children;
	vector<CDrawableContext *>::iterator m_ChildIterator;
	vector<CDrawableContext *>::iterator m_LastChildIteratorUsed;
	int m_nChildren;

	CDrawableObject *m_pDrawableObj;

	DRAWTYPE m_eDrawType;	//!< Ordering type for child sprites.
	int m_nOrder;			//!< Number of sprites added before this one to the layer.
	int m_nSubLayer;		//!< Object's sub layer (relative to the layer)
	CPoint m_Position;		//!< Object's position.

	mutable CSize m_Size;	//!< If the context's size is 0,0 then the size is obtained from the drawable object.

protected:
	DWORD m_dwStatus;

	void AddChild(CDrawableContext *object);

public:
	IBuffer *m_pBuffer; //!< Buffer for the drawable context (to use as needed)

	CDrawableContext();
	~CDrawableContext();

	void SetSize(const SIZE &_size);
	void SetSize(int x, int y);
	void GetSize(CSize &_Size) const;

	void SetSubLayer(int layer);

	void MoveTo(int x, int y);
	void MoveTo(const POINT &_point);

	void GetPosition(CPoint &_Point) const;
	void GetAbsPosition(CPoint &_Point) const;

	void GetRect(CRect &_Rect) const;
	void GetAbsRect(CRect &_Rect) const;
	void GetAbsFinalRect(CRect &_Rect) const;

	void SetRect(const RECT &_rect);

	bool isAt(int x, int y) const;
	bool isAt(const POINT &_point) const;
	bool isAt(const RECT &_rect) const;

	bool isIn(const RECT &_rect) const;

	IGraphics* GetGraphicsDevice() const;
	void SetGraphicsDevice(IGraphics *pIGraphics);

	void SetStatus(DWORD dwStatus);
	DWORD GetStatus() const;

	void ShowSprite(bool bShow = true);
	bool isVisible() const;

	bool Draw(IGraphics *pIGraphics=NULL);

	bool GetFirstChildAt(const POINT &point_, CDrawableContext **ppDrawableContext_);
	bool GetNextChildAt(const POINT &point_, CDrawableContext **ppDrawableContext_);

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
		Rect.SetRect(0,0,0,0);
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
	eIDC_ARROWDEL
};

class CDrawableSelection
{
	CSimpleArray<CDrawableContext*> m_Objects; //!< Sprites in the selection.

	CDrawableContext **m_ppMainDrawable;
	CRect m_rcSelection;
	bool m_bSelecting;

public:
	CDrawableSelection(CDrawableContext **ppDrawableContext_);
	void StartSelection(const POINT &point_);
	void DragSelection(const POINT &point_);
	void CancelSelection();
	void EndSelectionAdd(const POINT &point_);
	void PointSelectionAdd(const POINT &point_);
	void PointSelectionRemove(const POINT &point_);
	void EndSelectionRemove(const POINT &point_);
	void CleanSelection();
	 
	CURSOR GetMouseStateAt(const IGraphics *pGraphics_, const POINT &point_);
	virtual void Draw(const IGraphics *pGraphics_);
};