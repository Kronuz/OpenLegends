/* QuestDesigner - Open Zelda's Project
   Copyright (C) 2003. Kronuz (Germán Méndez Bravo)
   Copyright (C) 2001-2003. Open Zelda's Project
 
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

#include "../IGame.h"

bool SObjProp::isFlagged() {
	return bSubselected;
}
void SObjProp::Flag(bool bFlag) { 
	bSubselected = bFlag;
}
bool SObjProp::GetInfo(SInfo *pI) const
{
	bool ret = pContext->GetInfo(pI);
	if(!ret) return false;

	pI->pPropObject = (IPropertyEnabled*)this;
	return true;
}
bool SObjProp::GetProperties(SPropertyList *pPL) const
{
	bool ret = pContext->GetProperties(pPL);
	if(!ret) return false;

	pPL->Information.pPropObject = (IPropertyEnabled*)this;

	pPL->AddList("Horizontal Chain", eXChain, "0 - Relative, 1 - Fill, 2 - Right, 3 - Left, 4 - Fixed");
	pPL->AddList("Vertical Chain", eYChain, "0 - Relative, 1 - Fill, 2 - Up, 3 - Down, 4 - Fixed");

	return true;
}
bool SObjProp::SetProperties(SPropertyList &PL)
{
	SProperty* pP;
	bool bChanged = pContext->SetProperties(PL);
	if(bChanged) pSelection->m_bChanged = pSelection->m_bModified = true;

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
	pContext->Commit();
}
void SObjProp::Cancel()
{
	pContext->Cancel();
}

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
	m_nOrder(0),
	m_nSubLayer(-1), // DrawContexts are initially on sublayer -1 (not drew)
	m_Position(0,0),
	m_Size(-1,-1),
	m_nInsertion(0),
	m_bValidMap(false),
	m_pPtr(NULL),
	m_rgbBkColor(255,0,0,0)
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
	Clean();
};

bool CDrawableContext::AddSibling(CDrawableContext *object, bool bAllowDups_) 
{
	if(m_pParent == NULL) return false;
	return m_pParent->AddChild(object, bAllowDups_);
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
		vector<CDrawableContext*>::iterator Iterator =
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
bool CDrawableContext::AddChild(CDrawableContext *object, bool bAllowDups_) 
{ 
	ASSERT(object);

	if(!bAllowDups_) {
		vector<CDrawableContext*>::iterator Iterator = 
			find_if(
				m_Children.begin(), m_Children.end(), 
				bind2nd(ptr_equal_to<CDrawableContext*>(), object));
		if(Iterator != m_Children.end()) {
			(*Iterator)->m_nOrder = m_nInsertion++;	// renew the order of the located item, 
			return false;							// but skip duplicates.
		}
	}

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
	vector<CDrawableContext*>::iterator Iterator = m_Children.begin();
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

	// Start cleaning:
	for_each(
		pDrawableContext->m_Children.begin(), pDrawableContext->m_Children.end(), 
		CleanTempContext());

	if(pDrawableContext->isTemp()) {
		CDrawableContext *pParent = pDrawableContext->m_pParent;
		vector<CDrawableContext *>::iterator Iterator = find(
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
	vector<CDrawableContext*>::iterator Iterator = m_Children.begin();
	while(Iterator != m_Children.end()) {
		init = (*Iterator)->Objects(init);
		Iterator++;
	}

	if(m_pDrawableObj) init++;
	return init;
}
bool CDrawableContext::PopChild(CDrawableContext *pDrawableContext_)
{
	// Search for the requested child and if found, erase if from the list and return true
	vector<CDrawableContext*>::iterator Iterator =
		find(m_Children.begin(), m_Children.end(), pDrawableContext_);
	if(Iterator != m_Children.end()) {
		m_bValidMap = false;
		m_eSorted[(*Iterator)->m_nSubLayer] = noOrder;
		(*Iterator)->m_pParent = NULL; // this will be an orphan child.
		m_Children.erase(Iterator);
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
	// Propagate de search for the child:
	vector<CDrawableContext*>::iterator Iterator = Iterator = m_Children.begin();
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
bool CDrawableContext::KillChildEx(CDrawableContext *pDrawableContext_)
{
	ASSERT(pDrawableContext_);

	// Search for the requested child and if found, kill it (its own children get killed too)
	if(KillChild(pDrawableContext_)) {
		return true;
	}
	// Propagate de search for the child:
	vector<CDrawableContext*>::iterator Iterator = Iterator = m_Children.begin();
	while(Iterator != m_Children.end()) {
		if((*Iterator)->KillChildEx(pDrawableContext_)) {
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
			m_pBuffer[i]->Invalidate();
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

inline bool CDrawableSelection::ObjPropContextEqual::operator()(const SObjProp &a, const CDrawableContext *b) const
{
	return (a.pContext == b);
}
inline bool CDrawableSelection::ObjPropLayerEqual::operator()(const SObjProp &a, const int &b) const
{
	ASSERT(a.pContext);
	return (a.pContext->GetObjLayer() == b);
}
inline bool CDrawableSelection::SelectionCompare::operator()(const SObjProp &a, const SObjProp &b) const
{
	ASSERT(a.pContext && a.pContext);
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
	m_bChanged(true),
	m_bModified(false),
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

	m_Objects.resize(16); // begin with 16 gropus
	m_ObjectsNames.resize(16);

	m_ppMainDrawable = ppDrawableContext_;
	ASSERT(m_ppMainDrawable);
}
int CDrawableSelection::GetBoundingRect(CRect *pRect_, int nPasteGroup_) 
{
	int nObjects = 0;
	CRect RectTmp;
	pRect_->SetRectEmpty();
	vectorObject::iterator Iterator;
	for(Iterator = m_Objects[nPasteGroup_].begin(); Iterator != m_Objects[nPasteGroup_].end(); Iterator++) {
		Iterator->pContext->GetAbsFinalRect(RectTmp);
		pRect_->UnionRect(pRect_, RectTmp);
		nObjects++;
	}
	return nObjects;
}

bool CDrawableSelection::isGroup()
{
	return (
		m_nCurrentGroup > 0 && 
		m_Objects[0].size() > 1 &&
		!m_Objects[m_nCurrentGroup].empty() );
}
// this function copies every object in a group to the main selection 
bool CDrawableSelection::SelectGroupWith(const CDrawableContext *pDrawableContext) 
{
	bool bEmpty = m_Objects[0].empty();
	for(int i=1; i<(int)m_Objects.size(); i++) {
		vectorObject::iterator Iterator = 
			find_if(m_Objects[i].begin(), m_Objects[i].end(), bind2nd(m_equalContext, pDrawableContext));
		if(Iterator != m_Objects[i].end()) {
			// Save the last current selected group (if there were no other selections):
			if(bEmpty) m_nCurrentGroup = i;
			else m_nCurrentGroup = 0;

			for(Iterator = m_Objects[i].begin(); Iterator != m_Objects[i].end(); Iterator++) {
				Iterator->pContext->SelectContext();
				m_Objects[0].push_back(*Iterator);
			}
			return true;
		}
	}
	return false;
}
// delete a context from all groups
void CDrawableSelection::DeleteInGroups(const CDrawableContext *pDrawableContext) 
{
	for(int i=1; i<(int)m_Objects.size(); i++) {
		vectorObject::iterator removed = 
			remove_if(m_Objects[i].begin(), m_Objects[i].end(), bind2nd(m_equalContext, pDrawableContext));
		if(removed != m_Objects[i].end()) {
			m_Objects[i].erase(removed, m_Objects[i].end());
			return;
		}
	}
}

IPropertyEnabled* CDrawableSelection::SelPointAdd(const CPoint &point_, int Chains) 
{
	int nLayer = m_Objects[0].size() ? m_nLayer : -1;
	CDrawableContext *pDrawableContext = NULL;

	(*m_ppMainDrawable)->GetFirstChildAt(point_, &pDrawableContext);

	while(pDrawableContext) {
		int nNewLayer = pDrawableContext->GetObjLayer();
		if(!isLocked(nNewLayer) && (nNewLayer == nLayer || nLayer == -1 || m_bAllowMultiLayerSel)) {
			m_nLayer = nLayer = nNewLayer;

			// try to find the sprite in the current selection:
			vectorObject::iterator Iterator = 
				find_if(m_Objects[0].begin(), m_Objects[0].end(), bind2nd(m_equalContext, pDrawableContext));
			if(Iterator == m_Objects[0].end()) {
				// the sprite is not yet selected, so try to select the full group containing the sprite (if any)
				if(SelectGroupWith(pDrawableContext)) return NULL;

				CRect Rect;
				pDrawableContext->GetRect(Rect);
				pDrawableContext->SelectContext();

				m_nCurrentGroup = 0;
				m_Objects[0].push_back(SObjProp(this, pDrawableContext, Rect, relative, relative));

				// set the last selection:
				m_pLastSelected = pDrawableContext;

				pDrawableContext->SelectContext();
				CONSOLE_DEBUG("%d of %d objects selected.\n", m_Objects[0].size(), (*m_ppMainDrawable)->Objects());
				CONSOLE_DEBUG("The selected object is on layer %d, and sublayer: %d.\n", pDrawableContext->GetObjLayer(), pDrawableContext->GetObjSubLayer());
				return NULL;
			}

			if(Chains>0) {
				if(m_Objects[0].size()<=1) return NULL; // ignore if there's only one object
				if(Iterator->eXChain<4) Iterator->eXChain = (_Chain)((int)Iterator->eXChain+1);
				else {
					Iterator->eXChain = (_Chain)0;
					if(Iterator->eYChain<4) Iterator->eYChain = (_Chain)((int)Iterator->eYChain+1);
					else Iterator->eYChain = (_Chain)0;
				}
				// set the last selection:
				m_pLastSelected = Iterator->pContext;
				CONSOLE_DEBUG("XChain: %d, YChain: %d\n", (int)Iterator->eXChain, (int)Iterator->eYChain);
				return static_cast<IPropertyEnabled *>(Iterator.operator ->());
			} else if(Chains<0) {
				if(m_Objects[0].size()<=1) return NULL; // ignore if there's only one object
				if(Iterator->eXChain>0) Iterator->eXChain = (_Chain)((int)Iterator->eXChain-1);
				else {
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
		pDrawableContext = NULL;
		(*m_ppMainDrawable)->GetNextChildAt(point_, &pDrawableContext);
	}
	return NULL;
}
void CDrawableSelection::SelPointRemove(const CPoint &point_) 
{
	int nLayer = m_Objects[0].size() ? m_nLayer : -1;
	CDrawableContext *pDrawableContext = NULL;
	(*m_ppMainDrawable)->GetFirstChildAt(point_, &pDrawableContext);
	while(pDrawableContext) {
		int nNewLayer = pDrawableContext->GetObjLayer();
		if(!isLocked(nNewLayer) && (nNewLayer == nLayer || nLayer == -1 || m_bAllowMultiLayerSel)) {
			nLayer = nNewLayer;

			// remove the last selection:
			if(m_pLastSelected == pDrawableContext) m_pLastSelected = NULL;

			pDrawableContext->SelectContext(false);
			vectorObject::iterator removed = 
				remove_if(m_Objects[0].begin(), m_Objects[0].end(), bind2nd(m_equalContext, pDrawableContext));
			if(removed != m_Objects[0].end()) {
				m_nCurrentGroup = 0;
				m_Objects[0].erase(removed, m_Objects[0].end());
				return;
			}
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
int CDrawableSelection::Count()
{
	return m_Objects[0].size();
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
	for(Iterator = m_Objects[0].begin(); Iterator != m_Objects[0].end(); Iterator++) {
		ResizeObject(*Iterator, m_rcSelection, rcNewBoundaries, true);
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

	if(rcNewBoundaries != m_rcSelection) m_bModified = m_bChanged = true;

	m_eCurrentState = eNone;
}
void CDrawableSelection::GetSelBounds(CRect *pRect_)
{
	GetBoundingRect(pRect_);
}

void CDrawableSelection::SetLayerSelection(int nLayer)
{
	if(m_bFloating) return;

	// first sort selection
	SortSelection();

	int nObjects = 0;
	m_nLayer = nLayer;
	vectorObject::iterator Iterator = m_Objects[0].begin();
	while(Iterator != m_Objects[0].end()) {
		if(Iterator->pContext->GetObjLayer() != m_nLayer) {
			m_bModified = m_bChanged = true;
			Iterator->pContext->SetObjLayer(m_nLayer);
			nObjects++;
		}
		Iterator++;
	}

	if(nObjects) {
		CONSOLE_DEBUG("%d objects moved to layer %d.\n", nObjects, nLayer);
	}
}

int CDrawableSelection::DeleteSelection()
{
	vectorObject::iterator Iterator = m_Objects[0].begin();
	if(m_bHoldSelection) { // if the selection is held, just remove from selection.
		while(Iterator != m_Objects[0].end()) {
			if(Iterator->bSubselected) {
				Iterator->pContext->SelectContext(false);
				Iterator = m_Objects[0].erase(Iterator);
			} else {
				Iterator++;
			}
		}
		CONSOLE_DEBUG("%d objects left in the selection.\n", m_Objects[0].size());
	} else {
		while(Iterator != m_Objects[0].end()) {
			Iterator->pContext->SelectContext(false);
			DeleteInGroups(Iterator->pContext);
			(*m_ppMainDrawable)->KillChildEx(Iterator->pContext);
			Iterator++;
		}

		m_pLastSelected = NULL;
		m_Objects[0].clear();
		if(m_bFloating) m_bFloating = false;
		else m_bModified = m_bChanged = true;
		CONSOLE_DEBUG("%d objects left.\n", (*m_ppMainDrawable)->Objects());
	}
	return m_Objects[0].size();
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
		for(Iterator = m_Objects[0].begin(); Iterator != m_Objects[0].end(); Iterator++) {
			Iterator->pContext->SelectContext(false);
		}
		m_pLastSelected = NULL;
		m_Objects[0].clear();
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
	for(Iterator = m_Objects[0].begin(); Iterator != m_Objects[0].end(); Iterator++) {
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

	if(rcNewBoundaries != m_rcSelection || m_bFloating) m_bModified = m_bChanged = true;

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

	if(bLock) {
		// deselect all:
		vectorObject::iterator Iterator;
		for(Iterator = m_Objects[0].begin(); Iterator != m_Objects[0].end(); Iterator++) {
			Iterator->pContext->SelectContext(false);
		}

		// delete items in the locked layer:
		Iterator = 
			remove_if(
				m_Objects[0].begin(), m_Objects[0].end(), 
				bind2nd(m_equalLayer, nLayer)
			);

		m_pLastSelected = NULL;
		m_Objects[0].erase(Iterator, m_Objects[0].end());

		// reselect remaining items:
		for(Iterator = m_Objects[0].begin(); Iterator != m_Objects[0].end(); Iterator++) {
			Iterator->pContext->SelectContext();
		}

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
	vectorObject::const_iterator Iterator = m_Objects[0].begin();
	while(Iterator != m_Objects[0].end()) {
		if(Iterator->pContext->isAt(point_)) return true;
		Iterator++;
	}
	return false;
}

void CDrawableSelection::SubSelPoint(bool bAdd, const CPoint &point_, int Chains)
{
	vectorObject::reverse_iterator Iterator = m_Objects[0].rbegin();
	while(Iterator != m_Objects[0].rend()) {
		if(Iterator->pContext->isAt(point_) && Iterator->bSubselected != bAdd) {
			if(Chains==0 || !bAdd) {
				if(Iterator->pContext == m_pLastSelected && !bAdd) m_pLastSelected = NULL;
				if(bAdd) m_pLastSelected = Iterator->pContext;
				Iterator->bSubselected = bAdd;
			} else if(Chains>0) {
				if(m_Objects[0].size()<=1) return; // ignore if there's only one object
				if(Iterator->eXChain<4) Iterator->eXChain = (_Chain)((int)Iterator->eXChain+1);
				else {
					Iterator->eXChain = (_Chain)0;
					if(Iterator->eYChain<4) Iterator->eYChain = (_Chain)((int)Iterator->eYChain+1);
					else Iterator->eYChain = (_Chain)0;
				}
				m_pLastSelected = Iterator->pContext;
				CONSOLE_DEBUG("XChain: %d, YChain: %d\n", (int)Iterator->eXChain, (int)Iterator->eYChain);
			} else if(Chains<0) {
				if(m_Objects[0].size()<=1) return; // ignore if there's only one object
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

		Iterator++;
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

	vectorObject::reverse_iterator Iterator = m_Objects[0].rbegin();
	while(Iterator != m_Objects[0].rend()) {
		if(Iterator->pContext->isIn(m_rcSelection)) {
			if(Iterator->pContext == m_pLastSelected && !bAdd) m_pLastSelected = NULL;
			if(bAdd) m_pLastSelected = Iterator->pContext;
			Iterator->bSubselected = bAdd;
		}
		Iterator++;
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
	int nLayer = m_Objects[0].size() ? m_nLayer : -1;
	CDrawableContext *pDrawableContext = NULL;
	(*m_ppMainDrawable)->GetFirstChildIn(m_rcSelection, &pDrawableContext);
	while(pDrawableContext) {
		int nNewLayer = pDrawableContext->GetObjLayer();
		if(!isLocked(nNewLayer) && (nNewLayer == nLayer || nLayer == -1 || m_bAllowMultiLayerSel)) {
			m_nLayer = nLayer = nNewLayer;

			vectorObject::const_iterator Iterator =
				find_if(m_Objects[0].begin(), m_Objects[0].end(), bind2nd(m_equalContext, pDrawableContext));
			if(Iterator==m_Objects[0].end()) {
				CRect Rect;
				pDrawableContext->GetRect(Rect);
				pDrawableContext->SelectContext();

				m_pLastSelected = pDrawableContext;

				m_nCurrentGroup = 0;
				m_Objects[0].push_back(SObjProp(this, pDrawableContext, Rect, relative, relative));
			}
		}
		pDrawableContext = NULL;
		(*m_ppMainDrawable)->GetNextChildIn(m_rcSelection, &pDrawableContext);
	}
	CONSOLE_DEBUG("%d of %d objects selected.\n", m_Objects[0].size(), (*m_ppMainDrawable)->Objects());
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

	int nLayer = m_Objects[0].size() ? m_nLayer : -1;
	CDrawableContext *pDrawableContext = NULL;
	(*m_ppMainDrawable)->GetFirstChildIn(m_rcSelection, &pDrawableContext);
	while(pDrawableContext) {
		int nNewLayer = pDrawableContext->GetObjLayer();
		if(!isLocked(nNewLayer) && (nNewLayer == nLayer || nLayer == -1 || m_bAllowMultiLayerSel)) {
			nLayer = nNewLayer;

			if(pDrawableContext == m_pLastSelected) m_pLastSelected = NULL;

			pDrawableContext->SelectContext(false);
			vectorObject::iterator remove = 
				remove_if(m_Objects[0].begin(), m_Objects[0].end(), bind2nd(m_equalContext, pDrawableContext));
			if(remove != m_Objects[0].end()) {
				m_nCurrentGroup = 0;
				m_Objects[0].erase(remove, m_Objects[0].end());
			}
		}
		pDrawableContext = NULL;
		(*m_ppMainDrawable)->GetNextChildIn(m_rcSelection, &pDrawableContext);
	}
}
void CDrawableSelection::CleanSelection() {
	vectorObject::iterator Iterator;
	for(Iterator = m_Objects[0].begin(); Iterator != m_Objects[0].end(); Iterator++) {
		if(!m_bHoldSelection) Iterator->pContext->SelectContext(false);
		else Iterator->bSubselected = false;
	}
	m_pLastSelected = NULL;
	if(!m_bHoldSelection) m_Objects[0].clear();
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
	for(Iterator = m_Objects[0].begin(); Iterator != m_Objects[0].end(); Iterator++) {
		Iterator->pContext->GetAbsFinalRect(RectTmp);
		if(Iterator->pContext->isAt(WorldPoint) && m_bCanMove) m_CurrentCursor = eIDC_SIZEALL; // The cursor is ovet the object.
		Rect.UnionRect(Rect, RectTmp);	// Bounding Rect
	}

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
	sort(m_Objects[0].begin(), m_Objects[0].end(), m_cmpSelection);
}

inline bool CDrawableSelection::BeginPaint(IGraphics *pGraphicsI, WORD wFlags)
{
	ASSERT(m_ppMainDrawable);
	if(!*m_ppMainDrawable) return false;

	static bool bret = false;
	static DWORD qq = GetTickCount();
	static float fAux = 1.00f;
	CSprite::SetShowOptions(wFlags);

	ARGBCOLOR argb = COLOR_ARGB((int)(255.0f*(0.5f+(fAux/2.0f))), 128, 128, 128);

	int move = (int)(400.0f - 800.0f * fAux);
	DWORD color = ::GetSysColor(COLOR_APPWORKSPACE);
	pGraphicsI->SetFilterBkColor(COLOR_ARGB(255, GetRValue(color), GetGValue(color), GetBValue(color)));
	pGraphicsI->SetFilter(Alpha, &COLOR_ARGB(255,128,128,128));

	float pixelate = 17.0f - 16.0f * fAux;
	pGraphicsI->SetFilter(Pixelate, &pixelate);
	//pGraphicsI->SetFilter(Alpha, &argb);
	//pGraphicsI->SetFilter(HorzMove, &move);
	pGraphicsI->SetClearColor((*m_ppMainDrawable)->GetBkColor());
	if(pGraphicsI->BeginPaint()) {

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
	// Get the current layer's size and draw a frame around it:
	CRect Rect;
	(*m_ppMainDrawable)->GetChild(m_nLayer)->GetRect(Rect);
	if(!Rect.IsRectEmpty()) pGraphicsI->SelectingBox(Rect, COLOR_ARGB(128, 0, 255, 0)); 

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
	for(int i=1; i<(int)m_Objects.size(); i++) {
		m_Objects[i].clear();
	}
}

int CDrawableSelection::SetNextPasteGroup(LPCSTR szGroupName, int nNew)
{
	if(nNew >= 0 && nNew < (int)m_Objects.size()) m_nPasteGroup = nNew;
	else if(nNew != -1) return m_nPasteGroup;

	for(int i=1; i<(int)m_Objects.size() && nNew==-1; i++) {
		if(m_Objects[i].empty()) nNew = i;
	}
	if(nNew == -1) {
		nNew = m_Objects.size();
		m_Objects.resize(nNew + 1);
		m_ObjectsNames.resize(nNew + 1);
	}
	ASSERT(m_Objects.size() == m_ObjectsNames.size());
	ASSERT(nNew >= 0 && nNew < (int)m_Objects.size());

	m_ObjectsNames[nNew] = szGroupName;
	m_nPasteGroup = nNew;

	return m_nPasteGroup;
}
SObjProp* CDrawableSelection::GetFirstSelection()
{
	m_CurrentSel = m_Objects[0].begin();
	if(m_CurrentSel == m_Objects[0].end()) return NULL;
	return (m_CurrentSel++).operator ->();
}

SObjProp* CDrawableSelection::GetNextSelection() 
{
	if(m_CurrentSel == NULL) return NULL;
	if(m_CurrentSel == m_Objects[0].end()) return NULL;
	return (m_CurrentSel++).operator ->();
}
