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
// PropertiesDockingView.cpp : implementation of the properties view dialog class
// Todo: multiple selection in the combo box and multiple edition in the properties.

#include "stdafx.h"

#include "PropertiesDockingView.h"

PROPDESC PropDesc[] = {
{ "Description",		IDS_DESC_DESC },
{ "Appearance",			IDS_DESC_EMPTY },
	{ "Name",				IDS_DESC_NAME },
	{ "X",					IDS_DESC_X },
	{ "Y",					IDS_DESC_Y },
	{ "Width",				IDS_DESC_WIDTH },
	{ "Height",				IDS_DESC_HEIGHT },
	{ "IsMirrored",			IDS_DESC_ISMIRRORED },
	{ "IsFlipped",			IDS_DESC_ISFLIPPED },
	{ "Rotation",			IDS_DESC_ROTATION },
	{ "Alpha",				IDS_DESC_ALPHA },
{ "Misc",				IDS_DESC_EMPTY },
	{ "Layer",				IDS_DESC_LAYER },
	{ "SubLayer",			IDS_DESC_SUBLAYER },
	{ "Sprite Sheet",		IDS_DESC_SPRITESHEET },
{ "Behavior",			IDS_DESC_EMPTY },
	{ "IsVisible",			IDS_DESC_ISVISIBLE },
	{ "Vertical Chain",		IDS_DESC_VCHAIN },
	{ "Horizontal Chain",	IDS_DESC_HCHAIN }
};

LRESULT CPropertyView::OnItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
	LPNMPROPERTYITEM selectioninfo = (LPNMPROPERTYITEM)pnmh;

	if(!selectioninfo->prop) return 0;

	LPCSTR Name = selectioninfo->prop->GetName();
	SProperty *pP = m_PropertyList.FindProperty(Name);
	ASSERT(pP);
	if(!pP) return 0;

	VARIANT value;
	memset(&value,0,sizeof(VARIANT));
	value.vt = VT_I4;
	if(pP->eType == SProperty::ptString) value.vt = VT_BSTR;
	if(!selectioninfo->prop->GetValue(&value)) return 0;

	switch(pP->eType) {
		case SProperty::ptString: 
			strncpy(pP->szString, COLE2CT(value.bstrVal), pP->uMDL-1);
			pP->bChanged = true;
			break;
		case SProperty::ptValue: 
			pP->nValue = value.intVal;
			pP->bChanged = true;
			break;
		case SProperty::ptBoolean: 
			pP->bBoolean = !(value.boolVal==FALSE);
			pP->bChanged = true;
			break;
		case SProperty::ptList: 
			pP->nIndex = value.intVal;
			pP->bChanged = true;
			break;
	}

	CONSOLE_DEBUG("Property changed, updating...\n");
	int idx = m_ctrlComboBox.GetFirstSel();
	int items = 0;
	while(idx != -1) {
		IPropertyEnabled *pProperty = (IPropertyEnabled *)m_ctrlComboBox.GetItemDataPtr(idx);
		if((int)pProperty == -1) return 0;

		if(pProperty->SetProperties(m_PropertyList)) items++;
		
		idx = m_ctrlComboBox.GetNextSel();
	}
	m_PropertyList.Touch(); // cleans all changed flags in the property list.

	CONSOLE_DEBUG("%d items changed.\n", items);

	return 0;
}
LRESULT CPropertyView::OnSelChanged(int idCtrl, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
	LPNMPROPERTYITEM selectioninfo = (LPNMPROPERTYITEM)pnmh;

	if(!selectioninfo->prop) return 0;

	LPCSTR Name = selectioninfo->prop->GetName();

	UINT uID = IDS_DESC_EMPTY;
	for(int i=0; i<sizeof(PropDesc)/sizeof(PROPDESC); i++) {
		if(!stricmp(PropDesc[i].Name, Name)) {
			uID = PropDesc[i].uID;
			break;
		}
	}

	char szBuffer[500];
	::LoadStringA(_Module.GetResourceInstance(), uID, szBuffer, sizeof(szBuffer));
	SetDlgItemText(IDC_TITLE, Name);
	SetDlgItemText(IDC_DESCRIPTION, szBuffer);
	return 0;
}

// Combo box:
LRESULT CPropertyView::OnSelChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& bHandled)
{
	bHandled = FALSE;
	ASSERT(GetDlgItem(IDC_COMBO) == hWndCtl);

	return OnSetProperties();
}

LRESULT CPropertyView::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CRect Rect;
	GetClientRect(&Rect);
	int w = Rect.Width() - m_nMinWidth;
	int h = Rect.Height() - m_nMinHeight;
	if(w < 0 || h < 0) {
		if(w < 0 ) Rect.right -= w;
		if(h < 0 ) Rect.bottom -= h;
		MoveWindow(&Rect);
		return 1;
	}
	bHandled = FALSE;
	return 0;
}

LRESULT CPropertyView::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	RECT rcClient;
	GetClientRect(&rcClient);

	m_ctrlToolbar.SubclassWindow( CFrameWindowImplBase<>::CreateSimpleToolBarCtrl(m_hWnd, IDR_TB_PROPVIEW, FALSE, ATL_SIMPLE_TOOLBAR_STYLE | CCS_NODIVIDER | CCS_NOPARENTALIGN | TBSTYLE_FLAT) );
	m_ctrlToolbar.MoveWindow(0,0, rcClient.right - rcClient.left, 10);
	m_ctrlComboBox.SubclassWindow(GetDlgItem(IDC_COMBO));

	m_ctrlComboBox.SetMultiSelection();
	m_ctrlComboBox.SetStatesBitmap(IDB_COMBO_ICONS, 16);
	m_ctrlComboBox.AddIcon("Icon");

	static CFont font;
	CLogFont lf = GetFont();
	lf.SetBold();
	font.CreateFontIndirect(&lf);
	CStatic ctrlTitle = GetDlgItem(IDC_TITLE);
	ctrlTitle.SetFont(font);

	m_ctrlList.SubclassWindow(GetDlgItem(IDC_LIST));
	m_ctrlList.SetExtendedListStyle(PLS_EX_CATEGORIZED | PLS_EX_XPLOOK);

	// Set the minimum width and height of the content
	m_nMinWidth = 100;
	m_nMinHeight = 120;

	DlgResize_Init(false, true, WS_CLIPCHILDREN);
	return TRUE;
}
LRESULT CPropertyView::OnClear(WPARAM wParam, LPARAM lParam)
{
	m_ctrlComboBox.SetCurSel(0);
	m_ctrlComboBox.ResetContent();
	m_ctrlList.ResetContent();

	char szBuffer[500];
	::LoadStringA(_Module.GetResourceInstance(), IDS_DESC_DESC, szBuffer, sizeof(szBuffer));
	SetDlgItemText(IDC_TITLE, "Description");
	SetDlgItemText(IDC_DESCRIPTION, szBuffer);

	return 0;
}

void CPropertyView::AddProperties(SPropertyList *pPL)
{
	HPROPERTY hProp = NULL;
	for(int i=0; i<pPL->nProperties; i++) {
		hProp = m_ctrlList.FindProperty(pPL->aProperties[i].szPropName);
		if(hProp) {
			if(pPL->aProperties[i].eType != SProperty::ptCategory) {
				VARIANT value;
				memset(&value,0,sizeof(VARIANT));

				if(pPL->aProperties[i].eType == SProperty::ptString) {
					value.vt = VT_LPSTR;
					value.pcVal = pPL->aProperties[i].szString;
					m_ctrlList.SetItemValue(hProp, &value);
				} else if(pPL->aProperties[i].eType == SProperty::ptValue) {
					value.vt = VT_I4;
					value.intVal = pPL->aProperties[i].nValue;
					m_ctrlList.SetItemValue(hProp, &value);
				} else if(pPL->aProperties[i].eType == SProperty::ptBoolean) {
					value.vt = VT_BOOL;
					value.boolVal = pPL->aProperties[i].bBoolean;
					m_ctrlList.SetItemValue(hProp, &value);
				} else if(pPL->aProperties[i].eType == SProperty::ptList) {
					value.vt = VT_I4;
					value.intVal = pPL->aProperties[i].nIndex;
					m_ctrlList.SetItemValue(hProp, &value);
				}
			}
			if(hProp) {
				m_ctrlList.SetItemEnabled(hProp, pPL->aProperties[i].bEnabled);
				m_ctrlList.SetItemMultivalue(hProp, pPL->aProperties[i].bMultivalue);
			}
		} else {
			if(pPL->aProperties[i].eType == SProperty::ptCategory) {
				hProp = m_ctrlList.AddItem( PropCreateCategory(pPL->aProperties[i].szPropName) );
			} else if(pPL->aProperties[i].eType == SProperty::ptString) {
				hProp = m_ctrlList.AddItem( PropCreateSimple(pPL->aProperties[i].szPropName, pPL->aProperties[i].szString) );
			} else if(pPL->aProperties[i].eType == SProperty::ptValue) {
				hProp = m_ctrlList.AddItem( PropCreateSimple(pPL->aProperties[i].szPropName, pPL->aProperties[i].nValue) );
			} else if(pPL->aProperties[i].eType == SProperty::ptBoolean) {
				hProp = m_ctrlList.AddItem( PropCreateSimple(pPL->aProperties[i].szPropName, pPL->aProperties[i].bBoolean) );
			} else if(pPL->aProperties[i].eType == SProperty::ptList) {
				hProp = m_ctrlList.AddItem( PropCreateList( pPL->aProperties[i].szPropName, pPL->aProperties[i].List, pPL->aProperties[i].nIndex) );
			}
			if(hProp) {
				m_ctrlList.SetItemEnabled(hProp, pPL->aProperties[i].bEnabled);
				m_ctrlList.SetItemMultivalue(hProp, pPL->aProperties[i].bMultivalue);
			}
		}
	}
	memset(&m_PropertyList, 0, sizeof(SPropertyList));
	m_PropertyList.Merge(pPL); // save the properties for later.
}

LRESULT CPropertyView::OnAddInfo(WPARAM wParam, LPARAM lParam)
{
	ASSERT(lParam);

	IPropertyEnabled *pPropObj = (IPropertyEnabled *)wParam;
	SInfo *pI = (SInfo *)lParam;

	CString sInfo;

	if(*(pI->szName)=='\0' || *(pI->szName)=='*' ) sInfo = "unnamed";
	else sInfo = pI->szName;

	sInfo += ": ";
	sInfo += pI->szScope;
	sInfo += " ";

	int nIconIndex = 0;
	switch(pI->eType) {
		case itMask:
			sInfo += "(Mask)";
			break;
		case itEntity:
			sInfo += "(Entity)";
			nIconIndex = 5;
			break;
		case itBackground:
			sInfo += "(Background)";
			nIconIndex = 6;
			break;
		case itMapGroup:
			sInfo += "(MapGroup)";
			nIconIndex = 7;
			break;
		case itSound:
			sInfo += "(Sound)";
			nIconIndex = 8;
			break;
		case itSpriteSheet:
			sInfo += "(SpriteSheet)";
			break;
		default:
			sInfo += "(Unknown)";
			break;
	}

	int idx = m_ctrlComboBox.AddString(sInfo);
	m_ctrlComboBox.SetItemDataPtr(idx, (LPVOID)pI->pPropObject);
	m_ctrlComboBox.SetItemIcon(idx, "Icon", nIconIndex);

	if(pPropObj) {
		if(pI->pPropObject == pPropObj) 
			m_ctrlComboBox.SetCurSel(idx);
	} else {
		m_ctrlComboBox.SetSel(idx, pI->pPropObject->isFlagged());
	}
	return 0;
}
LRESULT CPropertyView::OnSetProperties(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	m_ctrlList.ResetContent();

	SPropertyList FinalProperties;
	memset(&FinalProperties, 0, sizeof(SPropertyList));

	for(int idx = 0; idx < m_ctrlComboBox.GetCount(); idx++) {
		IPropertyEnabled *pProperty = (IPropertyEnabled *)m_ctrlComboBox.GetItemDataPtr(idx);
		ASSERT((int)pProperty != -1);
		if((int)pProperty == -1) return 0;

		if(m_ctrlComboBox.GetSel(idx) == TRUE) {
			SPropertyList Properties;
			memset(&Properties, 0, sizeof(SPropertyList));
			pProperty->GetProperties(&Properties);
			if(!FinalProperties.Merge(&Properties)) break;
			pProperty->Flag(true);
		} else {
			pProperty->Flag(false);
		}
	}

	if(FinalProperties.nProperties == 0) m_ctrlList.ResetContent();
	AddProperties(&FinalProperties);

	BOOL bDummy;
	m_ctrlComboBox.OnSelChange(0,0,0,bDummy);

	return 0;
}

LRESULT CPropertyView::OnUpdate(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	SPropertyList FinalProperties;
	memset(&FinalProperties, 0, sizeof(SPropertyList));

	for(int idx = 0; idx < m_ctrlComboBox.GetCount(); idx++) {
		IPropertyEnabled *pProperty = (IPropertyEnabled *)m_ctrlComboBox.GetItemDataPtr(idx);
		ASSERT((int)pProperty != -1);
		if((int)pProperty == -1) return 0;
		if(pProperty->isFlagged()) {
			SPropertyList Properties;
			memset(&Properties, 0, sizeof(SPropertyList));
			pProperty->GetProperties(&Properties);
			if(!FinalProperties.Merge(&Properties)) break;
			m_ctrlComboBox.SetSel(idx, TRUE);
		} else {
			m_ctrlComboBox.SetSel(idx, FALSE);
		}
	}

	if(FinalProperties.nProperties == 0) m_ctrlList.ResetContent();
	AddProperties(&FinalProperties);

	BOOL bDummy;
	m_ctrlComboBox.OnSelChange(0,0,0,bDummy);

	return 0;
}
