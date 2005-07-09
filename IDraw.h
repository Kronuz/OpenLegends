/* QuestDesigner - Open Legends's Project
   Copyright (C) 2003-2004. Germán Méndez Bravo (Kronuz)
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

#pragma once

#include <Core.h>

#include <IGraphics.h>

interface IDrawableContext 
{
	virtual bool Draw(const IGraphics *pGraphics_) = 0; 
};

/////////////////////////////////////////////////////////////////////////////
/*! \interface	IDrawableSelection
	\brief		Drawable context interface.
	\author		Kronuz
	\version	1.0
	\date		June 28, 2003
*/
interface IDrawableSelection :
	public IDrawableContext
{
	virtual bool GetMouseStateAt(const IGraphics *pGraphics_, const CPoint &point_, CURSOR *pCursor) = 0;
	virtual void StartResizing(const CPoint &point_) = 0;
	virtual void ResizeTo(const CPoint &point_) = 0;
	virtual void EndResizing(const CPoint &point_) = 0;

	virtual void StartMoving(const CPoint &point_) = 0;
	virtual void MoveTo(const CPoint &point_) = 0;
	virtual void EndMoving(const CPoint &point_) = 0;

	virtual void StartSelBox(const CPoint &point_) = 0;
	virtual void CancelSelBox() = 0;
	virtual void SizeSelBox(const CPoint &point_) = 0;
	virtual IPropertyEnabled* EndSelBoxAdd(const CPoint &point_, int Chains) = 0;
	virtual void EndSelBoxRemove(const CPoint &point_) = 0;

	virtual void CleanSelection() = 0;

	virtual void SetSnapSize(int nSnapSize_, bool bShowGrid_) = 0;
	virtual void SetLayer(int nLayer_) = 0;
	virtual int GetLayer() = 0;

	virtual bool isResizing() = 0;
	virtual bool isMoving() = 0;
	virtual bool isSelecting() = 0;

};
