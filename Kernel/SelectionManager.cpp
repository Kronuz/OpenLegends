/* QuestDesigner - Open Legends's Project
   Copyright (C) 2003-2005. Germ�n M�ndez Bravo (Kronuz)
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

#include "stdafx.h"
#include "SelectionManager.h"

#include "GameManager.h"

#include <IGame.h>


//////////////////////////////////////////////////////////////////////////////////////////
// Mouse sensivility for selections:
#define SELSENS_LINE		4	
#define SELSENS_VERTEX		20

bool SObjProp::isFlagged() {
	return bSubselected;
}
void SObjProp::Flag(bool bFlag) { 
	bSubselected = bFlag;
}
bool SObjProp::GetInfo(SInfo *pI) const
{
	if(!pContext) return false;
	bool ret = pContext->GetInfo(pI);
	if(!ret) return false;

	pI->pPropObject = (IPropertyEnabled*)this;
	return true;
}
bool SObjProp::GetProperties(SPropertyList *pPL) const
{
	if(!pContext) return false;
	bool ret = pContext->GetProperties(pPL);
	if(!ret) return false;

	pPL->Information.pPropObject = (IPropertyEnabled*)this;

	pPL->AddList("Horizontal Chain", eXChain, "0 - Relative, 1 - Fill, 2 - Right, 3 - Left, 4 - Fixed");
	pPL->AddList("Vertical Chain", eYChain, "0 - Relative, 1 - Fill, 2 - Up, 3 - Down, 4 - Fixed");

	return true;
}
bool SObjProp::SetProperties(SPropertyList &PL)
{
	if(!pContext) return false;
	SProperty* pP;
	bool bChanged = pContext->SetProperties(PL);
	if(bChanged) pSelection->Touch(false);

	pP = PL.FindProperty("Horizontal Chain", "Behavior", SProperty::ptList);
	if(pP) if(pP->bEnabled && pP->bChanged) {
		if(eXChain != (_Chain)pP->nIndex) {
			eXChain = (_Chain)pP->nIndex;
			bChanged = true;
		}
	}
	
	pP = PL.FindProperty("Vertical Chain", "Behavior", SProperty::ptList);
	if(pP) if(pP->bEnabled && pP->bChanged)  {
		if(eYChain != (_Chain)pP->nIndex) {
			eYChain = (_Chain)pP->nIndex;
			bChanged = true;
		}
	}
	return bChanged;
}
void SObjProp::Commit() const
{
	if(!pContext) return;
	pContext->Commit();
}
void SObjProp::Cancel()
{
	if(!pContext) return;
	pContext->Cancel();
}

inline bool CDrawableSelection::ObjPropContextEqual::operator()(const SObjProp &a, const CDrawableContext *b) const
{
	return (a.pContext == b);
}

inline bool CDrawableSelection::ObjPropGroupEqual::operator()(const SObjProp &a, int b) const
{
	return (a.nGroup == b);
}

inline bool CDrawableSelection::ObjPropLayerEqual::operator()(const SObjProp &a, const int &b) const
{
	ASSERT(a.pContext); // NEED TO FIX *** add compare for groups
	return (a.pContext->GetObjLayer() == b);
}
inline bool CDrawableSelection::SelectionCompare::operator()(const SObjProp &a, const SObjProp &b) const
{
//	ASSERT(a.pContext && a.pContext); // NEED TO FIX *** add compare for groups
	if(a.pContext && a.pContext) {
		ASSERT(a.pContext->GetParent() && b.pContext->GetParent());

		// check layer:
		if(a.pContext->GetObjLayer() <  b.pContext->GetObjLayer()) return true;
		if(a.pContext->GetObjLayer() >  b.pContext->GetObjLayer()) return false;

		//check sublayer:
		if(a.pContext->GetObjSubLayer() < b.pContext->GetObjSubLayer()) return true;
		if(a.pContext->GetObjSubLayer() > b.pContext->GetObjSubLayer()) return false;

		//check order:
		if(a.pContext->GetObjOrder() < b.pContext->GetObjOrder()) return true;
		//if(a.pContext->GetObjOrder() > b.pContext->GetObjOrder()) return false;
	} else {
		// it's a group.
	}

	return false;
}

CDrawableSelection::CDrawableSelection(CDrawableContext **ppDrawableContext_) :
	m_CurrentCursor(eIDC_ARROW),
	m_nSnapSize(1),
	m_bShowGrid(false),
	m_nLayer(DEFAULT_LAYER), // Default Layer
	m_rcSelection(0,0,0,0),
	m_ptInitialPoint(0,0),
	m_bAllowMultiLayerSel(false),
	m_bFloating(false),
	m_bHoldSelection(false),
	m_pBitmap(NULL),
	m_pLastSelected(NULL),
	m_rcClip(0,0,0,0),
	m_CurrentSel(NULL),
	m_nPasteGroup(0),
	m_nCurrentGroup(0)
{
	for(int i=0; i<MAX_LAYERS; i++)
		m_bLockedLayers[i] = false;

	m_Groups.resize(16); // begin with 16 gropus

	m_ppMainDrawable = ppDrawableContext_;
	ASSERT(m_ppMainDrawable);
}
int CDrawableSelection::GetBoundingRect(CRect *pRect_, int nGroup_) 
{
	ASSERT(nGroup_ >= 0 && nGroup_ <= (int)m_Groups.size());

	int nSelected = 0;
	CRect RectTmp;
	pRect_->SetRectEmpty();

	vectorObject::iterator Iterator;
	for(Iterator = m_Groups[nGroup_].O.begin(); Iterator != m_Groups[nGroup_].O.end(); Iterator++) {
		if(Iterator->pContext) {
			Iterator->pContext->GetAbsFinalRect(RectTmp);
			pRect_->UnionRect(pRect_, RectTmp);
			nSelected++;
		} else {
			// Get the bounding rect of the children:
			int nChild = Iterator->nGroup;
			ASSERT(nChild >= 0 && nChild <= (int)m_Groups.size());
			ASSERT(m_Groups[nChild].P == nGroup_); // verify child's parent to be valid
			nSelected += GetBoundingRect(&RectTmp, nChild);
			pRect_->UnionRect(pRect_, RectTmp);
		}
	}
	return nSelected;
}

bool CDrawableSelection::isGroup()
{
	return (m_nCurrentGroup > 0);
}
// this function selects every object in a group (and all its children)
bool CDrawableSelection::SelectGroup(int nGroup_, bool bSelectContext) 
{
	ASSERT(nGroup_ >= 0 && nGroup_ <= (int)m_Groups.size());

	vectorObject::iterator Iterator;
	for(Iterator = m_Groups[nGroup_].O.begin(); Iterator != m_Groups[nGroup_].O.end(); Iterator++) {
		if(Iterator->pContext) {
			Iterator->pContext->SelectContext(bSelectContext);
		} else {
			// Select children (if any):
			int nChild = Iterator->nGroup;
			ASSERT(nChild >= 0 && nChild <= (int)m_Groups.size());
			ASSERT(m_Groups[nChild].P == nGroup_); // verify child's parent to be valid
			SelectGroup(nChild, bSelectContext);
		}
	}
	return true;
}
// this function looks for a drawable context in all the groups and selects the group if found
bool CDrawableSelection::SelectGroupWith(const CDrawableContext *pDrawableContext) 
{
	// are there any objects in the current selection?
	bool bEmpty = m_Groups[0].O.empty(); 

	for(int i=1; i<(int)m_Groups.size(); i++) {
		// Iterate through every context in every group looking for the required context:
		vectorObject::iterator Iterator = 
			find_if(m_Groups[i].O.begin(), m_Groups[i].O.end(), bind2nd(m_equalContext, pDrawableContext));
		if(Iterator != m_Groups[i].O.end()) {
			// If the group found has a parent select the parent group too:
            int nGroup = i;
			while(m_Groups[nGroup].P) {
				nGroup = m_Groups[nGroup].P;
				ASSERT(nGroup >= 0 && nGroup <= (int)m_Groups.size());
			}

			// Save the last current selected group (if there were no other selections):
			if(bEmpty) m_nCurrentGroup = nGroup;
			else m_nCurrentGroup = 0;

			// Select the new group, appending a child group to the current selection:
			CRect rcRect;
			GetBoundingRect(&rcRect, nGroup);
			m_Groups[0].O.push_back(SObjProp(this, NULL, nGroup, rcRect));

			CONSOLE_DEBUG("The group '%s' has been selected. (%d)\n", (LPCSTR)m_Groups[nGroup].Name.c_str(), m_Groups[nGroup].O.size());

			return SelectGroup(nGroup);
		}
	}
	return false;
}
// this function looks for a drawable context in all the groups and selects the group if found
bool CDrawableSelection::SelectGroupWithIn(const CRect &rect_, const CDrawableContext *pDrawableContext) 
{
	// are there any objects in the current selection?
	bool bEmpty = m_Groups[0].O.empty(); 

	for(int i=1; i<(int)m_Groups.size(); i++) {
		// Iterate through every context in every group looking for the required context:
		vectorObject::iterator Iterator = 
			find_if(m_Groups[i].O.begin(), m_Groups[i].O.end(), bind2nd(m_equalContext, pDrawableContext));
		if(Iterator != m_Groups[i].O.end()) {
			// If the group found has a parent select the parent group too:
			// NEED TO FIX *** (select parents up to the current group only??)
            int nGroup = i;
			while(m_Groups[nGroup].P) {
				nGroup = m_Groups[nGroup].P;
				ASSERT(nGroup >= 0 && nGroup <= (int)m_Groups.size());
			}
			// Check if the full group is within rect_ :
			CRect rcBoundaries;
			GetBoundingRect(&rcBoundaries, nGroup);
			if(!rect_.PtInRect(rcBoundaries.TopLeft()) || !rect_.PtInRect(rcBoundaries.BottomRight())) 
				return true;

			// Save the last current selected group (if there were no other selections):
			if(bEmpty) m_nCurrentGroup = nGroup;
			else m_nCurrentGroup = 0;

			// Append a child group to the current selection:
			CRect rcRect;
			GetBoundingRect(&rcRect, nGroup);
			// m_Groups[0].C.push_back(nGroup); NEED TO FIX *** select groups
			m_Groups[0].O.push_back(SObjProp(this, NULL, nGroup, rcRect));
			CONSOLE_DEBUG("The group '%s' has been selected. (%d)\n", (LPCSTR)m_Groups[nGroup].Name.c_str(), m_Groups[nGroup].O.size());

			return SelectGroup(nGroup);
		}
	}
	return false;
}

// find and delete a context in all groups (a context must exist in only one group)
void CDrawableSelection::DeleteInGroups(const CDrawableContext *pDrawableContext) 
{
	for(int i=1; i<(int)m_Groups.size(); i++) {
		vectorObject::iterator removed = 
			remove_if(m_Groups[i].O.begin(), m_Groups[i].O.end(), bind2nd(m_equalContext, pDrawableContext));
		if(removed != m_Groups[i].O.end()) {
			m_Groups[i].O.erase(removed, m_Groups[i].O.end());
			return;
		}
	}
}

// returns -1 if the context is not found, or the group if the context is found.
// If the context is found, it returns the context's object in *Iterator.
int CDrawableSelection::FindInGroup(vectorObject::iterator *Iterator, CDrawableContext *pDrawableContext, int nGroup_)
{
	*Iterator = find_if(m_Groups[nGroup_].O.begin(), m_Groups[nGroup_].O.end(), bind2nd(m_equalContext, pDrawableContext));
	if((*Iterator) != m_Groups[nGroup_].O.end()) {
		return nGroup_;
	}

	for((*Iterator) = m_Groups[nGroup_].O.begin(); (*Iterator) != m_Groups[nGroup_].O.end(); (*Iterator)++) {
		if((*Iterator)->pContext == NULL) {
			vectorObject::iterator Object;
			int nGroup = FindInGroup(&Object, pDrawableContext, (*Iterator)->nGroup);
			if(nGroup >= 0) {
				*Iterator = Object;
				return nGroup;
			}
		}
	}

	*Iterator = NULL;
	return -1;
}

IPropertyEnabled* CDrawableSelection::SelPointAdd(const CPoint &point_, int Chains) 
{
	int nLayer = m_Groups[0].O.empty() ? -1: m_nLayer; 
	CDrawableContext *pDrawableContext = NULL;

	(*m_ppMainDrawable)->GetFirstChildAt(point_, &pDrawableContext);

	while(pDrawableContext) {
		int nNewLayer = pDrawableContext->GetObjLayer();
		// If the layer is not locked and the new context is in the same layer as the currently selected layer:
		if(!isLocked(nNewLayer) && (nNewLayer == nLayer || nLayer == -1 || m_bAllowMultiLayerSel)) {
			m_nLayer = nLayer = nNewLayer;

			// try to find in the selected groups:
			vectorObject::iterator Iterator;
			int nChild = FindInGroup(&Iterator, pDrawableContext,0 );
			
			if(nChild<0 || Iterator == m_Groups[nChild].O.end()) {
				// the context is not yet selected, so try to select the full group containing it (if any)
				if(SelectGroupWith(pDrawableContext)) return NULL;

				CRect Rect;
				pDrawableContext->GetRect(Rect);
				pDrawableContext->SelectContext();

				// The sprite wasn't on any group, so we add it as a new object to the selection group:
				m_Groups[0].O.push_back(SObjProp(this, pDrawableContext, 0, Rect, relative, relative));
				m_nCurrentGroup = 0; // discard any currently selected group as a unique group.
				m_pLastSelected = pDrawableContext; // set the last selection.

				pDrawableContext->SelectContext();
				CONSOLE_DEBUG("%d of %d objects selected.\n", m_Groups[0].O.size(), (*m_ppMainDrawable)->Objects());
				CONSOLE_DEBUG("The selected object is on layer %d, and sublayer: %d.\n", pDrawableContext->GetObjLayer(), pDrawableContext->GetObjSubLayer());
				return NULL;
			}
			if(nChild == m_nCurrentGroup) {
				// The context was already selected, edit the chains:
				if(Chains>0) {
					if(m_Groups[m_nCurrentGroup].O.size() <= 1) 
						return NULL; // ignore if there's only one selected object

					if(Iterator->eXChain<4) {
						Iterator->eXChain = (_Chain)((int)Iterator->eXChain+1);
					} else {
						Iterator->eXChain = (_Chain)0;
						if(Iterator->eYChain<4) Iterator->eYChain = (_Chain)((int)Iterator->eYChain+1);
						else Iterator->eYChain = (_Chain)0;
					}
					// set the last selection:
					m_pLastSelected = Iterator->pContext;
					CONSOLE_DEBUG("XChain: %d, YChain: %d\n", (int)Iterator->eXChain, (int)Iterator->eYChain);
					return static_cast<IPropertyEnabled *>(Iterator.operator ->());
				} else if(Chains<0) {
					if(m_Groups[m_nCurrentGroup].O.size() <= 1) 
						return NULL; // ignore if there's only one selected object

					if(Iterator->eXChain>0) {
						Iterator->eXChain = (_Chain)((int)Iterator->eXChain-1);
					} else {
						Iterator->eXChain = (_Chain)4;
						if(Iterator->eYChain>0) Iterator->eYChain = (_Chain)((int)Iterator->eYChain-1);
						else Iterator->eYChain = (_Chain)4;
					}
					// set the last selection:
					m_pLastSelected = Iterator->pContext;
					CONSOLE_DEBUG("XChain: %d, YChain: %d\n", (int)Iterator->eXChain, (int)Iterator->eYChain);
					return static_cast<IPropertyEnabled *>(Iterator.operator ->());
				}
			}
		}
		pDrawableContext = NULL;
		(*m_ppMainDrawable)->GetNextChildAt(point_, &pDrawableContext);
	}
	return NULL;
}
void CDrawableSelection::SelPointRemove(const CPoint &point_) 
{
	int nLayer = m_Groups[0].O.empty() ? -1: m_nLayer; 
	CDrawableContext *pDrawableContext = NULL;

	(*m_ppMainDrawable)->GetFirstChildAt(point_, &pDrawableContext);

	while(pDrawableContext) {
		int nNewLayer = pDrawableContext->GetObjLayer();
		// If the layer is not locked and the new context is in the same layer as the currently selected layer:
		if(!isLocked(nNewLayer) && (nNewLayer == nLayer || nLayer == -1 || m_bAllowMultiLayerSel)) {
			nLayer = nNewLayer;

			// try to find in the selected groups:
			vectorObject::iterator Iterator;
			int nGroup = FindInGroup(&Iterator, pDrawableContext, 0);

			if(nGroup >= 0) {
				// Now that we have found the group on which the context is stored,
				// we look for the topmost group:
				while(m_Groups[nGroup].P) {
					nGroup = m_Groups[nGroup].P;
					ASSERT(nGroup >= 0 && nGroup <= (int)m_Groups.size());
				}

				vectorObject::iterator removed = 
					remove_if(m_Groups[0].O.begin(), m_Groups[0].O.end(), bind2nd(m_equalContext, pDrawableContext));
				if(removed != m_Groups[0].O.end()) {
					m_Groups[0].O.erase(removed, m_Groups[0].O.end());
					return;
				}

				// the context is not yet selected, so try to select the full group containing it (if any)
				SelectGroup(nGroup, false);
			}

			// remove the last selection:
			if(m_pLastSelected == pDrawableContext) m_pLastSelected = NULL;
		}
		pDrawableContext = NULL;
		(*m_ppMainDrawable)->GetNextChildAt(point_, &pDrawableContext);
	}
}

// Interface:
int CDrawableSelection::GetLayer()
{
	return m_nLayer;
}
void CDrawableSelection::SetLayer(int nLayer_)
{
	m_nLayer = nLayer_;
	if(m_nLayer < 0) m_nLayer = 0;
	if(m_nLayer > MAX_LAYERS) m_nLayer = MAX_LAYERS;
}
void CDrawableSelection::SetSnapSize(int nSnapSize_, bool bShowGrid_)
{
	m_bShowGrid = bShowGrid_;
	m_nSnapSize = nSnapSize_;
}
int CDrawableSelection::RealCount(int nGroup_)
{
	int nRet = 0;
	vectorObject::const_iterator Iterator;
	for(Iterator = m_Groups[nGroup_].O.begin(); Iterator != m_Groups[nGroup_].O.end(); Iterator++) {
		if(Iterator->pContext) nRet++;
		else nRet += RealCount(Iterator->nGroup);
	}
	return nRet;
}
int CDrawableSelection::Count()
{
	return m_Groups[0].O.size();
}
bool CDrawableSelection::isResizing() 
{ 
	return (m_eCurrentState==eResizing); 
}
bool CDrawableSelection::isMoving() 
{ 
	return (m_eCurrentState==eMoving); 
}
bool CDrawableSelection::isSelecting() 
{ 
	return (m_eCurrentState==eSelecting); 
}
bool CDrawableSelection::isFloating()
{
	return m_bFloating;
}
void CDrawableSelection::StartResizing(const CPoint &point_)
{
	m_eCurrentState = eResizing;
	BuildRealSelectionBounds(); // updates m_rcSelection
	m_ptInitialPoint = point_;
}
void CDrawableSelection::ResizeTo(const CPoint &point_)
{
	if(m_eCurrentState!=eResizing) return;

	CRect rcNewBoundaries;
	rcNewBoundaries = m_rcSelection;

	CPoint PointTmp = m_ptInitialPoint - point_;
	//if(PointTmp.x == 0 && PointTmp.y == 0) return;

	if(m_bCursorLeft) {
		rcNewBoundaries.left -= PointTmp.x;
		rcNewBoundaries.left = m_nSnapSize*((rcNewBoundaries.left+(m_nSnapSize/2))/m_nSnapSize);
	} else if(m_bCursorRight) {
		rcNewBoundaries.right -= PointTmp.x;
		rcNewBoundaries.right = m_nSnapSize*((rcNewBoundaries.right+(m_nSnapSize/2))/m_nSnapSize);
	}
	if(m_bCursorTop) {
		rcNewBoundaries.top -= PointTmp.y;
		rcNewBoundaries.top = m_nSnapSize*((rcNewBoundaries.top+(m_nSnapSize/2))/m_nSnapSize);
	} else if(m_bCursorBottom) {
		rcNewBoundaries.bottom -= PointTmp.y;
		rcNewBoundaries.bottom = m_nSnapSize*((rcNewBoundaries.bottom+(m_nSnapSize/2))/m_nSnapSize);
	}

	// Any negative resizes will be transformed to the minumum unit (perhaps the current snap size)
	if(rcNewBoundaries.Height()<m_nSnapSize && m_bCursorTop) {
		int h = m_rcSelection.Height()%m_nSnapSize; if(h==0) h = m_nSnapSize;
		rcNewBoundaries.bottom = m_rcSelection.bottom;
		rcNewBoundaries.top = m_rcSelection.bottom - h;
	} else if(rcNewBoundaries.Height()<m_nSnapSize && m_bCursorBottom) {
		int h = m_rcSelection.Height()%m_nSnapSize; if(h==0) h = m_nSnapSize;
		rcNewBoundaries.top = m_rcSelection.top;
		rcNewBoundaries.bottom = m_rcSelection.top + h;
	}
	if(rcNewBoundaries.Width()<m_nSnapSize && m_bCursorLeft) {
		int w = m_rcSelection.Width()%m_nSnapSize; if(w==0) w = m_nSnapSize;
		rcNewBoundaries.right = m_rcSelection.right;
		rcNewBoundaries.left = m_rcSelection.right - w;
	} else if(rcNewBoundaries.Width()<m_nSnapSize && m_bCursorRight) {
		int w = m_rcSelection.Width()%m_nSnapSize; if(w==0) w = m_nSnapSize;
		rcNewBoundaries.left = m_rcSelection.left;
		rcNewBoundaries.right = m_rcSelection.left + w;
	}
	rcNewBoundaries.NormalizeRect();

	vectorObject::const_iterator Iterator;
	for(Iterator = m_Groups[0].O.begin(); Iterator != m_Groups[0].O.end(); Iterator++) {
		ResizeObject(*Iterator, m_rcSelection, rcNewBoundaries, true);
		// NEED TO FIX *** (add group resizing)
	}
}
void CDrawableSelection::EndResizing(const CPoint &point_)
{
	if(m_eCurrentState!=eResizing) return;
	ResizeTo(point_);

	// Next, find out if any changes have actually taken place:
	CRect rcNewBoundaries;
	rcNewBoundaries = m_rcSelection; // save start boundaries rect
	BuildRealSelectionBounds(); // updates m_rcSelection

	if(rcNewBoundaries != m_rcSelection) Touch();

	m_eCurrentState = eNone;
}
void CDrawableSelection::GetSelBounds(CRect *pRect_)
{
	GetBoundingRect(pRect_);
}

int CDrawableSelection::SetLayerSelection(int nLayer, int nGroup_)
{
	int nSelected = 0;
	vectorObject::iterator Iterator;
	for(Iterator = m_Groups[nGroup_].O.begin(); Iterator != m_Groups[nGroup_].O.end(); Iterator++) {
		if(Iterator->pContext) {
			if(Iterator->pContext->GetObjLayer() != m_nLayer) {
				Touch();
				Iterator->pContext->SetObjLayer(m_nLayer);
				nSelected++;
			}
		} else {
			nSelected += SetLayerSelection(nLayer, Iterator->nGroup);
		}
	}
	return nSelected;
}
void CDrawableSelection::SetLayerSelection(int nLayer)
{
	if(m_bFloating) return;

	// first sort selection
	SortSelection();

	int nSelected = SetLayerSelection(nLayer, 0);
	if(nSelected) {
		m_nLayer = nLayer;
		CONSOLE_DEBUG("%d objects moved to layer %d.\n", nSelected, nLayer);
	}
}

int CDrawableSelection::DeleteSelection(int nGroup_)
{
	int nDeleted = 0;

	if(m_bHoldSelection) { // if the selection is held, just remove from selection.
		// NEED TO FIX *** add functionality to this
	} else {
		vectorObject::iterator Iterator;
		for(Iterator = m_Groups[nGroup_].O.begin(); Iterator != m_Groups[nGroup_].O.end(); Iterator++) {
			if(Iterator->pContext) {
				Iterator->pContext->SelectContext(false);
				DeleteInGroups(Iterator->pContext); // NEED TO FIX *** couldn't this be done using clear() instead?
				(*m_ppMainDrawable)->DeleteChildEx(Iterator->pContext);
				nDeleted++;
			} else {
				// Get the bounding rect of the children:
				int nChild = Iterator->nGroup;
				ASSERT(nChild >= 0 && nChild <= (int)m_Groups.size());
				ASSERT(m_Groups[nChild].P == nGroup_); // verify child's parent to be valid
				nDeleted += DeleteSelection(nChild);
			}
		}
		m_Groups[nGroup_].O.clear();
	}

	return nDeleted;
}
int CDrawableSelection::DeleteSelection()
{
	int nDeleted = DeleteSelection(0);

	m_pLastSelected = NULL;
	m_nCurrentGroup = 0;

	if(m_bFloating) m_bFloating = false;
	else Touch();
	CONSOLE_DEBUG("%d objects left.\n", (*m_ppMainDrawable)->Objects());

	return nDeleted;
}
void CDrawableSelection::Cancel()
{
	if(m_bFloating && !m_bHoldSelection) {
		DeleteSelection();
		m_eCurrentState = eNone;
	} else if(m_eCurrentState==eMoving) {
		EndMoving(m_ptInitialPoint);
	} else if(m_eCurrentState==eResizing) {
		EndResizing(m_ptInitialPoint);
	} else if(!m_bHoldSelection) {
		vectorObject::iterator Iterator;
		for(int nChild=0; nChild<(int)m_Groups.size(); nChild++) {
			for(Iterator = m_Groups[nChild].O.begin(); Iterator != m_Groups[nChild].O.end(); Iterator++) {
				if(Iterator->pContext) {
					Iterator->pContext->SelectContext(false);
				}
			}
		}
		m_pLastSelected = NULL;
		m_nCurrentGroup = 0;
		m_Groups[0].O.clear();
	}
}
void CDrawableSelection::SetInitialMovingPoint(const CPoint &point_)
{
	m_ptInitialPoint = point_;

	// adjust to check if the mouse is too close to the borders
	// of the boundaries.
	CRect rcBoundaries = m_rcSelection;

	// Extend the rectangle to snap
	rcBoundaries.bottom = rcBoundaries.bottom + m_nSnapSize - 1;
	rcBoundaries.right = rcBoundaries.right + m_nSnapSize - 1;

	rcBoundaries.top = (rcBoundaries.top/m_nSnapSize) * m_nSnapSize;
	rcBoundaries.left = (rcBoundaries.left/m_nSnapSize) * m_nSnapSize;
	rcBoundaries.right = (rcBoundaries.right/m_nSnapSize) * m_nSnapSize;
	rcBoundaries.bottom = (rcBoundaries.bottom/m_nSnapSize) * m_nSnapSize;

	// if the mouse is too close to the selection boundaries, we move it closer to the center
	rcBoundaries.DeflateRect(m_nSnapSize/2 + 1, m_nSnapSize/2 + 1);

	if(m_ptInitialPoint.x < rcBoundaries.left) m_ptInitialPoint.x = rcBoundaries.left;
	else if(m_ptInitialPoint.x > rcBoundaries.right) m_ptInitialPoint.x = rcBoundaries.right;
	if(m_ptInitialPoint.y < rcBoundaries.top) m_ptInitialPoint.y = rcBoundaries.top;
	else if(m_ptInitialPoint.y > rcBoundaries.bottom) m_ptInitialPoint.y = rcBoundaries.bottom;

	m_ptInitialPoint.Offset(-m_nSnapSize/2, -m_nSnapSize/2);
}
void CDrawableSelection::StartMoving(const CPoint &point_)
{
	m_eCurrentState = eMoving;
	BuildRealSelectionBounds();
	SetInitialMovingPoint(point_);
}
void CDrawableSelection::MoveTo(const CPoint &Point_)
{
	if(m_eCurrentState!=eMoving) return;

	CRect rcOldBoundaries, rcNewBoundaries;
	GetBoundingRect(&rcOldBoundaries);
	rcNewBoundaries = m_rcSelection;

	CPoint PointTmp = m_ptInitialPoint - Point_;
	CPoint OffsetPoint(PointTmp.x%m_nSnapSize, PointTmp.y%m_nSnapSize);

	int w = rcNewBoundaries.Width();
	int h = rcNewBoundaries.Height();

	rcNewBoundaries.left -= PointTmp.x;
	rcNewBoundaries.left = m_nSnapSize*(rcNewBoundaries.left/m_nSnapSize);
	rcNewBoundaries.top -= PointTmp.y;
	rcNewBoundaries.top = m_nSnapSize*(rcNewBoundaries.top/m_nSnapSize);
	rcNewBoundaries.right = rcNewBoundaries.left + w;
	rcNewBoundaries.bottom = rcNewBoundaries.top + h;
	
	if(rcNewBoundaries.left == rcOldBoundaries.left && rcNewBoundaries.top == rcOldBoundaries.top) return;

	vectorObject::const_iterator Iterator;
	for(Iterator = m_Groups[0].O.begin(); Iterator != m_Groups[0].O.end(); Iterator++) {
		ResizeObject(*Iterator, m_rcSelection, rcNewBoundaries, true);
	}
}
void CDrawableSelection::EndMoving(const CPoint &point_)
{
	if(m_eCurrentState!=eMoving) return;
	MoveTo(point_);

	// Next, find out if any changes have actually taken place:
	CRect rcNewBoundaries;
	rcNewBoundaries = m_rcSelection; // save start boundaries rect
	BuildRealSelectionBounds(); // updates m_rcSelection

	if(rcNewBoundaries != m_rcSelection || m_bFloating) Touch();

	m_bFloating = false;
	m_eCurrentState = eNone;
}
// pauses all changes and restores original state on the move or resize operations.
void CDrawableSelection::HoldOperation() 
{
	if(m_eCurrentState==eResizing) ResizeTo(m_ptInitialPoint);
	else if(m_eCurrentState==eMoving) MoveTo(m_ptInitialPoint);
}

void CDrawableSelection::StartSelBox(const CPoint &point_) {
	m_rcSelection.left = point_.x;
	m_rcSelection.top = point_.y;
	m_rcSelection.right = point_.x;
	m_rcSelection.bottom = point_.y;
	m_eCurrentState = eSelecting;

	m_bCanResize = false;	// This must be set before drawing
	m_bCanMove = false;		// the boundary box on the screen.

}
void CDrawableSelection::CancelSelBox() {
	if(m_eCurrentState!=eSelecting) return;
	m_eCurrentState = eNone;
}
void CDrawableSelection::SizeSelBox(const CPoint &point_) {
	if(m_eCurrentState!=eSelecting) return;
	m_rcSelection.right = point_.x;
	m_rcSelection.bottom = point_.y;
}

void CDrawableSelection::LockLayer(int nLayer, bool bLock)
{
	ASSERT(nLayer>=0 && nLayer<=MAX_LAYERS);

	if(nLayer>=0 && nLayer<=MAX_LAYERS)
		m_bLockedLayers[nLayer] = bLock;

	if(bLock && m_nLayer == nLayer) {
		// deselect all:
		vectorObject::iterator Iterator;
		for(int nChild=0; nChild<(int)m_Groups.size(); nChild++) {
			for(Iterator = m_Groups[nChild].O.begin(); Iterator != m_Groups[nChild].O.end(); Iterator++) {
				if(Iterator->pContext) {
					Iterator->pContext->SelectContext(false);
				}
			}
		}
		m_pLastSelected = NULL;
		m_nCurrentGroup = 0;
		m_Groups[0].O.clear();
		m_nLayer = -1;
	}
}
bool CDrawableSelection::isLocked(int nLayer)
{
	ASSERT(nLayer>=0 && nLayer<=MAX_LAYERS);

	if(nLayer>=0 && nLayer<=MAX_LAYERS)
		return m_bLockedLayers[nLayer];

	return false;
}
bool CDrawableSelection::SelectedAt(const CPoint &point_)
{
	vectorObject::const_iterator Iterator;
	for(Iterator = m_Groups[0].O.begin(); Iterator != m_Groups[0].O.end(); Iterator++) {
		if(Iterator->pContext) {
			if(Iterator->pContext->isAt(point_)) return true;
		} else {
			int nChild = Iterator->nGroup;
			CRect Rect;
			GetBoundingRect(&Rect, nChild);
			if(Rect.PtInRect(point_)) return true;
		}
	}
	return false;
}

void CDrawableSelection::SubSelPoint(bool bAdd, const CPoint &point_, int Chains)
{
	vectorObject::reverse_iterator Iterator;
	for(Iterator = m_Groups[0].O.rbegin(); Iterator != m_Groups[0].O.rend(); Iterator++) {
		if(Iterator->pContext == NULL) continue; // NEED TO FIX *** add group stuff
		if(Iterator->pContext->isAt(point_) && Iterator->bSubselected != bAdd) {
			if(Chains==0 || !bAdd) {
				if(Iterator->pContext == m_pLastSelected && !bAdd) m_pLastSelected = NULL;
				if(bAdd) m_pLastSelected = Iterator->pContext;
				Iterator->bSubselected = bAdd;
			} else if(Chains>0) {
				if(m_Groups[0].O.size()<=1) return; // ignore if there's only one object
				if(Iterator->eXChain<4) Iterator->eXChain = (_Chain)((int)Iterator->eXChain+1);
				else {
					Iterator->eXChain = (_Chain)0;
					if(Iterator->eYChain<4) Iterator->eYChain = (_Chain)((int)Iterator->eYChain+1);
					else Iterator->eYChain = (_Chain)0;
				}
				m_pLastSelected = Iterator->pContext;
				CONSOLE_DEBUG("XChain: %d, YChain: %d\n", (int)Iterator->eXChain, (int)Iterator->eYChain);
			} else if(Chains<0) {
				if(m_Groups[0].O.size()<=1) return; // ignore if there's only one object
				if(Iterator->eXChain>0) Iterator->eXChain = (_Chain)((int)Iterator->eXChain-1);
				else {
					Iterator->eXChain = (_Chain)4;
					if(Iterator->eYChain>0) Iterator->eYChain = (_Chain)((int)Iterator->eYChain-1);
					else Iterator->eYChain = (_Chain)4;
				}
				m_pLastSelected = Iterator->pContext;
				CONSOLE_DEBUG("XChain: %d, YChain: %d\n", (int)Iterator->eXChain, (int)Iterator->eYChain);
			}
			return;
		}
	}
}

void CDrawableSelection::EndSubSelBox(bool bAdd, const CPoint &point_, int Chains) 
{
	if(m_eCurrentState!=eSelecting) return;

	m_rcSelection.right = point_.x;
	m_rcSelection.bottom = point_.y;
	m_eCurrentState = eNone;

	m_rcSelection.NormalizeRect();
	if(m_rcSelection.Width()<=1 && m_rcSelection.Height()<=1) {
		SubSelPoint(bAdd, m_rcSelection.TopLeft(), Chains);
		return;
	}

	vectorObject::reverse_iterator Iterator;
	for(Iterator = m_Groups[0].O.rbegin(); Iterator != m_Groups[0].O.rend(); Iterator++) {
		if(Iterator->pContext == NULL) continue; // NEED TO FIX *** add group stuff
		if(Iterator->pContext->isIn(m_rcSelection)) {
			if(Iterator->pContext == m_pLastSelected && !bAdd) m_pLastSelected = NULL;
			if(bAdd) m_pLastSelected = Iterator->pContext;
			Iterator->bSubselected = bAdd;
		}
	}
}

IPropertyEnabled* CDrawableSelection::EndSelBoxAdd(const CPoint &point_, int Chains) 
{
	if(m_bHoldSelection) {
		EndSubSelBox(true, point_, Chains);
		return NULL;
	}
	if(m_eCurrentState!=eSelecting) return NULL;

	m_rcSelection.right = point_.x;
	m_rcSelection.bottom = point_.y;
	m_eCurrentState = eNone;

	m_rcSelection.NormalizeRect();
	if(m_rcSelection.Width()<=1 && m_rcSelection.Height()<=1) {
		return SelPointAdd(m_rcSelection.TopLeft(), Chains);
	}
	int nLayer = m_Groups[0].O.empty() ? -1 : m_nLayer;
	CDrawableContext *pDrawableContext = NULL;
	(*m_ppMainDrawable)->GetFirstChildIn(m_rcSelection, &pDrawableContext);
	while(pDrawableContext) {
		int nNewLayer = pDrawableContext->GetObjLayer();
		if(!isLocked(nNewLayer) && (nNewLayer == nLayer || nLayer == -1 || m_bAllowMultiLayerSel)) {
			m_nLayer = nLayer = nNewLayer;

			// try to find in the selected groups:
			vectorObject::iterator Iterator;
			int nChild = FindInGroup(&Iterator, pDrawableContext, 0);

			// NEED TO FIX *** group selection should be better, and should handle chains too
			if(nChild<0 || Iterator == m_Groups[nChild].O.end()) {
				if(!SelectGroupWithIn(m_rcSelection, pDrawableContext)) {
					CRect Rect;
					pDrawableContext->GetRect(Rect);
					pDrawableContext->SelectContext();

					m_pLastSelected = pDrawableContext;
	
					m_nCurrentGroup = 0;
					m_Groups[0].O.push_back(SObjProp(this, pDrawableContext, 0, Rect, relative, relative));
				}
			}
		}
		pDrawableContext = NULL;
		(*m_ppMainDrawable)->GetNextChildIn(m_rcSelection, &pDrawableContext);
	}
	CONSOLE_DEBUG("%d of %d objects selected.\n", m_Groups[0].O.size(), (*m_ppMainDrawable)->Objects());
	return NULL;
}
void CDrawableSelection::EndSelBoxRemove(const CPoint &point_) 
{
	if(m_bHoldSelection) {
		EndSubSelBox(false, point_, 0);
		return;
	}

	if(m_eCurrentState!=eSelecting) return;
	m_rcSelection.right = point_.x;
	m_rcSelection.bottom = point_.y;
	m_eCurrentState = eNone;

	m_rcSelection.NormalizeRect();
	if(m_rcSelection.Width()<=1 && m_rcSelection.Height()<=1) 
		SelPointRemove(m_rcSelection.TopLeft());

	int nLayer = m_Groups[0].O.empty() ? -1 : m_nLayer;
	CDrawableContext *pDrawableContext = NULL;
	(*m_ppMainDrawable)->GetFirstChildIn(m_rcSelection, &pDrawableContext);
	while(pDrawableContext) {
		int nNewLayer = pDrawableContext->GetObjLayer();
		if(!isLocked(nNewLayer) && (nNewLayer == nLayer || nLayer == -1 || m_bAllowMultiLayerSel)) {
			nLayer = nNewLayer;

			if(pDrawableContext == m_pLastSelected) m_pLastSelected = NULL;

			pDrawableContext->SelectContext(false);
			vectorObject::iterator remove = 
				remove_if(m_Groups[0].O.begin(), m_Groups[0].O.end(), bind2nd(m_equalContext, pDrawableContext));
			if(remove != m_Groups[0].O.end()) {
				m_nCurrentGroup = 0;
				m_Groups[0].O.erase(remove, m_Groups[0].O.end());
			}
		}
		pDrawableContext = NULL;
		(*m_ppMainDrawable)->GetNextChildIn(m_rcSelection, &pDrawableContext);
	}
}
void CDrawableSelection::CleanSelection() {
	vectorObject::iterator Iterator;
	for(int nChild=0; nChild<(int)m_Groups.size(); nChild++) {
		for(Iterator = m_Groups[nChild].O.begin(); Iterator != m_Groups[nChild].O.end(); Iterator++) {
			if(Iterator->pContext == NULL) continue; // NEED TO FIX *** add group stuff
			if(!m_bHoldSelection) Iterator->pContext->SelectContext(false);
			else Iterator->bSubselected = false;
		}
	}
	m_pLastSelected = NULL;
	if(!m_bHoldSelection) {
		m_nCurrentGroup = 0;
		m_Groups[0].O.clear();
	}
}
bool CDrawableSelection::GetMouseStateAt(const IGraphics *pGraphics_, const CPoint &point_, CURSOR *pCursor)
{
	CPoint WorldPoint = point_;
	// We get the world coordinates for the mouse position
	pGraphics_->ViewToWorld(&WorldPoint);

	CRect rcBoundaries;
	GetBoundingRect(&rcBoundaries);
	bool ret = rcBoundaries.PtInRect(WorldPoint);

	if(m_bFloating) m_CurrentCursor = eIDC_SIZEALL;
	*pCursor = m_CurrentCursor;

	if( m_eCurrentState==eResizing	||
		m_eCurrentState==eMoving	|| 
		m_eCurrentState==eSelecting	   ) return ret;

	m_CurrentCursor = eIDC_ARROW;
	m_bCursorLeft = m_bCursorTop = m_bCursorRight = m_bCursorBottom = false;

	CRect Rect(0,0,0,0);
	CRect RectTmp;

	// For each selected object, we check to see if the mouse is over it, also we build a bounding Rect:
	vectorObject::const_iterator Iterator;
	for(Iterator = m_Groups[0].O.begin(); Iterator != m_Groups[0].O.end(); Iterator++) {
		if(Iterator->pContext) {
			Iterator->pContext->GetAbsFinalRect(RectTmp);
		} else {
			GetBoundingRect(&RectTmp, Iterator->nGroup);
		}
		if(RectTmp.PtInRect(WorldPoint) && m_bCanMove) m_CurrentCursor = eIDC_SIZEALL; // The cursor is ovet the object.
		Rect.UnionRect(Rect, RectTmp);	// Bounding Rect
	}
	// NEET TO FIX *** (add mouse stuff over groups)

	// Now that we have the full bounding Rect (in world coordinates) we convert it to View coordinates.
	pGraphics_->WorldToView(&Rect);
	// Check the bounding box (in view coordinates) with the mouse point (also in view coordinates.):

	// We need to validate the cursor sensibility for all sides:
	int sens_inX = SELSENS_LINE;
	int sens_inY = SELSENS_LINE;
	if(sens_inX*4 > Rect.Width()) sens_inX = Rect.Width()/4;
	if(sens_inY*4 > Rect.Height()) sens_inY = Rect.Height()/4;

	if(point_.x >= Rect.left-SELSENS_LINE && point_.x < Rect.left+sens_inX) m_bCursorLeft = true;
	if(point_.x >= Rect.right-sens_inX && point_.x < Rect.right+SELSENS_LINE) m_bCursorRight = true;
	if(point_.y >= Rect.top-SELSENS_LINE && point_.y < Rect.top+sens_inY) m_bCursorTop = true;
	if(point_.y >= Rect.bottom-sens_inY && point_.y < Rect.bottom+SELSENS_LINE) m_bCursorBottom = true;

	// Are we over a line of the bounding box?
	if(m_bCanResize && (m_bCursorLeft || m_bCursorRight || m_bCursorTop || m_bCursorBottom)) {
		// We need to validate the cursor sensibility for all corners:
		int sens_cornerX = SELSENS_VERTEX;
		int sens_cornerY = SELSENS_VERTEX;
		if(sens_cornerX*3 > Rect.Width()) sens_cornerX = Rect.Width()/3;
		if(sens_cornerX == 0) sens_cornerX = 1;

		if(sens_cornerY*3 > Rect.Height()) sens_cornerY = Rect.Height()/3;
		if(sens_cornerY == 0) sens_cornerY = 1;
		
		int RectXMiddle = Rect.left+Rect.Width()/2;
		int RectYMiddle = Rect.top+Rect.Height()/2;

		// Are we in a middle point?
		if(point_.x >= RectXMiddle-sens_cornerX/2 && point_.x < RectXMiddle+sens_cornerX/2) {
			m_CurrentCursor = eIDC_SIZENS;
		}
		if(point_.y >= RectYMiddle-sens_cornerY/2 && point_.y < RectYMiddle+sens_cornerY/2) {
			m_CurrentCursor = eIDC_SIZEWE;
		}

		// Are we in any corner?
		if(m_bCursorTop || m_bCursorBottom) {
			if( point_.x >= Rect.left-SELSENS_LINE && point_.x < Rect.left+sens_cornerX ) {
				m_CurrentCursor = m_bCursorTop?eIDC_SIZENWSE:eIDC_SIZENESW;
				m_bCursorLeft = true;
			}
			if( point_.x > Rect.right-sens_cornerX && point_.x < Rect.right+SELSENS_LINE) {
				m_CurrentCursor = m_bCursorTop?eIDC_SIZENESW:eIDC_SIZENWSE;
				m_bCursorRight = true;
			}
		}
		if(m_bCursorLeft || m_bCursorRight) {
			if( point_.y >= Rect.top-SELSENS_LINE && point_.y < Rect.top+sens_cornerY ) {
				m_CurrentCursor = m_bCursorLeft?eIDC_SIZENWSE:eIDC_SIZENESW;
				m_bCursorTop = true;
			}
			if( point_.y > Rect.bottom-sens_cornerY && point_.y < Rect.bottom+SELSENS_LINE) {
				m_CurrentCursor = m_bCursorLeft?eIDC_SIZENESW:eIDC_SIZENWSE;
				m_bCursorBottom = true;
			}
		}
	}

	// if the selection is set to be held...
	if(m_bHoldSelection && m_CurrentCursor == eIDC_SIZEALL) {
		m_CurrentCursor = eIDC_ARROW;
	}

	*pCursor = m_CurrentCursor;
	return ret;
}

void CDrawableSelection::SortSelection()
{
	for(int nChild=0; nChild<(int)m_Groups.size(); nChild++) {
		sort(m_Groups[nChild].O.begin(), m_Groups[nChild].O.end(), m_cmpSelection);
	}
}

inline bool CDrawableSelection::BeginPaint(IGraphics *pGraphicsI, WORD wFlags)
{
	ASSERT(m_ppMainDrawable);
	if(!*m_ppMainDrawable) return false;

	/* // For testing only:
	static bool bret = false;
	static DWORD qq = GetTickCount();
	static float fAux = 1.00f;

	ARGBCOLOR argb = COLOR_ARGB((int)(255.0f*(0.5f+(fAux/2.0f))), 128, 128, 128);
	int move = (int)(400.0f - 800.0f * fAux);
	/**/

	CSprite::SetShowOptions(wFlags);
	DWORD color = ::GetSysColor(COLOR_APPWORKSPACE);
	pGraphicsI->SetFilterBkColor(COLOR_ARGB(255, GetRValue(color), GetGValue(color), GetBValue(color)));

	// Activate the filters:
	if((wFlags & GRAPHICS_FILTERS) == GRAPHICS_FILTERS) 
		pGraphicsI->SetFilter(EnableFilters, (void*)true);

	/* // For testing only:
	float pixelate = 17.0f - 16.0f * fAux;
	pGraphicsI->SetFilter(Pixelate, &pixelate);
	/*pGraphicsI->SetFilter(Alpha, &argb);
	pGraphicsI->SetFilter(HorzMove, &move);
	/**/
	pGraphicsI->SetClearColor((*m_ppMainDrawable)->GetBkColor());
	if(pGraphicsI->BeginPaint()) {
		/* // For testing only:
		if(GetTickCount() > qq + 60) {
			qq = GetTickCount();
			if(bret) fAux /= 0.8f;
			else fAux *= 0.8f;
			if(fAux<0.025f) {
				fAux = 0.025f;
				bret = true;
			} else if(fAux>1.0f) {
				fAux = 1.0f;
				qq+=2000;
				bret = false;
			}
		}
		/**/
		return true;
	}
	return false;
}
inline bool CDrawableSelection::EndPaint(IGraphics *pGraphicsI)
{
	return pGraphicsI->EndPaint();
}
inline bool CDrawableSelection::DrawAll(IGraphics *pGraphicsI)
{
	bool bRet = true;
	if(m_bHoldSelection) bRet &= (*m_ppMainDrawable)->DrawSelectedH(pGraphicsI);
	else bRet &= (*m_ppMainDrawable)->Draw(pGraphicsI);
	pGraphicsI->FlushFilters(true);
	pGraphicsI->SetFilter(ClearFilters, NULL);

	if(m_bShowGrid) 
		bRet &= pGraphicsI->DrawGrid(16, COLOR_ARGB(100,0,0,255));

	if(!m_rcClip.IsRectEmpty()) {
		bRet &= pGraphicsI->DrawFrame(m_rcClip, m_rgbClipColor, COLOR_ARGB(255, 255, 0, 0));
	}
	if(m_nLayer != -1) { // Layer not locked:
		// Get the current layer's size and draw a frame around it:
		CRect Rect;
		(*m_ppMainDrawable)->GetChild(m_nLayer)->GetRect(Rect);
		if(!Rect.IsRectEmpty()) pGraphicsI->SelectingBox(Rect, COLOR_ARGB(128, 0, 255, 0)); 
	}

	bRet &= Draw(pGraphicsI);

	return bRet;
}
bool CDrawableSelection::SetClip(const CRect *pRect, ARGBCOLOR rgbColor)
{
	if(pRect) {
		m_rcClip = *pRect;
	} else {
		m_rcClip.SetRect(0,0,0,0);
	}
	m_rgbClipColor = rgbColor;
	return true;
}

bool CDrawableSelection::Paint(IGraphics *pGraphicsI, WORD wFlags)
{
	bool bRet = true;
	if(BeginPaint(pGraphicsI, wFlags)) {
		bRet &= DrawAll(pGraphicsI);
		bRet &= EndPaint(pGraphicsI);
	} else return false;
	return bRet;
}
BITMAP* CDrawableSelection::Capture(IGraphics *pGraphicsI, float zoom)
{
	CSize szMap;
	(*m_ppMainDrawable)->GetSize(szMap);

	CRect RectFull(0, 0, (int)((float)szMap.cx*zoom), (int)((float)szMap.cy*zoom));
	if(RectFull.Width()%2) RectFull.right++;

	CRect RectPortion = RectFull;

	// Standard bitmaps are expected to be in a 16 bits boundary
	int bmWidthBytes = RectFull.Width() * sizeof(WORD); 
	// Allocate enough memory for the complete 16 bits bitmap
	BITMAP *pBitmap = (BITMAP*)new BYTE[sizeof(BITMAP) + RectFull.Height() * bmWidthBytes];

	ZeroMemory(pBitmap, sizeof(BITMAP));
	pBitmap->bmBits = (BYTE*)pBitmap + sizeof(BITMAP);
	pBitmap->bmWidth = RectFull.Width();
	pBitmap->bmHeight = RectFull.Height();
	pBitmap->bmWidthBytes = bmWidthBytes;
	pBitmap->bmBitsPixel = sizeof(WORD)*8;

	CSprite::SetShowOptions(SPRITE_ENTITIES); // show entities
	pGraphicsI->SetClearColor((*m_ppMainDrawable)->GetBkColor());
	// BeginCapture() and EndCapture() return 16 bits bitmaps in the RGB555 format.
	if(pGraphicsI->BeginCapture(&RectPortion, zoom)) {
		(*m_ppMainDrawable)->Draw(pGraphicsI);
		pGraphicsI->EndCapture((WORD*)pBitmap->bmBits, RectPortion, RectFull);
	}
	while(RectPortion.bottom!= RectFull.bottom || RectPortion.right!=RectFull.right) {
		if(RectPortion.right != RectFull.right) {
			RectPortion.left = RectPortion.right;
			RectPortion.right = RectFull.right;
		} else {
			RectPortion.left = RectFull.left;
			RectPortion.right = RectFull.right;
			RectPortion.top = RectPortion.bottom;
			RectPortion.bottom = RectFull.bottom;
		}
		if(pGraphicsI->BeginCapture(&RectPortion, zoom)) {
			(*m_ppMainDrawable)->Draw(pGraphicsI);
			pGraphicsI->EndCapture((WORD*)pBitmap->bmBits, RectPortion, RectFull);
		}
	} 

	return pBitmap;
}

// if(zoom == 0) zooming is calculated automagically
BITMAP* CDrawableSelection::CaptureSelection(IGraphics *pGraphicsI, float zoom)
{
	CRect RectFull;
	GetBoundingRect(&RectFull);
	if(zoom == 0) {
		zoom = 1.0f / max((float)RectFull.Width()/100.0f, (float)RectFull.Height()/100.0f);
		if(zoom > 3) zoom = 2;
		else if(zoom > 1) zoom = 1;
	}
	ASSERT(zoom != 0);

	RectFull.top = (int)((float)RectFull.top * zoom);
	RectFull.bottom = (int)((float)RectFull.bottom * zoom);
	RectFull.left = (int)((float)RectFull.left * zoom);
	RectFull.right = (int)((float)RectFull.right * zoom);
	if(RectFull.Width()%2) RectFull.right++;

	CRect RectPortion = RectFull;

	// Standard bitmaps are expected to be in a 16 bits boundary
	int bmWidthBytes = RectFull.Width() * sizeof(WORD); 
	// Allocate enough memory for the complete 16 bits bitmap Quest Designer Sprite Set
	BITMAP *pBitmap = (BITMAP*)new BYTE[sizeof(BITMAP) + RectFull.Height() * bmWidthBytes];

	ZeroMemory(pBitmap, sizeof(BITMAP));
	pBitmap->bmBits = (BYTE*)pBitmap + sizeof(BITMAP);
	pBitmap->bmWidth = RectFull.Width();
	pBitmap->bmHeight = RectFull.Height();
	pBitmap->bmWidthBytes = bmWidthBytes;
	pBitmap->bmBitsPixel = sizeof(WORD)*8;

	CSprite::SetShowOptions(SPRITE_ENTITIES); // show entities
	pGraphicsI->SetClearColor(COLOR_ARGB(255,255,255,255));
	// BeginCapture() and EndCapture() return 16 bits bitmaps in the RGB555 format.
	if(pGraphicsI->BeginCapture(&RectPortion, zoom)) {
		(*m_ppMainDrawable)->DrawSelected(pGraphicsI);
		pGraphicsI->EndCapture((WORD*)pBitmap->bmBits, RectPortion, RectFull);
	}
	while(RectPortion.bottom!= RectFull.bottom || RectPortion.right!=RectFull.right) {
		if(RectPortion.right != RectFull.right) {
			RectPortion.left = RectPortion.right;
			RectPortion.right = RectFull.right;
		} else {
			RectPortion.left = RectFull.left;
			RectPortion.right = RectFull.right;
			RectPortion.top = RectPortion.bottom;
			RectPortion.bottom = RectFull.bottom;
		}
		if(pGraphicsI->BeginCapture(&RectPortion, zoom)) {
			(*m_ppMainDrawable)->DrawSelected(pGraphicsI);
			pGraphicsI->EndCapture((WORD*)pBitmap->bmBits, RectPortion, RectFull);
		}
	} 

	return pBitmap;
}
void CDrawableSelection::CleanPasteGroups()
{
	// NEED TO FIX *** shouldn't we start on group 0:
	for(int i=1; i<(int)m_Groups.size(); i++) {
		m_nCurrentGroup = 0;
		m_Groups[i].O.clear();
	}
}

int CDrawableSelection::SetNextPasteGroup(LPCSTR szGroupName, int nNew)
{
	if(nNew >= 0 && nNew < (int)m_Groups.size()) m_nPasteGroup = nNew;
	else if(nNew != -1) return m_nPasteGroup;

	// find an empty existent group to paste the sprites:
	for(int i=1; i<(int)m_Groups.size() && nNew==-1; i++) {
		if(m_Groups[i].O.empty()) nNew = i;
	}
	// if no empty group was found, increase the number of groups:
	if(nNew == -1) {
		nNew = m_Groups.size();
		m_Groups.resize(nNew + 4);		 // increase by 4 groups each time it's needed (increasing a vector's size is costly)
	}
	ASSERT(nNew >= 0 && nNew < (int)m_Groups.size());

	// NEED TO FIX *** (If we are exlusively editing a group, we add the new paste group to that group:??)
	if(m_bHoldSelection) {
		m_Groups[nNew].P = m_nCurrentGroup; // the new group's parent is the current group.
	}
	m_Groups[nNew].Name = szGroupName;
	m_nPasteGroup = nNew;

	return m_nPasteGroup;
}
SObjProp* CDrawableSelection::GetFirstSelection()
{
	m_CurrentSel = m_Groups[0].O.begin();
	if(m_CurrentSel == m_Groups[0].O.end()) return NULL;
	return (m_CurrentSel++).operator ->();
}

SObjProp* CDrawableSelection::GetNextSelection() 
{
	if(m_CurrentSel == NULL) return NULL;
	if(m_CurrentSel == m_Groups[0].O.end()) return NULL;
	return (m_CurrentSel++).operator ->();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CSpriteSelection::BuildRealSelectionBounds(int nGroup_)
{
	CRect RectTmp;
	m_rcSelection.SetRectEmpty();
	vectorObject::iterator Iterator;
	// NEED TO FIX *** build from the current selection [m_nCurrentGroup] or from the main selection [0]??
	for(Iterator = m_Groups[nGroup_].O.begin(); Iterator != m_Groups[nGroup_].O.end(); Iterator++) {
		if(Iterator->pContext) {
			Iterator->pContext->GetAbsFinalRect(RectTmp);
		} else {
			BuildRealSelectionBounds(Iterator->nGroup);
			GetBoundingRect(&RectTmp, Iterator->nGroup);
		}
		// The main selection (nGroup_==0) updates the selection rect:
		if(nGroup_ == 0) m_rcSelection.UnionRect(m_rcSelection, RectTmp);
		// We need to keep the initial size and location of every selected object:
		Iterator->rcRect = RectTmp;
	}
}

void CSpriteSelection::SetSelectionName(LPCSTR szName)
{
	m_Groups[m_nCurrentGroup].Name = szName;
}

LPCSTR CSpriteSelection::GetSelectionName(LPSTR szName, int size)
{
	SObjProp *pSel = GetFirstSelection();
	*szName = '\0';

	if(m_nCurrentGroup>0 && m_nCurrentGroup<(int)m_Groups.size()) {
		strncpy(szName+1, (LPCSTR)m_Groups[m_nCurrentGroup].Name.c_str(), size-2);
		*szName = '@'; // To recognize them from regular sprites, Sprite Sets names internally start with '@' (as long as the name is not changed)
	} else if(pSel) {
		strncpy(szName, (LPCSTR)pSel->pContext->GetObjName(), size-1);
	}

	szName[size - 1] = '\0';

	return szName;
}

bool CSpriteSelection::Draw(const IGraphics *pGraphics_) {
	CRect Rect(0,0,0,0);
	CRect RectTmp;

	// Always draws what's in group 0 (the current selection group)
	int nSelected = (int)m_Groups[m_nCurrentGroup].O.size();
	const CSpriteContext *scontext = NULL;

	vectorObject::iterator Iterator;
	for(Iterator = m_Groups[m_nCurrentGroup].O.begin(); Iterator != m_Groups[m_nCurrentGroup].O.end(); Iterator++) {
		if(Iterator->pContext == NULL) {
			// NEED TO FIX *** (Show subgroups as orange selections:)
			GetBoundingRect(&RectTmp, Iterator->nGroup);
			pGraphics_->BoundingBox(RectTmp, COLOR_ARGB(160,255,192,96));
			Rect.UnionRect(Rect, RectTmp); // Add the boundaries to the final Rect
			continue;
		}

		scontext = static_cast<const CSpriteContext*>(Iterator->pContext);
		scontext->GetAbsFinalRect(RectTmp);
		Rect.UnionRect(Rect, RectTmp); // Add the boundaries to the final Rect

		if(nSelected>1) {
			if(!m_bHighlightOnly && !m_bFloating) {
				if(Iterator->bSubselected || !m_bHoldSelection) {
					if(m_bHoldSelection) {
						// if the object is subselected and the selection is held, draw in other color:
						pGraphics_->FillRect(RectTmp, COLOR_ARGB(92,128,128,255));
					} else {
						// only fill the selection if it isn't held:
//						if(!m_bHoldSelection) pGraphics_->FillRect(RectTmp, COLOR_ARGB(25,255,255,225));
					}
					// draw a bounding rect over the selected object:
					pGraphics_->BoundingBox(RectTmp, COLOR_ARGB(80,255,255,225));
				}
			}
			if(!m_bHighlightOnly) {
				// Draw the horizontal chain of the object:
				CRect rcSpecial, rcArrow;
				rcSpecial = RectTmp;
				if(Iterator->eXChain == left) {
					rcSpecial.right = rcSpecial.left+2;
					pGraphics_->FillRect(rcSpecial, COLOR_ARGB(255,255,0,0));
				} else if(Iterator->eXChain == right) {
					rcSpecial.left = rcSpecial.right-2;
					pGraphics_->FillRect(rcSpecial, COLOR_ARGB(255,255,0,0));
				} else if(Iterator->eXChain == fixed) {
					rcSpecial.right = rcSpecial.left+2;
					pGraphics_->FillRect(rcSpecial, COLOR_ARGB(255,0,0,255));
					rcSpecial = RectTmp;
					rcSpecial.left = rcSpecial.right-2;
					pGraphics_->FillRect(rcSpecial, COLOR_ARGB(255,0,0,255));
				} else if(Iterator->eXChain == stretch) {
					rcSpecial.top = rcSpecial.top+rcSpecial.Height()/2;
					rcSpecial.bottom = rcSpecial.top+1;
					pGraphics_->FillRect(rcSpecial, COLOR_ARGB(255,0,0,255));

					rcArrow = rcSpecial;
					rcArrow.bottom++;
					rcArrow.top--;
					rcArrow.left++;
					rcArrow.right = rcArrow.left+1;
					pGraphics_->FillRect(rcArrow, COLOR_ARGB(255,0,0,255));

					rcArrow = rcSpecial;
					rcArrow.bottom++;
					rcArrow.top--;
					rcArrow.right--;
					rcArrow.left = rcArrow.right-1;
					pGraphics_->FillRect(rcArrow, COLOR_ARGB(255,0,0,255));
				}

				// Draw the vertical chain of the object:
				rcSpecial = RectTmp;
				if(Iterator->eYChain == up) {
					rcSpecial.bottom = rcSpecial.top+2;
					pGraphics_->FillRect(rcSpecial, COLOR_ARGB(255,255,0,0));
				} else if(Iterator->eYChain == down) {
					rcSpecial.top = rcSpecial.bottom-2;
					pGraphics_->FillRect(rcSpecial, COLOR_ARGB(255,255,0,0));
				} else if(Iterator->eYChain == fixed) {
					rcSpecial.bottom = rcSpecial.top+2;
					pGraphics_->FillRect(rcSpecial, COLOR_ARGB(255,0,0,255));
					rcSpecial = RectTmp;
					rcSpecial.top = rcSpecial.bottom-2;
					pGraphics_->FillRect(rcSpecial, COLOR_ARGB(255,0,0,255));
				} else if(Iterator->eYChain == stretch) {
					rcSpecial.left = rcSpecial.left+rcSpecial.Width()/2;
					rcSpecial.right = rcSpecial.left+1;
					pGraphics_->FillRect(rcSpecial, COLOR_ARGB(255,0,0,255));

					rcArrow = rcSpecial;
					rcArrow.right++;
					rcArrow.left--;
					rcArrow.top++;
					rcArrow.bottom = rcArrow.top+1;
					pGraphics_->FillRect(rcArrow, COLOR_ARGB(255,0,0,255));

					rcArrow = rcSpecial;
					rcArrow.right++;
					rcArrow.left--;
					rcArrow.bottom--;
					rcArrow.top = rcArrow.bottom-1;
					pGraphics_->FillRect(rcArrow, COLOR_ARGB(255,0,0,255));
				}
			}
		}
	}
	m_bCanMove = true;
	m_bCanResize = true;

	// Show the final boundary box:
	if(!m_bFloating) {
		if(m_bHighlightOnly) {
			if(nSelected>0) {
				// draw a bounding rect over the selected objects:
				pGraphics_->FillRect(Rect, COLOR_ARGB(64,255,255,0));
				pGraphics_->BoundingBox(Rect, COLOR_ARGB(255,255,0,0));
			}
		} else {
			if(nSelected>1) {
				if(m_bHoldSelection) pGraphics_->SelectionBox(Rect, COLOR_ARGB(128,160,160,160));
				else {
					if(isGroup()) pGraphics_->SelectionBox(Rect, COLOR_ARGB(255,255,192,96));
 					else pGraphics_->SelectionBox(Rect, COLOR_ARGB(255,255,255,200));
				}
			} else if(nSelected==1) {
				if(scontext->isTiled()) {
					pGraphics_->SelectionBox(Rect, COLOR_ARGB(200,255,255,200));
				} else {
					m_bCanResize = false;
					pGraphics_->BoundingBox(Rect, COLOR_ARGB(255,255,255,200));
				}
			}
		}
	}

	if(m_eCurrentState==eSelecting) {
		RectTmp = m_rcSelection;
		RectTmp.NormalizeRect();
		if(RectTmp.Width()>1 && RectTmp.Height()>1) {
			pGraphics_->SelectingBox(m_rcSelection, COLOR_ARGB(128,255,255,255));
		}
	}
	return true;
}

// this function expects normalized rects
// simpler, yet much better ResizeObject()
void CSpriteSelection::ResizeObject(const SObjProp &ObjProp_, const CRect &rcOldBounds_, const CRect &rcNewBounds_, bool bAllowResize_)
{
	if(!ObjProp_.pContext) {
		vectorObject::const_iterator Iterator;
		for(Iterator = m_Groups[ObjProp_.nGroup].O.begin(); Iterator != m_Groups[ObjProp_.nGroup].O.end(); Iterator++) {
			ResizeObject(*Iterator, rcOldBounds_, rcNewBounds_, bAllowResize_);
		}
//		GetBoundingRect(&RectTmp, ObjProp_.nGroup);
		return;
	}

	CSpriteContext *scontext = static_cast<CSpriteContext*>(ObjProp_.pContext);
	if(!scontext->isTiled()) bAllowResize_ = false;

	int w = ObjProp_.rcRect.Width();
	int h = ObjProp_.rcRect.Height();
	// Won't resize to a null size.
	if(w<=0 || h<=0) return;

	float xFactor=1.0f, yFactor=1.0f;
	_Chain xChain = ObjProp_.eXChain;
	_Chain yChain = ObjProp_.eYChain;

	if(!bAllowResize_) {
		if(ObjProp_.rcRect.left == rcOldBounds_.left && ObjProp_.rcRect.right == rcOldBounds_.right) {
			if(m_bCursorLeft) xChain = right;
			else  xChain = left;
		} else if(ObjProp_.rcRect.left == rcOldBounds_.left) xChain = left;
		else if(ObjProp_.rcRect.right == rcOldBounds_.right) xChain = right;

		if(ObjProp_.rcRect.top == rcOldBounds_.top && ObjProp_.rcRect.bottom == rcOldBounds_.bottom) {
			if(m_bCursorTop) yChain = down;
			else yChain = up;
		} else if(ObjProp_.rcRect.top == rcOldBounds_.top) yChain = up;
		else if(ObjProp_.rcRect.bottom == rcOldBounds_.bottom) yChain = down;
	}

	CRect Rect;
	if(xChain == left) {
		Rect.left = rcNewBounds_.left + (ObjProp_.rcRect.left - rcOldBounds_.left);
		Rect.right = Rect.left + w;
	} else if(xChain == right) {
		Rect.right = rcNewBounds_.right - (rcOldBounds_.right - ObjProp_.rcRect.right);
		Rect.left = Rect.right - w;
	} else if(xChain == fixed || !bAllowResize_) {
		if(rcOldBounds_.Width()) xFactor = (float)rcNewBounds_.Width()/(float)rcOldBounds_.Width();
		Rect.left = rcNewBounds_.left + (int)(((float)ObjProp_.rcRect.left + (float)w/2.0f - (float)rcOldBounds_.left) * xFactor - (float)w/2.0f);
		Rect.right = Rect.left + w;
	} else if(xChain == stretch) {
		Rect.left = rcNewBounds_.left + (ObjProp_.rcRect.left - rcOldBounds_.left);
		Rect.right = rcNewBounds_.right - (rcOldBounds_.right - ObjProp_.rcRect.right);
	} else {
		if(rcOldBounds_.Width()) xFactor = (float)rcNewBounds_.Width()/(float)rcOldBounds_.Width();
		Rect.left = rcNewBounds_.left + (int)((float)(ObjProp_.rcRect.left - rcOldBounds_.left) * xFactor);
		Rect.right = rcNewBounds_.right - (int)((float)(rcOldBounds_.right - ObjProp_.rcRect.right) * xFactor);
	}

	if(yChain == up) {
		Rect.top = rcNewBounds_.top + (ObjProp_.rcRect.top - rcOldBounds_.top);
		Rect.bottom = Rect.top + h;
	} else if(yChain == down) {
		Rect.bottom = rcNewBounds_.bottom - (rcOldBounds_.bottom - ObjProp_.rcRect.bottom);
		Rect.top = Rect.bottom - h;
	} else if(yChain == fixed || !bAllowResize_) {
		if(rcOldBounds_.Height()) yFactor = (float)rcNewBounds_.Height()/(float)rcOldBounds_.Height();
		Rect.top = rcNewBounds_.top + (int)(((float)ObjProp_.rcRect.top + (float)h/2.0f - (float)rcOldBounds_.top) * yFactor - (float)h/2.0f);
		Rect.bottom = Rect.top + h;
	} else if(yChain == stretch) {
		Rect.top = rcNewBounds_.top + (ObjProp_.rcRect.top - rcOldBounds_.top);
		Rect.bottom = rcNewBounds_.bottom - (rcOldBounds_.bottom - ObjProp_.rcRect.bottom);
	} else {
		if(rcOldBounds_.Height()) yFactor = (float)rcNewBounds_.Height()/(float)rcOldBounds_.Height();
		Rect.top = rcNewBounds_.top + (int)((float)(ObjProp_.rcRect.top - rcOldBounds_.top) * yFactor);
		Rect.bottom = rcNewBounds_.bottom - (int)((float)(rcOldBounds_.bottom - ObjProp_.rcRect.bottom) * yFactor);
	}

	// Adjust non-resizable sprites:
	if(!bAllowResize_ || yChain == fixed || yChain == up || yChain == down) {
		if(m_bCursorTop) {
			if(Rect.top > rcNewBounds_.bottom-h) Rect.top = rcNewBounds_.bottom - h;
			Rect.bottom = Rect.top + h;
		} else {
			if(Rect.bottom < rcNewBounds_.top+h) Rect.bottom = rcNewBounds_.top + h;
			Rect.top = Rect.bottom - h;
		}
	}
	if(!bAllowResize_ || xChain == fixed || xChain == left || xChain == right) {
		if(m_bCursorLeft) {
			if(Rect.left > rcNewBounds_.right-w) Rect.left = rcNewBounds_.right - w;
			Rect.right  = Rect.left + w;
		} else {
			if(Rect.right < rcNewBounds_.left+w) Rect.right = rcNewBounds_.left + w;
			Rect.left  = Rect.right - w;
		}
	}

	if(rcOldBounds_.Width() != rcNewBounds_.Width()) {
		if(/*(bAllowResize_ && xChain == relative) ||*/ (bAllowResize_ && xChain == stretch)) {
			w = Rect.Width();
			Rect.left = m_nSnapSize*(Rect.left/m_nSnapSize);
			if(Rect.Width() <= 0) Rect.left = Rect.right-m_nSnapSize;
			if(Rect.left<rcNewBounds_.left) Rect.left = rcNewBounds_.left;
			Rect.right = m_nSnapSize*((Rect.left+w+m_nSnapSize-1)/m_nSnapSize);
			if(Rect.Width() <= 0) Rect.right = Rect.left+m_nSnapSize;
			if(Rect.right>rcNewBounds_.right) Rect.right = rcNewBounds_.right;
		}
	}

	if(rcOldBounds_.Height() != rcNewBounds_.Height()) {
		if(/*(bAllowResize_ && yChain == relative) ||*/ (bAllowResize_ && yChain == stretch)) {
			h = Rect.Height();
			Rect.top = m_nSnapSize*(Rect.top/m_nSnapSize);
			if(Rect.Height() <= 0) Rect.top = Rect.bottom-m_nSnapSize;
			if(Rect.top<rcNewBounds_.top) Rect.top = rcNewBounds_.top;
			Rect.bottom = m_nSnapSize*((Rect.top+h+m_nSnapSize-1)/m_nSnapSize);
			if(Rect.Height() <= 0) Rect.bottom = Rect.top+m_nSnapSize;
			if(Rect.bottom>rcNewBounds_.bottom) Rect.bottom = rcNewBounds_.bottom;
		}
	}

	ASSERT(Rect.right>Rect.left && Rect.bottom>Rect.top);
	// Updating ObjProp_ is very important, otherwise groups won't get updated:
	scontext->SetAbsFinalRect(Rect);
}

// pBitmap must point to a block of memory allocated by the Kernel (i.e. via CaptureSelection or similars)
HGLOBAL CSpriteSelection::Copy(BITMAP **ppBitmap, bool bDeleteBitmap)
{
	BITMAP *pBitmap = NULL;
	if(ppBitmap) pBitmap = *ppBitmap;

	// Now we are ready to copy the objects's information into the sprite set,
	// we start by sorting the selected objects (in the selection):
	SortSelection();

	if(m_Groups[0].O.size() == 0) {
		if(bDeleteBitmap && ppBitmap) {
			*ppBitmap = NULL;
			delete []pBitmap;
		}
		return NULL;
	}

	int bmpsize = 0;
	int indexsize = 0;
	int datasize = sizeof(_SpriteSet::_SpriteSetInfo);
	if(pBitmap) bmpsize = sizeof(BITMAP) + pBitmap->bmHeight * pBitmap->bmWidthBytes;

	///////////////////////////////////////////////////////////////////////////////////////
	// Analize the selected objeects to find out the needed size for the sprite set:

	// we need a map to hold every different object (to make an object index.)
	std::map<CDrawableObject*, int> ObjectsNames;

	vectorObject::iterator Iterator = m_Groups[0].O.begin();
	for(int i=0; Iterator != m_Groups[0].O.end(); i++, Iterator++) {
		if(Iterator->pContext == NULL) continue; // NEED TO FIX *** add group stuff

		CSpriteContext *scontext = static_cast<CSpriteContext*>(Iterator->pContext);
		ASSERT(scontext);
		SObjProp *pObjProp = static_cast<SObjProp *>(Iterator.operator ->());
		CSprite *pSprite = static_cast<CSprite *>(scontext->GetDrawableObj());
		ASSERT(pSprite);
		ASSERT(pObjProp);

		datasize += sizeof(_SpriteSet::_SpriteSetData);

		// if it is not an entity, with and height it will need mask: SSD_WIDTHHEIGHT
		if(pSprite->GetSpriteType() != tEntity) {
			datasize += sizeof(_SpriteSet::_SpriteSetData01);
		}

		// if it has chains or transformations it will need mask: SSD_CHAIN_X, SSD_CHAIN_Y, and SSD_TRANS
		if( pObjProp->eXChain != relative || pObjProp->eYChain != relative ||
			scontext->isMirrored() || scontext->isFlipped() || scontext->Rotation() ) {
			datasize += sizeof(_SpriteSet::_SpriteSetData02);
		}

		// if it has an alpha value set, it will need mask: SSD_ALPHA
		if( scontext->getARGB().rgbAlpha != 255 ) {
			datasize += sizeof(_SpriteSet::_SpriteSetData03);
		}

		// if it has an alpha value set, it will need mask: SSD_RGBL
		if( scontext->getRGB().dwColor != COLOR_RGB(128,128,128).dwColor ) {
			datasize += sizeof(_SpriteSet::_SpriteSetData04);
		}

		// try to insert the drawable object in the index...
		if( ObjectsNames.insert(
				std::pair<CDrawableObject*, int>(scontext->GetDrawableObj(), -1)
			).second == true) {
			// a new item has been found (and inserted), so we increase the index size
			int nNameLen = strlen(scontext->GetObjName()) + 1;
			ASSERT(nNameLen < 30);
			indexsize += nNameLen + sizeof(WORD); // name length + offset of the name in the index.
		}
	}
	int asize = ((datasize + indexsize + 15) / 16) * 16; // align to 16 bytes.

	// Now that we know the exact space all the sprites will need, we allocate it:
	HGLOBAL hglbCopy = ::GlobalAlloc(GMEM_MOVEABLE, asize + bmpsize); 
	if(hglbCopy == NULL) {
		if(bDeleteBitmap && ppBitmap) {
			*ppBitmap = NULL;
			delete []pBitmap;
		}
		return NULL;
	}

 	// Lock the handle and copy the text to the buffer. 
	LPBYTE pRawBuffer = (LPBYTE)::GlobalLock(hglbCopy);
	ASSERT(pRawBuffer);
#ifdef _DEBUG
	memset(pRawBuffer, 0, asize + bmpsize);
#else 
	memset(pRawBuffer, 0, sizeof(_SpriteSet::_SpriteSetInfo)); // clear the header.
#endif

	CRect rcBoundaries, RectTmp;
	GetBoundingRect(&rcBoundaries);

	// NEED TO FIX *** (sprite sets sprite's whithin the selection should not be copied as sprites, but as a single sprite sets)

	_SpriteSet *CopyBoard = (_SpriteSet*)pRawBuffer;
	strcpy(CopyBoard->Info.Header.ID, OLF_SPRITE_SET_ID);
	strcat(CopyBoard->Info.Header.ID, "\nUntitled Sprite Set\nDescription goes here.");
	CopyBoard->Info.Header.dwSignature = OLF_SPRITE_SET_SIGNATURE;
	CopyBoard->Info.Header.dwSize = asize + bmpsize;
	CopyBoard->Info.nSelected = (int)m_Groups[0].O.size();

	CopyBoard->Info.rcBoundaries = rcBoundaries;
	CopyBoard->Info.rcBoundaries.OffsetRect(-rcBoundaries.TopLeft());
	CopyBoard->Info.Header.dwBitmapOffset = 0;
	CopyBoard->Info.Header.dwDataOffset = sizeof(_SpriteSet::_SpriteSetInfo) + indexsize;

	LPWORD pIndexOffset = (LPWORD)(pRawBuffer + sizeof(_SpriteSet::_SpriteSetInfo));
	LPSTR szIndexNames = (LPSTR)(pIndexOffset + ObjectsNames.size());

	LPBYTE pData = pRawBuffer + CopyBoard->Info.Header.dwDataOffset;

	// fill the index:
	int index = 0;
	std::map<CDrawableObject*, int>::iterator IndexIterator = ObjectsNames.begin();
	while(IndexIterator != ObjectsNames.end()) {
		ASSERT(pIndexOffset < (LPWORD)szIndexNames);
		ASSERT((LPBYTE)szIndexNames < pData);
		*pIndexOffset++ = (LPBYTE)szIndexNames - pRawBuffer;
		strcpy(szIndexNames, IndexIterator->first->GetName());
		szIndexNames += (strlen(szIndexNames) + 1);
		IndexIterator->second = index++;
		IndexIterator++;
	}

	Iterator = m_Groups[0].O.begin();
	for(i=0; Iterator != m_Groups[0].O.end(); i++, Iterator++) {
		if(Iterator->pContext == NULL) continue; // NEED TO FIX *** add group stuff
		CSpriteContext *scontext = static_cast<CSpriteContext*>(Iterator->pContext);
		ASSERT(scontext);
		SObjProp *pObjProp = static_cast<SObjProp *>(Iterator.operator ->());
		CSprite *pSprite = static_cast<CSprite *>(scontext->GetDrawableObj());
		ASSERT(pSprite);
		ASSERT(pObjProp);

		int Mask = 0;
		_SpriteSet::_SpriteSetData01 *pSpriteSetData01 = NULL;
		_SpriteSet::_SpriteSetData02 *pSpriteSetData02 = NULL;
		_SpriteSet::_SpriteSetData03 *pSpriteSetData03 = NULL;
		_SpriteSet::_SpriteSetData04 *pSpriteSetData04 = NULL;

		// Search for all required options and mask. 

		_SpriteSet::_SpriteSetData *pSpriteSetData = (_SpriteSet::_SpriteSetData *)pData;
		pData += sizeof(_SpriteSet::_SpriteSetData);

		// if it is not an entity, with and height it will need mask: SSD_WIDTHHEIGHT
		if(pSprite->GetSpriteType() != tEntity) {
			Mask |= SSD_WIDTHHEIGHT;
			pSpriteSetData01 = (_SpriteSet::_SpriteSetData01 *)pData;
			pData += sizeof(_SpriteSet::_SpriteSetData01);
		}

		// if it has chains or transformations it will need mask: SSD_CHAIN_X, SSD_CHAIN_Y, and SSD_TRANS
		if( pObjProp->eXChain != relative || pObjProp->eYChain != relative ||
			scontext->isMirrored() || scontext->isFlipped() || scontext->Rotation() ) {
			Mask |= SSD_TRANS;
			if(pObjProp->eXChain != relative) Mask |= SSD_CHAIN_X;
			if(pObjProp->eYChain != relative) Mask |= SSD_CHAIN_Y;
			pSpriteSetData02 = (_SpriteSet::_SpriteSetData02 *)pData;
			pData += sizeof(_SpriteSet::_SpriteSetData02);
		}

		// if it has an alpha value set, it will need mask: SSD_ALPHA
		if( scontext->getARGB().rgbAlpha != 255 ) {
			Mask |= SSD_ALPHA;
			pSpriteSetData03 = (_SpriteSet::_SpriteSetData03 *)pData;
			pData += sizeof(_SpriteSet::_SpriteSetData03);
		}

		// if it has an alpha value set, it will need mask: SSD_RGBL
		if( scontext->getRGB().dwColor != COLOR_RGB(128,128,128).dwColor ) {
			Mask |= SSD_RGBL;
			pSpriteSetData04 = (_SpriteSet::_SpriteSetData04 *)pData;
			pData += sizeof(_SpriteSet::_SpriteSetData04);
		}

		///////////////////////////////////////////////////////////////////////////
		// Now fill in the needed data:

		scontext->GetAbsFinalRect(RectTmp);
		RectTmp.OffsetRect(-rcBoundaries.TopLeft());

		ASSERT(pSpriteSetData);

		ASSERT(Mask < 0x0040);
		ASSERT(scontext->GetObjLayer() < 0x0008);
		ASSERT(scontext->GetObjSubLayer() < 0x0008);

		pSpriteSetData->Mask = Mask;
		pSpriteSetData->Layer = scontext->GetObjLayer();
		pSpriteSetData->SubLayer = scontext->GetObjSubLayer();
		pSpriteSetData->X = (WORD)RectTmp.left;
		pSpriteSetData->Y = (WORD)RectTmp.top;

		IndexIterator = ObjectsNames.find(scontext->GetDrawableObj());
		ASSERT(IndexIterator != ObjectsNames.end());
		pSpriteSetData->ObjIndex = IndexIterator->second;
		ASSERT(pSpriteSetData->ObjIndex != -1);

		if(pSpriteSetData01) { // SSD_WIDTHHEIGHT
			ASSERT(RectTmp.Width() < 0x1000);
			ASSERT(RectTmp.Height() < 0x1000);

			pSpriteSetData01->Width = RectTmp.Width();
			pSpriteSetData01->Height = RectTmp.Height();
		}
		if(pSpriteSetData02) { // SSD_CHAIN_X, SSD_CHAIN_Y, and SSD_TRANS
			ASSERT(scontext->Rotation() < 0x0004);

			pSpriteSetData02->rotation = scontext->Rotation();
			pSpriteSetData02->mirrored = scontext->isMirrored();
			pSpriteSetData02->flipped = scontext->isFlipped();
			pSpriteSetData02->XChain = ((unsigned)pObjProp->eXChain - 1);
			pSpriteSetData02->YChain = ((unsigned)pObjProp->eYChain - 1);
		}
		if(pSpriteSetData03) { // SSD_ALPHA
			pSpriteSetData03->Alpha = scontext->getARGB().rgbAlpha;
		}
		if(pSpriteSetData04) { // SSD_RGBL
			pSpriteSetData04->Red = scontext->getARGB().rgbRed;
			pSpriteSetData04->Green = scontext->getARGB().rgbGreen;
			pSpriteSetData04->Blue = scontext->getARGB().rgbBlue;
		}
		ASSERT(pData <= pRawBuffer + indexsize + datasize);
	}

	ASSERT(pData == pRawBuffer + indexsize + datasize);
	CONSOLE_DEBUG("Sprite Set created, %d bytes: %d bytes used for data, and %d bytes for the thumbnail\n", asize + bmpsize, datasize, bmpsize);

	if(pBitmap) {
		CopyBoard->Info.Header.dwBitmapOffset = (DWORD)asize; // offset of the bitmap
		memcpy(pRawBuffer + asize, pBitmap, sizeof(BITMAP));
		memcpy(pRawBuffer + asize + sizeof(BITMAP), pBitmap->bmBits, bmpsize - sizeof(BITMAP));
	}

	::GlobalUnlock(hglbCopy); 

	if(bDeleteBitmap && ppBitmap) {
		*ppBitmap = NULL;
		delete []pBitmap; // free bitmap memory
	}
	return hglbCopy;
}
CRect CSpriteSelection::PasteSprite(CLayer *pLayer, CSprite *pSprite, const CPoint *pPoint, bool bPaste) 
{
	CRect RetRect(0,0,0,0);

	// PasteSprite needs no paste group, so we clear the paste group if set:
	m_nPasteGroup = 0;
	m_nCurrentGroup = 0;

	if(pSprite->GetSpriteType() == tMask) {
		if(!pPoint && bPaste) CONSOLE_PRINTF("Paste error: Attempt to use mask '%s' as a sprite\n", pSprite->GetName());
		return CRect(0,0,0,0);
	}
	if(!pLayer) {
		if(!pPoint && bPaste) CONSOLE_PRINTF("Paste error : Couldn't paste '%s' in the requested layer!\n", pSprite->GetName());
		return CRect(0,0,0,0);
	}

	CSize Size;
	pSprite->GetSize(Size);
	RetRect.SetRect(0, 0, Size.cx, Size.cy);

	if(bPaste) {
		if(!pPoint) CleanSelection();

		CSpriteContext *pSpriteContext = new CSpriteContext("");
		pSpriteContext->SetDrawableObj(pSprite);

		if(pSprite->GetSpriteType() == tBackground) pSpriteContext->Tile();
		pSpriteContext->SetObjSubLayer(static_cast<CBackground *>(pSprite)->GetObjSubLayer());
		pSpriteContext->Alpha(static_cast<CBackground *>(pSprite)->GetAlphaValue());

		// NEED TO FIX *** (Pasted sprite sets within pasted data, should be pasted here)
		pLayer->AddSpriteContext(pSpriteContext); // insert the sprite in the current layer
		if(!pPoint) {
			m_Groups[0].O.push_back(SObjProp(this, pSpriteContext, 0));
			pSpriteContext->MoveTo(0, 0);
			pSpriteContext->SelectContext();

			m_bFloating = true;
			StartMoving(CPoint(Size.cx/2, Size.cy/2));
		} else {
			pSpriteContext->MoveTo(*pPoint);
		}
	}

	if(pPoint) RetRect.OffsetRect(*pPoint);

	return RetRect;
}

CRect CSpriteSelection::PasteSprite(CLayer *pLayer, LPCSTR szSprite, const CPoint *pPoint, bool bPaste) 
{
	CSprite *pSprite = CGameManager::Instance()->FindSprite(szSprite);

	if(!pSprite) {
		if(!pPoint && bPaste) CONSOLE_PRINTF("Paste error : Couldn't find the requested sprite (1): '%s'!\n", szSprite);
		return CRect(0,0,0,0);
	}
	return PasteSprite(pLayer, pSprite, pPoint, bPaste);
}
CRect CSpriteSelection::PasteSpriteSet(CLayer *pLayer, const LPBYTE pRawBuffer, const CPoint *pPoint, bool bPaste)
{
	CRect RetRect(0,0,0,0);

	ASSERT(pRawBuffer);

	_SpriteSet *CopyBoard = (_SpriteSet*)pRawBuffer;

	if(!pPoint && bPaste) CleanSelection();
	LPSTR szName = NULL;

	LPWORD pIndexOffset = (LPWORD)((LPBYTE)CopyBoard + sizeof(_SpriteSet::_SpriteSetInfo));
	LPBYTE pData = (LPBYTE)CopyBoard + CopyBoard->Info.Header.dwDataOffset;

	UINT nLayer = -1;

	char szSpriteSetName[200];
	// get the next availible paste group:
	m_nCurrentGroup = SetNextPasteGroup(GetNameFromOLFile(&CopyBoard->Info.Header, szSpriteSetName, sizeof(szSpriteSetName)));
	CONSOLE_DEBUG("Sprite Set '%s' to be pasted as group #%d\n", szSpriteSetName, m_nPasteGroup);

	for(UINT i=0; i<CopyBoard->Info.nSelected; i++) {

		_SpriteSet::_SpriteSetData01 *pSpriteSetData01 = NULL;
		_SpriteSet::_SpriteSetData02 *pSpriteSetData02 = NULL;
		_SpriteSet::_SpriteSetData03 *pSpriteSetData03 = NULL;
		_SpriteSet::_SpriteSetData04 *pSpriteSetData04 = NULL;

		// Initialize structures from data and mask:
		_SpriteSet::_SpriteSetData *pSpriteSetData = (_SpriteSet::_SpriteSetData *)pData;
		pData += sizeof(_SpriteSet::_SpriteSetData);

		if((pSpriteSetData->Mask & SSD_WIDTHHEIGHT) == SSD_WIDTHHEIGHT) {
			pSpriteSetData01 = (_SpriteSet::_SpriteSetData01 *)pData;
			pData += sizeof(_SpriteSet::_SpriteSetData01);
		}
		if((pSpriteSetData->Mask & SSD_TRANS) == SSD_TRANS) {
			pSpriteSetData02 = (_SpriteSet::_SpriteSetData02 *)pData;
			pData += sizeof(_SpriteSet::_SpriteSetData02);
		}
		if((pSpriteSetData->Mask & SSD_ALPHA) == SSD_ALPHA) {
			pSpriteSetData03 = (_SpriteSet::_SpriteSetData03 *)pData;
			pData += sizeof(_SpriteSet::_SpriteSetData03);
		}
		if((pSpriteSetData->Mask & SSD_RGBL) == SSD_RGBL) {
			pSpriteSetData04 = (_SpriteSet::_SpriteSetData04 *)pData;
			pData += sizeof(_SpriteSet::_SpriteSetData04);
		}
		szName = (LPSTR)CopyBoard + pIndexOffset[pSpriteSetData->ObjIndex];

		// Build sprite:
		if(nLayer!=-1 || !pLayer) {
			if(nLayer != pSpriteSetData->Layer) {
				nLayer = pSpriteSetData->Layer;
				pLayer = static_cast<CLayer*>((*m_ppMainDrawable)->GetChild(nLayer));
			}
		} // else if there is a current layer selected, use that layer instead

		CSprite *pSprite = CGameManager::Instance()->FindSprite(szName);
		if(!pSprite) {
			if(!pPoint && bPaste) CONSOLE_PRINTF("Paste error : Couldn't find the requested sprite (2): '%s'!\n", szName);
			continue;
		}
		if(pSprite->GetSpriteType() == tMask) {
			if(!pPoint && bPaste) CONSOLE_PRINTF("Paste error: Attempt to use mask '%s' as a sprite\n", szName);
			continue;
		}
		if(!pLayer) {
			if(!pPoint && bPaste) CONSOLE_PRINTF("Paste error : Couldn't paste '%s' in the requested layer!\n", szName);
			continue;
		}

		// Start the Rect with the width and heignt of the sprite (defaults for entities)
		CRect RectTmp(pSpriteSetData->X, pSpriteSetData->Y, pSpriteSetData->X - 1, pSpriteSetData->Y - 1);

		if(bPaste) {
			// Fill sprite context data:

			ARGBCOLOR rgbColor = COLOR_ARGB(255,128,128,128);
			int rotation = 0;
			bool mirrored = false;
			bool flipped = false;
			
			_Chain XChain = relative;
			_Chain YChain = relative;

			CSpriteContext *pSpriteContext = new CSpriteContext(""); // New sprite context with no name.
			pSpriteContext->SetDrawableObj(pSprite);

			pSpriteContext->SetObjSubLayer(pSpriteSetData->SubLayer);

			// We ignore Width and Height from entities (shouldn't be there anyway) ...
			if(pSprite->GetSpriteType() != tEntity) {
				if(pSpriteSetData01) { //SSD_WIDTHHEIGHT
					ASSERT((pSpriteSetData->Mask & SSD_WIDTHHEIGHT) == SSD_WIDTHHEIGHT);
					RectTmp.right = RectTmp.left + pSpriteSetData01->Width;
					RectTmp.bottom = RectTmp.top + pSpriteSetData01->Height;
					pSpriteContext->Tile();
					if(RectTmp.right < RectTmp.left || RectTmp.bottom < RectTmp.top) {
						CONSOLE_PRINTF("Paste error: Corrupted Sprite: '%s' (in the Sprite Set '%s')\n", szName, szSpriteSetName);
						continue;
					}
				} else {
					CSize Size;
					pSprite->GetSize(Size);
					RectTmp.right = RectTmp.left + Size.cx;
					RectTmp.bottom = RectTmp.top + Size.cy;
				}
			}

			if(pSpriteSetData02) { //SSD_CHAIN_X, SSD_CHAIN_Y, and SSD_TRANS
				ASSERT((pSpriteSetData->Mask & SSD_TRANS) == SSD_TRANS);
				rotation = pSpriteSetData02->rotation;
				mirrored = pSpriteSetData02->mirrored;
				flipped = pSpriteSetData02->flipped;
				if((pSpriteSetData->Mask & SSD_CHAIN_X) == SSD_CHAIN_X)
					XChain = (_Chain)(pSpriteSetData02->XChain + 1);
				if((pSpriteSetData->Mask & SSD_CHAIN_Y) == SSD_CHAIN_Y)
					YChain = (_Chain)(pSpriteSetData02->YChain + 1);
			}
			if(pSpriteSetData03) { //SSD_ALPHA
				ASSERT((pSpriteSetData->Mask & SSD_ALPHA) == SSD_ALPHA);
				rgbColor.rgbAlpha = pSpriteSetData03->Alpha;
				if(rgbColor.rgbAlpha == 0) {
					CONSOLE_PRINTF("Paste warning: Invisible Sprite: '%s'\n", szName);
				}
			}
			if(pSpriteSetData04) { //SSD_RGBL
				ASSERT((pSpriteSetData->Mask & SSD_RGBL) == SSD_RGBL);
				rgbColor.rgbRed = pSpriteSetData04->Red;
				rgbColor.rgbGreen = pSpriteSetData04->Green;
				rgbColor.rgbBlue = pSpriteSetData04->Blue;
			}

			pSpriteContext->Rotate(rotation);
			pSpriteContext->Mirror(mirrored);
			pSpriteContext->Flip(flipped);
			pSpriteContext->ARGB(rgbColor);

			pLayer->AddSpriteContext(pSpriteContext); // insert the sprite in the current layer
			if(!pPoint) {
				// the sprite absolute postion must be set after inserting it in the layer.
				pSpriteContext->SelectContext();
				pSpriteContext->SetAbsFinalRect(RectTmp);

				if(m_nPasteGroup != 0) {
					// push to the current paste group:
					m_Groups[m_nPasteGroup].O.push_back(SObjProp(this, pSpriteContext, 0, RectTmp, XChain, YChain));
				} else {
					// push to the main group (the current selection group):
					m_Groups[0].O.push_back(SObjProp(this, pSpriteContext, 0, RectTmp, XChain, YChain));
				}
			} else {
				RectTmp.OffsetRect(*pPoint);
				pSpriteContext->SetAbsFinalRect(RectTmp);
				// push to the current paste group:
				if(m_nPasteGroup != 0) m_Groups[m_nPasteGroup].O.push_back(SObjProp(this, pSpriteContext, 0, RectTmp, XChain, YChain));
			}
		} else {
			if(pSpriteSetData01) { //SSD_WIDTHHEIGHT
				ASSERT((pSpriteSetData->Mask & SSD_WIDTHHEIGHT) == SSD_WIDTHHEIGHT);
				RectTmp.right = RectTmp.left + pSpriteSetData01->Width;
				RectTmp.bottom = RectTmp.top + pSpriteSetData01->Height;
			}
			if(pSpriteSetData02) { //SSD_CHAIN_X, SSD_CHAIN_Y, and SSD_TRANS
				ASSERT((pSpriteSetData->Mask & SSD_TRANS) == SSD_TRANS);
			}
			if(pSpriteSetData03) { //SSD_ALPHA
				ASSERT((pSpriteSetData->Mask & SSD_ALPHA) == SSD_ALPHA);
			}
			if(pSpriteSetData04) { //SSD_RGBL
				ASSERT((pSpriteSetData->Mask & SSD_RGBL) == SSD_RGBL);
			}
			if(pPoint) RectTmp.OffsetRect(*pPoint);
		}

		RetRect.UnionRect(RetRect, RectTmp);
	}

	// After we have our sprite set pasted (probably as a new group), we now select it:
	if(m_nPasteGroup != 0) { //If the paste group was the selection group, then it's already selected.
		bool bSingle = false;
		// there was only a single sprite in the sprite set, so we set the paste group to 0 and clear the used group:
		if(m_Groups[m_nPasteGroup].O.size() <= 1) bSingle = true;

		vectorObject::iterator Iterator;
		for(Iterator = m_Groups[m_nPasteGroup].O.begin(); Iterator != m_Groups[m_nPasteGroup].O.end(); Iterator++) {
			Iterator->pContext->SelectContext();
			m_Groups[0].O.push_back(*Iterator);
		}
		if(bSingle) {
			m_Groups[m_nPasteGroup].O.clear();
			m_nCurrentGroup = m_nPasteGroup = 0;
		}
	}

	if(!pPoint && bPaste) {
		m_bFloating = true;
		StartMoving(CopyBoard->Info.rcBoundaries.CenterPoint());
	}

	if(RetRect.IsRectNull()) CONSOLE_PRINTF("Paste error : Couldn't paste corrupted sprite set!\n");

	return RetRect;
}
CRect CSpriteSelection::PasteFile(CLayer *pLayer, LPCSTR szFilePath, const CPoint *pPoint, bool bPaste)
{
	CRect RetRect(0,0,0,0);

	// validate the possibility of receiving a group:
	if(*szFilePath == '@') szFilePath++;

	CVFile fnFile(szFilePath);
	if(!fnFile.FileExists()) return CRect(0,0,0,0);

	BYTE Header[sizeof(_SpriteSet::_SpriteSetInfo)];
	_SpriteSet *CopyBoard = (_SpriteSet*)Header;

	fnFile.Open("r");
	fnFile.Read(Header, sizeof(_SpriteSet::_SpriteSetInfo));
	fnFile.Close();
	if(strncmp(CopyBoard->Info.Header.ID, OLF_SPRITE_SET_ID, sizeof(OLF_SPRITE_SET_ID)-1)) CRect(0,0,0,0);
	else if(CopyBoard->Info.Header.dwSignature != OLF_SPRITE_SET_SIGNATURE) CRect(0,0,0,0);

	fnFile.Open("r");
	LPBYTE pRawBuffer = (LPBYTE)fnFile.ReadFile();
	RetRect = PasteSpriteSet(pLayer, pRawBuffer, pPoint, bPaste);
	fnFile.Close();

	return RetRect;
}
bool CSpriteSelection::GetPastedSize(LPCVOID pBuffer, SIZE *pSize)
{
	pSize->cx = 0;
	pSize->cy = 0;

	CPoint Point(0,0);
	CRect RetRect(0,0,0,0);

	// get the current selected layer:
	CLayer *pLayer = static_cast<CLayer*>((*m_ppMainDrawable)->GetChild(m_nLayer));

	_SpriteSet *CopyBoard = (_SpriteSet*)pBuffer;
	// is it a sprite set?
	if(strncmp(CopyBoard->Info.Header.ID, OLF_SPRITE_SET_ID, sizeof(OLF_SPRITE_SET_ID)-1)) {
		RetRect = PasteFile(pLayer, (LPCSTR)pBuffer, &Point, false);
		if(RetRect.IsRectNull()) {
			RetRect = PasteSprite(pLayer, (LPCSTR)pBuffer, &Point, false);
			if(RetRect.IsRectNull()) return false;
		}
	} else {
		if(CopyBoard->Info.Header.dwSignature != OLF_SPRITE_SET_SIGNATURE) {
			CONSOLE_PRINTF("Paste error : Attempt to paste an invalid Quest Designer Sprite Set!\n");
			return false;
		}
		RetRect = PasteSpriteSet(pLayer, (LPBYTE)pBuffer, &Point, false);
		if(RetRect.IsRectNull()) return false;
	}
	pSize->cx = RetRect.Width();
	pSize->cy = RetRect.Height();
	return true;
}
bool CSpriteSelection::GetPastedSize(CSprite *pSprite, SIZE *pSize)
{
	CSize Size;

	if(pSprite->GetSpriteType() == tMask) {
		pSize->cx = 0;
		pSize->cy = 0;
		return false;
	}

	pSprite->GetSize(Size);
	*pSize = Size;
	return true;
}

bool CSpriteSelection::FastPaste(CSprite *pSprite, const CPoint &point_ )
{
	CRect RetRect(0,0,0,0);

	// get the current selected layer:
	CLayer *pLayer = static_cast<CLayer*>((*m_ppMainDrawable)->GetChild(m_nLayer));
	RetRect = PasteSprite(pLayer, pSprite, &point_);
	return (RetRect.IsRectNull() == false);
}

bool CSpriteSelection::FastPaste(LPCVOID pBuffer, const CPoint &point_ )
{
	CRect RetRect(0,0,0,0);

	// get the current selected layer:
	CLayer *pLayer = static_cast<CLayer*>((*m_ppMainDrawable)->GetChild(m_nLayer));

	_SpriteSet *CopyBoard = (_SpriteSet*)pBuffer;
	// is it a sprite set?
	if(strncmp(CopyBoard->Info.Header.ID, OLF_SPRITE_SET_ID, sizeof(OLF_SPRITE_SET_ID)-1)) {
		RetRect = PasteFile(pLayer, (LPCSTR)pBuffer, &point_);
		if(RetRect.IsRectNull()) {
			RetRect = PasteSprite(pLayer, (LPCSTR)pBuffer, &point_);
			if(RetRect.IsRectNull()) return false;
		}
	} else {
		if(CopyBoard->Info.Header.dwSignature != OLF_SPRITE_SET_SIGNATURE) {
			CONSOLE_PRINTF("Paste error : Attempt to paste an invalid Quest Designer Sprite Set!\n");
			return false;
		}
		RetRect = PasteSpriteSet(pLayer, (LPBYTE)pBuffer, &point_);
		if(RetRect.IsRectNull()) return false;
	}

	return true;
}

bool CSpriteSelection::Paste(LPCVOID pBuffer, const CPoint &point_)
{
	ASSERT(pBuffer);
	if(m_bHoldSelection) {
		CONSOLE_PRINTF("Warning: Can not paste on held selection, release the selection first.\n");
		return false;
	}

	CRect RetRect(0,0,0,0);
	// get the current selected layer:
	CLayer *pLayer = static_cast<CLayer*>((*m_ppMainDrawable)->GetChild(m_nLayer));

	_SpriteSet *CopyBoard = (_SpriteSet*)pBuffer;
	// is it a sprite set?
	if(strncmp(CopyBoard->Info.Header.ID, OLF_SPRITE_SET_ID, sizeof(OLF_SPRITE_SET_ID)-1)) {
		RetRect = PasteFile(pLayer, (LPCSTR)pBuffer);
		if(RetRect.IsRectNull()) {
			RetRect = PasteSprite(pLayer, (LPCSTR)pBuffer);
			if(RetRect.IsRectNull()) return false;
		}
	} else {
		if(CopyBoard->Info.Header.dwSignature != OLF_SPRITE_SET_SIGNATURE) {
			CONSOLE_PRINTF("Paste error : Attempt to paste an invalid Quest Designer Sprite Set!\n");
			return false;
		}
		RetRect = PasteSpriteSet(pLayer, (LPBYTE)pBuffer);
		if(RetRect.IsRectNull()) return false;
	}

	if(m_Groups[m_nPasteGroup].O.empty()) {
		EndMoving(point_);
		if(m_nPasteGroup) CONSOLE_PRINTF("Paste error : Attempt to paste an empty Sprite!\n");
		else CONSOLE_PRINTF("Paste error : Attempt to paste an empty Quest Designer Sprite Set!\n");
		return false;
	}

	MoveTo(point_);

	CONSOLE_DEBUG("%d of %d objects selected.\n", m_Groups[m_nPasteGroup].O.size(), (*m_ppMainDrawable)->Objects());
	return true;
}

void CSpriteSelection::SelectionToGroup(LPCSTR szGroupName)
{
	// NEED TO FIX *** (if the group already exists, no new group is to be created??)
	// if(m_nCurrentGroup) return; // <-- ^^^

	if(m_bHoldSelection) return;
	// no new groups can be created from an empty selection or a single sprite selection.
	if(m_Groups[0].O.size() <= 1) return;

	// get the next availible free paste group to paste group into:
	m_nCurrentGroup = SetNextPasteGroup(szGroupName);
	CONSOLE_DEBUG("New group #%d created\n", m_nCurrentGroup);

	ASSERT(m_nCurrentGroup > 0);

	// go through all selected sprites adding the orphans 
	// to the new group, and setting relationships:
	vectorObject::iterator Iterator;
	for(Iterator = m_Groups[0].O.begin(); Iterator != m_Groups[0].O.end(); Iterator++) {
		if(Iterator->pContext) {
			ASSERT(Iterator->nGroup == 0); // sprites at this level must not belong to any group.
			m_Groups[m_nCurrentGroup].O.push_back(*Iterator);
			m_Groups[m_nCurrentGroup].O.back().nGroup = m_nCurrentGroup;
			Iterator->nGroup = m_nCurrentGroup;
		} else {
			ASSERT(m_Groups[Iterator->nGroup].P == 0); // groups at this level must not belong to any group.
			m_Groups[Iterator->nGroup].P = m_nCurrentGroup;
			m_Groups[m_nCurrentGroup].O.push_back(*Iterator);
			CONSOLE_DEBUG("  Subgroup #%d ('%s') inserted\n", Iterator->nGroup, (LPCSTR)m_Groups[Iterator->nGroup].Name.c_str());
		}
	}
	m_Groups[0].O.clear();

	// Select the newly created group:
	CRect rcRect;
	GetBoundingRect(&rcRect, m_nCurrentGroup);
	m_Groups[0].O.push_back(SObjProp(this, NULL, m_nCurrentGroup, rcRect));

	// SelectGroup(m_nCurrentGroup); // <-- the current group should already be selected (it hasn't been unselected)
	Touch();
}
// converts the currently selected group to a simple selection:
void CSpriteSelection::GroupToSelection()
{
	if(m_bHoldSelection) return;
	if(m_nCurrentGroup < 1) return;

	vectorObject::iterator Iterator;
	int nParent = m_Groups[m_nCurrentGroup].P;

	// Move the sprites from the current group to its parent:
	for(Iterator = m_Groups[m_nCurrentGroup].O.begin(); Iterator != m_Groups[m_nCurrentGroup].O.end(); Iterator++) {
		if(Iterator->pContext) Iterator->nGroup = nParent;
		m_Groups[nParent].O.push_back(*Iterator);
	}

	// Delete the group:
	m_Groups[m_nCurrentGroup].P = 0;
	m_Groups[m_nCurrentGroup].O.clear();
	m_Groups[m_nCurrentGroup].Name.clear();

	m_nCurrentGroup = nParent;
	Touch();
}
void CSpriteSelection::SelectionToTop()
{
	// get the current selected layer:
	CLayer *pLayer = static_cast<CLayer*>((*m_ppMainDrawable)->GetChild(m_nLayer));

	// we start by sorting the selected objects (in the selection):
	SortSelection();
	int nNextOrder = pLayer->ReOrder(1);

	vectorObject::iterator Iterator;
	for(Iterator = m_Groups[0].O.begin(); Iterator != m_Groups[0].O.end(); Iterator++) {
		if(Iterator->pContext == NULL) continue; // NEED TO FIX *** add group stuff
		if(m_bHoldSelection && Iterator->bSubselected || !m_bHoldSelection) {
			Iterator->pContext->SetObjOrder(nNextOrder++);
		}
	}
	Touch();
}
void CSpriteSelection::SelectionToBottom()
{
	// get the current selected layer:
	CLayer *pLayer = static_cast<CLayer*>((*m_ppMainDrawable)->GetChild(m_nLayer));

	// we start by sorting the selected objects (in the selection):
	SortSelection();
	int nNextOrder = pLayer->ReOrder(1, 0, m_Groups[0].O.size());

	vectorObject::iterator Iterator;
	for(Iterator = m_Groups[0].O.begin(); Iterator != m_Groups[0].O.end(); Iterator++) {
		if(Iterator->pContext == NULL) continue; // NEED TO FIX *** add group stuff
		if(m_bHoldSelection && Iterator->bSubselected || !m_bHoldSelection) {
			Iterator->pContext->SetObjOrder(nNextOrder++);
		}
	}
	Touch();
}
void CSpriteSelection::SelectionDown()
{
	// get the current selected layer:
	CLayer *pLayer = static_cast<CLayer*>((*m_ppMainDrawable)->GetChild(m_nLayer));

	// we start by sorting the selected objects (in the selection):
	SortSelection();
	int nNextOrder = pLayer->ReOrder(2); // reorder everything leaving a space between objects.

	vectorObject::iterator Iterator;
	for(Iterator = m_Groups[0].O.begin(); Iterator != m_Groups[0].O.end(); Iterator++) {
		if(Iterator->pContext == NULL) continue; // NEED TO FIX *** add group stuff
		if(m_bHoldSelection && Iterator->bSubselected || !m_bHoldSelection) {
			int nNextOrder = Iterator->pContext->GetObjOrder() - 3;
			if(nNextOrder < 0) nNextOrder = 0;
			Iterator->pContext->SetObjOrder(nNextOrder);
		}
	}
	Touch();
}
void CSpriteSelection::SelectionUp()
{
	// get the current selected layer:
	CLayer *pLayer = static_cast<CLayer*>((*m_ppMainDrawable)->GetChild(m_nLayer));

	// we start by sorting the selected objects (in the selection):
	SortSelection();
	int nNextOrder = pLayer->ReOrder(2); // reorder everything leaving a space between objects.

	vectorObject::iterator Iterator;
	for(Iterator = m_Groups[0].O.begin(); Iterator != m_Groups[0].O.end(); Iterator++) {
		if(Iterator->pContext == NULL) continue; // NEED TO FIX *** add group stuff
		if(m_bHoldSelection && Iterator->bSubselected || !m_bHoldSelection) {
			int nNextOrder = Iterator->pContext->GetObjOrder() + 3;
			Iterator->pContext->SetObjOrder(nNextOrder);
		}
	}
	Touch();
}

void CSpriteSelection::FlipSelection()
{
	CRect rcBoundaries, RectTmp, Rect;
	GetBoundingRect(&rcBoundaries);

	vectorObject::iterator Iterator;
	for(Iterator = m_Groups[0].O.begin(); Iterator != m_Groups[0].O.end(); Iterator++) {
		if(Iterator->pContext == NULL) continue; // NEED TO FIX *** add group stuff
		CSpriteContext *scontext = static_cast<CSpriteContext*>(Iterator->pContext);
		scontext->GetAbsFinalRect(RectTmp);

		if(Iterator->eYChain == up) Iterator->eYChain = down;
		else if(Iterator->eYChain == down) Iterator->eYChain = up;

		// no rotations nor transormations are allowed for entities 
		// (that should be handled in the scripts instead)
		if(static_cast<CSprite*>(scontext->GetDrawableObj())->GetSpriteType() != tEntity) {
			if(scontext->isMirrored() && !scontext->isFlipped()) {
				scontext->Mirror(false);
				scontext->Rotate((scontext->Rotation()-2)&(SROTATE_0|SROTATE_90|SROTATE_180|SROTATE_270));
			} else {
				scontext->Flip();
			}
		}
		Rect.top = rcBoundaries.top + (rcBoundaries.bottom - RectTmp.bottom);
		Rect.bottom = rcBoundaries.bottom - (RectTmp.top - rcBoundaries.top);
		Rect.left = RectTmp.left;
		Rect.right = RectTmp.right;

		Rect.NormalizeRect();

		scontext->SetAbsFinalRect(Rect);
		if(m_eCurrentState==eMoving) {
			Iterator->rcRect = Rect; // for floating selections
		}
	}

	if(m_eCurrentState==eMoving) {
		GetBoundingRect(&m_rcSelection);
		SetInitialMovingPoint(m_rcSelection.CenterPoint());
	}

	Touch();
}
void CSpriteSelection::MirrorSelection()
{
	CRect rcBoundaries, RectTmp, Rect;
	GetBoundingRect(&rcBoundaries);

	vectorObject::iterator Iterator;
	for(Iterator = m_Groups[0].O.begin(); Iterator != m_Groups[0].O.end(); Iterator++) {
		if(Iterator->pContext == NULL) continue; // NEED TO FIX *** add group stuff
		CSpriteContext *scontext = static_cast<CSpriteContext*>(Iterator->pContext);
		scontext->GetAbsFinalRect(RectTmp);

		if(Iterator->eXChain == left) Iterator->eXChain = right;
		else if(Iterator->eXChain == right) Iterator->eXChain = left;

		// no rotations nor transormations are allowed for entities 
		// (that should be handled in the scripts instead)
		if(static_cast<CSprite*>(scontext->GetDrawableObj())->GetSpriteType() != tEntity) {
			if(!scontext->isMirrored() && scontext->isFlipped()) {
				scontext->Flip(false);
				scontext->Rotate((scontext->Rotation()-2)&(SROTATE_0|SROTATE_90|SROTATE_180|SROTATE_270));
			} else {
				scontext->Mirror();
			}
		}
		Rect.top = RectTmp.top;
		Rect.bottom = RectTmp.bottom;
		Rect.left = rcBoundaries.left + (rcBoundaries.right - RectTmp.right);
		Rect.right = rcBoundaries.right - (RectTmp.left - rcBoundaries.left);

		Rect.NormalizeRect();

		scontext->SetAbsFinalRect(Rect);
		if(m_eCurrentState==eMoving) {
			Iterator->rcRect = Rect; // for floating selections
		}
	}

	if(m_eCurrentState==eMoving) {
		GetBoundingRect(&m_rcSelection);
		SetInitialMovingPoint(m_rcSelection.CenterPoint());
	}

	Touch();
}
void CSpriteSelection::CWRotateSelection()
{
	CRect rcBoundaries, RectTmp, Rect;
	GetBoundingRect(&rcBoundaries);

	vectorObject::iterator Iterator;
	for(Iterator = m_Groups[0].O.begin(); Iterator != m_Groups[0].O.end(); Iterator++) {
		if(Iterator->pContext == NULL) continue; // NEED TO FIX *** add group stuff

		CSpriteContext *scontext = static_cast<CSpriteContext*>(Iterator->pContext);
		scontext->GetAbsFinalRect(RectTmp);

		_Chain cTmp = Iterator->eXChain;
		Iterator->eXChain = Iterator->eYChain;
		if(cTmp == left) Iterator->eYChain = up;
		else if(cTmp == right) Iterator->eYChain = down;
		else Iterator->eYChain = cTmp;

		// no rotations nor transormations are allowed for entities 
		// (that should be handled in the scripts instead)
		if(static_cast<CSprite*>(scontext->GetDrawableObj())->GetSpriteType() != tEntity) {
			if(scontext->isMirrored() && scontext->isFlipped()) {
				scontext->Flip(false);
				scontext->Mirror(false);
				scontext->Rotate((scontext->Rotation()+1)&(SROTATE_0|SROTATE_90|SROTATE_180|SROTATE_270));
			} else {
				if(scontext->isMirrored() || scontext->isFlipped()) {
					scontext->Flip();
					scontext->Mirror();
				}
				scontext->Rotate((scontext->Rotation()-1)&(SROTATE_0|SROTATE_90|SROTATE_180|SROTATE_270));
			}
			Rect.top = rcBoundaries.top + (RectTmp.left - rcBoundaries.left);
			Rect.bottom = rcBoundaries.top + (RectTmp.right - rcBoundaries.left);
			Rect.left = rcBoundaries.left + (rcBoundaries.bottom - RectTmp.bottom);
			Rect.right = rcBoundaries.left + (rcBoundaries.bottom - RectTmp.top);
		} else {
			Rect.top = rcBoundaries.top + (RectTmp.left - rcBoundaries.left);
			Rect.bottom = Rect.top + RectTmp.Height();
			Rect.left = rcBoundaries.left + (rcBoundaries.bottom - RectTmp.bottom);
			Rect.right = Rect.left + RectTmp.Width();
		}

		Rect.NormalizeRect();

		scontext->SetAbsFinalRect(Rect);
		if(m_eCurrentState==eMoving) {
			Iterator->rcRect = Rect; // for floating selections
		}
	}

	if(m_eCurrentState==eMoving) {
		GetBoundingRect(&m_rcSelection);
		SetInitialMovingPoint(m_rcSelection.CenterPoint());
	}

	Touch();
}
void CSpriteSelection::CCWRotateSelection()
{
	CRect rcBoundaries, RectTmp, Rect;
	GetBoundingRect(&rcBoundaries);

	vectorObject::iterator Iterator;
	for(Iterator = m_Groups[0].O.begin(); Iterator != m_Groups[0].O.end(); Iterator++) {
		if(Iterator->pContext == NULL) continue; // NEED TO FIX *** add group stuff

		CSpriteContext *scontext = static_cast<CSpriteContext*>(Iterator->pContext);
		scontext->GetAbsFinalRect(RectTmp);

		_Chain cTmp = Iterator->eYChain;
		Iterator->eYChain = Iterator->eXChain;
		if(cTmp == up) Iterator->eXChain = left;
		else if(cTmp == down) Iterator->eXChain = right;
		else Iterator->eYChain = cTmp;

		// no rotations nor transormations are allowed for entities 
		// (that should be handled in the scripts instead)
		if(static_cast<CSprite*>(scontext->GetDrawableObj())->GetSpriteType() != tEntity) {
			if(scontext->isMirrored() && scontext->isFlipped()) {
				scontext->Flip(false);
				scontext->Mirror(false);
				scontext->Rotate((scontext->Rotation()-1)&(SROTATE_0|SROTATE_90|SROTATE_180|SROTATE_270));
			} else {
				if(scontext->isMirrored() || scontext->isFlipped()) {
					scontext->Flip();
					scontext->Mirror();
				}
				scontext->Rotate((scontext->Rotation()+1)&(SROTATE_0|SROTATE_90|SROTATE_180|SROTATE_270));
			}
			Rect.top = rcBoundaries.top + (rcBoundaries.right - RectTmp.right);
			Rect.bottom = rcBoundaries.top + (rcBoundaries.right - RectTmp.left);
			Rect.left = rcBoundaries.left + (RectTmp.top - rcBoundaries.top);
			Rect.right = rcBoundaries.left + (RectTmp.bottom - rcBoundaries.top);
		} else {
			Rect.top = rcBoundaries.top + (rcBoundaries.right - RectTmp.right);
			Rect.bottom = Rect.top + RectTmp.Height();
			Rect.left = rcBoundaries.left + (RectTmp.top - rcBoundaries.top);
			Rect.right = Rect.left + RectTmp.Width();
		}

		Rect.NormalizeRect();

		scontext->SetAbsFinalRect(Rect);
		if(m_eCurrentState==eMoving) {
			Iterator->rcRect = Rect; // for floating selections
		}
	}

	if(m_eCurrentState==eMoving) {
		GetBoundingRect(&m_rcSelection);
		SetInitialMovingPoint(m_rcSelection.CenterPoint());
	}

	Touch();
}
