#pragma once

#include <cstdio>
#include <cstdlib>
#include <climits>

#ifndef ASSERT
	#include <cassert>
	#define ASSERT assert
#endif

#define FEF_NOEXT		0x01
#define FEF_NODIR		0x02

// How should the filename and path be passed to the callback function
#define FEF_LOWERCASE	0x10
#define FEF_BEAUTIFY	0x14
#define FEF_UPPERCASE	0x20

typedef int CALLBACK FILESPROC(LPCSTR lpFile, LPARAM lParam);
int ForEachFile(LPCSTR lpFileName, FILESPROC ForEach, LPARAM lParam, int flags = FEF_BEAUTIFY);

/////////////////////////////////////////////////////////////////////////////
#ifndef _NO_MISCTYPES

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
class CSize;
class CPoint;
class CRect;

/////////////////////////////////////////////////////////////////////////////
// CSize - Wrapper for Windows SIZE structure.
class CSize : public tagSIZE
{
public:
// Constructors
	CSize();
	CSize(int initCX, int initCY);
	CSize(SIZE initSize);
	CSize(POINT initPt);
	CSize(DWORD dwSize);

// Operations
	bool operator==(SIZE size) const;
	bool operator!=(SIZE size) const;
	void operator+=(SIZE size);
	void operator-=(SIZE size);
	void SetSize(int CX, int CY);

// Operators returning CSize values
	CSize operator+(SIZE size) const;
	CSize operator-(SIZE size) const;
	CSize operator-() const;

// Operators returning CPoint values
	CPoint operator+(POINT point) const;
	CPoint operator-(POINT point) const;

// Operators returning CRect values
	CRect operator+(const RECT* lpRect) const;
	CRect operator-(const RECT* lpRect) const;
};


/////////////////////////////////////////////////////////////////////////////
// CPoint - Wrapper for Windows POINT structure.

class CPoint : public tagPOINT
{
public:
// Constructors
	CPoint();
	CPoint(int initX, int initY);
	CPoint(POINT initPt);
	CPoint(SIZE initSize);
	CPoint(DWORD dwPoint);

// Operations
	void Offset(int xOffset, int yOffset);
	void Offset(POINT point);
	void Offset(SIZE size);
	bool operator==(POINT point) const;
	bool operator!=(POINT point) const;
	void operator+=(SIZE size);
	void operator-=(SIZE size);
	void operator+=(POINT point);
	void operator-=(POINT point);
	void SetPoint(int X, int Y);

// Operators returning CPoint values
	CPoint operator+(SIZE size) const;
	CPoint operator-(SIZE size) const;
	CPoint operator-() const;
	CPoint operator+(POINT point) const;

// Operators returning CSize values
	CSize operator-(POINT point) const;

// Operators returning CRect values
	CRect operator+(const RECT* lpRect) const;
	CRect operator-(const RECT* lpRect) const;
};


/////////////////////////////////////////////////////////////////////////////
// CRect - Wrapper for Windows RECT structure.

class CRect : public tagRECT
{
public:
// Constructors
	CRect();
	CRect(int l, int t, int r, int b);
	CRect(const RECT& srcRect);
	CRect(LPCRECT lpSrcRect);
	CRect(POINT point, SIZE size);
	CRect(POINT topLeft, POINT bottomRight);

// Attributes (in addition to RECT members)
	int Width() const;
	int Height() const;
	CSize Size() const;
	CPoint& TopLeft();
	CPoint& BottomRight();
	const CPoint& TopLeft() const;
	const CPoint& BottomRight() const;
	CPoint CenterPoint() const;

	// convert between CRect and LPRECT/LPCRECT (no need for &)
	operator LPRECT();
	operator LPCRECT() const;

	bool IsRectEmpty() const;
	bool IsRectNull() const;
	bool PtInRect(POINT point) const;

// Operations
	void SetRect(int x1, int y1, int x2, int y2);
	void SetRect(POINT topLeft, POINT bottomRight);
	void SetRectEmpty();
	void CopyRect(LPCRECT lpSrcRect);
	bool EqualRect(LPCRECT lpRect) const;

	void InflateRect(int x, int y);
	void InflateRect(SIZE size);
	void InflateRect(LPCRECT lpRect);
	void InflateRect(int l, int t, int r, int b);
	void DeflateRect(int x, int y);
	void DeflateRect(SIZE size);
	void DeflateRect(LPCRECT lpRect);
	void DeflateRect(int l, int t, int r, int b);

	void OffsetRect(int x, int y);
	void OffsetRect(SIZE size);
	void OffsetRect(POINT point);
	void NormalizeRect();

	// absolute position of rectangle
	void MoveToY(int y);
	void MoveToX(int x);
	void MoveToXY(int x, int y);
	void MoveToXY(POINT point);

	// operations that fill '*this' with result
	bool IntersectRect(LPCRECT lpRect1, LPCRECT lpRect2);
	bool UnionRect(LPCRECT lpRect1, LPCRECT lpRect2);
	bool SubtractRect(LPCRECT lpRectSrc1, LPCRECT lpRectSrc2);

// Additional Operations
	void operator=(const RECT& srcRect);
	bool operator==(const RECT& rect) const;
	bool operator!=(const RECT& rect) const;
	void operator+=(POINT point);
	void operator+=(SIZE size);
	void operator+=(LPCRECT lpRect);
	void operator-=(POINT point);
	void operator-=(SIZE size);
	void operator-=(LPCRECT lpRect);
	void operator&=(const RECT& rect);
	void operator|=(const RECT& rect);

// Operators returning CRect values
	CRect operator+(POINT point) const;
	CRect operator-(POINT point) const;
	CRect operator+(LPCRECT lpRect) const;
	CRect operator+(SIZE size) const;
	CRect operator-(SIZE size) const;
	CRect operator-(LPCRECT lpRect) const;
	CRect operator&(const RECT& rect2) const;
	CRect operator|(const RECT& rect2) const;
	CRect MulDiv(int nMultiplier, int nDivisor) const;
};


/////////////////////////////////////////////////////////////////////////////
// CSize, CPoint, CRect Implementation

// CSize
inline CSize::CSize()
	{ /* random filled */ }
inline CSize::CSize(int initCX, int initCY)
	{ cx = initCX; cy = initCY; }
inline CSize::CSize(SIZE initSize)
	{ *(SIZE*)this = initSize; }
inline CSize::CSize(POINT initPt)
	{ *(POINT*)this = initPt; }
inline CSize::CSize(DWORD dwSize)
	{
		cx = (short)LOWORD(dwSize);
		cy = (short)HIWORD(dwSize);
	}
inline bool CSize::operator==(SIZE size) const
	{ return (cx == size.cx && cy == size.cy); }
inline bool CSize::operator!=(SIZE size) const
	{ return (cx != size.cx || cy != size.cy); }
inline void CSize::operator+=(SIZE size)
	{ cx += size.cx; cy += size.cy; }
inline void CSize::operator-=(SIZE size)
	{ cx -= size.cx; cy -= size.cy; }
inline void CSize::SetSize(int CX, int CY)
	{ cx = CX; cy = CY; }	
inline CSize CSize::operator+(SIZE size) const
	{ return CSize(cx + size.cx, cy + size.cy); }
inline CSize CSize::operator-(SIZE size) const
	{ return CSize(cx - size.cx, cy - size.cy); }
inline CSize CSize::operator-() const
	{ return CSize(-cx, -cy); }
inline CPoint CSize::operator+(POINT point) const
	{ return CPoint(cx + point.x, cy + point.y); }
inline CPoint CSize::operator-(POINT point) const
	{ return CPoint(cx - point.x, cy - point.y); }
inline CRect CSize::operator+(const RECT* lpRect) const
	{ return CRect(lpRect) + *this; }
inline CRect CSize::operator-(const RECT* lpRect) const
	{ return CRect(lpRect) - *this; }

// CPoint
inline CPoint::CPoint()
	{ /* random filled */ }
inline CPoint::CPoint(int initX, int initY)
	{ x = initX; y = initY; }
inline CPoint::CPoint(POINT initPt)
	{ *(POINT*)this = initPt; }
inline CPoint::CPoint(SIZE initSize)
	{ *(SIZE*)this = initSize; }
inline CPoint::CPoint(DWORD dwPoint)
	{
		x = (short)LOWORD(dwPoint);
		y = (short)HIWORD(dwPoint);
	}
inline void CPoint::Offset(int xOffset, int yOffset)
	{ x += xOffset; y += yOffset; }
inline void CPoint::Offset(POINT point)
	{ x += point.x; y += point.y; }
inline void CPoint::Offset(SIZE size)
	{ x += size.cx; y += size.cy; }
inline bool CPoint::operator==(POINT point) const
	{ return (x == point.x && y == point.y); }
inline bool CPoint::operator!=(POINT point) const
	{ return (x != point.x || y != point.y); }
inline void CPoint::operator+=(SIZE size)
	{ x += size.cx; y += size.cy; }
inline void CPoint::operator-=(SIZE size)
	{ x -= size.cx; y -= size.cy; }
inline void CPoint::operator+=(POINT point)
	{ x += point.x; y += point.y; }
inline void CPoint::operator-=(POINT point)
	{ x -= point.x; y -= point.y; }
inline void CPoint::SetPoint(int X, int Y)
	{ x = X; y = Y; }
inline CPoint CPoint::operator+(SIZE size) const
	{ return CPoint(x + size.cx, y + size.cy); }
inline CPoint CPoint::operator-(SIZE size) const
	{ return CPoint(x - size.cx, y - size.cy); }
inline CPoint CPoint::operator-() const
	{ return CPoint(-x, -y); }
inline CPoint CPoint::operator+(POINT point) const
	{ return CPoint(x + point.x, y + point.y); }
inline CSize CPoint::operator-(POINT point) const
	{ return CSize(x - point.x, y - point.y); }
inline CRect CPoint::operator+(const RECT* lpRect) const
	{ return CRect(lpRect) + *this; }
inline CRect CPoint::operator-(const RECT* lpRect) const
	{ return CRect(lpRect) - *this; }

// CRect
inline CRect::CRect()
	{ /* random filled */ }
inline CRect::CRect(int l, int t, int r, int b)
	{ left = l; top = t; right = r; bottom = b; }
inline CRect::CRect(const RECT& srcRect)
	{ ::CopyRect(this, &srcRect); }
inline CRect::CRect(LPCRECT lpSrcRect)
	{ ::CopyRect(this, lpSrcRect); }
inline CRect::CRect(POINT point, SIZE size)
	{ right = (left = point.x) + size.cx; bottom = (top = point.y) + size.cy; }
inline CRect::CRect(POINT topLeft, POINT bottomRight)
	{ left = topLeft.x; top = topLeft.y;
		right = bottomRight.x; bottom = bottomRight.y; }
inline int CRect::Width() const
	{ return right - left; }
inline int CRect::Height() const
	{ return bottom - top; }
inline CSize CRect::Size() const
	{ return CSize(right - left, bottom - top); }
inline CPoint& CRect::TopLeft()
	{ return *((CPoint*)this); }
inline CPoint& CRect::BottomRight()
	{ return *((CPoint*)this + 1); }
inline const CPoint& CRect::TopLeft() const
	{ return *((CPoint*)this); }
inline const CPoint& CRect::BottomRight() const
	{ return *((CPoint*)this + 1); }
inline CPoint CRect::CenterPoint() const
	{ return CPoint((left + right) / 2, (top + bottom) / 2); }
inline CRect::operator LPRECT()
	{ return this; }
inline CRect::operator LPCRECT() const
	{ return this; }
inline bool CRect::IsRectEmpty() const
	{ return (::IsRectEmpty(this)==TRUE); }
inline bool CRect::IsRectNull() const
	{ return (left == 0 && right == 0 && top == 0 && bottom == 0); }
inline bool CRect::PtInRect(POINT point) const
	{ return (::PtInRect(this, point)==TRUE); }
inline void CRect::SetRect(int x1, int y1, int x2, int y2)
	{ ::SetRect(this, x1, y1, x2, y2); }
inline void CRect::SetRect(POINT topLeft, POINT bottomRight)
	{ ::SetRect(this, topLeft.x, topLeft.y, bottomRight.x, bottomRight.y); }
inline void CRect::SetRectEmpty()
	{ ::SetRectEmpty(this); }
inline void CRect::CopyRect(LPCRECT lpSrcRect)
	{ ::CopyRect(this, lpSrcRect); }
inline bool CRect::EqualRect(LPCRECT lpRect) const
	{ return (::EqualRect(this, lpRect)==TRUE); }
inline void CRect::InflateRect(int x, int y)
	{ ::InflateRect(this, x, y); }
inline void CRect::InflateRect(SIZE size)
	{ ::InflateRect(this, size.cx, size.cy); }
inline void CRect::DeflateRect(int x, int y)
	{ ::InflateRect(this, -x, -y); }
inline void CRect::DeflateRect(SIZE size)
	{ ::InflateRect(this, -size.cx, -size.cy); }
inline void CRect::OffsetRect(int x, int y)
	{ ::OffsetRect(this, x, y); }
inline void CRect::OffsetRect(POINT point)
	{ ::OffsetRect(this, point.x, point.y); }
inline void CRect::OffsetRect(SIZE size)
	{ ::OffsetRect(this, size.cx, size.cy); }
inline bool CRect::IntersectRect(LPCRECT lpRect1, LPCRECT lpRect2)
	{ return (::IntersectRect(this, lpRect1, lpRect2)==TRUE); }
inline bool CRect::UnionRect(LPCRECT lpRect1, LPCRECT lpRect2)
	{ return (::UnionRect(this, lpRect1, lpRect2)==TRUE); }
inline void CRect::operator=(const RECT& srcRect)
	{ ::CopyRect(this, &srcRect); }
inline bool CRect::operator==(const RECT& rect) const
	{ return (::EqualRect(this, &rect)==TRUE); }
inline bool CRect::operator!=(const RECT& rect) const
	{ return (::EqualRect(this, &rect)==FALSE); }
inline void CRect::operator+=(POINT point)
	{ ::OffsetRect(this, point.x, point.y); }
inline void CRect::operator+=(SIZE size)
	{ ::OffsetRect(this, size.cx, size.cy); }
inline void CRect::operator+=(LPCRECT lpRect)
	{ InflateRect(lpRect); }
inline void CRect::operator-=(POINT point)
	{ ::OffsetRect(this, -point.x, -point.y); }
inline void CRect::operator-=(SIZE size)
	{ ::OffsetRect(this, -size.cx, -size.cy); }
inline void CRect::operator-=(LPCRECT lpRect)
	{ DeflateRect(lpRect); }
inline void CRect::operator&=(const RECT& rect)
	{ ::IntersectRect(this, this, &rect); }
inline void CRect::operator|=(const RECT& rect)
	{ ::UnionRect(this, this, &rect); }
inline CRect CRect::operator+(POINT pt) const
	{ CRect rect(*this); ::OffsetRect(&rect, pt.x, pt.y); return rect; }
inline CRect CRect::operator-(POINT pt) const
	{ CRect rect(*this); ::OffsetRect(&rect, -pt.x, -pt.y); return rect; }
inline CRect CRect::operator+(SIZE size) const
	{ CRect rect(*this); ::OffsetRect(&rect, size.cx, size.cy); return rect; }
inline CRect CRect::operator-(SIZE size) const
	{ CRect rect(*this); ::OffsetRect(&rect, -size.cx, -size.cy); return rect; }
inline CRect CRect::operator+(LPCRECT lpRect) const
	{ CRect rect(this); rect.InflateRect(lpRect); return rect; }
inline CRect CRect::operator-(LPCRECT lpRect) const
	{ CRect rect(this); rect.DeflateRect(lpRect); return rect; }
inline CRect CRect::operator&(const RECT& rect2) const
	{ CRect rect; ::IntersectRect(&rect, this, &rect2);
		return rect; }
inline CRect CRect::operator|(const RECT& rect2) const
	{ CRect rect; ::UnionRect(&rect, this, &rect2);
		return rect; }
inline bool CRect::SubtractRect(LPCRECT lpRectSrc1, LPCRECT lpRectSrc2)
	{ return (::SubtractRect(this, lpRectSrc1, lpRectSrc2)==TRUE); }

inline void CRect::NormalizeRect()
{
	int nTemp;
	if (left > right)
	{
		nTemp = left;
		left = right;
		right = nTemp;
	}
	if (top > bottom)
	{
		nTemp = top;
		top = bottom;
		bottom = nTemp;
	}
}

inline void CRect::MoveToY(int y)
	{ bottom = Height() + y; top = y; }
inline void CRect::MoveToX(int x)
	{ right = Width() + x; left = x; }
inline void CRect::MoveToXY(int x, int y)
	{ MoveToX(x); MoveToY(y); }
inline void CRect::MoveToXY(POINT pt)
	{ MoveToX(pt.x); MoveToY(pt.y); }

inline void CRect::InflateRect(LPCRECT lpRect)
{
	left -= lpRect->left;
	top -= lpRect->top;
	right += lpRect->right;
	bottom += lpRect->bottom;
}

inline void CRect::InflateRect(int l, int t, int r, int b)
{
	left -= l;
	top -= t;
	right += r;
	bottom += b;
}

inline void CRect::DeflateRect(LPCRECT lpRect)
{
	left += lpRect->left;
	top += lpRect->top;
	right -= lpRect->right;
	bottom -= lpRect->bottom;
}

inline void CRect::DeflateRect(int l, int t, int r, int b)
{
	left += l;
	top += t;
	right -= r;
	bottom -= b;
}

inline CRect CRect::MulDiv(int nMultiplier, int nDivisor) const
{
	return CRect(
		::MulDiv(left, nMultiplier, nDivisor),
		::MulDiv(top, nMultiplier, nDivisor),
		::MulDiv(right, nMultiplier, nDivisor),
		::MulDiv(bottom, nMultiplier, nDivisor));
}
#endif // _NO_MISCTYPES


/////////////////////////////////////////////////////////////////////////////
#ifndef _NO_MISCSTRING

struct CBStringData
{
	long nRefs;     // reference count
	int nDataLength;
	int nAllocLength;
	// char data[nAllocLength]

	char* data() { return (char*)(this + 1); }
};

// Globals

// For an empty string, m_pchData will point here
// (note: avoids special case of checking for NULL m_pchData)
// empty string data (and locked)
_declspec(selectany) int rgInitData[] = { -1, 0, 0, 0 };
_declspec(selectany) CBStringData* _tmpDataNil = (CBStringData*)&rgInitData;
_declspec(selectany) LPCSTR _tmpPchNil = (LPCSTR)(((BYTE*)&rgInitData) + sizeof(CBStringData));

// This class is based on ATL's CString class, and it's used in order to avoid ATL libraries
// when they are not needed. CBString should be used whenever possible, to keep adaptability between
// modules.
class CBString
{
public:
// Constructors
	CBString();
	CBString(const CBString& stringSrc);
	CBString(char ch, int nRepeat = 1);
	CBString(LPCSTR lpsz);
	CBString(LPCSTR lpch, int nLength);
	CBString(const unsigned char* psz);

// Attributes & Operations
	// as an array of characters
	int GetLength() const;
	bool IsEmpty() const;
	void Empty();                      // free up the data

	char GetAt(int nIndex) const;      // 0 based
	char operator[](int nIndex) const; // same as GetAt
	void SetAt(int nIndex, char ch);
	operator LPCSTR() const;           // as a C string

	// overloaded assignment
	const CBString& operator=(const CBString& stringSrc);
	const CBString& operator=(char ch);
	const CBString& operator=(LPCSTR lpsz);
	const CBString& operator=(const unsigned char* psz);

	// string concatenation
	const CBString& operator+=(const CBString& string);
	const CBString& operator+=(char ch);
	const CBString& operator+=(LPCSTR lpsz);

	friend CBString __stdcall operator+(const CBString& string1, const CBString& string2);
	friend CBString __stdcall operator+(const CBString& string, char ch);
	friend CBString __stdcall operator+(char ch, const CBString& string);
	friend CBString __stdcall operator+(const CBString& string, LPCSTR lpsz);
	friend CBString __stdcall operator+(LPCSTR lpsz, const CBString& string);

	// string comparison
	int Compare(LPCSTR lpsz) const;         // straight character
	int CompareNoCase(LPCSTR lpsz) const;   // ignore case

	// simple sub-string extraction
	CBString Mid(int nFirst, int nCount) const;
	CBString Mid(int nFirst) const;
	CBString Left(int nCount) const;
	CBString Right(int nCount) const;

	CBString SpanIncluding(LPCSTR lpszCharSet) const;
	CBString SpanExcluding(LPCSTR lpszCharSet) const;

	// upper/lower/reverse conversion
	void MakeUpper();
	void MakeLower();
	void MakeReverse();

	// trimming whitespace (either side)
	void TrimRight();
	void TrimLeft();
	void Trim();

	// remove continuous occurrences of chTarget starting from right
	void TrimRight(char chTarget);
	// remove continuous occcurrences of characters in passed string,
	// starting from right
	void TrimRight(LPCSTR lpszTargets);
	// remove continuous occurrences of chTarget starting from left
	void TrimLeft(char chTarget);
	// remove continuous occcurrences of characters in
	// passed string, starting from left
	void TrimLeft(LPCSTR lpszTargets);

	// advanced manipulation
	// replace occurrences of chOld with chNew
	int Replace(char chOld, char chNew);
	// replace occurrences of substring lpszOld with lpszNew;
	// empty lpszNew removes instances of lpszOld
	int Replace(LPCSTR lpszOld, LPCSTR lpszNew);
	// remove occurrences of chRemove
	int Remove(char chRemove);
	// insert character at zero-based index; concatenates
	// if index is past end of string
	int Insert(int nIndex, char ch);
	// insert substring at zero-based index; concatenates
	// if index is past end of string
	int Insert(int nIndex, LPCSTR pstr);
	// delete nCount characters starting at zero-based index
	int Delete(int nIndex, int nCount = 1);

	// searching (return starting index, or -1 if not found)
	// look for a single character match
	int Find(char ch) const;                     // like "C" strchr
	int ReverseFind(char ch) const;
	int Find(char ch, int nStart) const;         // starting at index
	int FindOneOf(LPCSTR lpszCharSet) const;

	// look for a specific sub-string
	int Find(LPCSTR lpszSub) const;        // like "C" strstr
	int Find(LPCSTR lpszSub, int nStart) const;  // starting at index

	// Concatentation for non strings
	const CBString& Append(int n)
	{
		char szBuffer[10];
		sprintf(szBuffer,"%d",n);
		ConcatInPlace(SafeStrlen(szBuffer), szBuffer);
		return *this;
	}

	// simple formatting
	bool __cdecl Format(LPCSTR lpszFormat, ...);
	bool FormatV(LPCSTR lpszFormat, va_list argList);

	// Access to string implementation buffer as "C" character array
	LPTSTR GetBuffer(int nMinBufLength);
	void ReleaseBuffer(int nNewLength = -1);
	LPTSTR GetBufferSetLength(int nNewLength);
	void FreeExtra();

	// Use LockBuffer/UnlockBuffer to turn refcounting off
	LPTSTR LockBuffer();
	void UnlockBuffer();

// Implementation
public:
	~CBString();
	int GetAllocLength() const;

	static bool __stdcall _IsValidString(LPCSTR lpsz, int nLength = -1)
	{
		if(lpsz == NULL)
			return FALSE;
		return !::IsBadStringPtrA(lpsz, nLength);
	}

protected:
	LPTSTR m_pchData;   // pointer to ref counted string data

	// implementation helpers
	CBStringData* GetData() const;
	void Init();
	void AllocCopy(CBString& dest, int nCopyLen, int nCopyIndex, int nExtraLen) const;
	bool AllocBuffer(int nLen);
	void AssignCopy(int nSrcLen, LPCSTR lpszSrcData);
	bool ConcatCopy(int nSrc1Len, LPCSTR lpszSrc1Data, int nSrc2Len, LPCSTR lpszSrc2Data);
	void ConcatInPlace(int nSrcLen, LPCSTR lpszSrcData);
	void CopyBeforeWrite();
	bool AllocBeforeWrite(int nLen);
	void Release();
	static void PASCAL Release(CBStringData* pData);
	static int PASCAL SafeStrlen(LPCSTR lpsz);

	static const CBString& __stdcall _GetEmptyString()
	{
		return *(CBString*)&_tmpPchNil;
	}
};

// Compare helpers
bool __stdcall operator==(const CBString& s1, const CBString& s2);
bool __stdcall operator==(const CBString& s1, LPCSTR s2);
bool __stdcall operator==(LPCSTR s1, const CBString& s2);
bool __stdcall operator!=(const CBString& s1, const CBString& s2);
bool __stdcall operator!=(const CBString& s1, LPCSTR s2);
bool __stdcall operator!=(LPCSTR s1, const CBString& s2);
bool __stdcall operator<(const CBString& s1, const CBString& s2);
bool __stdcall operator<(const CBString& s1, LPCSTR s2);
bool __stdcall operator<(LPCSTR s1, const CBString& s2);
bool __stdcall operator>(const CBString& s1, const CBString& s2);
bool __stdcall operator>(const CBString& s1, LPCSTR s2);
bool __stdcall operator>(LPCSTR s1, const CBString& s2);
bool __stdcall operator<=(const CBString& s1, const CBString& s2);
bool __stdcall operator<=(const CBString& s1, LPCSTR s2);
bool __stdcall operator<=(LPCSTR s1, const CBString& s2);
bool __stdcall operator>=(const CBString& s1, const CBString& s2);
bool __stdcall operator>=(const CBString& s1, LPCSTR s2);
bool __stdcall operator>=(LPCSTR s1, const CBString& s2);


/////////////////////////////////////////////////////////////////////////////
// CBString Implementation

inline CBStringData* CBString::GetData() const
	{ ASSERT(m_pchData != NULL); return ((CBStringData*)m_pchData) - 1; }
inline void CBString::Init()
	{ m_pchData = _GetEmptyString().m_pchData; }
inline CBString::CBString(const unsigned char* lpsz)
	{ Init(); *this = (LPCSTR)lpsz; }
inline const CBString& CBString::operator=(const unsigned char* lpsz)
	{ *this = (LPCSTR)lpsz; return *this; }

inline int CBString::GetLength() const
	{ return GetData()->nDataLength; }
inline int CBString::GetAllocLength() const
	{ return GetData()->nAllocLength; }
inline bool CBString::IsEmpty() const
	{ return GetData()->nDataLength == 0; }
inline CBString::operator LPCSTR() const
	{ return m_pchData; }
inline int PASCAL CBString::SafeStrlen(LPCSTR lpsz)
	{ return (lpsz == NULL) ? 0 : (int)strlen(lpsz); }

// CBString support (windows specific)
inline int CBString::Compare(LPCSTR lpsz) const
	{ return strcmp(m_pchData, lpsz); }    // MBCS/Unicode aware
inline int CBString::CompareNoCase(LPCSTR lpsz) const
	{ return stricmp(m_pchData, lpsz); }   // MBCS/Unicode aware

inline char CBString::GetAt(int nIndex) const
{
	ASSERT(nIndex >= 0);
	ASSERT(nIndex < GetData()->nDataLength);
	return m_pchData[nIndex];
}
inline char CBString::operator[](int nIndex) const
{
	// same as GetAt
	ASSERT(nIndex >= 0);
	ASSERT(nIndex < GetData()->nDataLength);
	return m_pchData[nIndex];
}
inline bool __stdcall operator==(const CBString& s1, const CBString& s2)
	{ return s1.Compare(s2) == 0; }
inline bool __stdcall operator==(const CBString& s1, LPCSTR s2)
	{ return s1.Compare(s2) == 0; }
inline bool __stdcall operator==(LPCSTR s1, const CBString& s2)
	{ return s2.Compare(s1) == 0; }
inline bool __stdcall operator!=(const CBString& s1, const CBString& s2)
	{ return s1.Compare(s2) != 0; }
inline bool __stdcall operator!=(const CBString& s1, LPCSTR s2)
	{ return s1.Compare(s2) != 0; }
inline bool __stdcall operator!=(LPCSTR s1, const CBString& s2)
	{ return s2.Compare(s1) != 0; }
inline bool __stdcall operator<(const CBString& s1, const CBString& s2)
	{ return s1.Compare(s2) < 0; }
inline bool __stdcall operator<(const CBString& s1, LPCSTR s2)
	{ return s1.Compare(s2) < 0; }
inline bool __stdcall operator<(LPCSTR s1, const CBString& s2)
	{ return s2.Compare(s1) > 0; }
inline bool __stdcall operator>(const CBString& s1, const CBString& s2)
	{ return s1.Compare(s2) > 0; }
inline bool __stdcall operator>(const CBString& s1, LPCSTR s2)
	{ return s1.Compare(s2) > 0; }
inline bool __stdcall operator>(LPCSTR s1, const CBString& s2)
	{ return s2.Compare(s1) < 0; }
inline bool __stdcall operator<=(const CBString& s1, const CBString& s2)
	{ return s1.Compare(s2) <= 0; }
inline bool __stdcall operator<=(const CBString& s1, LPCSTR s2)
	{ return s1.Compare(s2) <= 0; }
inline bool __stdcall operator<=(LPCSTR s1, const CBString& s2)
	{ return s2.Compare(s1) >= 0; }
inline bool __stdcall operator>=(const CBString& s1, const CBString& s2)
	{ return s1.Compare(s2) >= 0; }
inline bool __stdcall operator>=(const CBString& s1, LPCSTR s2)
	{ return s1.Compare(s2) >= 0; }
inline bool __stdcall operator>=(LPCSTR s1, const CBString& s2)
	{ return s2.Compare(s1) <= 0; }

inline CBString::CBString()
{
	Init();
}

inline CBString::CBString(const CBString& stringSrc)
{
	ASSERT(stringSrc.GetData()->nRefs != 0);
	if (stringSrc.GetData()->nRefs >= 0)
	{
		ASSERT(stringSrc.GetData() != _tmpDataNil);
		m_pchData = stringSrc.m_pchData;
		InterlockedIncrement(&GetData()->nRefs);
	}
	else
	{
		Init();
		*this = stringSrc.m_pchData;
	}
}

inline bool CBString::AllocBuffer(int nLen)
// always allocate one extra character for '\0' termination
// assumes [optimistically] that data length will equal allocation length
{
	ASSERT(nLen >= 0);
	ASSERT(nLen <= INT_MAX - 1);    // max size (enough room for 1 extra)

	if (nLen == 0)
	{
		Init();
	}
	else
	{
		CBStringData* pData = NULL;
		pData = (CBStringData*)new BYTE[sizeof(CBStringData) + (nLen + 1) * sizeof(char)];
		if(pData == NULL)
			return FALSE;

		pData->nRefs = 1;
		pData->data()[nLen] = '\0';
		pData->nDataLength = nLen;
		pData->nAllocLength = nLen;
		m_pchData = pData->data();
	}

	return TRUE;
}

inline void CBString::Release()
{
	if (GetData() != _tmpDataNil)
	{
		ASSERT(GetData()->nRefs != 0);
		if (InterlockedDecrement(&GetData()->nRefs) <= 0)
			delete[] (BYTE*)GetData();
		Init();
	}
}

inline void PASCAL CBString::Release(CBStringData* pData)
{
	if (pData != _tmpDataNil)
	{
		ASSERT(pData->nRefs != 0);
		if (InterlockedDecrement(&pData->nRefs) <= 0)
			delete[] (BYTE*)pData;
	}
}

inline void CBString::Empty()
{
	if (GetData()->nDataLength == 0)
		return;

	if (GetData()->nRefs >= 0)
		Release();
	else
		*this = "";

	ASSERT(GetData()->nDataLength == 0);
	ASSERT(GetData()->nRefs < 0 || GetData()->nAllocLength == 0);
}

inline void CBString::CopyBeforeWrite()
{
	if (GetData()->nRefs > 1)
	{
		CBStringData* pData = GetData();
		Release();
		if(AllocBuffer(pData->nDataLength))
			memcpy(m_pchData, pData->data(), (pData->nDataLength + 1) * sizeof(char));
	}
	ASSERT(GetData()->nRefs <= 1);
}

inline bool CBString::AllocBeforeWrite(int nLen)
{
	bool bRet = TRUE;
	if (GetData()->nRefs > 1 || nLen > GetData()->nAllocLength)
	{
		Release();
		bRet = AllocBuffer(nLen);
	}
	ASSERT(GetData()->nRefs <= 1);
	return bRet;
}

inline CBString::~CBString()
//  free any attached data
{
	if (GetData() != _tmpDataNil)
	{
		if (InterlockedDecrement(&GetData()->nRefs) <= 0)
			delete[] (BYTE*)GetData();
	}
}

inline void CBString::AllocCopy(CBString& dest, int nCopyLen, int nCopyIndex,
	 int nExtraLen) const
{
	// will clone the data attached to this string
	// allocating 'nExtraLen' characters
	// Places results in uninitialized string 'dest'
	// Will copy the part or all of original data to start of new string

	int nNewLen = nCopyLen + nExtraLen;
	if (nNewLen == 0)
	{
		dest.Init();
	}
	else
	{
		if(dest.AllocBuffer(nNewLen))
			memcpy(dest.m_pchData, m_pchData + nCopyIndex, nCopyLen * sizeof(char));
	}
}

inline CBString::CBString(LPCSTR lpsz)
{
	Init();
	int nLen = SafeStrlen(lpsz);
	if (nLen != 0)
	{
		if(AllocBuffer(nLen))
			memcpy(m_pchData, lpsz, nLen * sizeof(char));
	}
}

// Assignment operators
//  All assign a new value to the string
//      (a) first see if the buffer is big enough
//      (b) if enough room, copy on top of old buffer, set size and type
//      (c) otherwise free old string data, and create a new one
//
//  All routines return the new string (but as a 'const CBString&' so that
//      assigning it again will cause a copy, eg: s1 = s2 = "hi there".
//

inline void CBString::AssignCopy(int nSrcLen, LPCSTR lpszSrcData)
{
	if(AllocBeforeWrite(nSrcLen))
	{
		memcpy(m_pchData, lpszSrcData, nSrcLen * sizeof(char));
		GetData()->nDataLength = nSrcLen;
		m_pchData[nSrcLen] = '\0';
	}
}

inline const CBString& CBString::operator=(const CBString& stringSrc)
{
	if (m_pchData != stringSrc.m_pchData)
	{
		if ((GetData()->nRefs < 0 && GetData() != _tmpDataNil) || stringSrc.GetData()->nRefs < 0)
		{
			// actual copy necessary since one of the strings is locked
			AssignCopy(stringSrc.GetData()->nDataLength, stringSrc.m_pchData);
		}
		else
		{
			// can just copy references around
			Release();
			ASSERT(stringSrc.GetData() != _tmpDataNil);
			m_pchData = stringSrc.m_pchData;
			InterlockedIncrement(&GetData()->nRefs);
		}
	}
	return *this;
}

inline const CBString& CBString::operator=(LPCSTR lpsz)
{
	ASSERT(lpsz == NULL || _IsValidString(lpsz));
	AssignCopy(SafeStrlen(lpsz), lpsz);
	return *this;
}

// Concatenation
// NOTE: "operator+" is done as friend functions for simplicity
//      There are three variants:
//          CBString + CBString
// and for ? = char, LPCSTR
//          CBString + ?
//          ? + CBString

inline bool CBString::ConcatCopy(int nSrc1Len, LPCSTR lpszSrc1Data,
	int nSrc2Len, LPCSTR lpszSrc2Data)
{
  // -- master concatenation routine
  // Concatenate two sources
  // -- assume that 'this' is a new CBString object

	bool bRet = TRUE;
	int nNewLen = nSrc1Len + nSrc2Len;
	if (nNewLen != 0)
	{
		bRet = AllocBuffer(nNewLen);
		if (bRet)
		{
			memcpy(m_pchData, lpszSrc1Data, nSrc1Len * sizeof(char));
			memcpy(m_pchData + nSrc1Len, lpszSrc2Data, nSrc2Len * sizeof(char));
		}
	}
	return bRet;
}

inline CBString __stdcall operator+(const CBString& string1, const CBString& string2)
{
	CBString s;
	s.ConcatCopy(string1.GetData()->nDataLength, string1.m_pchData, string2.GetData()->nDataLength, string2.m_pchData);
	return s;
}

inline CBString __stdcall operator+(const CBString& string, LPCSTR lpsz)
{
	ASSERT(lpsz == NULL || CBString::_IsValidString(lpsz));
	CBString s;
	s.ConcatCopy(string.GetData()->nDataLength, string.m_pchData, CBString::SafeStrlen(lpsz), lpsz);
	return s;
}

inline CBString __stdcall operator+(LPCSTR lpsz, const CBString& string)
{
	ASSERT(lpsz == NULL || CBString::_IsValidString(lpsz));
	CBString s;
	s.ConcatCopy(CBString::SafeStrlen(lpsz), lpsz, string.GetData()->nDataLength, string.m_pchData);
	return s;
}

inline void CBString::ConcatInPlace(int nSrcLen, LPCSTR lpszSrcData)
{
	//  -- the main routine for += operators

	// concatenating an empty string is a no-op!
	if (nSrcLen == 0)
		return;

	// if the buffer is too small, or we have a width mis-match, just
	//   allocate a new buffer (slow but sure)
	if (GetData()->nRefs > 1 || GetData()->nDataLength + nSrcLen > GetData()->nAllocLength)
	{
		// we have to grow the buffer, use the ConcatCopy routine
		CBStringData* pOldData = GetData();
		if (ConcatCopy(GetData()->nDataLength, m_pchData, nSrcLen, lpszSrcData))
		{
			ASSERT(pOldData != NULL);
			CBString::Release(pOldData);
		}
	}
	else
	{
		// fast concatenation when buffer big enough
		memcpy(m_pchData + GetData()->nDataLength, lpszSrcData, nSrcLen * sizeof(char));
		GetData()->nDataLength += nSrcLen;
		ASSERT(GetData()->nDataLength <= GetData()->nAllocLength);
		m_pchData[GetData()->nDataLength] = '\0';
	}
}

inline const CBString& CBString::operator+=(LPCSTR lpsz)
{
	ASSERT(lpsz == NULL || _IsValidString(lpsz));
	ConcatInPlace(SafeStrlen(lpsz), lpsz);
	return *this;
}

inline const CBString& CBString::operator+=(char ch)
{
	ConcatInPlace(1, &ch);
	return *this;
}

inline const CBString& CBString::operator+=(const CBString& string)
{
	ConcatInPlace(string.GetData()->nDataLength, string.m_pchData);
	return *this;
}

inline LPTSTR CBString::GetBuffer(int nMinBufLength)
{
	ASSERT(nMinBufLength >= 0);

	if (GetData()->nRefs > 1 || nMinBufLength > GetData()->nAllocLength)
	{
		// we have to grow the buffer
		CBStringData* pOldData = GetData();
		int nOldLen = GetData()->nDataLength;   // AllocBuffer will tromp it
		if (nMinBufLength < nOldLen)
			nMinBufLength = nOldLen;

		if(!AllocBuffer(nMinBufLength))
			return NULL;

		memcpy(m_pchData, pOldData->data(), (nOldLen + 1) * sizeof(char));
		GetData()->nDataLength = nOldLen;
		CBString::Release(pOldData);
	}
	ASSERT(GetData()->nRefs <= 1);

	// return a pointer to the character storage for this string
	ASSERT(m_pchData != NULL);
	return m_pchData;
}

inline void CBString::ReleaseBuffer(int nNewLength)
{
	CopyBeforeWrite();  // just in case GetBuffer was not called

	if (nNewLength == -1)
		nNewLength = (int)strlen(m_pchData); // zero terminated

	ASSERT(nNewLength <= GetData()->nAllocLength);
	GetData()->nDataLength = nNewLength;
	m_pchData[nNewLength] = '\0';
}

inline LPTSTR CBString::GetBufferSetLength(int nNewLength)
{
	ASSERT(nNewLength >= 0);

	if(GetBuffer(nNewLength) == NULL)
		return NULL;

	GetData()->nDataLength = nNewLength;
	m_pchData[nNewLength] = '\0';
	return m_pchData;
}

inline void CBString::FreeExtra()
{
	ASSERT(GetData()->nDataLength <= GetData()->nAllocLength);
	if (GetData()->nDataLength != GetData()->nAllocLength)
	{
		CBStringData* pOldData = GetData();
		if(AllocBuffer(GetData()->nDataLength))
		{
			memcpy(m_pchData, pOldData->data(), pOldData->nDataLength * sizeof(char));
			ASSERT(m_pchData[GetData()->nDataLength] == '\0');
			CBString::Release(pOldData);
		}
	}
	ASSERT(GetData() != NULL);
}

inline LPTSTR CBString::LockBuffer()
{
	LPTSTR lpsz = GetBuffer(0);
	if(lpsz != NULL)
		GetData()->nRefs = -1;
	return lpsz;
}

inline void CBString::UnlockBuffer()
{
	ASSERT(GetData()->nRefs == -1);
	if (GetData() != _tmpDataNil)
		GetData()->nRefs = 1;
}

inline int CBString::Find(char ch) const
{
	return Find(ch, 0);
}

inline int CBString::Find(char ch, int nStart) const
{
	int nLength = GetData()->nDataLength;
	if (nStart >= nLength)
		return -1;

	// find first single character
	LPTSTR lpsz = strchr(m_pchData + nStart, (char)ch);

	// return -1 if not found and index otherwise
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

inline int CBString::FindOneOf(LPCSTR lpszCharSet) const
{
	ASSERT(_IsValidString(lpszCharSet));
	LPTSTR lpsz = strpbrk(m_pchData, lpszCharSet);
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

inline void CBString::MakeUpper()
{
	CopyBeforeWrite();
	strupr(m_pchData);
}

inline void CBString::MakeLower()
{
	CopyBeforeWrite();
	strlwr(m_pchData);
}

inline void CBString::MakeReverse()
{
	CopyBeforeWrite();
	strrev(m_pchData);
}

inline void CBString::SetAt(int nIndex, char ch)
{
	ASSERT(nIndex >= 0);
	ASSERT(nIndex < GetData()->nDataLength);

	CopyBeforeWrite();
	m_pchData[nIndex] = ch;
}

inline CBString::CBString(char ch, int nLength)
{
	Init();
	if (nLength >= 1)
	{
		if(AllocBuffer(nLength))
		{
			memset(m_pchData, ch, nLength);
		}
	}
}

inline CBString::CBString(LPCSTR lpch, int nLength)
{
	Init();
	if (nLength != 0)
	{
		if(AllocBuffer(nLength))
			memcpy(m_pchData, lpch, nLength * sizeof(char));
	}
}

inline const CBString& CBString::operator=(char ch)
{
	AssignCopy(1, &ch);
	return *this;
}

inline CBString __stdcall operator+(const CBString& string1, char ch)
{
	CBString s;
	s.ConcatCopy(string1.GetData()->nDataLength, string1.m_pchData, 1, &ch);
	return s;
}

inline CBString __stdcall operator+(char ch, const CBString& string)
{
	CBString s;
	s.ConcatCopy(1, &ch, string.GetData()->nDataLength, string.m_pchData);
	return s;
}

inline CBString CBString::Mid(int nFirst) const
{
	return Mid(nFirst, GetData()->nDataLength - nFirst);
}

inline CBString CBString::Mid(int nFirst, int nCount) const
{
	// out-of-bounds requests return sensible things
	if (nFirst < 0)
		nFirst = 0;
	if (nCount < 0)
		nCount = 0;

	if (nFirst + nCount > GetData()->nDataLength)
		nCount = GetData()->nDataLength - nFirst;
	if (nFirst > GetData()->nDataLength)
		nCount = 0;

	CBString dest;
	AllocCopy(dest, nCount, nFirst, 0);
	return dest;
}

inline CBString CBString::Right(int nCount) const
{
	if (nCount < 0)
		nCount = 0;
	else if (nCount > GetData()->nDataLength)
		nCount = GetData()->nDataLength;

	CBString dest;
	AllocCopy(dest, nCount, GetData()->nDataLength-nCount, 0);
	return dest;
}

inline CBString CBString::Left(int nCount) const
{
	if (nCount < 0)
		nCount = 0;
	else if (nCount > GetData()->nDataLength)
		nCount = GetData()->nDataLength;

	CBString dest;
	AllocCopy(dest, nCount, 0, 0);
	return dest;
}

// strspn equivalent
inline CBString CBString::SpanIncluding(LPCSTR lpszCharSet) const
{
	ASSERT(_IsValidString(lpszCharSet));
	return Left((int)strspn(m_pchData, lpszCharSet));
}

// strcspn equivalent
inline CBString CBString::SpanExcluding(LPCSTR lpszCharSet) const
{
	ASSERT(_IsValidString(lpszCharSet));
	return Left((int)strcspn(m_pchData, lpszCharSet));
}

inline int CBString::ReverseFind(char ch) const
{
	// find last single character
	LPTSTR lpsz = strrchr(m_pchData, (char)ch);

	// return -1 if not found, distance from beginning otherwise
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

// find a sub-string (like strstr)
inline int CBString::Find(LPCSTR lpszSub) const
{
	return Find(lpszSub, 0);
}

inline int CBString::Find(LPCSTR lpszSub, int nStart) const
{
	ASSERT(_IsValidString(lpszSub));

	int nLength = GetData()->nDataLength;
	if (nStart > nLength)
		return -1;

	// find first matching substring
	LPTSTR lpsz = strstr(m_pchData + nStart, lpszSub);

	// return -1 for not found, distance from beginning otherwise
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

inline bool CBString::FormatV(LPCSTR lpszFormat, va_list argList)
{
	ASSERT(_IsValidString(lpszFormat));

	va_list argListSave = argList;

	// make a guess at the maximum length of the resulting string
	int nMaxLen = 0;
	for (LPCSTR lpsz = lpszFormat; *lpsz != '\0'; lpsz++)
	{
		// handle '%' character, but watch out for '%%'
		if (*lpsz != '%' || *(++lpsz) == '%')
		{
			// this is instead of strlen()
			nMaxLen++;
			continue;
		}

		int nItemLen = 0;

		// handle '%' character with format
		int nWidth = 0;
		for (; *lpsz != '\0'; lpsz++)
		{
			// check for valid flags
			if (*lpsz == '#')
				nMaxLen += 2;   // for '0x'
			else if (*lpsz == '*')
				nWidth = va_arg(argList, int);
			else if (*lpsz == '-' || *lpsz == '+' || *lpsz == '0' || *lpsz == ' ')
				;
			else // hit non-flag character
				break;
		}
		// get width and skip it
		if (nWidth == 0)
		{
			// width indicated by
			nWidth = atoi(lpsz);
			for (; *lpsz != '\0' && isdigit(*lpsz); lpsz++)
				;
		}
		ASSERT(nWidth >= 0);

		int nPrecision = 0;
		if (*lpsz == '.')
		{
			// skip past '.' separator (width.precision)
			lpsz++;

			// get precision and skip it
			if (*lpsz == '*')
			{
				nPrecision = va_arg(argList, int);
				lpsz++;
			}
			else
			{
				nPrecision = atoi(lpsz);
				for (; *lpsz != '\0' && isdigit(*lpsz); lpsz++)
					;
			}
			ASSERT(nPrecision >= 0);
		}

		// should be on type modifier or specifier
		switch (*lpsz)
		{
			// modifiers that affect size
			case 'h':
			case 'l':
			case 'F':
			case 'N':
			case 'L':
				lpsz++;
				break;
		}

		// now should be on specifier
		switch (*lpsz) {
			// single characters
			case 'c':
			case 'C':
				nItemLen = 2;
				va_arg(argList, char);
				break;
	
			// strings
			case 's':
			case 'S':
			{
				LPCSTR pstrNextArg = va_arg(argList, LPCSTR);
				if (pstrNextArg == NULL) {
					nItemLen = 6;  // "(null)"
				} else {
					nItemLen = (int)strlen(pstrNextArg);
					nItemLen = max(1, nItemLen);
				}
				break;
			}
		}
		// adjust nItemLen for strings
		if (nItemLen != 0) {
			nItemLen = max(nItemLen, nWidth);
			if (nPrecision != 0)
				nItemLen = min(nItemLen, nPrecision);
		} else {
			switch (*lpsz)
			{
				// integers
				case 'd':
				case 'i':
				case 'u':
				case 'x':
				case 'X':
				case 'o':
					va_arg(argList, int);
					nItemLen = 32;
					nItemLen = max(nItemLen, nWidth + nPrecision);
					break;
	
				case 'e':
				case 'g':
				case 'G':
					va_arg(argList, double);
					nItemLen = 128;
					nItemLen = max(nItemLen, nWidth + nPrecision);
					break;
				case 'f':
				{
					double f;
					LPTSTR pszTemp;
	
					// 312 == strlen("-1+(309 zeroes).")
					// 309 zeroes == max precision of a double
					// 6 == adjustment in case precision is not specified,
					//   which means that the precision defaults to 6
					pszTemp = (LPTSTR)new char[max(nWidth, 312 + nPrecision + 6)];
					f = va_arg(argList, double);
					sprintf(pszTemp, "%*.*f", nWidth, nPrecision + 6, f);
					nItemLen = (int)strlen(pszTemp);
					delete []pszTemp;

					break;
				}

				case 'p':
					va_arg(argList, LPVOID);
					nItemLen = 32;
					nItemLen = max(nItemLen, nWidth + nPrecision);
					break;
	
				// no output
				case 'n':
					va_arg(argList, int*);
					break;
	
				default:
					ASSERT(FALSE);  // unknown formatting option
			}
		}

		// adjust nMaxLen for output nItemLen
		nMaxLen += nItemLen;
	}

	if(GetBuffer(nMaxLen) == NULL)
		return FALSE;
	int nRet = vsprintf(m_pchData, lpszFormat, argListSave);
	nRet;	// ref
	ASSERT(nRet <= GetAllocLength());
	ReleaseBuffer();

	va_end(argListSave);
	return TRUE;
}

// formatting (using sprintf style formatting)
inline bool __cdecl CBString::Format(LPCSTR lpszFormat, ...)
{
	ASSERT(_IsValidString(lpszFormat));

	va_list argList;
	va_start(argList, lpszFormat);
	bool bRet = FormatV(lpszFormat, argList);
	va_end(argList);
	return bRet;
}

inline void CBString::Trim() {
	TrimLeft();
	TrimRight();
}
inline void CBString::TrimRight()
{
	CopyBeforeWrite();

	// find beginning of trailing spaces by starting at beginning (DBCS aware)
	LPTSTR lpsz = m_pchData;
	LPTSTR lpszLast = NULL;
	while (*lpsz != '\0')
	{
		if (isspace(*lpsz))
		{
			if (lpszLast == NULL)
				lpszLast = lpsz;
		}
		else
		{
			lpszLast = NULL;
		}
		lpsz++;
	}

	if (lpszLast != NULL)
	{
		// truncate at trailing space start
		*lpszLast = '\0';
		GetData()->nDataLength = (int)(DWORD_PTR)(lpszLast - m_pchData);
	}
}

inline void CBString::TrimLeft()
{
	CopyBeforeWrite();

	// find first non-space character
	LPCSTR lpsz = m_pchData;
	while (isspace(*lpsz))
		lpsz++;

	// fix up data and length
	int nDataLength = GetData()->nDataLength - (int)(DWORD_PTR)(lpsz - m_pchData);
	memmove(m_pchData, lpsz, (nDataLength + 1) * sizeof(char));
	GetData()->nDataLength = nDataLength;
}

inline void CBString::TrimRight(LPCSTR lpszTargetList)
{
	// find beginning of trailing matches
	// by starting at beginning (DBCS aware)

	CopyBeforeWrite();
	LPTSTR lpsz = m_pchData;
	LPTSTR lpszLast = NULL;

	while (*lpsz != '\0')
	{
		if (strchr(lpszTargetList, *lpsz) != NULL)
		{
			if (lpszLast == NULL)
				lpszLast = lpsz;
		}
		else
			lpszLast = NULL;
		lpsz++;
	}

	if (lpszLast != NULL)
	{
		// truncate at left-most matching character
		*lpszLast = '\0';
		GetData()->nDataLength = (int)(DWORD_PTR)(lpszLast - m_pchData);
	}
}

inline void CBString::TrimRight(char chTarget)
{
	// find beginning of trailing matches
	// by starting at beginning (DBCS aware)

	CopyBeforeWrite();
	LPTSTR lpsz = m_pchData;
	LPTSTR lpszLast = NULL;

	while (*lpsz != '\0')
	{
		if (*lpsz == chTarget)
		{
			if (lpszLast == NULL)
				lpszLast = lpsz;
		}
		else
			lpszLast = NULL;
		lpsz++;
	}

	if (lpszLast != NULL)
	{
		// truncate at left-most matching character
		*lpszLast = '\0';
		GetData()->nDataLength = (int)(DWORD_PTR)(lpszLast - m_pchData);
	}
}

inline void CBString::TrimLeft(LPCSTR lpszTargets)
{
	// if we're not trimming anything, we're not doing any work
	if (SafeStrlen(lpszTargets) == 0)
		return;

	CopyBeforeWrite();
	LPCSTR lpsz = m_pchData;

	while (*lpsz != '\0')
	{
		if (strchr(lpszTargets, *lpsz) == NULL)
			break;
		lpsz++;
	}

	if (lpsz != m_pchData)
	{
		// fix up data and length
		int nDataLength = GetData()->nDataLength - (int)(DWORD_PTR)(lpsz - m_pchData);
		memmove(m_pchData, lpsz, (nDataLength + 1) * sizeof(char));
		GetData()->nDataLength = nDataLength;
	}
}

inline void CBString::TrimLeft(char chTarget)
{
	// find first non-matching character

	CopyBeforeWrite();
	LPCSTR lpsz = m_pchData;

	while (chTarget == *lpsz)
		lpsz++;

	if (lpsz != m_pchData)
	{
		// fix up data and length
		int nDataLength = GetData()->nDataLength - (int)(DWORD_PTR)(lpsz - m_pchData);
		memmove(m_pchData, lpsz, (nDataLength + 1) * sizeof(char));
		GetData()->nDataLength = nDataLength;
	}
}

inline int CBString::Delete(int nIndex, int nCount /* = 1 */)
{
	if (nIndex < 0)
		nIndex = 0;
	int nNewLength = GetData()->nDataLength;
	if (nCount > 0 && nIndex < nNewLength)
	{
		CopyBeforeWrite();
		int nBytesToCopy = nNewLength - (nIndex + nCount) + 1;

		memmove(m_pchData + nIndex, m_pchData + nIndex + nCount, nBytesToCopy * sizeof(char));
		GetData()->nDataLength = nNewLength - nCount;
	}

	return nNewLength;
}

inline int CBString::Insert(int nIndex, char ch)
{
	CopyBeforeWrite();

	if (nIndex < 0)
		nIndex = 0;

	int nNewLength = GetData()->nDataLength;
	if (nIndex > nNewLength)
		nIndex = nNewLength;
	nNewLength++;

	if (GetData()->nAllocLength < nNewLength)
	{
		CBStringData* pOldData = GetData();
		LPTSTR pstr = m_pchData;
		if(!AllocBuffer(nNewLength))
			return -1;
		memcpy(m_pchData, pstr, (pOldData->nDataLength + 1) * sizeof(char));
		CBString::Release(pOldData);
	}

	// move existing bytes down
	memmove(m_pchData + nIndex + 1, m_pchData + nIndex, (nNewLength - nIndex) * sizeof(char));
	m_pchData[nIndex] = ch;
	GetData()->nDataLength = nNewLength;

	return nNewLength;
}

inline int CBString::Insert(int nIndex, LPCSTR pstr)
{
	if (nIndex < 0)
		nIndex = 0;

	int nInsertLength = SafeStrlen(pstr);
	int nNewLength = GetData()->nDataLength;
	if (nInsertLength > 0)
	{
		CopyBeforeWrite();
		if (nIndex > nNewLength)
			nIndex = nNewLength;
		nNewLength += nInsertLength;

		if (GetData()->nAllocLength < nNewLength)
		{
			CBStringData* pOldData = GetData();
			LPTSTR pstr = m_pchData;
			if(!AllocBuffer(nNewLength))
				return -1;
			memcpy(m_pchData, pstr, (pOldData->nDataLength + 1) * sizeof(char));
			CBString::Release(pOldData);
		}

		// move existing bytes down
		memmove(m_pchData + nIndex + nInsertLength, m_pchData + nIndex, (nNewLength - nIndex - nInsertLength + 1) * sizeof(char));
		memcpy(m_pchData + nIndex, pstr, nInsertLength * sizeof(char));
		GetData()->nDataLength = nNewLength;
	}

	return nNewLength;
}

inline int CBString::Replace(char chOld, char chNew)
{
	int nCount = 0;

	// short-circuit the nop case
	if (chOld != chNew)
	{
		// otherwise modify each character that matches in the string
		CopyBeforeWrite();
		LPTSTR psz = m_pchData;
		LPTSTR pszEnd = psz + GetData()->nDataLength;
		while (psz < pszEnd)
		{
			// replace instances of the specified character only
			if (*psz == chOld)
			{
				*psz = chNew;
				nCount++;
			}
			psz++;
		}
	}
	return nCount;
}

inline int CBString::Replace(LPCSTR lpszOld, LPCSTR lpszNew)
{
	// can't have empty or NULL lpszOld

	int nSourceLen = SafeStrlen(lpszOld);
	if (nSourceLen == 0)
		return 0;
	int nReplacementLen = SafeStrlen(lpszNew);

	// loop once to figure out the size of the result string
	int nCount = 0;
	LPTSTR lpszStart = m_pchData;
	LPTSTR lpszEnd = m_pchData + GetData()->nDataLength;
	LPTSTR lpszTarget;
	while (lpszStart < lpszEnd)
	{
		while ((lpszTarget = strstr(lpszStart, lpszOld)) != NULL)
		{
			nCount++;
			lpszStart = lpszTarget + nSourceLen;
		}
		lpszStart += strlen(lpszStart) + 1;
	}

	// if any changes were made, make them
	if (nCount > 0)
	{
		CopyBeforeWrite();

		// if the buffer is too small, just
		//   allocate a new buffer (slow but sure)
		int nOldLength = GetData()->nDataLength;
		int nNewLength =  nOldLength + (nReplacementLen - nSourceLen) * nCount;
		if (GetData()->nAllocLength < nNewLength || GetData()->nRefs > 1)
		{
			CBStringData* pOldData = GetData();
			LPTSTR pstr = m_pchData;
			if(!AllocBuffer(nNewLength))
				return -1;
			memcpy(m_pchData, pstr, pOldData->nDataLength * sizeof(char));
			CBString::Release(pOldData);
		}
		// else, we just do it in-place
		lpszStart = m_pchData;
		lpszEnd = m_pchData + GetData()->nDataLength;

		// loop again to actually do the work
		while (lpszStart < lpszEnd)
		{
			while ( (lpszTarget = strstr(lpszStart, lpszOld)) != NULL)
			{
				int nBalance = nOldLength - ((int)(DWORD_PTR)(lpszTarget - m_pchData) + nSourceLen);
				memmove(lpszTarget + nReplacementLen, lpszTarget + nSourceLen, nBalance * sizeof(char));
				memcpy(lpszTarget, lpszNew, nReplacementLen * sizeof(char));
				lpszStart = lpszTarget + nReplacementLen;
				lpszStart[nBalance] = '\0';
				nOldLength += (nReplacementLen - nSourceLen);
			}
			lpszStart += strlen(lpszStart) + 1;
		}
		ASSERT(m_pchData[nNewLength] == '\0');
		GetData()->nDataLength = nNewLength;
	}

	return nCount;
}

inline int CBString::Remove(char chRemove)
{
	CopyBeforeWrite();

	LPTSTR pstrSource = m_pchData;
	LPTSTR pstrDest = m_pchData;
	LPTSTR pstrEnd = m_pchData + GetData()->nDataLength;

	while (pstrSource < pstrEnd)
	{
		if (*pstrSource != chRemove)
		{
			*pstrDest = *pstrSource;
			pstrDest++;
		}
		pstrSource++;
	}
	*pstrDest = '\0';
	int nCount = (int)(DWORD_PTR)(pstrSource - pstrDest);
	GetData()->nDataLength -= nCount;

	return nCount;
}

#endif // _NO_MISCSTRING

inline char * __cdecl stristr (
        const char * str1,
        const char * str2
        )
{
        char *cp = (char *) str1;
        char *s1, *s2;

        if ( !*str2 )
            return((char *)str1);

        while (*cp)
        {
                s1 = cp;
                s2 = (char *) str2;

                while ( *s1 && *s2 && !(tolower(*s1)-tolower(*s2)) )
                        s1++, s2++;

                if (!*s2)
                        return(cp);

                cp++;
        }

        return(NULL);

}

UINT RegGetProfileInt( LPCSTR pszRegistryKey, LPCSTR pszProfileName, LPCSTR pszSection, LPCSTR pszEntry, int nDefault );
LPCSTR RegGetProfileString( LPCSTR pszRegistryKey, LPCSTR pszProfileName, LPCSTR pszSection, LPCSTR pszEntry, LPSTR pszValue, LPCSTR pszDefault );
BOOL RegGetProfileBinary( LPCSTR pszRegistryKey, LPCSTR pszProfileName, LPCSTR pszSection, LPCSTR pszEntry,	BYTE** ppData, UINT* pBytes );
BOOL RegWriteProfileInt( LPCSTR pszRegistryKey, LPCSTR pszProfileName, LPCSTR pszSection, LPCSTR pszEntry, int nValue );
BOOL RegWriteProfileString( LPCSTR pszRegistryKey, LPCSTR pszProfileName, LPCSTR pszSection, LPCSTR pszEntry, LPCSTR pszValue );
BOOL RegWriteProfileBinary( LPCSTR pszRegistryKey, LPCSTR pszProfileName, LPCSTR pszSection, LPCSTR pszEntry, LPBYTE pData, UINT nBytes );
