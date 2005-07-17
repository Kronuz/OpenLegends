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
/*! \file		Memento.cpp
	\author		Germán Méndez Bravo (Kronuz)
	\brief		Implementation of the Memento pattern class.
	\date		July 15, 2005:
					* Creation date.
	\remarks	This file implements the classes to handle object state saving 
				using checkpoints.
*/

#include "stdafx.h"
#include "Memento.h"

inline bool CMemento::CheckpointDestroy::operator()(const State* &a, const UINT b) const
{
	ASSERT(a);
	ASSERT(m_pMemento);
	ASSERT(a->checkpoint > b);
	if(a->checkpoint > b) {
		ASSERT(m_pMemento->m__DestroyCheckpoint || !a->data);
		if(m_pMemento->m__DestroyCheckpoint && a->data) {
			// callback the destroyer for the checkpoint data:
			m_pMemento->m__DestroyCheckpoint(a->data, m_pMemento->m__lParam);
		}
		delete a; a = NULL;
		return true;
	}
	return false;
}

inline bool CMemento::CheckpointCmp::operator()( State* const &a, State* const &b) const
{
	ASSERT(a && b);
	return(a->checkpoint < b->checkpoint);
}

#pragma warning (push)
#pragma warning(disable : 4355) // ignore the C4355 warning
CMemento::CMemento() :
	m__DestroyCheckpoint(NULL),
	m__lParam(0),
	m_destroyCheckpoint(this)
{
}
#pragma warning (pop)

CMemento::~CMemento()
{
	BEGIN_DESTRUCTOR
	Flush();
	END_DESTRUCTOR
}

void CMemento::Flush(UINT checkpoint)
{
	Checkpoints::iterator Iterator =
		upper_bound(m_Checkpoints.begin(), m_Checkpoints.end(), &State(checkpoint, NULL), m_cmpCheckpoint);
	for_each(Iterator, m_Checkpoints.end(), bind2nd(m_destroyCheckpoint, checkpoint));
	m_Checkpoints.erase(Iterator, m_Checkpoints.end());
}

int CMemento::SetState(UINT checkpoint, StateData *data)
{
	// First delete all the objects that are larger than the chekpoint to set:
	Flush(checkpoint);
	// Get the current state for the given checkpoint:
	StateData *actual = GetState(checkpoint);
	if(actual) {
		// If the current saved state is equal to the new state, then abort the state saving:
		if(actual == data) {
			ASSERT(m__DestroyCheckpoint || !data);
			if(m__DestroyCheckpoint && data) {
				// callback the destroyer for the checkpoint data:
				m__DestroyCheckpoint(data, m__lParam);
			}
			return 0;
		}
	}
	// Finally add the new state to the vector:
	m_Checkpoints.push_back(new State(checkpoint, data));
	return 1;
}

CMemento::StateData* CMemento::GetState(UINT checkpoint)
{
	Checkpoints::iterator Iterator =
		upper_bound(m_Checkpoints.begin(), m_Checkpoints.end(), &State(checkpoint, NULL), m_cmpCheckpoint);

	if(Iterator != m_Checkpoints.begin()) {
		Iterator--;
		if(*Iterator) return (*Iterator)->data;
	}
	return NULL;
}

int CMemento::StateCount(UINT checkpoint)
{
	Checkpoints::iterator Iterator =
		upper_bound(m_Checkpoints.begin(), m_Checkpoints.end(), &State(checkpoint, NULL), m_cmpCheckpoint);
	return distance(m_Checkpoints.begin(), Iterator);
}
