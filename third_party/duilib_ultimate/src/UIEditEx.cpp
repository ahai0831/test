#include "stdafx.h"
#include "UIEditEx.h"

namespace DuiLib
{
	class CEditWndEx : public CWindowWnd
	{
	public:
		CEditWndEx();

		void Init(CEditUIEx* pOwner);
		RECT CalPos();

		LPCTSTR GetWindowClassName() const;
		LPCTSTR GetSuperClassName() const;
		void OnFinalMessage(HWND hWnd);

		LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnEditChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		void HandleKeyMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);   //处理键盘消息，转为快捷键消息
	protected:
		CEditUIEx* m_pOwner;
		HBRUSH m_hBkBrush;
		bool m_bInit;
	};


	CEditWndEx::CEditWndEx() : m_pOwner(NULL), m_hBkBrush(NULL), m_bInit(false)
	{
	}

	void CEditWndEx::Init(CEditUIEx* pOwner)
	{
		m_pOwner = pOwner;
		RECT rcPos = CalPos();
		UINT uStyle = 0;
		if (m_pOwner->GetManager()->IsSetUpdatelayered())
		{
			uStyle = WS_POPUP | ES_AUTOHSCROLL | WS_VISIBLE;
			RECT rcWnd={0};
			::GetWindowRect(m_pOwner->GetManager()->GetPaintWindow(), &rcWnd);
			rcPos.left += rcWnd.left;
			rcPos.right += rcWnd.left;
			rcPos.top += rcWnd.top;
			rcPos.bottom += rcWnd.top;
		}
		else
		{
			uStyle = WS_CHILD | ES_AUTOHSCROLL;
		}	
		if( m_pOwner->IsPasswordMode() ) uStyle |= ES_PASSWORD;
		Create(m_pOwner->GetManager()->GetPaintWindow(), NULL, uStyle, 0, rcPos);
		HFONT hFont=NULL;
		int iFontIndex=m_pOwner->GetFont();
		if (iFontIndex!=-1)
			hFont=m_pOwner->GetManager()->GetFont(iFontIndex);
		if (hFont==NULL)
			hFont=m_pOwner->GetManager()->GetDefaultFontInfo()->hFont;

		SetWindowFont(m_hWnd, hFont, TRUE);
		Edit_LimitText(m_hWnd, m_pOwner->GetMaxChar());
		if( m_pOwner->IsPasswordMode() ) Edit_SetPasswordChar(m_hWnd, m_pOwner->GetPasswordChar());
		Edit_SetText(m_hWnd, m_pOwner->GetText());
		Edit_SetModify(m_hWnd, FALSE);
		SendMessage(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(0, 0));
		Edit_Enable(m_hWnd, m_pOwner->IsEnabled() == true);
		Edit_SetReadOnly(m_hWnd, m_pOwner->IsReadOnly() == true);

		//Styls
		LONG styleValue = ::GetWindowLong(m_hWnd, GWL_STYLE);
		styleValue |= pOwner->GetWindowStyls();
		::SetWindowLong(GetHWND(), GWL_STYLE, styleValue);
		::ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);
		::SetFocus(m_hWnd);
		m_bInit = true;    
	}

	RECT CEditWndEx::CalPos()
	{
		CRect rcPos = m_pOwner->GetPos();
		RECT rcInset = m_pOwner->GetTextPadding();
		rcPos.left += rcInset.left;
		rcPos.top += rcInset.top;
		rcPos.right -= rcInset.right;
		rcPos.bottom -= rcInset.bottom;
		LONG lEditHeight = m_pOwner->GetManager()->GetFontInfo(m_pOwner->GetFont())->tm.tmHeight;
		if( lEditHeight < rcPos.GetHeight() ) {
			rcPos.top += (rcPos.GetHeight() - lEditHeight) / 2;
			rcPos.bottom = rcPos.top + lEditHeight;
		}
		return rcPos;
	}

	LPCTSTR CEditWndEx::GetWindowClassName() const
	{
		return _T("EditWnd");
	}

	LPCTSTR CEditWndEx::GetSuperClassName() const
	{
		return WC_EDIT;
	}

	void CEditWndEx::OnFinalMessage(HWND /*hWnd*/)
	{
		m_pOwner->Invalidate();
		// Clear reference and die
		if( m_hBkBrush != NULL ) ::DeleteObject(m_hBkBrush);
		m_pOwner->m_pWindow = NULL;
		delete this;
	}

	LRESULT CEditWndEx::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		LRESULT lRes = 0;
		BOOL bHandled = TRUE;
		if( uMsg == WM_KILLFOCUS ) lRes = OnKillFocus(uMsg, wParam, lParam, bHandled);
		else if( uMsg == OCM_COMMAND ) {
			if( GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE ) lRes = OnEditChanged(uMsg, wParam, lParam, bHandled);
			else if( GET_WM_COMMAND_CMD(wParam, lParam) == EN_UPDATE ) {
				RECT rcClient;
				::GetClientRect(m_hWnd, &rcClient);
				::InvalidateRect(m_hWnd, &rcClient, FALSE);
			}
		}
		else if( uMsg == WM_KEYDOWN && TCHAR(wParam) == VK_RETURN ){
			::SendMessage(m_pOwner->GetManager()->GetPaintWindow(),WM_HOTKEY, Accelerator_Enter, 0);
			//m_pOwner->GetManager()->SendNotify(m_pOwner, _T("return"));

		}
		else if( uMsg == OCM__BASE + WM_CTLCOLOREDIT  || uMsg == OCM__BASE + WM_CTLCOLORSTATIC ) {
			if( m_pOwner->GetNativeEditBkColor() == 0xFFFFFFFF ) return NULL;
			::SetBkMode((HDC)wParam, TRANSPARENT);

			DWORD dwTextColor;
			if (m_pOwner->GetNativeEditTextColor() != 0x000000)
				dwTextColor = m_pOwner->GetNativeEditTextColor();
			else
				dwTextColor = m_pOwner->GetTextColor();

			::SetTextColor((HDC)wParam, RGB(GetBValue(dwTextColor),GetGValue(dwTextColor),GetRValue(dwTextColor)));
			if( m_hBkBrush == NULL ) {
				DWORD clrColor = m_pOwner->GetNativeEditBkColor();
				m_hBkBrush = ::CreateSolidBrush(RGB(GetBValue(clrColor), GetGValue(clrColor), GetRValue(clrColor)));
			}
			return (LRESULT)m_hBkBrush;
		}
		else bHandled = FALSE;

		if( !bHandled ) return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
		return lRes;
	}

	LRESULT CEditWndEx::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LRESULT lRes = ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
		if (m_pOwner)
		{
			m_pOwner->GetManager()->SendNotify(m_pOwner, _T("killfocus"));
		}
		PostMessage(WM_CLOSE);
		return lRes;
	}

	LRESULT CEditWndEx::OnEditChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		if( !m_bInit ) return 0;
		if( m_pOwner == NULL ) return 0;
		// Copy text back
		int cchLen = ::GetWindowTextLength(m_hWnd) + 1;
		LPTSTR pstr = static_cast<LPTSTR>(_alloca(cchLen * sizeof(TCHAR)));
		ASSERT(pstr);
		if( pstr == NULL ) return 0;
		::GetWindowText(m_hWnd, pstr, cchLen);
		m_pOwner->m_sText = pstr;
		m_pOwner->GetManager()->SendNotify(m_pOwner, _T("textchanged"));
		return 0;
	}

	void CEditWndEx::HandleKeyMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if ((hwnd != NULL) && ::IsWindow(hwnd))
		{
			if (uMsg == WM_KEYDOWN)
			{
				switch (wParam)
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


	/////////////////////////////////////////////////////////////////////////////////////
	//
	//

	CEditUIEx::CEditUIEx() : m_pWindow(NULL), m_uMaxChar(255), m_bReadOnly(false), 
		m_bPasswordMode(false), m_cPasswordChar(_T('*')), m_uButtonState(0), 
		m_dwEditbkColor(0xFFFFFFFF), m_dwEditTextColor(0x00000000), m_iWindowStyls(0),m_dwTipValueColor(0xFFBAC0C5)
	{
		SetTextPadding(CRect(4, 3, 4, 3));
		SetBkColor(0xFFFFFFFF);
	}

	LPCTSTR CEditUIEx::GetClass() const
	{
		return _T("EditUIEx");
	}

	LPVOID CEditUIEx::GetInterface(LPCTSTR pstrName)
	{
		if( _tcscmp(pstrName, _T("EditEx")) == 0 ) return static_cast<CEditUIEx*>(this);
		return CLabelUI::GetInterface(pstrName);
	}

	UINT CEditUIEx::GetControlFlags() const
	{
		if( !IsEnabled() ) return CControlUI::GetControlFlags();

		return UIFLAG_SETCURSOR | UIFLAG_TABSTOP;
	}

	void CEditUIEx::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( m_pParent != NULL ) m_pParent->DoEvent(event);
			else CLabelUI::DoEvent(event);
			return;
		}

		if( event.Type == UIEVENT_SETCURSOR && IsEnabled() )
		{
			::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_IBEAM)));
			return;
		}
		if( event.Type == UIEVENT_WINDOWSIZE )
		{
			if( m_pWindow != NULL ) m_pManager->SetFocusNeeded(this);
		}
		if( event.Type == UIEVENT_SCROLLWHEEL )
		{
			if( m_pWindow != NULL ) return;
		}
		if( event.Type == UIEVENT_SETFOCUS && IsEnabled() ) 
		{
			if( m_pWindow ) return;
			m_pWindow = new CEditWndEx();
			ASSERT(m_pWindow);
			m_pWindow->Init(this);
			Invalidate();
		}
		if( event.Type == UIEVENT_KILLFOCUS && IsEnabled() ) 
		{
			Invalidate();
		}
		if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK || event.Type == UIEVENT_RBUTTONDOWN) 
		{
			if( IsEnabled() ) {
				GetManager()->ReleaseCapture();
				if( IsFocused() && m_pWindow == NULL )
				{
					m_pWindow = new CEditWndEx();
					ASSERT(m_pWindow);
					m_pWindow->Init(this);

					if( PtInRect(&m_rcItem, event.ptMouse) )
					{
						int nSize = GetWindowTextLength(*m_pWindow);
						if( nSize == 0 )
							nSize = 1;

						Edit_SetSel(*m_pWindow, 0, nSize);
					}
				}
				else if( m_pWindow != NULL )
				{
#if 1
					int nSize = GetWindowTextLength(*m_pWindow);
					if( nSize == 0 )
						nSize = 1;

					Edit_SetSel(*m_pWindow, 0, nSize);
#else
					POINT pt = event.ptMouse;
					pt.x -= m_rcItem.left + m_rcTextPadding.left;
					pt.y -= m_rcItem.top + m_rcTextPadding.top;
					::SendMessage(*m_pWindow, WM_LBUTTONDOWN, event.wParam, MAKELPARAM(pt.x, pt.y));
#endif
				}
			}
			return;
		}
		if( event.Type == UIEVENT_MOUSEMOVE ) 
		{
			return;
		}
		if( event.Type == UIEVENT_BUTTONUP ) 
		{
			return;
		}
		if( event.Type == UIEVENT_CONTEXTMENU )
		{
			return;
		}
		if( event.Type == UIEVENT_MOUSEENTER )
		{
			if( IsEnabled() ) {
				m_uButtonState |= UISTATE_HOT;
				Invalidate();
			}
			return;
		}
		if( event.Type == UIEVENT_MOUSELEAVE )
		{
			if( IsEnabled() ) {
				m_uButtonState &= ~UISTATE_HOT;
				Invalidate();
			}
			return;
		}
		CLabelUI::DoEvent(event);
	}

	void CEditUIEx::SetEnabled(bool bEnable)
	{
		CControlUI::SetEnabled(bEnable);
		if( !IsEnabled() ) {
			m_uButtonState = 0;
		}
	}

	void CEditUIEx::SetText(LPCTSTR pstrText)
	{
		m_sText = pstrText;
		if( m_pWindow != NULL ) Edit_SetText(*m_pWindow, m_sText);
		Invalidate();
	}

	void CEditUIEx::SetMaxChar(UINT uMax)
	{
		m_uMaxChar = uMax;
		if( m_pWindow != NULL ) Edit_LimitText(*m_pWindow, m_uMaxChar);
	}

	UINT CEditUIEx::GetMaxChar()
	{
		return m_uMaxChar;
	}

	void CEditUIEx::SetReadOnly(bool bReadOnly)
	{
		if( m_bReadOnly == bReadOnly ) return;

		m_bReadOnly = bReadOnly;
		if( m_pWindow != NULL ) Edit_SetReadOnly(*m_pWindow, m_bReadOnly);
		Invalidate();
	}

	bool CEditUIEx::IsReadOnly() const
	{
		return m_bReadOnly;
	}

	void CEditUIEx::SetNumberOnly(bool bNumberOnly)
	{
		if( bNumberOnly )
		{
			m_iWindowStyls |= ES_NUMBER;
		}
		else
		{
			m_iWindowStyls &= ~ES_NUMBER;
		}
	}

	bool CEditUIEx::IsNumberOnly() const
	{
		return m_iWindowStyls&ES_NUMBER ? true:false;
	}

	int CEditUIEx::GetWindowStyls() const 
	{
		return m_iWindowStyls;
	}

	void CEditUIEx::SetPasswordMode(bool bPasswordMode)
	{
		if( m_bPasswordMode == bPasswordMode ) return;
		m_bPasswordMode = bPasswordMode;
		Invalidate();
		if( m_pWindow != NULL ) 
		{
			LONG styleValue = ::GetWindowLong(*m_pWindow, GWL_STYLE);
			bPasswordMode ? styleValue |= ES_PASSWORD : styleValue &= ~ES_PASSWORD;
			::SetWindowLong(*m_pWindow, GWL_STYLE, styleValue);
		}
	}

	bool CEditUIEx::IsPasswordMode() const
	{
		return m_bPasswordMode;
	}

	void CEditUIEx::SetPasswordChar(TCHAR cPasswordChar)
	{
		if( m_cPasswordChar == cPasswordChar ) return;
		m_cPasswordChar = cPasswordChar;
		if( m_pWindow != NULL ) Edit_SetPasswordChar(*m_pWindow, m_cPasswordChar);
		Invalidate();
	}

	TCHAR CEditUIEx::GetPasswordChar() const
	{
		return m_cPasswordChar;
	}

	LPCTSTR CEditUIEx::GetNormalImage()
	{
		return m_sNormalImage;
	}

	void CEditUIEx::SetNormalImage(LPCTSTR pStrImage)
	{
		m_sNormalImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CEditUIEx::GetHotImage()
	{
		return m_sHotImage;
	}

	void CEditUIEx::SetHotImage(LPCTSTR pStrImage)
	{
		m_sHotImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CEditUIEx::GetFocusedImage()
	{
		return m_sFocusedImage;
	}

	void CEditUIEx::SetFocusedImage(LPCTSTR pStrImage)
	{
		m_sFocusedImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CEditUIEx::GetDisabledImage()
	{
		return m_sDisabledImage;
	}

	void CEditUIEx::SetDisabledImage(LPCTSTR pStrImage)
	{
		m_sDisabledImage = pStrImage;
		Invalidate();
	}

	void CEditUIEx::SetNativeEditBkColor(DWORD dwBkColor)
	{
		m_dwEditbkColor = dwBkColor;
	}

	DWORD CEditUIEx::GetNativeEditBkColor() const
	{
		return m_dwEditbkColor;
	}

	void CEditUIEx::SetNativeEditTextColor( LPCTSTR pStrColor )
	{
		if( *pStrColor == _T('#')) pStrColor = ::CharNext(pStrColor);
		LPTSTR pstr = NULL;
		DWORD clrColor = _tcstoul(pStrColor, &pstr, 16);

		m_dwEditTextColor = clrColor;
	}

	DWORD CEditUIEx::GetNativeEditTextColor() const
	{
		return m_dwEditTextColor;
	}

	void CEditUIEx::SetSel(long nStartChar, long nEndChar)
	{
		if( m_pWindow != NULL ) Edit_SetSel(*m_pWindow, nStartChar,nEndChar);
	}

	void CEditUIEx::SetSelAll()
	{
		SetSel(0,-1);
	}

	void CEditUIEx::SetReplaceSel(LPCTSTR lpszReplace)
	{
		if( m_pWindow != NULL ) Edit_ReplaceSel(*m_pWindow, lpszReplace);
	}

	void CEditUIEx::SetTipValue( LPCTSTR pStrTipValue )
	{
		m_sTipValue	= pStrTipValue;
	}

	LPCTSTR CEditUIEx::GetTipValue()
	{
		return m_sTipValue.GetData();
	}

	void CEditUIEx::SetTipValueColor( LPCTSTR pStrColor )
	{
		if( *pStrColor == _T('#')) pStrColor = ::CharNext(pStrColor);
		LPTSTR pstr = NULL;
		DWORD clrColor = _tcstoul(pStrColor, &pstr, 16);

		m_dwTipValueColor = clrColor;
	}

	DWORD CEditUIEx::GetTipValueColor()
	{
		return m_dwTipValueColor;
	}

	void CEditUIEx::SetPos(RECT rc, bool bNeedInvalidate)
	{
		CControlUI::SetPos(rc);
		if( m_pWindow != NULL ) {
			RECT rcPos = m_pWindow->CalPos();
			::SetWindowPos(m_pWindow->GetHWND(), NULL, rcPos.left, rcPos.top, rcPos.right - rcPos.left, 
				rcPos.bottom - rcPos.top, SWP_NOZORDER | SWP_NOACTIVATE);        
		}
	}

	void CEditUIEx::SetVisible(bool bVisible)
	{
		CControlUI::SetVisible(bVisible);
		if( !IsVisible() && m_pWindow != NULL ) m_pManager->SetFocus(NULL);
	}

	void CEditUIEx::SetInternVisible(bool bVisible)
	{
		if( !IsVisible() && m_pWindow != NULL ) m_pManager->SetFocus(NULL);
	}

	SIZE CEditUIEx::EstimateSize(SIZE szAvailable)
	{
		if (m_cxyFixed.cy == 0) return CSize(m_cxyFixed.cx, m_pManager->GetFontInfo(GetFont())->tm.tmHeight + 6);
		return CControlUI::EstimateSize(szAvailable);
	}

	void CEditUIEx::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcscmp(pstrName, _T("readonly")) == 0 ) SetReadOnly(_tcscmp(pstrValue, _T("true")) == 0);
		else if( _tcscmp(pstrName, _T("numberonly")) == 0 ) SetNumberOnly(_tcscmp(pstrValue, _T("true")) == 0);
		else if( _tcscmp(pstrName, _T("password")) == 0 ) SetPasswordMode(_tcscmp(pstrValue, _T("true")) == 0);
		else if( _tcscmp(pstrName, _T("passwordchar")) == 0 ) SetPasswordChar(*pstrValue);
		else if( _tcscmp(pstrName, _T("maxchar")) == 0 ) SetMaxChar(_ttoi(pstrValue));
		else if( _tcscmp(pstrName, _T("normalimage")) == 0 ) SetNormalImage(pstrValue);
		else if( _tcscmp(pstrName, _T("hotimage")) == 0 ) SetHotImage(pstrValue);
		else if( _tcscmp(pstrName, _T("focusedimage")) == 0 ) SetFocusedImage(pstrValue);
		else if( _tcscmp(pstrName, _T("disabledimage")) == 0 ) SetDisabledImage(pstrValue);
		else if( _tcscmp(pstrName, _T("tipvalue")) == 0 ) SetTipValue(pstrValue);
		else if( _tcscmp(pstrName, _T("tipvaluecolor")) == 0 ) SetTipValueColor(pstrValue);
		else if( _tcscmp(pstrName, _T("nativetextcolor")) == 0 ) SetNativeEditTextColor(pstrValue);
		else if( _tcscmp(pstrName, _T("nativebkcolor")) == 0 ) {
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetNativeEditBkColor(clrColor);
		}
		else CLabelUI::SetAttribute(pstrName, pstrValue);
	}

	void CEditUIEx::PaintStatusImage(HDC hDC)
	{
		if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
		else m_uButtonState &= ~ UISTATE_FOCUSED;
		if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
		else m_uButtonState &= ~ UISTATE_DISABLED;

		if( (m_uButtonState & UISTATE_DISABLED) != 0 ) {
			if( !m_sDisabledImage.IsEmpty() ) {
				if( !DrawImage(hDC, (LPCTSTR)m_sDisabledImage) ) m_sDisabledImage.Empty();
				else return;
			}
		}
		else if( (m_uButtonState & UISTATE_FOCUSED) != 0 ) {
			if( !m_sFocusedImage.IsEmpty() ) {
				if( !DrawImage(hDC, (LPCTSTR)m_sFocusedImage) ) m_sFocusedImage.Empty();
				else return;
			}
		}
		else if( (m_uButtonState & UISTATE_HOT) != 0 ) {
			if( !m_sHotImage.IsEmpty() ) {
				if( !DrawImage(hDC, (LPCTSTR)m_sHotImage) ) m_sHotImage.Empty();
				else return;
			}
		}

		if( !m_sNormalImage.IsEmpty() ) {
			if( !DrawImage(hDC, (LPCTSTR)m_sNormalImage) ) m_sNormalImage.Empty();
			else return;
		}
	}

	void CEditUIEx::PaintText(HDC hDC)
	{
		DWORD mCurTextColor = m_dwTextColor;

		if( m_dwTextColor == 0 ) mCurTextColor = m_dwTextColor = m_pManager->GetDefaultFontColor();		
		if( m_dwDisabledTextColor == 0 ) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

		CStdString sText;
		if(GetText() == m_sTipValue || GetText() == _T(""))	
		{
			mCurTextColor = m_dwTipValueColor;
			sText = m_sTipValue;			
		}
		else
		{
			sText = m_sText;

			if( m_bPasswordMode ) {
				sText.Empty();
				LPCTSTR p = m_sText.GetData();
				while( *p != _T('\0') ) {
					sText += m_cPasswordChar;
					p = ::CharNext(p);
				}
			}
		}

		RECT rc = m_rcItem;
		rc.left += m_rcTextPadding.left;
		rc.right -= m_rcTextPadding.right;
		rc.top += m_rcTextPadding.top;
		rc.bottom -= m_rcTextPadding.bottom;
		if( IsEnabled() ) {
			CRenderEngine::DrawText(hDC, m_pManager, rc, sText, mCurTextColor, \
				m_iFont, DT_SINGLELINE | m_uTextStyle);
		}
		else {
			CRenderEngine::DrawText(hDC, m_pManager, rc, sText, m_dwDisabledTextColor, \
				m_iFont, DT_SINGLELINE | m_uTextStyle);
		}
	}
}
