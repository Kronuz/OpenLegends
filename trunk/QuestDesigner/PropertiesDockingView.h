
#pragma once

#include <PropertyList.h>
#include "ToolBarBox.h"

class CPropertyView : 
	public CDialogImpl<CPropertyView>,
	public CDialogResize<CPropertyView>
{
	IPropertyEnabled *m_pProperty;
	int m_nMinWidth;
	int m_nMinHeight;
public:
	enum { IDD = IDD_PROPERTYVIEW };

	CToolBarBox m_ctrlToolbar;
	CPropertyListCtrl m_ctrlList;

	BEGIN_MSG_MAP(CPropertyView)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		COMMAND_CODE_HANDLER(CBN_SELCHANGE, OnSelChange)

		CHAIN_MSG_MAP( CDialogResize<CPropertyView> )
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CPropertyView)
		DLGRESIZE_CONTROL(IDC_COMBO, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_LIST, DLSZ_SIZE_X | DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_FRAME, DLSZ_SIZE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_TITLE, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_DESCRIPTION, DLSZ_SIZE_X | DLSZ_MOVE_Y)
	END_DLGRESIZE_MAP()

	LRESULT OnSelChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& bHandled)
	{
		bHandled = false;
		if(GetDlgItem(IDC_COMBO) != hWndCtl) return 0;

		CComboBox ctrlCombo = hWndCtl;
		int idx = ctrlCombo.GetCurSel();
		m_pProperty = (IPropertyEnabled *)ctrlCombo.GetItemDataPtr(idx);

		SPropertyList Properties;
		memset(&Properties, 0, sizeof(SPropertyList));
		m_pProperty->GetProperties(&Properties);
		m_ctrlList.ResetContent();
		AddProperties(&Properties);

		return 0;
	}

	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
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

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		RECT rcClient;
		GetClientRect(&rcClient);

		m_pProperty = NULL;

		m_ctrlToolbar.SubclassWindow( CFrameWindowImplBase<>::CreateSimpleToolBarCtrl(m_hWnd, IDR_TB_PROPVIEW, FALSE, ATL_SIMPLE_TOOLBAR_STYLE | CCS_NODIVIDER | CCS_NOPARENTALIGN | TBSTYLE_FLAT) );
		m_ctrlToolbar.MoveWindow(0,0, rcClient.right - rcClient.left, 10);

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
	LRESULT OnClear(WPARAM wParam, LPARAM lParam)
	{
		CComboBox ctrlCombo = GetDlgItem(IDC_COMBO);
		ctrlCombo.SetCurSel(0);
		ctrlCombo.ResetContent();
		m_ctrlList.ResetContent();
		return 0;
	}
	LRESULT OnAddInfo(WPARAM wParam, LPARAM lParam)
	{
		ASSERT(lParam);

		IPropertyEnabled *pPropObj = (IPropertyEnabled *)wParam;

		SInfo *pI = (SInfo *)lParam;
		CString sInfo;
		switch(pI->eType) {
			case itMask:			sInfo="[Mask]";			break;
			case itEntity:			sInfo="[Entity]";		break;
			case itBackground:		sInfo="[Background]";	break;
			case itMapGroup:		sInfo="[MapGroup]";		break;
			case itSound:			sInfo="[Sound]";		break;
			case itSpriteSheet:		sInfo="[SpriteSheet]";	break;
			default:				sInfo="[Unknown]";		break;
		}
		sInfo += "   ";
		sInfo += pI->szScope;
		sInfo += "::";
		if(*(pI->szName)=='\0') sInfo += "unnamed";
		else sInfo += pI->szName;

		CComboBox ctrlCombo = GetDlgItem(IDC_COMBO);
		int idx = ctrlCombo.AddString(sInfo);
		ctrlCombo.SetItemDataPtr(idx, (LPVOID)pI->pPropObject);

		if(pPropObj) {
			if(pI->pPropObject == pPropObj) 
				ctrlCombo.SetCurSel(idx);
		} else {
			ctrlCombo.SetCurSel(0);
		}
		return 0;
	}
	void AddProperties(SPropertyList *pPL)
	{
		HPROPERTY hProp = NULL;
		m_ctrlList.ResetContent();
		for(int i=0; i<pPL->nProperties; i++) {
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
			if(hProp) m_ctrlList.SetItemEnabled(hProp, pPL->aProperties[i].bEnabled);
		}
	}
	LRESULT OnSetProperties(WPARAM wParam, LPARAM lParam)
	{
		ASSERT(lParam);
		SPropertyList *pPL = (SPropertyList *)lParam;

		m_pProperty = pPL->Information.pPropObject;

		AddProperties(pPL);

		return 0;
	}
	LRESULT OnUpdate(WPARAM wParam, LPARAM lParam)
	{
		SPropertyList Properties;
		memset(&Properties, 0, sizeof(SPropertyList));
		m_pProperty->GetProperties(&Properties);
		m_ctrlList.ResetContent();
		AddProperties(&Properties);
		return 0;
	}

};

