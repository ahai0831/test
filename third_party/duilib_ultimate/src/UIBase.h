#ifndef __UIBASE_H__
#define __UIBASE_H__


#pragma once

namespace DuiLib {
/////////////////////////////////////////////////////////////////////////////////////
//

#define UI_WNDSTYLE_CONTAINER  (0)
#define UI_WNDSTYLE_FRAME      (WS_VISIBLE | WS_OVERLAPPEDWINDOW)
#define UI_WNDSTYLE_CHILD      (WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN)
#define UI_WNDSTYLE_DIALOG     (WS_VISIBLE | WS_POPUPWINDOW | WS_CAPTION | WS_DLGFRAME | WS_CLIPSIBLINGS | WS_CLIPCHILDREN)

#define UI_WNDSTYLE_EX_FRAME   (WS_EX_WINDOWEDGE)
#define UI_WNDSTYLE_EX_DIALOG  (WS_EX_TOOLWINDOW | WS_EX_DLGMODALFRAME)

#define UI_CLASSSTYLE_CONTAINER  (0)
#define UI_CLASSSTYLE_FRAME      (CS_VREDRAW | CS_HREDRAW)
#define UI_CLASSSTYLE_CHILD      (CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS | CS_SAVEBITS)
#define UI_CLASSSTYLE_DIALOG     (CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS | CS_SAVEBITS)


/////////////////////////////////////////////////////////////////////////////////////
//
#ifndef ASSERT
#define ASSERT(expr)  _ASSERTE(expr)
#endif

#ifdef _DEBUG
#ifndef TRACE
#define TRACE __Trace
#endif
#define TRACEMSG __TraceMsg
#else
#ifndef TRACE
#define TRACE
#endif
#define TRACEMSG _T("")
#endif

void UILIB_API __Trace(LPCTSTR pstrFormat, ...);
LPCTSTR UILIB_API __TraceMsg(UINT uMsg);

/////////////////////////////////////////////////////////////////////////////////////
//

class STRINGorID
{
public:
    STRINGorID(LPCTSTR lpString) : m_lpstr(lpString)
    { }
    STRINGorID(UINT nID) : m_lpstr(MAKEINTRESOURCE(nID))
    { }
    LPCTSTR m_lpstr;
};

/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CPoint : public tagPOINT
{
public:
    CPoint();
    CPoint(const POINT& src);
    CPoint(int x, int y);
    CPoint(LPARAM lParam);
};


/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CSize : public tagSIZE
{
public:
    CSize();
    CSize(const SIZE& src);
    CSize(const RECT rc);
    CSize(int cx, int cy);
};


/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CRect : public tagRECT
{
public:
    CRect();
    CRect(const RECT& src);
    CRect(int iLeft, int iTop, int iRight, int iBottom);

    int GetWidth() const;
    int GetHeight() const;
    void Empty();
    bool IsNull() const;
    void Join(const RECT& rc);
    void ResetOffset();
    void Normalize();
    void Offset(int cx, int cy);
    void Inflate(int cx, int cy);
    void Deflate(int cx, int cy);
    void Union(CRect& rc);
};

/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CStdPtrArray
{
public:
    CStdPtrArray(int iPreallocSize = 0);
	CStdPtrArray(const CStdPtrArray& src);
    ~CStdPtrArray();

    void Empty();
    void Resize(int iSize);
    bool IsEmpty() const;
    int Find(LPVOID iIndex) const;
    bool Add(LPVOID pData);
    bool SetAt(int iIndex, LPVOID pData);
    bool InsertAt(int iIndex, LPVOID pData);
    bool Remove(int iIndex);
    int GetSize() const;
    LPVOID* GetData();

    LPVOID GetAt(int iIndex) const;
    LPVOID operator[] (int nIndex) const;

protected:
    LPVOID* m_ppVoid;
    int m_nCount;
    int m_nAllocated;
};


/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CStdValArray
{
public:
    CStdValArray(int iElementSize, int iPreallocSize = 0);
    ~CStdValArray();

    void Empty();
    bool IsEmpty() const;
    bool Add(LPCVOID pData);
    bool Remove(int iIndex);
    int GetSize() const;
    LPVOID GetData();

    LPVOID GetAt(int iIndex) const;
    LPVOID operator[] (int nIndex) const;

protected:
    LPBYTE m_pVoid;
    int m_iElementSize;
    int m_nCount;
    int m_nAllocated;
};


/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CStdString
{
public:
    enum { MAX_LOCAL_STRING_LEN = 63 };

    CStdString();
    CStdString(const TCHAR ch);
    CStdString(const CStdString& src);
    CStdString(LPCTSTR lpsz, int nLen = -1);
    ~CStdString();

    void Empty();
    int GetLength() const;
    bool IsEmpty() const;
    TCHAR GetAt(int nIndex) const;
    void Append(LPCTSTR pstr);
    void Assign(LPCTSTR pstr, int nLength = 0);
    LPCTSTR GetData();

    void SetAt(int nIndex, TCHAR ch);
    operator LPCTSTR() const;

    TCHAR operator[] (int nIndex) const;
    const CStdString& operator=(const CStdString& src);
    const CStdString& operator=(const TCHAR ch);
    const CStdString& operator=(LPCTSTR pstr);
#ifndef _UNICODE
    const CStdString& CStdString::operator=(LPCWSTR lpwStr);
#endif
    CStdString operator+(const CStdString& src);
    CStdString operator+(LPCTSTR pstr);
    const CStdString& operator+=(const CStdString& src);
    const CStdString& operator+=(LPCTSTR pstr);
    const CStdString& operator+=(const TCHAR ch);

    bool operator == (LPCTSTR str) const;
    bool operator != (LPCTSTR str) const;
    bool operator <= (LPCTSTR str) const;
    bool operator <  (LPCTSTR str) const;
    bool operator >= (LPCTSTR str) const;
    bool operator >  (LPCTSTR str) const;

    int Compare(LPCTSTR pstr) const;
    int CompareNoCase(LPCTSTR pstr) const;

    void MakeUpper();
    void MakeLower();
	void TrimLeft();   // 去掉左边的空格

    CStdString Left(int nLength) const;
    CStdString Mid(int iPos, int nLength = -1) const;
    CStdString Right(int nLength) const;

    int Find(TCHAR ch, int iPos = 0) const;
    int Find(LPCTSTR pstr, int iPos = 0) const;
    int ReverseFind(TCHAR ch) const;
    int Replace(LPCTSTR pstrFrom, LPCTSTR pstrTo);

    int __cdecl Format(LPCTSTR pstrFormat, ...);
    int __cdecl SmallFormat(LPCTSTR pstrFormat, ...);



protected:
    LPTSTR m_pstr;
    TCHAR m_szBuffer[MAX_LOCAL_STRING_LEN + 1];
};


/////////////////////////////////////////////////////////////////////////////////////
//

struct TITEM
{
	CStdString Key;
	LPVOID Data;
	struct TITEM* pPrev;
	struct TITEM* pNext;
};

class UILIB_API CStdStringPtrMap
{
public:
	CStdStringPtrMap(int nSize = 83);
	~CStdStringPtrMap();

	void Resize(int nSize = 83);
	LPVOID Find(LPCTSTR key, bool optimize = true) const;
	bool Insert(LPCTSTR key, LPVOID pData);
	LPVOID Set(LPCTSTR key, LPVOID pData);
	bool Remove(LPCTSTR key);
	void RemoveAll();
	int GetSize() const;
	LPCTSTR GetAt(int iIndex) const;
	LPCTSTR operator[] (int nIndex) const;

protected:
	TITEM** m_aT;
	int m_nBuckets;
	int m_nCount;
};

template<typename T = LPVOID>
class UILIB_API TStdStringPtrMap : public CStdStringPtrMap
{
public:
	TStdStringPtrMap(int nSize = 83):CStdStringPtrMap(nSize){};
	T GetAtObj(int iIndex) const {
		LPCTSTR nkey = GetAt(iIndex);
		if(!nkey)
			return NULL;
		return Find(nkey);
	}
	T Find(LPCTSTR key, bool optimize = true) const{return static_cast<T>(CStdStringPtrMap::Find(key,optimize));};
	bool Insert(LPCTSTR key,T pData){return CStdStringPtrMap::Insert(key);};
	T Set(LPCTSTR key,T pData){return static_cast<T>(CStdStringPtrMap::Set(key,pData));};
};


/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CWindowWnd
{
public:
    CWindowWnd();

    HWND GetHWND() const;
    operator HWND() const;

    bool RegisterWindowClass();
    bool RegisterSuperclass();

    HWND Create(HWND hwndParent, LPCTSTR pstrName, DWORD dwStyle, DWORD dwExStyle, const RECT rc, HMENU hMenu = NULL);
    HWND Create(HWND hwndParent, LPCTSTR pstrName, DWORD dwStyle, DWORD dwExStyle, int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int cx = CW_USEDEFAULT, int cy = CW_USEDEFAULT, HMENU hMenu = NULL);
    HWND Subclass(HWND hWnd);
    void Unsubclass();
    void ShowWindow(bool bShow = true, bool bTakeFocus = true);
	void ResizeWindow(bool bShow);
    bool ShowModal();
    void Close();
	BOOL DestroyWindow();
    void CenterWindow();
	CPoint CenterWindowPoint(int nWidth, int nHeight);
    void SetIcon(UINT nRes);

    LRESULT SendMessage(UINT uMsg, WPARAM wParam = 0, LPARAM lParam = 0L);
    LRESULT PostMessage(UINT uMsg, WPARAM wParam = 0, LPARAM lParam = 0L);
    void ResizeClient(int cx = -1, int cy = -1);

	virtual void RightBottomWindow();  //右下角显示窗口
	virtual void ShellTraySideWindow(bool bTopMost = false);    // 在托盘区域附近显示窗口

public:

protected:
    virtual LPCTSTR GetWindowClassName() const = 0;
    virtual LPCTSTR GetSuperClassName() const;
    virtual UINT GetClassStyle() const;

    virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual void OnFinalMessage(HWND hWnd);

    static LRESULT CALLBACK __WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK __ControlProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK CtrlWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);

protected:
    HWND m_hWnd;
    WNDPROC m_OldWndProc;
    bool m_bSubclassed;
};


/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CWaitCursor
{
public:
    CWaitCursor();
    ~CWaitCursor();

protected:
    HCURSOR m_hOrigCursor;
};

/////////////////////////////////////////////////////////////////////////////////////
//

class CVariant : public VARIANT
{
public:
	CVariant() 
	{ 
		VariantInit(this); 
	}
	CVariant(int i)
	{
		VariantInit(this);
		this->vt = VT_I4;
		this->intVal = i;
	}
	CVariant(float f)
	{
		VariantInit(this);
		this->vt = VT_R4;
		this->fltVal = f;
	}
	CVariant(LPOLESTR s)
	{
		VariantInit(this);
		this->vt = VT_BSTR;
		this->bstrVal = s;
	}
	CVariant(IDispatch *disp)
	{
		VariantInit(this);
		this->vt = VT_DISPATCH;
		this->pdispVal = disp;
	}

	~CVariant() 
	{ 
		VariantClear(this); 
	}
};


} // namespace DuiLib

#endif // __UIBASE_H__
