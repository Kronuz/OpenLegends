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
/*! \file		WorldManager.cpp
	\brief		Implementation of the CProjectManager class.
	\date		April 28, 2003

	This file implements the classes that manage a world in the game,
	this includes the methods to create a new world, maps for that
	world, and being living on it.

*/

#include "stdafx.h"
#include "DrawManager.h"

// This functions are used to sort the sprite list befor rendering to the screen.
bool CDrawableContext::ContextSubLayerCompare::operator()(CDrawableContext* a, CDrawableContext* b) {
	if(a->m_nSubLayer < b->m_nSubLayer) return true;
	if(a->m_nSubLayer >= b->m_nSubLayer) return false;
	return false;
}
bool CDrawableContext::ContextYCompare::operator()(CDrawableContext* a, CDrawableContext* b) {
	CSize SizeA, SizeB;
	a->GetSize(SizeA);
	b->GetSize(SizeB);

	if(a->m_Position.y+SizeA.cy < b->m_Position.y+SizeB.cy) return true;
	if(a->m_Position.y+SizeA.cy >= b->m_Position.y+SizeB.cy) return false;
	if(a->m_nOrder < b->m_nOrder) return true;
	//if(a->m_nOrder >= b->m_nOrder) return false;

	return false;
}
bool CDrawableContext::ContextYXCompare::operator()(CDrawableContext* a, CDrawableContext* b) {
	CSize SizeA, SizeB;
	a->GetSize(SizeA);
	b->GetSize(SizeB);

	if(a->m_Position.y+SizeA.cy < b->m_Position.y+SizeB.cy) return true;
	if(a->m_Position.y+SizeA.cy >= b->m_Position.y+SizeB.cy) return false;
	if(a->m_Position.x+SizeA.cx < b->m_Position.x+SizeB.cx) return true;
	//if(a->m_Position.x+SizeA.cx >= b->m_Position.x+SizeB.cx) return false;

	return false;
}
bool CDrawableContext::ContextYiXCompare::operator()(CDrawableContext* a, CDrawableContext* b) {
	CSize SizeA, SizeB;
	a->GetSize(SizeA);
	b->GetSize(SizeB);

	if(a->m_Position.y+SizeA.cy < b->m_Position.y+SizeB.cy) return true;
	if(a->m_Position.y+SizeA.cy >= b->m_Position.y+SizeB.cy) return false;
	if(a->m_Position.x >= b->m_Position.x) return true;
	//if(a->m_Position.x < b->m_Position.x) return false;

	return false;
}
bool CDrawableContext::ContextOrderCompare::operator()(CDrawableContext* a, CDrawableContext* b) {

	if(a->m_nOrder < b->m_nOrder) return true;
	//if(a->m_nOrder >= b->m_nOrder) return false;

	return false;
}

CDrawableContext::CDrawableContext() : 
	m_pIGraphics(NULL), 
	m_pDrawableObj(NULL), 
	m_pParent(NULL), 
	m_dwStatus(0), 
	m_nOrder(0),
	m_nSubLayer(-1),
	m_Position(0,0),
	m_Size(-1,-1),
	m_nChildren(0),
	m_bValidMap(false),
	m_pBuffer(NULL) 
{
	for(int i=0; i<MAX_SUBLAYERS; i++) {
		// The default ordering is birthOrder, birthOrder, yOrder, birthOrder, birthOrder, yOrder... (one and one)
		m_eDrawType[i] = (((i+1)%3)==0)?yOrder:birthOrder;
		m_eSorted[i] = noOrder;
	}
}
CDrawableContext::~CDrawableContext() 
{
	if(m_pBuffer) {
		m_pBuffer->Invalidate();
		m_pBuffer->Release();
	}
	vector<CDrawableContext *>::iterator Iterator;
	for(Iterator = m_Children.begin(); Iterator != m_Children.end(); Iterator++) {
		delete (*Iterator);
	}
	m_Children.clear();
};

bool CDrawableContext::AddSibling(CDrawableContext *object) 
{
	if(m_pParent == NULL) return false;
	return m_pParent->AddChild(object);
}
bool CDrawableContext::AddChild(CDrawableContext *object) 
{ 
	CRect newRect, lstRect;
	object->GetRect(newRect);
	vector<CDrawableContext*>::iterator Iterator = m_Children.begin();
	while(Iterator!=m_Children.end()) {
		if( object->m_pDrawableObj == (*Iterator)->m_pDrawableObj &&
			object->m_nSubLayer == (*Iterator)->m_nSubLayer ) {
			(*Iterator)->GetRect(lstRect);
			if(lstRect == newRect) return false; // Skip duplicates
		}
		Iterator++;
	}

	object->m_pParent = this;
	object->m_nOrder = m_nChildren++;
	m_Children.push_back(object);
	m_bValidMap = false;
	m_eSorted[object->m_nSubLayer] = noOrder;
	return true;
}

inline void CDrawableContext::SetSize(const SIZE &_size) 
{
	m_Size = _size;
	m_pBuffer->Invalidate();
}
void CDrawableContext::SetSize(int x, int y) 
{ 
	m_Size.SetSize(x,y); 
}

inline void CDrawableContext::GetSize(CSize &_Size) const 
{ 
	if(m_Size.cx==-1 && m_Size.cy==-1)
		if(m_pDrawableObj) m_pDrawableObj->GetSize(m_Size);
	_Size = m_Size; 
}

void CDrawableContext::SetSubLayer(int layer) 
{ 
	m_nSubLayer = layer; 
	if(m_pParent) {
		m_bValidMap = false;
		m_pParent->m_eSorted[m_nSubLayer] = noOrder;
	}
}

void CDrawableContext::MoveTo(int x, int y) 
{ 
	m_Position.SetPoint(x,y); 
	if(m_pParent) m_pParent->m_eSorted[m_nSubLayer] = noOrder;
}
void CDrawableContext::MoveTo(const CPoint &_point) 
{
	m_Position = _point;
	if(m_pParent) m_pParent->m_eSorted[m_nSubLayer] = noOrder;
}

void CDrawableContext::GetPosition(CPoint &_Point) const 
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
void CDrawableContext::GetAbsRect(CRect &_Rect) const 
{
	CPoint Position;
	GetAbsPosition(Position);
	if(m_Size.cx==-1 && m_Size.cy==-1)
		if(m_pDrawableObj) m_pDrawableObj->GetSize(m_Size);
	_Rect.SetRect(Position, Position + m_Size);
}
void CDrawableContext::GetAbsFinalRect(CRect &_Rect) const 
{
	GetAbsRect(_Rect);
	if(isRotated()) {
		int w = _Rect.Width();
		int h = _Rect.Height();
		_Rect.bottom = _Rect.top+w;
		_Rect.right = _Rect.left+h;;
	}
}

inline void CDrawableContext::SetRect(const RECT &_rect) 
{
	m_Position.SetPoint(_rect.left, _rect.top);
	m_Size.SetSize(_rect.right-_rect.left, _rect.bottom-_rect.top);

	if(m_pParent) m_pParent->m_eSorted[m_nSubLayer] = noOrder;
	m_pBuffer->Invalidate();
}
void CDrawableContext::SetAbsRect(const CRect &_Rect)
{
	CPoint Position(0,0);
	if(m_pParent) m_pParent->GetAbsPosition(Position);
	m_Position = _Rect.TopLeft() - Position;
	m_Size.SetSize(_Rect.Width(), _Rect.Height());

	if(m_pParent) m_pParent->m_eSorted[m_nSubLayer] = noOrder;
	m_pBuffer->Invalidate();
}
void CDrawableContext::SetAbsFinalRect(const CRect &_Rect)
{
	CPoint Position(0,0);
	if(m_pParent) m_pParent->GetAbsPosition(Position);
	m_Position = _Rect.TopLeft() - Position;
	if(isRotated()) m_Size.SetSize(_Rect.Height(), _Rect.Width());
	else m_Size.SetSize(_Rect.Width(), _Rect.Height());

	if(m_pParent) m_pParent->m_eSorted[m_nSubLayer] = noOrder;
	m_pBuffer->Invalidate();
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

IGraphics* CDrawableContext::GetGraphicsDevice() const 
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
inline void CDrawableContext::ShowSprite(bool bShow) 
{
	if(bShow)	m_dwStatus |= (DVISIBLE<<_DRW_SHFT);
	else		m_dwStatus &= ~(DVISIBLE<<_DRW_SHFT);
}
inline bool CDrawableContext::isVisible() const 
{ 
	return ((m_dwStatus&(DVISIBLE<<_DRW_SHFT))==(DVISIBLE<<_DRW_SHFT)); 
}
// Sublayer or -1 to draw all layers
// We build a layers map to speed things up a little:
void CDrawableContext::PreSort() 
{
	// First, we sort all the elements by their sublayer:
	ContextSubLayerCompare cmpSubLayer;
	stable_sort(m_Children.begin(), m_Children.end(), cmpSubLayer);

	{ // Now we generate the map of children
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
	}

	{ // Now we generate the reverse map of children
		vector<CDrawableContext *>::reverse_iterator Iterator;
		Iterator = m_Children.rbegin();
		for(int i=MAX_SUBLAYERS; i>=0; i--) {
			if(Iterator != m_Children.rend()) {
				if((*Iterator)->m_nSubLayer == i) break;
			}
			m_LayersRMap[i+1] = Iterator;

		}
		for(; i>=0 && Iterator!=m_Children.rend(); i--) {
			m_LayersRMap[i+1] = Iterator;
			while(Iterator != m_Children.rend()) {
				if((*Iterator)->m_nSubLayer != i) break;
				Iterator++;
			}
		}
		for(; i>=0; i--)  m_LayersRMap[i+1] = Iterator;
		while(Iterator != m_Children.rend()) Iterator++;
		m_LayersRMap[0] = Iterator;
	}

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

bool CDrawableContext::Draw(IGraphics *pIGraphics) 
{
	for(int i=0; i<MAX_SUBLAYERS; i++) {
		Draw(i, pIGraphics);
	}
	return true;
}

bool CDrawableContext::Draw(int nSubLayer, IGraphics *pIGraphics) 
{
	if(pIGraphics) m_pIGraphics = pIGraphics;

	if(!m_bValidMap) PreSort();
	if(m_eSorted[nSubLayer] != m_eDrawType[nSubLayer]) Sort(nSubLayer);

	vector<CDrawableContext *>::iterator Iterator;
	for(Iterator = m_LayersMap[0]; Iterator!=m_LayersMap[1]; Iterator++) {
		(*Iterator)->Draw(nSubLayer, m_pIGraphics);
	}
	for(Iterator = m_LayersMap[nSubLayer+1];  Iterator!=m_LayersMap[nSubLayer+2]; Iterator++) {
		(*Iterator)->Draw(nSubLayer, m_pIGraphics);
	}

	if(m_pDrawableObj && nSubLayer==m_nSubLayer) {
		if(m_pDrawableObj->NeedToDraw(*this))
			return m_pDrawableObj->Draw(*this);
	}
	return true;
}
bool CDrawableContext::GetFirstChildAt(const CPoint &point_, CDrawableContext **ppDrawableContext_)
{
	CDrawableContext *pToRet = NULL;
	m_ChildIterator = m_Children.rbegin();
	while(m_ChildIterator!= m_Children.rend()) {
		m_LastChildIteratorUsed = m_ChildIterator;
		if( (*m_ChildIterator)->GetFirstChildAt(point_, &pToRet) ) m_ChildIterator++;
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
	while(m_ChildIterator!= m_Children.rend()) {
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


void CDrawableContext::SetDrawableObj(CDrawableObject *pDrawableObj) 
{ 
	m_pDrawableObj = pDrawableObj; 
}
inline CDrawableObject* CDrawableContext::GetDrawableObj() 
{ 
	return m_pDrawableObj; 
}

CDrawableSelection::CDrawableSelection(CDrawableContext **ppDrawableContext_) :
	m_CurrentCursor(eIDC_ARROW),
	m_nSnapSize(1)
{
	m_ppMainDrawable = ppDrawableContext_;
	ASSERT(m_ppMainDrawable);
}
void CDrawableSelection::SetSnapSize(int nSnapSize_)
{
	m_nSnapSize = nSnapSize_;
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
int CDrawableSelection::GetBoundingRect(CRect &Rect_) 
{
	CRect RectTmp;
	int nObjects = m_Objects.GetSize();
	Rect_.SetRectEmpty();
	for(int i=0; i<nObjects; i++) {
		m_Objects.GetKeyAt(i)->GetAbsFinalRect(RectTmp);
		Rect_.UnionRect(Rect_, RectTmp);
	}
	return nObjects;
}
void CDrawableSelection::StartResizing(const CPoint &point_)
{
	m_eCurrentState = eResizing;
	BuildRealSelectionBounds();
	m_ptInitialPoint = point_;
}
void CDrawableSelection::ResizeTo(const CPoint &point_)
{
	if(m_eCurrentState!=eResizing) return;

	CRect rcNewBoundaries;
	rcNewBoundaries = m_rcSelection;

	CPoint PointTmp = m_ptInitialPoint - point_;
	if(PointTmp.x == 0 && PointTmp.y == 0) return;

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

	if(rcNewBoundaries.Height()<m_nSnapSize/*rcNewBoundaries.top>=m_rcSelection.bottom*/ && m_bCursorTop) {
		int h = m_rcSelection.Height()%m_nSnapSize; if(h==0) h = m_nSnapSize;
		rcNewBoundaries.bottom = m_rcSelection.bottom;
		rcNewBoundaries.top = m_rcSelection.bottom - h;
	} else if(rcNewBoundaries.Height()<m_nSnapSize/*rcNewBoundaries.bottom<=m_rcSelection.top*/ && m_bCursorBottom) {
		int h = m_rcSelection.Height()%m_nSnapSize; if(h==0) h = m_nSnapSize;
		rcNewBoundaries.top = m_rcSelection.top;
		rcNewBoundaries.bottom = m_rcSelection.top + h;
	}
	if(rcNewBoundaries.Width()<m_nSnapSize/*rcNewBoundaries.left>=m_rcSelection.right*/ && m_bCursorLeft) {
		int w = m_rcSelection.Width()%m_nSnapSize; if(w==0) w = m_nSnapSize;
		rcNewBoundaries.right = m_rcSelection.right;
		rcNewBoundaries.left = m_rcSelection.right - w;
	} else if(rcNewBoundaries.Width()<m_nSnapSize/*rcNewBoundaries.right<=m_rcSelection.left*/ && m_bCursorRight) {
		int w = m_rcSelection.Width()%m_nSnapSize; if(w==0) w = m_nSnapSize;
		rcNewBoundaries.left = m_rcSelection.left;
		rcNewBoundaries.right = m_rcSelection.left + w;
	}
	rcNewBoundaries.NormalizeRect();

	int nObjects = m_Objects.GetSize();
	for(int i=0; i<nObjects; i++) {
		ResizeObject(m_Objects.GetKeyAt(i), m_Objects.GetValueAt(i), m_rcSelection, rcNewBoundaries, true);
	}
}
void CDrawableSelection::EndResizing(const CPoint &point_)
{
	if(m_eCurrentState!=eResizing) return;
	m_eCurrentState = eNone;
}
void CDrawableSelection::StartMoving(const CPoint &point_)
{
	m_eCurrentState = eMoving;
	BuildRealSelectionBounds();
	m_ptInitialPoint = point_;
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

	int nObjects = m_Objects.GetSize();
	for(int i=0; i<nObjects; i++) {
		ResizeObject(m_Objects.GetKeyAt(i), m_Objects.GetValueAt(i), m_rcSelection, rcNewBoundaries, false);
	}
}

void CDrawableSelection::EndMoving(const CPoint &point_)
{
	if(m_eCurrentState!=eMoving) return;
	m_eCurrentState = eNone;
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

#define SELSENS_LINE		4
#define SELSENS_VERTEX		20

void CDrawableSelection::EndSelBoxAdd(const CPoint &point_) {
	if(m_eCurrentState!=eSelecting) return;
	m_rcSelection.right = point_.x;
	m_rcSelection.bottom = point_.y;
	m_eCurrentState = eNone;

	m_rcSelection.NormalizeRect();
	if(m_rcSelection.Width()<=1 && m_rcSelection.Height()<=1) 
		SelPointAdd(m_rcSelection.TopLeft());

	CDrawableContext *pDrawableContext = NULL;
	(*m_ppMainDrawable)->GetFirstChildIn(m_rcSelection, &pDrawableContext);
	while(pDrawableContext) {
		if(m_Objects.FindKey(pDrawableContext) == -1) {
			CRect Rect;
			pDrawableContext->GetRect(Rect);
			m_Objects.Add(pDrawableContext, Rect);
		}
		pDrawableContext = NULL;
		(*m_ppMainDrawable)->GetNextChildIn(m_rcSelection, &pDrawableContext);
	}
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
		m_Objects.Remove(pDrawableContext);
		pDrawableContext = NULL;
		(*m_ppMainDrawable)->GetNextChildIn(m_rcSelection, &pDrawableContext);
	}
}
void CDrawableSelection::SelPointAdd(const CPoint &point_) {
	CDrawableContext *pDrawableContext = NULL;
	(*m_ppMainDrawable)->GetFirstChildAt(point_, &pDrawableContext);
	while(pDrawableContext) {
		if(m_Objects.FindKey(pDrawableContext) == -1) {
			CRect Rect;
			pDrawableContext->GetRect(Rect);
			m_Objects.Add(pDrawableContext, Rect);
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
		if(m_Objects.Remove(pDrawableContext)) return;
		pDrawableContext = NULL;
		(*m_ppMainDrawable)->GetNextChildAt(point_, &pDrawableContext);
	}
}
void CDrawableSelection::CleanSelection() {
	m_Objects.RemoveAll();
}
CURSOR CDrawableSelection::GetMouseStateAt(const IGraphics *pGraphics_, const CPoint &point_)
{
	if( m_eCurrentState==eResizing	||
		m_eCurrentState==eMoving	|| 
		m_eCurrentState==eSelecting	   ) return m_CurrentCursor;

	m_CurrentCursor = eIDC_ARROW;
	m_bCursorLeft = m_bCursorTop = m_bCursorRight = m_bCursorBottom = false;

	CPoint WorldPoint = point_;
	// We get the world coordinates for the mouse position
	pGraphics_->GetWorldPosition(WorldPoint);

	CRect Rect(0,0,0,0);
	CRect RectTmp;

	// For each selected object, we check to see if the mouse is over it, also we build a bounding Rect:
	int nObjects = m_Objects.GetSize();
	for(int i=0; i<nObjects; i++) {
		(m_Objects.GetKeyAt(i))->GetAbsFinalRect(RectTmp);
		if((m_Objects.GetKeyAt(i))->isAt(WorldPoint) && m_bCanMove) m_CurrentCursor = eIDC_SIZEALL; // The cursor is ovet the object.
		Rect.UnionRect(Rect, RectTmp);	// Bounding Rect
	}

	// Now that we have the full bounding Rect (in world coordinates) we convert it to View coordinates.
	pGraphics_->GetViewRect(Rect);
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
	return m_CurrentCursor;
}
