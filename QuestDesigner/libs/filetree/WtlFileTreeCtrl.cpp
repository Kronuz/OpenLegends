/*
Module : WtlFileTreeCtrl.cpp
Purpose: Interface for an WTL class which provides a tree control similiar 
         to the left hand side of explorer

Copyright (c) 2003 by iLYA Solnyshkin. 
E-mail: isolnyshkin@yahoo.com 
All rights reserved.

Modified and improved by Kronuz. July 29, 2003
*/

#include "stdafx.h"
#include "WtlFileTreeCtrl.h"
#include "FileFind.h"

#define MIN_SHELL_ID 1
#define MAX_SHELL_ID 30000

int CSystemImageList::m_nRefCount = 0;

////////////////////////////// Implementation of CTreeInfo /////////////////////////////////
bool CTreeInfo::isFlagged() 
{ 
	return false; 
}
void CTreeInfo::Flag(bool bFlag) 
{ 
}
bool CTreeInfo::GetInfo(SInfo *pI) const 
{ 
	return false; 
}
bool CTreeInfo::GetProperties(SPropertyList *pPL) const 
{ 
	return false; 
}
bool CTreeInfo::SetProperties(SPropertyList &PL) 
{ 
	return false; 
}
void CTreeInfo::Commit() const 
{
}
void CTreeInfo::Cancel() 
{
}

void CTreeInfo::_Data2Info(LPBYTE pRawData)
{
	ATLASSERT(pRawData != NULL);
	if(pRawData == NULL) return;
	LPCOZFILE pOZFile = (LPCOZFILE)pRawData;
	if(!VerifyOZFile(&pOZFile)) return;

	// Find name and description:
	char tmp[sizeof(pOZFile->ID)];
	strcpy(tmp, pOZFile->ID);
	strtok(tmp, "\n");						// signature
	LPSTR name = strtok(NULL, "\n");		// name
	LPSTR desc = name + strlen(name) + 1;	// description

	if(name) m_sDisplayName = name;
	if(desc) m_sDescription = desc;
	m_eType = titFile;
}
void CTreeInfo::_Data2Thumbnail()
{
	ATLASSERT(m_pRawData != NULL);
	if(m_pRawData == NULL) return;
	LPCOZFILE pOZFile = (LPCOZFILE)m_pRawData;
	if(!VerifyOZFile(&pOZFile)) return;

	// Load the thumbnail:
	if(pOZFile->dwBitmapOffset) {
		LPBITMAP pBitmap = (LPBITMAP)(m_pRawData + pOZFile->dwBitmapOffset);
		pBitmap->bmBits = (LPVOID)((LPBYTE)pBitmap + sizeof(BITMAP));
		m_bmpThumbnail = *pBitmap;
		m_dwMask |= ITEMTHUMBNAIL;
		m_eType = titFile;
	}
}

CTreeInfo::CTreeInfo(BYTE _cSubType, const std::string _sItemPath) :
	m_eType(titFolder),
	m_cSubType(_cSubType),
	m_hItem(NULL),
	m_hWndParent(NULL),
	m_dwMask(0),
	m_DataSize(0),
	m_pRawData(NULL)
{
	memset(&m_bmpThumbnail, 0, sizeof(BITMAP));
	int idx = _sItemPath.rfind('\\');
	if(idx != std::string::npos) {
		m_sItemPath = _sItemPath.substr(0, idx);
		m_sDisplayName = _sItemPath.substr(idx+1);
	} else {
		m_sDisplayName = _sItemPath;
	}
}
CTreeInfo::CTreeInfo(BYTE _cSubType, const std::string _sItemPath, LPBYTE _pRawData, size_t _DataSize) :
	m_eType(titFolder),
	m_cSubType(_cSubType),
	m_hItem(NULL),
	m_hWndParent(NULL),
	m_dwMask(0),
	m_DataSize(0),
	m_pRawData(NULL)
{
	memset(&m_bmpThumbnail, 0, sizeof(BITMAP));
	int idx = _sItemPath.rfind('\\');
	if(idx != std::string::npos) {
		m_sItemPath = _sItemPath.substr(0, idx);
		m_sDisplayName = _sItemPath.substr(idx+1);
	} else {
		m_sDisplayName = _sItemPath;
	}

	SetData(_pRawData, _DataSize);
	_Data2Info(m_pRawData);
	_Data2Thumbnail();
}
CTreeInfo::CTreeInfo(BYTE _cSubType, const std::string _sItemPath, const std::string _sFilePath) :
	m_eType(titFolder),
	m_cSubType(_cSubType),
	m_hItem(NULL),
	m_hWndParent(NULL),
	m_dwMask(0),
	m_DataSize(0),
	m_pRawData(NULL)
{
	SetFilePath(_sFilePath.c_str());

	memset(&m_bmpThumbnail, 0, sizeof(BITMAP));
	int idx = _sItemPath.rfind('\\');
	if(idx != std::string::npos) {
		m_sItemPath = _sItemPath.substr(0, idx);
		m_sDisplayName = _sItemPath.substr(idx+1);
	} else {
		m_sDisplayName = _sItemPath;
	}

	m_vFile.SetFileDesc(m_sDisplayName.c_str());
}
CTreeInfo::~CTreeInfo()
{
	delete []m_pRawData;
}

bool CTreeInfo::SetData(LPBYTE _pData, size_t _DataSize) 
{
	delete []m_pRawData; // delete current data (if any)
	m_pRawData = _pData;
	m_DataSize = _DataSize;
	m_dwMask |= ITEMDATA;
	m_dwMask &= ~ITEMTHUMBNAIL;

	return true;
}
bool CTreeInfo::SetThumbnail(BITMAP _bmpThumbnail) 
{
	if((m_dwMask & ITEMDATA) != ITEMDATA) return false;
	m_bmpThumbnail = _bmpThumbnail;
	m_dwMask |= ITEMTHUMBNAIL;
	return true;
}
bool CTreeInfo::SetFilePath(LPCSTR _szFilePath) 
{
	ATLASSERT(_szFilePath);
	if(!_szFilePath) return false;
	if(!*_szFilePath) return false;

	m_vFile.SetFilePath(_szFilePath);
	m_sDisplayName = m_vFile.GetFileDesc();
	m_dwMask |= ITEMFILE;
	return true;
}
LPCSTR CTreeInfo::GetDisplayName() 
{
	return m_sDisplayName.c_str();
}
LPCSTR CTreeInfo::GetItemPath() 
{
	return m_sItemPath.c_str();
}
const CVFile* CTreeInfo::GetFile() 
{
	if((m_dwMask & ITEMFILE) != ITEMFILE) return NULL;
	return &m_vFile;
}
const LPBITMAP CTreeInfo::GetThumbnail() 
{
	if((m_dwMask & ITEMTHUMBNAIL) != ITEMTHUMBNAIL) return NULL;
	return &m_bmpThumbnail;
}
size_t CTreeInfo::GetDataSize() 
{
	if((m_dwMask & ITEMDATA) != ITEMDATA) return 0;
	return m_DataSize;
}
LPBYTE CTreeInfo::GetData() 
{
	if((m_dwMask & ITEMDATA) != ITEMDATA) return NULL;
	return m_pRawData;
}
bool CTreeInfo::Save() 
{
	if((m_dwMask & ITEMFILE) != ITEMFILE) return false;
	if(m_DataSize == 0) return false;

	if(m_vFile.Write(m_pRawData, m_DataSize) != m_DataSize) return false;

	m_eType = titFile;
	return true;
}
bool CTreeInfo::Save(LPCSTR _szFilePath) 
{
	ATLASSERT(_szFilePath);
	if(!_szFilePath) return false;
	if(!*_szFilePath) return false;
	
	m_dwMask |= ITEMFILE;
	m_vFile.SetFilePath(_szFilePath);
	return Save();
}
bool CTreeInfo::Load() 
{
	if((m_dwMask & ITEMFILE) != ITEMFILE) return false;
	if(m_vFile.Open("r")) {
		m_eType = titFile;
		int size = m_vFile.GetFileSize();
		if(size) {
			LPBYTE pBuffer = new BYTE[size];
			m_vFile.Read(pBuffer, size);
			m_vFile.Close();

			SetData(pBuffer, size);
			_Data2Info(m_pRawData);
			_Data2Thumbnail();
			return true;
		} else {
			m_vFile.Close();
			delete []m_pRawData;
			m_pRawData = NULL;
			m_DataSize = 0;
			m_dwMask &= ~ITEMDATA;
			m_dwMask &= ~ITEMTHUMBNAIL;
		}
	}
	return false;
}

bool CTreeInfo::LoadThumbnail() 
{
	if((m_dwMask & ITEMFILE) != ITEMFILE) return false;

	if((m_dwMask & ITEMDATA) == ITEMDATA) return false;
	if((m_dwMask & ITEMTHUMBNAIL) == ITEMTHUMBNAIL) return false;

	if(m_vFile.Open("r")) {
		m_eType = titFile;
		_OpenLegendsFile OZFile;
		if(m_vFile.Read(&OZFile, sizeof(_OpenLegendsFile)) != sizeof(_OpenLegendsFile)) {
			m_vFile.Close();
			return false;
		}
		_Data2Info((LPBYTE)&OZFile);

		LPCOZFILE pOZFile = &OZFile;
		VerifyOZFile(&pOZFile);

		// Load the thumbnail:
		if(pOZFile && OZFile.dwBitmapOffset) {
			size_t size = m_vFile.GetFileSize();
			if(OZFile.dwBitmapOffset + sizeof(BITMAP) < size) {
				size -= OZFile.dwBitmapOffset;
				LPBYTE pBuffer = new BYTE[size];
				m_vFile.Seek(OZFile.dwBitmapOffset, SEEK_SET);
				m_vFile.Read(pBuffer, size);
				SetData(pBuffer, size);
				m_vFile.Close();

				LPBITMAP pBitmap = (LPBITMAP)(pBuffer);
				pBitmap->bmBits = (LPVOID)((LPBYTE)pBitmap + sizeof(BITMAP));
				m_bmpThumbnail = *pBitmap;
				m_dwMask |= ITEMTHUMBNAIL;
				return true;
			}
		}
		m_vFile.Close();
	}
	return false;
}
bool CTreeInfo::Delete()
{
	if((m_dwMask & ITEMFILE) != ITEMFILE) return false;
	if(m_vFile.Delete() == -1) return false;
	m_eType = titFile;
	return true;
}

bool CTreeInfo::Update() 
{
	bool ret = false;
	if((m_dwMask & ITEMFILE) != ITEMFILE) return false;
	if(m_vFile.Open("wb")) {
		m_eType = titFile;
		_OpenLegendsFile OZFile;
		m_vFile.Read(&OZFile, sizeof(_OpenLegendsFile));

		LPCOZFILE pOZFile = &OZFile;
		VerifyOZFile(&pOZFile);

		if(pOZFile) {
			LPSTR aux = strchr(OZFile.ID, '\n');
			if(aux) {
				aux++;
				std::string sTmp = m_sDisplayName + "\n" + m_sDescription;
				strncpy(aux, sTmp.c_str(), sizeof(OZFile.ID) - (aux - OZFile.ID) - 1);

				m_vFile.Seek(0, SEEK_SET);
				if(m_vFile.Write(&OZFile, sizeof(_OpenLegendsFile)) == sizeof(_OpenLegendsFile)) ret = true;
			}
		}
		m_vFile.Close();
	} 
	return ret;
}
bool CTreeInfo::Rename(LPCSTR _szName) 
{
	ATLASSERT(_szName);
	if(!_szName) return false;
	if(!*_szName) return false;

	if((m_dwMask & ITEMFILE) != ITEMFILE) return false;
	CVFile vFile = m_vFile;
	vFile.SetFileTitle(_szName);
	if(m_vFile.Rename(vFile) == -1) return false;
	m_sDisplayName = _szName;
	m_eType = titFile;
	Update();
	return true;
}
bool CTreeInfo::MoveFile(LPCSTR _szFilePath) 
{
	ATLASSERT(_szFilePath);
	if(!_szFilePath) return false;
	if(!*_szFilePath) return false;

	if((m_dwMask & ITEMFILE) != ITEMFILE) return false;
	CVFile vFile;
	vFile.SetFileTitle(_szFilePath);
	m_sDisplayName = vFile.GetFileDesc();
	if(m_vFile.Rename(vFile) == -1) return false;
	m_eType = titFile;
	return true;
}

////////////////////////////// Implementation of CSystemImageList /////////////////////////////////

CSystemImageList::CSystemImageList()
{
  // We need to implement reference counting to 
  // overcome an limitation whereby you cannot
  // have two CImageLists attached to the one underlyinh
  // HIMAGELIST.
  if (m_nRefCount == 0)
  {
    // Attach to the system image list
    SHFILEINFO sfi;
    HIMAGELIST hSystemImageList = (HIMAGELIST) SHGetFileInfo( _T("C:\\"), 0, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_SMALLICON );
    m_ImageList.Attach( hSystemImageList );
  }  
  // Increment the reference count
  m_nRefCount++;
}

CSystemImageList::~CSystemImageList()
{
  //Decrement the reference count
  m_nRefCount--;  

  if( m_nRefCount == 0 )
  {
    // Detach from the image list to prevent problems on 95/98 where
    // the system image list is shared across processes
    m_ImageList.Detach();
  }
}

CImageList* CSystemImageList::GetImageList()
{
  return &m_ImageList;
}

////////////////////////////// Implementation of CSystemImageList /////////////////////////////////
BOOL CWtlFileTreeCtrl::PreTranslateMessage(MSG* pMsg) 
{
	return FALSE;
}

BOOL CWtlFileTreeCtrl::SubclassWindow( HWND hWnd )
{
	BOOL bRet = CWindowImpl<CWtlFileTreeCtrl, CTreeViewCtrl>::SubclassWindow( hWnd );
	return bRet;
}

int CWtlFileTreeCtrl::CompareByFilenameNoCase( std::string element1, std::string element2 ) 
{
	std::transform( element1.begin(), element1.end(), element1.begin(), toupper );
	std::transform( element2.begin(), element2.end(), element2.begin(), toupper );
	return lstrcmpi( element1.c_str(), element2.c_str() );
}

std::string CWtlFileTreeCtrl::GetSelectedPath(BOOL bVF)
{
  return ItemToPath( GetSelectedItem(), bVF );
}

void CWtlFileTreeCtrl::SetShowFiles( BOOL bFiles ) 
{ 
  m_bShowFiles = bFiles; 
  if( ::IsWindow( m_hWnd ) )
    OnViewRefresh();
}

void CWtlFileTreeCtrl::OnViewRefresh() 
{
	if(m_RootItem == 0) return;

	SetRedraw( FALSE );

	// Get the item which is currently selected
	HTREEITEM hSelItem = GetSelectedItem();
	std::string sItem  = ItemToPath( hSelItem );
	BOOL bExpanded = ( GetChildItem( hSelItem ) != NULL ); 

	// No items, display the folder items in the tree
	if( sItem == "" ) {
		DisplayDrives( m_RootItem, TRUE );
	} else {
		HTREEITEM hItem = hSelItem;
		std::string sPath = sItem;
		// If the selected path is not a directory, we go up one level
		CFileFind find;
		BOOL bFind = find.FindFile( sPath.c_str() );  
		if( (!bFind || !find.IsDirectory()) && hItem != m_RootItem) {
			hItem = GetParentItem( hItem );
			sPath  = ItemToPath( hItem );
		}
		if(sPath == m_sRootFolder) hItem = m_RootItem;
		DisplayPath( sPath, hItem, TRUE );
	}
  
	// Reselect the initially selected item
	if( sItem.size() )
		hSelItem = SetSelectedPath( sItem, bExpanded );

	SetRedraw( TRUE );
}

std::string CWtlFileTreeCtrl::ItemToPath( HTREEITEM hItem, BOOL bVF )
{
	std::string sPath = "";
  
	// Create the full string of the tree item
	HTREEITEM hParent = hItem;
	std::string sItem = "";
	int nLength = 0;
	char ItemText[512];

	while( hParent && hParent != m_RootItem )
	{
		GetItemText( hParent, ItemText, 512 );
		sItem = ItemText;
		nLength = sItem.size();
		hParent = GetParentItem( hParent );

		if( sItem[ nLength - 1 ] == _T('\\') )
			sPath = sItem + sPath;
		else
		{
			if( sPath.size() )
				sPath = sItem + _T('\\') + sPath;
			else
				sPath = sItem;
		}
	}

	if(bVF) {
		if(hParent != m_RootItem) return m_sRootFolder; // not found in file system

		// Add the root folder if there is one
		if( m_sRootFolder.size() )
			sPath = m_sRootFolder + _T('\\') + sPath;
	}
	return sPath;
}

int CWtlFileTreeCtrl::GetIconIndex( HTREEITEM hItem )
{
	TV_ITEM tvi;
	ZeroMemory( &tvi, sizeof(TV_ITEM) );
	tvi.mask	= TVIF_IMAGE;
	tvi.hItem	= hItem;
	if( GetItem( &tvi ) )
		return tvi.iImage;
	else
		return -1;
}

int CWtlFileTreeCtrl::GetIconIndex( const std::string sFilename )
{
	int idx = -1;
	if(m_hImageList) {
		std::string sExt;
		ImageIndex::iterator Iterator = m_ImageIndex.end();
		int nDot = sFilename.rfind('.');
		if(nDot>=0) {
			sExt = sFilename.substr(nDot);
			Iterator = m_ImageIndex.find(sExt);
		}
		if(Iterator == m_ImageIndex.end()) {
			if(sExt.size() > 0 && sExt.size() <= 5 && nDot) Iterator = m_ImageIndex.find("File");
			else Iterator = m_ImageIndex.find("Folder");
		}
		if(Iterator != m_ImageIndex.end())
			idx = (int)(Iterator->second >> 8);
	} else {
		// Retreive the icon index for a specified file/folder
		SHFILEINFO sfi;
		ZeroMemory(&sfi, sizeof(SHFILEINFO));

		if( SHGetFileInfo( sFilename.c_str(), 0, &sfi, sizeof(SHFILEINFO), SHGFI_ICON|SHGFI_SMALLICON ) == 0 )
			return -1;
		idx = sfi.iIcon;
	}
	return idx;
}

int CWtlFileTreeCtrl::GetSelIconIndex( const std::string sFilename )
{
	int idx = -1;
	if(m_hImageList) {
		std::string sExt;
		ImageIndex::iterator Iterator = m_ImageIndex.end();
		int nDot = sFilename.rfind('.');
		if(nDot>=0) {
			sExt = sFilename.substr(nDot);
			Iterator = m_ImageIndex.find(sExt);
		}
		if(Iterator == m_ImageIndex.end()) {
			if(sExt.size() > 0 && sExt.size() <= 5 && nDot) Iterator = m_ImageIndex.find("File");
			else Iterator = m_ImageIndex.find("Folder");
		}
		if(Iterator != m_ImageIndex.end())
			idx = (int)(Iterator->second & 0xff);
	} else {
		// Retreive the icon index for a specified file/folder
		SHFILEINFO sfi;
		if( SHGetFileInfo( sFilename.c_str(), 0, &sfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_OPENICON | SHGFI_SMALLICON ) == 0 )
			return -1;
		idx = sfi.iIcon;
	}
	return idx;
}

LRESULT CWtlFileTreeCtrl::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if(wParam == VK_F5) {
		OnViewRefresh();
		return 1;
	} else if(wParam == VK_F2) {
		EditLabel(GetSelectedItem());
	} else if(wParam == VK_DELETE) {
		CTreeInfo *pTreeInfo = (CTreeInfo *)GetItemData(GetSelectedItem());
		if(pTreeInfo) {
			if(pTreeInfo->Delete()) {
				HTREEITEM hParent = GetParentItem(GetSelectedItem());
				DeleteItem(GetSelectedItem());
				if(!GetChildItem(hParent)) {
					TVITEM item;
					item.hItem = hParent;
					item.mask = TVIF_CHILDREN;
					item.cChildren = FALSE;
					SetItem(&item);
				}
			}
		}
	} else bHandled = FALSE;
	return 0;
}

BOOL CWtlFileTreeCtrl::HasGotSubEntries( const std::string sDirectory )
{
	CFileFind find;
	std::string sFile;
	if( sDirectory[ sDirectory.size() - 1 ] == _T('\\') )
		sFile = sDirectory + _T("*.*");
	else
		sFile = sDirectory + _T("\\*.*");
	
	BOOL bFind = find.FindFile( sFile.c_str() );  
	while( bFind )
	{
		bFind = find.FindNextFile();
		if( find.IsDirectory() && !find.IsDots() )
			return TRUE;
		else if( !find.IsDirectory() && !find.IsHidden() && m_bShowFiles )
			return TRUE;
	}
	return FALSE;
}

void CWtlFileTreeCtrl::SetRootFolder( const std::string sPath )
{
	int nLength = sPath.size();
	if (nLength) {
		if ( sPath[ nLength - 1 ] == _T('\\') )
			m_sRootFolder = sPath.substr( 0, nLength - 1 );
		else
			m_sRootFolder = sPath;
	}
	else m_sRootFolder = "";

	if( IsWindow( ) )
		OnViewRefresh();
}

HTREEITEM CWtlFileTreeCtrl::InsertRootItem( LPCSTR szFile, LPCSTR szPath, HTREEITEM hParent, DWORD Idx )
{
	m_RootItem = InsertFileItem(szFile, szPath, hParent, Idx);
	return m_RootItem;
}
HTREEITEM CWtlFileTreeCtrl::InsertFileItem( LPCSTR szFile, LPCSTR szPath, HTREEITEM hParent, DWORD Idx )
{
	return InsertFileItem(std::string(szFile), std::string(szPath), hParent, Idx);
}

HTREEITEM CWtlFileTreeCtrl::InsertFileItem(const std::string sFile, const std::string sPath, HTREEITEM hParent, DWORD Idx )
{
	// Retreive the icon indexes for the specified file/folder
	std::string sFullPath = sPath;
	if( sFullPath[ sFullPath.length() - 1 ] != '\\' && !sFullPath.empty())
		sFullPath += "\\";
	sFullPath += sFile;

	int nIconIndex = (int)(Idx >> 8);
	int nSelIconIndex = (int)(Idx & 0xff);
	if( Idx == -1 ) {
		nIconIndex = GetIconIndex( sFullPath );
		nSelIconIndex = GetSelIconIndex( sFullPath );
	}
	if( nIconIndex == -1 || nSelIconIndex == -1 ) {
		if(m_hImageList == NULL)
			ATLTRACE( _T("Failed in call to SHGetFileInfo for %s, GetLastError:%d\n"), sPath.c_str(), ::GetLastError() );
		else 
			ATLTRACE( _T("No image index set for for type 'Folder' or 'File'\n"));
		return NULL;
	}

	//Add the actual item
	std::string sTemp;
	if( sFile != "" ) sTemp = sFile;
	else sTemp = sPath;

	// Build the tree information:
	CTreeInfo *pTreeInfo = new CTreeInfo('?', sTemp, sFullPath);
	pTreeInfo->LoadThumbnail(); // if the file is found, the titFile type is set
	
	TV_INSERTSTRUCT tvis;
	ZeroMemory( &tvis, sizeof(TV_INSERTSTRUCT) );
	tvis.hParent		= hParent;
	tvis.hInsertAfter	= TVI_LAST;
	tvis.item.mask		= TVIF_CHILDREN | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
	tvis.item.pszText	= (LPSTR)pTreeInfo->GetDisplayName();
	tvis.item.iImage	= nIconIndex;
	tvis.item.iSelectedImage = nSelIconIndex;
	tvis.item.cChildren = HasGotSubEntries( sFullPath );
	tvis.item.lParam	= (LPARAM)pTreeInfo;

	HTREEITEM hItem = InsertItem( &tvis );
	pTreeInfo->m_hItem = hItem;
	pTreeInfo->m_hWndParent = m_hWnd;
	ATLASSERT(hItem);
	if(!hItem) delete pTreeInfo;

	return hItem;
}

void CWtlFileTreeCtrl::DisplayDrives( HTREEITEM hParent, BOOL bUseSetRedraw )
{
	CWaitCursor c;

	// Speed up the job by turning off redraw
	if( bUseSetRedraw )
		SetRedraw( FALSE );

	// Remove all the items currently under hParent
	HTREEITEM hChild = GetChildItem( hParent );
	while( hChild )
	{
		DeleteItem( hChild );
		hChild = GetChildItem( hParent );
	}

	// Enumerate the drive letters and add them to the tree control
	DWORD dwDrives = GetLogicalDrives();
	DWORD dwMask = 1;
	//std::string sDrive = "";
	char DriveLetter[2] = { ' ', '\0' };
	std::string sDrive;
	
	int nChildren = 0;
	for(int i = 0; i < 32; i++ )
	{
		if( dwDrives & dwMask )
		{
			DriveLetter[0] = i + _T('A');
			sDrive = DriveLetter;
			sDrive += ":\\";

			if( sDrive == _T("A:\\") || sDrive == _T("B:\\") ){}
			else {
				InsertFileItem( "", sDrive, hParent );
				nChildren++;
			}
		}
		dwMask <<= 1;
	}
	ItemWithChildren(hParent, nChildren?TRUE:FALSE);

	if( bUseSetRedraw )
		SetRedraw( TRUE );
}

void CWtlFileTreeCtrl::DisplayPath( const std::string sPath, HTREEITEM hParent, BOOL bUseSetRedraw )
{
	CWaitCursor c;

	// Speed up the job by turning off redraw
	if( bUseSetRedraw )
		SetRedraw( FALSE );

	// Remove all the items currently under hParent
	HTREEITEM hChild = GetChildItem( hParent );
	while( hChild )
	{
		DeleteItem( hChild );
		hChild = GetChildItem( hParent );
	}
	
	// Find all the directories and files underneath sPath
	std::vector< std::string > DirectoryPaths;
	std::vector< std::string > FilePaths;
	
	CFileFind find;
	std::string sFile;
	std::string sNewPath;
	if( sPath[ sPath.size()-1] == _T('\\') )
		sFile = sPath + _T("*.*");
	else
		sFile = sPath + _T("\\*.*");

	BOOL bFind = find.FindFile( sFile.c_str() );  
	while( bFind )
	{
		bFind = find.FindNextFile();
		if( find.IsDirectory() )
		{
			if( !find.IsDots() )
			{
				sNewPath = find.GetFilePath();
				DirectoryPaths.push_back( sNewPath );
			}
		}
		else 
		{
			if( !find.IsHidden() && m_bShowFiles )
			{
				sNewPath = find.GetFilePath();
				FilePaths.push_back( sNewPath );
			}  
		}
	}

    TCHAR path_buffer[_MAX_PATH];
    TCHAR fname[_MAX_FNAME];
    TCHAR ext[_MAX_EXT];

	int nChildren = 0;

	// Now add all the directories to the tree control
	for( int i = 0; i < (int)DirectoryPaths.size(); i++ )
	{
		_tsplitpath( DirectoryPaths[i].c_str(), NULL, NULL, fname, ext );
		_tmakepath( path_buffer, NULL, NULL, fname, ext);
		InsertFileItem( path_buffer, sPath, hParent );
		nChildren++;
	}

	// And the files to the tree control (if required)
	for( i = 0; i < (int)FilePaths.size(); i++ )
	{
		_tsplitpath( FilePaths[i].c_str(), NULL, NULL, fname, ext);
		_tmakepath(path_buffer, NULL, NULL, fname, ext);
		InsertFileItem(path_buffer, sPath, hParent);
		nChildren++;
	}

	ItemWithChildren(hParent, nChildren?TRUE:FALSE);

	// Turn back on the redraw flag
	if( bUseSetRedraw )
		SetRedraw( TRUE );
}

void CWtlFileTreeCtrl::ItemWithChildren(HTREEITEM hItem, BOOL bChildren)
{
	if(hItem == NULL) return;

	TVITEM item;
	item.hItem = hItem;
	item.mask = TVIF_CHILDREN;
	item.cChildren = bChildren;
	::SendMessage(m_hWnd, TVM_SETITEM, 0, (LPARAM)&item);
}

HTREEITEM CWtlFileTreeCtrl::FindSibling( HTREEITEM hParent, const std::string sItem )
{
	HTREEITEM hChild = GetChildItem( hParent );
	char sFound[ _MAX_PATH ];
	while( hChild )
	{
		GetItemText(hChild, sFound, _MAX_PATH );
		if( CompareByFilenameNoCase( sItem, sFound ) == 0 )
			return hChild;
		hChild = GetNextItem( hChild, TVGN_NEXT );
	}
	return NULL;
}

HTREEITEM CWtlFileTreeCtrl::SetSelectedPath( const std::string sPath, BOOL bExpanded )
{
	std::string sSearch = sPath;
	
	int nSearchLength = sSearch.size();
	if( nSearchLength == 0 )
	{
		ATLTRACE( _T("Cannot select a empty path\n") );
		return NULL;
	}

	// Remove trailing "\" from the path
	if( nSearchLength > 3 && sSearch[ nSearchLength - 1 ] == _T('\\'))
    sSearch = sSearch.substr( 0, nSearchLength - 1 );
  
	// Remove initial part of path if the root folder is setup
	int nRootLength = m_sRootFolder.size();
	if( nRootLength )
	{
		if(sSearch ==  m_sRootFolder) {
			sSearch = "";
		} else {
			if( sSearch.find( m_sRootFolder ) != 0 ) {
				ATLTRACE( _T("Could not select the path %s as the root has been configued as %s\n"), sPath, m_sRootFolder );
				return NULL;
			}
			sSearch = sSearch.substr( nRootLength + 1 );
		}
	}

	if( sSearch == "" )
		return NULL;
	
	SetRedraw( FALSE );

	HTREEITEM hItemFound = m_RootItem;
	int nFound = sSearch.find( _T('\\') );
	BOOL bDriveMatch = TRUE;
	if( m_sRootFolder != "" )
		bDriveMatch = FALSE;
	
	std::string sMatch = "";
	while( nFound != -1 )
	{
		if( bDriveMatch )
		{
			sMatch = sSearch.substr( 0, nFound + 1 );
			bDriveMatch = FALSE;
		}
		else
			sMatch = sSearch.substr( 0, nFound );
		hItemFound = FindSibling( hItemFound, sMatch );
		if ( hItemFound == NULL )
			break;
		else
			Expand( hItemFound, TVE_EXPAND );

		sSearch = sSearch.substr(nFound + 1 );
		nFound = sSearch.find( _T('\\') );
	}

	// The last item 
	if( hItemFound )
	{
		if( sSearch.size() )
			hItemFound = FindSibling( hItemFound, sSearch );
		SelectItem( hItemFound );

		if( bExpanded )
		Expand( hItemFound, TVE_EXPAND );
	}

	SetRedraw( TRUE );

	return hItemFound;
}

LRESULT CWtlFileTreeCtrl::OnPopulateTree(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// attach the image list to the tree control
	if(m_hImageList) {
		SetImageList( m_hImageList, TVSIL_NORMAL );
	} else {
		SetImageList( *(m_SysImageList.GetImageList()), TVSIL_NORMAL );
	}
	// Force a refresh
	OnViewRefresh();
	return 0L;
}
LRESULT CWtlFileTreeCtrl::OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}
LRESULT CWtlFileTreeCtrl::OnRButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}
LRESULT CWtlFileTreeCtrl::OnLButtonDblClick(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if(ItemHasChildren(GetSelectedItem())) {
		Expand(GetSelectedItem(), TVE_TOGGLE);
	} else {
		CTreeInfo *pTreeInfo = (CTreeInfo *)GetItemData( GetSelectedItem() );
		if(pTreeInfo) ::SendMessage( GetParent(), WM_ITEM_SELECTED, (WPARAM)pTreeInfo, 0 );
	}
	return 0;	
}

LRESULT CWtlFileTreeCtrl::OnItemExpanding(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pnmh;
	if( pNMTreeView->action == TVE_EXPAND )
	{
		// Add the new items to the tree if it does not have any child items
		// already
		if( !GetChildItem( pNMTreeView->itemNew.hItem ) && m_sRootFolder.size() > 0) {
			std::string sPath = ItemToPath(pNMTreeView->itemNew.hItem);
			DisplayPath( sPath, pNMTreeView->itemNew.hItem );
		}
	}
	return 0;
}

LRESULT CWtlFileTreeCtrl::OnEndLabelEdit(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
	LPNMTVDISPINFO pNMTVDispInfo = (LPNMTVDISPINFO)pnmh;
	if(pNMTVDispInfo->item.pszText == NULL) return FALSE;

	CTreeInfo *pTreeInfo = (CTreeInfo *)GetItemData( GetSelectedItem() );
	if(!pTreeInfo) return FALSE;

	if(!pTreeInfo->Rename(pNMTVDispInfo->item.pszText)) return FALSE;
	return TRUE;
}
LRESULT CWtlFileTreeCtrl::OnDeleteItem(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pnmh;
	if( pNMTreeView->action == TVM_DELETEITEM )	{
		delete (CTreeInfo*)pNMTreeView->itemNew.lParam;
	}
	return 0;
}

