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

CDrawableContext::CDrawableContext() : 
	m_pIGraphics(NULL), 
	m_pDrawableObj(NULL), 
	m_pParent(NULL), 
	m_dwStatus(0), 
	m_eDrawType(topView),
	m_nOrder(0),
	m_nSubLayer(0),
	m_Position(0,0),
	m_Size(0,0),
	m_nChildren(0),
	m_pBuffer(NULL) 
{
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

void CDrawableContext::AddChild(CDrawableContext *object) 
{ 
	object->m_pParent = this;
	object->m_nOrder = m_nChildren++;
	m_Children.push_back(object); 
}

inline void CDrawableContext::SetSize(const SIZE &_size) 
{
	m_Size = _size;
}
void CDrawableContext::SetSize(int x, int y) 
{ 
	m_Size.SetSize(x,y); 
}
inline void CDrawableContext::GetSize(CSize &_Size) const 
{ 
	_Size = m_Size; 
}

void CDrawableContext::SetSubLayer(int layer) 
{ 
	m_nSubLayer = layer; 
}

void CDrawableContext::MoveTo(int x, int y) 
{ 
	m_Position.SetPoint(x,y); 
}
inline void CDrawableContext::MoveTo(const POINT &_point) 
{
	m_Position = _point;
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
	if(m_Size.cx==0 || m_Size.cy==0)
		if(m_pDrawableObj) m_pDrawableObj->GetSize(m_Size);
	_Rect.SetRect(m_Position, m_Position + m_Size);
}

void CDrawableContext::GetAbsFinalRect(CRect &_Rect) const 
{
	GetAbsRect(_Rect);
	if((m_dwStatus&DROTATE) == DROTATE) {
		int w = _Rect.Width();
		int h = _Rect.Height();
		_Rect.bottom = _Rect.top+w;
		_Rect.right = _Rect.left+h;;
	}
}

void CDrawableContext::GetAbsRect(CRect &_Rect) const 
{
	CPoint Position;
	GetAbsPosition(Position);
	if(m_Size.cx==0 || m_Size.cy==0)
		if(m_pDrawableObj) m_pDrawableObj->GetSize(m_Size);
	_Rect.SetRect(Position, Position + m_Size);
}

inline void CDrawableContext::SetRect(const RECT &_rect) 
{
	m_Position.SetPoint(_rect.left, _rect.top);
	m_Size.SetSize(_rect.right-_rect.left, _rect.bottom-_rect.top);
}

inline bool CDrawableContext::isAt(int x, int y) const 
{ 
	return isAt(CPoint(x, y)); 
}
inline bool CDrawableContext::isAt(const POINT &_point) const 
{ 
	CRect Rect;
	GetAbsFinalRect(Rect);
	if(Rect.IsRectNull()) return false;
	return ( _point.x>=Rect.left && _point.x<Rect.right &&
			 _point.y>=Rect.top  && _point.y<Rect.bottom );

}
inline bool CDrawableContext::isAt(const RECT &_rect) const 
{
	CRect Rect;
	GetAbsFinalRect(Rect);
	if(Rect.IsRectNull()) return false;
	return (Rect.IntersectRect(Rect, &_rect)==TRUE);
}

inline bool CDrawableContext::isIn(const RECT &_rect) const 
{
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

inline void CDrawableContext::ShowSprite(bool bShow) 
{
	if(bShow)	m_dwStatus |= (DVISIBLE<<_DRW_SHFT);
	else		m_dwStatus &= ~(DVISIBLE<<_DRW_SHFT);
}
inline bool CDrawableContext::isVisible() const 
{ 
	return ((m_dwStatus&(DVISIBLE<<_DRW_SHFT))==(DVISIBLE<<_DRW_SHFT)); 
}

bool CDrawableContext::Draw(IGraphics *pIGraphics) 
{
	if(pIGraphics) m_pIGraphics = pIGraphics;

	// Order the sprite list to draw
	if(m_eDrawType == topView) {
		ContextOrderCompare cmp;
		sort(m_Children.begin(), m_Children.end(), cmp);
	} else if(m_eDrawType == leftIso) {
		ContextZYXCompare cmp;
		sort(m_Children.begin(), m_Children.end(), cmp);
	} else if(m_eDrawType == rightIso) {
		ContextZYiXCompare cmp;
		sort(m_Children.begin(), m_Children.end(), cmp);
	}

	// Draw propagation
	vector<CDrawableContext *>::iterator Iterator;
	for(Iterator = m_Children.begin(); Iterator != m_Children.end(); Iterator++) {
		(*Iterator)->Draw(m_pIGraphics);
	}
	if(m_pDrawableObj) {
		if(m_pDrawableObj->NeedToDraw(*this))
			return m_pDrawableObj->Draw(*this);
	}
	return true;
}
bool CDrawableContext::GetFirstChildAt(const POINT &point_, CDrawableContext **ppDrawableContext_)
{
	// Order the sprite list to draw
	if(m_eDrawType == topView) {
		ContextOrderCompare cmp;
		sort(m_Children.rbegin(), m_Children.rend(), cmp);
	} else if(m_eDrawType == leftIso) {
		ContextZYXCompare cmp;
		sort(m_Children.rbegin(), m_Children.rend(), cmp);
	} else if(m_eDrawType == rightIso) {
		ContextZYiXCompare cmp;
		sort(m_Children.rbegin(), m_Children.rend(), cmp);
	}

	CDrawableContext *pToRet = NULL;
	m_ChildIterator = m_Children.begin();
	while(m_ChildIterator!= m_Children.end()) {
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
bool CDrawableContext::GetNextChildAt(const POINT &point_, CDrawableContext **ppDrawableContext_)
{
	CDrawableContext *pToRet = NULL;
	while(m_ChildIterator!= m_Children.end()) {
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
	// Order the sprite list to draw
	if(m_eDrawType == topView) {
		ContextOrderCompare cmp;
		sort(m_Children.rbegin(), m_Children.rend(), cmp);
	} else if(m_eDrawType == leftIso) {
		ContextZYXCompare cmp;
		sort(m_Children.rbegin(), m_Children.rend(), cmp);
	} else if(m_eDrawType == rightIso) {
		ContextZYiXCompare cmp;
		sort(m_Children.rbegin(), m_Children.rend(), cmp);
	}

	CDrawableContext *pToRet = NULL;
	m_ChildIterator = m_Children.begin();
	while(m_ChildIterator!= m_Children.end()) {
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
	while(m_ChildIterator!= m_Children.end()) {
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


CDrawableSelection::CDrawableSelection(CDrawableContext **ppDrawableContext_) {
	m_ppMainDrawable = ppDrawableContext_;
	ASSERT(m_ppMainDrawable);
}
void CDrawableSelection::StartSelection(const POINT &point_) {
	m_rcSelection.left = point_.x;
	m_rcSelection.top = point_.y;
	m_rcSelection.right = point_.x;
	m_rcSelection.bottom = point_.y;
	m_bSelecting = true;
}
void CDrawableSelection::CancelSelection() {
	m_bSelecting = false;
}
void CDrawableSelection::DragSelection(const POINT &point_) {
	if(!m_bSelecting) return;
	m_rcSelection.right = point_.x;
	m_rcSelection.bottom = point_.y;
}

#define SELSENS_LINE		4
#define SELSENS_VERTEX		20

void CDrawableSelection::EndSelectionAdd(const POINT &point_) {
	if(!m_bSelecting) return;
	m_rcSelection.right = point_.x;
	m_rcSelection.bottom = point_.y;
	m_bSelecting = false;

	m_rcSelection.NormalizeRect();
	if(m_rcSelection.Width()<2 && m_rcSelection.Height()<2) 
		PointSelectionAdd(m_rcSelection.TopLeft());

	CDrawableContext *pDrawableContext = NULL;
	(*m_ppMainDrawable)->GetFirstChildIn(m_rcSelection, &pDrawableContext);
	while(pDrawableContext) {
		if(m_Objects.Find(pDrawableContext) == -1) m_Objects.Add(pDrawableContext);
		pDrawableContext = NULL;
		(*m_ppMainDrawable)->GetNextChildIn(m_rcSelection, &pDrawableContext);
	}
}
void CDrawableSelection::EndSelectionRemove(const POINT &point_) {
	if(!m_bSelecting) return;
	m_rcSelection.right = point_.x;
	m_rcSelection.bottom = point_.y;
	m_bSelecting = false;

	m_rcSelection.NormalizeRect();
	if(m_rcSelection.Width()<2 && m_rcSelection.Height()<2) 
		PointSelectionRemove(m_rcSelection.TopLeft());

	CDrawableContext *pDrawableContext = NULL;
	(*m_ppMainDrawable)->GetFirstChildIn(m_rcSelection, &pDrawableContext);
	while(pDrawableContext) {
		m_Objects.Remove(pDrawableContext);
		pDrawableContext = NULL;
		(*m_ppMainDrawable)->GetNextChildIn(m_rcSelection, &pDrawableContext);
	}
}
void CDrawableSelection::PointSelectionAdd(const POINT &point_) {
	CDrawableContext *pDrawableContext = NULL;
	(*m_ppMainDrawable)->GetFirstChildAt(point_, &pDrawableContext);
	while(pDrawableContext) {
		if(m_Objects.Find(pDrawableContext) == -1) {
			m_Objects.Add(pDrawableContext);
			return;
		}
		pDrawableContext = NULL;
		(*m_ppMainDrawable)->GetNextChildAt(point_, &pDrawableContext);
	}
}
void CDrawableSelection::PointSelectionRemove(const POINT &point_) {
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
void CDrawableSelection::Draw(const IGraphics *pGraphics_) {
	CRect Rect(0,0,0,0);
	CRect RectTmp;

	int nObjects = m_Objects.GetSize();
	for(int i=0; i<nObjects; i++) {
		m_Objects[i]->GetAbsFinalRect(RectTmp);
		Rect.UnionRect(Rect, RectTmp);
		if(nObjects>1) {
			pGraphics_->DrawRect(RectTmp,150,255,255,225,2);
			pGraphics_->BoundingBox(RectTmp, 200, 0, 0, 0);
		}
	}
	if(nObjects>1) {
		pGraphics_->SelectionBox(Rect, 255, 255, 255, 200);
	} else if(nObjects==1) {
		pGraphics_->SelectionBox(Rect, 200, 255, 255, 200);
	}
	if(m_bSelecting) {
		pGraphics_->BoundingBox(m_rcSelection, 128, 0, 0, 0);
	}

}
CURSOR CDrawableSelection::GetMouseStateAt(const IGraphics *pGraphics_, const POINT &point_)
{
	CURSOR retCursor = eIDC_ARROW;
	bool left, right, top, bottom;
	left = right = top = bottom = false;

	CPoint WorldPoint = point_;
	// We get the world coordinates for the mouse position
	pGraphics_->GetWorldPosition(WorldPoint);

	CRect Rect(0,0,0,0);
	CRect RectTmp;

	// For each selected object, we check to see if the mouse is over it, also we build a bounding Rect:
	int nObjects = m_Objects.GetSize();
	for(int i=0; i<nObjects; i++) {
		m_Objects[i]->GetAbsFinalRect(RectTmp);
		if(m_Objects[i]->isAt(WorldPoint)) retCursor = eIDC_SIZEALL; // The cursor is ovet the object.
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

	if(point_.x >= Rect.left-SELSENS_LINE && point_.x < Rect.left+sens_inX) left = true;
	if(point_.x >= Rect.right-sens_inX && point_.x < Rect.right+SELSENS_LINE) right = true;
	if(point_.y >= Rect.top-SELSENS_LINE && point_.y < Rect.top+sens_inY) top = true;
	if(point_.y >= Rect.bottom-sens_inY && point_.y < Rect.bottom+SELSENS_LINE) bottom = true;

	// Are we over a line of the bounding box?
	if(left || right || top || bottom) {
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
		if(point_.x >= RectXMiddle-sens_cornerX/2 && point_.x < RectXMiddle+sens_cornerX/2) retCursor = eIDC_SIZENS;
		if(point_.y >= RectYMiddle-sens_cornerY/2 && point_.y < RectYMiddle+sens_cornerY/2) retCursor = eIDC_SIZEWE;

		// Are we in any corner?
		if(top || bottom) {
			if( point_.x >= Rect.left-SELSENS_LINE && point_.x < Rect.left+sens_cornerX ) 
				retCursor = top?eIDC_SIZENWSE:eIDC_SIZENESW;
			if( point_.x > Rect.right-sens_cornerX && point_.x < Rect.right+SELSENS_LINE) 
				retCursor = top?eIDC_SIZENESW:eIDC_SIZENWSE;
		}
		if(left || right) {
			if( point_.y >= Rect.top-SELSENS_LINE && point_.y < Rect.top+sens_cornerY ) 
				retCursor = left?eIDC_SIZENWSE:eIDC_SIZENESW;
			if( point_.y > Rect.bottom-sens_cornerY && point_.y < Rect.bottom+SELSENS_LINE) 
				retCursor = left?eIDC_SIZENESW:eIDC_SIZENWSE;
		}
	}

	return retCursor;
}
