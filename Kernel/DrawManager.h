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
/*! \file		DrawManager.h 
	\brief		Interface for CDrawableObject and CDrawableContext.
	\date		April 28, 2003
				September 06, 2003: 
						+ Resorting is not needed when moving or resizing birthOrder type objects.
						  * Optimization 1: Only schedule a sort for sorting types that have
						    anything to do with the position of the context, such as yOrder.
						    This sorting was frequently done, but seldom really needed.
						+ There is no need to create a sorting functor every time.
						  * Optimization 2: Sorting functors where made member objects of the 
						    CDrawableContext class.
				September 06, 2003: 
						+ Object sorting added for selections. (Object map was changed for vector)

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

#include "../IGraphics.h"
#include "../IConsole.h"

#include "../Core.h"

#include <vector>
#include <map>
#include <functional>
#include <algorithm>

using namespace std;

#define MAX_SUBLAYERS	10	// Maximum Sublayers
#define MAX_LAYERS		10	// Maximum Layers
#define DEFAULT_LAYER	3	// Default layer

#define CONTEXT_BUFFERS 2
/////////////////////////////////////////////////////////////////////////////
// Forward declarations
class CDrawableObject;
class CDrawableSelection;

enum DRAWTYPE { birthOrder, yOrder, leftIso, rightIso, noOrder };

// Functor class to easily delete objects within containers:
struct ptr_delete {
	template<typename Type>
	void operator()(const Type *ptr) const {
		delete ptr;
	}
};
template<class Type>
struct ptr_equal_to : public binary_function<Type, Type, bool> 
{
	bool operator()(const Type& _Left, const Type& _Right) const {
		return(*_Left == *_Right);
	}
};

// This struct is used at run context time, it gives the required
// information to the run procedures.
typedef struct __RUNACTION {
	HANDLE hSemaphore;		// Semaphore for the run (if needed.)
	bool bJustWait;			// Just wait for the running process to end (or kill them.)
	LPVOID m_Ptr;			// Multipurpose pointer passed to the run procedures.
} RUNACTION;


/////////////////////////////////////////////////////////////////////////////
/*! \class		CDrawableContext
	\brief		Flyweight drawable-objects context class.
	\author		Kronuz
	\version	1.0
	\date		April 28, 2003

	This class maintains the extrinsic status of flyweight drawable-objects. 
	It is an abstract class that must be implemented in any derivated class.
*/
class CDrawableContext :
	public CNamedObj,
	public IPropertyEnabled
{
protected:
	const struct ContextSubLayerCompare : 
	public binary_function<const CDrawableContext*, const CDrawableContext*, bool> {
		bool operator()(const CDrawableContext *a, const CDrawableContext *b) const;
	} m_cmpSubLayer; // *Optimization (2)
	const struct ContextYCompare :
	public binary_function<const CDrawableContext*, const CDrawableContext*, bool> {
		bool operator()(const CDrawableContext *a, const CDrawableContext *b) const;
	} m_cmpY; // *Optimization (2)
	const struct ContextYXCompare :
	public binary_function<const CDrawableContext*, const CDrawableContext*, bool> {
		bool operator()(const CDrawableContext *a, const CDrawableContext *b) const;
	} m_cmpYX; // *Optimization (2)
	const struct ContextYiXCompare :
	public binary_function<const CDrawableContext*, const CDrawableContext*, bool> {
		bool operator()(const CDrawableContext *a, const CDrawableContext *b) const;
	} m_cmpYiX; // *Optimization (2)
	const struct ContextOrderCompare :
	public binary_function<const CDrawableContext*, const CDrawableContext*, bool> {
		bool operator()(const CDrawableContext *a, const CDrawableContext *b) const;
	} m_cmpOrder; // *Optimization (2)

	const class DrawContext :
	public binary_function<CDrawableContext*, const IGraphics *, bool> {
		bool m_bHighlight;
		bool m_bSelected;
		bool m_bVisible;
	public:
		DrawContext(bool bVisible, bool bSelected, bool bHighlight) : m_bVisible(bVisible), m_bSelected(bSelected), m_bHighlight(bHighlight) {}
		bool operator()(CDrawableContext *pDrawableContext, const IGraphics *pIGraphics) const;
	};

	const class RunContext :
	public binary_function<CDrawableContext*, RUNACTION, bool> {
		bool m_bVisible;
	public:
		RunContext(bool bVisible) : m_bVisible(bVisible) {}
		bool operator()(CDrawableContext *pDrawableContext, RUNACTION action) const;
	};

	const class CleanTempContext :
	public unary_function<CDrawableContext*, bool> {
	public:
		bool operator()(CDrawableContext *pDrawableContext) const;
	};
	
	friend ContextSubLayerCompare;
	friend ContextYXCompare;
	friend ContextYiXCompare;
	friend ContextOrderCompare;
	
	friend DrawContext;
	friend RunContext;
	friend CleanTempContext;

private:
	const IGraphics *m_pIGraphics;

	vector<CDrawableContext *> m_Children;
	vector<CDrawableContext *>::iterator m_LayersMap[MAX_SUBLAYERS+2];

	vector<CDrawableContext *>::reverse_iterator m_ChildIterator;
	vector<CDrawableContext *>::reverse_iterator m_LastChildIteratorUsed;
	size_t m_nInsertion;

	DRAWTYPE m_eDrawType[MAX_SUBLAYERS];	//!< Ordering type for child sprites.
	DRAWTYPE m_eSorted[MAX_SUBLAYERS];		//!< The current sort of the children.
	bool m_bValidMap;						//!< Indicates if the layers map is valid. (also if iterators are valid)
	size_t m_nOrder;						//!< Number of siblings at the time of the creation.

protected:
	CDrawableObject *m_pDrawableObj;		//!< Drawable object for the context (if any)
	ARGBCOLOR m_rgbBkColor;					//!< If there is no drawable object, a background color should exist

	int m_nSubLayer;						//!< Object's current sub layer (relative to the layer)
	CPoint m_Position;						//!< Object's position.

	mutable CSize m_Size;			//!< If the context's size is 0,0 then the size is obtained from the drawable object.
	CDrawableContext *m_pParent;	//!< parent drawable object.
	DWORD m_dwStatus;
	bool m_bSelected;

	void PreSort();
	void Sort(int nSubLayer);

	bool __Draw(const IGraphics *pIGraphics);

	bool GetFirstChildAt(int nSubLayer, const CPoint &point_, CDrawableContext **ppDrawableContext_);
	bool GetNextChildAt(int nSubLayer, const CPoint &point_, CDrawableContext **ppDrawableContext_);

	bool GetFirstChildIn(int nSubLayer, const RECT &rect_, CDrawableContext **ppDrawableContext_);
	bool GetNextChildIn(int nSubLayer, const RECT &rect_, CDrawableContext **ppDrawableContext_);

	bool AddSibling(CDrawableContext *object, bool bAllowDups_=true);
	bool AddChild(CDrawableContext *object, bool bAllowDups_=true);
	bool InsertChild(CDrawableContext *object, int nInsertion = -1);
public:

	mutable LPVOID m_pPtr;							//!< Multipurpose pointer for the drawable context.
	mutable IBuffer *m_pBuffer[CONTEXT_BUFFERS];	//!< Buffers for the drawable context (to use as needed)

	// reorder the objects, leaving a space between nRoomAt and nRoomAt+nRoomSize. 
	// Returns the next availible free Ordering position (i.e. nRoomAt if specified)
	// leaves holes between objects of nStep size (nStep must be >= 1)
	int ReOrder(int nStep, int nRoomAt = -1, int nRoomSize = 0);

	void Invalidate() {
		for(int i=0; i<CONTEXT_BUFFERS; i++) {
			if(m_pBuffer[i]) m_pBuffer[i]->Invalidate(true);
		}
	}
	void Touch() {
		for(int i=0; i<CONTEXT_BUFFERS; i++) {
			if(m_pBuffer[i]) m_pBuffer[i]->Touch();
		}
	}

	CDrawableContext(LPCSTR szName="");
	virtual ~CDrawableContext();

	const CBString& GetObjName() const;

	void SetSize(const CSize &_size);
	void SetSize(int x, int y);
	void GetSize(CSize &_Size) const;

	bool SetObjSubLayer(int nLayer);
	bool SetObjLayer(int nLayer);

	int GetObjSubLayer() const;
	int GetObjLayer() const;
	int GetObjOrder() const;

	void SetObjOrder(int nNewOrder); // use this carefully

	CDrawableContext* GetParent() const;
	CDrawableContext* GetSibling(int idx) const;
	CDrawableContext* GetChild(int idx) const;

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
	void SetRect(const CRect &_Rect);
	void SetAbsRect(const CRect &_Rect);
	void SetAbsFinalRect(const CRect &_Rect);

	bool isAt(int x, int y) const;
	bool isAt(const CPoint &_point) const;
	bool isAt(const RECT &_rect) const;

	bool isIn(const RECT &_rect) const;

	const IGraphics* GetGraphicsDevice() const;
	void SetGraphicsDevice(IGraphics *pIGraphics);

	void SetStatus(DWORD dwStatus);
	DWORD GetStatus() const;

	void ShowContext(bool bShow = true);
	bool isVisible() const;

	void SetTemp(bool bTemp = true);
	bool isTemp() const;

	void SelectContext(bool bSelect = true);
	bool isSelected() const;

	void Rotate(bool bRotate = true);
	bool isRotated() const;

	bool GetFirstChildAt(const CPoint &point_, CDrawableContext **ppDrawableContext_);
	bool GetNextChildAt(const CPoint &point_, CDrawableContext **ppDrawableContext_);

	bool GetFirstChildIn(const RECT &rect_, CDrawableContext **ppDrawableContext_);
	bool GetNextChildIn(const RECT &rect_, CDrawableContext **ppDrawableContext_);

	bool PopChild(CDrawableContext *pDrawableContext_);
	bool PopChildEx(CDrawableContext *pDrawableContext_); // extensive search of the object in children

	bool KillChild(CDrawableContext *pDrawableContext_);
	bool KillChildEx(CDrawableContext *pDrawableContext_); // extensive search of the object in children

	void Clean(); // recursivelly clean the Drawable context freeing all allocated memory.

	int Objects(int init=0); // How many not-null objects are under this context. 

	void SetDrawableObj(CDrawableObject *pDrawableObj);
	CDrawableObject* GetDrawableObj() const;
	void SetBkColor(ARGBCOLOR rgbColor);
	ARGBCOLOR GetBkColor() const;

	// Interface:
	virtual bool isFlagged() { return false; }
	virtual void Flag(bool bFlag) { bFlag; }
	virtual bool GetInfo(SInfo *pI) const { return false; }
	virtual bool GetProperties(SPropertyList *pPL) const { return false; }
	virtual bool SetProperties(SPropertyList &PL) { return false; }
	virtual void Commit() const {};
	virtual void Cancel() {};

	virtual bool Draw(const IGraphics *pIGraphics=NULL);
	virtual bool DrawSelected(const IGraphics *pIGraphics=NULL);
	virtual bool DrawSelectedH(const IGraphics *pIGraphics=NULL);

	virtual bool Run(RUNACTION action);

	virtual bool CleanTemp();

	// Operators:
	bool operator==(const CDrawableContext &context) const {
		if( m_pDrawableObj == context.m_pDrawableObj &&	m_nSubLayer == context.m_nSubLayer ) {
			CRect thisRect, contextRect;
			GetRect(thisRect);
			context.GetRect(contextRect);
			if(thisRect == contextRect) {
				return true;
			}
		}
		return false;
	}
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
class CDrawableObject :
	public CNamedObj
{
public:
	CDrawableObject(LPCSTR szName="") : CNamedObj(szName) {};

	/*! \brief Run anything about the object.
	
		Runs a command for the drawable object.
	*/
	virtual bool Run(const CDrawableContext &, RUNACTION) {
		return false;
	}

	/*! \brief Draw the object on the screen.
	
		Draw the object to the screen in its current state
		returns true if the object was drawn, or false if it
		was not drawn (this is not necessarily an error - the
		object may have determinated that it was obscured by others)
	*/
	virtual bool Draw(const CDrawableContext &, const ARGBCOLOR * = NULL) {
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

	/*! \brief Obtains a rect represening the objects origin or base point.

		The reference CPoint sended is modified in this function.
		
		\remarks
		This is used for drawing order. If, for instance, isometric or
		Y-Ordered drawing order is selected, the location of the base points
		(probably where the object touches or should touch the ground) is needed.
	*/
	virtual void GetOrigin(CPoint &Point) {
		Point.SetPoint(0, 0);
	}

};
enum _Chain { relative=0, stretch=1, left=3, right=2, up=2, down=3, fixed=4 };

struct SObjProp :
	public IPropertyEnabled
{
	SObjProp *pRef; // Objects can point to a reference of themselves (the real object, if for instance it's in a group)
	CDrawableSelection *pSelection;
	CDrawableContext *pContext;
	bool bSubselected;
	CRect rcRect;
	_Chain eXChain;
	_Chain eYChain;
	SObjProp(CDrawableSelection *pSelection_, CDrawableContext *pContext_, const CRect &Rect_, _Chain eXChain_, _Chain eYChain_) : pSelection(pSelection_), pContext(pContext_), bSubselected(true), rcRect(Rect_), eXChain(eXChain_), eYChain(eYChain_), pRef(NULL) {}
	SObjProp(CDrawableSelection *pSelection_, CDrawableContext *pContext_) : pSelection(pSelection_), pContext(pContext_), bSubselected(true), eXChain(relative), eYChain(relative), pRef(NULL) {
		pContext->GetAbsFinalRect(rcRect);
	}
	virtual bool isFlagged();
	virtual void Flag(bool bFlag);
	virtual bool GetInfo(SInfo *pI) const;
	virtual bool GetProperties(SPropertyList *pPL) const;
	virtual bool SetProperties(SPropertyList &PL);
	virtual void Commit() const;
	virtual void Cancel();
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
class CDrawableSelection
{
	friend SObjProp;

	void EndSubSelBox(bool bAdd, const CPoint &point_, int Chains);
	void SubSelPoint(bool bAdd, const CPoint &point_, int Chains);

	IPropertyEnabled* SelPointAdd(const CPoint &point_, int Chains);
	void SelPointRemove(const CPoint &point_);

protected:
	const struct ObjPropContextEqual : 
	public binary_function<SObjProp, const CDrawableContext*, bool> {
		bool operator()(const SObjProp &a, const CDrawableContext *b) const;
	} m_equalContext;

	const struct ObjPropLayerEqual : 
	public binary_function<SObjProp, int, bool> {
		bool operator()(const SObjProp &a, const int &b) const;
	} m_equalLayer;

	const struct SelectionCompare : 
	public binary_function<SObjProp, SObjProp, bool> {
		bool operator()(const SObjProp &a, const SObjProp &b) const;
	} m_cmpSelection;

	BITMAP *m_pBitmap;

	CURSOR m_CurrentCursor;
	enum _CurrentState { eNone, eSelecting, eMoving, eResizing } 
		m_eCurrentState;

	bool m_bAllowMultiLayerSel;

	bool m_bCursorLeft;
	bool m_bCursorTop;
	bool m_bCursorRight;
	bool m_bCursorBottom;

	bool m_bShowGrid;
	int m_nSnapSize;
	int m_nLayer;

	bool m_bChanged;
	bool m_bModified;
	bool m_bFloating;
	bool m_bCanResize;
	bool m_bCanMove;

	bool m_bHoldSelection;

	CRect m_rcClip;
	ARGBCOLOR m_rgbClipColor;

	CRect m_rcSelection;
	CPoint m_ptInitialPoint;
	CDrawableContext **m_ppMainDrawable;

	bool m_bLockedLayers[MAX_LAYERS]; // keeps the locked layers

	CDrawableContext *m_pLastSelected; // Last context selected.
	typedef vector<SObjProp> vectorObject;
	vectorObject::iterator m_CurrentSel;

	struct SGroup {
		string Name; // This is the name of the group. (It's either a regular name or a relative path to a sprite set)
		vectorObject O; // This is the actual vector of objects.
		vector<int> C; // This is the list of children the group has.
		int P; // This is the address to the parent group.
		SGroup() : P(0) {}
	};

	int m_nPasteGroup;
	int m_nCurrentGroup;
	vector<SGroup> m_Groups; //!< Contexts in the group.

	int GetBoundingRect(CRect *pRect_, int nPasteGroup_ = 0);

	virtual void ResizeObject(const SObjProp &ObjProp_, const CRect &rcOldBounds_, const CRect &rcNewBounds_, bool bAllowResize_) = 0;
	virtual void BuildRealSelectionBounds() = 0;

	void SortSelection();

	bool BeginPaint(IGraphics *pGraphicsI, WORD wFlags = 0);
	bool DrawAll(IGraphics *pGraphicsI);
	bool EndPaint(IGraphics *pGraphicsI);

	void SetInitialMovingPoint(const CPoint &point_);

	bool SelectGroupWith(const CDrawableContext *pDrawableContext);
	void DeleteInGroups(const CDrawableContext *pDrawableContext);

public:
	CDrawableSelection(CDrawableContext **ppDrawableContext_);
	virtual ~CDrawableSelection() {
		delete []m_pBitmap;
	}

	// Interface Definition:
	virtual void CleanPasteGroups();
	virtual int SetNextPasteGroup(LPCSTR szGroupName, int nNew = -1);

	virtual LPCSTR GetSelectionName(LPSTR szName, int size) = 0;
	virtual void SetSelectionName(LPCSTR szName) = 0;

	virtual SObjProp* GetFirstSelection();
	virtual SObjProp* GetNextSelection();

	virtual CDrawableContext* GetLastSelected() { return m_pLastSelected; }

	virtual void HoldSelection(bool bHold = true) { if(bHold) SortSelection(); m_bHoldSelection = bHold; }
	virtual bool isHeld() { return m_bHoldSelection; }
	
	virtual void LockLayer(int nLayer, bool bLock = true);
	virtual bool isLocked(int nLayer);
	
	// returns true if the object has been modified from its initial state.
	virtual bool IsModified() { return m_bModified; }

	// returns whether or not the object has changed since last call to hasChanged()
	virtual bool HasChanged() { if(m_bChanged) { m_bChanged = false; return true; } return false; }

	virtual bool SelectedAt(const CPoint &point_); // Is there a selected object at this point?

	virtual void StartResizing(const CPoint &point_);
	virtual void ResizeTo(const CPoint &point_);
	virtual void EndResizing(const CPoint &point_);

	virtual void StartMoving(const CPoint &point_);
	virtual void MoveTo(const CPoint &point_);
	virtual void EndMoving(const CPoint &point_);
	
	virtual void HoldOperation();

	virtual void StartSelBox(const CPoint &point_);
	virtual void CancelSelBox();
	virtual void SizeSelBox(const CPoint &point_);
	virtual IPropertyEnabled* EndSelBoxAdd(const CPoint &point_, int Chains);
	virtual void EndSelBoxRemove(const CPoint &point_);

	virtual void GetSelBounds(CRect *pRect_);
	virtual void SetLayerSelection(int nLayer);
	virtual int DeleteSelection();
	virtual void Cancel();

	virtual void CleanSelection();

	virtual void SetSnapSize(int nSnapSize_, bool bShowGrid_);
	virtual void SetLayer(int nLayer_);
	virtual int GetLayer();

	virtual bool isResizing();
	virtual bool isMoving();
	virtual bool isSelecting();
	virtual bool isFloating();
	virtual bool isGroup();

	virtual int Count();

	virtual bool GetMouseStateAt(const IGraphics *pGraphics_, const CPoint &point_, CURSOR *pCursor);

	// Make abstract methods:
	virtual bool Draw(const IGraphics *pGraphics_) = 0; 
	virtual HGLOBAL Copy(BITMAP **ppBitmap = NULL, bool bDeleteBitmap = false) = 0;
	virtual bool Paste(LPCVOID pBuffer, const CPoint &point_) = 0;

	virtual bool FastPaste(LPCVOID pBuffer, const CPoint &point_) = 0;
	virtual bool GetPastedSize(LPCVOID pBuffer, SIZE *pSize) = 0;

	virtual bool SetClip(const CRect *pRect, ARGBCOLOR rgbColor = COLOR_ARGB(0,0,0,0));
	virtual bool Paint(IGraphics *pGraphicsI, WORD wFlags); // render the map group to the screen
	virtual BITMAP* Capture(IGraphics *pGraphicsI, float zoom); // creates a new BITMAP with the map group
	virtual BITMAP* CaptureSelection(IGraphics *pGraphicsI, float zoom); // creates a new BITMAP with the selection

	virtual BITMAP* GetThumbnail() const { return m_pBitmap; }
	virtual void SetThumbnail(BITMAP *pBitmap) { 
		delete []m_pBitmap;
		m_pBitmap = pBitmap; 
	}
};

// Implementations:
inline const CBString& CDrawableContext::GetObjName() const
{
	static const CBString szNoName = "";
	if(m_pDrawableObj) return m_pDrawableObj->GetName();
	return szNoName;
}
inline void CDrawableContext::SetSize(const CSize &_Size) 
{
	if(m_Size != _Size) {
		m_Size = _Size;
		Invalidate();
	}
}
inline void CDrawableContext::SetSize(int x, int y) 
{ 
	if(m_Size.cx != x || m_Size.cy != y) {
		m_Size.SetSize(x, y); 
		Invalidate();
	}
}
inline void CDrawableContext::GetSize(CSize &_Size) const 
{ 
	if(m_Size.cx==-1 && m_Size.cy==-1)
		if(m_pDrawableObj) m_pDrawableObj->GetSize(m_Size);
	_Size = m_Size; 
}
inline int CDrawableContext::GetObjSubLayer() const
{
	return m_nSubLayer;
}
inline void CDrawableContext::SetObjOrder(int nNewOrder)
{
	m_nOrder = nNewOrder;
	m_pParent->m_eSorted[m_nSubLayer] = noOrder;
}
inline int CDrawableContext::GetObjOrder() const
{
	return m_nOrder;
}

inline int CDrawableContext::GetObjLayer() const
{
	if(m_pParent) return m_pParent->m_nSubLayer;
	return -1;
}
inline bool CDrawableContext::SetObjLayer(int nLayer) 
{
	bool bRet = false;
	if(m_pParent) {
		if(nLayer == m_pParent->m_nSubLayer) 
			return true; //same current layer

		CDrawableContext *pNewLayer = m_pParent->GetSibling(nLayer);
		if(pNewLayer) { // if the new layer exists, try changing the layer:
			if(m_pParent->PopChild(this)) {
				bRet = pNewLayer->InsertChild(this, m_nOrder);
				ASSERT(bRet);
			}
		}
	}
	return bRet;
}
inline bool CDrawableContext::SetObjSubLayer(int nLayer) 
{ 
	m_nSubLayer = nLayer; 
	if(m_pParent) {
		// let the parent know one of its layers has changed.
		m_pParent->m_bValidMap = false;
		m_pParent->m_eSorted[m_nSubLayer] = noOrder;
	}
	return true;
}
inline void CDrawableContext::MoveTo(int x, int y) 
{ 
	m_Position.SetPoint(x,y); 
	if(m_pParent) {
		if(m_pParent->m_eSorted[m_nSubLayer] != birthOrder) // *Optimization (1)
			m_pParent->m_eSorted[m_nSubLayer] = noOrder;
	}
}
inline void CDrawableContext::MoveTo(const CPoint &_point) 
{
	m_Position = _point;
	if(m_pParent) {
		if(m_pParent->m_eSorted[m_nSubLayer] != birthOrder) // *Optimization (1)
			m_pParent->m_eSorted[m_nSubLayer] = noOrder;
	}
}
inline void CDrawableContext::GetPosition(CPoint &_Point) const 
{ 
	_Point = m_Position; 
}
inline void CDrawableContext::GetAbsPosition(CPoint &_Point) const 
{
	_Point.SetPoint(0,0);
	if(m_pParent) m_pParent->GetAbsPosition(_Point);
	_Point += m_Position;
}
inline void CDrawableContext::GetRect(CRect &_Rect) const 
{
	if(m_Size.cx==-1 && m_Size.cy==-1)
		if(m_pDrawableObj) m_pDrawableObj->GetSize(m_Size);
	_Rect.SetRect(m_Position, m_Position + m_Size);
}
inline void CDrawableContext::GetAbsRect(CRect &_Rect) const 
{
	CPoint Position;
	GetAbsPosition(Position);
	if(m_Size.cx==-1 && m_Size.cy==-1)
		if(m_pDrawableObj) m_pDrawableObj->GetSize(m_Size);
	_Rect.SetRect(Position, Position + m_Size);
}
inline void CDrawableContext::GetAbsFinalRect(CRect &_Rect) const 
{
	GetAbsRect(_Rect);
	if(isRotated()) {
		int w = _Rect.Width();
		int h = _Rect.Height();
		_Rect.bottom = _Rect.top+w;
		_Rect.right = _Rect.left+h;
	}
}
inline void CDrawableContext::SetRect(const CRect &_Rect) 
{
	m_Position = _Rect.TopLeft();
	if(m_pParent) {
		if(m_pParent->m_eSorted[m_nSubLayer] != birthOrder) // *Optimization (1)
			m_pParent->m_eSorted[m_nSubLayer] = noOrder;
	}

	if(m_Size.cx != _Rect.Width() || m_Size.cy != _Rect.Height()) {
		m_Size.SetSize(_Rect.Width(), _Rect.Height());
		Invalidate(); // Invalidate only if the size changes.
	}
}
inline void CDrawableContext::SetAbsRect(const CRect &_Rect)
{
	CPoint Position(0,0);
	if(m_pParent) m_pParent->GetAbsPosition(Position);

	m_Position = _Rect.TopLeft() - Position;
	if(m_pParent) {
		if(m_pParent->m_eSorted[m_nSubLayer] != birthOrder) // *Optimization (1)
			m_pParent->m_eSorted[m_nSubLayer] = noOrder;
	}

	if(m_Size.cx != _Rect.Width() || m_Size.cy != _Rect.Height()) {
		m_Size.SetSize(_Rect.Width(), _Rect.Height());
		Invalidate();
	}
}
inline void CDrawableContext::SetAbsFinalRect(const CRect &_Rect)
{
	CPoint Position(0,0);
	if(m_pParent) m_pParent->GetAbsPosition(Position);

	m_Position = _Rect.TopLeft() - Position;
	if(m_pParent) {
		if(m_pParent->m_eSorted[m_nSubLayer] != birthOrder) // *Optimization (1)
			m_pParent->m_eSorted[m_nSubLayer] = noOrder;
	}

	if(isRotated()) {
		if(m_Size.cx != _Rect.Height() || m_Size.cy != _Rect.Width()) {
			m_Size.SetSize(_Rect.Height(), _Rect.Width());
			Invalidate();
		}
	} else {
		if(m_Size.cx != _Rect.Width() || m_Size.cy != _Rect.Height()) {
			m_Size.SetSize(_Rect.Width(), _Rect.Height());
			Invalidate();
		}
	}
}
inline bool CDrawableContext::isAt(int x, int y) const 
{ 
	return isAt(CPoint(x, y)); 
}
inline bool CDrawableContext::isAt(const CPoint &_point) const 
{ 
	if(m_pDrawableObj==NULL) return false;
	CRect Rect;
	GetAbsFinalRect(Rect);
	if(Rect.IsRectNull()) return false;
	return ( _point.x>=Rect.left && _point.x<Rect.right &&
			 _point.y>=Rect.top  && _point.y<Rect.bottom );

}
inline bool CDrawableContext::isAt(const RECT &_rect) const 
{
	if(m_pDrawableObj==NULL) return false;
	CRect Rect;
	GetAbsFinalRect(Rect);
	if(Rect.IsRectNull()) return false;
	return (Rect.IntersectRect(Rect, &_rect)==TRUE);
}
inline bool CDrawableContext::isIn(const RECT &_rect) const 
{
	if(m_pDrawableObj==NULL) return false;
	CRect Rect;
	GetAbsFinalRect(Rect);
	if(Rect.IsRectNull()) return false;
	return ( Rect.top>=_rect.top && Rect.bottom<=_rect.bottom &&
			 Rect.left>=_rect.left && Rect.right<=_rect.right );
}
inline const IGraphics* CDrawableContext::GetGraphicsDevice() const 
{ 
	return m_pIGraphics; 
}
inline void CDrawableContext::SetGraphicsDevice(IGraphics *pIGraphics) 
{ 
	m_pIGraphics = pIGraphics; 
}
inline void CDrawableContext::SetStatus(DWORD dwStatus) 
{ 
	m_dwStatus = dwStatus; 
}
inline DWORD CDrawableContext::GetStatus() const 
{ 
	return m_dwStatus; 
}
inline void CDrawableContext::Rotate(bool bRotate)
{
	if(bRotate)	m_dwStatus |= DROTATE;
	else		m_dwStatus &= ~DROTATE;
}
inline bool CDrawableContext::isRotated() const
{
	return ((m_dwStatus&DROTATE) == DROTATE);
}
inline void CDrawableContext::ShowContext(bool bShow) 
{
	if(bShow)	m_dwStatus |= (DVISIBLE<<_DRW_SHFT);
	else		m_dwStatus &= ~(DVISIBLE<<_DRW_SHFT);
}
inline bool CDrawableContext::isVisible() const 
{ 
	return ((m_dwStatus&(DVISIBLE<<_DRW_SHFT))==(DVISIBLE<<_DRW_SHFT)); 
}

inline void CDrawableContext::SetTemp(bool bTemp) 
{
	if(bTemp)	m_dwStatus |= (DTEMP<<_DRW_SHFT);
	else		m_dwStatus &= ~(DTEMP<<_DRW_SHFT);
}
inline bool CDrawableContext::isTemp() const 
{ 
	return ((m_dwStatus&(DTEMP<<_DRW_SHFT))==(DTEMP<<_DRW_SHFT)); 
}

inline void CDrawableContext::SelectContext(bool bSelect)
{
	m_bSelected = bSelect;
}
inline bool CDrawableContext::isSelected() const
{
	return m_bSelected;
}

inline CDrawableContext* CDrawableContext::GetParent() const
{
	return m_pParent;
}

inline CDrawableContext* CDrawableContext::GetSibling(int idx) const
{
	if(m_pParent == NULL) return NULL;
	return m_pParent->GetChild(idx);
}

inline CDrawableContext* CDrawableContext::GetChild(int idx) const
{
	if(idx >= (int)m_Children.size() || idx<0) return NULL;
	return m_Children[idx];
}
inline void CDrawableContext::SetDrawableObj(CDrawableObject *pDrawableObj) 
{ 
	m_pDrawableObj = pDrawableObj; 
}
inline CDrawableObject* CDrawableContext::GetDrawableObj() const
{ 
	return m_pDrawableObj; 
}
inline void CDrawableContext::SetBkColor(ARGBCOLOR rgbColor) 
{ 
	m_rgbBkColor = rgbColor;
}
inline ARGBCOLOR CDrawableContext::GetBkColor() const
{ 
	return m_rgbBkColor; 
}
