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

#include "../Core.h"
#include "../FilePath.h"

const UINT WM_POPULATE_TREE = WM_APP + 1;
const UINT WM_ITEM_SELECTED = WM_APP + 2;

// Masks for the items:
#define ITEMFILE		0x00000001	// Is the item a real file?
#define ITEMTHUMBNAIL	0x00000002	// Does the item has a thumbnail attached?
#define ITEMDATA		0x00000004	// Does the item has data attached?

enum TreeItemType { titFolder, titFile };

// Class to handle Tree Items.
// This class can save items (to file), load items, 
// rename items (and the file)
class CTreeInfo :
	public IPropertyEnabled
{
protected:
	DWORD m_dwMask;
	std::string m_sDescription;
	std::string m_sDisplayName;
	std::string m_sItemPath;

	CVFile m_vFile;

	BITMAP m_bmpThumbnail;

	size_t m_DataSize;	// DataSize is 0 if unknown (can not save)
	LPBYTE m_pRawData;
public:
	HWND m_hWndParent; // Parent Window
	HTREEITEM m_hItem; // Item handle

	TreeItemType m_eType;
	BYTE m_cSubType; // there should be one letter to identify the subtype

///////////////////////////////////////////////////////////

	virtual bool isFlagged();
	virtual void Flag(bool bFlag = true);
	virtual bool GetInfo(SInfo *pI) const;
	virtual bool GetProperties(SPropertyList *pPL) const;
	virtual bool SetProperties(SPropertyList &PL);
	virtual void Commit() const;
	virtual void Cancel();

protected:
	// finds the name, and description from the data.
	void _Data2Info(LPBYTE pRawData);
	// finds the thumbnail from the data.
	void _Data2Thumbnail();

public:
	CTreeInfo(BYTE _cSubType, const std::string _sItemPath);
	CTreeInfo(BYTE _cSubType, const std::string _sItemPath, LPBYTE _pRawData, size_t _DataSize);
	CTreeInfo(BYTE _cSubType, const std::string _sItemPath, const std::string _sFilePath);
	~CTreeInfo();

	// If the data is changed, the thumbnail becomes invalid.
	// the thumbnail's bits must point somewhere in the data.
	bool SetData(LPBYTE _pData, size_t _DataSize);
	bool SetThumbnail(BITMAP _bmpThumbnail);
	bool SetFilePath(LPCSTR _szFilePath);
	LPCSTR GetDisplayName();
	LPCSTR GetItemPath();
	const CVFile* GetFile();
	const LPBITMAP GetThumbnail();
	size_t GetDataSize();
	LPBYTE GetData();

	virtual bool Save();
	virtual bool Save(LPCSTR _szFilePath);
	virtual bool Load();
	// this will fail if any data already exists
	// this also loads the name and the description from the file header.
	virtual bool LoadThumbnail();
	virtual bool Delete();
	bool Update();
	virtual bool Rename(LPCSTR _szName);
	virtual bool MoveFile(LPCSTR _szFilePath);
};

/////////////////////////////////////////////////
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

class CWtlFileTreeCtrl : 
	public CWindowImpl< CWtlFileTreeCtrl, CTreeViewCtrl>
{
	typedef CWindowImpl< CWtlFileTreeCtrl, CTreeViewCtrl> baseClass;

public:
	CWtlFileTreeCtrl() :
		m_hImageList(NULL),
		m_bShowFiles(TRUE),
		m_hSelItem(NULL),
		m_RootItem(0)
	{
	}

	virtual ~CWtlFileTreeCtrl() {};
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
		MESSAGE_HANDLER(WM_RBUTTONDOWN, OnRButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDblClick)
		MESSAGE_HANDLER(WM_POPULATE_TREE, OnPopulateTree)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)

		REFLECTED_NOTIFY_CODE_HANDLER(TVN_ITEMEXPANDING, OnItemExpanding )
		REFLECTED_NOTIFY_CODE_HANDLER(TVN_ENDLABELEDIT, OnEndLabelEdit )
		REFLECTED_NOTIFY_CODE_HANDLER(TVN_DELETEITEM, OnDeleteItem )

		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()
	
	LRESULT OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnRButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonDblClick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPopulateTree(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnItemExpanding(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
	LRESULT OnDeleteItem(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
	LRESULT OnEndLabelEdit(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);

	void OnViewRefresh();

	HTREEITEM m_RootItem;

	HTREEITEM InsertRootItem( LPCSTR szFile, LPCSTR szPath, HTREEITEM hParent, DWORD Idx );
	HTREEITEM InsertFileItem( LPCSTR szFile, LPCSTR szPath, HTREEITEM hParent, DWORD Idx );

protected:

	void      ItemWithChildren(HTREEITEM hItem, BOOL bChildren);

	void      DisplayPath( const std::string sPath, HTREEITEM hParent, BOOL bUseSetRedraw = TRUE );
	void      DisplayDrives( HTREEITEM hParent, BOOL bUseSetRedraw = TRUE );
	int       GetIconIndex( const std::string sFilename );
	int       GetIconIndex( HTREEITEM hItem );
	int       GetSelIconIndex( const std::string sFilename );
	int       GetSelIconIndex( HTREEITEM hItem );
	BOOL      HasGotSubEntries( const std::string sDirectory );
	HTREEITEM InsertFileItem( const std::string sFile, const std::string sPath, HTREEITEM hParent, DWORD Idx=-1 );
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