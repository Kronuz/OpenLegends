/*
Module : WtlFileTreeCtrl.h
Purpose: Interface for an WTL class which provides a tree control similiar 
         to the left hand side of explorer

Copyright (c) 2003 by iLYA Solnyshkin. 
E-mail: isolnyshkin@yahoo.com 
All rights reserved.

Modified and improved by Kronuz. July 29, 2003
*/
#if !defined( WTL_FILETREECTRL_H__)
#define WTL_FILETREECTRL_H__

//Class which encapsulates access to the System image list which contains
//all the icons used by the shell to represent the file system

#include <string>
#include <algorithm>
#include <vector>
#include <map>

#include <atlctrls.h>
#include <atlctrlx.h>
#include <atlmisc.h>
#include <shlobj.h>

const UINT WM_POPULATE_TREE = WM_APP + 1;
const UINT WM_ITEM_SELECTED = WM_APP + 2;

class CSystemImageList
{
public:
	CSystemImageList();
	~CSystemImageList();

	//Methods
	CImageList* GetImageList();

protected:
	CImageList m_ImageList;
	static int m_nRefCount;
};

class CWtlFileTreeCtrl : public CWindowImpl< CWtlFileTreeCtrl, CTreeViewCtrl>			                   
{
public:
	CWtlFileTreeCtrl::CWtlFileTreeCtrl() :
		m_hImageList(NULL),
		m_bShowFiles(TRUE),
		m_hSelItem(NULL),
		m_RootItem(TVI_ROOT)
	{
	}

	virtual ~CWtlFileTreeCtrl(){};
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL SubclassWindow( HWND hWnd );

// Public methods
	void		SetRootFolder( const std::string sPath );
	std::string GetRootFolder() const { return m_sRootFolder; };
	std::string	ItemToPath( HTREEITEM hItem, BOOL bVF=TRUE );
	std::string	GetSelectedPath(BOOL bVF=TRUE);
	HTREEITEM	SetSelectedPath( const std::string sPath, BOOL bExpanded = FALSE );
	void		SetShowFiles( BOOL bFiles );
	BOOL		GetShowFiles() const { return m_bShowFiles; };

	void UseImageList(HIMAGELIST hImageList) { 
		m_hImageList = hImageList; 
		SetImageList( m_hImageList, TVSIL_NORMAL );
	}
	void AddFileType(const std::string sExt, DWORD Idx) { 
		m_ImageIndex.insert(std::pair<std::string, DWORD>(sExt, Idx)); 
	}

	DECLARE_WND_SUPERCLASS(NULL, CTreeViewCtrl::GetWndClassName())
	BEGIN_MSG_MAP(CWtlFileTreeCtrl)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDblClick)
		MESSAGE_HANDLER(WM_POPULATE_TREE, OnPopulateTree)

		REFLECTED_NOTIFY_CODE_HANDLER(TVN_ITEMEXPANDING, OnItemExpanding )

		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()
	
	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonDblClick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPopulateTree(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnItemExpanding(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);

	void OnViewRefresh();

	HTREEITEM m_RootItem;

protected:

	void      ItemWithChildren(HTREEITEM hItem, BOOL bChildren);

	void      DisplayPath( const std::string sPath, HTREEITEM hParent, BOOL bUseSetRedraw = TRUE );
	void      DisplayDrives( HTREEITEM hParent, BOOL bUseSetRedraw = TRUE );
	int       GetIconIndex( const std::string sFilename );
	int       GetIconIndex( HTREEITEM hItem );
	int       GetSelIconIndex( const std::string sFilename );
	int       GetSelIconIndex( HTREEITEM hItem );
	BOOL      HasGotSubEntries( const std::string sDirectory );
	HTREEITEM InsertFileItem( const std::string sFile, const std::string sPath, HTREEITEM hParent );
	HTREEITEM FindSibling( HTREEITEM hParent, const std::string sItem );
	static int CompareByFilenameNoCase( std::string element1, std::string element2 );

	std::map<std::string, DWORD> m_ImageIndex;
	typedef std::map<std::string, DWORD> ImageIndex;


	HIMAGELIST		 m_hImageList;
	CSystemImageList m_SysImageList;
	std::string      m_sRootFolder;
	BOOL             m_bShowFiles;
	HTREEITEM        m_hSelItem;
};

#endif //WTL_FILETREECTRL_H__