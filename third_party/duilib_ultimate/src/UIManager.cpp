#include "StdAfx.h"
#include <zmouse.h>

DECLARE_HANDLE(HZIP);	// An HZIP identifies a zip file that has been opened
typedef DWORD ZRESULT;
#define OpenZip OpenZipU
#define CloseZip(hz) CloseZipU(hz)
extern HZIP OpenZipU(void *z,unsigned int len,DWORD flags);
extern ZRESULT CloseZipU(HZIP hz);

#pragma comment(lib, "gdiplus.lib")

static HMONITOR g_hMainMonitor = NULL;   // 主要的窗口所在的显示器,用于多显示器设置

namespace DuiLib {

/////////////////////////////////////////////////////////////////////////////////////
//
//

static UINT MapKeyState()
{
    UINT uState = 0;
    if( ::GetKeyState(VK_CONTROL) < 0 ) uState |= MK_CONTROL;
    if( ::GetKeyState(VK_RBUTTON) < 0 ) uState |= MK_LBUTTON;
    if( ::GetKeyState(VK_LBUTTON) < 0 ) uState |= MK_RBUTTON;
    if( ::GetKeyState(VK_SHIFT) < 0 ) uState |= MK_SHIFT;
    if( ::GetKeyState(VK_MENU) < 0 ) uState |= MK_ALT;
    return uState;
}

//csk?
#define IDB_ICONS16 200
#define IDB_ICONS24 201
#define IDB_ICONS32 202
#define IDB_ICONS50 203

typedef struct tagFINDTABINFO
{
    CControlUI* pFocus;
    CControlUI* pLast;
    bool bForward;
    bool bNextIsIt;
} FINDTABINFO;

typedef struct tagFINDSHORTCUT
{
    TCHAR ch;
    bool bPickNext;
} FINDSHORTCUT;

typedef struct tagTIMERINFO
{
    CControlUI* pSender;
    UINT nLocalID;
    HWND hWnd;
    UINT uWinTimer;
    bool bKilled;
} TIMERINFO;


/////////////////////////////////////////////////////////////////////////////////////
CAnimationSpooler m_anim;
HPEN m_hPens[UICOLOR__LAST] = { 0 };
HFONT m_hFonts[UIFONT__LAST] = { 0 };
HBRUSH m_hBrushes[UICOLOR__LAST] = { 0 };
LOGFONT m_aLogFonts[UIFONT__LAST] = { 0 };
COLORREF m_clrColors[UICOLOR__LAST][2] = { 0 };
TEXTMETRIC m_aTextMetrics[UIFONT__LAST] = { 0 };
HIMAGELIST m_himgIcons16 = NULL;
HIMAGELIST m_himgIcons24 = NULL;
HIMAGELIST m_himgIcons32 = NULL;
HIMAGELIST m_himgIcons50 = NULL;

HPEN m_hUpdateRectPen = NULL;
HINSTANCE CPaintManagerUI::m_hInstance = NULL;
HINSTANCE CPaintManagerUI::m_hResourceInstance = NULL;
CStdString CPaintManagerUI::m_pStrResourcePath;
CStdString CPaintManagerUI::m_pStrResourceZip;
bool CPaintManagerUI::m_bCachedResourceZip = false;
HANDLE CPaintManagerUI::m_hResourceZip = NULL;

short CPaintManagerUI::m_H = 180;
short CPaintManagerUI::m_S = 100;
short CPaintManagerUI::m_L = 100;
CStdPtrArray CPaintManagerUI::m_aPreMessages;

CPaintManagerUI::CPaintManagerUI() :
m_hWndPaint(NULL),
m_hDcPaint(NULL),
m_hDcOffscreen(NULL),
m_hDcBackground(NULL),
m_hbmpOffscreen(NULL),
m_hbmpBackground(NULL),
m_hwndTooltip(NULL),
m_bShowUpdateRect(false),
m_uTimerID(0x1000),
m_pRoot(NULL),
m_pFocus(NULL),
m_pEventHover(NULL),
m_pLastHover(NULL),
m_pEventClick(NULL),
m_pEventKey(NULL),
m_bFirstLayout(true),
m_bFocusNeeded(false),
m_bUpdateNeeded(false),
m_bMouseTracking(false),
m_bMouseCapture(false),
m_bOffscreenPaint(true),
m_bAlphaBackground(false),
m_pParentResourcePM(NULL)
{
    m_dwDefaultDisabledColor = 0xFFA7A6AA;
    m_dwDefaultFontColor = 0xFF000000;
    m_dwDefaultLinkFontColor = 0xFF0000FF;
    m_dwDefaultLinkHoverFontColor = 0xFFD3215F;
    m_dwDefaultSelectedBkColor = 0xFFBAE4FF;
    LOGFONT lf = { 0 };
    ::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
    lf.lfCharSet = DEFAULT_CHARSET;
    HFONT hDefaultFont = ::CreateFontIndirect(&lf);
    m_DefaultFontInfo.hFont = hDefaultFont;
    m_DefaultFontInfo.sFontName = lf.lfFaceName;
    m_DefaultFontInfo.iSize = -lf.lfHeight;
    m_DefaultFontInfo.bBold = (lf.lfWeight >= FW_BOLD);
    m_DefaultFontInfo.bUnderline = (lf.lfUnderline == TRUE);
    m_DefaultFontInfo.bItalic = (lf.lfItalic == TRUE);
    ::ZeroMemory(&m_DefaultFontInfo.tm, sizeof(m_DefaultFontInfo.tm));

    if( m_hUpdateRectPen == NULL ) {
        m_hUpdateRectPen = ::CreatePen(PS_SOLID, 1, RGB(220, 0, 0));
        // Boot Windows Common Controls (for the ToolTip control)
        ::InitCommonControls();
        ::LoadLibrary(_T("msimg32.dll"));
    }

    m_szMinWindow.cx = 0;
    m_szMinWindow.cy = 0;
    m_szMaxWindow.cx = 0;
    m_szMaxWindow.cy = 0;
    m_szInitWindowSize.cx = 0;
    m_szInitWindowSize.cy = 0;
    m_szRoundCorner.cx = m_szRoundCorner.cy = 0;
    ::ZeroMemory(&m_rcSizeBox, sizeof(m_rcSizeBox));
    ::ZeroMemory(&m_rcCaption, sizeof(m_rcCaption));
    m_ptLastMousePos.x = m_ptLastMousePos.y = -1;

	//add by lighten
	m_pCapturedEvent = NULL;
	m_bUpdateLayered = false;
	m_bTranslate = false;

	//开启gdi+环境
 	GdiplusStartupInput gdiplusStartupInput;
 	GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
 

	// 
//	g_hMainMonitor = NULL;
//  	UpdateLayeredWindow = NULL;
//  	m_hFuncInst = NULL;
}

CPaintManagerUI::~CPaintManagerUI()
{
    // Delete the control-tree structures
    for( int i = 0; i < m_aDelayedCleanup.GetSize(); i++ ) delete static_cast<CControlUI*>(m_aDelayedCleanup[i]);
	m_aDelayedCleanup.Empty();
    for( int i = 0; i < m_aAsyncNotify.GetSize(); i++ ) delete static_cast<TNotifyUI*>(m_aAsyncNotify[i]);
	m_aAsyncNotify.Empty();

    m_mNameHash.Resize(0);
    delete m_pRoot;

    ::DeleteObject(m_DefaultFontInfo.hFont);
    RemoveAllFonts();
    RemoveAllImages();
    RemoveAllDefaultAttributeList();
    RemoveAllOptionGroups();
    RemoveAllTimers();
	
    // Reset other parts...
    if( m_hwndTooltip != NULL ) ::DestroyWindow(m_hwndTooltip);
    if( m_hDcOffscreen != NULL ) ::DeleteDC(m_hDcOffscreen);
    if( m_hDcBackground != NULL ) ::DeleteDC(m_hDcBackground);
    if( m_hbmpOffscreen != NULL ) ::DeleteObject(m_hbmpOffscreen);
    if( m_hbmpBackground != NULL ) ::DeleteObject(m_hbmpBackground);
    if( m_hDcPaint != NULL ) ::ReleaseDC(m_hWndPaint, m_hDcPaint);
    m_aPreMessages.Remove(m_aPreMessages.Find(this));

// 	if (m_hFuncInst != NULL)
// 	{
// 		FreeLibrary(m_hFuncInst);
// 		m_hFuncInst = NULL;
// 	}

 	//关闭gdiplus的环境
 	GdiplusShutdown(m_gdiplusToken);
}

void CPaintManagerUI::Init(HWND hWnd)
{
    ASSERT(::IsWindow(hWnd));
    // Remember the window context we came from
    m_hWndPaint = hWnd;
    m_hDcPaint = ::GetDC(hWnd);
    // We'll want to filter messages globally too
    m_aPreMessages.Add(this);
}

HINSTANCE CPaintManagerUI::GetInstance()
{
    return m_hInstance;
}

CStdString CPaintManagerUI::GetInstancePath()
{
    if( m_hInstance == NULL ) return _T('\0');
    
    TCHAR tszModule[MAX_PATH + 1] = { 0 };
    ::GetModuleFileName(m_hInstance, tszModule, MAX_PATH);
    CStdString sInstancePath = tszModule;
    int pos = sInstancePath.ReverseFind(_T('\\'));
    if( pos >= 0 ) sInstancePath = sInstancePath.Left(pos + 1);
    return sInstancePath;
}

CStdString CPaintManagerUI::GetCurrentPath()
{
    TCHAR tszModule[MAX_PATH + 1] = { 0 };
    ::GetCurrentDirectory(MAX_PATH, tszModule);
    return tszModule;
}

HINSTANCE CPaintManagerUI::GetResourceDll()
{
    if( m_hResourceInstance == NULL ) return m_hInstance;
    return m_hResourceInstance;
}

HMONITOR CPaintManagerUI::GetMainMonitor()
{
	return g_hMainMonitor;
}

const CStdString& CPaintManagerUI::GetResourcePath()
{
    return m_pStrResourcePath;
}

const CStdString& CPaintManagerUI::GetResourceZip()
{
    return m_pStrResourceZip;
}

bool CPaintManagerUI::IsCachedResourceZip()
{
	return m_bCachedResourceZip;
}

HANDLE CPaintManagerUI::GetResourceZipHandle()
{
	return m_hResourceZip;
}

void CPaintManagerUI::SetInstance(HINSTANCE hInst)
{
    m_hInstance = hInst;
}

void CPaintManagerUI::SetCurrentPath(LPCTSTR pStrPath)
{
    ::SetCurrentDirectory(pStrPath);
}

void CPaintManagerUI::SetResourceDll(HINSTANCE hInst)
{
    m_hResourceInstance = hInst;
}

void CPaintManagerUI::SetResourcePath(LPCTSTR pStrPath)
{
    m_pStrResourcePath = pStrPath;
    if( m_pStrResourcePath.IsEmpty() ) return;
    TCHAR cEnd = m_pStrResourcePath.GetAt(m_pStrResourcePath.GetLength() - 1);
    if( cEnd != _T('\\') && cEnd != _T('/') ) m_pStrResourcePath += _T('\\');
}

void CPaintManagerUI::SetMainMonitor(HMONITOR  hMonitor)
{
	g_hMainMonitor = hMonitor;
}

void CPaintManagerUI::SetResourceZip(LPVOID pVoid, unsigned int len)
{
	if( m_pStrResourceZip == _T("membuffer") ) return;
	if( m_bCachedResourceZip && m_hResourceZip != NULL ) {
		CloseZip((HZIP)m_hResourceZip);
		m_hResourceZip = NULL;
	}
	m_pStrResourceZip = _T("membuffer");
	m_bCachedResourceZip = true;
	if( m_bCachedResourceZip ) 
		m_hResourceZip = (HANDLE)OpenZip(pVoid, len, 3);
}

void CPaintManagerUI::SetResourceZip(LPCTSTR pStrPath, bool bCachedResourceZip)
{
	if( m_pStrResourceZip == pStrPath && m_bCachedResourceZip == bCachedResourceZip ) return;
	if( m_bCachedResourceZip && m_hResourceZip != NULL ) {
		CloseZip((HZIP)m_hResourceZip);
		m_hResourceZip = NULL;
	}
	m_pStrResourceZip = pStrPath;
	m_bCachedResourceZip = bCachedResourceZip;
	if( m_bCachedResourceZip ) {
		CStdString sFile = CPaintManagerUI::GetResourcePath();
		sFile += CPaintManagerUI::GetResourceZip();
		m_hResourceZip = (HANDLE)OpenZip((void*)sFile.GetData(), 0, 2);
	}
}
// void CPaintManagerUI::SetResourceZip(LPCTSTR pStrPath)
// {
//     m_pStrResourceZip = pStrPath;
// }

void CPaintManagerUI::GetHSL(short* H, short* S, short* L)
{
    *H = m_H;
    *S = m_S;
    *L = m_L;
}

void CPaintManagerUI::Term()
{
	if( m_bCachedResourceZip && m_hResourceZip != NULL ) {
		CloseZip((HZIP)m_hResourceZip);
		m_hResourceZip = NULL;
	}
}

void CPaintManagerUI::SetHSL(bool bUseHSL, short H, short S, short L)
{
    if( H == m_H && S == m_S && L == m_L ) return;
    m_H = CLAMP(H, 0, 360);
    m_S = CLAMP(S, 0, 200);
    m_L = CLAMP(L, 0, 200);
    for( int i = 0; i < m_aPreMessages.GetSize(); i++ ) {
        CPaintManagerUI* pManager = static_cast<CPaintManagerUI*>(m_aPreMessages[i]);
        if( pManager != NULL && pManager->GetRoot() != NULL )
            pManager->GetRoot()->Invalidate();
    }
}

void CPaintManagerUI::ReloadSkin()
{
    for( int i = 0; i < m_aPreMessages.GetSize(); i++ ) {
        CPaintManagerUI* pManager = static_cast<CPaintManagerUI*>(m_aPreMessages[i]);
        pManager->ReloadAllImages();
    }
}

HWND CPaintManagerUI::GetPaintWindow() const
{
    return m_hWndPaint;
}

HWND CPaintManagerUI::GetTooltipWindow() const
{
    return m_hwndTooltip;
}

HDC CPaintManagerUI::GetPaintDC() const
{
    return m_hDcPaint;
}

POINT CPaintManagerUI::GetMousePos() const
{
    return m_ptLastMousePos;
}

SIZE CPaintManagerUI::GetClientSize() const
{
    RECT rcClient = { 0 };
    ::GetClientRect(m_hWndPaint, &rcClient);
    return CSize(rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);
}

SIZE CPaintManagerUI::GetInitSize()
{
    return m_szInitWindowSize;
}

void CPaintManagerUI::SetInitSize(int cx, int cy)
{
    m_szInitWindowSize.cx = cx;
    m_szInitWindowSize.cy = cy;
    if( m_pRoot == NULL && m_hWndPaint != NULL ) {
        ::SetWindowPos(m_hWndPaint, NULL, 0, 0, cx, cy, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
    }
}

RECT& CPaintManagerUI::GetSizeBox()
{
    return m_rcSizeBox;
}

void CPaintManagerUI::SetSizeBox(RECT& rcSizeBox)
{
    m_rcSizeBox = rcSizeBox;
}

RECT& CPaintManagerUI::GetCaptionRect()
{
    return m_rcCaption;
}

void CPaintManagerUI::SetCaptionRect(RECT& rcCaption)
{
    m_rcCaption = rcCaption;
}

SIZE CPaintManagerUI::GetRoundCorner() const
{
    return m_szRoundCorner;
}

void CPaintManagerUI::SetRoundCorner(int cx, int cy)
{
    m_szRoundCorner.cx = cx;
    m_szRoundCorner.cy = cy;
}

SIZE CPaintManagerUI::GetMinInfo() const
{
	return m_szMinWindow;
}

void CPaintManagerUI::SetMinInfo(int cx, int cy)
{
    ASSERT(cx>=0 && cy>=0);
    m_szMinWindow.cx = cx;
    m_szMinWindow.cy = cy;
}

SIZE CPaintManagerUI::GetMaxInfo() const
{
    return m_szMaxWindow;
}

void CPaintManagerUI::SetMaxInfo(int cx, int cy)
{
    ASSERT(cx>=0 && cy>=0);
    m_szMaxWindow.cx = cx;
    m_szMaxWindow.cy = cy;
}

void CPaintManagerUI::SetTransparent(int nOpacity)
{
    if( m_hWndPaint != NULL ) {
        typedef BOOL (__stdcall *PFUNCSETLAYEREDWINDOWATTR)(HWND, COLORREF, BYTE, DWORD);
        PFUNCSETLAYEREDWINDOWATTR fSetLayeredWindowAttributes;

        HMODULE hUser32 = ::GetModuleHandle(_T("User32.dll"));
        if (hUser32)
        {
            fSetLayeredWindowAttributes = 
                (PFUNCSETLAYEREDWINDOWATTR)::GetProcAddress(hUser32, "SetLayeredWindowAttributes");
            if( fSetLayeredWindowAttributes == NULL ) return;
        }

        DWORD dwStyle = ::GetWindowLong(m_hWndPaint, GWL_EXSTYLE);
        DWORD dwNewStyle = dwStyle;
        if( nOpacity >= 0 && nOpacity < 256 )
			dwNewStyle |= WS_EX_LAYERED;
        else dwNewStyle &= ~WS_EX_LAYERED;
        if(dwStyle != dwNewStyle) ::SetWindowLong(m_hWndPaint, GWL_EXSTYLE, dwNewStyle);
       fSetLayeredWindowAttributes(m_hWndPaint, RGB(0,0,0), nOpacity, LWA_ALPHA);
		m_bTranslate = true;
    }
}

void CPaintManagerUI::SetTransparentAndColor(int nOpacity, DWORD dwColor/* = 0*/)   //add by lighten 2013.03.18
{
	if( m_hWndPaint != NULL ) {
		typedef BOOL (__stdcall *PFUNCSETLAYEREDWINDOWATTR)(HWND, COLORREF, BYTE, DWORD);
		PFUNCSETLAYEREDWINDOWATTR fSetLayeredWindowAttributes;

		HMODULE hUser32 = ::GetModuleHandle(_T("User32.dll"));
		if (hUser32)
		{
			fSetLayeredWindowAttributes = 
				(PFUNCSETLAYEREDWINDOWATTR)::GetProcAddress(hUser32, "SetLayeredWindowAttributes");
			if( fSetLayeredWindowAttributes == NULL ) return;
		}

		DWORD dwStyle = ::GetWindowLong(m_hWndPaint, GWL_EXSTYLE);
		DWORD dwNewStyle = dwStyle;
		if( nOpacity >= 0 && nOpacity < 256 )
			dwNewStyle |= WS_EX_LAYERED;
		else dwNewStyle &= ~WS_EX_LAYERED;
		if(dwStyle != dwNewStyle) ::SetWindowLong(m_hWndPaint, GWL_EXSTYLE, dwNewStyle);
		if (dwColor == 0)
		{
			fSetLayeredWindowAttributes(m_hWndPaint, RGB(0,0,0), nOpacity, LWA_ALPHA);
		}
		else
		{
			fSetLayeredWindowAttributes(m_hWndPaint, RGB(GetBValue(dwColor), GetGValue(dwColor), GetRValue(dwColor)), nOpacity, LWA_ALPHA | LWA_COLORKEY);
		}
		m_bTranslate = true;
	}
}

void CPaintManagerUI::SetUpdateTransparent(bool bUpdateLayered /* = true */, int nAlpha /* = 255 */)
{
	m_bUpdateLayered = bUpdateLayered;
	memset(&m_Blend, 0x00, sizeof(BLENDFUNCTION));
	// Initialize GDI+.
	m_Blend.BlendOp=AC_SRC_OVER; //theonlyBlendOpdefinedinWindows2000
	m_Blend.BlendFlags=0; //nothingelseisspecial...
	m_Blend.AlphaFormat=AC_SRC_ALPHA; //...
	m_Blend.SourceConstantAlpha = nAlpha;//AC_SRC_ALPHA

}

bool CPaintManagerUI::IsSetUpdatelayered() const
{
	return m_bUpdateLayered;
}

// void CPaintManagerUI::ADDToPostPaint(CControlUI *pControl)
// {
// 	if (pControl != NULL)
// 	{
// 		m_aPostPaintControls.Add(pControl);
// 	}
// }
// 
// void CPaintManagerUI::RemoveAtFromPostPaint(CControlUI *pControl)
// {
// 	if (m_aPostPaintControls.GetSize() > 0 && pControl != NULL)
// 	{
// 		for (int i =0; i< m_aPostPaintControls.GetSize(); i++)
// 		{
// 			CControlUI *pTemp = static_cast<CControlUI *>(m_aPostPaintControls.GetAt(i));
// 			if (pTemp == pControl)
// 			{
// 				m_aPostPaintControls.Remove(i);
// 				return;
// 			}
// 		}
// 	}
// }

void CPaintManagerUI::SetBackgroundTransparent(bool bTrans)
{
    m_bAlphaBackground = bTrans;
}

bool CPaintManagerUI::IsShowUpdateRect() const
{
	return m_bShowUpdateRect;
}

void CPaintManagerUI::SetShowUpdateRect(bool show)
{
    m_bShowUpdateRect = show;
}

bool CPaintManagerUI::PreMessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& /*lRes*/)
{
    for( int i = 0; i < m_aPreMessageFilters.GetSize(); i++ ) 
    {
        bool bHandled = false;
        LRESULT lResult = static_cast<IMessageFilterUI*>(m_aPreMessageFilters[i])->MessageHandler(uMsg, wParam, lParam, bHandled);
        if( bHandled ) {
            return true;
        }
    }
    switch( uMsg ) {
    case WM_KEYDOWN:
        {
           // Tabbing between controls
           if( wParam == VK_TAB ) {
               if( m_pFocus && m_pFocus->IsVisible() && m_pFocus->IsEnabled() && _tcsstr(m_pFocus->GetClass(), _T("RichEditUI")) != NULL ) {
                   if( static_cast<CRichEditUI*>(m_pFocus)->IsWantTab() ) return false;
               }
               SetNextTabControl(::GetKeyState(VK_SHIFT) >= 0);
               return true;
           }
        }
        break;
    case WM_SYSCHAR:
        {
           // Handle ALT-shortcut key-combinations
           FINDSHORTCUT fs = { 0 };
           fs.ch = toupper((int)wParam);
           CControlUI* pControl = m_pRoot->FindControl(__FindControlFromShortcut, &fs, UIFIND_ENABLED | UIFIND_ME_FIRST | UIFIND_TOP_FIRST);
           if( pControl != NULL ) {
               pControl->SetFocus();
               pControl->Activate();
               return true;
           }
        }
        break;
    case WM_SYSKEYDOWN:
        {
           if( m_pFocus != NULL ) {
               TEventUI event = { 0 };
               event.Type = UIEVENT_SYSKEY;
               event.chKey = (TCHAR)wParam;
               event.ptMouse = m_ptLastMousePos;
               event.wKeyState = MapKeyState();
               event.dwTimestamp = ::GetTickCount();
               m_pFocus->Event(event);
           }
        }
        break;
    }
    return false;
}

bool CPaintManagerUI::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lRes)
{
//#ifdef _DEBUG
//    switch( uMsg ) {
//    case WM_NCPAINT:
//    case WM_NCHITTEST:
//    case WM_SETCURSOR:
//       break;
//    default:
//       TRACE(_T("MSG: %-20s (%08ld)"), TRACEMSG(uMsg), ::GetTickCount());
//    }
//#endif
    // Not ready yet?
    if( m_hWndPaint == NULL ) return false;
    
    TNotifyUI* pMsg = NULL;
    while( pMsg = static_cast<TNotifyUI*>(m_aAsyncNotify.GetAt(0)) ) {
        m_aAsyncNotify.Remove(0);
        if( pMsg->pSender != NULL ) {
            if( pMsg->pSender->OnNotify ) pMsg->pSender->OnNotify(pMsg);
        }
        for( int j = 0; j < m_aNotifiers.GetSize(); j++ ) {
            static_cast<INotifyUI*>(m_aNotifiers[j])->Notify(*pMsg);
        }
        delete pMsg;
    }
    
    // Cycle through listeners
    for( int i = 0; i < m_aMessageFilters.GetSize(); i++ ) 
    {
        bool bHandled = false;
        LRESULT lResult = static_cast<IMessageFilterUI*>(m_aMessageFilters[i])->MessageHandler(uMsg, wParam, lParam, bHandled);
        if( bHandled ) {
            lRes = lResult;
            return true;
        }
    }
    // Custom handling of events
    switch( uMsg ) {
    case WM_APP + 1:
        {
            for( int i = 0; i < m_aDelayedCleanup.GetSize(); i++ ) 
                delete static_cast<CControlUI*>(m_aDelayedCleanup[i]);
            m_aDelayedCleanup.Empty();
        }
        break;
    case WM_CLOSE:
        {
            // Make sure all matching "closing" events are sent
            TEventUI event = { 0 };
            event.ptMouse = m_ptLastMousePos;
            event.dwTimestamp = ::GetTickCount();
            if( m_pEventHover != NULL ) {
                event.Type = UIEVENT_MOUSELEAVE;
                event.pSender = m_pEventHover;
                m_pEventHover->Event(event);
            }
            if( m_pEventClick != NULL ) {
//                 event.Type = UIEVENT_BUTTONUP;
//                 event.pSender = m_pEventClick;
//                 m_pEventClick->Event(event);
				m_pEventClick = NULL;
            }

            SetFocus(NULL);

            // Hmmph, the usual Windows tricks to avoid
            // focus loss...
            HWND hwndParent = GetWindowOwner(m_hWndPaint);
            if( hwndParent != NULL ) ::SetFocus(hwndParent);
        }
        break;
	case WM_EFFECTS:
		{
			//
			// Render screen
			//
			if( m_anim.IsAnimating() )
			{
				// 3D animation in progress
				//   3D动画  
				m_anim.Render();
				// Do a minimum paint loop  做一个最小的绘制循环
				// Keep the client area invalid so we generate lots of
				// WM_PAINT messages. Cross fingers that Windows doesn't
				// batch these somehow in the future.
				PAINTSTRUCT ps = { 0 };
				::BeginPaint(m_hWndPaint, &ps);
				::EndPaint(m_hWndPaint, &ps);
				::InvalidateRect(m_hWndPaint, NULL, FALSE);
			}
			else if( m_anim.IsJobScheduled() ) {
				// Animation system needs to be initialized
				//	动画系统需要初始化
				m_anim.Init(m_hWndPaint);
				// A 3D animation was scheduled; allow the render engine to
				// capture the window content and repaint some other time
				//翻译(by 金山词霸)一个3d动画被准备;允许渲染引擎捕获窗口内容，并且适时重画

				if( !m_anim.PrepareAnimation(m_hWndPaint)) m_anim.CancelJobs();
				::InvalidateRect(m_hWndPaint, NULL, TRUE);
			}
		}
		break;
    case WM_ERASEBKGND:
        {
            // We'll do the painting here...
            lRes = 1;
        }
        return true;
    case WM_PAINT:
        {
            // Should we paint?
            RECT rcPaint = { 0 };
            if( !::GetUpdateRect(m_hWndPaint, &rcPaint, FALSE) ) return true;
            if( m_pRoot == NULL ) {
                PAINTSTRUCT ps = { 0 };
                ::BeginPaint(m_hWndPaint, &ps);
                ::EndPaint(m_hWndPaint, &ps);
                return true;
            }            
            // Do we need to resize anything?
            // This is the time where we layout the controls on the form.
            // We delay this even from the WM_SIZE messages since resizing can be
            // a very expensize operation.
            if( m_bUpdateNeeded || m_bTranslate) {
                m_bUpdateNeeded = false;
                RECT rcClient = { 0 };
                ::GetClientRect(m_hWndPaint, &rcClient);
                if( !::IsRectEmpty(&rcClient) ) {
                    if( m_pRoot->IsUpdateNeeded() ) {
                        m_pRoot->SetPos(rcClient);
                        if( m_hDcOffscreen != NULL ) ::DeleteDC(m_hDcOffscreen);
                        if( m_hDcBackground != NULL ) ::DeleteDC(m_hDcBackground);
                        if( m_hbmpOffscreen != NULL ) ::DeleteObject(m_hbmpOffscreen);
                        if( m_hbmpBackground != NULL ) ::DeleteObject(m_hbmpBackground);
                        m_hDcOffscreen = NULL;
                        m_hDcBackground = NULL;
                        m_hbmpOffscreen = NULL;
                        m_hbmpBackground = NULL;
                    }
                    else {
                        CControlUI* pControl = NULL;
                        while( pControl = m_pRoot->FindControl(__FindControlFromUpdate, NULL, UIFIND_VISIBLE | UIFIND_ME_FIRST) ) {
                            pControl->SetPos( pControl->GetPos() );
                        }
                    }
                    // We'll want to notify the window when it is first initialized
                    // with the correct layout. The window form would take the time
                    // to submit swipes/animations.
                    if( m_bFirstLayout ) {
                        m_bFirstLayout = false;
                        SendNotify(m_pRoot, _T("windowinit"), DUILIB_WND_INIT, 0, 0, false);
                    }
                }
            }
            // Set focus to first control?
            if( m_bFocusNeeded ) {
                SetNextTabControl();
            }
            //
            // Render screen
            //
			if( m_anim.IsAnimating() || m_anim.IsJobScheduled())
				::PostMessage(m_hWndPaint,WM_EFFECTS,NULL,NULL);
            // Prepare offscreen bitmap?
			//add by lighten
			else
			{
				if ((m_hbmpOffscreen != NULL) && IsSetUpdatelayered())
				{
					::DeleteDC(m_hDcOffscreen);
					m_hDcOffscreen = NULL;

					::DeleteObject(m_hbmpOffscreen);
					m_hbmpOffscreen = NULL;
				}

				if( m_bOffscreenPaint && (m_hbmpOffscreen == NULL ))
				{

					RECT rcClient = { 0 };
					::GetClientRect(m_hWndPaint, &rcClient);
					DWORD dwDCType = ::GetObjectType(m_hDcPaint);
					if (dwDCType != OBJ_DC)   // 保护上下文设备句柄有效性
					{
						::ReleaseDC(m_hWndPaint, m_hDcPaint);
						m_hDcPaint = ::GetDC(m_hWndPaint);
					}
					m_hDcOffscreen = ::CreateCompatibleDC(m_hDcPaint);
					// 				if (m_hDcOffscreen == NULL)
					// 				{
					// 					DWORD dwError = ::GetLastError();
					// 					int i = 0;
					// 				}
					m_hbmpOffscreen = ::CreateCompatibleBitmap(m_hDcPaint, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top); 
					// 				if (m_hbmpOffscreen == NULL)
					// 				{
					// 					DWORD dwError = ::GetLastError();
					// 					int i = 0;
					// 				}

					if ((m_hDcOffscreen == NULL) || (m_hbmpOffscreen == NULL))
					{
						return false;
					}
					ASSERT(m_hDcOffscreen);
					ASSERT(m_hbmpOffscreen);

				}
				// Begin Windows paint
				PAINTSTRUCT ps = { 0 };
				::BeginPaint(m_hWndPaint, &ps);
				if( m_bOffscreenPaint )
				{
					HBITMAP hOldBitmap = (HBITMAP)::SelectObject(m_hDcOffscreen, m_hbmpOffscreen);
					int iSaveDC = ::SaveDC(m_hDcOffscreen);
					if( m_bAlphaBackground ) {
						if( m_hbmpBackground == NULL )
						{
							HWND hDesktopWnd = ::GetNextWindow(m_hWndPaint, GW_HWNDNEXT);/*GetDesktopWindow();*/
							HDC  hDesttopDC;
							if (hDesktopWnd != NULL)
							{
								hDesttopDC = GetDC(hDesktopWnd);
							}

							RECT rcClient = { 0 };
							::GetWindowRect(m_hWndPaint, &rcClient);
							POINT pt = {rcClient.left, rcClient.top};
							ClientToScreen(hDesktopWnd, &pt);
							m_hDcBackground = ::CreateCompatibleDC(/*m_hDcPaint*/hDesttopDC);;
							m_hbmpBackground = ::CreateCompatibleBitmap(/*m_hDcPaint*/hDesttopDC, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top); 
							ASSERT(m_hDcBackground);
							ASSERT(m_hbmpBackground);
							::SelectObject(m_hDcBackground, m_hbmpBackground);
							::BitBlt(m_hDcBackground, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right - ps.rcPaint.left,
								ps.rcPaint.bottom - ps.rcPaint.top, hDesttopDC/*ps.hdc*/, pt.x, pt.y, SRCCOPY);
							// 						::BitBlt(m_hDcBackground, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right - ps.rcPaint.left,
							// 							ps.rcPaint.bottom - ps.rcPaint.top, ps.hdc, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);

							ReleaseDC(hDesktopWnd, hDesttopDC);
						}
						else
							::SelectObject(m_hDcBackground, m_hbmpBackground);
						::BitBlt(m_hDcOffscreen, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right - ps.rcPaint.left,
							ps.rcPaint.bottom - ps.rcPaint.top, m_hDcBackground, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);
					}

					::SendMessage(m_hWndPaint, WM_PAINT_BACKGROUND, (WPARAM)m_hDcOffscreen, 0);

					if (m_bUpdateLayered == false)
					{
						m_pRoot->DoPaint(m_hDcOffscreen, ps.rcPaint);
						for( int i = 0; i < m_aPostPaintControls.GetSize(); i++ ) {
							CControlUI* pPostPaintControl = static_cast<CControlUI*>(m_aPostPaintControls[i]);
							pPostPaintControl->DoPostPaint(m_hDcOffscreen, ps.rcPaint);
						}

						::RestoreDC(m_hDcOffscreen, iSaveDC);


						::BitBlt(ps.hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right - ps.rcPaint.left,
							ps.rcPaint.bottom - ps.rcPaint.top, m_hDcOffscreen, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);
						::SelectObject(m_hDcOffscreen, hOldBitmap);
					}
					else
					{
						RECT rcClient = { 0 };
						::GetClientRect(m_hWndPaint, &rcClient);

						m_pRoot->DoPaint(m_hDcOffscreen, rcClient);
						for( int i = 0; i < m_aPostPaintControls.GetSize(); i++ ) {
							CControlUI* pPostPaintControl = static_cast<CControlUI*>(m_aPostPaintControls[i]);
							pPostPaintControl->DoPostPaint(m_hDcOffscreen, rcClient);
						}

						::RestoreDC(m_hDcOffscreen, iSaveDC);

						RECT rct;
						GetWindowRect(m_hWndPaint, &rct);
						POINT ptWinPos={rct.left, rct.top};

						POINT ptSrc={0, 0};
						SIZE sizeWindow={rct.right - rct.left, rct.bottom - rct.top};

						m_dwExStyle = GetWindowLong(m_hWndPaint, GWL_EXSTYLE);
						if((m_dwExStyle & WS_EX_LAYERED)!=WS_EX_LAYERED)
						{
							SetWindowLong(m_hWndPaint, GWL_EXSTYLE, m_dwExStyle| WS_EX_LAYERED);
						}

						BOOL bRet = ::UpdateLayeredWindow( m_hWndPaint, ps.hdc, &ptWinPos,
							&sizeWindow, m_hDcOffscreen, &ptSrc ,0, &m_Blend, ULW_ALPHA);
						::SelectObject(m_hDcOffscreen, hOldBitmap);
					}

					if( m_bShowUpdateRect ) {
						HPEN hOldPen = (HPEN)::SelectObject(ps.hdc, m_hUpdateRectPen);
						HBRUSH hobj = (HBRUSH)::SelectObject(ps.hdc, ::GetStockObject(HOLLOW_BRUSH));
						::Rectangle(ps.hdc, rcPaint.left, rcPaint.top, rcPaint.right, rcPaint.bottom);
						::SelectObject(ps.hdc, hobj);
						::SelectObject(ps.hdc, hOldPen);
					}
				}
				else
				{
					// A standard paint job
					int iSaveDC = ::SaveDC(ps.hdc);
					m_pRoot->DoPaint(ps.hdc, ps.rcPaint);
					::RestoreDC(ps.hdc, iSaveDC);
				}
				// All Done!
				::EndPaint(m_hWndPaint, &ps);
			}
		}
        // If any of the painting requested a resize again, we'll need
        // to invalidate the entire window once more.
        if( m_bUpdateNeeded ) {
            ::InvalidateRect(m_hWndPaint, NULL, FALSE);
        }
        return true;
    case WM_PRINTCLIENT:
        {
            RECT rcClient;
            ::GetClientRect(m_hWndPaint, &rcClient);
            HDC hDC = (HDC) wParam;
            int save = ::SaveDC(hDC);
            m_pRoot->DoPaint(hDC, rcClient);
            // Check for traversing children. The crux is that WM_PRINT will assume
            // that the DC is positioned at frame coordinates and will paint the child
            // control at the wrong position. We'll simulate the entire thing instead.
            if( (lParam & PRF_CHILDREN) != 0 ) {
                HWND hWndChild = ::GetWindow(m_hWndPaint, GW_CHILD);
                while( hWndChild != NULL ) {
                    RECT rcPos = { 0 };
                    ::GetWindowRect(hWndChild, &rcPos);
                    ::MapWindowPoints(HWND_DESKTOP, m_hWndPaint, reinterpret_cast<LPPOINT>(&rcPos), 2);
                    ::SetWindowOrgEx(hDC, -rcPos.left, -rcPos.top, NULL);
                    // NOTE: We use WM_PRINT here rather than the expected WM_PRINTCLIENT
                    //       since the latter will not print the nonclient correctly for
                    //       EDIT controls.
                    ::SendMessage(hWndChild, WM_PRINT, wParam, lParam | PRF_NONCLIENT);
                    hWndChild = ::GetWindow(hWndChild, GW_HWNDNEXT);
                }
            }
            ::RestoreDC(hDC, save);
        }
        break;
    case WM_GETMINMAXINFO:
        {
            LPMINMAXINFO lpMMI = (LPMINMAXINFO) lParam;
            if( m_szMinWindow.cx > 0 ) lpMMI->ptMinTrackSize.x = m_szMinWindow.cx;
            if( m_szMinWindow.cy > 0 ) lpMMI->ptMinTrackSize.y = m_szMinWindow.cy;
            if( m_szMaxWindow.cx > 0 ) lpMMI->ptMaxTrackSize.x = m_szMaxWindow.cx;
            if( m_szMaxWindow.cy > 0 ) lpMMI->ptMaxTrackSize.y = m_szMaxWindow.cy;
        }
        break;
    case WM_SIZE:
        {
            if( m_pFocus != NULL ) {
                TEventUI event = { 0 };
                event.Type = UIEVENT_WINDOWSIZE;
                event.pSender = m_pFocus;
                event.dwTimestamp = ::GetTickCount();
                m_pFocus->Event(event);
            }
            if( m_pRoot != NULL ) m_pRoot->NeedUpdate();
			//当对话框尺寸变化时 删除动画 job
			if( m_anim.IsAnimating() ) m_anim.CancelJobs();
			m_bUpdateNeeded= true;  //csk_no
			if( m_pRoot != NULL ) m_pRoot->NeedUpdate();
        }
        return true;
    case WM_TIMER:
        {
            for( int i = 0; i < m_aTimers.GetSize(); i++ ) {
                const TIMERINFO* pTimer = static_cast<TIMERINFO*>(m_aTimers[i]);
                if( pTimer->hWnd == m_hWndPaint && pTimer->uWinTimer == LOWORD(wParam) && pTimer->bKilled == false) {
                    TEventUI event = { 0 };
                    event.Type = UIEVENT_TIMER;
                    event.pSender = pTimer->pSender;
                    event.wParam = pTimer->nLocalID;
                    event.dwTimestamp = ::GetTickCount();
                    pTimer->pSender->Event(event);
                    break;
                }
            }
        }
        break;
    case WM_MOUSEHOVER:
        {
            m_bMouseTracking = false;
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            CControlUI* pHover = FindControl(pt);
            if( pHover == NULL ) break;
			if (m_pLastHover == NULL)
			{
				m_pLastHover = pHover;
			}
			else
			{
				if (m_pLastHover == pHover)
				{
					break;
				}
				else
				{
					m_pLastHover = pHover;
				}
			}
            // Generate mouse hover event
            if( m_pEventHover != NULL ) {
                TEventUI event = { 0 };
                event.ptMouse = pt;
                event.Type = UIEVENT_MOUSEHOVER;
                event.pSender = m_pEventHover;
                event.dwTimestamp = ::GetTickCount();
                m_pEventHover->Event(event);
            }
            // Create tooltip information
            CStdString sToolTip = pHover->GetToolTip();
			
			if(pHover->IsEnabled())
			{
				if( sToolTip.IsEmpty() ) return true;
				int nMaxWidth = pHover->GetToolTipMaxWidth();
				::ZeroMemory(&m_ToolTip, sizeof(TOOLINFO));
				m_ToolTip.cbSize = sizeof(TOOLINFO);
				m_ToolTip.uFlags = TTF_IDISHWND;
				m_ToolTip.hwnd = m_hWndPaint;
				m_ToolTip.uId = (UINT_PTR) m_hWndPaint;
				m_ToolTip.hinst = m_hInstance;
				m_ToolTip.lpszText = const_cast<LPTSTR>( (LPCTSTR) sToolTip );
				m_ToolTip.rect = pHover->GetPos();
				if( m_hwndTooltip == NULL ) {
					m_hwndTooltip = ::CreateWindowEx(0, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, m_hWndPaint, NULL, m_hInstance, NULL);
					::SendMessage(m_hwndTooltip, TTM_ADDTOOL, 0, (LPARAM) &m_ToolTip);
				}
				::SendMessage(m_hwndTooltip, TTM_SETTOOLINFO, 0, (LPARAM) &m_ToolTip);
				if (nMaxWidth != 0)
				{
					::SendMessage(m_hwndTooltip, TTM_SETMAXTIPWIDTH, 0, nMaxWidth);
				}
				::SendMessage(m_hwndTooltip, TTM_TRACKACTIVATE, TRUE, (LPARAM) &m_ToolTip);
			}
        }
        return true;
    case WM_MOUSELEAVE:
        {
            if( m_hwndTooltip != NULL ) ::SendMessage(m_hwndTooltip, TTM_TRACKACTIVATE, FALSE, (LPARAM) &m_ToolTip);
            if( m_bMouseTracking ) ::SendMessage(m_hWndPaint, WM_MOUSEMOVE, 0, (LPARAM) -1);
            m_bMouseTracking = false;
			m_pLastHover = NULL;
        }
        break;
    case WM_MOUSEMOVE:
        {
            // Start tracking this entire window again...
            if( !m_bMouseTracking ) {
                TRACKMOUSEEVENT tme = { 0 };
                tme.cbSize = sizeof(TRACKMOUSEEVENT);
                tme.dwFlags = TME_HOVER | TME_LEAVE;
                tme.hwndTrack = m_hWndPaint;
                tme.dwHoverTime = m_hwndTooltip == NULL ? 1000UL : (DWORD) ::SendMessage(m_hwndTooltip, TTM_GETDELAYTIME, TTDT_INITIAL, 0L);
                _TrackMouseEvent(&tme);
                m_bMouseTracking = true;
            }
            // Generate the appropriate mouse messages
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            m_ptLastMousePos = pt;
			CControlUI* pNewHover = FindControl(pt);
			
			//add by lighten
			TEventUI event = { 0 };
			event.ptMouse = pt;
			event.dwTimestamp = ::GetTickCount();

            if( (pNewHover != NULL) && (pNewHover->GetManager() != this))
				break;
			

            if( (pNewHover != m_pEventHover) && (m_pEventHover != NULL) ) {
                event.Type = UIEVENT_MOUSELEAVE;
                event.pSender = m_pEventHover;
                m_pEventHover->Event(event);
                m_pEventHover = NULL;
                if( m_hwndTooltip != NULL ) ::SendMessage(m_hwndTooltip, TTM_TRACKACTIVATE, FALSE, (LPARAM) &m_ToolTip);
				m_pLastHover = NULL;
            }
            if( pNewHover != m_pEventHover && pNewHover != NULL ) {
                event.Type = UIEVENT_MOUSEENTER;
                event.pSender = pNewHover;
                pNewHover->Event(event);
                m_pEventHover = pNewHover;
            }

			//add by lighten
			if (m_pCapturedEvent != NULL)
			{
				event.Type = UIEVENT_MOUSEMOVE;
				event.pSender = m_pCapturedEvent;
				m_pCapturedEvent->Event(event);
				if (pNewHover != NULL && (_tcscmp(pNewHover->GetClass(), m_pCapturedEvent->GetClass()) == 0))   //点击的是capture 的子项目
				{
// 					static int i= 0;
// 					TCHAR wstr[256] = {0};
// 					::wsprintf(wstr, _T("The move index is %d\n"), i);
// 					OutputDebugStr(wstr);
// 					i++;

					event.Type = UIEVENT_MOUSEMOVE;
					event.pSender = pNewHover;
					pNewHover->Event(event);
					break;
				}
				if( m_pEventClick != NULL && (_tcscmp(m_pEventClick->GetClass(), m_pCapturedEvent->GetClass()) != 0)) {
					event.Type = UIEVENT_MOUSEMOVE;
					event.pSender = m_pEventClick;
					m_pEventClick->Event(event);
					break;
				}
			}
			else
			{
				if( m_pEventClick != NULL ) {
					event.Type = UIEVENT_MOUSEMOVE;
					event.pSender = m_pEventClick;
					m_pEventClick->Event(event);
				}
				else if( pNewHover != NULL ) {
					event.Type = UIEVENT_MOUSEMOVE;
					event.pSender = pNewHover;
					pNewHover->Event(event);
				}
			}
		
// 			if ((m_pEventClick != NULL) && (_tcscmp(m_pEventClick->GetClass(), _T("ListUIEx")) != 0 ))
// 			{
// 				event.Type = UIEVENT_MOUSEMOVE;
// 				event.pSender = m_pEventClick;
// 				m_pEventClick->Event(event);
// 			}
// 			else if ((m_pEventClick != NULL) && (_tcscmp(m_pEventClick->GetClass(), _T("ListUIEx")) == 0))
// 			{
// 				if( pNewHover != NULL  && (_tcscmp(pNewHover->GetClass(), _T("ListContainerElementUIEx")) == 0)) 
// 				{
// 					event.Type = UIEVENT_MOUSEMOVE;
// 					event.pSender = pNewHover;
// 					pNewHover->Event(event);
// 				}
// 				else 
// 				{
// 					if (m_pCapturedEvent != NULL)
// 					{
// 						event.Type = UIEVENT_MOUSEMOVE;
// 						event.pSender = m_pCapturedEvent;
// 						m_pCapturedEvent->Event(event);
// 					}
// 				}
// 			}
// 			else
//  			{
// 				if( pNewHover != NULL ) 
// 				{
// 					event.Type = UIEVENT_MOUSEMOVE;
// 					event.pSender = pNewHover;
// 					pNewHover->Event(event);
// 				}
// 			}
        }
        break;
    case WM_LBUTTONDOWN:
        {
            // We alway set focus back to our app (this helps
            // when Win32 child windows are placed on the dialog
            // and we need to remove them on focus change).
            ::SetFocus(m_hWndPaint);
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            m_ptLastMousePos = pt;
            CControlUI* pControl = FindControl(pt);
            if( pControl == NULL ) break;
            if( pControl->GetManager() != this ) break;
            pControl->SetFocus();
			
			//add by lighten 
// 			if (_tcscmp(pControl->GetClass(), _T("ListContainerElementUIEx")) == 0)   //如果是list的子项
// 			{
// 				m_pEventClick = static_cast<CListContainerElementUIEx *>(pControl)->GetOwner();
// 			}
// 			else
			{
				m_pEventClick = pControl;
			}
			SetCapture();

            TEventUI event = { 0 };
            event.Type = UIEVENT_BUTTONDOWN;
            event.pSender = pControl;
            event.wParam = wParam;
            event.lParam = lParam;
            event.ptMouse = pt;
            event.wKeyState = (WORD)wParam;
            event.dwTimestamp = ::GetTickCount();
            pControl->Event(event);
			// No need to burden user with 3D animations
			if( m_anim.IsAnimating() ) m_anim.CancelJobs();
        }
        break;
    case WM_LBUTTONDBLCLK:
        {
            ::SetFocus(m_hWndPaint);
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            m_ptLastMousePos = pt;
            CControlUI* pControl = FindControl(pt);
            if( pControl == NULL ) break;
            if( pControl->GetManager() != this ) break;

//			SetCapture();
			m_pEventClick = pControl;
			TEventUI event = { 0 };
			event.Type = UIEVENT_DBLCLICK;
			event.pSender = pControl;
			event.ptMouse = pt;
			event.wKeyState = (WORD)wParam;
			event.dwTimestamp = ::GetTickCount();
            pControl->Event(event);
        }
        break;
    case WM_LBUTTONUP:
        {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            m_ptLastMousePos = pt;
//             if( m_pEventClick == NULL ) break;
			ReleaseCapture();

			//add by lighten
			TEventUI event = { 0 };
			event.Type = UIEVENT_BUTTONUP;
			event.wParam = wParam;
			event.lParam = lParam;
			event.ptMouse = pt;
			event.wKeyState = (WORD)wParam;
			event.dwTimestamp = ::GetTickCount();

			if (m_pCapturedEvent != NULL)
			{
				event.pSender = m_pCapturedEvent;
				m_pCapturedEvent->Event(event);
				m_pCapturedEvent = NULL;
				break;
			}

			if( m_pEventClick == NULL ) 
				break;

//             ReleaseCapture();
 			if (m_pEventClick != NULL)
 			{
				event.pSender = m_pEventClick;
	            m_pEventClick->Event(event);
			    m_pEventClick = NULL;
 			}
        }
        break;
    case WM_RBUTTONDOWN:
        {
            ::SetFocus(m_hWndPaint);
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            m_ptLastMousePos = pt;
            CControlUI* pControl = FindControl(pt);
            if( pControl == NULL ) break;
            if( pControl->GetManager() != this ) break;
            pControl->SetFocus();
			m_pEventClick = pControl;

			SetCapture();
            TEventUI event = { 0 };
            event.Type = UIEVENT_RBUTTONDOWN;
            event.pSender = pControl;
            event.wParam = wParam;
            event.lParam = lParam;
            event.ptMouse = pt;
            event.wKeyState = (WORD)wParam;
            event.dwTimestamp = ::GetTickCount();
            pControl->Event(event);
        }
        break;
	case WM_RBUTTONUP:
		{
			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			ReleaseCapture();
			//add by lighten
// 			TEventUI event = { 0 };
// 			event.Type = UIEVENT_RBUTTONUP;
// 			event.wParam = wParam;
// 			event.lParam = lParam;
// 			event.ptMouse = pt;
// 			event.wKeyState = (WORD)wParam;
// 			event.dwTimestamp = ::GetTickCount();
// 			if (m_pEventClick != NULL)
// 			{
// 				event.pSender = m_pEventClick;
// 				m_pEventClick->Event(event);
// 				m_pEventClick = NULL;
// 			}

		}
		break;
    case WM_CONTEXTMENU:
        {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            ::ScreenToClient(m_hWndPaint, &pt);
            m_ptLastMousePos = pt;
			ReleaseCapture();
            if( m_pEventClick == NULL ) break;
            TEventUI event = { 0 };
            event.Type = UIEVENT_CONTEXTMENU;
            event.pSender = m_pEventClick;
            event.ptMouse = pt;
            event.wKeyState = (WORD)wParam;
            event.lParam = (LPARAM)m_pEventClick;
            event.dwTimestamp = ::GetTickCount();
            m_pEventClick->Event(event);
            m_pEventClick = NULL;
        }
        break;
    case WM_MOUSEWHEEL:
        {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            ::ScreenToClient(m_hWndPaint, &pt);
            m_ptLastMousePos = pt;
            CControlUI* pControl = FindControl(pt);
            if( pControl == NULL ) break;
            if( pControl->GetManager() != this ) break;
            int zDelta = (int) (short) HIWORD(wParam);
            TEventUI event = { 0 };
            event.Type = UIEVENT_SCROLLWHEEL;
            event.pSender = pControl;
            event.wParam = MAKELPARAM(zDelta < 0 ? SB_LINEDOWN : SB_LINEUP, 0);
            event.lParam = lParam;
            event.wKeyState = MapKeyState();
            event.dwTimestamp = ::GetTickCount();
            pControl->Event(event);

            // Let's make sure that the scroll item below the cursor is the same as before...
            ::SendMessage(m_hWndPaint, WM_MOUSEMOVE, 0, (LPARAM) MAKELPARAM(m_ptLastMousePos.x, m_ptLastMousePos.y));
        }
        break;
    case WM_CHAR:
        {
            if( m_pFocus == NULL ) break;
            TEventUI event = { 0 };
            event.Type = UIEVENT_CHAR;
            event.chKey = (TCHAR)wParam;
            event.ptMouse = m_ptLastMousePos;
            event.wKeyState = MapKeyState();
            event.dwTimestamp = ::GetTickCount();
            m_pFocus->Event(event);
        }
        break;
    case WM_KEYDOWN:
        {
            if( m_pFocus == NULL ) break;
            TEventUI event = { 0 };
            event.Type = UIEVENT_KEYDOWN;
            event.chKey = (TCHAR)wParam;
            event.ptMouse = m_ptLastMousePos;
            event.wKeyState = MapKeyState();
            event.dwTimestamp = ::GetTickCount();
            m_pFocus->Event(event);
            m_pEventKey = m_pFocus;
        }
        break;
    case WM_KEYUP:
        {
            if( m_pEventKey == NULL ) break;
            TEventUI event = { 0 };
            event.Type = UIEVENT_KEYUP;
            event.chKey = (TCHAR)wParam;
            event.ptMouse = m_ptLastMousePos;
            event.wKeyState = MapKeyState();
            event.dwTimestamp = ::GetTickCount();
            m_pEventKey->Event(event);
            m_pEventKey = NULL;
        }
        break;
    case WM_SETCURSOR:
        {
            if( LOWORD(lParam) != HTCLIENT ) break;
            if( m_bMouseCapture ) return true;

            POINT pt = { 0 };
            ::GetCursorPos(&pt);
            ::ScreenToClient(m_hWndPaint, &pt);
            CControlUI* pControl = FindControl(pt);
            if( pControl == NULL ) break;
            if( (pControl->GetControlFlags() & UIFLAG_SETCURSOR) == 0 ) break;
            TEventUI event = { 0 };
            event.Type = UIEVENT_SETCURSOR;
            event.wParam = wParam;
            event.lParam = lParam;
            event.ptMouse = pt;
            event.wKeyState = MapKeyState();
            event.dwTimestamp = ::GetTickCount();
            pControl->Event(event);
        }
        return true;
    case WM_NOTIFY:
        {
            LPNMHDR lpNMHDR = (LPNMHDR) lParam;
            if( lpNMHDR != NULL ) lRes = ::SendMessage(lpNMHDR->hwndFrom, OCM__BASE + uMsg, wParam, lParam);
            return true;
        }
        break;
    case WM_COMMAND:
        {
            if( lParam == 0 ) break;
            HWND hWndChild = (HWND) lParam;
            lRes = ::SendMessage(hWndChild, OCM__BASE + uMsg, wParam, lParam);
            return true;
        }
        break;
    case WM_CTLCOLOREDIT:
	case WM_CTLCOLORSTATIC:
        {
			// Refer To: http://msdn.microsoft.com/en-us/library/bb761691(v=vs.85).aspx
			// Read-only or disabled edit controls do not send the WM_CTLCOLOREDIT message; instead, they send the WM_CTLCOLORSTATIC message.
            if( lParam == 0 ) break;
            HWND hWndChild = (HWND) lParam;
            lRes = ::SendMessage(hWndChild, OCM__BASE + uMsg, wParam, lParam);
            return true;
        }
        break;
    default:
        break;
    }

    pMsg = NULL;
    while( pMsg = static_cast<TNotifyUI*>(m_aAsyncNotify.GetAt(0)) ) {
        m_aAsyncNotify.Remove(0);
        if( pMsg->pSender != NULL ) {
            if( pMsg->pSender->OnNotify ) pMsg->pSender->OnNotify(pMsg);
        }
        for( int j = 0; j < m_aNotifiers.GetSize(); j++ ) {
            static_cast<INotifyUI*>(m_aNotifiers[j])->Notify(*pMsg);
        }
        delete pMsg;
    }
    return false;
}

void CPaintManagerUI::NeedUpdate()
{
    m_bUpdateNeeded = true;
}

void CPaintManagerUI::Invalidate(RECT& rcItem)
{
    ::InvalidateRect(m_hWndPaint, &rcItem, FALSE);
}

bool CPaintManagerUI::AttachDialog(CControlUI* pControl)
{
    ASSERT(::IsWindow(m_hWndPaint));
    // Reset any previous attachment
    SetFocus(NULL);
    m_pEventKey = NULL;
    m_pEventHover = NULL;
    m_pEventClick = NULL;
    // Remove the existing control-tree. We might have gotten inside this function as
    // a result of an event fired or similar, so we cannot just delete the objects and
    // pull the internal memory of the calling code. We'll delay the cleanup.
    if( m_pRoot != NULL ) {
        m_aPostPaintControls.Empty();
        AddDelayedCleanup(m_pRoot);
    }
    // Set the dialog root element
    m_pRoot = pControl;
    // Go ahead...
    m_bUpdateNeeded = true;
    m_bFirstLayout = true;
    m_bFocusNeeded = true;
    // Initiate all control
    return InitControls(pControl);
}

bool CPaintManagerUI::InitControls(CControlUI* pControl, CControlUI* pParent /*= NULL*/)
{
    ASSERT(pControl);
    if( pControl == NULL ) return false;
    pControl->SetManager(this, pParent != NULL ? pParent : pControl->GetParent(), true);
    pControl->FindControl(__FindControlFromNameHash, this, UIFIND_ALL);
    return true;
}

void CPaintManagerUI::ReapObjects(CControlUI* pControl)
{
    if( pControl == m_pEventKey ) m_pEventKey = NULL;
    if( pControl == m_pEventHover ) m_pEventHover = NULL;
    if( pControl == m_pEventClick ) m_pEventClick = NULL;
    if( pControl == m_pFocus ) m_pFocus = NULL;
    KillTimer(pControl);
    const CStdString& sName = pControl->GetName();
    if( !sName.IsEmpty() ) {
        if( pControl == FindControl(sName) ) m_mNameHash.Remove(sName);
    }
    for( int i = 0; i < m_aAsyncNotify.GetSize(); i++ ) {
        TNotifyUI* pMsg = static_cast<TNotifyUI*>(m_aAsyncNotify[i]);
        if( pMsg->pSender == pControl ) pMsg->pSender = NULL;
    }    
}

bool CPaintManagerUI::AddOptionGroup(LPCTSTR pStrGroupName, CControlUI* pControl)
{
    LPVOID lp = m_mOptionGroup.Find(pStrGroupName);
    if( lp ) {
        CStdPtrArray* aOptionGroup = static_cast<CStdPtrArray*>(lp);
        for( int i = 0; i < aOptionGroup->GetSize(); i++ ) {
            if( static_cast<CControlUI*>(aOptionGroup->GetAt(i)) == pControl ) {
                return false;
            }
        }
        aOptionGroup->Add(pControl);
    }
    else {
        CStdPtrArray* aOptionGroup = new CStdPtrArray(6);
        aOptionGroup->Add(pControl);
        m_mOptionGroup.Insert(pStrGroupName, aOptionGroup);
    }
    return true;
}

CStdPtrArray* CPaintManagerUI::GetOptionGroup(LPCTSTR pStrGroupName)
{
    LPVOID lp = m_mOptionGroup.Find(pStrGroupName);
    if( lp ) return static_cast<CStdPtrArray*>(lp);
    return NULL;
}

void CPaintManagerUI::RemoveOptionGroup(LPCTSTR pStrGroupName, CControlUI* pControl)
{
	LPVOID lp = m_mOptionGroup.Find(pStrGroupName);
	if( lp ) {
		CStdPtrArray* aOptionGroup = static_cast<CStdPtrArray*>(lp);
		if( aOptionGroup == NULL ) return;
		for( int i = 0; i < aOptionGroup->GetSize(); i++ ) {
			if( static_cast<CControlUI*>(aOptionGroup->GetAt(i)) == pControl ) {
				aOptionGroup->Remove(i);
				break;
			}
		}
		if( aOptionGroup->IsEmpty() ) {
			delete aOptionGroup;
			m_mOptionGroup.Remove(pStrGroupName);
		}
	}
}

void CPaintManagerUI::RemoveAllOptionGroups()
{
	CStdPtrArray* aOptionGroup;
	for( int i = 0; i< m_mOptionGroup.GetSize(); i++ ) {
		if(LPCTSTR key = m_mOptionGroup.GetAt(i)) {
			aOptionGroup = static_cast<CStdPtrArray*>(m_mOptionGroup.Find(key));
			delete aOptionGroup;
		}
	}
}

//************************************
	// Method:    AddEffectsStyle
	// FullName:  CPaintManagerUI::AddEffectsStyle
	// Access:    public 
	// Returns:   bool
	// Qualifier:
	// Parameter: LPCTSTR pStrStyleName
	// Parameter: LPCTSTR pStrStyleValue
	// Note:	  
	//************************************
	bool CPaintManagerUI::AddEffectsStyle( LPCTSTR pStrStyleName,LPCTSTR pStrStyleValue )
	{
		try
		{
			CStdString* pEffectsStyleValue = new CStdString(pStrStyleValue);

			if(pEffectsStyleValue != NULL)
			{
				if( m_mEffectsStyle.Find(pStrStyleName) == NULL )
					m_mEffectsStyle.Set(pStrStyleName,pEffectsStyleValue);
				else
					delete pEffectsStyleValue;
				pEffectsStyleValue = NULL;
				return true;
			}
			return false;
		}
		catch (...)
		{
			throw "CPaintManagerUI::AddEffectsStyle";
		}
	}

	//************************************
	// Method:    GetEffectsStyle
	// FullName:  CPaintManagerUI::GetEffectsStyle
	// Access:    public 
	// Returns:   UiLib::CDuiString
	// Qualifier:
	// Parameter: LPCTSTR pStrStyleName
	// Note:	  
	//************************************
	CStdString CPaintManagerUI::GetEffectsStyle( LPCTSTR pStrStyleName )
	{
		try
		{
			CStdString* pEffectsStyleValue = static_cast<CStdString*>(m_mEffectsStyle.Find(pStrStyleName));
			if( !pEffectsStyleValue && m_pParentResourcePM ) return m_pParentResourcePM->GetEffectsStyle(pStrStyleName);

			if( pEffectsStyleValue ) return pEffectsStyleValue->GetData();
			else return CStdString(_T(""));
		}
		catch (...)
		{
			throw "CPaintManagerUI::GetEffectsStyle";
		}
	}

	//************************************
	// Method:    GetEffectsStyles
	// FullName:  CPaintManagerUI::GetEffectsStyles
	// Access:    public 
	// Returns:   CStdStringPtrMap&
	// Qualifier: const
	// Note:	  
	//************************************
	const CStdStringPtrMap& CPaintManagerUI::GetEffectsStyles() const
	{
		try
		{
			return m_mEffectsStyle;
		}
		catch (...)
		{
			throw "CPaintManagerUI::GetEffectsStyles";
		}
	}

	//************************************
	// Method:    RemoveEffectStyle
	// FullName:  CPaintManagerUI::RemoveEffectStyle
	// Access:    public 
	// Returns:   bool
	// Qualifier:
	// Parameter: LPCTSTR pStrStyleName
	// Note:	  
	//************************************
	bool CPaintManagerUI::RemoveEffectStyle( LPCTSTR pStrStyleName )
	{
		try
		{
			CStdString* pEffectsStyleValue = static_cast<CStdString*>(m_mEffectsStyle.Find(pStrStyleName));
			if( !pEffectsStyleValue ) return false;

			delete pEffectsStyleValue;
			return m_mEffectsStyle.Remove(pStrStyleName);
		}
		catch (...)
		{
			throw "CPaintManagerUI::RemoveEffectStyle";
		}
	}

	//************************************
	// Method:    RemoveAllEffectStyle
	// FullName:  CPaintManagerUI::RemoveAllEffectStyle
	// Access:    public 
	// Returns:   void
	// Qualifier:
	// Note:	  
	//************************************
	void CPaintManagerUI::RemoveAllEffectStyle()
	{
		try
		{
			CStdString* pEffectsStyleValue;
			for( int i = 0; i< m_mEffectsStyle.GetSize(); i++ ) {
				if(LPCTSTR key = m_mEffectsStyle.GetAt(i)) {
					pEffectsStyleValue = static_cast<CStdString*>(m_mEffectsStyle.Find(key));
					if(pEffectsStyleValue)
						delete pEffectsStyleValue;
					pEffectsStyleValue = NULL;
				}
			}
			m_mEffectsStyle.RemoveAll();
		}
		catch (...)
		{
			throw "CPaintManagerUI::RemoveAllEffectStyle";
		}
	}

void CPaintManagerUI::MessageLoop()
{
    MSG msg = { 0 };
    while( ::GetMessage(&msg, NULL, 0, 0) ) {
        if( !CPaintManagerUI::TranslateMessage(&msg) ) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
    }
}

void CPaintManagerUI::DlgMessageLoop(volatile BOOL & bEndLoop)
{
	MSG msg = { 0 };
	while( ::GetMessage(&msg, NULL, 0, 0) ) {
		if( !CPaintManagerUI::TranslateMessage(&msg) ) {
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}

		if (bEndLoop == false)
		{
			break;
		}
	}

}


bool CPaintManagerUI::TranslateAccelerator(LPMSG pMsg)
{
	for (int i = 0; i < m_aTranslateAccelerator.GetSize(); i++)
	{
		LRESULT lResult = static_cast<ITranslateAccelerator *>(m_aTranslateAccelerator[i])->TranslateAccelerator(pMsg);
		if( lResult == S_OK ) return true;
	}
	return false;
}

bool CPaintManagerUI::TranslateMessage(const LPMSG pMsg)
{
	// Pretranslate Message takes care of system-wide messages, such as
	// tabbing and shortcut key-combos. We'll look for all messages for
	// each window and any child control attached.
	//     HWND hwndParent = ::GetParent(pMsg->hwnd);
	//     UINT uStyle = GetWindowStyle(pMsg->hwnd);
	//     LRESULT lRes = 0;
	//     for( int i = 0; i < m_aPreMessages.GetSize(); i++ ) {
	//         CPaintManagerUI* pT = static_cast<CPaintManagerUI*>(m_aPreMessages[i]);
	//         if( pMsg->hwnd == pT->GetPaintWindow() || (hwndParent == pT->GetPaintWindow() && ((uStyle & WS_CHILD) != 0)) )
	//         {
	//             if( pT->PreMessageHandler(pMsg->message, pMsg->wParam, pMsg->lParam, lRes) ) return true;
	//         }
	//     }
	//     return false;
	UINT uStyle = GetWindowStyle(pMsg->hwnd);
	UINT uChildRes = uStyle & WS_CHILD;	
	LRESULT lRes = 0;
	if (uChildRes != 0)
	{
		HWND hWndParent = ::GetParent(pMsg->hwnd);

		for( int i = 0; i < m_aPreMessages.GetSize(); i++ ) 
		{
			CPaintManagerUI* pT = static_cast<CPaintManagerUI*>(m_aPreMessages[i]);        
			HWND hTempParent = hWndParent;
			while(hTempParent)
			{
				if(pMsg->hwnd == pT->GetPaintWindow() || hTempParent == pT->GetPaintWindow())
				{
					if (pT->TranslateAccelerator(pMsg))
						return true;

					if( pT->PreMessageHandler(pMsg->message, pMsg->wParam, pMsg->lParam, lRes) ) 
						return true;

					return false;
				}
				hTempParent = GetParent(hTempParent);
			}
		}
	}
	else
	{
		for( int i = 0; i < m_aPreMessages.GetSize(); i++ ) 
		{
			CPaintManagerUI* pT = static_cast<CPaintManagerUI*>(m_aPreMessages[i]);
			if(pMsg->hwnd == pT->GetPaintWindow())
			{
				if (pT->TranslateAccelerator(pMsg))
					return true;

				if( pT->PreMessageHandler(pMsg->message, pMsg->wParam, pMsg->lParam, lRes) ) 
					return true;

				return false;
			}
		}
	}
	return false;
}

bool CPaintManagerUI::AddTranslateAccelerator(ITranslateAccelerator *pTranslateAccelerator)
{
	ASSERT(m_aTranslateAccelerator.Find(pTranslateAccelerator) < 0);
	return m_aTranslateAccelerator.Add(pTranslateAccelerator);
}

bool CPaintManagerUI::RemoveTranslateAccelerator(ITranslateAccelerator *pTranslateAccelerator)
{
	for (int i = 0; i < m_aTranslateAccelerator.GetSize(); i++)
	{
		if (static_cast<ITranslateAccelerator *>(m_aTranslateAccelerator[i]) == pTranslateAccelerator)
		{
			return m_aTranslateAccelerator.Remove(i);
		}
	}
	return false;
}

CControlUI* CPaintManagerUI::GetFocus() const
{
    return m_pFocus;
}

void CPaintManagerUI::SetFocus(CControlUI* pControl)
{
    // Paint manager window has focus?
    HWND hFocusWnd = ::GetFocus();
    if( hFocusWnd != m_hWndPaint && pControl != m_pFocus ) ::SetFocus(m_hWndPaint);
    // Already has focus?
    if( pControl == m_pFocus ) return;
    // Remove focus from old control
    if( m_pFocus != NULL ) 
    {
        TEventUI event = { 0 };
        event.Type = UIEVENT_KILLFOCUS;
        event.pSender = pControl;
        event.dwTimestamp = ::GetTickCount();
        m_pFocus->Event(event);
        SendNotify(m_pFocus, _T("killfocus"), DUILIB_WND_KILLFOCUS);
        m_pFocus = NULL;
    }
    if( pControl == NULL ) return;
    // Set focus to new control
    if( pControl != NULL 
        && pControl->GetManager() == this 
        && pControl->IsVisible() 
        && pControl->IsEnabled() ) 
    {
        m_pFocus = pControl;
        TEventUI event = { 0 };
        event.Type = UIEVENT_SETFOCUS;
        event.pSender = pControl;
        event.dwTimestamp = ::GetTickCount();
        m_pFocus->Event(event);
        SendNotify(m_pFocus, _T("setfocus"), DUILIB_WND_SETFOCUS);
    }
}

void CPaintManagerUI::SetFocusNeeded(CControlUI* pControl)
{
	// 如果设置为空，则不需要强制刷新
	if (pControl == NULL)
	{
		m_bFocusNeeded = false;
		return;
	}
	::SetFocus(m_hWndPaint);
    if( m_pFocus != NULL ) {
        TEventUI event = { 0 };
        event.Type = UIEVENT_KILLFOCUS;
        event.pSender = pControl;
        event.dwTimestamp = ::GetTickCount();
        m_pFocus->Event(event);
        SendNotify(m_pFocus, _T("killfocus"), DUILIB_WND_KILLFOCUS);
        m_pFocus = NULL;
    }
    FINDTABINFO info = { 0 };
    info.pFocus = pControl;
    info.bForward = false;
    m_pFocus = m_pRoot->FindControl(__FindControlFromTab, &info, UIFIND_VISIBLE | UIFIND_ENABLED | UIFIND_ME_FIRST);
    m_bFocusNeeded = true;
    if( m_pRoot != NULL ) m_pRoot->NeedUpdate();
}

bool CPaintManagerUI::SetTimer(CControlUI* pControl, UINT nTimerID, UINT uElapse)
{
    ASSERT(pControl!=NULL);
    ASSERT(uElapse>0);
    for( int i = 0; i< m_aTimers.GetSize(); i++ ) {
        TIMERINFO* pTimer = static_cast<TIMERINFO*>(m_aTimers[i]);
        if( pTimer->pSender == pControl
            && pTimer->hWnd == m_hWndPaint
            && pTimer->nLocalID == nTimerID )
        {
            if( pTimer->bKilled == true ) {
                if( ::SetTimer(m_hWndPaint, pTimer->uWinTimer, uElapse, NULL) ) {
                    pTimer->bKilled = false;
                    return true;
                }

                return false;
            }

            return false;
        }
    }

    m_uTimerID = (++m_uTimerID) % 0xFF;
    if( !::SetTimer(m_hWndPaint, m_uTimerID, uElapse, NULL) ) 
	{
		DWORD dwError = ::GetLastError();
		return false;
	}
    TIMERINFO* pTimer = new TIMERINFO;
    if( pTimer == NULL ) return FALSE;
    pTimer->hWnd = m_hWndPaint;
    pTimer->pSender = pControl;
    pTimer->nLocalID = nTimerID;
    pTimer->uWinTimer = m_uTimerID;
    pTimer->bKilled = false;
    return m_aTimers.Add(pTimer);
}

bool CPaintManagerUI::KillTimer(CControlUI* pControl, UINT nTimerID)
{
    ASSERT(pControl!=NULL);
    for( int i = 0; i< m_aTimers.GetSize(); i++ ) {
        TIMERINFO* pTimer = static_cast<TIMERINFO*>(m_aTimers[i]);
        if( pTimer->pSender == pControl
            && pTimer->hWnd == m_hWndPaint
            && pTimer->nLocalID == nTimerID )
        {
            if( pTimer->bKilled == false ) {
                if( ::IsWindow(m_hWndPaint) ) ::KillTimer(pTimer->hWnd, pTimer->uWinTimer);
                pTimer->bKilled = true;
                return true;
            }
        }
    }
    return false;
}

void CPaintManagerUI::KillTimer(CControlUI* pControl)
{
    ASSERT(pControl!=NULL);
    int count = m_aTimers.GetSize();
    for( int i = 0, j = 0; i < count; i++ ) {
        TIMERINFO* pTimer = static_cast<TIMERINFO*>(m_aTimers[i - j]);
        if( pTimer->pSender == pControl && pTimer->hWnd == m_hWndPaint ) {
            if( pTimer->bKilled == false ) 
				::KillTimer(pTimer->hWnd, pTimer->uWinTimer);
			delete pTimer;
            m_aTimers.Remove(i);
            j++;
        }
    }
}

void CPaintManagerUI::RemoveAllTimers()
{
    for( int i = 0; i < m_aTimers.GetSize(); i++ ) {
        TIMERINFO* pTimer = static_cast<TIMERINFO*>(m_aTimers[i]);
        if( pTimer->hWnd == m_hWndPaint )
        {
            if( pTimer->bKilled == false ) {
                if( ::IsWindow(m_hWndPaint) ) ::KillTimer(m_hWndPaint, pTimer->uWinTimer);
            }
            delete pTimer;
        }
    }

    m_aTimers.Empty();
}

void CPaintManagerUI::SetCapture()
{
    ::SetCapture(m_hWndPaint);
    m_bMouseCapture = true;
}

void CPaintManagerUI::ReleaseCapture()
{
    ::ReleaseCapture();
    m_bMouseCapture = false;
}

bool CPaintManagerUI::IsCaptured()
{
    return m_bMouseCapture;
}

//add by ligthen
void CPaintManagerUI::SetMCaptured(CControlUI *pControl)
{
	if (pControl != NULL)
	{
		m_pCapturedEvent = pControl;
	}
}

CControlUI *CPaintManagerUI::GetMCapture() const
{
	return m_pCapturedEvent;
}

void CPaintManagerUI::ReleaseMCapture()
{
	m_pCapturedEvent = NULL;
}

bool CPaintManagerUI::SetNextTabControl(bool bForward)
{
    // If we're in the process of restructuring the layout we can delay the
    // focus calulation until the next repaint.
    if( m_bUpdateNeeded && bForward ) {
        m_bFocusNeeded = true;
        ::InvalidateRect(m_hWndPaint, NULL, FALSE);
        return true;
    }
    // Find next/previous tabbable control
    FINDTABINFO info1 = { 0 };
    info1.pFocus = m_pFocus;
    info1.bForward = bForward;
    CControlUI* pControl = m_pRoot->FindControl(__FindControlFromTab, &info1, UIFIND_VISIBLE | UIFIND_ENABLED | UIFIND_ME_FIRST);
    if( pControl == NULL ) {  
        if( bForward ) {
            // Wrap around
            FINDTABINFO info2 = { 0 };
            info2.pFocus = bForward ? NULL : info1.pLast;
            info2.bForward = bForward;
            pControl = m_pRoot->FindControl(__FindControlFromTab, &info2, UIFIND_VISIBLE | UIFIND_ENABLED | UIFIND_ME_FIRST);
        }
        else {
            pControl = info1.pLast;
        }
    }
    if( pControl != NULL ) SetFocus(pControl);
    m_bFocusNeeded = false;
    return true;
}

bool CPaintManagerUI::AddNotifier(INotifyUI* pNotifier)
{
    ASSERT(m_aNotifiers.Find(pNotifier)<0);
    return m_aNotifiers.Add(pNotifier);
}

bool CPaintManagerUI::RemoveNotifier(INotifyUI* pNotifier)
{
    for( int i = 0; i < m_aNotifiers.GetSize(); i++ ) {
        if( static_cast<INotifyUI*>(m_aNotifiers[i]) == pNotifier ) {
            return m_aNotifiers.Remove(i);
        }
    }
    return false;
}

bool CPaintManagerUI::AddPreMessageFilter(IMessageFilterUI* pFilter)
{
    ASSERT(m_aPreMessageFilters.Find(pFilter)<0);
    return m_aPreMessageFilters.Add(pFilter);
}

bool CPaintManagerUI::RemovePreMessageFilter(IMessageFilterUI* pFilter)
{
    for( int i = 0; i < m_aPreMessageFilters.GetSize(); i++ ) {
        if( static_cast<IMessageFilterUI*>(m_aPreMessageFilters[i]) == pFilter ) {
            return m_aPreMessageFilters.Remove(i);
        }
    }
    return false;
}

bool CPaintManagerUI::AddMessageFilter(IMessageFilterUI* pFilter)
{
    ASSERT(m_aMessageFilters.Find(pFilter)<0);
    return m_aMessageFilters.Add(pFilter);
}

bool CPaintManagerUI::RemoveMessageFilter(IMessageFilterUI* pFilter)
{
    for( int i = 0; i < m_aMessageFilters.GetSize(); i++ ) {
        if( static_cast<IMessageFilterUI*>(m_aMessageFilters[i]) == pFilter ) {
            return m_aMessageFilters.Remove(i);
        }
    }
    return false;
}

bool CPaintManagerUI::AddAnimJob(const CAnimJobUI& job)
{
	CAnimJobUI* pJob = new CAnimJobUI(job);
	if( pJob == NULL ) return false;
	::InvalidateRect(m_hWndPaint, NULL, FALSE);
	return m_anim.AddJob(pJob);
}

int CPaintManagerUI::GetPostPaintCount() const
{
    return m_aPostPaintControls.GetSize();
}

bool CPaintManagerUI::AddPostPaint(CControlUI* pControl)
{
//    ASSERT(m_aPostPaintControls.Find(pControl) < 0);
	if (m_aPostPaintControls.Find(pControl) >= 0)
	{
		return true;
	}
    return m_aPostPaintControls.Add(pControl);
}

bool CPaintManagerUI::RemovePostPaint(CControlUI* pControl)
{
    for( int i = 0; i < m_aPostPaintControls.GetSize(); i++ ) {
        if( static_cast<CControlUI*>(m_aPostPaintControls[i]) == pControl ) {
            return m_aPostPaintControls.Remove(i);
        }
    }
    return false;
}

bool CPaintManagerUI::SetPostPaintIndex(CControlUI* pControl, int iIndex)
{
    RemovePostPaint(pControl);
    return m_aPostPaintControls.InsertAt(iIndex, pControl);
}

void CPaintManagerUI::AddDelayedCleanup(CControlUI* pControl)
{
    pControl->SetManager(this, NULL, false);
    m_aDelayedCleanup.Add(pControl);
    ::PostMessage(m_hWndPaint, WM_APP + 1, 0, 0L);
}

void CPaintManagerUI::SendNotify(CControlUI* pControl, LPCTSTR pstrMessage, DWORD dwMessageID /*= 0*/, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/, bool bAsync /*= false*/)
{
    TNotifyUI Msg;
    Msg.pSender = pControl;
    Msg.sType = pstrMessage;
	Msg.dwMessageID = dwMessageID;
    Msg.wParam = wParam;
    Msg.lParam = lParam;
	
	TCHAR strClassName[256] = {0};
	GetClassName(m_hWndPaint, strClassName, 256);
	
	pBtnFunc func = NULL;
	if (pControl != NULL)
	{
		func = CNotifyMapManager::FindFunc(dwMessageID, pControl->GetName(), strClassName);   //在map中查找是否存在关联的处理函数
	}
	if ((func != NULL) && (bAsync == FALSE))   //没有
	{
		func(Msg);
	}
	else
	{
		SendNotify(Msg, bAsync);
	}
}

void CPaintManagerUI::SendNotify(TNotifyUI& Msg, bool bAsync /*= false*/)
{
    Msg.ptMouse = m_ptLastMousePos;
    Msg.dwTimestamp = ::GetTickCount();
    if( !bAsync ) {
        // Send to all listeners
        if( Msg.pSender != NULL ) {
            if( Msg.pSender->OnNotify ) Msg.pSender->OnNotify(&Msg);
        }
        for( int i = 0; i < m_aNotifiers.GetSize(); i++ ) {
            static_cast<INotifyUI*>(m_aNotifiers[i])->Notify(Msg);
        }
    }
    else {
        TNotifyUI *pMsg = new TNotifyUI;
        pMsg->pSender = Msg.pSender;
        pMsg->sType = Msg.sType;
		pMsg->dwMessageID = Msg.dwMessageID;
        pMsg->wParam = Msg.wParam;
        pMsg->lParam = Msg.lParam;
        pMsg->ptMouse = Msg.ptMouse;
        pMsg->dwTimestamp = Msg.dwTimestamp;
        m_aAsyncNotify.Add(pMsg);
    }
}

bool CPaintManagerUI::UseParentResource(CPaintManagerUI* pm)
{
    if( pm == NULL ) {
        m_pParentResourcePM = NULL;
        return true;
    }
    if( pm == this ) return false;

    CPaintManagerUI* pParentPM = pm->GetParentResource();
    while( pParentPM ) {
        if( pParentPM == this ) return false;
        pParentPM = pParentPM->GetParentResource();
    }
    m_pParentResourcePM = pm;
    return true;
}

CPaintManagerUI* CPaintManagerUI::GetParentResource() const
{
    return m_pParentResourcePM;
}

DWORD CPaintManagerUI::GetDefaultDisabledColor() const
{
    if( m_pParentResourcePM ) return m_pParentResourcePM->GetDefaultDisabledColor();
    return m_dwDefaultDisabledColor;
}

void CPaintManagerUI::SetDefaultDisabledColor(DWORD dwColor)
{
    m_dwDefaultDisabledColor = dwColor;
}

DWORD CPaintManagerUI::GetDefaultFontColor() const
{
    if( m_pParentResourcePM ) return m_pParentResourcePM->GetDefaultFontColor();
    return m_dwDefaultFontColor;
}

void CPaintManagerUI::SetDefaultFontColor(DWORD dwColor)
{
    m_dwDefaultFontColor = dwColor;
}

DWORD CPaintManagerUI::GetDefaultLinkFontColor() const
{
    if( m_pParentResourcePM ) return m_pParentResourcePM->GetDefaultLinkFontColor();
    return m_dwDefaultLinkFontColor;
}

void CPaintManagerUI::SetDefaultLinkFontColor(DWORD dwColor)
{
    m_dwDefaultLinkFontColor = dwColor;
}

DWORD CPaintManagerUI::GetDefaultLinkHoverFontColor() const
{
    if( m_pParentResourcePM ) return m_pParentResourcePM->GetDefaultLinkHoverFontColor();
    return m_dwDefaultLinkHoverFontColor;
}

void CPaintManagerUI::SetDefaultLinkHoverFontColor(DWORD dwColor)
{
    m_dwDefaultLinkHoverFontColor = dwColor;
}

DWORD CPaintManagerUI::GetDefaultSelectedBkColor() const
{
    if( m_pParentResourcePM ) return m_pParentResourcePM->GetDefaultSelectedBkColor();
    return m_dwDefaultSelectedBkColor;
}

void CPaintManagerUI::SetDefaultSelectedBkColor(DWORD dwColor)
{
    m_dwDefaultSelectedBkColor = dwColor;
}

TFontInfo* CPaintManagerUI::GetDefaultFontInfo()
{
    if( m_pParentResourcePM ) return m_pParentResourcePM->GetDefaultFontInfo();

    if( m_DefaultFontInfo.tm.tmHeight == 0 ) {
        HFONT hOldFont = (HFONT) ::SelectObject(m_hDcPaint, m_DefaultFontInfo.hFont);
        ::GetTextMetrics(m_hDcPaint, &m_DefaultFontInfo.tm);
        ::SelectObject(m_hDcPaint, hOldFont);
    }
    return &m_DefaultFontInfo;
}

void CPaintManagerUI::SetDefaultFont(LPCTSTR pStrFontName, int nSize, bool bBold, bool bUnderline, bool bItalic)
{
    LOGFONT lf = { 0 };
    ::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
    _tcscpy(lf.lfFaceName, pStrFontName);
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfHeight = -nSize;
    if( bBold ) lf.lfWeight += FW_BOLD;
    if( bUnderline ) lf.lfUnderline = TRUE;
    if( bItalic ) lf.lfItalic = TRUE;
    HFONT hFont = ::CreateFontIndirect(&lf);
    if( hFont == NULL ) return;

    ::DeleteObject(m_DefaultFontInfo.hFont);
    m_DefaultFontInfo.hFont = hFont;
    m_DefaultFontInfo.sFontName = pStrFontName;
    m_DefaultFontInfo.iSize = nSize;
    m_DefaultFontInfo.bBold = bBold;
    m_DefaultFontInfo.bUnderline = bUnderline;
    m_DefaultFontInfo.bItalic = bItalic;
    ::ZeroMemory(&m_DefaultFontInfo.tm, sizeof(m_DefaultFontInfo.tm));
    if( m_hDcPaint ) {
        HFONT hOldFont = (HFONT) ::SelectObject(m_hDcPaint, hFont);
        ::GetTextMetrics(m_hDcPaint, &m_DefaultFontInfo.tm);
        ::SelectObject(m_hDcPaint, hOldFont);
    }
}

DWORD CPaintManagerUI::GetCustomFontCount() const
{
    return m_aCustomFonts.GetSize();
}

HFONT CPaintManagerUI::AddFont(LPCTSTR pStrFontName, int nSize, bool bBold, bool bUnderline, bool bItalic)
{
    LOGFONT lf = { 0 };
    ::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
    _tcscpy(lf.lfFaceName, pStrFontName);
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfHeight = -nSize;
    if( bBold ) lf.lfWeight += FW_BOLD;
    if( bUnderline ) lf.lfUnderline = TRUE;
    if( bItalic ) lf.lfItalic = TRUE;
    HFONT hFont = ::CreateFontIndirect(&lf);
    if( hFont == NULL ) return NULL;

    TFontInfo* pFontInfo = new TFontInfo;
    if( !pFontInfo ) return false;
    ::ZeroMemory(pFontInfo, sizeof(TFontInfo));
    pFontInfo->hFont = hFont;
    pFontInfo->sFontName = pStrFontName;
    pFontInfo->iSize = nSize;
    pFontInfo->bBold = bBold;
    pFontInfo->bUnderline = bUnderline;
    pFontInfo->bItalic = bItalic;
    if( m_hDcPaint ) {
        HFONT hOldFont = (HFONT) ::SelectObject(m_hDcPaint, hFont);
        ::GetTextMetrics(m_hDcPaint, &pFontInfo->tm);
        ::SelectObject(m_hDcPaint, hOldFont);
    }
    if( !m_aCustomFonts.Add(pFontInfo) ) {
        ::DeleteObject(hFont);
        delete pFontInfo;
        return NULL;
    }

    return hFont;
}

HFONT CPaintManagerUI::AddFontAt(int index, LPCTSTR pStrFontName, int nSize, bool bBold, bool bUnderline, bool bItalic)
{
    LOGFONT lf = { 0 };
    ::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
    _tcscpy(lf.lfFaceName, pStrFontName);
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfHeight = -nSize;
    if( bBold ) lf.lfWeight += FW_BOLD;
    if( bUnderline ) lf.lfUnderline = TRUE;
    if( bItalic ) lf.lfItalic = TRUE;
    HFONT hFont = ::CreateFontIndirect(&lf);
    if( hFont == NULL ) return NULL;

    TFontInfo* pFontInfo = new TFontInfo;
    if( !pFontInfo ) return false;
    ::ZeroMemory(pFontInfo, sizeof(TFontInfo));
    pFontInfo->hFont = hFont;
    pFontInfo->sFontName = pStrFontName;
    pFontInfo->iSize = nSize;
    pFontInfo->bBold = bBold;
    pFontInfo->bUnderline = bUnderline;
    pFontInfo->bItalic = bItalic;
    if( m_hDcPaint ) {
        HFONT hOldFont = (HFONT) ::SelectObject(m_hDcPaint, hFont);
        ::GetTextMetrics(m_hDcPaint, &pFontInfo->tm);
        ::SelectObject(m_hDcPaint, hOldFont);
    }
    if( !m_aCustomFonts.InsertAt(index, pFontInfo) ) {
        ::DeleteObject(hFont);
        delete pFontInfo;
        return NULL;
    }

    return hFont;
}

HFONT CPaintManagerUI::GetFont(int index)
{
    if( index < 0 || index >= m_aCustomFonts.GetSize() ) return GetDefaultFontInfo()->hFont;
    TFontInfo* pFontInfo = static_cast<TFontInfo*>(m_aCustomFonts[index]);
    return pFontInfo->hFont;
}

HFONT CPaintManagerUI::GetFont(LPCTSTR pStrFontName, int nSize, bool bBold, bool bUnderline, bool bItalic)
{
    TFontInfo* pFontInfo = NULL;
    for( int it = 0; it < m_aCustomFonts.GetSize(); it++ ) {
        pFontInfo = static_cast<TFontInfo*>(m_aCustomFonts[it]);
        if( pFontInfo->sFontName == pStrFontName && pFontInfo->iSize == nSize && 
            pFontInfo->bBold == bBold && pFontInfo->bUnderline == bUnderline && pFontInfo->bItalic == bItalic) 
            return pFontInfo->hFont;
    }
    if( m_pParentResourcePM ) return m_pParentResourcePM->GetFont(pStrFontName, nSize, bBold, bUnderline, bItalic);
    return NULL;
}

bool CPaintManagerUI::FindFont(HFONT hFont)
{
    TFontInfo* pFontInfo = NULL;
    for( int it = 0; it < m_aCustomFonts.GetSize(); it++ ) {
        pFontInfo = static_cast<TFontInfo*>(m_aCustomFonts[it]);
        if( pFontInfo->hFont == hFont ) return true;
    }
    if( m_pParentResourcePM ) return m_pParentResourcePM->FindFont(hFont);
    return false;
}

bool CPaintManagerUI::FindFont(LPCTSTR pStrFontName, int nSize, bool bBold, bool bUnderline, bool bItalic)
{
    TFontInfo* pFontInfo = NULL;
    for( int it = 0; it < m_aCustomFonts.GetSize(); it++ ) {
        pFontInfo = static_cast<TFontInfo*>(m_aCustomFonts[it]);
        if( pFontInfo->sFontName == pStrFontName && pFontInfo->iSize == nSize && 
            pFontInfo->bBold == bBold && pFontInfo->bUnderline == bUnderline && pFontInfo->bItalic == bItalic) 
            return true;
    }
    if( m_pParentResourcePM ) return m_pParentResourcePM->FindFont(pStrFontName, nSize, bBold, bUnderline, bItalic);
    return false;
}

int CPaintManagerUI::GetFontIndex(HFONT hFont)
{
    TFontInfo* pFontInfo = NULL;
    for( int it = 0; it < m_aCustomFonts.GetSize(); it++ ) {
        pFontInfo = static_cast<TFontInfo*>(m_aCustomFonts[it]);
        if( pFontInfo->hFont == hFont ) return it;
    }
    return -1;
}

int CPaintManagerUI::GetFontIndex(LPCTSTR pStrFontName, int nSize, bool bBold, bool bUnderline, bool bItalic)
{
    TFontInfo* pFontInfo = NULL;
    for( int it = 0; it < m_aCustomFonts.GetSize(); it++ ) {
        pFontInfo = static_cast<TFontInfo*>(m_aCustomFonts[it]);
        if( pFontInfo->sFontName == pStrFontName && pFontInfo->iSize == nSize && 
            pFontInfo->bBold == bBold && pFontInfo->bUnderline == bUnderline && pFontInfo->bItalic == bItalic) 
            return it;
    }
    return -1;
}

bool CPaintManagerUI::RemoveFont(HFONT hFont)
{
    TFontInfo* pFontInfo = NULL;
    for( int it = 0; it < m_aCustomFonts.GetSize(); it++ ) {
        pFontInfo = static_cast<TFontInfo*>(m_aCustomFonts[it]);
        if( pFontInfo->hFont == hFont ) {
            ::DeleteObject(pFontInfo->hFont);
            delete pFontInfo;
            return m_aCustomFonts.Remove(it);
        }
    }

    return false;
}

bool CPaintManagerUI::RemoveFontAt(int index)
{
    if( index < 0 || index >= m_aCustomFonts.GetSize() ) return false;
    TFontInfo* pFontInfo = static_cast<TFontInfo*>(m_aCustomFonts[index]);
    ::DeleteObject(pFontInfo->hFont);
    delete pFontInfo;
    return m_aCustomFonts.Remove(index);
}

void CPaintManagerUI::RemoveAllFonts()
{
    TFontInfo* pFontInfo;
    for( int it = 0; it < m_aCustomFonts.GetSize(); it++ ) {
        pFontInfo = static_cast<TFontInfo*>(m_aCustomFonts[it]);
        ::DeleteObject(pFontInfo->hFont);
        delete pFontInfo;
    }
    m_aCustomFonts.Empty();
}

TFontInfo* CPaintManagerUI::GetFontInfo(int index)
{
    if( index < 0 || index >= m_aCustomFonts.GetSize() ) return GetDefaultFontInfo();
    TFontInfo* pFontInfo = static_cast<TFontInfo*>(m_aCustomFonts[index]);
    if( pFontInfo->tm.tmHeight == 0 ) {
        HFONT hOldFont = (HFONT) ::SelectObject(m_hDcPaint, pFontInfo->hFont);
        ::GetTextMetrics(m_hDcPaint, &pFontInfo->tm);
        ::SelectObject(m_hDcPaint, hOldFont);
    }
    return pFontInfo;
}

TFontInfo* CPaintManagerUI::GetFontInfo(HFONT hFont)
{
    TFontInfo* pFontInfo = NULL;
    for( int it = 0; it < m_aCustomFonts.GetSize(); it++ ) {
        pFontInfo = static_cast<TFontInfo*>(m_aCustomFonts[it]);
        if( pFontInfo->hFont == hFont ) {
            if( pFontInfo->tm.tmHeight == 0 ) {
                HFONT hOldFont = (HFONT) ::SelectObject(m_hDcPaint, pFontInfo->hFont);
                ::GetTextMetrics(m_hDcPaint, &pFontInfo->tm);
                ::SelectObject(m_hDcPaint, hOldFont);
            }
            return pFontInfo;
        }
    }

    if( m_pParentResourcePM ) return m_pParentResourcePM->GetFontInfo(hFont);
    return GetDefaultFontInfo();
}

const TImageInfo* CPaintManagerUI::GetImage(LPCTSTR bitmap)
{
    TImageInfo* data = static_cast<TImageInfo*>(m_mImageHash.Find(bitmap));
    if( !data && m_pParentResourcePM ) return m_pParentResourcePM->GetImage(bitmap);
    else return data;
}

const TImageInfo* CPaintManagerUI::GetImageEx(LPCTSTR bitmap, LPCTSTR type, DWORD mask)
{
    TImageInfo* data = static_cast<TImageInfo*>(m_mImageHash.Find(bitmap));
    if( !data ) {
        if( AddImage(bitmap, type, mask) ) {
            data = static_cast<TImageInfo*>(m_mImageHash.Find(bitmap));
        }
    }

    return data;
}

//加入旋转的临时数据
void  CPaintManagerUI::AddRotationImage(LPCTSTR bitmap, HBITMAP hBitmap, double nRotation)
{
	TImageInfo* data = static_cast<TImageInfo*>(m_mImageHash.Find(bitmap));
	if( data )
	{
		if (NULL != data->hTmpBitmap)
		{
			::DeleteObject(data->hTmpBitmap);
			data->hTmpBitmap = NULL;
		}
		data->hTmpBitmap = hBitmap;
		data->bitmapParam.nRotation = nRotation;
	}
}


const TImageInfo* CPaintManagerUI::AddImage(LPCTSTR bitmap, LPCTSTR type, DWORD mask)
{
    TImageInfo* data = NULL;
    if( type != NULL ) {
        if( isdigit(*bitmap) ) {
            LPTSTR pstr = NULL;
            int iIndex = _tcstol(bitmap, &pstr, 10);
            data = CRenderEngine::LoadImage(iIndex, type, mask);
        }
    }
    else {
        data = CRenderEngine::LoadImage(bitmap, NULL, mask);
    }

    if( !data ) return NULL;
    if( type != NULL ) data->sResType = type;
    data->dwMask = mask;
    if( !m_mImageHash.Insert(bitmap, data) ) {
        ::DeleteObject(data->hBitmap);
		if (NULL != data->hTmpBitmap)
		{
			::DeleteObject(data->hTmpBitmap);
			data->hTmpBitmap = NULL;
		}
        delete data;
		data = NULL;
    }

    return data;
}

const TImageInfo* CPaintManagerUI::AddImage(LPCTSTR bitmap, HBITMAP hBitmap, int iWidth, int iHeight, bool bAlpha)
{
    if( hBitmap == NULL || iWidth <= 0 || iHeight <= 0 ) return NULL;

    TImageInfo* data = new TImageInfo;
    data->hBitmap = hBitmap;
    data->nX = iWidth;
    data->nY = iHeight;
    data->alphaChannel = bAlpha;
	data->hTmpBitmap = NULL;
    //data->sResType = _T("");
    data->dwMask = 0;
    if( !m_mImageHash.Insert(bitmap, data) ) {
        ::DeleteObject(data->hBitmap);
		if (NULL != data->hTmpBitmap)
		{
			::DeleteObject(data->hTmpBitmap);
			data->hTmpBitmap = NULL;
		}
        delete data;
		data = NULL;
    }

    return data;
}

bool CPaintManagerUI::RemoveImage(LPCTSTR bitmap)
{
    const TImageInfo* data = GetImage(bitmap);
    if( !data ) return false;

    ::DeleteObject(data->hBitmap);
	if (data->hTmpBitmap)
	::DeleteObject(data->hTmpBitmap);
    delete data;

    return m_mImageHash.Remove(bitmap);
}

void CPaintManagerUI::RemoveAllImages()
{
    TImageInfo* data;
    for( int i = 0; i< m_mImageHash.GetSize(); i++ ) {
        if(LPCTSTR key = m_mImageHash.GetAt(i)) {
            data = static_cast<TImageInfo*>(m_mImageHash.Find(key));
            ::DeleteObject(data->hBitmap);
			if (data->hTmpBitmap)
				::DeleteObject(data->hTmpBitmap);
            delete data;
        }
    }
}

void CPaintManagerUI::ReloadAllImages()
{
    bool bRedraw = false;
    TImageInfo* data;
    TImageInfo* pNewData;
    for( int i = 0; i< m_mImageHash.GetSize(); i++ ) {
        if(LPCTSTR bitmap = m_mImageHash.GetAt(i)) {
            data = static_cast<TImageInfo*>(m_mImageHash.Find(bitmap));
            if( data != NULL ) {
                if( !data->sResType.IsEmpty() ) {
                    if( isdigit(*bitmap) ) {
                        LPTSTR pstr = NULL;
                        int iIndex = _tcstol(bitmap, &pstr, 10);
                        pNewData = CRenderEngine::LoadImage(iIndex, data->sResType.GetData(), data->dwMask);
                    }
                }
                else {
                    pNewData = CRenderEngine::LoadImage(bitmap, NULL, data->dwMask);
                }
                if( pNewData == NULL ) continue;

				if (data->hBitmap != NULL)
				{
					::DeleteObject(data->hBitmap);
					if (data->hTmpBitmap)
						::DeleteObject(data->hTmpBitmap);
				}
                data->hBitmap = pNewData->hBitmap;
                data->nX = pNewData->nX;
                data->nY = pNewData->nY;
                data->alphaChannel = pNewData->alphaChannel;
				data->hTmpBitmap = pNewData->hTmpBitmap;
                delete pNewData;
                bRedraw = true;
            }
        }
    }
    if( bRedraw && m_pRoot ) m_pRoot->Invalidate();
}

void CPaintManagerUI::AddDefaultAttributeList(LPCTSTR pStrControlName, LPCTSTR pStrControlAttrList)
{
	CStdString* pDefaultAttr = new CStdString(pStrControlAttrList);
	if (pDefaultAttr != NULL)
	{
		if (m_DefaultAttrHash.Find(pStrControlName) == NULL)
			m_DefaultAttrHash.Set(pStrControlName, (LPVOID)pDefaultAttr);
		else
			delete pDefaultAttr;
	}
}

LPCTSTR CPaintManagerUI::GetDefaultAttributeList(LPCTSTR pStrControlName) const
{
    CStdString* pDefaultAttr = static_cast<CStdString*>(m_DefaultAttrHash.Find(pStrControlName));
    if( !pDefaultAttr && m_pParentResourcePM ) return m_pParentResourcePM->GetDefaultAttributeList(pStrControlName);
    
    if( pDefaultAttr ) return pDefaultAttr->GetData();
    else return NULL;
}

bool CPaintManagerUI::RemoveDefaultAttributeList(LPCTSTR pStrControlName)
{
    CStdString* pDefaultAttr = static_cast<CStdString*>(m_DefaultAttrHash.Find(pStrControlName));
    if( !pDefaultAttr ) return false;

    delete pDefaultAttr;
    return m_DefaultAttrHash.Remove(pStrControlName);
}

const CStdStringPtrMap& CPaintManagerUI::GetDefaultAttribultes() const
{
	return m_DefaultAttrHash;
}

void CPaintManagerUI::RemoveAllDefaultAttributeList()
{
	CStdString* pDefaultAttr;
	for( int i = 0; i< m_DefaultAttrHash.GetSize(); i++ ) {
		if(LPCTSTR key = m_DefaultAttrHash.GetAt(i)) {
			pDefaultAttr = static_cast<CStdString*>(m_DefaultAttrHash.Find(key));
			delete pDefaultAttr;
		}
	}
}

HFONT CPaintManagerUI::GetThemeFont(UITYPE_FONT Index) const
{
	if( Index <= UIFONT__FIRST || Index >= UIFONT__LAST ) return NULL;
	if( m_hFonts[Index] == NULL ) m_hFonts[Index] = ::CreateFontIndirect(&m_aLogFonts[Index]);
	return m_hFonts[Index];
}

HICON CPaintManagerUI::GetThemeIcon(int iIndex, int cxySize) const
{
	if( m_himgIcons16 == NULL ) {
		m_himgIcons16 = ImageList_LoadImage(m_hInstance, MAKEINTRESOURCE(IDB_ICONS16), 16, 0, RGB(255,0,255), IMAGE_BITMAP, LR_CREATEDIBSECTION);
		m_himgIcons24 = ImageList_LoadImage(m_hInstance, MAKEINTRESOURCE(IDB_ICONS16), 16, 0, RGB(255,0,255), IMAGE_BITMAP, LR_CREATEDIBSECTION);
		m_himgIcons32 = ImageList_LoadImage(m_hInstance, MAKEINTRESOURCE(IDB_ICONS16), 16, 0, RGB(255,0,255), IMAGE_BITMAP, LR_CREATEDIBSECTION);
		m_himgIcons50 = ImageList_LoadImage(m_hInstance, MAKEINTRESOURCE(IDB_ICONS50), 50, 0, RGB(255,0,255), IMAGE_BITMAP, LR_CREATEDIBSECTION);
	}
	if( cxySize == 16 ) return ImageList_GetIcon(m_himgIcons16, iIndex, ILD_NORMAL);
	else if( cxySize == 24 ) return ImageList_GetIcon(m_himgIcons24, iIndex, ILD_NORMAL);
	else if( cxySize == 32 ) return ImageList_GetIcon(m_himgIcons32, iIndex, ILD_NORMAL);
	else if( cxySize == 50 ) return ImageList_GetIcon(m_himgIcons50, iIndex, ILD_NORMAL);
	return NULL;
}

HPEN CPaintManagerUI::GetThemePen(UITYPE_COLOR Index) const
{
	if( Index <= UICOLOR__FIRST || Index >= UICOLOR__LAST ) return NULL;
	if( m_hPens[Index] == NULL ) m_hPens[Index] = ::CreatePen(PS_SOLID, 1, m_clrColors[Index][0]);
	return m_hPens[Index];
}

HBRUSH CPaintManagerUI::GetThemeBrush(UITYPE_COLOR Index) const
{
	if( Index <= UICOLOR__FIRST || Index >= UICOLOR__LAST ) return NULL;
	if( m_hBrushes[Index] == NULL ) m_hBrushes[Index] = ::CreateSolidBrush(m_clrColors[Index][0]);
	return m_hBrushes[Index];
}

const TEXTMETRIC& CPaintManagerUI::GetThemeFontInfo(UITYPE_FONT Index) const
{
	if( Index <= UIFONT__FIRST || Index >= UIFONT__LAST ) return m_aTextMetrics[0];
	if( m_aTextMetrics[Index].tmHeight == 0 ) {
		HFONT hOldFont = (HFONT) ::SelectObject(m_hDcPaint, GetThemeFont(Index));
		::GetTextMetrics(m_hDcPaint, &m_aTextMetrics[Index]);
		::SelectObject(m_hDcPaint, hOldFont);
	}
	return m_aTextMetrics[Index];
}

COLORREF CPaintManagerUI::GetThemeColor(UITYPE_COLOR Index) const
{
	if( Index <= UICOLOR__FIRST || Index >= UICOLOR__LAST ) return RGB(0,0,0);
	return m_clrColors[Index][0];
}

bool CPaintManagerUI::GetThemeColorPair(UITYPE_COLOR Index, COLORREF& clr1, COLORREF& clr2) const
{
	if( Index <= UICOLOR__FIRST || Index >= UICOLOR__LAST ) return false;
	clr1 = m_clrColors[Index][0];
	clr2 = m_clrColors[Index][1];
	return true;
}

CControlUI* CPaintManagerUI::GetRoot() const
{
    ASSERT(m_pRoot);
    return m_pRoot;
}

CControlUI* CPaintManagerUI::FindControl(POINT pt) const
{
    ASSERT(m_pRoot);
    return m_pRoot->FindControl(__FindControlFromPoint, &pt, UIFIND_VISIBLE | UIFIND_HITTEST | UIFIND_TOP_FIRST);
}

CControlUI* CPaintManagerUI::FindControl(LPCTSTR pstrName) const
{
    ASSERT(m_pRoot);
    return static_cast<CControlUI*>(m_mNameHash.Find(pstrName));
}

CControlUI* CPaintManagerUI::FindSubControlByPoint(CControlUI* pParent, POINT pt) const
{
    if( pParent == NULL ) pParent = GetRoot();
    ASSERT(pParent);
    return pParent->FindControl(__FindControlFromPoint, &pt, UIFIND_VISIBLE | UIFIND_HITTEST | UIFIND_TOP_FIRST);
}

CControlUI* CPaintManagerUI::FindSubControlByName(CControlUI* pParent, LPCTSTR pstrName) const
{
    if( pParent == NULL ) pParent = GetRoot();
    ASSERT(pParent);
    return pParent->FindControl(__FindControlFromName, (LPVOID)pstrName, UIFIND_ALL);
}

CControlUI* CPaintManagerUI::FindSubControlByClass(CControlUI* pParent, LPCTSTR pstrClass, int iIndex)
{
    if( pParent == NULL ) pParent = GetRoot();
    ASSERT(pParent);
    m_aFoundControls.Resize(iIndex + 1);
    return pParent->FindControl(__FindControlFromClass, (LPVOID)pstrClass, UIFIND_ALL);
}

CStdPtrArray* CPaintManagerUI::FindSubControlsByClass(CControlUI* pParent, LPCTSTR pstrClass)
{
    if( pParent == NULL ) pParent = GetRoot();
    ASSERT(pParent);
    m_aFoundControls.Empty();
    pParent->FindControl(__FindControlsFromClass, (LPVOID)pstrClass, UIFIND_ALL);
    return &m_aFoundControls;
}

CStdPtrArray* CPaintManagerUI::GetSubControlsByClass()
{
    return &m_aFoundControls;
}

CControlUI* CALLBACK CPaintManagerUI::__FindControlFromNameHash(CControlUI* pThis, LPVOID pData)
{
    CPaintManagerUI* pManager = static_cast<CPaintManagerUI*>(pData);
    const CStdString& sName = pThis->GetName();
    if( sName.IsEmpty() ) 
		return NULL;
    // Add this control to the hash list
    pManager->m_mNameHash.Set(sName, pThis);
    return NULL; // Attempt to add all controls
}

CControlUI* CALLBACK CPaintManagerUI::__FindControlFromCount(CControlUI* /*pThis*/, LPVOID pData)
{
    int* pnCount = static_cast<int*>(pData);
    (*pnCount)++;
    return NULL;  // Count all controls
}

CControlUI* CALLBACK CPaintManagerUI::__FindControlFromPoint(CControlUI* pThis, LPVOID pData)
{
    LPPOINT pPoint = static_cast<LPPOINT>(pData);
    return ::PtInRect(&pThis->GetPos(), *pPoint) ? pThis : NULL;
}

CControlUI* CALLBACK CPaintManagerUI::__FindControlFromTab(CControlUI* pThis, LPVOID pData)
{
    FINDTABINFO* pInfo = static_cast<FINDTABINFO*>(pData);
    if( pInfo->pFocus == pThis ) {
        if( pInfo->bForward ) pInfo->bNextIsIt = true;
        return pInfo->bForward ? NULL : pInfo->pLast;
    }
    if( (pThis->GetControlFlags() & UIFLAG_TABSTOP) == 0 ) return NULL;
    pInfo->pLast = pThis;
    if( pInfo->bNextIsIt ) return pThis;
    if( pInfo->pFocus == NULL ) return pThis;
    return NULL;  // Examine all controls
}

CControlUI* CALLBACK CPaintManagerUI::__FindControlFromShortcut(CControlUI* pThis, LPVOID pData)
{
    if( !pThis->IsVisible() ) return NULL; 
    FINDSHORTCUT* pFS = static_cast<FINDSHORTCUT*>(pData);
    if( pFS->ch == toupper(pThis->GetShortcut()) ) pFS->bPickNext = true;
    if( _tcsstr(pThis->GetClass(), _T("LabelUI")) != NULL ) return NULL;   // Labels never get focus!
    return pFS->bPickNext ? pThis : NULL;
}

CControlUI* CALLBACK CPaintManagerUI::__FindControlFromUpdate(CControlUI* pThis, LPVOID pData)
{
    return pThis->IsUpdateNeeded() ? pThis : NULL;
}

CControlUI* CALLBACK CPaintManagerUI::__FindControlFromName(CControlUI* pThis, LPVOID pData)
{
    LPCTSTR pstrName = static_cast<LPCTSTR>(pData);
    const CStdString& sName = pThis->GetName();
    if( sName.IsEmpty() ) return NULL;
    return (_tcsicmp(sName, pstrName) == 0) ? pThis : NULL;
}

CControlUI* CALLBACK CPaintManagerUI::__FindControlFromClass(CControlUI* pThis, LPVOID pData)
{
    LPCTSTR pstrType = static_cast<LPCTSTR>(pData);
    LPCTSTR pType = pThis->GetClass();
    CStdPtrArray* pFoundControls = pThis->GetManager()->GetSubControlsByClass();
    if( _tcscmp(pstrType, _T("*")) == 0 || _tcscmp(pstrType, pType) == 0 ) {
        int iIndex = -1;
        while( pFoundControls->GetAt(++iIndex) != NULL ) ;
        if( iIndex < pFoundControls->GetSize() ) pFoundControls->SetAt(iIndex, pThis);
    }
    if( pFoundControls->GetAt(pFoundControls->GetSize() - 1) != NULL ) return pThis; 
    return NULL;
}

CControlUI* CALLBACK CPaintManagerUI::__FindControlsFromClass(CControlUI* pThis, LPVOID pData)
{
    LPCTSTR pstrType = static_cast<LPCTSTR>(pData);
    LPCTSTR pType = pThis->GetClass();
    if( _tcscmp(pstrType, _T("*")) == 0 || _tcscmp(pstrType, pType) == 0 ) 
        pThis->GetManager()->GetSubControlsByClass()->Add((LPVOID)pThis);
    return NULL;
}

} // namespace DuiLib
