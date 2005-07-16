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
/*! \file		Memento.h 
	\author		Germán Méndez Bravo (Kronuz)
	\brief		Interface for CMemento.
	\date		July 15, 2005:
						* Creation date.

	This interface records the internal state of an object. This can be used to implement
	checkpoints for undo/redo mechanism that allow the user to back out of operations or
	recover from errors.
	The basic functionality is implemented through a set of methods that let the derivated class 
	to keep track of the state of the object at any given point in time, using checkpoints.
	For instance, when the object is first created no state has yet been saved, but as soon as
	the first checkpoint message is received, the full state of the object is saved within the
	current chekpoint mark, this can happen several times and all the states are saved in the
	current checkpoint mark, which is ideally incremented before any more changes take place;
	later when the user wants to recover a prior state in time, it sends a message to the 
	CMemento interface asking for the state of the object at a given checkpoint mark; CMemento
	resolves by replacing the actual state of the object with the saved information.

	\remarks
	It is needed, for each derived class, to implement the pure virtual functions:
		_SaveState(checkpoint);      - Saves the state of the object.
		_RestoreState(checkpoint);   - Recovers the state of an object.
		DestroyCheckpoint(data);     - Gives the object a chance to clean or delete data pointer 
		                               at the time a checkpointed state is deleted.
	For that, the implementation must use the protected members:
		SetState(checkpoint, data);  - Sets the checkpointed state to point 
		                               to a void* that keeps the state of the object.
		data GetState(checkpoint);   - Gets a pointer to a void* that keeps the state of the object.

	The Interface implements a list of chekpointed states and maintains it clean, by flushing all the
	saved states after the checkpoint indicated by SaveState. for every saved checkpoint state that is
	flushed it's data pointer is deleted.

	CMemento is basic implementation of the explained interface.
*/

#pragma once

#include <IConsole.h>

#include <Core.h>

#include <vector>
#include <functional>
#include <algorithm>

/////////////////////////////////////////////////////////////////////////////
/*! \class		CMemento
	\brief		Memento object state saving.
	\author		Germán Méndez Bravo (Kronuz)
	\version	1.0
	\date		July 15, 2005

	This class implements the basic interface to mantain the state of objects.
*/
class CMemento 
{
protected:
	struct StateData {
		virtual bool operator==(const StateData& state) const = 0;
	};
private:
	struct State {
		UINT checkpoint;
		StateData *data;
		State(UINT checkpoint_, StateData *data_) : checkpoint(checkpoint_), data(data_) {}
	};

	const struct CheckpointDestroy : 
	public std::binary_function<State*, UINT, bool> {
		CMemento *m_pMemento;
		CheckpointDestroy(CMemento *pMemento) : m_pMemento(pMemento) {	/* Warning C4355: THIS CONSTRUCTOR MUST NEVER MAKE ACCESS ANY MEMBERS OF pMemento */}
		bool operator()(const State* &a, const UINT b) const;
	} m_destroyCheckpoint;

	const struct CheckpointCmp : 
	public std::binary_function<State*, UINT, bool> {
		bool operator()(State* const &a, State* const &b) const;
	} m_cmpCheckpoint;

	typedef std::vector<State*> Checkpoints;
	Checkpoints m_Checkpoints;

	void Flush(UINT checkpoint = 0);

protected:
	int SetState(UINT checkpoint, StateData *data);
	StateData* GetState(UINT checkpoint);

	virtual int _SaveState(UINT checkpoint) = 0;
	virtual int _RestoreState(UINT checkpoint) = 0;
	// This function must ALWAYS delete the data:
	virtual void DestroyCheckpoint(StateData *data) { ASSERT(!"Abstract class call"); } //!< Finction called when a state is about to be destroyed.

	virtual void ReadState(StateData *data) = 0;
	virtual void WriteState(StateData *data) = 0;

public:
	CMemento();
	~CMemento();
};
