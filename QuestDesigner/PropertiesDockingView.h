
#pragma once

#include <PropertyList.h>
#include "ToolBarBox.h"

class CPropertyView : 
   public CDialogImpl<CPropertyView>,
   public CDialogResize<CPropertyView>
{
public:
   enum { IDD = IDD_PROPERTYVIEW };

   CToolBarBox m_ctrlToolbar;
   CPropertyListCtrl m_ctrlList;

   BEGIN_MSG_MAP(CPropertyView)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
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

   LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
   {
      RECT rcClient;
      GetClientRect(&rcClient);

      CComboBox ctrlCombo = GetDlgItem(IDC_COMBO);
	  ctrlCombo.AddString(_T("_crabenemy::enemy1"));
      ctrlCombo.SetCurSel(0);

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
      m_ctrlList.AddItem( PropCreateCategory(_T("Appearance")) );
      m_ctrlList.AddItem( PropCreateSimple(_T("Name"), _T("enemy1")) );
      m_ctrlList.AddItem( PropCreateSimple(_T("X"), 123L) );
      m_ctrlList.AddItem( PropCreateSimple(_T("Y"), 456L) );
      m_ctrlList.AddItem( PropCreateSimple(_T("Width"), 32L) );
      m_ctrlList.AddItem( PropCreateSimple(_T("Height"), 32L) );
      m_ctrlList.AddItem( PropCreateSimple(_T("IsMirrored"), false) );
      m_ctrlList.AddItem( PropCreateSimple(_T("IsFlipped"), false) );

	  m_ctrlList.AddItem( PropCreateCategory(_T("Behaviour")) );
      m_ctrlList.AddItem( PropCreateSimple(_T("Horizontal Chain"), _T("none")) );
      m_ctrlList.AddItem( PropCreateSimple(_T("Vertical Chain"), _T("Stretch")) );
      m_ctrlList.AddItem( PropCreateSimple(_T("IsVisible"), true) );

	  m_ctrlList.AddItem( PropCreateCategory(_T("Misc")) );
      m_ctrlList.AddItem( PropCreateSimple(_T("Sprite Sheet"), _T("Enemy Sheets")) );
      m_ctrlList.AddItem( PropCreateFileName(_T("Sound"), _T("C:\\Temp\\crab.wav")) );

      DlgResize_Init(false, true, WS_CLIPCHILDREN);
      return TRUE;
   }
};

class CPropertiesDockingView : 
	public dockwins::CTitleDockingWindowImpl<CPropertiesDockingView,CWindow,dockwins::COutlookLikeTitleDockingWindowTraits>
{
	typedef CPropertiesDockingView thisClass;
	typedef dockwins::CTitleDockingWindowImpl<CPropertiesDockingView, CWindow,dockwins::COutlookLikeTitleDockingWindowTraits> baseClass;

	CPropertyView m_PropertyView;

public:
    DECLARE_WND_CLASS(_T("CPropertiesDockingView"))
	BEGIN_MSG_MAP(thisClass)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		CHAIN_MSG_MAP(baseClass)
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
      m_PropertyView.Create(m_hWnd);
      m_PropertyView.SetWindowText(_T("Property View"));
	  return 0;
	}
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		CRect Rect;
		GetClientRect(&Rect);
		if(Rect.Width()<100) Rect.right = Rect.left + 100;
		if(Rect.Height()<120) Rect.bottom = Rect.top + 120;

		m_PropertyView.SetWindowPos(NULL, &Rect, SWP_NOZORDER | SWP_NOACTIVATE);
		return 0;
	}

};
