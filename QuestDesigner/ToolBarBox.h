#pragma once

template <class T, class TBase = CToolBarCtrl>
class ATL_NO_VTABLE CTrueColorToolBarImpl : 
	public CWindowImpl<T, TBase>
{
	bool m_bDropDown;
	CImageList m_ImageList1;
	CImageList m_ImageList2;
	CImageList m_ImageList3;

	struct stDropDownInfo {
		UINT uButtonID;
		UINT uMenuID;
		HWND hParent;
	};
	
	CSimpleArray<stDropDownInfo> m_lstDropDownButton;
public:
	DECLARE_WND_SUPERCLASS(NULL, TBase::GetWndClassName())

	CTrueColorToolBarImpl() :
		m_bDropDown(false),
		m_ImageList1(NULL),
		m_ImageList2(NULL),
		m_ImageList3(NULL)
	{
	}

	// Operations

	BOOL SubclassWindow(HWND hWnd)
	{
		ATLASSERT(m_hWnd==NULL);
		ATLASSERT(::IsWindow(hWnd));
		BOOL bRet = CWindowImpl< T, TBase >::SubclassWindow(hWnd);
		if( bRet ) _Init();
		return bRet;
	}

	BOOL LoadTrueColorToolBar(UINT uToolBar, UINT uToolBarHot=0, UINT uToolBarDisabled=0)
	{
//return FALSE;
		ATLASSERT(IS_INTRESOURCE(uToolBar));
		if(!SetTrueColorToolBar(m_ImageList1, TB_SETIMAGELIST, uToolBar))
			return FALSE;

		if(uToolBarHot) {
			ATLASSERT(IS_INTRESOURCE(uToolBarHot));
			if(!SetTrueColorToolBar(m_ImageList2, TB_SETHOTIMAGELIST, uToolBarHot))
				return FALSE;
		}

		if(uToolBarDisabled) {
			ATLASSERT(IS_INTRESOURCE(uToolBarDisabled));
			if(!SetTrueColorToolBar(m_ImageList3, TB_SETDISABLEDIMAGELIST, uToolBarDisabled))
				return FALSE;
		}

		return TRUE;
	}
	BOOL SetTrueColorToolBar(CImageList &cImageList, UINT uToolBarType, UINT uToolBar)
	{
		CImage Image;
		if(!LoadImage(&Image, _Module.GetModuleInstance(), uToolBar)) 
			return FALSE;

		int nBtnHeight = Image.GetHeight();
		int nBtnWidth = nBtnHeight;
		int	nNbBtn	= Image.GetWidth() / nBtnWidth;

		if(!cImageList.IsNull()) cImageList.Destroy();
		if(!cImageList.Create(nBtnWidth, nBtnHeight, ILC_COLOR32, nNbBtn, 0))
			return FALSE;

		if(cImageList.Add(Image) == -1)
			return FALSE;

		SendMessage(uToolBarType, 0, (LPARAM)cImageList.m_hImageList);

		return TRUE;
	}
	void AddDropDownButton(HWND hParent, UINT uButtonID, UINT uMenuID)
	{
		if(!m_bDropDown) {
			SendMessage(TB_SETEXTENDEDSTYLE, 0, (LPARAM)TBSTYLE_EX_DRAWDDARROWS);
			m_bDropDown = true;
		}

		SetButtonStyle(uButtonID, TBSTYLE_DROPDOWN);

		stDropDownInfo DropDownInfo;
		DropDownInfo.hParent	= hParent;
		DropDownInfo.uButtonID	= uButtonID;
		DropDownInfo.uMenuID	= uMenuID;
		m_lstDropDownButton.Add(DropDownInfo);
	}
	BOOL SetButtonStyle(int nID, DWORD nStyle)
	{
		TBBUTTONINFO tbbi;
		tbbi.cbSize = sizeof(TBBUTTONINFO);
		tbbi.dwMask = TBIF_STYLE;
		if(GetButtonInfo(nID, &tbbi) == -1) return FALSE;;
		tbbi.fsStyle |= nStyle;
		return SetButtonInfo(nID, &tbbi);
	}

	// Implementation
	void _Init()
	{
		ATLASSERT(::IsWindow(m_hWnd));
	}

	BEGIN_MSG_MAP(CToolBarBox)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)

		REFLECTED_NOTIFY_CODE_HANDLER(TBN_DROPDOWN, OnToolbarDropDown)

		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	LRESULT OnToolbarDropDown( int /* idCtrl */, LPNMHDR pnmh, BOOL & /* bHandled */ )
	{
		LPNMTOOLBAR lpnmtb = (LPNMTOOLBAR)pnmh;
		for(int i = 0; i < m_lstDropDownButton.GetSize(); i++) {
			stDropDownInfo DropDownInfo = m_lstDropDownButton[i];
			if (DropDownInfo.uButtonID == lpnmtb->iItem) {

				CMenu menu;
				menu.LoadMenu(DropDownInfo.uMenuID);
				CMenu Popup = menu.GetSubMenu(0);
				
				CRect rc;
				SendMessage(TB_GETRECT, (WPARAM)lpnmtb->iItem, (LPARAM)&rc);
				ClientToScreen(&rc);
				
				Popup.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_VERTICAL,
									rc.left, rc.bottom, DropDownInfo.hParent, &rc);
				break;
			}
		}

		return 0;
	}

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		LRESULT lRes = DefWindowProc(uMsg, wParam, lParam);
		_Init();
		return lRes;
	}
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		if(!m_ImageList1.IsNull()) m_ImageList1.Destroy();
		if(!m_ImageList2.IsNull()) m_ImageList2.Destroy();
		if(!m_ImageList3.IsNull()) m_ImageList3.Destroy();
		return 0;
	}
};

class CTrueColorToolBarCtrl : public CTrueColorToolBarImpl<CTrueColorToolBarCtrl>
{
public:
	DECLARE_WND_SUPERCLASS(_T("WTL_CTrueColorToolBar"), GetWndClassName())
};

