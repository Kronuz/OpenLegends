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
/*! \file		DrawManager.cpp
	\brief		Implementation of the Drawing classes.
	\date		June 15, 2003

	This file implements the classes to handle drawing objects and selections.
*/

#include "stdafx.h"
#include "DrawManager.h"
#include "SpriteManager.h"

// This functions are used to sort the sprite list before rendering to the screen.
inline bool CDrawableContext::ContextSubLayerCompare::operator()(const CDrawableContext *a, const CDrawableContext *b) const
{
	if(a->m_nSubLayer < b->m_nSubLayer) return true;
	if(a->m_nSubLayer > b->m_nSubLayer) return false;
	return false;
}
inline bool CDrawableContext::ContextYCompare::operator()(const CDrawableContext *a, const CDrawableContext *b) const
{
	CSize SizeA, SizeB;
	a->GetSize(SizeA);
	b->GetSize(SizeB);

	if(a->m_nSubLayer < b->m_nSubLayer) return true;
	if(a->m_nSubLayer > b->m_nSubLayer) return false;

	if(a->m_Position.y+SizeA.cy < b->m_Position.y+SizeB.cy) return true;
	if(a->m_Position.y+SizeA.cy >= b->m_Position.y+SizeB.cy) return false;
	if(a->m_nOrder < b->m_nOrder) return true;
	//if(a->m_nOrder >= b->m_nOrder) return false;

	return false;
}
inline bool CDrawableContext::ContextYXCompare::operator()(const CDrawableContext *a, const CDrawableContext *b) const
{
	CSize SizeA, SizeB;
	a->GetSize(SizeA);
	b->GetSize(SizeB);

	if(a->m_nSubLayer < b->m_nSubLayer) return true;
	if(a->m_nSubLayer > b->m_nSubLayer) return false;

	if(a->m_Position.y+SizeA.cy < b->m_Position.y+SizeB.cy) return true;
	if(a->m_Position.y+SizeA.cy >= b->m_Position.y+SizeB.cy) return false;
	if(a->m_Position.x+SizeA.cx < b->m_Position.x+SizeB.cx) return true;
	//if(a->m_Position.x+SizeA.cx >= b->m_Position.x+SizeB.cx) return false;

	return false;
}
inline bool CDrawableContext::ContextYiXCompare::operator()(const CDrawableContext *a, const CDrawableContext *b) const
{
	CSize SizeA, SizeB;
	a->GetSize(SizeA);
	b->GetSize(SizeB);

	if(a->m_nSubLayer < b->m_nSubLayer) return true;
	if(a->m_nSubLayer > b->m_nSubLayer) return false;

	if(a->m_Position.y+SizeA.cy < b->m_Position.y+SizeB.cy) return true;
	if(a->m_Position.y+SizeA.cy >= b->m_Position.y+SizeB.cy) return false;
	if(a->m_Position.x >= b->m_Position.x) return true;
	//if(a->m_Position.x < b->m_Position.x) return false;

	return false;
}
inline bool CDrawableContext::ContextOrderCompare::operator()(const CDrawableContext *a, const CDrawableContext *b) const
{
	if(a->m_nSubLayer < b->m_nSubLayer) return true;
	if(a->m_nSubLayer > b->m_nSubLayer) return false;

	if(a->m_nOrder < b->m_nOrder) return true;
	//if(a->m_nOrder >= b->m_nOrder) return false;

	return false;
}

CDrawableContext::CDrawableContext(LPCSTR szName) : 
	CNamedObj(szName),
	m_pIGraphics(NULL), 
	m_pDrawableObj(NULL), 
	m_pParent(NULL), 
	m_dwStatus((DVISIBLE<<_DRW_SHFT)), // DrawContexts are initially visibles.
	m_bSelected(false),
	m_nOrder(0),
	m_nSubLayer(-1), // DrawContexts are initially on sublayer -1 (not drew)
	m_Position(0,0),
	m_Size(-1,-1),
	m_nChildren(0),
	m_bValidMap(false)
{
	memset(m_pBuffer, 0, sizeof(m_pBuffer));
	for(int i=0; i<MAX_SUBLAYERS; i++) {
		// The default ordering is birthOrder, birthOrder, yOrder, birthOrder, birthOrder, yOrder... (one and one)
		m_eDrawType[i] = (((i+1)%3)==0)?yOrder:birthOrder;
		m_eSorted[i] = noOrder;
	}
}
CDrawableContext::~CDrawableContext() 
{
	for(int i=0; i<CONTEXT_BUFFERS; i++) {
		if(m_pBuffer[i]) {
			m_pBuffer[i]->Invalidate();
			m_pBuffer[i]->Release();
		}
	}

	// Optimizated using for_each() instead of a loop
	for_each(m_Children.begin(), m_Children.end(), ptr_delete());
	m_Children.clear();
};

bool CDrawableContext::AddSibling(CDrawableContext *object, bool bAllowDups_) 
{
	if(m_pParent == NULL) return false;
	return m_pParent->AddChild(object, bAllowDups_);
}
bool CDrawableContext::AddChild(CDrawableContext *object, bool bAllowDups_) 
{ 
	ASSERT(object);

	if(!bAllowDups_) {
		vector<CDrawableContext*>::iterator Iterator = 
			find_if(
				m_Children.begin(), m_Children.end(), 
				bind2nd(ptr_equal_to<CDrawableContext*>(), object));
		if(Iterator != m_Children.end()) {
			(*Iterator)->m_nOrder = m_nChildren++;	// renew the order, 
			return false;							// but skip duplicates.
		}
	}
	object->m_pParent = this;
	object->m_nOrder = m_nChildren++;
	m_Children.push_back(object);
	m_bValidMap = false;
	m_eSorted[object->m_nSubLayer] = noOrder;
	return true;
}
// Sublayer or -1 to draw all layers
// We build a layers map to speed things up a little:
void CDrawableContext::PreSort() 
{
	// First, we sort all the elements by their sublayer:
	ContextSubLayerCompare cmpSubLayer;
	stable_sort(m_Children.begin(), m_Children.end(), cmpSubLayer);

	// Generate the map of children
	vector<CDrawableContext *>::iterator Iterator;
	m_LayersMap[0] = Iterator = m_Children.begin();
	while(Iterator != m_Children.end()) {
		if((*Iterator)->m_nSubLayer != -1) break;
		Iterator++;
	}
	for(int i=0; i<MAX_SUBLAYERS && Iterator!=m_Children.end(); i++) {
		m_LayersMap[i+1] = Iterator;
		while(Iterator != m_Children.end()) {
			if((*Iterator)->m_nSubLayer != i) break;
			Iterator++;
		}
	}
	for(; i<=MAX_SUBLAYERS; i++) m_LayersMap[i+1] = Iterator;

	m_bValidMap = true;
}
void CDrawableContext::Sort(int nSubLayer) 
{
	ContextYCompare cmpY;
	ContextYXCompare cmpYX;
	ContextYiXCompare cmpYiX;
	ContextOrderCompare cmpOrder;
	switch(m_eDrawType[nSubLayer]) {
		case birthOrder: sort(m_LayersMap[nSubLayer+1], m_LayersMap[nSubLayer+2], cmpOrder); break;
		case yOrder: sort(m_LayersMap[nSubLayer+1], m_LayersMap[nSubLayer+2], cmpY); break;
		case rightIso: sort(m_LayersMap[nSubLayer+1], m_LayersMap[nSubLayer+2], cmpYX); break;
		case leftIso: sort(m_LayersMap[nSubLayer+1], m_LayersMap[nSubLayer+2], cmpYiX); break;
	}
	m_eSorted[nSubLayer] = m_eDrawType[nSubLayer];
}

inline bool CDrawableContext::DrawContext::operator()(CDrawableContext *pDrawableContext, const IGraphics *pIGraphics) const
{
	ASSERT(pDrawableContext);

	if(pIGraphics) pDrawableContext->m_pIGraphics = pIGraphics;

	if(!pDrawableContext->m_bValidMap) pDrawableContext->PreSort();
	for(int nSubLayer=0; nSubLayer<MAX_SUBLAYERS; nSubLayer++) {
		if(pDrawableContext->m_eSorted[nSubLayer] != pDrawableContext->m_eDrawType[nSubLayer]) 
			pDrawableContext->Sort(nSubLayer);
	}

	// Start drawing objects from sublayer 0 (at position [1] in the layers map):
	for_each(
		pDrawableContext->m_LayersMap[1], pDrawableContext->m_Children.end(), 
		bind2nd(DrawContext(m_bVisible, m_bSelected), pIGraphics));

	if(pDrawableContext->m_pDrawableObj) {
		if( (pDrawableContext->m_pDrawableObj->NeedToDraw(*pDrawableContext)) && 
			(pDrawableContext->isVisible()&&m_bVisible || pDrawableContext->isSelected()&&m_bSelected) )
			return pDrawableContext->m_pDrawableObj->Draw(*pDrawableContext);
	}
	return true;
}

bool CDrawableContext::Draw(const IGraphics *pIGraphics) 
{
	DrawContext Draw(true, false);
	return Draw(this, pIGraphics);
}

bool CDrawableContext::DrawSelected(const IGraphics *pIGraphics) 
{
	DrawContext Draw(false, true);
	return Draw(this, pIGraphics);
}

// counts the number of drawable contexts with a drawable object assigned to it.
int CDrawableContext::Objects(int init)
{
	vector<CDrawableContext*>::iterator Iterator = m_Children.begin();
	while(Iterator != m_Children.end()) {
		init = (*Iterator)->Objects(init);
		Iterator++;
	}

	if(m_pDrawableObj) init++;
	return init;
}
bool CDrawableContext::KillChild(CDrawableContext *pDrawableContext_)
{
	ASSERT(pDrawableContext_);
	// Search for the requested child and if found, kill it (its own childs get killed too)
	vector<CDrawableContext*>::iterator Iterator =
		find(m_Children.begin(), m_Children.end(), pDrawableContext_);
	if(Iterator != m_Children.end()) {
		m_bValidMap = false;
		m_eSorted[(*Iterator)->m_nSubLayer] = noOrder;
		delete (*Iterator);
		m_Children.erase(Iterator);
		return true;
	}

	// Propagate de search for the child:
	Iterator = m_Children.begin();
	while(Iterator != m_Children.end()) {
		if((*Iterator)->KillChild(pDrawableContext_)) {
			return true;
		}
		Iterator++;
	}
	return false;
}

bool CDrawableContext::GetFirstChildAt(const CPoint &point_, CDrawableContext **ppDrawableContext_)
{
	CDrawableContext *pToRet = NULL;
	m_ChildIterator = m_Children.rbegin();
	while(m_ChildIterator != m_Children.rend()) {
		m_LastChildIteratorUsed = m_ChildIterator;
		if((*m_ChildIterator)->GetFirstChildAt(point_, &pToRet) ) m_ChildIterator++;
		if(pToRet) {
			*ppDrawableContext_ = pToRet;
			return false;
		}
	}
	if(isAt(point_)) {
		*ppDrawableContext_ = this;
	}
	return true;
}
bool CDrawableContext::GetNextChildAt(const CPoint &point_, CDrawableContext **ppDrawableContext_)
{
	CDrawableContext *pToRet = NULL;
	while(m_ChildIterator != m_Children.rend()) {
		if(m_ChildIterator != m_LastChildIteratorUsed) {
			m_LastChildIteratorUsed = m_ChildIterator;
			if( (*m_ChildIterator)->GetFirstChildAt(point_, &pToRet) ) m_ChildIterator++;
			if(pToRet) {
				*ppDrawableContext_ = pToRet;
				return false;
			}
		} else {
			if( (*m_ChildIterator)->GetNextChildAt(point_, &pToRet) ) m_ChildIterator++;
			if(pToRet) {
				*ppDrawableContext_ = pToRet;
				return false;
			}
		}
	}
	if(isAt(point_)) {
		*ppDrawableContext_ = this;
	}
	return true;
}

bool CDrawableContext::GetFirstChildIn(const RECT &rect_, CDrawableContext **ppDrawableContext_)
{
	CDrawableContext *pToRet = NULL;
	m_ChildIterator = m_Children.rbegin();
	while(m_ChildIterator!= m_Children.rend()) {
		m_LastChildIteratorUsed = m_ChildIterator;
		if( (*m_ChildIterator)->GetFirstChildIn(rect_, &pToRet) ) m_ChildIterator++;
		if(pToRet) {
			*ppDrawableContext_ = pToRet;
			return false;
		}
	}
	if(isIn(rect_)) {
		*ppDrawableContext_ = this;
	}
	return true;
}
bool CDrawableContext::GetNextChildIn(const RECT &rect_, CDrawableContext **ppDrawableContext_)
{
	CDrawableContext *pToRet = NULL;
	while(m_ChildIterator!= m_Children.rend()) {
		if(m_ChildIterator != m_LastChildIteratorUsed) {
			m_LastChildIteratorUsed = m_ChildIterator;
			if( (*m_ChildIterator)->GetFirstChildIn(rect_, &pToRet) ) m_ChildIterator++;
			if(pToRet) {
				*ppDrawableContext_ = pToRet;
				return false;
			}
		} else {
			if( (*m_ChildIterator)->GetNextChildIn(rect_, &pToRet) ) m_ChildIterator++;
			if(pToRet) {
				*ppDrawableContext_ = pToRet;
				return false;
			}
		}
	}
	if(isIn(rect_)) {
		*ppDrawableContext_ = this;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Mouse sensivility for selections:
#define SELSENS_LINE		4	
#define SELSENS_VERTEX		20

CDrawableSelection::CDrawableSelection(CDrawableContext **ppDrawableContext_) :
	m_CurrentCursor(eIDC_ARROW),
	m_nSnapSize(1),
	m_rcSelection(0,0,0,0),
	m_ptInitialPoint(0,0),
	m_bFloating(false),
	m_bChanged(true),
	m_pBitmap(NULL)
{
	m_ppMainDrawable = ppDrawableContext_;
	ASSERT(m_ppMainDrawable);
}
int CDrawableSelection::GetBoundingRect(CRect &Rect_) 
{
	int nObjects = 0;
	CRect RectTmp;
	Rect_.SetRect(0,0,0,0);
	map<CDrawableContext*, SObjProp>::iterator Iterator;
	for(Iterator = m_Objects.begin(); Iterator != m_Objects.end(); Iterator++) {
		Iterator->first->GetAbsFinalRect(RectTmp);
		Rect_.UnionRect(Rect_, RectTmp);
		nObjects++;
	}
	return nObjects;
}

void CDrawableSelection::SelPointAdd(const CPoint &point_, int Chains) {
	CDrawableContext *pDrawableContext = NULL;
	(*m_ppMainDrawable)->GetFirstChildAt(point_, &pDrawableContext);
	while(pDrawableContext) {
		map<CDrawableContext*, SObjProp>::iterator Iterator;
		Iterator = m_Objects.find(pDrawableContext);
		if(Iterator==m_Objects.end()) {
			CRect Rect;
			pDrawableContext->GetRect(Rect);
			pDrawableContext->SelectContext();
			m_Objects.insert(pairObject(pDrawableContext, SObjProp(Rect, relative, relative)));
			pDrawableContext->SelectContext();
			CONSOLE_DEBUG("%d of %d objects selected.\n", m_Objects.size(), (*m_ppMainDrawable)->Objects());
			CONSOLE_DEBUG("The selected object is on layer %d, and sublayer: %d.\n", pDrawableContext->GetObjLayer(), pDrawableContext->GetObjSubLayer());
			return;
		}
		if(Chains>0) {
			if(m_Objects.size()<=1) return; // ignore if there's only one object
			if(Iterator->second.eXChain<4) Iterator->second.eXChain = (_Chain)((int)Iterator->second.eXChain+1);
			else {
				Iterator->second.eXChain = (_Chain)0;
				if(Iterator->second.eYChain<4) Iterator->second.eYChain = (_Chain)((int)Iterator->second.eYChain+1);
				else Iterator->second.eYChain = (_Chain)0;
			}
			CONSOLE_DEBUG("XChain: %d, YChain: %d\n", (int)Iterator->second.eXChain, (int)Iterator->second.eYChain);
			return;
		} else if(Chains<0) {
			if(m_Objects.size()<=1) return; // ignore if there's only one object
			if(Iterator->second.eXChain>0) Iterator->second.eXChain = (_Chain)((int)Iterator->second.eXChain-1);
			else {
				Iterator->second.eXChain = (_Chain)4;
				if(Iterator->second.eYChain>0) Iterator->second.eYChain = (_Chain)((int)Iterator->second.eYChain-1);
				else Iterator->second.eYChain = (_Chain)4;
			}
			CONSOLE_DEBUG("XChain: %d, YChain: %d\n", (int)Iterator->second.eXChain, (int)Iterator->second.eYChain);
			return;
		}
		pDrawableContext = NULL;
		(*m_ppMainDrawable)->GetNextChildAt(point_, &pDrawableContext);
	}
}
void CDrawableSelection::SelPointRemove(const CPoint &point_) {
	CDrawableContext *pDrawableContext = NULL;
	(*m_ppMainDrawable)->GetFirstChildAt(point_, &pDrawableContext);
	while(pDrawableContext) {
		pDrawableContext->SelectContext(false);
		if(m_Objects.erase(pDrawableContext) != 0) return;
		pDrawableContext = NULL;
		(*m_ppMainDrawable)->GetNextChildAt(point_, &pDrawableContext);
	}
}

// Interface:
void CDrawableSelection::SetSnapSize(int nSnapSize_)
{
	m_nSnapSize = nSnapSize_;
}
int CDrawableSelection::Count()
{
	return m_Objects.size();
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

	map<CDrawableContext*, SObjProp>::const_iterator Iterator;
	for(Iterator = m_Objects.begin(); Iterator != m_Objects.end(); Iterator++) {
		ResizeObject(Iterator->first, Iterator->second, m_rcSelection, rcNewBoundaries, true);
	}
}
void CDrawableSelection::EndResizing(const CPoint &point_)
{
	if(m_eCurrentState!=eResizing) return;
	ResizeTo(point_);
	m_eCurrentState = eNone;
	if(m_ptInitialPoint != point_) m_bChanged = true;
}
void CDrawableSelection::StartMoving(const CPoint &point_)
{
	m_eCurrentState = eMoving;
	BuildRealSelectionBounds();
	m_ptInitialPoint = point_;
}
void CDrawableSelection::DeleteSelection()
{
	map<CDrawableContext*, SObjProp>::iterator Iterator = m_Objects.begin();
	while(Iterator != m_Objects.end()) {
		Iterator->first->SelectContext(false);
		(*m_ppMainDrawable)->KillChild(Iterator->first);
		Iterator++;
	}

	m_Objects.clear();
	m_bFloating = false;
	m_bChanged = true;
	CONSOLE_DEBUG("%d objects left.\n", (*m_ppMainDrawable)->Objects());
}
void CDrawableSelection::Cancel()
{
	if(m_bFloating) {
		DeleteSelection();
		m_eCurrentState = eNone;
	} else if(m_eCurrentState==eMoving) {
		MoveTo(m_ptInitialPoint);
		EndMoving(m_ptInitialPoint);
	} else if(m_eCurrentState==eResizing) {
		ResizeTo(m_ptInitialPoint);
		EndResizing(m_ptInitialPoint);
	} else {
		map<CDrawableContext*, SObjProp>::iterator Iterator;
		for(Iterator = m_Objects.begin(); Iterator != m_Objects.end(); Iterator++) {
			Iterator->first->SelectContext(false);
		}
		m_Objects.clear();
	}
}
void CDrawableSelection::MoveTo(const CPoint &Point_)
{
	if(m_eCurrentState!=eMoving) return;

	CRect rcOldBoundaries, rcNewBoundaries;
	GetBoundingRect(rcOldBoundaries);
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

	map<CDrawableContext*, SObjProp>::const_iterator Iterator;
	for(Iterator = m_Objects.begin(); Iterator != m_Objects.end(); Iterator++) {
		ResizeObject(Iterator->first, Iterator->second, m_rcSelection, rcNewBoundaries, true);
	}
}
void CDrawableSelection::EndMoving(const CPoint &point_)
{
	if(m_eCurrentState!=eMoving) return;
	MoveTo(point_);
	m_eCurrentState = eNone;
	m_bFloating = false;
	if(m_ptInitialPoint != point_) m_bChanged = true;
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
bool CDrawableSelection::SelectedAt(const CPoint &point_)
{
	map<CDrawableContext*, SObjProp>::const_iterator Iterator = m_Objects.begin();
	while(Iterator != m_Objects.end()) {
		if(Iterator->first->isAt(point_)) return true;
		Iterator++;
	}
	return false;
}
void CDrawableSelection::EndSelBoxAdd(const CPoint &point_, int Chains) {
	if(m_eCurrentState!=eSelecting) return;
	m_rcSelection.right = point_.x;
	m_rcSelection.bottom = point_.y;
	m_eCurrentState = eNone;

	m_rcSelection.NormalizeRect();
	if(m_rcSelection.Width()<=1 && m_rcSelection.Height()<=1) {
		SelPointAdd(m_rcSelection.TopLeft(), Chains);
		return;
	}

	CDrawableContext *pDrawableContext = NULL;
	(*m_ppMainDrawable)->GetFirstChildIn(m_rcSelection, &pDrawableContext);
	while(pDrawableContext) {
		map<CDrawableContext*, SObjProp>::const_iterator Iterator;
		Iterator = m_Objects.find(pDrawableContext);
		if(Iterator==m_Objects.end()) {
			CRect Rect;
			pDrawableContext->GetRect(Rect);
			pDrawableContext->SelectContext();
			m_Objects.insert(pairObject(pDrawableContext, SObjProp(Rect, relative, relative)));
		}
		pDrawableContext = NULL;
		(*m_ppMainDrawable)->GetNextChildIn(m_rcSelection, &pDrawableContext);
	}
	CONSOLE_DEBUG("%d of %d objects selected.\n", m_Objects.size(), (*m_ppMainDrawable)->Objects());
}
void CDrawableSelection::EndSelBoxRemove(const CPoint &point_) {
	if(m_eCurrentState!=eSelecting) return;
	m_rcSelection.right = point_.x;
	m_rcSelection.bottom = point_.y;
	m_eCurrentState = eNone;

	m_rcSelection.NormalizeRect();
	if(m_rcSelection.Width()<=1 && m_rcSelection.Height()<=1) 
		SelPointRemove(m_rcSelection.TopLeft());

	CDrawableContext *pDrawableContext = NULL;
	(*m_ppMainDrawable)->GetFirstChildIn(m_rcSelection, &pDrawableContext);
	while(pDrawableContext) {
		pDrawableContext->SelectContext(false);
		m_Objects.erase(pDrawableContext);
		pDrawableContext = NULL;
		(*m_ppMainDrawable)->GetNextChildIn(m_rcSelection, &pDrawableContext);
	}
}
void CDrawableSelection::CleanSelection() {
	map<CDrawableContext*, SObjProp>::iterator Iterator;
	for(Iterator = m_Objects.begin(); Iterator != m_Objects.end(); Iterator++) {
		Iterator->first->SelectContext(false);
	}
	m_Objects.clear();
}
bool CDrawableSelection::GetMouseStateAt(const IGraphics *pGraphics_, const CPoint &point_, CURSOR *pCursor)
{
	CPoint WorldPoint = point_;
	// We get the world coordinates for the mouse position
	pGraphics_->GetWorldPosition(&WorldPoint);

	CRect rcBoundaries;
	GetBoundingRect(rcBoundaries);
	bool ret = rcBoundaries.PtInRect(WorldPoint);

	*pCursor = m_CurrentCursor;

	if( m_eCurrentState==eResizing	||
		m_eCurrentState==eMoving	|| 
		m_eCurrentState==eSelecting	   ) return ret;

	m_CurrentCursor = eIDC_ARROW;
	m_bCursorLeft = m_bCursorTop = m_bCursorRight = m_bCursorBottom = false;

	CRect Rect(0,0,0,0);
	CRect RectTmp;

	// For each selected object, we check to see if the mouse is over it, also we build a bounding Rect:
	map<CDrawableContext*, SObjProp>::const_iterator Iterator;
	for(Iterator = m_Objects.begin(); Iterator != m_Objects.end(); Iterator++) {
		Iterator->first->GetAbsFinalRect(RectTmp);
		if(Iterator->first->isAt(WorldPoint) && m_bCanMove) m_CurrentCursor = eIDC_SIZEALL; // The cursor is ovet the object.
		Rect.UnionRect(Rect, RectTmp);	// Bounding Rect
	}

	// Now that we have the full bounding Rect (in world coordinates) we convert it to View coordinates.
	pGraphics_->GetViewRect(&Rect);
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
	*pCursor = m_CurrentCursor;
	return ret;
}

bool CDrawableSelection::Paint(IGraphics *pGraphicsI, WORD wFlags)
{
	ASSERT(m_ppMainDrawable);
	if(!*m_ppMainDrawable) return false;

	bool bRet = true;
	CSprite::SetShowOptions(wFlags);
	pGraphicsI->SetClearColor(COLOR_ARGB(255,0,0,0));
	if(pGraphicsI->BeginPaint()) {
		bRet &= (*m_ppMainDrawable)->Draw(pGraphicsI);
		bRet &= pGraphicsI->DrawFrame();
		bRet &= Draw(pGraphicsI);
		bRet &= pGraphicsI->EndPaint();
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

	CSprite::SetShowOptions(0);
	pGraphicsI->SetClearColor(COLOR_ARGB(255,0,0,0));
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
	GetBoundingRect(RectFull);
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
	// Allocate enough memory for the complete 16 bits bitmapQuest Designer Sprite Set
	BITMAP *pBitmap = (BITMAP*)new BYTE[sizeof(BITMAP) + RectFull.Height() * bmWidthBytes];

	ZeroMemory(pBitmap, sizeof(BITMAP));
	pBitmap->bmBits = (BYTE*)pBitmap + sizeof(BITMAP);
	pBitmap->bmWidth = RectFull.Width();
	pBitmap->bmHeight = RectFull.Height();
	pBitmap->bmWidthBytes = bmWidthBytes;
	pBitmap->bmBitsPixel = sizeof(WORD)*8;

	CSprite::SetShowOptions(0);
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
