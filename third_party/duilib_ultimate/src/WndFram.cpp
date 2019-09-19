#include "stdafx.h"

namespace DuiLib {

CWndFram::CWndFram(void)
{
}

CWndFram::~CWndFram(void)
{
}


//处理键盘消息，转为快捷键消息
void CWndFram::HandleKeyMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if ((hwnd != NULL) && ::IsWindow(hwnd))
	{
		if (uMsg == WM_KEYDOWN)
		{
			switch(wParam)
			{
			case VK_F5:
				SendMessage(WM_HOTKEY, Accelerator_F5, 0);
				break;
			case VK_BACK:
				SendMessage(WM_HOTKEY, Accelerator_Backspace, 0);
				break;
			case VK_RETURN:
				SendMessage(WM_HOTKEY, Accelerator_Enter, 0);
				break;
			case VK_ESCAPE:
				SendMessage(WM_HOTKEY, Accelerator_ESC, 0);
				break;
			case 'A':
				{
					if (::GetKeyState(VK_CONTROL) < 0)
					{
						SendMessage(WM_HOTKEY, Accelerator_SelectAll, 0);
					}
				}
				break;
			case 'C':
				{
					if (::GetKeyState(VK_CONTROL) < 0)
					{
						SendMessage(WM_HOTKEY, Accelerator_Copy, 0);
					}
				}
				break;
			case 'V':
				{
					if (::GetKeyState(VK_CONTROL) < 0)
					{
						SendMessage(WM_HOTKEY, Accelerator_Paste, 0);
					}
				}
				break;
			case 'X':
				{
					if (::GetKeyState(VK_CONTROL) < 0)
					{
						SendMessage(WM_HOTKEY, Accelerator_Cut, 0);
					}
				}
				break;
			case VK_DELETE:
				{
					SendMessage(WM_HOTKEY, Accelerator_Delete, 0);
				}
				break;
			case VK_LEFT:
				{
					SendMessage(WM_HOTKEY, Accelerator_VK_Left, 0);
				}
				break;
			case VK_RIGHT:
				{
					SendMessage(WM_HOTKEY, Accelerator_VK_Right, 0);
				}
				break;
			case VK_UP:
				{
					SendMessage(WM_HOTKEY, Accelerator_VK_Up, 0);
				}
				break;
			case VK_DOWN:
				{
					SendMessage(WM_HOTKEY, Accelerator_VK_Down, 0);
				}
				break;
			default:
				break;
			}
		}
	}

}

LRESULT CWndFram::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = 0;
	HandleKeyMessage(m_hWnd, uMsg, wParam, lParam);
	BOOL bHandled = TRUE; 
	switch( uMsg ) {
		case WM_CLOSE:        
			lRes = OnClose(uMsg, wParam, lParam, bHandled); break;
		case WM_DESTROY:     
			lRes = OnDestroy(uMsg, wParam, lParam, bHandled); break;
		case WM_NCACTIVATE:   
			lRes = OnNcActivate(uMsg, wParam, lParam, bHandled); break;
		case WM_NCCALCSIZE: 
			lRes = OnNcCalcSize(uMsg, wParam, lParam, bHandled); break;
		case WM_NCPAINT:    
			lRes = OnNcPaint(uMsg, wParam, lParam, bHandled); break;
		case WM_NCHITTEST:   
			lRes = OnNcHitTest(uMsg, wParam, lParam, bHandled); break;
		case WM_SIZE:        
			lRes = OnSize(uMsg, wParam, lParam, bHandled); break;
		case WM_GETMINMAXINFO: 
			lRes = OnGetMinMaxInfo(uMsg, wParam, lParam, bHandled); break;
		case WM_SYSCOMMAND:   
			lRes = OnSysCommand(uMsg, wParam, lParam, bHandled); break;
		default:
			bHandled = FALSE;
	}
	if( bHandled ) return lRes;
	if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;
	return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
}

LRESULT CWndFram::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//::PostQuitMessage(0);
	bHandled = FALSE;
	return 0;
}

LRESULT CWndFram::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//::PostQuitMessage(0L);
	bHandled = FALSE;
	return 0;
}

LRESULT CWndFram::OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if( ::IsIconic(*this) ) bHandled = FALSE;
	return (wParam == 0) ? TRUE : FALSE;
}

LRESULT CWndFram::OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

LRESULT CWndFram::OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}
LRESULT CWndFram::OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	POINT pt; pt.x = GET_X_LPARAM(lParam); pt.y = GET_Y_LPARAM(lParam);
	::ScreenToClient(*this, &pt);

	RECT rcClient;
	::GetClientRect(*this, &rcClient);

	//www 2016.04.01 还原拖动功能
	if (!::IsZoomed(*this)) {
		RECT rcSizeBox = m_pm.GetSizeBox();
		if (pt.y < rcClient.top + rcSizeBox.top) {
			if (pt.x < rcClient.left + rcSizeBox.left) return HTTOPLEFT;
			if (pt.x > rcClient.right - rcSizeBox.right) return HTTOPRIGHT;
			return HTTOP;
		}
		else if (pt.y > rcClient.bottom - rcSizeBox.bottom) {
			if (pt.x < rcClient.left + rcSizeBox.left) return HTBOTTOMLEFT;
			if (pt.x > rcClient.right - rcSizeBox.right) return HTBOTTOMRIGHT;
			return HTBOTTOM;
		}
		if (pt.x < rcClient.left + rcSizeBox.left) return HTLEFT;
		if (pt.x > rcClient.right - rcSizeBox.right) return HTRIGHT;
	}

	RECT rcCaption = m_pm.GetCaptionRect();
	if( pt.x >= rcClient.left + rcCaption.left && pt.x < rcClient.right - rcCaption.right \
		&& pt.y >= rcCaption.top && pt.y < rcCaption.bottom ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(pt));
			if( pControl && _tcscmp(pControl->GetClass(), _T("ButtonUI")) != 0 && 
				_tcscmp(pControl->GetClass(), _T("OptionUI")) != 0 &&
				_tcscmp(pControl->GetClass(), _T("TextButton")) !=0  && 
				_tcscmp(pControl->GetClass(),_T("NonButton")) != 0)
				return HTCAPTION;
	}

	return HTCLIENT;
}

LRESULT CWndFram::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	SIZE szRoundCorner = m_pm.GetRoundCorner();
	if( !::IsIconic(*this) && (szRoundCorner.cx != 0 || szRoundCorner.cy != 0) ) {
		CRect rcWnd;
		::GetWindowRect(*this, &rcWnd);
   		rcWnd.Offset(-rcWnd.left, -rcWnd.top);
  		rcWnd.right++; rcWnd.bottom++;
// 		RECT rc = { rcWnd.left, rcWnd.top/* + szRoundCorner.cx*/, rcWnd.right, rcWnd.bottom };
// 		HRGN hRgn1 = ::CreateRectRgnIndirect( &rc );
		szRoundCorner.cx += 2;
		HRGN hRgn2 = ::CreateRoundRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom /*- szRoundCorner.cx*/, szRoundCorner.cx, szRoundCorner.cy);
//		::CombineRgn( hRgn1, hRgn1, hRgn2, RGN_OR );
		::SetWindowRgn(*this, hRgn2, TRUE);
//		::DeleteObject(hRgn1);
		::DeleteObject(hRgn2);
	}

	bHandled = FALSE;

	if ((DWORD)wParam == SIZE_MAXIMIZED)
	{
		// 修复双屏幕下最大化窗口显示不全的问题 youyifang
		int nWidthWin = LOWORD(lParam);
		int nHeightWin = HIWORD(lParam);
		HMONITOR hMonitor;
		MONITORINFO mi;
		hMonitor = MonitorFromWindow(GetHWND(), MONITOR_DEFAULTTONEAREST);
		if (hMonitor != INVALID_HANDLE_VALUE)
		{
			mi.cbSize = sizeof(mi);
			GetMonitorInfo(hMonitor, &mi);
			int y = mi.rcMonitor.bottom - mi.rcMonitor.top;
			int x = mi.rcMonitor.right - mi.rcMonitor.left;
			int nHeight = mi.rcWork.bottom - mi.rcWork.top;
			int nWidth = mi.rcWork.right - mi.rcWork.left;

			bool bResetPos = false;
			if (nWidthWin > nWidth)
			{
				bResetPos = true;
				nWidthWin = nWidth;
			}
			if (nHeightWin > nHeight)
			{
				bResetPos = true;
				nHeightWin = nHeight;
			}
			if (bResetPos)
			{
				bHandled = false;
				::SetWindowPos(GetHWND(), NULL, 0, 0, nWidthWin, nHeightWin, SWP_NOMOVE);
				return 0;
			}
		}
	}

	return 0;
}

LRESULT CWndFram::OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	MONITORINFO oMonitor = {};
	oMonitor.cbSize = sizeof(oMonitor);
	::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTOPRIMARY), &oMonitor);
	CRect rcWork = oMonitor.rcWork;
	rcWork.Offset(-rcWork.left, -rcWork.top);

	LPMINMAXINFO lpMMI = (LPMINMAXINFO) lParam;
	lpMMI->ptMaxPosition.x	= rcWork.left;
	lpMMI->ptMaxPosition.y	= rcWork.top;
	lpMMI->ptMaxSize.x		= rcWork.right;
	lpMMI->ptMaxSize.y		= rcWork.bottom;

	bHandled = FALSE;
	return 0;
}

LRESULT CWndFram::OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// 有时会在收到WM_NCDESTROY后收到wParam为SC_CLOSE的WM_SYSCOMMAND
	if( wParam == SC_CLOSE ) {
		//::PostQuitMessage(0L);
		Close();
		bHandled = TRUE;
		return 0;
	}
	BOOL bZoomed = ::IsZoomed(*this);
	LRESULT lRes = CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	if( ::IsZoomed(*this) != bZoomed ) {
		if( !bZoomed ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("maxbtn")));
			if( pControl ) pControl->SetVisible(false);
			pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("restorebtn")));
			if( pControl ) pControl->SetVisible(true);
		}
		else {
			CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("maxbtn")));
			if( pControl ) pControl->SetVisible(true);
			pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("restorebtn")));
			if( pControl ) pControl->SetVisible(false);
		}
	}
	return lRes;
}



}