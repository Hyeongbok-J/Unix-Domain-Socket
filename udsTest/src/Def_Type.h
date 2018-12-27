#ifndef Def_Type_h__
#define Def_Type_h__

#ifdef _MSC_VER
// VC로 compile하는 경우에는 CAtlStringA, 즉, CStringA를 사용하도록 한다.
#include <atlstr.h>
#else
// gcc로 compile하는 경우에는 아래 Class를 참고하도록 한다.
#include <string>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#endif

#include <malloc.h>
#include <vector>

#ifndef _MSC_VER
#include <unistd.h>		// sleep() 사용
#endif

// mkdir()
#ifdef _MSC_VER
#include <direct.h> 
#else
#include <sys/stat.h>
#endif

#pragma warning(disable : 4996)

//=============================================================================
// 기본형 정의
// Last Update		: 2017/07/04 - 17:44
// Desc.			: Linux 에서 사용될 데이터 타입 정의
//=============================================================================
#if defined(_WIN64)
typedef __int64 INT_PTR, *PINT_PTR;
typedef unsigned __int64 UINT_PTR, *PUINT_PTR;
typedef __int64 LONG_PTR, *PLONG_PTR;
typedef unsigned __int64 ULONG_PTR, *PULONG_PTR;
#define __int3264   __int64
#else
// [ToDo] _W64 가 붙어있었음
typedef int INT_PTR, *PINT_PTR;
typedef unsigned int UINT_PTR, *PUINT_PTR;
typedef long LONG_PTR, *PLONG_PTR;
typedef unsigned long ULONG_PTR, *PULONG_PTR;
#define __int3264   __int32
#endif

typedef LONG_PTR			LRESULT;
typedef int					INT;
typedef unsigned int		UINT;
typedef unsigned long long	DWORD64;// , *PDWORD64;
typedef const char			*LPCSTR, *PCSTR;
typedef char				*NPSTR, *LPSTR, *PSTR;
typedef double				DOUBLE;
typedef unsigned short		WORD;
typedef unsigned long		DWORD;
typedef unsigned char		BYTE;
typedef float				FLOAT;
typedef BYTE	           *PBYTE;
typedef BYTE				*LPBYTE;
typedef DWORD				COLORREF;

#ifndef BOOL
typedef int		BOOL;
#endif

#ifndef FALSE
#define FALSE	0
#endif

#ifndef TRUE
#define TRUE	1
#endif

#define LVCFMT_LEFT                 0x0000 // Same as HDF_LEFT
#define LVCFMT_RIGHT                0x0001 // Same as HDF_RIGHT
#define LVCFMT_CENTER               0x0002 // Same as HDF_CENTER
#define LVCFMT_JUSTIFYMASK          0x0003 // Same as HDF_JUSTIFYMASK

#ifndef VOID
#define VOID void
typedef char CHAR;
typedef short SHORT;
typedef long LONG;
#endif

#define CHAR_BIT      8         /* number of bits in a char */
#define SCHAR_MIN   (-128)      /* minimum signed char value */
#define SCHAR_MAX     127       /* maximum signed char value */
#define UCHAR_MAX     0xff      /* maximum unsigned char value */

#define M_PI       3.14159265358979323846

//=============================================================================
// 문자열 관련 타입 정의
// Last Update		: 2017/07/04 - 17:44
// Desc.			: Linux 에서 사용될 문자열 타입 정의
//=============================================================================
#ifndef _T(str)
#define _T(str)			(LPCTSTR)(str)
#endif

#ifndef _MAC
typedef wchar_t WCHAR;    // wc,   16-bit UNICODE character
#else
// some Macintosh compilers don't define wchar_t in a convenient location, or define it as a char
typedef unsigned short WCHAR;    // wc,   16-bit UNICODE character
#endif

#ifdef  UNICODE                     // r_winnt

#ifndef _TCHAR_DEFINED
typedef WCHAR TCHAR, *PTCHAR;
typedef WCHAR TBYTE, *PTBYTE;
#define _TCHAR_DEFINED
#endif /* !_TCHAR_DEFINED */

typedef _Null_terminated_ const WCHAR *LPCWSTR, *PCWSTR;
typedef LPCWCH LPCTCH, PCTCH;
typedef LPWSTR PTSTR, LPTSTR;
typedef LPCWSTR PCTSTR, LPCTSTR;
typedef LPUWSTR PUTSTR, LPUTSTR;
typedef LPCUWSTR PCUTSTR, LPCUTSTR;
typedef LPWSTR LP;
#define __TEXT(quote) L##quote      // r_winnt

#else   /* UNICODE */               // r_winnt

#ifndef _TCHAR_DEFINED
typedef char TCHAR, *PTCHAR;
typedef unsigned char TBYTE, *PTBYTE;
#define _TCHAR_DEFINED
#endif /* !_TCHAR_DEFINED */

typedef LPSTR PTSTR, LPTSTR, PUTSTR, LPUTSTR;
typedef LPCSTR PCTSTR, LPCTSTR, PCUTSTR, LPCUTSTR;
#define __TEXT(quote) quote         // r_winnt

#endif /* UNICODE */                // r_winnt

//=============================================================================
// 매크로 정의
// Last Update		: 2017/07/04 - 17:44
// Desc.			: Linux 에서 사용될 매크로 정의
//=============================================================================
#define RGB(r,g,b)		((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))
#define __max(a,b)		(((a) > (b)) ? (a) : (b))
#define __min(a,b)		(((a) < (b)) ? (a) : (b))

#define SAFE_DELETE(p)			{ if (p) { delete (p);		(p) = NULL;	} } 
#define SAFE_DELETE_ARRAY(p)	{ if (p) { delete[](p);		(p) = NULL;	} }
#define SAFE_RELEASE(p)			{ if (p) { (p)->Release();	(p) = NULL;	} }

#ifndef _MSC_VER
#define _msize(p)			malloc_usable_size(p)
#define Sleep(p)			sleep(p)
#define _mkdir(p)			mkdir(p, 0777)
#endif

//=============================================================================
// CPoint 클래스 정의
// Last Update		: 2017/07/04 - 17:44
// Desc.			: CPoint 클래스 정의
//=============================================================================
class CPoint
{
public:
	LONG  x;
	LONG  y;
public:
	// create an uninitialized point
	CPoint() throw();
	// create from two integers
	CPoint(
		int initX,
		int initY) throw();

	void SetPoint(
		int X,
		int Y) throw();
	BOOL operator==(CPoint point) const throw();
	BOOL operator!=(CPoint point) const throw();
	void operator+=(CPoint point) throw();
	void operator-=(CPoint point) throw();
};

// CPoint
inline CPoint::CPoint() throw()
{
	x = 0;
	y = 0;
}

inline CPoint::CPoint(
	int initX,
	int initY) throw()
{
	x = initX;
	y = initY;
}

inline void CPoint::SetPoint(
	int X,
	int Y) throw()
{
	x = X;
	y = Y;
}

inline BOOL CPoint::operator==(CPoint point) const throw()
{
	return (x == point.x && y == point.y);
}

inline BOOL CPoint::operator!=(CPoint point) const throw()
{
	return (x != point.x || y != point.y);
}

inline void CPoint::operator+=(CPoint point) throw()
{
	x += point.x;
	y += point.y;
}

inline void CPoint::operator-=(CPoint point) throw()
{
	x -= point.x;
	y -= point.y;
}

//=============================================================================
// CRect 클래스 정의
// Last Update		: 2017/07/04 - 17:44
// Desc.			: CRect 클래스 정의
//=============================================================================
class CRect
{
public:
	LONG    left;
	LONG    top;
	LONG    right;
	LONG    bottom;

public:
	// uninitialized rectangle
	CRect() throw();
	// from left, top, right, and bottom
	CRect(
		int l,
		int t,
		int r,
		int b) throw();

	// from two points
	CRect(
		CPoint topLeft,
		CPoint bottomRight) throw();

	// retrieves the width
	int Width() const throw();
	// returns the height
	int Height() const throw();
	// returns the size
	//CSize Size() const throw();
	// reference to the top-left point
	CPoint& TopLeft() throw();
	// reference to the bottom-right point
	CPoint& BottomRight() throw();
	// const reference to the top-left point
	const CPoint& TopLeft() const throw();
	// const reference to the bottom-right point
	const CPoint& BottomRight() const throw();
	// the geometric center point of the rectangle
	CPoint CenterPoint() const throw();


	// returns TRUE if rectangle is at (0,0) and has no area
	BOOL IsRectNull() const throw();

	// set rectangle from left, top, right, and bottom
	void SetRect(
		int x1,
		int y1,
		int x2,
		int y2) throw();
	void SetRect(
		CPoint topLeft,
		CPoint bottomRight) throw();
	// empty the rectangle
	void SetRectEmpty() throw();

	// translate the rectangle by moving its top and left
	void OffsetRect(
		int x,
		int y) throw();
	void OffsetRect(CPoint point) throw();

	// absolute position of rectangle
	void MoveToY(int y) throw();
	void MoveToX(int x) throw();
	void MoveToXY(
		int x,
		int y) throw();
	void MoveToXY(CPoint point) throw();
};

// CRect
inline CRect::CRect() throw()
{
	left = 0;
	top = 0;
	right = 0;
	bottom = 0;
}

inline CRect::CRect(
	int l,
	int t,
	int r,
	int b) throw()
{
	left = l;
	top = t;
	right = r;
	bottom = b;
}

inline CRect::CRect(
	CPoint topLeft,
	CPoint bottomRight) throw()
{
	left = topLeft.x;
	top = topLeft.y;
	right = bottomRight.x;
	bottom = bottomRight.y;
}

inline int CRect::Width() const throw()
{
	return right - left;
}

inline int CRect::Height() const throw()
{
	return bottom - top;
}

inline CPoint& CRect::TopLeft() throw()
{
	return *((CPoint*)this);
}

inline CPoint& CRect::BottomRight() throw()
{
	return *((CPoint*)this + 1);
}

inline const CPoint& CRect::TopLeft() const throw()
{
	return *((CPoint*)this);
}

inline const CPoint& CRect::BottomRight() const throw()
{
	return *((CPoint*)this + 1);
}

inline CPoint CRect::CenterPoint() const throw()
{
	return CPoint((left + right) / 2, (top + bottom) / 2);
}

inline BOOL CRect::IsRectNull() const throw()
{
	return (left == 0 && right == 0 && top == 0 && bottom == 0);
}

inline void CRect::SetRect(
	int x1,
	int y1,
	int x2,
	int y2) throw()
{
	left = x1;
	top = y1;
	right = x2;
	bottom = y2;
}

inline void CRect::SetRect(
	CPoint topLeft,
	CPoint bottomRight) throw()
{
	SetRect(topLeft.x, topLeft.y, bottomRight.x, bottomRight.y);
}

inline void CRect::SetRectEmpty() throw()
{
	left = 0;
	top = 0;
	right = 0;
	bottom = 0;
}

inline void CRect::MoveToY(int y) throw()
{
	bottom = Height() + y;
	top = y;
}

inline void CRect::MoveToX(int x) throw()
{
	right = Width() + x;
	left = x;
}

inline void CRect::MoveToXY(
	int x,
	int y) throw()
{
	MoveToX(x);
	MoveToY(y);
}

inline void CRect::MoveToXY(CPoint pt) throw()
{
	MoveToX(pt.x);
	MoveToY(pt.y);
}

inline void CRect::OffsetRect(
	int x,
	int y) throw()
{
	left += x;
	top += y;
	right += x;
	bottom += y;
}

inline void CRect::OffsetRect(CPoint point) throw()
{
	OffsetRect(point.x, point.y);
}

///// 구현은 완료지만 사용하지 않음, CArray -> std::vector<>로 바꾸는 방향으로 ///
////=============================================================================
//// CArray 클래스 정의
//// Last Update		: 2017/07/06 - 17:44
//// Desc.			: CArray 클래스 정의 (STL Vector 활용)
////	코드 호환을 위해 프로토타입만 동일하게 맞추고 내부 동작은 기존 클래스와 다름
////=============================================================================
//#include <vector>
//template<class TYPE, class ARG_TYPE = const TYPE&>
//class CArray
//{
//public:
//	// Construction
//	CArray();
//
//	// Attributes
//	INT_PTR GetSize() const;
//	INT_PTR GetCount() const;
//	BOOL IsEmpty() const;
//
//	// Operations
//	void RemoveAll();
//
//	// Accessing elements
//	const TYPE& GetAt(INT_PTR nIndex) const;
//	TYPE& GetAt(INT_PTR nIndex);
//	void SetAt(INT_PTR nIndex, ARG_TYPE newElement);
//
//	// Potentially growing the array
//	INT_PTR Add(ARG_TYPE newElement);
//
//	// overloaded operator helpers
//	const TYPE& operator[](INT_PTR nIndex) const;
//	TYPE& operator[](INT_PTR nIndex);
//
//	// Operations that move elements around
//	void InsertAt(INT_PTR nIndex, ARG_TYPE newElement, INT_PTR nCount = 1);
//	void RemoveAt(INT_PTR nIndex, INT_PTR nCount = 1);
//
//	// Implementation
//protected:
//	std::vector<TYPE*> m_pData;
//
//public:
//	~CArray();
//};
//
//template<class TYPE, class ARG_TYPE>
//CArray<TYPE, ARG_TYPE>::CArray()
//{
//	m_pData.clear();
//}
//
//template<class TYPE, class ARG_TYPE>
//CArray<TYPE, ARG_TYPE>::~CArray()
//{
//	RemoveAll();
//}
//
//template<class TYPE, class ARG_TYPE>
//inline INT_PTR CArray<TYPE, ARG_TYPE>::GetSize() const
//{
//	return m_pData.size();
//}
//template<class TYPE, class ARG_TYPE>
//inline INT_PTR CArray<TYPE, ARG_TYPE>::GetCount() const
//{
//	return m_pData.size();
//}
//template<class TYPE, class ARG_TYPE>
//inline BOOL CArray<TYPE, ARG_TYPE>::IsEmpty() const
//{
//	return m_pData.empty();
//}
//
//template<class TYPE, class ARG_TYPE>
//inline void CArray<TYPE, ARG_TYPE>::RemoveAll()
//{
//	typename std::vector<TYPE*>::iterator it;
//	for (int i = 0; i < m_pData.size(); i++)
//	{
//		it = m_pData.begin();
//		SAFE_DELETE(*it);
//		m_pData.erase(it);
//	}
//	m_pData.clear();
//}
//
//template<class TYPE, class ARG_TYPE>
//inline TYPE& CArray<TYPE, ARG_TYPE>::GetAt(INT_PTR nIndex)
//{
//	TYPE ret;
//	memset(&ret, 0, sizeof(ret));
//	if (nIndex >= 0 && m_pData.size() > 0 && m_pData.size() - 1 >= nIndex)
//		return *m_pData[nIndex];
//	else
//		return ret;
//}
//template<class TYPE, class ARG_TYPE>
//inline const TYPE& CArray<TYPE, ARG_TYPE>::GetAt(INT_PTR nIndex) const
//{
//	return (const TYPE*)GetAt(nIndex);
//}
//template<class TYPE, class ARG_TYPE>
//inline void CArray<TYPE, ARG_TYPE>::SetAt(INT_PTR nIndex, ARG_TYPE newElement)
//{
//	if (m_pData.size() - 1 >= nIndex && nIndex >= 0)
//		memcpy(m_pData[nIndex], &newElement, sizeof(newElement));
//}
//
//template<class TYPE, class ARG_TYPE>
//inline INT_PTR CArray<TYPE, ARG_TYPE>::Add(ARG_TYPE newElement)
//{
//	TYPE* buf = new TYPE;
//	memcpy(buf, &newElement, sizeof(newElement));
//	m_pData.push_back(buf);
//	return m_pData.size() - 1;
//}
//
//template<class TYPE, class ARG_TYPE>
//inline const TYPE& CArray<TYPE, ARG_TYPE>::operator[](INT_PTR nIndex) const
//{
//	return (const TYPE*)GetAt(nIndex);
//}
//template<class TYPE, class ARG_TYPE>
//inline TYPE& CArray<TYPE, ARG_TYPE>::operator[](INT_PTR nIndex)
//{
//	return GetAt(nIndex);
//}
//
//template<class TYPE, class ARG_TYPE>
//void CArray<TYPE, ARG_TYPE>::InsertAt(INT_PTR nIndex, ARG_TYPE newElement, INT_PTR nCount /*=1*/)
//{
//	if (nCount <= 0 || nIndex < 0)
//		return;
//
//	TYPE* buf = NULL;
//
//	// 중간
//	if (0 <= nIndex && nIndex < m_pData.size())
//	{
//		for (int i = 0; i < nCount; i++)
//		{
//			buf = new TYPE;
//			memcpy(buf, &newElement, sizeof(newElement));
//			m_pData.insert(m_pData.begin() + nIndex + i, buf);
//		}
//	}
//	// 비었을 때 인덱스 0번 or n개 중 인덱스 n번 일 경우
//	else if (nIndex == m_pData.size())
//	{
//		for (int i = 0; i < nCount; i++)
//			Add(newElement);
//	}
//	else
//		return;
//}
//
//template<class TYPE, class ARG_TYPE>
//void CArray<TYPE, ARG_TYPE>::RemoveAt(INT_PTR nIndex, INT_PTR nCount)
//{
//	if (nCount <= 0 || nIndex < 0 || m_pData.size() <= nIndex ||
//		m_pData.size() - nIndex < nCount)
//		return;
//
//	typename std::vector<TYPE*>::iterator it;
//	while (nCount > 0)
//	{
//		it = m_pData.begin() + nIndex;
//		SAFE_DELETE(*it);
//		m_pData.erase(it);
//		nCount--;
//	}
//}

#endif