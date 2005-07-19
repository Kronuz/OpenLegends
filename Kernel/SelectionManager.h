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
/*
	\todo	FIXME: Merge CDrawableSelection and CSpriteSelection
*/

#pragma once

#include <IGraphics.h>
#include <IConsole.h>

#include <IArchiver.h>

#include "DrawManager.h"
#include <cmath>

#include <Core.h>
#include "Memento.h"

#include <vector>
#include <map>
#include <functional>
#include <algorithm>

#include "SpriteManager.h"

#define SSD_WIDTHHEIGHT	0x01	// 000001
#define SSD_CHAIN_X		0x02	// 000010
#define SSD_CHAIN_Y		0x04	// 000100
#define SSD_TRANS     	0x08	// 001000
#define SSD_ALPHA		0x10	// 010000
#define SSD_RGBL		0x20	// 100000

class CDrawableSelection;

enum _Chain { relative=0, stretch=1, left=3, right=2, up=2, down=3, fixed=4 };

struct SObjProp :
	public IPropertyEnabled
{
	CDrawableSelection *pSelection;
	CDrawableContext *pContext;

	std::vector<int> C; // This is the list of group children the object has.

	int nGroup; // what group does it belong to? (0 = no group, its group number if it's a group)
	bool bSubselected;
	CRect rcRect; // this must always be updated from Resizes and other position changes
	_Chain eXChain;
	_Chain eYChain;
	SObjProp(CDrawableSelection *pSelection_, CDrawableContext *pContext_, int nGroup_, const CRect &Rect_, _Chain eXChain_, _Chain eYChain_) : pSelection(pSelection_), pContext(pContext_), bSubselected(true), rcRect(Rect_), eXChain(eXChain_), eYChain(eYChain_), nGroup(nGroup_) {}
	SObjProp(CDrawableSelection *pSelection_, CDrawableContext *pContext_, int nGroup_, const CRect &Rect_) : pSelection(pSelection_), pContext(pContext_), bSubselected(true), eXChain(relative), eYChain(relative), nGroup(nGroup_) {}
	SObjProp(CDrawableSelection *pSelection_, CDrawableContext *pContext_, int nGroup_) : pSelection(pSelection_), pContext(pContext_), bSubselected(true), eXChain(relative), eYChain(relative), nGroup(nGroup_) {
		if(pContext) pContext->GetAbsFinalRect(rcRect);
		else ASSERT(0); // You should provide Rect_ for groups.;
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
	\author		Germán Méndez Bravo (Kronuz)
	\version	1.0
	\date		May 31, 2003

	CDrawableSelection is used to provide a selection class. This class
	manages selections of the objects, as well as changes in their size
	and location. Many objects can be selected at the same time,
	and this class can maintain each of them.

	\sa CDrawableContext, CDrawableObject
*/
class CDrawableSelection :
	public CMutable
{
	friend SObjProp;

	void EndSubSelBox(bool bAdd, const CPoint &point_, int Chains);
	void SubSelPoint(bool bAdd, const CPoint &point_, int Chains);

	IPropertyEnabled* SelPointAdd(const CPoint &point_, int Chains);
	void SelPointRemove(const CPoint &point_);

protected:
	const struct ObjPropContextEqual : 
	public std::binary_function<SObjProp, const CDrawableContext*, bool> {
		bool operator()(const SObjProp &a, const CDrawableContext *b) const;
	} m_equalContext;

	const struct ObjPropGroupEqual : 
	public std::binary_function<SObjProp, int, bool> {
		bool operator()(const SObjProp &a, int b) const;
	} m_equalGroup;

	const struct ObjPropLayerEqual : 
	public std::binary_function<SObjProp, int, bool> {
		bool operator()(const SObjProp &a, const int &b) const;
	} m_equalLayer;

	const struct SelectionCompare : 
	public std::binary_function<SObjProp, SObjProp, bool> {
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
	typedef std::vector<SObjProp> vectorObject;
	vectorObject::iterator m_CurrentSel;

	struct SGroup {
		std::string Name; // This is the name of the group. (It's either a regular name or a relative path to a sprite set)
		vectorObject O; // This is the actual vector of objects.
		int P; // This is the address to the parent group.
		SGroup() : P(0) {}
	};

	int m_nPasteGroup;
	int m_nCurrentGroup;
	std::vector<SGroup> m_Groups;

	int GetBoundingRect(CRect *pRect_, int nGroup_ = 0);
	int FindInGroup(vectorObject::iterator *Iterator, CDrawableContext *pDrawableContext, int nGroup_);

	virtual void ResizeObject(const SObjProp &ObjProp_, const CRect &rcOldBounds_, const CRect &rcNewBounds_, bool bAllowResize_) = 0;
	virtual void BuildRealSelectionBounds(int nGroup_ = 0) = 0;

	void SortSelection();
	int SetLayerSelection(int nLayer, int nGroup_);
	int DeleteSelection(int nGroup_);

	bool BeginPaint(IGraphics *pGraphicsI, WORD wFlags = 0);
	bool DrawAll(IGraphics *pGraphicsI);
	bool EndPaint(IGraphics *pGraphicsI);

	void SetInitialMovingPoint(const CPoint &point_);

	bool SelectGroup(int nGroup_, bool bSelectContext = true);
	bool SelectGroupWith(const CDrawableContext *pDrawableContext);
	bool SelectGroupWithIn(const CRect &rect_, const CDrawableContext *pDrawableContext);
	void DeleteInGroups(const CDrawableContext *pDrawableContext);

public:
	CDrawableSelection(CDrawableContext **ppDrawableContext_);
	virtual ~CDrawableSelection() {
		BEGIN_DESTRUCTOR
		delete []m_pBitmap;
		END_DESTRUCTOR
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
	virtual int RealCount(int nGroup_ = 0);

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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma pack(1)
struct _SpriteSet {
	struct _SpriteSetInfo {
		_OpenLegendsFile Header;
		UINT nSelected;
		CRect rcBoundaries;
	} Info;

	// Here comes the index. A list of offsets (from the begining to the start of the name)...
	// WORD Offset_to_the_first_name;
	// WORD Offset_to_the_second_name;
	//             ...
	// WORD Offset_to_the_last_name;

	// Here comes the NULL terminated strings of the names (referred by the offsets above)...

	// Here starts the data:
	struct _SpriteSetData {	// (7 bytes)
		WORD Mask :		6;
		WORD Layer :	3;
		WORD SubLayer :	3;
		WORD ObjIndex :	12;
		WORD X :		16;
		WORD Y :		16;
	};
	struct _SpriteSetData01 { // mask SSD_WIDTHHEIGHT	(4 bytes)
		WORD Width :	16;
		WORD Height :	16;
	};
	struct _SpriteSetData02 { // masks SSD_CHAIN_X, SSD_CHAIN_Y, and SSD_TRANS (1 byte)
		BYTE rotation :	2;
		BYTE mirrored :	1;
		BYTE flipped :	1;
		BYTE XChain :	2; // = Xchain - 1
		BYTE YChain :	2; // = Ychain - 1
	};
	struct _SpriteSetData03 { // mask SSD_ALPHA (1 byte)
		BYTE Alpha;
	};
	struct _SpriteSetData04 { // mask SSD_RGBL (4 bytes)
		BYTE Red;
		BYTE Green;
		BYTE Blue;
	};
	// ...the bitmap continues here (starts in a 16 bytes alignment)
};
#pragma pack()

class CSpriteSelection :
	public CDrawableSelection
{
	bool m_bHighlightOnly;

	void ResizeObject(const SObjProp &ObjProp_, const CRect &rcOldBounds_, const CRect &rcNewBounds_, bool bAllowResize_);
	void BuildRealSelectionBounds(int nGroup_ = 0);

	// Pastes a buffer in the specified point, without selecting it, and returns a
	// rect with the ending location of the pasted buffer (empty on fail)
	CRect PasteSpriteSet(CLayer *pLayer, const LPBYTE pRawBuffer, const CPoint *pPoint = NULL, bool bPaste = true);
	CRect PasteFile(CLayer *pLayer, LPCSTR szFilePath, const CPoint *pPoint = NULL, bool bPaste = true);
	CRect PasteSprite(CLayer *pLayer, LPCSTR szSprite, const CPoint *pPoint = NULL, bool bPaste = true);
	CRect PasteSprite(CLayer *pLayer, CSprite *pSprite, const CPoint *pPoint = NULL, bool bPaste = true) ;

public:
	CSpriteSelection(CDrawableContext **ppDrawableContext_) : 
	  CDrawableSelection(ppDrawableContext_), m_bHighlightOnly(false) 
	  {
	  }

	// Interface Definition:
	virtual void SetHighlightMode(bool bHighlight = true) { m_bHighlightOnly = bHighlight; }

	virtual void SelectionToGroup(LPCSTR szGroupName = "");
	virtual void GroupToSelection();

	virtual void SelectionToTop();
	virtual void SelectionToBottom();
	virtual void SelectionDown();
	virtual void SelectionUp();

	virtual void FlipSelection();
	virtual void MirrorSelection();
	virtual void CWRotateSelection();
	virtual void CCWRotateSelection();

	virtual bool Draw(const IGraphics *pGraphics_);

	// If no bitmap is provided, no thumbnail bitmap is added to the copy.
	// If a bitmap is provided, and the memory for the bitmap has been allocated 
	// by the kernel you can set bDeleteBitmap to true, so the memory gets deleted 
	// in the copy process. In this case, *ppBitmap is nulled to avoid missuses.
	virtual HGLOBAL Copy(BITMAP **ppBitmap = NULL, bool bDeleteBitmap = false); 
	virtual bool Paste(LPCVOID pBuffer, const CPoint &point_);

	virtual bool FastPaste(LPCVOID pBuffer, const CPoint &point_);
	virtual bool FastPaste(CSprite *pSprite, const CPoint &point_ );

	virtual bool GetPastedSize(LPCVOID pBuffer, SIZE *pSize);
	virtual bool GetPastedSize(CSprite *pSprite, SIZE *pSize);
	
	virtual LPCSTR GetSelectionName(LPSTR szName, int size);
	virtual void SetSelectionName(LPCSTR szName);
};
