
#pragma once

class CPropertiesDockingView : 
	public dockwins::CTitleDockingWindowImpl<CPropertiesDockingView,CWindow,dockwins::COutlookLikeTitleDockingWindowTraits>
{
	typedef CPropertiesDockingView thisClass;
	typedef dockwins::CTitleDockingWindowImpl<CPropertiesDockingView, CWindow,dockwins::COutlookLikeTitleDockingWindowTraits> baseClass;
public:
    DECLARE_WND_CLASS(_T("CPropertiesDockingView"))
	BEGIN_MSG_MAP(thisClass)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		CHAIN_MSG_MAP(baseClass)
	END_MSG_MAP()

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		CPaintDC dc(m_hWnd);
		RECT rc;
		GetClientRect(&rc);
		dc.FillRect(&rc, (HBRUSH)LongToPtr(COLOR_WINDOW + 1));
		dc.SetBkMode(TRANSPARENT);
		dc.DrawText(_T("Properties under construction :P"),-1,&rc, DT_END_ELLIPSIS | DT_CENTER | DT_VCENTER | DT_SINGLELINE );			

		return 0;
	}
};
