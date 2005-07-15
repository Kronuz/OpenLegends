///////////////////////////////////////////////////////////////////////////////
//
// MenuXP.h
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "TabbedMDI.h"
#include "Draw.h"

namespace WTL
{

template <class T, class TBase = CTabbedMDICommandBarCtrlImpl< T > >
class ATL_NO_VTABLE CCommandBarCtrlImplXP : public TBase 
{
public:
DECLARE_WND_SUPERCLASS(NULL, TBase::GetWndClassName())

	// Message map and handlers
	typedef CCommandBarCtrlImplXP< T, TBase > thisClass;
	typedef TBase baseClass;

    BEGIN_MSG_MAP(CCommandBarCtrlXP)
        CHAIN_MSG_MAP(baseClass)
    ALT_MSG_MAP(1)      // Parent window messages
        NOTIFY_CODE_HANDLER(NM_CUSTOMDRAW, OnCustomDraw)
        CHAIN_MSG_MAP_ALT(baseClass,1)
	ALT_MSG_MAP(2)		// MDI client window messages
		CHAIN_MSG_MAP_ALT(baseClass,2)
	ALT_MSG_MAP(3)		// Message hook messages
		CHAIN_MSG_MAP_ALT(baseClass,3)

    END_MSG_MAP()

    LRESULT OnCustomDraw (int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled)
    {		
        TCHAR sClass[128];

        GetClassName (pnmh->hwndFrom, sClass, 128);

        if ( _tcscmp (sClass, _T("WTL_CommandBarXP")) )
        {
			// Modified By Jackslack... it may affect other custom draw stuff...
			// such as toolbars...
			bHandled = FALSE;
            return CDRF_DODEFAULT;
        }
        NMCUSTOMDRAW* pCustomDraw = (NMCUSTOMDRAW*)pnmh;

        if ( pCustomDraw->dwDrawStage == CDDS_PREPAINT )
        {
            // Request prepaint notifications for each item
            return CDRF_NOTIFYITEMDRAW;
        }
        if ( pCustomDraw->dwDrawStage == CDDS_ITEMPREPAINT )
        {
            CDCHandle cDC (pCustomDraw->hdc);
            CRect rc = pCustomDraw->rc;
            TCHAR sBtnText[128];

    		::SendMessage (pnmh->hwndFrom, TB_GETBUTTONTEXT, pCustomDraw->dwItemSpec, (LPARAM)sBtnText);

            if ( pCustomDraw->uItemState & CDIS_HOT )
            {
                COLORREF crHighLight = ::GetSysColor (COLOR_HIGHLIGHT);
                CPenDC pen (cDC, crHighLight);
                CBrushDC brush (cDC, (pCustomDraw->uItemState & CDIS_SELECTED) ? HLS_TRANSFORM (crHighLight, +50, -50) : HLS_TRANSFORM (crHighLight, +70, -57));

                cDC.Rectangle (rc);
                cDC.SetTextColor ((pCustomDraw->uItemState & CDIS_SELECTED) ? ::GetSysColor (COLOR_HIGHLIGHTTEXT) : RGB(0,0,0));
            }
            else
            {
            	cDC.FillSolidRect (rc, ::GetSysColor(COLOR_MENU));
                cDC.SetTextColor (::GetSysColor (m_bParentActive ? COLOR_BTNTEXT : COLOR_3DSHADOW));
            }
            cDC.SetBkMode (TRANSPARENT);
            cDC.SelectFont ((HFONT)GetStockObject (DEFAULT_GUI_FONT));
            cDC.DrawText (sBtnText, _tcslen (sBtnText), rc, DT_CENTER|DT_VCENTER|DT_SINGLELINE);

            return CDRF_SKIPDEFAULT;
        }
        bHandled = FALSE;

        return CDRF_DODEFAULT;
    }

#define IMGPADDING 6
#define TEXTPADDING 8

// From <winuser.h>
#define OBM_CHECK 32760

	void DrawItem (LPDRAWITEMSTRUCT lpDrawItemStruct)
	{
		_MenuItemData* pmd = (_MenuItemData*)lpDrawItemStruct->itemData;
		CDCHandle dc = lpDrawItemStruct->hDC;
		const RECT& rcItem = lpDrawItemStruct->rcItem;
        LPCRECT pRect = &rcItem;
		BOOL bDisabled = lpDrawItemStruct->itemState & ODS_GRAYED;
    	BOOL bSelected = lpDrawItemStruct->itemState & ODS_SELECTED;
		BOOL bChecked = lpDrawItemStruct->itemState & ODS_CHECKED;
        COLORREF crBackImg = CLR_NONE;
        CDCHandle* pDC = &dc; 

        if ( bSelected )
        {
            COLORREF crHighLight = ::GetSysColor (COLOR_HIGHLIGHT);
            CPenDC pen (*pDC, crHighLight);
            CBrushDC brush (*pDC, crBackImg = bDisabled ? HLS_TRANSFORM (::GetSysColor (COLOR_3DFACE), +73, 0) : HLS_TRANSFORM (crHighLight, +70, -57));

            pDC->Rectangle (pRect);
        }
        else
        {
            CRect rc (pRect);

            rc.right = m_szBitmap.cx+IMGPADDING;
            pDC->FillSolidRect (rc, crBackImg = HLS_TRANSFORM (::GetSysColor (COLOR_3DFACE), +20, 0));
            rc.left = rc.right;
            rc.right = pRect->right;
            pDC->FillSolidRect (rc, HLS_TRANSFORM (::GetSysColor (COLOR_3DFACE), +75, 0));
        }
        if ( pmd->fType & MFT_SEPARATOR )
        {
            CPenDC pen (*pDC, HLS_TRANSFORM (::GetSysColor (COLOR_3DFACE), -18, 0));

            pDC->MoveTo (pRect->left+m_szBitmap.cx+IMGPADDING+TEXTPADDING,  (pRect->top+pRect->bottom)/2);
            pDC->LineTo (pRect->right-1, (pRect->top+pRect->bottom)/2);
        }
		else
		{
            CRect rc (pRect);
            CString sCaption = pmd->lpstrText;
            int nTab = sCaption.Find ('\t');

            if ( nTab >= 0 )
            {
                sCaption = sCaption.Left (nTab);
            }
            pDC->SetTextColor (bDisabled ? HLS_TRANSFORM (::GetSysColor (COLOR_3DFACE), -18, 0) : ::GetSysColor (COLOR_MENUTEXT));
            pDC->SetBkMode (TRANSPARENT);

            CBoldDC bold (*pDC, (lpDrawItemStruct->itemState & ODS_DEFAULT) != 0);

            rc.left = m_szBitmap.cx+IMGPADDING+TEXTPADDING;
            pDC->DrawText (sCaption, sCaption.GetLength(), rc, DT_SINGLELINE|DT_VCENTER|DT_LEFT);

            if ( nTab >= 0 )
            {    
                rc.right -= TEXTPADDING+4;
                pDC->DrawText (pmd->lpstrText+nTab+1, _tcslen (pmd->lpstrText+nTab+1), rc, DT_SINGLELINE|DT_VCENTER|DT_RIGHT);
            }
            if ( bChecked  )
            {
                COLORREF crHighLight = ::GetSysColor (COLOR_HIGHLIGHT);
                CPenDC pen (*pDC, crHighLight);
                CBrushDC brush (*pDC, crBackImg = bDisabled ? HLS_TRANSFORM (::GetSysColor (COLOR_3DFACE), +73, 0) :
                                                              (bSelected ? HLS_TRANSFORM (crHighLight, +50, -50) : HLS_TRANSFORM (crHighLight, +70, -57)));

                pDC->Rectangle (CRect (pRect->left+1, pRect->top+1, pRect->left+m_szButton.cx-2, pRect->bottom-1));
            }
			if ( m_hImageList != NULL && pmd->iButton >= 0 )
            {
                bool bOver = !bDisabled && bSelected;

                if ( bDisabled || (bSelected && !bChecked) )
                {
                    HICON hIcon = ImageList_ExtractIcon (NULL, m_hImageList, pmd->iButton);
                    CBrush brush;

                    brush.CreateSolidBrush (bOver ? HLS_TRANSFORM (::GetSysColor (COLOR_HIGHLIGHT), +50, -66) : HLS_TRANSFORM (::GetSysColor (COLOR_3DFACE), -27, 0));
                    pDC->DrawState (CPoint (pRect->left + ( bOver ? 4 : 3 ), rc.top + ( bOver ? 5 : 4 )),
                                    CSize (m_szBitmap.cx, m_szBitmap.cx), hIcon, DSS_MONO, brush);
                    DestroyIcon (hIcon);
                }
                if ( !bDisabled )
                {
                    ::ImageList_Draw (m_hImageList, pmd->iButton, pDC->m_hDC,
                                      pRect->left+( (bSelected && !bChecked) ? 2 : 3 ), rc.top+( (bSelected && !bChecked) ? 3 : 4 ), ILD_TRANSPARENT);
                }
            }
            else if ( bChecked )
            {
                // Draw the check mark
                rc.left  = pRect->left+5;
                rc.right = rc.left + m_szBitmap.cx+IMGPADDING;
                pDC->SetBkColor (crBackImg);
                HBITMAP hBmp = LoadBitmap (NULL, MAKEINTRESOURCE(OBM_CHECK));
                pDC->DrawState (CPoint (rc.left,rc.top+3), CSize(rc.Size()), hBmp, DSS_NORMAL, (HBRUSH)NULL);
                DeleteObject (hBmp);
            }
		}
    }

	// Edited By Jackslack 2004-1-6
	// To Support 24 Bit Bitmap for menu images.
	BOOL LoadImages(_U_STRINGorID image, UINT uBitdepth = ILC_COLOR)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		HINSTANCE hInstance = _Module.GetResourceInstance();

		HRSRC hRsrc = ::FindResource(hInstance, image.m_lpstr, (LPTSTR)RT_TOOLBAR);
		if(hRsrc == NULL)
			return FALSE;

		HGLOBAL hGlobal = ::LoadResource(hInstance, hRsrc);
		if(hGlobal == NULL)
			return FALSE;

		_ToolBarData* pData = (_ToolBarData*)::LockResource(hGlobal);
		if(pData == NULL)
			return FALSE;
		ATLASSERT(pData->wVersion == 1);

		WORD* pItems = pData->items();
		int nItems = pData->wItemCount;

		// Set internal data
		SetImageSize(pData->wWidth, pData->wHeight);

		// Create image list if needed
		if(m_hImageList == NULL)
		{
			// Check if the bitmap is 32-bit (alpha channel) bitmap (valid for Windows XP only)			
			m_bAlphaImages = AtlIsAlphaBitmapResource(image);

			// Create 24 Bit menu image.
			UINT uFlags = (m_bAlphaImages ? ILC_COLOR32 : uBitdepth) | ILC_MASK;
			m_hImageList = ::ImageList_Create(m_szBitmap.cx, m_szBitmap.cy, uFlags, pData->wItemCount, 1);
			ATLASSERT(m_hImageList != NULL);
		}

		// Add bitmap to our image list
		CBitmap bmp;
		bmp = (HBITMAP)::LoadImage(_Module.GetResourceInstance(), image.m_lpstr, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_DEFAULTSIZE);
		ATLASSERT(bmp.m_hBitmap != NULL);

		if(bmp.m_hBitmap == NULL)
			return FALSE;
		
		if(::ImageList_AddMasked(m_hImageList, bmp, m_clrMask) == -1)
			return FALSE;

		// Fill the array with command IDs
		for(int i = 0; i < nItems; i++)
		{
			if(pItems[i] != 0)
				m_arrCommand.Add(pItems[i]);
		}

		ATLASSERT(::ImageList_GetImageCount(m_hImageList) == m_arrCommand.GetSize());
		if(::ImageList_GetImageCount(m_hImageList) != m_arrCommand.GetSize())
			return FALSE;

		return TRUE;
	}
};

class CMDICommandBarCtrlXP : public CCommandBarCtrlImplXP<CMDICommandBarCtrlXP, CMDICommandBarCtrlImpl<CMDICommandBarCtrlXP> >
{
public:
DECLARE_WND_SUPERCLASS(_T("WTL_MDICommandBarXP"), GetWndClassName())
};

class CCommandBarCtrlXP : public CCommandBarCtrlImplXP<CCommandBarCtrlXP>
{
public:
    DECLARE_WND_SUPERCLASS(_T("WTL_CommandBarXP"), GetWndClassName())
};

}; //namespace WTL
