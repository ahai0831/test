#include "StdAfx.h"
#include "EditComboUI.h"

namespace DuiLib {

CComboUIEx::CComboUIEx(void): m_uTextStyle(DT_VCENTER), m_dwTextColor(1), 
    m_dwDisabledTextColor(1), m_iFont(-1), m_bShowHtml(false)
{
	m_szBtn.cx = m_szBtn.cy = 0;
	m_rcBtnOffset.left = m_rcBtnOffset.right = m_rcBtnOffset.top = m_rcBtnOffset.bottom = 0;
	m_nLastSel = -1;
	m_bShowIcon = false;
	m_rcBtnPos = { 0 };
	m_bDelayHover = false;
}

CComboUIEx::~CComboUIEx(void)
{
}

LPCTSTR CComboUIEx::GetClass() const
{
	return _T("ComboUIEx");
}

LPVOID CComboUIEx::GetInterface(LPCTSTR pstrName)
{
	if( _tcscmp(pstrName, _T("ComboEx")) == 0 ) return static_cast<CComboUIEx*>(this);
	return CComboUI::GetInterface(pstrName);
}

UINT CComboUIEx::GetControlFlags() const
{
	return UIFLAG_TABSTOP;
}

bool CComboUIEx::ClickItem(int iIndex, bool bTakeFocus /* = false */)
{
	if (m_iCurSel == iIndex)
	{
		if( m_pManager != NULL ) m_pManager->SendNotify(this, _T("itemselect"), DUILIB_COMBO_ITMESELECT, m_iCurSel, 0);
	}
	
	m_nLastSel = iIndex;
	bool bret = SelectItem(iIndex, bTakeFocus);
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

void CComboUIEx::SetItemShowIcon(int nIndex)
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

void CComboUIEx::DoEvent(TEventUI& event)
{
	if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND) {
		if (m_pParent != NULL) m_pParent->DoEvent(event);
		else CContainerUI::DoEvent(event);
		return;
	}
	if (event.Type == UIEVENT_MOUSEENTER)
	{
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
		if ((m_uButtonState & UISTATE_HOT) != 0) {
			{
				if (m_bDelayHover && m_pManager)
				{
					m_pManager->RemovePostPaint(this);
				}
				m_uButtonState &= ~UISTATE_HOT;
			}
			Invalidate();
		}
		return;
	}

	CComboUI::DoEvent(event);
}

void CComboUIEx::DoPaint(HDC hDC, const RECT& rcPaint)
{
	CComboUI::DoPaint(hDC, rcPaint);
}

void CComboUIEx::DoPostPaint(HDC hDC, const RECT& rcPaint)
{
	if ((m_uButtonState & UISTATE_HOT) != 0)
	{
		CComboUI::DoPaint(hDC, rcPaint);
// 		if (!::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem)) return;
// 
// 		// 绘制循序：背景颜色->背景图->状态图->文本->边框
// 		if (m_cxyBorderRound.cx > 0 || m_cxyBorderRound.cy > 0) {
// 			CRenderClip roundClip;
// 			CRenderClip::GenerateRoundClip(hDC, m_rcPaint, m_rcItem, m_cxyBorderRound.cx, m_cxyBorderRound.cy, roundClip);
// 			PaintStatusImage(hDC);
// 		}
// 		else {
// 			PaintStatusImage(hDC);
// 		}

	}
}

bool CComboUIEx::CheckItem(int iIndex, bool bTakeFocus /* = false */)
{
	if (bTakeFocus == true)
	{
		if (m_nLastSel == -1)
		{
			m_nLastSel = m_iCurSel;
		}
		if( iIndex == m_iCurSel ) return true;
		int iOldSel = m_iCurSel;
		if( m_iCurSel >= 0 ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[m_iCurSel]);
			if( !pControl ) return false;
			IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
			if( pListItem != NULL ) pListItem->Select(false);
		}
		if( iIndex < 0 ) return false;
		if( m_items.GetSize() == 0 ) return false;
		if( iIndex >= m_items.GetSize() ) iIndex = m_items.GetSize() - 1;
		CControlUI* pControl = static_cast<CControlUI*>(m_items[iIndex]);
		if( !pControl || !pControl->IsVisible() || !pControl->IsEnabled() ) return false;
		IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
		if( pListItem == NULL ) return false;
		m_iCurSel = iIndex;
		if( m_pWindow != NULL ) pControl->SetFocus();
		pListItem->Select(true);
		Invalidate();
		return true;
	}
	return false;
}

void CComboUIEx::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcscmp(pstrName, _T("iconoffset")) == 0 )
	{
		RECT rcBtnPos = { 0 };
		LPTSTR pstr = NULL;
		rcBtnPos.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
		rcBtnPos.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
		rcBtnPos.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
		rcBtnPos.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);    
		SetIconOffset(rcBtnPos);
	}
	else if (_tcscmp(pstrName, _T("iconsize")) == 0 )
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
		if( _tcscmp(pstrValue, _T("true")) == 0 )
		{
			m_bShowIcon = true;
		}
		else m_bShowIcon = false;
	}
	else if (_tcscmp(pstrName, _T("btnoffset")) == 0 )
	{
		RECT rcBtnPos = { 0 };
		LPTSTR pstr = NULL;
		rcBtnPos.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
		rcBtnPos.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
		rcBtnPos.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
		rcBtnPos.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);    
		SetBtnOffset(rcBtnPos);
	}
	else if (_tcscmp(pstrName, _T("btnsize")) == 0 )
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
	else if( _tcscmp(pstrName, _T("align")) == 0 ) {
		if( _tcsstr(pstrValue, _T("left")) != NULL ) {
			m_uTextStyle &= ~(DT_CENTER | DT_RIGHT | DT_TOP | DT_BOTTOM);
			m_uTextStyle |= DT_LEFT;
		}
		if( _tcsstr(pstrValue, _T("center")) != NULL ) {
			m_uTextStyle &= ~(DT_LEFT | DT_RIGHT );
			m_uTextStyle |= DT_CENTER;
		}
		if( _tcsstr(pstrValue, _T("right")) != NULL ) {
			m_uTextStyle &= ~(DT_LEFT | DT_CENTER | DT_TOP | DT_BOTTOM);
			m_uTextStyle |= DT_RIGHT;
		}
		if( _tcsstr(pstrValue, _T("top")) != NULL ) {
			m_uTextStyle &= ~(DT_BOTTOM | DT_VCENTER | DT_LEFT | DT_RIGHT);
			m_uTextStyle |= DT_TOP;
		}
		if( _tcsstr(pstrValue, _T("bottom")) != NULL ) {
			m_uTextStyle &= ~(DT_TOP | DT_VCENTER | DT_LEFT | DT_RIGHT);
			m_uTextStyle |= DT_BOTTOM;
		}
	}
	else if( _tcscmp(pstrName, _T("endellipsis")) == 0 ) {
		if( _tcscmp(pstrValue, _T("true")) == 0 )
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
	else if( _tcscmp(pstrName, _T("font")) == 0 ) SetFont(_ttoi(pstrValue));
	else if( _tcscmp(pstrName, _T("textcolor")) == 0 ) {
		if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		LPTSTR pstr = NULL;
		DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
		SetTextColor(clrColor);
	}
	else if( _tcscmp(pstrName, _T("disabledtextcolor")) == 0 ) {
		if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		LPTSTR pstr = NULL;
		DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
		SetDisabledTextColor(clrColor);
	}
	else if( _tcscmp(pstrName, _T("textpadding")) == 0 ) {
		RECT rcTextPadding = { 0 };
		LPTSTR pstr = NULL;
		rcTextPadding.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
		rcTextPadding.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
		rcTextPadding.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
		rcTextPadding.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);    
		SetTextPadding(rcTextPadding);
	}
	else if( _tcscmp(pstrName, _T("showhtml")) == 0 ) SetShowHtml(_tcscmp(pstrValue, _T("true")) == 0);
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
	else CComboUI::SetAttribute(pstrName, pstrValue);
}

void CComboUIEx::SetBtnNormalImage(LPCTSTR lpstr)
{
	m_sBtnNormalImage = lpstr;
}

LPCTSTR CComboUIEx::GetBtnNormalImage()
{
	return m_sBtnNormalImage;
}

void CComboUIEx::SetBtnHotImage(LPCTSTR lpstr)
{
	m_sBtnHotImage = lpstr;
}

LPCTSTR CComboUIEx::GetBtnHotImage()
{
	return m_sBtnHotImage;
}

void CComboUIEx::SetBtnPushedImage(LPCTSTR lpstr)
{
	m_sBtnPushedImage = lpstr;
}

LPCTSTR CComboUIEx::GetBtnPushedImage()
{
	return m_sBtnPushedImage;
}

void CComboUIEx::SetBtnOffset(RECT rc)
{
	m_rcBtnOffset = rc;
}

RECT CComboUIEx::GetBtnOffset()
{
	return m_rcBtnOffset;
}

//显示
void CComboUIEx::SetIconOffset(RECT rc)   //设置Icon偏移量，右边的设置右下角偏移量
{
	m_rcIconOffset = rc;
}

RECT CComboUIEx::GetIconOffset()
{
	return m_rcIconOffset;
}

SIZE CComboUIEx::GetIconSize()      //按钮大小
{
	return m_szIcon;
}

void CComboUIEx::SetIconImage(LPCTSTR lpstr)
{
	m_sIconImage = lpstr;
}

LPCTSTR CComboUIEx::GetIconImage()
{
	return m_sIconImage;
}

void CComboUIEx::SetDescIconImage(LPCTSTR lpstr)
{
	m_sDescIconImage = lpstr;
}

LPCTSTR CComboUIEx::GetDescIconImage()
{
	return m_sDescIconImage;
}

SIZE CComboUIEx::GetBtnSize()
{
	return m_szBtn;
}

void CComboUIEx::SetTextStyle(UINT uStyle)
{
	m_uTextStyle = uStyle;
	Invalidate();
}

UINT CComboUIEx::GetTextStyle() const
{
	return m_uTextStyle;
}

void CComboUIEx::SetTextColor(DWORD dwTextColor)
{
	m_dwTextColor = dwTextColor;
}

DWORD CComboUIEx::GetTextColor() const
{
	return m_dwTextColor;
}

void CComboUIEx::SetDisabledTextColor(DWORD dwTextColor)
{
	m_dwDisabledTextColor = dwTextColor;
}

DWORD CComboUIEx::GetDisabledTextColor() const
{
	return m_dwDisabledTextColor;
}

void CComboUIEx::SetFont(int index)
{
	m_iFont = index;
}

int CComboUIEx::GetFont() const
{
	return m_iFont;
}

RECT CComboUIEx::GetTextPadding() const
{
	return m_rcTextPadding;
}

void CComboUIEx::SetTextPadding(RECT rc)
{
	m_rcTextPadding = rc;
	Invalidate();
}

bool CComboUIEx::IsShowHtml()
{
	return m_bShowHtml;
}

void CComboUIEx::SetShowHtml(bool bShowHtml)
{
	if( m_bShowHtml == bShowHtml ) return;

	m_bShowHtml = bShowHtml;
	Invalidate();
}

void CComboUIEx::PaintText(HDC hDC)
{
	RECT rcText = m_rcItem;
	rcText.left += m_rcTextPadding.left;
	rcText.right -= m_rcTextPadding.right;
	rcText.top += m_rcTextPadding.top;
	rcText.bottom -= m_rcTextPadding.bottom;
    int nLinks = 0;

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

	if( m_nLastSel >= 0 ) {
		CControlUI* pControl = static_cast<CControlUI*>(m_items[m_nLastSel]);
		IListItemUI* pElement = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
		if( pElement != NULL ) {
			CStdString sText = pControl->GetText();			
			if( IsEnabled() ) {
				if( m_bShowHtml )
					CRenderEngine::DrawHtmlText(hDC, m_pManager, rcText, sText, m_dwTextColor, \
					NULL, NULL, nLinks, DT_SINGLELINE | m_uTextStyle);
				else
					CRenderEngine::DrawText(hDC, m_pManager, rcText, sText, m_dwTextColor, \
					m_iFont, DT_SINGLELINE | m_uTextStyle);
			}
			else {
				if( m_bShowHtml )
					CRenderEngine::DrawHtmlText(hDC, m_pManager, rcText, sText, m_dwDisabledTextColor, \
					NULL, NULL, nLinks, DT_SINGLELINE | m_uTextStyle);
				else
					CRenderEngine::DrawText(hDC, m_pManager, rcText, sText, m_dwDisabledTextColor, \
					m_iFont, DT_SINGLELINE | m_uTextStyle);
			}
		}
		else {
			RECT rcOldPos = pControl->GetPos();
			pControl->SetPos(rcText);
			pControl->DoPaint(hDC, rcText);
			pControl->SetPos(rcOldPos);
		}
	}
}


bool CComboUIEx::RemoveItem(int iIndex)
{
	if( m_pWindow != NULL ) m_pWindow->Close();
	if( m_pManager != NULL ) m_pManager->SendNotify(this, _T("itemdelete"), DUILIB_COMBO_ITMEDELETE, iIndex, 0);
	//RemoveAt(iIndex);
	return true;
}

CComboElementUI::CComboElementUI() : 
m_iIndex(-1),
m_pOwner(NULL), 
m_uButtonState(0)
{
	m_bSelected = false;
	m_bShowIcon = false;
	m_bShowButton = false;
	m_nShowCount = 0;
	m_rcBtnPos = { 0 };
	m_rcIconPos = { 0 };
}

LPCTSTR CComboElementUI::GetClass() const
{
	return _T("CComboElementUI");
}

UINT CComboElementUI::GetControlFlags() const
{
	return UIFLAG_WANTRETURN;
}

LPVOID CComboElementUI::GetInterface(LPCTSTR pstrName)
{
	if( _tcscmp(pstrName, _T("ListItem")) == 0 ) return static_cast<IListItemUI*>(this);
	if( _tcscmp(pstrName, _T("ComboElement")) == 0 ) return static_cast<CComboElementUI*>(this);
	return CControlUI::GetInterface(pstrName);
}

IListOwnerUI* CComboElementUI::GetOwner()
{
	return m_pListOwner;
}

void CComboElementUI::SetOwner(CControlUI* pOwner)
{
	if (pOwner)
	{
		CStdString strClass = pOwner->GetClass();
		if (strClass == _T("ComboUIEx"))
		{
			CComboUIEx *pComboUIex = static_cast<CComboUIEx*>(pOwner);
			m_pOwner = static_cast<IComboExUI*>(pComboUIex);
			m_pListOwner = static_cast<IListOwnerUI*>(pComboUIex);
		}
		else if (strClass == _T("EditComboUI"))
		{
			CEditComboUI *pComboUIex = static_cast<CEditComboUI*>(pOwner);
			m_pOwner = static_cast<IComboExUI*>(pComboUIex);
			m_pListOwner = static_cast<IListOwnerUI*>(pComboUIex);
		}
	}
}

void CComboElementUI::SetVisible(bool bVisible)
{
	if( !IsVisible() && m_bSelected)
	{
		m_bSelected = false;
		if (m_pListOwner != NULL) m_pListOwner->SelectItem(-1);
	}
}

void CComboElementUI::SetEnabled(bool bEnable)
{
	CControlUI::SetEnabled(bEnable);
	if( !IsEnabled() ) {
		m_uButtonState = 0;
	}
}

int CComboElementUI::GetIndex() const
{
	return m_iIndex;
}

void CComboElementUI::SetIndex(int iIndex)
{
	m_iIndex = iIndex;
}

void CComboElementUI::Invalidate()
{
	if( !IsVisible() ) return;

	if( GetParent() ) {
		CContainerUI* pParentContainer = static_cast<CContainerUI*>(GetParent()->GetInterface(_T("Container")));
		if( pParentContainer ) {
			RECT rc = pParentContainer->GetPos();
			RECT rcInset = pParentContainer->GetInset();
			rc.left += rcInset.left;
			rc.top += rcInset.top;
			rc.right -= rcInset.right;
			rc.bottom -= rcInset.bottom;
			CScrollBarUI* pVerticalScrollBar = pParentContainer->GetVerticalScrollBar();
			if( pVerticalScrollBar && pVerticalScrollBar->IsVisible() ) rc.right -= pVerticalScrollBar->GetFixedWidth();
			CScrollBarUI* pHorizontalScrollBar = pParentContainer->GetHorizontalScrollBar();
			if( pHorizontalScrollBar && pHorizontalScrollBar->IsVisible() ) rc.bottom -= pHorizontalScrollBar->GetFixedHeight();

			RECT invalidateRc = m_rcItem;
			if( !::IntersectRect(&invalidateRc, &m_rcItem, &rc) ) 
			{
				return;
			}

			CControlUI* pParent = GetParent();
			RECT rcTemp;
			RECT rcParent;
			while( pParent = pParent->GetParent() )
			{
				rcTemp = invalidateRc;
				rcParent = pParent->GetPos();
				if( !::IntersectRect(&invalidateRc, &rcTemp, &rcParent) ) 
				{
					return;
				}
			}

			if( m_pManager != NULL ) m_pManager->Invalidate(invalidateRc);
		}
		else {
			CControlUI::Invalidate();
		}
	}
	else {
		CControlUI::Invalidate();
	}
}

bool CComboElementUI::Activate()
{
	if( !CControlUI::Activate() ) return false;
	if( m_pManager != NULL ) m_pManager->SendNotify(this, _T("itemactivate"), DUILIB_LIST_ITEMACTIVE);
	return true;
}

bool CComboElementUI::IsSelected() const
{
	return m_bSelected;
}

bool CComboElementUI::Select(bool bSelect /* = true */, bool bInvalidate /* = true */)
{
	if( !IsEnabled() ) return false;
	if( bSelect == m_bSelected )
		return true;
	m_bSelected = bSelect;
	if (bInvalidate == true)
	{
		Invalidate();
	}

	return true;
}

bool CComboElementUI::IsExpanded() const
{
	return false;
}

bool CComboElementUI::Expand(bool /*bExpand = true*/)
{
	return false;
}

void CComboElementUI::DoEvent(TEventUI& event)
{
	if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
		if (m_pListOwner != NULL) m_pListOwner->DoEvent(event);
		else CControlUI::DoEvent(event);
		return;
	}

	if( event.Type == UIEVENT_DBLCLICK )
	{
		if( IsEnabled() ) {
			Activate();
			Invalidate();
		}
		return;
	}
	if( event.Type == UIEVENT_KEYDOWN && IsEnabled() )
	{
		if( event.chKey == VK_RETURN ) {
			Activate();
			Invalidate();
			return;
		}
	}
	if( event.Type == UIEVENT_BUTTONDOWN /*|| event.Type == UIEVENT_RBUTTONDOWN*/ )
	{
		if( ::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled() ) 
		{
			if (m_pOwner)
			{
				if (::PtInRect(&m_rcBtnPos, event.ptMouse))
				{
					m_uButtonState |= UISTATE_PUSHED | UISTATE_CAPTURED;
					Invalidate();
				}
				else
					m_pOwner->ClickItem(m_iIndex);
			}
		}
		return;
	}
	if( event.Type == UIEVENT_BUTTONUP ) 
	{
		if ((m_uButtonState & UISTATE_CAPTURED) != 0)
		{
			if (m_pOwner)
			{
				if (::PtInRect(&m_rcBtnPos, event.ptMouse))
				{
					m_pOwner->RemoveItem(m_iIndex);
				}
			}
			m_uButtonState &= ~(UISTATE_PUSHED | UISTATE_CAPTURED);
			Invalidate();
		}
		return;
	}
	if( event.Type == UIEVENT_MOUSEMOVE )
	{
		if( IsEnabled() && m_pOwner)    
		{
			if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) 
			{
				if (::PtInRect(&m_rcBtnPos, event.ptMouse))
				{
					m_uButtonState |= UISTATE_PUSHED;						
				}
				else 
					m_uButtonState &= ~UISTATE_PUSHED;
				Invalidate();
			}
			else if (::PtInRect(&m_rcBtnPos, event.ptMouse))
			{
				m_uButtonState = UISTATE_HOT;
				Invalidate();
			}
			else if (!PtInRect(&m_rcBtnPos, event.ptMouse))
			{
				m_uButtonState &= ~UISTATE_HOT;
				Invalidate();
			}
		}
		return;
	}
	if( event.Type == UIEVENT_MOUSEENTER )
	{
		if (m_pOwner)
		{
			m_pOwner->CheckItem(m_iIndex, true);
		}

		return;
	}
	if( event.Type == UIEVENT_MOUSELEAVE )
	{
		if( (m_uButtonState & UISTATE_HOT) != 0 )
		{
			m_uButtonState &= ~UISTATE_HOT;
			Invalidate();
		}
		return;
	}
	if (event.Type == UIEVENT_SETFOCUS)
	{
		return;
	}
	if (event.Type == UIEVENT_KILLFOCUS)
	{
		return;
	}

	// An important twist: The list-item will send the event not to its immediate
	// parent but to the "attached" list. A list may actually embed several components
	// in its path to the item, but key-presses etc. needs to go to the actual list.
	if (m_pListOwner != NULL) m_pListOwner->DoEvent(event); else CControlUI::DoEvent(event);
}

void CComboElementUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if( _tcscmp(pstrName, _T("selected")) == 0 ) Select();
	else CControlUI::SetAttribute(pstrName, pstrValue);
}

void CComboElementUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	if( !::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem) ) return;
	DrawItemBk(hDC, m_rcItem);
	DrawItemText(hDC, m_rcItem);
}

//add by lighten
SIZE CComboElementUI::EstimateSize(SIZE szAvailable)
{
	SIZE cXY = {0, m_cxyFixed.cy};
	return cXY;
}

void CComboElementUI::SetPos(RECT rc)
{
	CControlUI::SetPos(rc);
}

void CComboElementUI::CalIconPos()
{
	if (m_pOwner == NULL)
	{
		return;
	}

	RECT rcBtnOffset = m_pOwner->GetIconOffset();
	SIZE szBtn = m_pOwner->GetIconSize();
	if (szBtn.cx == 0 || szBtn.cy == 0)
	{
//		m_bShowIcon = false;
		return;
	}
	else
//		m_bShowIcon = true;

	m_rcIconPos.right = m_rcItem.right - rcBtnOffset.right;
	if (m_rcIconPos.right < m_rcItem.left)  //修正大小
	{
		m_rcIconPos.right = m_rcItem.left;
	}
	m_rcIconPos.left = m_rcIconPos.right - szBtn.cx;
	if (m_rcIconPos.left < m_rcItem.left)
	{
		m_rcIconPos.left = m_rcItem.left;
	}

	m_rcIconPos.bottom = m_rcItem.bottom - rcBtnOffset.bottom;
	if (m_rcIconPos.bottom < m_rcItem.top)
	{
		m_rcIconPos.bottom = m_rcItem.top;
	}

	m_rcIconPos.top = m_rcIconPos.bottom - szBtn.cy;
	if (m_rcIconPos.top < m_rcItem.top)
	{
		m_rcIconPos.top = m_rcItem.top;
	}
}

void CComboElementUI::SetShowIcon(bool bShow /* = true */)
{
	m_bShowIcon = bShow;
	if (bShow == true)
	{
		m_nShowCount ++;
	}
	else
	{
		m_nShowCount = 0;
	}
}

//计算Btn所在的位置
void CComboElementUI::CalBtnPos()
{
	if (m_pOwner == NULL)
	{
		return;
	}
	
	RECT rcBtnOffset = m_pOwner->GetBtnOffset();
	SIZE szBtn = m_pOwner->GetBtnSize();
	if (szBtn.cx == 0 || szBtn.cy == 0)
	{
		m_bShowButton = false;
		return;
	}
	else
		m_bShowButton = true;

	m_rcBtnPos.right = m_rcItem.right - rcBtnOffset.right;
	if (m_rcBtnPos.right < m_rcItem.left)  //修正大小
	{
		m_rcBtnPos.right = m_rcItem.left;
	}
	m_rcBtnPos.left = m_rcBtnPos.right - szBtn.cx;
	if (m_rcBtnPos.left < m_rcItem.left)
	{
		m_rcBtnPos.left = m_rcItem.left;
	}

	m_rcBtnPos.bottom = m_rcItem.bottom - rcBtnOffset.bottom;
	if (m_rcBtnPos.bottom < m_rcItem.top)
	{
		m_rcBtnPos.bottom = m_rcItem.top;
	}

	m_rcBtnPos.top = m_rcBtnPos.bottom - szBtn.cy;
	if (m_rcBtnPos.top < m_rcItem.top)
	{
		m_rcBtnPos.top = m_rcItem.top;
	}
}

void CComboElementUI::DrawItemText(HDC hDC, const RECT& rcItem)
{
	if( m_sText.IsEmpty() ) return;

	if (m_pListOwner == NULL) return;
	TListInfoUI* pInfo = m_pListOwner->GetListInfo();
	DWORD iTextColor = pInfo->dwTextColor;
	if( (m_uButtonState & UISTATE_HOT) != 0 ) {
		iTextColor = pInfo->dwHotTextColor;
	}
	if( IsSelected() ) {
		iTextColor = pInfo->dwSelectedTextColor;
	}
	if( !IsEnabled() ) {
		iTextColor = pInfo->dwDisabledTextColor;
	}
	int nLinks = 0;
	RECT rcText = rcItem;
	rcText.left += pInfo->rcTextPadding.left;
	rcText.right -= pInfo->rcTextPadding.right;
	rcText.top += pInfo->rcTextPadding.top;
	rcText.bottom -= pInfo->rcTextPadding.bottom;

	if( pInfo->bShowHtml )
		CRenderEngine::DrawHtmlText(hDC, m_pManager, rcText, m_sText, iTextColor, \
		NULL, NULL, nLinks, DT_SINGLELINE | pInfo->uTextStyle);
	else
		CRenderEngine::DrawText(hDC, m_pManager, rcText, m_sText, iTextColor, \
		pInfo->nFont, DT_SINGLELINE | pInfo->uTextStyle);
	return;
}

void CComboElementUI::DrawItemBk(HDC hDC, const RECT& rcItem)
{
	ASSERT(m_pListOwner);
	if (m_pListOwner == NULL) return;
	CalBtnPos();
	CalIconPos();

	TListInfoUI* pInfo = m_pListOwner->GetListInfo();
	DWORD iBackColor = 0;
	if( m_iIndex % 2 == 0 ) iBackColor = pInfo->dwBkColor;

	if( (m_uButtonState & UISTATE_HOT) != 0 ) {
		iBackColor = pInfo->dwHotBkColor;
	}
	if( IsSelected() ) {
		iBackColor = pInfo->dwSelectedBkColor;
	}
	if( !IsEnabled() ) {
		iBackColor = pInfo->dwDisabledBkColor;
	}
	if ( iBackColor != 0 ) {
		CRenderEngine::DrawColor(hDC, m_pManager, m_rcItem, GetAdjustColor(iBackColor));
	}

	if( !IsEnabled() ) {
		if( !pInfo->sDisabledImage.IsEmpty() ) {
			if( !DrawImage(hDC, (LPCTSTR)pInfo->sDisabledImage) ) pInfo->sDisabledImage.Empty();
			else return;
		}
	}
 	if( IsSelected() ) {
 		if( !pInfo->sSelectedImage.IsEmpty() ) {
 			if( !DrawImage(hDC, (LPCTSTR)pInfo->sSelectedImage) ) pInfo->sSelectedImage.Empty();
 			/*else return;*/
 		}
 	}

	if( (m_uButtonState & UISTATE_HOT) != 0 ) {
		if( !pInfo->sHotImage.IsEmpty() ) {
			if( !DrawImage(hDC, (LPCTSTR)pInfo->sHotImage) ) pInfo->sHotImage.Empty();
		/*	else return;*/
		}
	}

	if( m_sBkImage.IsEmpty() ) {
		if( !pInfo->sBkImage.IsEmpty() ) {
			if( !DrawImage(hDC, (LPCTSTR)pInfo->sBkImage) ) pInfo->sBkImage.Empty();
		/*	else return;*/
		}
	}

	if ( pInfo->dwLineColor != 0 ) {
		RECT rcLine = { m_rcItem.left, m_rcItem.bottom - 1, m_rcItem.right, m_rcItem.bottom - 1 };
		CRenderEngine::DrawLine(hDC, rcLine, 1, GetAdjustColor(pInfo->dwLineColor));
	}
	
	//绘制按钮
	if (m_bShowButton)
	{
		DrawBtn(hDC, rcItem);
	}

	//绘制图标
	if (m_bShowIcon)
	{
		DrawIcon(hDC, rcItem);
	}
}


void CComboElementUI::DrawIcon(HDC hDC, const RECT& rcItem)
{
	if (m_pOwner == NULL)
	{
		return;
	}
	if (m_nShowCount%2 == 1)
	{
		if( m_pOwner->GetIconImage() != NULL)
		{
			if( !DrawImage(hDC, (LPCTSTR)m_pOwner->GetIconImage(), m_rcIconPos) ) m_pOwner->SetIconImage(NULL);
			else return;
		}
	}
	else
	{
		if( m_pOwner->GetDescIconImage() != NULL)
		{
			if( !DrawImage(hDC, (LPCTSTR)m_pOwner->GetDescIconImage(), m_rcIconPos) ) m_pOwner->SetDescIconImage(NULL);
			else return;
		}
	}
}

void CComboElementUI::DrawBtn(HDC hDC, const RECT& rcItem)
{
	if (!IsSelected())
	{
		return;
	}
	if (m_pOwner == NULL)
	{
		return;
	}
	if ((m_uButtonState & UISTATE_PUSHED) != 0)
	{
		if( m_pOwner->GetBtnPushedImage() != NULL) {
			if( !DrawImage(hDC, (LPCTSTR)m_pOwner->GetBtnPushedImage(), m_rcBtnPos) ) m_pOwner->SetBtnPushedImage(NULL);
			else return;
		}
	}
	else if( (m_uButtonState & UISTATE_HOT) != 0 ) {
		if( m_pOwner->GetBtnHotImage() != NULL ) {
			if( !DrawImage(hDC, (LPCTSTR)m_pOwner->GetBtnHotImage(), m_rcBtnPos) ) m_pOwner->SetBtnHotImage(NULL);
			else return;
		}
	}

	if( m_pOwner->GetBtnNormalImage() != NULL) {
		if( !DrawImage(hDC, m_pOwner->GetBtnNormalImage(), m_rcBtnPos) ) m_pOwner->SetBtnNormalImage(NULL);
		else return;
	}
}
}