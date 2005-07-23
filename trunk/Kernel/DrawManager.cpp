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
	m_bSuperContext(false),
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
	ASSERT(curr);

	curr->bDeleted = m_bDeleted;
	curr->rgbBkColor = m_rgbBkColor;
	curr->nSubLayer = m_nSubLayer;
	curr->Position = m_Position;
	curr->Size = m_Size;
	curr->dwStatus = m_dwStatus;
	curr->bSelected = m_bSelected;
	curr->nOrder = m_nOrder;
	curr->pParent = m_pParent;
	curr->pSuperContext = GetSuperContext();
}
void CDrawableContext::WriteState(StateData *data)
{
	StateDrawableContext *curr = static_cast<StateDrawableContext *>(data);
	ASSERT(curr);

	CDrawableContext *pSuperContext = GetSuperContext();

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
	m_nOrder = curr->nOrder;

	if(curr->pSuperContext != pSuperContext && curr->pSuperContext && pSuperContext) {
		CDrawableContext *pParent = m_pParent;
		VERIFY(pSuperContext->PopChild(this) &&  curr->pSuperContext->InsertChild(this, m_nOrder));
		if(pParent != pSuperContext) m_pParent = pParent; // recover parent of the supercontext's non-immediate children.
	}

	if(curr->pParent != m_pParent && curr->pParent && m_pParent) {
		if(m_pParent->m_bSuperContext) m_pParent = NULL; // make the child pseudo-orphan
		else VERIFY(m_pParent->PopChild(this));
		if(curr->pParent->m_bSuperContext) m_pParent = curr->pParent;
		else VERIFY(curr->pParent->InsertChild(this, m_nOrder));
	}

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

	m_bValidMap = false; // force a PreSort()
	m_eSorted[object->m_nSubLayer] = noOrder; // force a Sort() for the sublayer

	return true;
}

bool CDrawableContext::AddChild(CDrawableContext *object) 
{ 
	ASSERT(object);
	ASSERT(object->m_pParent == NULL); // child must be orphan.
	object->m_pParent = this;
	object->m_nOrder = m_nInsertion++;
	m_Children.push_back(object);
	m_bValidMap = false; // force a PreSort()
	m_eSorted[object->m_nSubLayer] = noOrder; // force a Sort() for the sublayer
	return true;
}

int CDrawableContext::_MergeChildren(CDrawableContext *object) 
{
	if(object->isDeleted()) return 0; // this also prevents context sets to be merged

	int nMerged = 0;
	// Merge contexts (never merge entities):
	CDrawableObject *TrueObject = object->GetDrawableObj();
	if(TrueObject == NULL) return object->MergeChildren();

	if(!CanMerge(TrueObject)) return 0;

	CRect Rect, NewRect;
	object->GetAbsFinalRect(NewRect);

	std::vector<CDrawableContext*>::iterator Iterator = m_Children.begin();
	while(Iterator != m_Children.end()) {
		if(object->m_pParent != (*Iterator)->m_pParent) {
			Iterator++;
			continue;
		}
		CDrawableObject *Tow = (*Iterator)->GetDrawableObj();
		if(*Iterator == object || Tow == NULL || (*Iterator)->isDeleted()) {
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
	// First we sort all objects in YX order, so the algorithm works better:
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
	// We sort back to the proper ordering on every sublayer (expensive, but necessary):
	if(!m_bValidMap) PreSort();
	for(int i=0; i<MAX_SUBLAYERS; i++) Sort(i);
	return nMerged;
}

CDrawableContext* CDrawableContext::SetGroup(LPCSTR szGroupName)
{
	CDrawableContext *pSuperContext = GetSuperContext();
	ASSERT(pSuperContext);
	
	pSuperContext->ReOrder(); // sorry, but creating groups is expensive.
	CDrawableContext *pSiblingContext = GetSibling(szGroupName);
	if(!pSiblingContext) {
		pSiblingContext = MakeGroup(szGroupName);
		if(!m_pParent || !pSiblingContext || !m_pParent->InsertChild(pSiblingContext, m_nOrder)) {
			delete pSiblingContext;
			return NULL;
		}
		if(m_pParent != pSuperContext) {
			VERIFY(pSuperContext->InsertChild(pSiblingContext, m_nOrder));
		}
		pSiblingContext->m_nOrder = m_nOrder;
		pSiblingContext->m_nSubLayer = m_nSubLayer;
	}
	ASSERT(m_pParent);
	// If the parent is not a super context, do not pop the sprite from it, just add it to the new context
	if(!m_pParent->m_bSuperContext) VERIFY(m_pParent->PopChild(this));
	else m_pParent = NULL; // make the child pseudo-orphan
    VERIFY(pSiblingContext->InsertChild(this, m_nOrder));
	Touch();
	return pSiblingContext;
}

// Reorder the objects (birth order is reordered):
int CDrawableContext::ReOrder(int nStep, int nRoomAt, int nRoomSize) 
{
	// resort everything (expensive, but necessary):
	if(!m_bValidMap) PreSort();
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
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// All children of a drawable context are kept in the same container (m_Children vector); however, as we know,
// children can be drawn in different order (Y, XY, birth, XiY order, etc.); now this ordering system depends on 
// the sublayer each object is in, but having them all in the same container makes it difficult to sort with different 
// schemes. The solutiuon we use is simple: before really sorting the sprites (byt ther supposed order for a given 
// sub layer) we first sort all the sprites in sublayers blocks and we put each block in a map of layers (m_LayersMap)
// After doing this, we end up with an array of iterators, each iterator in the array pointing to the first
// element on the children's container with a different sublayer than the last one. This strategy lets us later
// sort just the part of the continer that has actually changed or that is needed to sort (as sorting just a single
// layer.
// Having explained that, PreSort() *must* (or at least should) be called whenever you insert or add a children.
// We build a layer's map to speed things up a little (or rather a lot):
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

	// only clean objects whose real parent is the supercontext:
	if(pDrawableContext->m_pParent && m_pParent != pDrawableContext->m_pParent->GetSuperContext()) 
		return true;

	// Start cleaning:
	for_each(
		pDrawableContext->m_Children.begin(), pDrawableContext->m_Children.end(), 
		CleanTempContext(pDrawableContext));

	if(pDrawableContext->isTemp() || pDrawableContext->isDeleted()) {
		CDrawableContext *pParent = pDrawableContext->m_pParent;
		//FIXME: what about objects in groups, they should be poped from the supercontext as well??
		VERIFY(pParent && pParent->KillChild(pDrawableContext));
	}
	return true;
}
bool CDrawableContext::CleanTemp() 
{
	CleanTempContext CleanTemp(GetSuperContext());
	return CleanTemp(this);
}

inline bool CDrawableContext::RunContext::operator()(CDrawableContext *pDrawableContext, RUNACTION action) const
{
	ASSERT(pDrawableContext);
	if(!pDrawableContext) return false;

	// only run objects whose real parent is the supercontext:
	if(pDrawableContext->m_pParent && m_pParent != pDrawableContext->m_pParent->GetSuperContext()) 
		return true;

	if( pDrawableContext->isDeleted() ) 
		return true;

	if( !pDrawableContext->isVisible() && m_bVisible )
		return true;

	// Start executing the scripts (or whatever)
	for_each(
		pDrawableContext->m_Children.begin(), pDrawableContext->m_Children.end(), 
		bind2nd(RunContext(pDrawableContext, m_bVisible), action));

	if(pDrawableContext->m_pDrawableObj) {
		return pDrawableContext->m_pDrawableObj->Run(*pDrawableContext, action);
	}
	return true;
}
bool CDrawableContext::Run(RUNACTION action) 
{
	RunContext Run(GetSuperContext(), true);
	return Run(this, action);
}

inline bool CDrawableContext::DrawContext::operator()(CDrawableContext *pDrawableContext, const IGraphics *pIGraphics) const
{
	ASSERT(pDrawableContext);
	if(!pDrawableContext) return false;

	// only draw objects whose real parent is the supercontext:
	if(pDrawableContext->m_pParent && m_pParent != pDrawableContext->m_pParent->GetSuperContext()) 
		return true;

	// is this context flagged as deleted? (if it is, all of its children are deleted too)
	if( pDrawableContext->isDeleted() ) 
		return true;

	// is this context visible? (if not, none of its children are either)
	if( !pDrawableContext->isVisible() && m_bVisible ) 
		return true;

	// is this context selected? (if not, and it's not a leaf, maybe some of the children are selected)
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
		bind2nd(DrawContext(pDrawableContext, m_bVisible, m_bSelected, m_bHighlight), pIGraphics));

	if(pDrawableContext->m_pDrawableObj) {
		if(!pDrawableContext->isSelected() && m_bHighlight) {
			ARGBCOLOR rgbColor = COLOR_ARGB(255,128,128,128);
			return pDrawableContext->m_pDrawableObj->Draw(*pDrawableContext, &rgbColor);
		}
		return pDrawableContext->m_pDrawableObj->Draw(*pDrawableContext);
	}/**/
	return true;
}

bool CDrawableContext::Draw(const IGraphics *pIGraphics) 
{
	DrawContext Draw(GetSuperContext(), true, false, false);
	return Draw(this, pIGraphics);
}

bool CDrawableContext::DrawSelected(const IGraphics *pIGraphics) 
{
	DrawContext Draw(GetSuperContext(), false, true, false);
	return Draw(this, pIGraphics);
}
bool CDrawableContext::DrawSelectedH(const IGraphics *pIGraphics) 
{
	DrawContext Draw(GetSuperContext(), true, false, true);
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

	if(!isDeleted() && m_pDrawableObj) init++;
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
		(*Iterator)->DeleteContext(); // flag it as "deleted"
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

bool CDrawableContext::GetFirstChildAt(const CPoint &point_, CDrawableContext **ppDrawableContext_, CDrawableContext *pParent_)
{
	if(pParent_ && pParent_ != m_pParent) return true;
	CDrawableContext *pToRet = NULL;
	m_ChildIterator = m_Children.rbegin();
	while(m_ChildIterator != m_Children.rend()) {
		m_LastChildIteratorUsed = m_ChildIterator;
		if((*m_ChildIterator)->GetFirstChildAt(point_, &pToRet, this) ) m_ChildIterator++;
		if(pToRet) {
			*ppDrawableContext_ = pToRet;
			return false;
		}
	}
	if(!isDeleted() && isAt(point_)) {
		*ppDrawableContext_ = this;
	}
	return true;
}
bool CDrawableContext::GetNextChildAt(const CPoint &point_, CDrawableContext **ppDrawableContext_, CDrawableContext *pParent_)
{
	if(pParent_ && pParent_ != m_pParent) return true;
	CDrawableContext *pToRet = NULL;
	while(m_ChildIterator != m_Children.rend()) {
		if(m_ChildIterator != m_LastChildIteratorUsed) {
			m_LastChildIteratorUsed = m_ChildIterator;
			if( (*m_ChildIterator)->GetFirstChildAt(point_, &pToRet, this) ) m_ChildIterator++;
			if(pToRet) {
				*ppDrawableContext_ = pToRet;
				return false;
			}
		} else {
			if( (*m_ChildIterator)->GetNextChildAt(point_, &pToRet, this) ) m_ChildIterator++;
			if(pToRet) {
				*ppDrawableContext_ = pToRet;
				return false;
			}
		}
	}
	if(!isDeleted() && isAt(point_)) {
		*ppDrawableContext_ = this;
	}
	return true;
}

bool CDrawableContext::GetFirstChildIn(const RECT &rect_, CDrawableContext **ppDrawableContext_, CDrawableContext *pParent_)
{
	if(pParent_ && pParent_ != m_pParent) return true;
	CDrawableContext *pToRet = NULL;
	m_ChildIterator = m_Children.rbegin();
	while(m_ChildIterator!= m_Children.rend()) {
		m_LastChildIteratorUsed = m_ChildIterator;
		if( (*m_ChildIterator)->GetFirstChildIn(rect_, &pToRet, this) ) m_ChildIterator++;
		if(pToRet) {
			*ppDrawableContext_ = pToRet;
			return false;
		}
	}
	if(!isDeleted() && isIn(rect_)) {
		*ppDrawableContext_ = this;
	}
	return true;
}
bool CDrawableContext::GetNextChildIn(const RECT &rect_, CDrawableContext **ppDrawableContext_, CDrawableContext *pParent_)
{
	if(pParent_ && pParent_ != m_pParent) return true;
	CDrawableContext *pToRet = NULL;
	while(m_ChildIterator!= m_Children.rend()) {
		if(m_ChildIterator != m_LastChildIteratorUsed) {
			m_LastChildIteratorUsed = m_ChildIterator;
			if( (*m_ChildIterator)->GetFirstChildIn(rect_, &pToRet, this) ) m_ChildIterator++;
			if(pToRet) {
				*ppDrawableContext_ = pToRet;
				return false;
			}
		} else {
			if( (*m_ChildIterator)->GetNextChildIn(rect_, &pToRet, this) ) m_ChildIterator++;
			if(pToRet) {
				*ppDrawableContext_ = pToRet;
				return false;
			}
		}
	}
	if(!isDeleted() && isIn(rect_)) {
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
			// RestoreState can move children around layers, etc.
			// SaveState, above, explains the following...
			int nDist = distance(m_Children.begin(), Iterator);
			CDrawableContext *curr = *Iterator;
			nRet += curr->RestoreState(checkpoint); 
			Iterator = m_Children.begin();
			advance(Iterator, nDist);
			if(*Iterator != curr || Iterator == m_Children.end()) continue;
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
