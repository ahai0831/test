#include "StdAfx.h"
#include "EditComboUI.h"

namespace DuiLib {

class CComboEditWnd : public CWindowWnd
{
public:
	CComboEditWnd(bool bInitShow = false);

    void Init(CEditComboUI* pOwner);
    RECT CalPos();
	void SetShowAlways(bool bShow);
	bool GetShowType();

    LPCTSTR GetWindowClassName() const;
    LPCTSTR GetSuperClassName() const;
    void OnFinalMessage(HWND hWnd);

    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnEditChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

protected:
    CEditComboUI* m_pOwner;
    HBRUSH m_hBkBrush;
	bool m_bMouseTrack = true;
	bool m_bInitShow = false;   // 初始化显示时，不被失去焦点删除
};


CComboEditWnd::CComboEditWnd(bool bInitShow /*= false*/) : m_pOwner(NULL), m_hBkBrush(NULL)
{
	m_bInitShow = bInitShow;
}

void CComboEditWnd::Init(CEditComboUI* pOwner)
{
	if (pOwner == NULL)
	{
		return;
	}
    m_pOwner = pOwner;
    RECT rcPos = CalPos();
    UINT uStyle = WS_CHILD | ES_AUTOHSCROLL;
    if( m_pOwner->IsPasswordMode() ) uStyle |= ES_PASSWORD;
    Create(m_pOwner->GetManager()->GetPaintWindow(), NULL, uStyle, 0, rcPos);

    SetWindowFont(m_hWnd, m_pOwner->GetManager()->GetFontInfo(m_pOwner->GetFont())->hFont, TRUE);
    Edit_LimitText(m_hWnd, m_pOwner->GetMaxChar());
    if( m_pOwner->IsPasswordMode() )
		Edit_SetPasswordChar(m_hWnd, m_pOwner->GetPasswordChar());

    Edit_SetText(m_hWnd, m_pOwner->GetText());
    Edit_SetModify(m_hWnd, FALSE);
    SendMessage(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(0, 0));
    Edit_Enable(m_hWnd, m_pOwner->IsEnabled() == true);
    Edit_SetReadOnly(m_hWnd, m_pOwner->IsReadOnly() == true);
    ::ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);
    ::SetFocus(m_hWnd);
	Edit_SetSel(m_hWnd, 0, -1);
}

RECT CComboEditWnd::CalPos()   //计算位置
{
    CRect rcPos = m_pOwner->GetEditRect();

	RECT rcInset = m_pOwner->GetTextPadding();
    rcPos.left += rcInset.left;
    rcPos.top += rcInset.top;
    rcPos.right -= rcInset.right;
    rcPos.bottom -= rcInset.bottom;
    LONG lEditHeight =m_pOwner->GetManager()->GetFontInfo(m_pOwner->GetFont())->tm.tmHeight;
    if( lEditHeight < rcPos.GetHeight() ) {
        rcPos.top += (rcPos.GetHeight() - lEditHeight) / 2;
        rcPos.bottom = rcPos.top + lEditHeight;
    }
    return rcPos;
}

void CComboEditWnd::SetShowAlways(bool bShow)
{
	m_bInitShow = bShow;
}

bool CComboEditWnd::GetShowType()
{
	return m_bInitShow;
}

LPCTSTR CComboEditWnd::GetWindowClassName() const
{
    return _T("ComboEditWnd");
}

LPCTSTR CComboEditWnd::GetSuperClassName() const
{
    return WC_EDIT;
}

void CComboEditWnd::OnFinalMessage(HWND /*hWnd*/)
{
    // Clear reference and die
    if( m_hBkBrush != NULL ) ::DeleteObject(m_hBkBrush);
	m_pOwner->m_pEditWindow = NULL;
    delete this;
}

LRESULT CComboEditWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lRes = 0;
    BOOL bHandled = TRUE;
    if( uMsg == WM_KILLFOCUS )
	{
		if (!m_bInitShow)
		{
			lRes = OnKillFocus(uMsg, wParam, lParam, bHandled);
		}
	}
    else if( uMsg == OCM_COMMAND ) {
        if( GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE ) lRes = OnEditChanged(uMsg, wParam, lParam, bHandled);
        else if( GET_WM_COMMAND_CMD(wParam, lParam) == EN_UPDATE ) {
            RECT rcClient;
            ::GetClientRect(m_hWnd, &rcClient);
            ::InvalidateRect(m_hWnd, &rcClient, FALSE); 
        }
    }
    else if( uMsg == WM_KEYDOWN) {
		if (TCHAR(wParam) == VK_RETURN && m_pOwner)
		{
			m_pOwner->GetManager()->RemovePostPaint(m_pOwner);
			m_pOwner->SetKeyState(true);
			m_pOwner->Invalidate();
// 			::SendMessage(m_pOwner->GetManager()->GetPaintWindow(), WM_PAINT, 0, 0);
			m_pOwner->GetManager()->SendNotify(m_pOwner, _T("return"), DUILIB_EDIT_VK_RETURN);
		}
		else
			::SendMessage(m_pOwner->GetManager()->GetPaintWindow(), uMsg, wParam, lParam);
		//        m_pOwner->GetManager()->SendNotify(m_pOwner, _T("return"), DUILIB_EDIT_VK_RETURN);
   }
    else if( uMsg == OCM__BASE + WM_CTLCOLOREDIT  || uMsg == OCM__BASE + WM_CTLCOLORSTATIC ) {
		DWORD dwTextColor = m_pOwner->GetTextColor();
		::SetTextColor((HDC)wParam, RGB(GetBValue(dwTextColor),GetGValue(dwTextColor),GetRValue(dwTextColor)));
        if( m_pOwner->GetNativeEditBkColor() == 0xFFFFFFFF ) return NULL;
        ::SetBkMode((HDC)wParam, TRANSPARENT);
        if( m_hBkBrush == NULL ) {
            DWORD clrColor = m_pOwner->GetBkColor();
            m_hBkBrush = ::CreateSolidBrush(RGB(GetBValue(clrColor), GetGValue(clrColor), GetRValue(clrColor)));
        }
        return (LRESULT)m_hBkBrush;
    }
	else if(uMsg == WM_CHAR)
	{
		if(m_pOwner && m_pOwner->IsNumberOnly())
		{
			DWORD dNum = wParam;
			if( (dNum >= 48 && dNum <= 57) || VK_BACK == dNum )
			{
				bHandled = FALSE;
			}

			if (GetKeyState(VK_LCONTROL) < 0)
			{
				bHandled = FALSE;
			}
		}
		else
		{
			bHandled = FALSE;
		}
	}
	else if (uMsg == WM_MOUSEMOVE)
	{
		if (m_bMouseTrack)     // 若允许 追踪，则。 
		{
			TRACKMOUSEEVENT csTME;
			csTME.cbSize = sizeof(csTME);
			csTME.dwFlags = TME_LEAVE | TME_HOVER;
			csTME.hwndTrack = m_hWnd;// 指定要 追踪 的窗口 
			csTME.dwHoverTime = 10;  // 鼠标在按钮上停留超过 10ms ，才认为状态为 HOVER
			::_TrackMouseEvent(&csTME); // 开启 Windows 的 WM_MOUSELEAVE ， WM_MOUSEHOVER 事件支持


			m_bMouseTrack = false;   // 若已经 追踪 ，则停止 追踪 
		}
		else
			bHandled = FALSE;
	}
	else if (uMsg == WM_MOUSELEAVE)
	{
		m_bMouseTrack = true;
		if (m_pOwner)
		{
			m_pOwner->m_bMouseInEdit = false;
			m_pOwner->Invalidate();
		}
	}
	else if (uMsg == WM_MOUSEHOVER)
	{
		if (m_pOwner)
		{
			m_pOwner->m_bMouseInEdit = true;
			m_pOwner->Invalidate();
		}
	}
    else bHandled = FALSE;
    if( !bHandled ) return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
    return lRes;
}

LRESULT CComboEditWnd::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LRESULT lRes = ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
	PostMessage(WM_CLOSE);
    return lRes;
}

LRESULT CComboEditWnd::OnEditChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    if( m_pOwner == NULL ) return 0;
    // Copy text back
    int cchLen = ::GetWindowTextLength(m_hWnd) + 1;
    LPTSTR pstr = static_cast<LPTSTR>(_alloca(cchLen * sizeof(TCHAR)));
    ASSERT(pstr);
    if( pstr == NULL ) return 0;
    ::GetWindowText(m_hWnd, pstr, cchLen);

	if(m_pOwner->IsNumberOnly())
	{
		//处理粘贴数据过滤掉，只处理数字
		TCHAR szTemp[MAX_PATH] = {0};
		if(cchLen < MAX_PATH)
		{
			memcpy(szTemp,pstr,cchLen * sizeof(TCHAR));
			int Len = wcslen(szTemp);
			bool bNumberOnly = true;

			for(int i = 0 ; i < Len; i++)
			{
				if( szTemp[i] < '0' || szTemp[i] > '9')
				{
					bNumberOnly = false;
					break;
				}
			}
			if(bNumberOnly)
			{
				int nNum = _wtol(szTemp);
				m_pOwner->m_sText = pstr;
				if(!m_pOwner->m_sText.IsEmpty() &&  0 == nNum)
				{
					m_pOwner->m_sText.Empty();
					WORD dEnd = -1;
					::SetWindowText(m_hWnd,m_pOwner->m_sText);
					Edit_SetSel(GetHWND(),(WPARAM)dEnd,(LPARAM)dEnd);
				}
				m_pOwner->GetManager()->SendNotify(m_pOwner, _T("textchanged"), DUILIB_EDIT_TEXTCHANGED);
			}
			else
			{
				WORD dEnd = -1;
				::SetWindowText(m_hWnd,m_pOwner->m_sText);
				Edit_SetSel(GetHWND(),(WPARAM)dEnd,(LPARAM)dEnd);
			}
		}
	}
	else
	{
		m_pOwner->m_sText = pstr;
		m_pOwner->GetManager()->SendNotify(m_pOwner, _T("textchanged"), DUILIB_EDIT_TEXTCHANGED);
	}
    return 0;
}

//////////////////////////////////////////////////////////////////////////
// CEditComboUI
CEditComboUI::CEditComboUI() :CComboUI()
{
	m_pEditWindow = NULL;
	m_uMaxChar = 255;
	m_bReadOnly = false;
	m_bPasswordMode = false;
	m_bNumberOnly = false;
	m_cPasswordChar = _T('*');
	m_uButtonState = 0;
	m_dwEditbkColor = 0xFFFFFFFF;
	SetTextPadding(CRect(4, 3, 4, 3));
	SetBkColor(0xFFFFFFFF);
	m_editOffSet = RECT{ 0, 0, 0, 0 };

	m_szBtn.cx = m_szBtn.cy = 0;
	m_rcBtnOffset.left = m_rcBtnOffset.right = m_rcBtnOffset.top = m_rcBtnOffset.bottom = 0;
	m_nLastSel = -1;
	m_bShowIcon = false;
	m_rcBtnPos = { 0 };
	m_bDelayHover = false;
	m_uTextStyle = DT_VCENTER;
	m_dwTextColor = 1;
	m_dwDisabledTextColor = 1;
	m_iFont = -1;
	m_bShowHtml = false;
	m_rcEdit = RECT{ 0, 0, 0, 0 };
}

CEditComboUI::~CEditComboUI()
{
}

LPCTSTR CEditComboUI::GetClass() const
{
	return _T("EditComboUI");
}

LPVOID CEditComboUI::GetInterface(LPCTSTR pstrName)
{
	if( _tcscmp(pstrName, _T("EditComboUI")) == 0 ) return static_cast<CEditComboUI*>(this);
	return CComboUI::GetInterface(pstrName);
}

bool CEditComboUI::ClickItem(int iIndex, bool bTakeFocus /* = false */)
{
	if (m_iCurSel == iIndex)
	{
		if (m_pManager != NULL) m_pManager->SendNotify(this, _T("itemselect"), DUILIB_COMBO_ITMESELECT, m_iCurSel, 0);
	}

	m_nLastSel = iIndex;
	
	// 赋值Text
	if (m_items.GetSize() == 0)
	{
		return false;
	}
	bool bret = SelectItem(iIndex, bTakeFocus);
	CComboElementUI* pControl = static_cast<CComboElementUI*>(m_items[iIndex]);
	if (pControl)
	{
		CStdString strText = pControl->GetText();
		SetText(strText);
	}

	if (m_bShowIcon == false)
	{
		return bret;
	}
	for (int it = 0; it < m_items.GetSize(); it++)
	{
		CComboElementUI* pControl = static_cast<CComboElementUI*>(m_items[it]);
		if (pControl == NULL)
		{
			continue;
		}
		if (iIndex == it)
		{
			pControl->SetShowIcon(true);
		}
		else
			pControl->SetShowIcon(false);
	}
	return bret;
}

bool CEditComboUI::CheckItem(int iIndex, bool bTakeFocus /* = false */)
{
	if (bTakeFocus == true)
	{
		if (m_nLastSel == -1)
		{
			m_nLastSel = m_iCurSel;
		}
		if (iIndex == m_iCurSel) return true;
		int iOldSel = m_iCurSel;
		if (m_iCurSel >= 0) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[m_iCurSel]);
			if (!pControl) return false;
			IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
			if (pListItem != NULL) pListItem->Select(false);
		}
		if (iIndex < 0) return false;
		if (m_items.GetSize() == 0) return false;
		if (iIndex >= m_items.GetSize()) iIndex = m_items.GetSize() - 1;
		CControlUI* pControl = static_cast<CControlUI*>(m_items[iIndex]);
		if (!pControl || !pControl->IsVisible() || !pControl->IsEnabled()) return false;
		IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
		if (pListItem == NULL) return false;
		m_iCurSel = iIndex;
		if (m_pWindow != NULL) pControl->SetFocus();
		pListItem->Select(true);
		Invalidate();
		return true;
	}
	return false;
}

bool CEditComboUI::RemoveItem(int iIndex)
{
	if (m_pWindow != NULL) m_pWindow->Close();
	if (m_pManager != NULL) m_pManager->SendNotify(this, _T("itemdelete"), DUILIB_COMBO_ITMEDELETE, iIndex, 0);
	//RemoveAt(iIndex);
	return true;
}

UINT CEditComboUI::GetControlFlags() const
{
	return UIFLAG_TABSTOP | UIFLAG_SETCURSOR;
}

void CEditComboUI::SetText(LPCTSTR pstrText)
{
	m_sText = pstrText;
	if (m_pEditWindow != NULL)
	{
		Edit_SetText(*m_pEditWindow, m_sText);
	}
	else
	{
		if (m_pManager != NULL) 
			m_pManager->SendNotify(this, _T("textchanged"), DUILIB_EDIT_TEXTCHANGED);
	}
	Invalidate();
}

CStdString CEditComboUI::GetText() const
{
	return m_sText;
}

CStdString CEditComboUI::GetSelText() const
{
	return CComboUI::GetText();
}

void CEditComboUI::SetBtnNormalImage(LPCTSTR lpstr)
{
	m_sBtnNormalImage = lpstr;
}

LPCTSTR CEditComboUI::GetBtnNormalImage()
{
	return m_sBtnNormalImage;
}

void CEditComboUI::SetBtnHotImage(LPCTSTR lpstr)
{
	m_sBtnHotImage = lpstr;
}

LPCTSTR CEditComboUI::GetBtnHotImage()
{
	return m_sBtnHotImage;
}

void CEditComboUI::SetBtnPushedImage(LPCTSTR lpstr)
{
	m_sBtnPushedImage = lpstr;
}

LPCTSTR CEditComboUI::GetBtnPushedImage()
{
	return m_sBtnPushedImage;
}

void CEditComboUI::SetBtnOffset(RECT rc)
{
	m_rcBtnOffset = rc;
}

RECT CEditComboUI::GetBtnOffset()
{
	return m_rcBtnOffset;
}

//显示
void CEditComboUI::SetIconOffset(RECT rc)   //设置Icon偏移量，右边的设置右下角偏移量
{
	m_rcIconOffset = rc;
}

RECT CEditComboUI::GetIconOffset()
{
	return m_rcIconOffset;
}

SIZE CEditComboUI::GetIconSize()      //按钮大小
{
	return m_szIcon;
}

void CEditComboUI::SetIconImage(LPCTSTR lpstr)
{
	m_sIconImage = lpstr;
}

LPCTSTR CEditComboUI::GetIconImage()
{
	return m_sIconImage;
}

void CEditComboUI::SetDescIconImage(LPCTSTR lpstr)
{
	m_sDescIconImage = lpstr;
}

LPCTSTR CEditComboUI::GetDescIconImage()
{
	return m_sDescIconImage;
}

SIZE CEditComboUI::GetBtnSize()
{
	return m_szBtn;
}

void CEditComboUI::SetTextStyle(UINT uStyle)
{
	m_uTextStyle = uStyle;
	Invalidate();
}

UINT CEditComboUI::GetTextStyle() const
{
	return m_uTextStyle;
}

void CEditComboUI::SetTextColor(DWORD dwTextColor)
{
	m_dwTextColor = dwTextColor;
}

DWORD CEditComboUI::GetTextColor() const
{
	return m_dwTextColor;
}

void CEditComboUI::SetDisabledTextColor(DWORD dwTextColor)
{
	m_dwDisabledTextColor = dwTextColor;
}

DWORD CEditComboUI::GetDisabledTextColor() const
{
	return m_dwDisabledTextColor;
}

void CEditComboUI::SetFont(int index)
{
	m_iFont = index;
}

int CEditComboUI::GetFont() const
{
	return m_iFont;
}

RECT CEditComboUI::GetTextPadding() const
{
	return m_rcTextPadding;
}

void CEditComboUI::SetTextPadding(RECT rc)
{
	m_rcTextPadding = rc;
	Invalidate();
}

bool CEditComboUI::IsShowHtml()
{
	return m_bShowHtml;
}

void CEditComboUI::SetShowHtml(bool bShowHtml)
{
	if (m_bShowHtml == bShowHtml) return;

	m_bShowHtml = bShowHtml;
	Invalidate();
}

void CEditComboUI::SetItemShowIcon(int nIndex)
{
	if (nIndex < 0)
	{
		nIndex = 0;
	}
	if (nIndex >= m_items.GetSize())
	{
		nIndex = m_items.GetSize() - 1;
	}
	for (int it = 0; it < m_items.GetSize(); it++)
	{
		CComboElementUI* pControl = static_cast<CComboElementUI*>(m_items[it]);
		if (pControl == NULL)
		{
			continue;
		}
		if (nIndex == it)
		{
			pControl->SetShowIcon(true);
		}
		else
			pControl->SetShowIcon(false);
	}
}

void CEditComboUI::SetKeyState(bool bVkReturn)   // 设置状态，控制是否需要最后绘制
{
	m_bVK_Return = bVkReturn;
}

void CEditComboUI::SetMaxChar(UINT uMax)
{
	m_uMaxChar = uMax;
	if (m_pEditWindow != NULL) Edit_LimitText(*m_pEditWindow, m_uMaxChar);
}

UINT CEditComboUI::GetMaxChar()
{
	return m_uMaxChar;
}

void CEditComboUI::SetReadOnly(bool bReadOnly)
{
	if (m_bReadOnly == bReadOnly) return;

	m_bReadOnly = bReadOnly;
	if (m_pEditWindow != NULL) Edit_SetReadOnly(*m_pEditWindow, m_bReadOnly);
	Invalidate();
}

bool CEditComboUI::IsReadOnly() const
{
	return m_bReadOnly;
}

void CEditComboUI::SetPasswordMode(bool bPasswordMode)
{
	if (m_bPasswordMode == bPasswordMode) return;
	m_bPasswordMode = bPasswordMode;
	Invalidate();
}

bool CEditComboUI::IsPasswordMode() const
{
	return m_bPasswordMode;
}

void CEditComboUI::SetPasswordChar(TCHAR cPasswordChar)
{
	if (m_cPasswordChar == cPasswordChar) return;
	m_cPasswordChar = cPasswordChar;
	if (m_pEditWindow != NULL) Edit_SetPasswordChar(*m_pEditWindow, m_cPasswordChar);
	Invalidate();
}

TCHAR CEditComboUI::GetPasswordChar() const
{
	return m_cPasswordChar;
}

void CEditComboUI::SetNumberOnly(bool bNumberOnly)
{
	m_bNumberOnly = bNumberOnly;
}

bool CEditComboUI::IsNumberOnly() const
{
	return m_bNumberOnly;
}

void CEditComboUI::SetNativeEditBkColor(DWORD dwBkColor)
{
	m_dwEditbkColor = dwBkColor;
}

DWORD CEditComboUI::GetNativeEditBkColor() const
{
	return m_dwEditbkColor;
}

void CEditComboUI::SetEditOffSet(RECT rc)
{
	m_editOffSet = rc;
}

RECT CEditComboUI::GetEditRect() const
{
	return m_rcEdit;
}

void CEditComboUI::SetDefaultText(LPCTSTR pStrDefaultText)
{
	m_strDefaultText = pStrDefaultText;
}

LPCTSTR CEditComboUI::GetDefaultText()
{
	return m_strDefaultText;
}


void CEditComboUI::SetPos(RECT rc)
{
	CComboUI::SetPos(rc);
	m_rcEdit.left = m_rcItem.left + m_editOffSet.left;
	if (m_rcEdit.left > m_rcItem.right)
	{
		m_rcEdit.left = m_rcItem.right;
	}

	m_rcEdit.top = m_rcItem.top + m_editOffSet.top;
	m_rcEdit.right = m_rcItem.right - m_editOffSet.right;
	m_rcEdit.bottom = m_rcItem.bottom - m_editOffSet.bottom;
}

void CEditComboUI::DoEvent(TEventUI& event)
{
	if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND) {
		if (m_pParent != NULL) m_pParent->DoEvent(event);
		else CContainerUI::DoEvent(event);
		return;
	}
	if (PtInRect(&m_rcEdit, event.ptMouse))
	{
		if (event.Type == UIEVENT_SETCURSOR && IsEnabled())
		{
			::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_IBEAM)));
			return;
		}
		if (event.Type == UIEVENT_WINDOWSIZE)
		{
			if (m_pEditWindow != NULL) m_pManager->SetFocusNeeded(this);
		}
		if (event.Type == UIEVENT_SCROLLWHEEL)
		{
			if (m_pEditWindow != NULL) return;
		}
		if (event.Type == UIEVENT_SETFOCUS && IsEnabled())
		{
			Invalidate();
		}
		if (event.Type == UIEVENT_KILLFOCUS && IsEnabled())
		{
			Invalidate();
		}
		if (event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK || event.Type == UIEVENT_RBUTTONDOWN)
		{
			m_bMouseInEdit = true;
			if (IsEnabled()) {
				GetManager()->ReleaseCapture();
				if (IsFocused() && m_pEditWindow == NULL)
				{
					m_pEditWindow = new CComboEditWnd();
					ASSERT(m_pEditWindow);
					m_pEditWindow->Init(this);

					int nSize = GetWindowTextLength(*m_pEditWindow);
					if (nSize == 0)
						nSize = 1;

					Edit_SetSel(*m_pEditWindow, 0, nSize);
				}
				else if (m_pEditWindow != NULL)
				{
#if 1
					int nSize = GetWindowTextLength(*m_pEditWindow);
					if (nSize == 0)
						nSize = 1;

					Edit_SetSel(*m_pEditWindow, 0, nSize);
#else
					POINT pt = event.ptMouse;
					pt.x -= m_rcItem.left + m_rcTextPadding.left;
					pt.y -= m_rcItem.top + m_rcTextPadding.top;
					::SendMessage(*m_pEditWindow, WM_LBUTTONDOWN, event.wParam, MAKELPARAM(pt.x, pt.y));
#endif
				}
			}
			return;
		}
		if (event.Type == UIEVENT_MOUSEMOVE)
		{
			m_bMouseInEdit = true;
			Invalidate();
			return;
		}
	}
	else if (PtInRect(&m_rcItem, event.ptMouse))
	{
		if (event.Type == UIEVENT_SETFOCUS)
		{
			Invalidate();
		}
		if (event.Type == UIEVENT_KILLFOCUS)
		{
			Invalidate();
		}
		if (event.Type == UIEVENT_BUTTONDOWN)
		{
			m_bMouseInEdit = false;
			if (IsEnabled()) {
				Activate();
				m_uButtonState |= UISTATE_PUSHED | UISTATE_CAPTURED;
			}
			return;
		}
		if (event.Type == UIEVENT_BUTTONUP)
		{
			m_bMouseInEdit = false;
			if ((m_uButtonState & UISTATE_CAPTURED) != 0) {
				m_uButtonState &= ~UISTATE_CAPTURED;
				Invalidate();
			}
			return;
		}
		if (event.Type == UIEVENT_MOUSEMOVE)
		{
			CStdString str;
			str.Format(_T("mouse move x=%d, y=%d\n"), event.ptMouse.x, event.ptMouse.y);
			OutputDebugString(str);
			m_bMouseInEdit = false;
			Invalidate();
			return;
		}
		if (event.Type == UIEVENT_KEYDOWN)
		{
			switch (event.chKey) {
			case VK_F4:
			{
				Activate();
			}
			return;
			case VK_UP:
				SelectItem(FindSelectable(m_iCurSel - 1, false));
				return;
			case VK_DOWN:
				SelectItem(FindSelectable(m_iCurSel + 1, true));
				return;
			case VK_PRIOR:
				SelectItem(FindSelectable(m_iCurSel - 1, false));
				return;
			case VK_NEXT:
				SelectItem(FindSelectable(m_iCurSel + 1, true));
				return;
			case VK_HOME:
				SelectItem(FindSelectable(0, false));
				return;
			case VK_END:
				SelectItem(FindSelectable(GetCount() - 1, true));
				return;
			}
		}
		if (event.Type == UIEVENT_SCROLLWHEEL)
		{
			bool bDownward = LOWORD(event.wParam) == SB_LINEDOWN;
			//        SelectItem(FindSelectable(m_iCurSel + (bDownward ? 1 : -1), bDownward));
			if (bDownward)
				LineDown();
			else
				LineUp();
			return;
		}
		if (event.Type == UIEVENT_CONTEXTMENU)
		{
			return;
		}
	}
	if (event.Type == UIEVENT_KILLFOCUS)
	{
		if (m_pEditWindow && m_pEditWindow->GetShowType())
		{
			PostMessage(*m_pEditWindow, WM_CLOSE, 0, 0);
		}
		Invalidate();

	}
	if (event.Type == UIEVENT_MOUSEENTER)
	{
		if (m_bVK_Return)
		{
			return;
		}
		if (::PtInRect(&m_rcItem, event.ptMouse)) {
			if ((m_uButtonState & UISTATE_HOT) == 0)
			{
				if (m_bDelayHover && m_pManager)
				{
					m_pManager->AddPostPaint(this);
				}
				m_uButtonState |= UISTATE_HOT;
			}
			Invalidate();
		}
		return;
	}
	if (event.Type == UIEVENT_MOUSELEAVE)
	{
		if (m_bVK_Return)
		{
			return;
		}
		if ((m_uButtonState & UISTATE_HOT) != 0) {
			{
				if (m_bDelayHover && m_pManager && !m_pEditWindow && ((m_uButtonState & UISTATE_FOCUSED) == 0))
				{
					m_pManager->RemovePostPaint(this);
				}
				m_uButtonState &= ~UISTATE_HOT;
			}
			Invalidate();
		}
		return;
	}

	CControlUI::DoEvent(event);
//	CComboUI::DoEvent(event);
}

void CEditComboUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	CComboUI::DoPaint(hDC, rcPaint);
}

void CEditComboUI::DoPostPaint(HDC hDC, const RECT& rcPaint)
{
	if (m_bVK_Return)
	{
		return;
	}
	if (((m_uButtonState & UISTATE_HOT) != 0) || ((m_uButtonState & UISTATE_FOCUSED) != 0))
	{
//		CComboUI::DoPaint(hDC, rcPaint);
		if (!::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem)) return;
		// 绘制循序：背景颜色->背景图->状态图->文本->边框
		if (m_cxyBorderRound.cx > 0 || m_cxyBorderRound.cy > 0) {
			CRenderClip roundClip;
			CRenderClip::GenerateRoundClip(hDC, m_rcPaint, m_rcItem, m_cxyBorderRound.cx, m_cxyBorderRound.cy, roundClip);
			PaintStatusImage(hDC);
		}
		else {
			PaintStatusImage(hDC);
		}

	}
}

void CEditComboUI::PaintStatusImage(HDC hDC)
{
	if (IsFocused()) m_uButtonState |= UISTATE_FOCUSED;
	else m_uButtonState &= ~UISTATE_FOCUSED;
	if (!IsEnabled()) m_uButtonState |= UISTATE_DISABLED;
	else m_uButtonState &= ~UISTATE_DISABLED;

	if ((m_uButtonState & UISTATE_DISABLED) != 0) {
		if (!m_sDisabledImage.IsEmpty()) {
			if (!DrawImage(hDC, (LPCTSTR)m_sDisabledImage)) m_sDisabledImage.Empty();
			else return;
		}
	}
	else if ((m_uButtonState & UISTATE_PUSHED) != 0) {
		if (!m_sPushedImage.IsEmpty()) {
			if (!DrawImage(hDC, (LPCTSTR)m_sPushedImage)) m_sPushedImage.Empty();
			else return;
		}
	}
	else if ((m_uButtonState & UISTATE_HOT) != 0) {
		if (m_bMouseInEdit)
		{
			if (!m_sEditHotImage.IsEmpty()) {
				if (!DrawImage(hDC, (LPCTSTR)m_sEditHotImage)) m_sEditHotImage.Empty();
				else return;
			}
		}
		else
		{
			if (!m_sHotImage.IsEmpty()) {
				if (!DrawImage(hDC, (LPCTSTR)m_sHotImage)) m_sHotImage.Empty();
				else return;
			}
		}
	}
	else if ((m_uButtonState & UISTATE_FOCUSED) != 0) {
		if (m_bMouseInEdit)
		{
			if (!m_sEditFocusedImage.IsEmpty()) {
				if (!DrawImage(hDC, (LPCTSTR)m_sEditFocusedImage)) m_sEditFocusedImage.Empty();
				else return;
			}
		}
		else
		{
			if (!m_sFocusedImage.IsEmpty()) {
				if (!DrawImage(hDC, (LPCTSTR)m_sFocusedImage)) m_sFocusedImage.Empty();
				else return;
			}
		}
	}

	if (!m_sNormalImage.IsEmpty()) {
		if (!DrawImage(hDC, (LPCTSTR)m_sNormalImage)) m_sNormalImage.Empty();
		else return;
	}
}

// 设置Edit focus状态
void CEditComboUI::SetEditFocus()
{
	m_bMouseInEdit = true;
	if (IsEnabled()) {
		GetManager()->ReleaseCapture();
		if (m_pEditWindow == NULL)
		{
			m_pEditWindow = new CComboEditWnd(true);
			ASSERT(m_pEditWindow);
			m_pEditWindow->Init(this);

			int nSize = GetWindowTextLength(*m_pEditWindow);
			if (nSize == 0)
				nSize = 1;

			Edit_SetSel(*m_pEditWindow, 0, nSize);
		}
		else if (m_pEditWindow != NULL)
		{
			int nSize = GetWindowTextLength(*m_pEditWindow);
			if (nSize == 0)
				nSize = 1;

			Edit_SetSel(*m_pEditWindow, 0, nSize);
		}
		if (m_bDelayHover && m_pManager)
		{
			m_pManager->AddPostPaint(this);
		}
		m_uButtonState |= UISTATE_FOCUSED;
	}

}

LPCTSTR CEditComboUI::GetEditHotImage() const
{
	return m_sEditHotImage;
}

void CEditComboUI::SetEditHotImage(LPCTSTR pStrImage)
{
	m_sEditHotImage = pStrImage;
	Invalidate();
}

LPCTSTR CEditComboUI::GetEditFocusedImage() const
{
	return m_sEditFocusedImage;
}

void CEditComboUI::SetEditFocusedImage(LPCTSTR pStrImage)
{
	m_sEditFocusedImage = pStrImage;
	Invalidate();
}


void CEditComboUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcscmp(pstrName, _T("iconoffset")) == 0)
	{
		RECT rcBtnPos = { 0 };
		LPTSTR pstr = NULL;
		rcBtnPos.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
		rcBtnPos.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
		rcBtnPos.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
		rcBtnPos.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
		SetIconOffset(rcBtnPos);
	}
	else if (_tcscmp(pstrName, _T("iconsize")) == 0)
	{
		LPTSTR pstr = NULL;
		m_szIcon.cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
		m_szIcon.cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
	}
	else if (_tcscmp(pstrName, _T("iconimage")) == 0)
	{
		m_sIconImage = pstrValue;
	}
	else if (_tcscmp(pstrName, _T("desciconimage")) == 0)
	{
		m_sDescIconImage = pstrValue;
	}
	else if (_tcscmp(pstrName, _T("showicon")) == 0)
	{
		if (_tcscmp(pstrValue, _T("true")) == 0)
		{
			m_bShowIcon = true;
		}
		else m_bShowIcon = false;
	}
	else if (_tcscmp(pstrName, _T("btnoffset")) == 0)
	{
		RECT rcBtnPos = { 0 };
		LPTSTR pstr = NULL;
		rcBtnPos.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
		rcBtnPos.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
		rcBtnPos.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
		rcBtnPos.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
		SetBtnOffset(rcBtnPos);
	}
	else if (_tcscmp(pstrName, _T("btnsize")) == 0)
	{
		LPTSTR pstr = NULL;
		m_szBtn.cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
		m_szBtn.cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
	}
	else if (_tcscmp(pstrName, _T("btnnormalimage")) == 0)
	{
		m_sBtnNormalImage = pstrValue;
	}
	else if (_tcscmp(pstrName, _T("btnhotimage")) == 0)
	{
		m_sBtnHotImage = pstrValue;
	}
	else if (_tcscmp(pstrName, _T("btnpushedimage")) == 0)
	{
		m_sBtnPushedImage = pstrValue;
	}
	else if (_tcscmp(pstrName, _T("align")) == 0) {
		if (_tcsstr(pstrValue, _T("left")) != NULL) {
			m_uTextStyle &= ~(DT_CENTER | DT_RIGHT | DT_TOP | DT_BOTTOM);
			m_uTextStyle |= DT_LEFT;
		}
		if (_tcsstr(pstrValue, _T("center")) != NULL) {
			m_uTextStyle &= ~(DT_LEFT | DT_RIGHT);
			m_uTextStyle |= DT_CENTER;
		}
		if (_tcsstr(pstrValue, _T("right")) != NULL) {
			m_uTextStyle &= ~(DT_LEFT | DT_CENTER | DT_TOP | DT_BOTTOM);
			m_uTextStyle |= DT_RIGHT;
		}
		if (_tcsstr(pstrValue, _T("top")) != NULL) {
			m_uTextStyle &= ~(DT_BOTTOM | DT_VCENTER | DT_LEFT | DT_RIGHT);
			m_uTextStyle |= DT_TOP;
		}
		if (_tcsstr(pstrValue, _T("bottom")) != NULL) {
			m_uTextStyle &= ~(DT_TOP | DT_VCENTER | DT_LEFT | DT_RIGHT);
			m_uTextStyle |= DT_BOTTOM;
		}
	}
	else if (_tcscmp(pstrName, _T("endellipsis")) == 0) {
		if (_tcscmp(pstrValue, _T("true")) == 0)
		{
			m_uTextStyle &= ~DT_PATH_ELLIPSIS;
			m_uTextStyle |= DT_END_ELLIPSIS;
		}
		else m_uTextStyle &= ~DT_END_ELLIPSIS;
	}
	else if (_tcscmp(pstrName, _T("pathellipsis")) == 0)    //add by lighten 2013.03.29, 支持中间省略
	{
		if (_tcscmp(pstrValue, _T("true")) == 0)
		{
			m_uTextStyle &= ~DT_END_ELLIPSIS;
			m_uTextStyle |= DT_PATH_ELLIPSIS;
		}
		else m_uTextStyle &= ~DT_PATH_ELLIPSIS;
	}
	else if (_tcscmp(pstrName, _T("font")) == 0) SetFont(_ttoi(pstrValue));
	else if (_tcscmp(pstrName, _T("textcolor")) == 0) {
		if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		LPTSTR pstr = NULL;
		DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
		SetTextColor(clrColor);
	}
	else if (_tcscmp(pstrName, _T("disabledtextcolor")) == 0) {
		if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		LPTSTR pstr = NULL;
		DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
		SetDisabledTextColor(clrColor);
	}
	else if (_tcscmp(pstrName, _T("textpadding")) == 0) {
		RECT rcTextPadding = { 0 };
		LPTSTR pstr = NULL;
		rcTextPadding.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
		rcTextPadding.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
		rcTextPadding.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
		rcTextPadding.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
		SetTextPadding(rcTextPadding);
	}
	else if (_tcscmp(pstrName, _T("showhtml")) == 0) SetShowHtml(_tcscmp(pstrValue, _T("true")) == 0);
	else if (_tcscmp(pstrName, _T("delayhover")) == 0)
	{
		if (_tcscmp(pstrValue, _T("true")) == 0)
		{
			m_bDelayHover = true;
		}
		else
		{
			m_bDelayHover = false;
		}
	}
	else if (_tcscmp(pstrName, _T("readonly")) == 0) SetReadOnly(_tcscmp(pstrValue, _T("true")) == 0);
	else if (_tcscmp(pstrName, _T("numberonly")) == 0) SetNumberOnly(_tcscmp(pstrValue, _T("true")) == 0);
	else if (_tcscmp(pstrName, _T("password")) == 0) SetPasswordMode(_tcscmp(pstrValue, _T("true")) == 0);
	else if (_tcscmp(pstrName, _T("passwordChar")) == 0) SetPasswordChar(pstrValue[0]);
	else if (_tcscmp(pstrName, _T("maxchar")) == 0) SetMaxChar(_ttoi(pstrValue));
	else if (_tcscmp(pstrName, _T("nativebkcolor")) == 0) {
		if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		LPTSTR pstr = NULL;
		DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
		SetNativeEditBkColor(clrColor);
	}
	else if (_tcscmp(pstrName, _T("editoffset")) == 0)
	{
		RECT rcEditOffset = { 0 };
		LPTSTR pstr = NULL;
		rcEditOffset.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
		rcEditOffset.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
		rcEditOffset.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
		rcEditOffset.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
		SetEditOffSet(rcEditOffset);
	}
	else if (_tcscmp(pstrName, _T("defaulttextfont")) == 0) m_iDefaultTextFont = _ttoi(pstrValue);
	else if (_tcscmp(pstrName, _T("defaulttextcolor")) == 0) {
		if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		LPTSTR pstr = NULL;
		DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
		m_dwDefaultTextColor = clrColor;
	}
	else if (_tcscmp(pstrName, _T("defaulttext")) == 0) SetDefaultText(pstrValue);
	else if (_tcscmp(pstrName, _T("edithotimage")) == 0) SetEditHotImage(pstrValue);
	else if (_tcscmp(pstrName, _T("editfocusedimage")) == 0) SetEditFocusedImage(pstrValue);
	else
	 CComboUI::SetAttribute(pstrName, pstrValue);
}


bool CEditComboUI::RemoveAt(int iIndex)
{
	bool bRet = CComboUI::RemoveAt(iIndex);
	CStdString strText = GetSelText();
// 	if (m_items.GetSize() == 0)
// 	{
// 		CStdString strText = _T("");
		SetText(strText);
//	}
	return bRet;
}


void CEditComboUI::PaintText(HDC hDC)
{
	RECT rcText = m_rcItem;
	rcText.left += m_rcTextPadding.left;
	rcText.right -= m_rcTextPadding.right;
	rcText.top += m_rcTextPadding.top;
	rcText.bottom -= m_rcTextPadding.bottom;
    int nLinks = 0;

	if (m_iCurSel == -1)
	{
		if (m_sText.IsEmpty())
		{
			if (!m_strDefaultText.IsEmpty())
			{
				CRenderEngine::DrawText(hDC, m_pManager, rcText, m_strDefaultText, m_dwDefaultTextColor, \
					m_iDefaultTextFont, DT_SINGLELINE | m_uTextStyle);
				//m_sText = m_strDefaultText;
			}
			return;
		}
	}


	if (m_nLastSel == -1)
	{
		if (m_bShowIcon && (m_items.GetSize() > m_iCurSel))
		{
			CComboElementUI* pControl = static_cast<CComboElementUI*>(m_items[m_iCurSel]);
			if (pControl)
			{
				pControl->SetShowIcon(true);
			}
		}
		m_nLastSel = m_iCurSel;
	}
	if (m_nLastSel >= m_items.GetSize())
	{
		m_nLastSel = m_iCurSel;
	}

//	if( m_nLastSel >= 0 ) {
// 		CControlUI* pControl = static_cast<CControlUI*>(m_items[m_nLastSel]);
// 		IListItemUI* pElement = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
// 		if( pElement != NULL ) {
//			CStdString sText = pControl->GetText();			
	CStdString sText = m_sText;
	if (IsEnabled()) {
		if (m_bShowHtml)
			CRenderEngine::DrawHtmlText(hDC, m_pManager, rcText, sText, m_dwTextColor, \
			NULL, NULL, nLinks, DT_SINGLELINE | m_uTextStyle);
		else
			CRenderEngine::DrawText(hDC, m_pManager, rcText, sText, m_dwTextColor, \
			m_iFont, DT_SINGLELINE | m_uTextStyle);
	}
	else {
		if (m_bShowHtml)
			CRenderEngine::DrawHtmlText(hDC, m_pManager, rcText, sText, m_dwDisabledTextColor, \
			NULL, NULL, nLinks, DT_SINGLELINE | m_uTextStyle);
		else
			CRenderEngine::DrawText(hDC, m_pManager, rcText, sText, m_dwDisabledTextColor, \
			m_iFont, DT_SINGLELINE | m_uTextStyle);
	}
// 		}
// 		else {
// 			RECT rcOldPos = pControl->GetPos();
// 			pControl->SetPos(rcText);
// 			pControl->DoPaint(hDC, rcText);
// 			pControl->SetPos(rcOldPos);
// 		}
// 	}
}

}