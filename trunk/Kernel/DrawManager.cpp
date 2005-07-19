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
/////////////////////////////////////////////////////////////////////////////
/*! \file		DrawManager.cpp
	\author		Germán Méndez Bravo (Kronuz)
	\brief		Implementation of the Drawing classes.
	\date		June 15, 2003:
					* Creation date.
				May 29, 2005
					- Fix: The [i] issue that prevented compilation has been corrected.
					- Bug: New bugs were created. Groups are not yet functional.
				July 15, 2005:
						* Added CMutable Touch() calls
				July 16, 2005:
						- Many undo/redo fixes (improved robustness)
				July 19, 2005:
						* Moved DrawableSelection to the file SelectionManager.(cpp|h)

	\remarks	This file implements the classes to handle drawing objects and selections.
				Known bugs:
				This has a major bug while working with groups. need to be fixed
*/

#include "stdafx.h"
#include "DrawManager.h"

#include <IGame.h>


// This functions are used to sort the contexs list before rendering to the screen.
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
	if(a->m_Position.x+SizeA.cx < b->m_Position.x+(SizeB.cx/2)) return true;
	//if(a->m_Position.x+SizeA.cx >= b->m_Position.x+(SizeB.cx/2)) return false;

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
	if(a->m_Position.x >= b->m_Position.x+(SizeB.cx/2)) return true;
	//if(a->m_Position.x < b->m_Position.x+(SizeB.cx/2)) return false;

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
	m_bDeleted(false),
	m_nOrder(0),
	m_nSubLayer(-1), // DrawContexts are initially on sublayer -1 (not drew)
	m_Position(0,0),
	m_Size(-1,-1),
	m_nInsertion(0),
	m_bValidMap(false),
	m_pPtr(NULL),
	m_rgbBkColor(255,0,0,0)
{
	DestroyStateCallback(CDrawableContext::DestroyCheckpoint, (LPARAM)this);
	memset(m_pBuffer, 0, sizeof(m_pBuffer));
	for(int i=0; i<MAX_SUBLAYERS; i++) {
		// The default ordering is birthOrder, birthOrder, yOrder, birthOrder, birthOrder, yOrder... (one and one)
		m_eDrawType[i] = (((i+1)%3)==0)?yOrder:birthOrder;
		m_eSorted[i] = noOrder;
	}
}
CDrawableContext::~CDrawableContext() 
{
	BEGIN_DESTRUCTOR
	Clean();
	END_DESTRUCTOR
};

// Memento interface
void CDrawableContext::ReadState(StateData *data)
{
	StateDrawableContext *curr = static_cast<StateDrawableContext *>(data);
	curr->bDeleted = m_bDeleted;
	curr->rgbBkColor = m_rgbBkColor;
	curr->nSubLayer = m_nSubLayer;
	curr->Position = m_Position;
	curr->Size = m_Size;
	curr->dwStatus = m_dwStatus;
	curr->bSelected = m_bSelected;
}
void CDrawableContext::WriteState(StateData *data)
{
	StateDrawableContext *curr = static_cast<StateDrawableContext *>(data);

	bool bInvalidate = false;
	if(m_Size != curr->Size) bInvalidate = true;
	if(m_dwStatus != curr->dwStatus) bInvalidate = true;

	m_bDeleted = curr->bDeleted;
	m_rgbBkColor = curr->rgbBkColor;
	m_nSubLayer = curr->nSubLayer;
	m_Position = curr->Position;
	m_Size = curr->Size;
	m_dwStatus = curr->dwStatus;
	m_bSelected = curr->bSelected;

	if(bInvalidate) InvalidateBuffers();
}
int CALLBACK CDrawableContext::DestroyCheckpoint(LPVOID Interface, LPARAM lParam)
{
	StateDrawableContext *curr = static_cast<StateDrawableContext *>(Interface);
	delete curr;
	return 1;
}

bool CDrawableContext::AddSibling(CDrawableContext *object) 
{
	if(m_pParent == NULL) return false;
	return m_pParent->AddChild(object);
}

bool CDrawableContext::InsertChild(CDrawableContext *object, int nInsertion) 
{

	// First order the children if needed (expensive but necessary):
	// (could be avoided here if it was done at a higher level)
	if(!m_bValidMap) PreSort();
	if(m_eSorted[object->m_nSubLayer] != m_eDrawType[object->m_nSubLayer])
		Sort(object->m_nSubLayer);

	if(nInsertion != -1) m_nInsertion = nInsertion;

	ASSERT(object->m_pParent == NULL); // child must be orphan.

	object->m_pParent = this;
	if(m_nInsertion >= m_Children.size()) {
		m_Children.push_back(object);
		m_nInsertion = m_Children.size();
	} else {
		std::vector<CDrawableContext*>::iterator Iterator =
			m_Children.insert(m_Children.begin() + m_nInsertion, object);
		int nNewOrder = ++m_nInsertion;
		while(++Iterator != m_Children.end()) {
			(*Iterator)->m_nOrder = nNewOrder++;
		}
	}

	m_bValidMap = false;
	m_eSorted[object->m_nSubLayer] = noOrder;

	return true;
}

int CDrawableContext::_MergeChildren(CDrawableContext *object) 
{
	if(object->m_bDeleted) return 0; // this also prevents context sets to be merged

	int nMerged = 0;
	// Merge contexts (never merge entities):
	CDrawableObject *TrueObject = object->GetDrawableObj();
	if(TrueObject == NULL) return object->MergeChildren();

	if(!CanMerge(TrueObject)) return 0;

	CRect Rect, NewRect;
	object->GetAbsFinalRect(NewRect);

	std::vector<CDrawableContext*>::iterator Iterator = m_Children.begin();
	while(Iterator != m_Children.end()) {
		CDrawableObject *Tow = (*Iterator)->GetDrawableObj();
		if(*Iterator == object || Tow == NULL || (*Iterator)->m_bDeleted) {
			Iterator++;
			continue;
		}
		if(Tow == TrueObject) {
			bool bMerge = false;
			(*Iterator)->GetAbsFinalRect(Rect);
			if(Rect == NewRect) {
				bMerge = true;
			} else if(Rect.top == NewRect.bottom && NewRect.left == Rect.left && NewRect.right == Rect.right) {
				bMerge = true;
				NewRect.bottom = Rect.bottom;
			} else if(Rect.bottom == NewRect.top && NewRect.left == Rect.left && NewRect.right == Rect.right) {
				bMerge = true;
				NewRect.top = Rect.top;
			} else if(Rect.left == NewRect.right && NewRect.top == Rect.top && NewRect.bottom == Rect.bottom) {
				bMerge = true;
				NewRect.right = Rect.right;
			} else if(Rect.right == NewRect.left && NewRect.top == Rect.top && NewRect.bottom == Rect.bottom) {
				bMerge = true;
				NewRect.left = Rect.left;
			}
			if(bMerge) {
				object->SetAbsFinalRect(NewRect);
				DeleteChild(*Iterator);
				nMerged++;
			}
		}
		Iterator++;
	}
	return nMerged;
}

int CDrawableContext::MergeChildren() 
{
	for(int i=0; i<MAX_SUBLAYERS; i++) sort(m_LayersMap[i+1], m_LayersMap[i+2], m_cmpYX); 
	int nMerged = 0;
	int cnt;
	do {
		cnt = 0;
		std::vector<CDrawableContext*>::iterator Iterator = m_Children.begin();
		while(Iterator != m_Children.end()) {
			cnt += _MergeChildren(*Iterator);
			Iterator++;
		}
		nMerged += cnt;
	} while(cnt);
	for(int i=0; i<MAX_SUBLAYERS; i++) Sort(i);
	return nMerged;
}

bool CDrawableContext::AddChild(CDrawableContext *object) 
{ 
	ASSERT(object);
	ASSERT(object->m_pParent == NULL); // child must be orphan.
	object->m_pParent = this;
	object->m_nOrder = m_nInsertion++;
	m_Children.push_back(object);
	m_bValidMap = false;
	m_eSorted[object->m_nSubLayer] = noOrder;
	return true;
}

// Reorder the objects (birth order is reordered):
int CDrawableContext::ReOrder(int nStep, int nRoomAt, int nRoomSize) 
{
	// resort everything (expensive, but necessary):
	for(int i=0; i<MAX_SUBLAYERS; i++) Sort(i);

	// Reassign birth orders:
	m_nInsertion = 0;
	std::vector<CDrawableContext*>::iterator Iterator = m_Children.begin();
	while(Iterator != m_Children.end()) {
		if(m_nInsertion == nRoomAt) m_nInsertion += nRoomSize;
		(*Iterator)->m_nOrder = m_nInsertion;
		m_nInsertion += nStep;
		Iterator++;
	}
	if(nRoomAt != -1) return nRoomAt;
	return m_nInsertion;
}

// Sublayer or -1 to draw all layers
// We build a layers map to speed things up a little:
void CDrawableContext::PreSort() 
{
	// First, we sort all the elements by their sublayer:
	stable_sort(m_Children.begin(), m_Children.end(), m_cmpSubLayer);

	// Generate the map of children
	std::vector<CDrawableContext *>::iterator Iterator;
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
	// *Optimization (2)
	switch(m_eDrawType[nSubLayer]) {
		case yOrder: 
			sort(m_LayersMap[nSubLayer+1], m_LayersMap[nSubLayer+2], m_cmpY); 
			break;
		case rightIso: 
			sort(m_LayersMap[nSubLayer+1], m_LayersMap[nSubLayer+2], m_cmpYX); 
			break;
		case leftIso: 
			sort(m_LayersMap[nSubLayer+1], m_LayersMap[nSubLayer+2], m_cmpYiX); 
			break;
		case birthOrder: 
			sort(m_LayersMap[nSubLayer+1], m_LayersMap[nSubLayer+2], m_cmpOrder); 
			break;
	}
	m_eSorted[nSubLayer] = m_eDrawType[nSubLayer];
}

inline bool CDrawableContext::CleanTempContext::operator()(CDrawableContext *pDrawableContext) const
{
	ASSERT(pDrawableContext);
	if(!pDrawableContext) return false;

	if( pDrawableContext->m_bDeleted ) return true;

	// Start cleaning:
	for_each(
		pDrawableContext->m_Children.begin(), pDrawableContext->m_Children.end(), 
		CleanTempContext());

	if(pDrawableContext->isTemp()) {
		CDrawableContext *pParent = pDrawableContext->m_pParent;
		std::vector<CDrawableContext *>::iterator Iterator = find(
			pParent->m_Children.begin(),
			pParent->m_Children.end(),
			pDrawableContext );
		if(Iterator != pParent->m_Children.end()) {
			ASSERT(*Iterator == pDrawableContext);
			pParent->m_bValidMap = false;
			pParent->m_eSorted[pDrawableContext->m_nSubLayer] = noOrder;
			delete pDrawableContext;
			pParent->m_Children.erase(Iterator);
		}
	}
	return true;
}
bool CDrawableContext::CleanTemp() 
{
	CleanTempContext CleanTemp;
	return CleanTemp(this);
}

inline bool CDrawableContext::RunContext::operator()(CDrawableContext *pDrawableContext, RUNACTION action) const
{
	ASSERT(pDrawableContext);
	if(!pDrawableContext) return false;

	if( pDrawableContext->m_bDeleted ) return true;

	if( !pDrawableContext->isVisible() && m_bVisible )
		return true;

	// Start executing the scripts (or whatever)
	for_each(
		pDrawableContext->m_Children.begin(), pDrawableContext->m_Children.end(), 
		bind2nd(RunContext(m_bVisible), action));

	if(pDrawableContext->m_pDrawableObj) {
		return pDrawableContext->m_pDrawableObj->Run(*pDrawableContext, action);
	}
	return true;
}
bool CDrawableContext::Run(RUNACTION action) 
{
	RunContext Run(true);
	return Run(this, action);
}

inline bool CDrawableContext::DrawContext::operator()(CDrawableContext *pDrawableContext, const IGraphics *pIGraphics) const
{
	ASSERT(pDrawableContext);
	if(!pDrawableContext) return false;

	if( pDrawableContext->m_bDeleted ) return true;

	// is this context visible? (if not, none of its children are either)
	if( !pDrawableContext->isVisible() && m_bVisible ) 
		return true;

	if( !pDrawableContext->isSelected() && m_bSelected && pDrawableContext->m_pDrawableObj ) 
		return true;

	if(pIGraphics) pDrawableContext->m_pIGraphics = pIGraphics;

	if(!pDrawableContext->m_bValidMap) pDrawableContext->PreSort();
	for(int nSubLayer=0; nSubLayer<MAX_SUBLAYERS; nSubLayer++) {
		if(pDrawableContext->m_eSorted[nSubLayer] != pDrawableContext->m_eDrawType[nSubLayer]) 
			pDrawableContext->Sort(nSubLayer);
	}

	// Start drawing objects from sublayer 0 (at position [1] in the layers map):
	for_each(
		pDrawableContext->m_LayersMap[1], pDrawableContext->m_Children.end(), 
		bind2nd(DrawContext(m_bVisible, m_bSelected, m_bHighlight), pIGraphics));

	if(pDrawableContext->m_pDrawableObj) {
		if(!pDrawableContext->isSelected() && m_bHighlight) {
			ARGBCOLOR rgbColor = COLOR_ARGB(255,128,128,128);
			return pDrawableContext->m_pDrawableObj->Draw(*pDrawableContext, &rgbColor);
		}
		return pDrawableContext->m_pDrawableObj->Draw(*pDrawableContext);
	}
	return true;
}

bool CDrawableContext::Draw(const IGraphics *pIGraphics) 
{
	DrawContext Draw(true, false, false);
	return Draw(this, pIGraphics);
}

bool CDrawableContext::DrawSelected(const IGraphics *pIGraphics) 
{
	DrawContext Draw(false, true, false);
	return Draw(this, pIGraphics);
}
bool CDrawableContext::DrawSelectedH(const IGraphics *pIGraphics) 
{
	DrawContext Draw(true, false, true);
	return Draw(this, pIGraphics);
}

// counts the number of drawable contexts with a drawable object assigned to it.
int CDrawableContext::Objects(int init)
{
	std::vector<CDrawableContext*>::iterator Iterator = m_Children.begin();
	while(Iterator != m_Children.end()) {
		init = (*Iterator)->Objects(init);
		Iterator++;
	}

	if(!m_bDeleted && m_pDrawableObj) init++;
	return init;
}
bool CDrawableContext::PopChild(CDrawableContext *pDrawableContext_)
{
	ASSERT(pDrawableContext_);
	if(pDrawableContext_ == NULL) return true;

	//ASSERT(!"This is probably not what you want, try DeleteChild()");
	// Search for the requested child and if found, erase if from the list and return true
	std::vector<CDrawableContext*>::iterator Iterator =
		find(m_Children.begin(), m_Children.end(), pDrawableContext_);
	if(Iterator != m_Children.end()) {
		ASSERT((*Iterator) == pDrawableContext_);
		m_bValidMap = false; //FIXME: this is probably not needed
		m_eSorted[(*Iterator)->m_nSubLayer] = noOrder; //FIXME: this is probably not needed
		(*Iterator)->m_pParent = NULL; // this will be an orphan child.
		VERIFY(m_Children.erase(Iterator) != m_Children.end());
		return true;
	}
	return false;
}
bool CDrawableContext::PopChildEx(CDrawableContext *pDrawableContext_)
{
	ASSERT(pDrawableContext_);

	if(PopChild(pDrawableContext_)) {
		return true;
	}
	// Propagate de search to the children:
	std::vector<CDrawableContext*>::iterator Iterator = Iterator = m_Children.begin();
	while(Iterator != m_Children.end()) {
		if((*Iterator)->PopChildEx(pDrawableContext_)) {
			return true;
		}
		Iterator++;
	}
	return false;

}
bool CDrawableContext::KillChild(CDrawableContext *pDrawableContext_)
{
	ASSERT(pDrawableContext_);

	// Search for the requested child and if found, kill it (its own children get killed too)
	if(PopChild(pDrawableContext_)) {
		delete pDrawableContext_;
		return true;
	}
	return false;
}
// This function really kills and erases a child, so use it careful 
// (instead you probably should be using DeleteChild)
bool CDrawableContext::KillChildEx(CDrawableContext *pDrawableContext_)
{
	ASSERT(pDrawableContext_);

	// Search for the requested child and if found, kill it (its own children get killed too)
	if(KillChild(pDrawableContext_)) {
		return true;
	}
	// Propagate the search to the children:
	std::vector<CDrawableContext*>::iterator Iterator = Iterator = m_Children.begin();
	while(Iterator != m_Children.end()) {
		if((*Iterator)->KillChildEx(pDrawableContext_)) {
			return true;
		}
		Iterator++;
	}
	return false;
}

bool CDrawableContext::DeleteChild(CDrawableContext *pDrawableContext_)
{
	ASSERT(pDrawableContext_);

	std::vector<CDrawableContext*>::iterator Iterator =
		find(m_Children.begin(), m_Children.end(), pDrawableContext_);
	if(Iterator != m_Children.end()) {
		ASSERT(!(*Iterator)->m_bSelected);
		(*Iterator)->m_bDeleted = true; // flag it as "deleted"
		(*Iterator)->Touch();
		return true;
	}
	return false;
}

bool CDrawableContext::DeleteChildEx(CDrawableContext *pDrawableContext_)
{
	ASSERT(pDrawableContext_);

	// Search for the requested child and if found, flag it as deleted
	if(DeleteChild(pDrawableContext_)) {
		return true;
	}
	// Propagate the search to the children:
	std::vector<CDrawableContext*>::iterator Iterator = Iterator = m_Children.begin();
	while(Iterator != m_Children.end()) {
		if((*Iterator)->DeleteChildEx(pDrawableContext_)) {
			return true;
		}
		Iterator++;
	}
	return false;
}

void CDrawableContext::Clean()
{
	for(int i=0; i<CONTEXT_BUFFERS; i++) {
		if(m_pBuffer[i]) {
			m_pBuffer[i]->Invalidate(true);
			m_pBuffer[i]->Release();
		}
	}

	// Optimizated using for_each() instead of a loop
	for_each(m_Children.begin(), m_Children.end(), ptr_delete());
	m_Children.clear();
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
	if(!m_bDeleted && isAt(point_)) {
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
	if(!m_bDeleted && isAt(point_)) {
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
	if(!m_bDeleted && isIn(rect_)) {
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
	if(!m_bDeleted && isIn(rect_)) {
		*ppDrawableContext_ = this;
	}
	return true;
}

int CDrawableContext::SaveState(UINT checkpoint) 
{
	int nRet = 0;
	// Propagate the state saving to the children:
	std::vector<CDrawableContext*>::iterator Iterator = Iterator = m_Children.begin();
	while(Iterator != m_Children.end()) {
		if(*Iterator) {
			// We need to keep the current position of the Iterator:
			int nDist = distance(m_Children.begin(), Iterator);
			// SaveState kills useless children (garbage collector) but if a children
			// was erased, the order of the vector, or the full vector itself
			// might have had changed, thus making the Iterator useless.
			CDrawableContext *curr = *Iterator;
			nRet += curr->SaveState(checkpoint); 
			Iterator = m_Children.begin();
			// If SaveState in fact killed a child, advance will create a new iterator
			// that would already point to next item to work with, otherwise it will
			// still point to current drawable context.
			advance(Iterator, nDist);
			if(*Iterator != curr || Iterator == m_Children.end()) continue;
		}
		Iterator++;
	}
	// Save this context state:
	nRet += _SaveState(checkpoint);
	return nRet;
}

int CDrawableContext::RestoreState(UINT checkpoint)
{
	int nRet = 0;
	// Propagate the state restoring to the children:
	std::vector<CDrawableContext*>::iterator Iterator = Iterator = m_Children.begin();
	while(Iterator != m_Children.end()) {
		if(*Iterator) {
			nRet += (*Iterator)->RestoreState(checkpoint);
		}
		Iterator++;
	}
	if(nRet) Touch(false);
	// Restore this context state:
	nRet += _RestoreState(checkpoint);
	return nRet;
}

bool CDrawableContext::HasChanged()
{
	// Restore this context state:
	bool bRet = CMutable::HasChanged();
	// Propagate the call to the children:
	std::vector<CDrawableContext*>::iterator Iterator = Iterator = m_Children.begin();
	while(Iterator != m_Children.end()) {
		ASSERT(*Iterator);
		if(*Iterator) {
			bRet |= (*Iterator)->HasChanged();
		}
		Iterator++;
	}
	return bRet;
}

bool CDrawableContext::IsModified()
{
	// Restore this context state:
	bool bRet = CMutable::IsModified();
	// Propagate the call to the children:
	std::vector<CDrawableContext*>::iterator Iterator = Iterator = m_Children.begin();
	while(Iterator != m_Children.end()) {
		ASSERT(*Iterator);
		if(*Iterator) {
			bRet |= (*Iterator)->IsModified();
		}
		Iterator++;
	}
	return bRet;
}

void CDrawableContext::WasSaved()
{
	// Restore this context state:
	CMutable::WasSaved();
	// Propagate the call to the children:
	std::vector<CDrawableContext*>::iterator Iterator = Iterator = m_Children.begin();
	while(Iterator != m_Children.end()) {
		ASSERT(*Iterator);
		if(*Iterator) {
			(*Iterator)->WasSaved();
		}
		Iterator++;
	}
}
