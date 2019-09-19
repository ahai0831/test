#include "StdAfx.h"
namespace DuiLib {
void CComboWndEx::Init(CURLBar* pOwner)
{
	m_pOwner = pOwner;
	m_pLayout = NULL;
	m_iOldSel = m_pOwner->GetCurSel();

	// Position the popup window in absolute space
	SIZE szDrop = m_pOwner->GetDropBoxSize();
	RECT rcOwner = pOwner->GetPos();
	RECT rc = rcOwner;
	rc.top = rc.bottom;
	rc.bottom = rc.top + szDrop.cy;
	if( szDrop.cx > 0 ) rc.right = rc.left + szDrop.cx;

	SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };
	int cyFixed = 0;
	for( int it = m_pOwner->GetHidenItems() - 1; it >= 0; it-- ) {
		CControlUI* pControl = static_cast<CControlUI*>(pOwner->GetItemAt(it));
		if( !pControl->IsVisible() ) continue;
		SIZE sz = pControl->EstimateSize(szAvailable);
		cyFixed += sz.cy;
	}
	cyFixed += 4; // CVerticalLayoutUI 默认的Inset 调整
	rc.bottom = rc.top + MIN(cyFixed, szDrop.cy);

	::MapWindowRect(pOwner->GetManager()->GetPaintWindow(), HWND_DESKTOP, &rc);

	MONITORINFO oMonitor = {};
	oMonitor.cbSize = sizeof(oMonitor);
	::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTOPRIMARY), &oMonitor);
	CRect rcWork = oMonitor.rcWork;
	if( rc.bottom > rcWork.bottom ) {
		rc.left = rcOwner.left;
		rc.right = rcOwner.right;
		if( szDrop.cx > 0 ) rc.right = rc.left + szDrop.cx;
		rc.top = rcOwner.top - MIN(cyFixed, szDrop.cy);
		rc.bottom = rcOwner.top;
		::MapWindowRect(pOwner->GetManager()->GetPaintWindow(), HWND_DESKTOP, &rc);
	}

	Create(pOwner->GetManager()->GetPaintWindow(), NULL, WS_POPUP, WS_EX_TOOLWINDOW, rc);
	//add by lighten
	for (int i = 0; i< m_pOwner->GetManager()->GetCustomFontCount();i++)
	{
		TFontInfo *pFontInfo = m_pOwner->GetManager()->GetFontInfo(i);
		m_pm.AddFont(pFontInfo->sFontName, pFontInfo->iSize, pFontInfo->bBold, pFontInfo->bUnderline, pFontInfo->bItalic);
	}

	// HACK: Don't deselect the parent's caption

	// add by lighten , visible the sel item
	POINT pt={rc.left, rc.top};
	::ScreenToClient(m_pm.GetPaintWindow(), &pt);
	RECT rcClient;
	rcClient.left = pt.x;
	rcClient.top = pt.y;
	rcClient.right = rcClient.left + (rc.right - rc.left);
	rcClient.bottom = rcClient.top + (rc.bottom - rc.top);
	m_pLayout->SetPos(rcClient);
//	EnsureVisible(m_iOldSel);

	HWND hWndParent = m_hWnd;
	while( ::GetParent(hWndParent) != NULL ) hWndParent = ::GetParent(hWndParent);
	::ShowWindow(m_hWnd, SW_SHOW);
	::SendMessage(hWndParent, WM_NCACTIVATE, TRUE, 0L);
}

LPCTSTR CComboWndEx::GetWindowClassName() const
{
	return _T("ComboWndEx");
}

void CComboWndEx::OnFinalMessage(HWND hWnd)
{
	m_pm.ReapObjects(m_pLayout);
	m_pOwner->m_pWindow = NULL;
	m_pOwner->m_uButtonState &= ~ UISTATE_PUSHED;
	m_pOwner->Invalidate();
	delete this;
}

LRESULT CComboWndEx::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if( uMsg == WM_CREATE ) {
		m_pm.Init(m_hWnd);
		// The trick is to add the items to the new container. Their owner gets
		// reassigned by this operation - which is why it is important to reassign
		// the items back to the righfull owner/manager when the window closes.
		m_pLayout = new CVerticalLayoutUI;
		m_pm.UseParentResource(m_pOwner->GetManager());
		m_pLayout->SetManager(&m_pm, NULL, true);
		m_pLayout->SetInset(CRect(2, 2, 2, 2));
		m_pLayout->SetBkColor(0xFFFFFFFF);
		m_pLayout->SetBorderColor(0xFF85E4FF);
		m_pLayout->SetBorderSize(1);
		m_pLayout->SetAutoDestroy(false);
		m_pLayout->EnableScrollBar();
		LPCTSTR pDefaultAttributes = m_pOwner->GetManager()->GetDefaultAttributeList(_T("Combo_DropDown"));
		if( pDefaultAttributes ) {
			m_pLayout->ApplyAttributeList(pDefaultAttributes);
		}
		m_pLayout->ApplyAttributeList(m_pOwner->GetDropBoxAttributeList());
		for( int i = m_pOwner->GetHidenItems() - 1; i >= 0; i-- ) {
			CControlUI *pControl = static_cast<CControlUI *>(m_pOwner->GetItemAt(i));
			m_pLayout->Add(static_cast<CControlUI*>(m_pOwner->GetItemAt(i)));
		}
		m_pm.AttachDialog(m_pLayout);
		return 0;
	}
	else if( uMsg == WM_CLOSE ) {
		m_pOwner->SetManager(m_pOwner->GetManager(), m_pOwner->GetParent(), false);
		m_pOwner->SetPos(m_pOwner->GetPos());
		m_pOwner->SetFocus();
	}
	else if( uMsg == WM_LBUTTONDOWN ) {
		POINT pt = { 0 };
		::GetCursorPos(&pt);
		::ScreenToClient(m_pm.GetPaintWindow(), &pt);
		CControlUI* pControl = m_pm.FindControl(pt);
		if( pControl && _tcscmp(pControl->GetClass(), _T("ScrollBarUI")) != 0 ) PostMessage(WM_KILLFOCUS);
	}
	else if( uMsg == WM_KEYDOWN ) {
		switch( wParam ) {
		case VK_ESCAPE:
			m_pOwner->SelectItem(m_iOldSel);
			EnsureVisible(m_iOldSel);
			// FALL THROUGH...
		case VK_RETURN:
			PostMessage(WM_KILLFOCUS);
			break;
		default:
			TEventUI event;
			event.Type = UIEVENT_KEYDOWN;
			event.chKey = (TCHAR)wParam;
			m_pOwner->DoEvent(event);
			EnsureVisible(m_pOwner->GetCurSel());
			return 0;
		}
	}
	else if( uMsg == WM_MOUSEWHEEL ) {
		int zDelta = (int) (short) HIWORD(wParam);
		TEventUI event = { 0 };
		event.Type = UIEVENT_SCROLLWHEEL;
		event.wParam = MAKELPARAM(zDelta < 0 ? SB_LINEDOWN : SB_LINEUP, 0);
		event.lParam = lParam;
		event.dwTimestamp = ::GetTickCount();

		switch( LOWORD(event.wParam) ) 
		{
		case SB_LINEUP:
			m_pLayout->LineUp();
			return 0;
		case SB_LINEDOWN:
			m_pLayout->LineDown();
			return 0;
		}
		//         m_pOwner->DoEvent(event);
		//        EnsureVisible(m_pOwner->GetCurSel());
		return 0;
	}
	else if( uMsg == WM_KILLFOCUS ) {
		if( m_hWnd != (HWND) wParam ) PostMessage(WM_CLOSE);
	}

	LRESULT lRes = 0;
	if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;
	return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
}

void CComboWndEx::EnsureVisible(int iIndex)
{
	if( m_pOwner->GetCurSel() < 0 ) return;
	m_pLayout->FindSelectable(m_pOwner->GetCurSel(), false);
	RECT rcItem = m_pLayout->GetItemAt(iIndex)->GetPos();
	RECT rcList = m_pLayout->GetPos();
	CScrollBarUI* pHorizontalScrollBar = m_pLayout->GetHorizontalScrollBar();
	if( pHorizontalScrollBar && pHorizontalScrollBar->IsVisible() ) rcList.bottom -= pHorizontalScrollBar->GetFixedHeight();
	int iPos = m_pLayout->GetScrollPos().cy;
	if( rcItem.top >= rcList.top && rcItem.bottom < rcList.bottom ) return;
	int dx = 0;
	if( rcItem.top < rcList.top ) dx = rcItem.top - rcList.top;
	if( rcItem.bottom > rcList.bottom ) dx = rcItem.bottom - rcList.bottom;
	Scroll(0, dx);
}

void CComboWndEx::Scroll(int dx, int dy)
{
	if( dx == 0 && dy == 0 ) return;
	SIZE sz = m_pLayout->GetScrollPos();
	m_pLayout->SetScrollPos(CSize(sz.cx + dx, sz.cy + dy));
}

CURLBar::CURLBar(void) : m_pWindow(NULL), m_iCurSel(-1), m_uButtonState(0)
{
	m_szDropBox = CSize(0, 100);
	::ZeroMemory(&m_rcTextPadding, sizeof(m_rcTextPadding));

	m_ListInfo.nColumns = 0;
	m_ListInfo.nFont = -1;
	m_ListInfo.uTextStyle = DT_VCENTER;
	m_ListInfo.dwTextColor = 0xFF000000;
	m_ListInfo.dwBkColor = 0;
	m_ListInfo.bAlternateBk = false;
	m_ListInfo.dwSelectedTextColor = 0xFF000000;
	m_ListInfo.dwSelectedBkColor = 0xFFC1E3FF;
	m_ListInfo.dwHotTextColor = 0xFF000000;
	m_ListInfo.dwHotBkColor = 0xFFE9F5FF;
	m_ListInfo.dwDisabledTextColor = 0xFFCCCCCC;
	m_ListInfo.dwDisabledBkColor = 0xFFFFFFFF;
	m_ListInfo.dwLineColor = 0;
	m_ListInfo.bShowHtml = false;
	m_ListInfo.bMultiExpandable = false;
	::ZeroMemory(&m_ListInfo.rcTextPadding, sizeof(m_ListInfo.rcTextPadding));
	::ZeroMemory(&m_ListInfo.rcColumn, sizeof(m_ListInfo.rcColumn));

	m_nHidenItems = 0;
	m_uArrowWith = 20;
	m_bShowDropBtn = false;
	m_uDropBtnWidth = 20;
	m_ArrowBtnSize.cx = m_ArrowBtnSize.cy = 0;
	m_DropBtnSize.cx = m_DropBtnSize.cy = 0;
	m_nItemHeight = 20;
}

CURLBar::~CURLBar(void)
{
	RemoveAll();
}

LPCTSTR CURLBar::GetClass() const
{
	return _T("URLBar");
}

LPVOID CURLBar::GetInterface(LPCTSTR pstrName)
{
	if( _tcscmp(pstrName, _T("URLBar")) == 0 ) return static_cast<CURLBar*>(this);
	return CHorizontalLayoutUI::GetInterface(pstrName);
}

UINT CURLBar::GetControlFlags() const
{
	return UIFLAG_TABSTOP;
}

int CURLBar::GetCurSel() const
{
	return m_iCurSel;
}

bool CURLBar::SelectItem(int iIndex, bool bTakeFocus /* = false */)
{
	//    if( m_pWindow != NULL ) m_pWindow->Close();
	if( iIndex == m_iCurSel ) return true;
	int iOldSel = m_iCurSel;
	if( m_iCurSel >= 0 ) {
		CControlUI* pControl = static_cast<CControlUI*>(m_items[m_iCurSel]);
		if( !pControl ) return false;
		IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
		if( pListItem != NULL ) pListItem->Select(false);
		m_iCurSel = -1;
	}
	if( iIndex < 0 ) return false;
	if( m_items.GetSize() == 0 ) return false;
	if( iIndex >= m_items.GetSize() ) iIndex = m_items.GetSize() - 1;
	CControlUI* pControl = static_cast<CControlUI*>(m_items[iIndex]);
	if( !pControl || !pControl->IsVisible() || !pControl->IsEnabled() ) return false;
	IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
	if( pListItem == NULL ) return false;
	m_iCurSel = iIndex;
	if( m_pWindow != NULL && bTakeFocus)
		pControl->SetFocus();
//	pListItem->Select(true);
	if( m_pManager != NULL ) m_pManager->SendNotify(this, _T("itemselect"), DUILIB_COMBO_ITMESELECT, m_iCurSel, iOldSel);

	//自动将选中项变成最后一项
// 	for (int it = m_items.GetSize() - 1; it > m_iCurSel; it --)
// 	{
// 		RemoveAt(it);
// 	}
// 	SetPos(m_rcItem);
// 	Invalidate();
	return true;
}

bool CURLBar::SetItemIndex(CControlUI* pControl, int iIndex)
{
	int iOrginIndex = GetItemIndex(pControl);
	if( iOrginIndex == -1 ) return false;
	if( iOrginIndex == iIndex ) return true;

	IListItemUI* pSelectedListItem = NULL;
	if( m_iCurSel >= 0 ) pSelectedListItem = 
		static_cast<IListItemUI*>(GetItemAt(m_iCurSel)->GetInterface(_T("ListItem")));
	if( !CContainerUI::SetItemIndex(pControl, iIndex) ) return false;
	int iMinIndex = min(iOrginIndex, iIndex);
	int iMaxIndex = max(iOrginIndex, iIndex);
	for(int i = iMinIndex; i < iMaxIndex + 1; ++i) {
		CControlUI* p = GetItemAt(i);
		IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(_T("ListItem")));
		if( pListItem != NULL ) {
			pListItem->SetIndex(i);
		}
	}
	if( m_iCurSel >= 0 && pSelectedListItem != NULL ) m_iCurSel = pSelectedListItem->GetIndex();
	return true;
}

bool CURLBar::AddItem(CStdString strText)
{
	CURLElement *pElement = new CURLElement();
	if (pElement == NULL)
	{
		return false;
	}
	pElement->SetText(strText);
	pElement->SetOwner(this);
	pElement->SetFixedHeight(m_nItemHeight);
	pElement->SetIndex(m_items.GetSize());
	return CContainerUI::Add(pElement);
}

bool CURLBar::Add(CControlUI* pControl)
{
	IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
	if( pListItem != NULL ) 
	{
		pListItem->SetOwner(this);
		pListItem->SetIndex(m_items.GetSize());
	}
	if (pControl )
	{
		pControl->SetFixedHeight(m_nItemHeight);
	}
	return CContainerUI::Add(pControl);
}

bool CURLBar::AddAt(CControlUI* pControl, int iIndex)
{
	if (!CContainerUI::AddAt(pControl, iIndex)) return false;

	// The list items should know about us
	IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
	if( pListItem != NULL ) {
		pListItem->SetOwner(this);
		pListItem->SetIndex(iIndex);
	}

	for(int i = iIndex + 1; i < GetCount(); ++i) {
		CControlUI* p = GetItemAt(i);
		pListItem = static_cast<IListItemUI*>(p->GetInterface(_T("ListItem")));
		if( pListItem != NULL ) {
			pListItem->SetIndex(i);
		}
	}
	if( m_iCurSel >= iIndex ) m_iCurSel += 1;
	return true;
}

bool CURLBar::Remove(CControlUI* pControl)
{
	int iIndex = GetItemIndex(pControl);
	if (iIndex == -1) return false;

	if (!CContainerUI::RemoveAt(iIndex)) return false;

	for(int i = iIndex; i < GetCount(); ++i) {
		CControlUI* p = GetItemAt(i);
		IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(_T("ListItem")));
		if( pListItem != NULL ) {
			pListItem->SetIndex(i);
		}
	}

	if( iIndex == m_iCurSel && m_iCurSel >= 0 ) {
		int iSel = m_iCurSel;
		m_iCurSel = -1;
		SelectItem(FindSelectable(iSel, false));
	}
	else if( iIndex < m_iCurSel ) m_iCurSel -= 1;
	return true;
}

bool CURLBar::RemoveAt(int iIndex)
{
	if (!CContainerUI::RemoveAt(iIndex)) 
		return false;

	for(int i = iIndex; i < GetCount(); ++i) {
		CControlUI* p = GetItemAt(i);
		IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(_T("ListItem")));
		if( pListItem != NULL ) pListItem->SetIndex(i);
	}

	if( iIndex == m_iCurSel && m_iCurSel >= 0 ) {
		int iSel = m_iCurSel;
		m_iCurSel = -1;
		SelectItem(FindSelectable(iSel, false));
	}
	else if( iIndex < m_iCurSel ) m_iCurSel -= 1;
	return true;
}

void CURLBar::RemoveAll()
{
	m_iCurSel = -1;
	CContainerUI::RemoveAll();
}

void CURLBar::DoEvent(TEventUI& event)
{
	if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
		if( m_pParent != NULL ) m_pParent->DoEvent(event);
		else CContainerUI::DoEvent(event);
		return;
	}

	if( event.Type == UIEVENT_SETFOCUS ) 
	{
		Invalidate();
	}
	if( event.Type == UIEVENT_KILLFOCUS ) 
	{
		Invalidate();
	}
	if( event.Type == UIEVENT_BUTTONDOWN )
	{
		if( IsEnabled() && m_bShowDropBtn ) {
			if (::PtInRect(&m_rcDropBtn, event.ptMouse))
			{
				Activate();
				m_uButtonState |= UISTATE_PUSHED | UISTATE_CAPTURED;
			}
		}
		return;
	}
	if( event.Type == UIEVENT_BUTTONUP )
	{
		if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
			m_uButtonState &= ~ UISTATE_CAPTURED;
			Invalidate();
		}
		return;
	}
	if( event.Type == UIEVENT_MOUSEMOVE )
	{
		return;
	}
	if( event.Type == UIEVENT_KEYDOWN )
	{
		switch( event.chKey ) {
		case VK_F4:
			Activate();
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
	if( event.Type == UIEVENT_SCROLLWHEEL )
	{
		bool bDownward = LOWORD(event.wParam) == SB_LINEDOWN;
		//        SelectItem(FindSelectable(m_iCurSel + (bDownward ? 1 : -1), bDownward));
		if (bDownward)
			LineDown();
		else 
			LineUp();
		return;
	}
	if( event.Type == UIEVENT_CONTEXTMENU )
	{
		return;
	}
	if( event.Type == UIEVENT_MOUSEENTER )
	{
		if( ::PtInRect(&m_rcItem, event.ptMouse ) ) {
			if( (m_uButtonState & UISTATE_HOT) == 0  )
				m_uButtonState |= UISTATE_HOT;
			Invalidate();
		}
		return;
	}
	if( event.Type == UIEVENT_MOUSELEAVE )
	{
		if( (m_uButtonState & UISTATE_HOT) != 0 ) {
			m_uButtonState &= ~UISTATE_HOT;
			Invalidate();
		}
		return;
	}
	CControlUI::DoEvent(event);
}

SIZE CURLBar::EstimateSize(SIZE szAvailable)
{
	if( m_cxyFixed.cy == 0 ) return CSize(m_cxyFixed.cx, m_pManager->GetDefaultFontInfo()->tm.tmHeight + 12);
	return CControlUI::EstimateSize(szAvailable);
}

bool CURLBar::Activate()
{
	if( !CControlUI::Activate() ) return false;
	if( m_pWindow ) return true;
	m_pWindow = new CComboWndEx();
	ASSERT(m_pWindow);
	m_pWindow->Init(this);
	if( m_pManager != NULL ) 
		m_pManager->SendNotify(this, _T("dropdown"), DUILIB_COMBO_DROPDOWN);
	Invalidate();
	return true;
}

CStdString CURLBar::GetText() const
{
	if( m_iCurSel < 0 ) return _T("");
	CControlUI* pControl = static_cast<CControlUI*>(m_items[m_iCurSel]);
	return pControl->GetText();
}

void CURLBar::SetEnabled(bool bEnable)
{
	CContainerUI::SetEnabled(bEnable);
	if( !IsEnabled() ) m_uButtonState = 0;
}

CStdString CURLBar::GetDropBoxAttributeList()
{
	return m_sDropBoxAttributes;
}

void CURLBar::SetDropBoxAttributeList(LPCTSTR pstrList)
{
	m_sDropBoxAttributes = pstrList;
}

SIZE CURLBar::GetDropBoxSize() const
{
	return m_szDropBox;
}

void CURLBar::SetDropBoxSize(SIZE szDropBox)
{
	m_szDropBox = szDropBox;
}

RECT CURLBar::GetTextPadding() const
{
	return m_rcTextPadding;
}

void CURLBar::SetTextPadding(RECT rc)
{
	m_rcTextPadding = rc;
	Invalidate();
}

LPCTSTR CURLBar::GetNormalImage() const
{
	return m_sNormalImage;
}

void CURLBar::SetNormalImage(LPCTSTR pStrImage)
{
	m_sNormalImage = pStrImage;
	Invalidate();
}

LPCTSTR CURLBar::GetHotImage() const
{
	return m_sHotImage;
}

void CURLBar::SetHotImage(LPCTSTR pStrImage)
{
	m_sHotImage = pStrImage;
	Invalidate();
}

LPCTSTR CURLBar::GetPushedImage() const
{
	return m_sPushedImage;
}

void CURLBar::SetPushedImage(LPCTSTR pStrImage)
{
	m_sPushedImage = pStrImage;
	Invalidate();
}

LPCTSTR CURLBar::GetFocusedImage() const
{
	return m_sFocusedImage;
}

void CURLBar::SetFocusedImage(LPCTSTR pStrImage)
{
	m_sFocusedImage = pStrImage;
	Invalidate();
}

LPCTSTR CURLBar::GetDisabledImage() const
{
	return m_sDisabledImage;
}

void CURLBar::SetDisabledImage(LPCTSTR pStrImage)
{
	m_sDisabledImage = pStrImage;
	Invalidate();
}

TListInfoUI* CURLBar::GetListInfo()
{
	return &m_ListInfo;
}

void CURLBar::SetItemFont(int index)
{
	m_ListInfo.nFont = index;
	Invalidate();
}

void CURLBar::SetItemTextStyle(UINT uStyle)
{
	m_ListInfo.uTextStyle = uStyle;
	Invalidate();
}

RECT CURLBar::GetItemTextPadding() const
{
	return m_ListInfo.rcTextPadding;
}

void CURLBar::SetItemTextPadding(RECT rc)
{
	m_ListInfo.rcTextPadding = rc;
	Invalidate();
}

void CURLBar::SetItemTextColor(DWORD dwTextColor)
{
	m_ListInfo.dwTextColor = dwTextColor;
	Invalidate();
}

void CURLBar::SetItemBkColor(DWORD dwBkColor)
{
	m_ListInfo.dwBkColor = dwBkColor;
}

void CURLBar::SetItemBkImage(LPCTSTR pStrImage)
{
	m_ListInfo.sBkImage = pStrImage;
}

DWORD CURLBar::GetItemTextColor() const
{
	return m_ListInfo.dwTextColor;
}

DWORD CURLBar::GetItemBkColor() const
{
	return m_ListInfo.dwBkColor;
}

LPCTSTR CURLBar::GetItemBkImage() const
{
	return m_ListInfo.sBkImage;
}

bool CURLBar::IsAlternateBk() const
{
	return m_ListInfo.bAlternateBk;
}

void CURLBar::SetAlternateBk(bool bAlternateBk)
{
	m_ListInfo.bAlternateBk = bAlternateBk;
}

void CURLBar::SetSelectedItemTextColor(DWORD dwTextColor)
{
	m_ListInfo.dwSelectedTextColor = dwTextColor;
}

void CURLBar::SetSelectedItemBkColor(DWORD dwBkColor)
{
	m_ListInfo.dwSelectedBkColor = dwBkColor;
}

void CURLBar::SetSelectedItemImage(LPCTSTR pStrImage)
{
	m_ListInfo.sSelectedImage = pStrImage;
}

DWORD CURLBar::GetSelectedItemTextColor() const
{
	return m_ListInfo.dwSelectedTextColor;
}

DWORD CURLBar::GetSelectedItemBkColor() const
{
	return m_ListInfo.dwSelectedBkColor;
}

LPCTSTR CURLBar::GetSelectedItemImage() const
{
	return m_ListInfo.sSelectedImage;
}

void CURLBar::SetHotItemTextColor(DWORD dwTextColor)
{
	m_ListInfo.dwHotTextColor = dwTextColor;
}

void CURLBar::SetHotItemBkColor(DWORD dwBkColor)
{
	m_ListInfo.dwHotBkColor = dwBkColor;
}

void CURLBar::SetHotItemImage(LPCTSTR pStrImage)
{
	m_ListInfo.sHotImage = pStrImage;
}

DWORD CURLBar::GetHotItemTextColor() const
{
	return m_ListInfo.dwHotTextColor;
}
DWORD CURLBar::GetHotItemBkColor() const
{
	return m_ListInfo.dwHotBkColor;
}

LPCTSTR CURLBar::GetHotItemImage() const
{
	return m_ListInfo.sHotImage;
}

void CURLBar::SetDisabledItemTextColor(DWORD dwTextColor)
{
	m_ListInfo.dwDisabledTextColor = dwTextColor;
}

void CURLBar::SetDisabledItemBkColor(DWORD dwBkColor)
{
	m_ListInfo.dwDisabledBkColor = dwBkColor;
}

void CURLBar::SetDisabledItemImage(LPCTSTR pStrImage)
{
	m_ListInfo.sDisabledImage = pStrImage;
}

DWORD CURLBar::GetDisabledItemTextColor() const
{
	return m_ListInfo.dwDisabledTextColor;
}

DWORD CURLBar::GetDisabledItemBkColor() const
{
	return m_ListInfo.dwDisabledBkColor;
}

LPCTSTR CURLBar::GetDisabledItemImage() const
{
	return m_ListInfo.sDisabledImage;
}

DWORD CURLBar::GetItemLineColor() const
{
	return m_ListInfo.dwLineColor;
}

void CURLBar::SetItemLineColor(DWORD dwLineColor)
{
	m_ListInfo.dwLineColor = dwLineColor;
}

bool CURLBar::IsItemShowHtml()
{
	return m_ListInfo.bShowHtml;
}

void CURLBar::SetItemShowHtml(bool bShowHtml)
{
	if( m_ListInfo.bShowHtml == bShowHtml ) return;

	m_ListInfo.bShowHtml = bShowHtml;
	Invalidate();
}

void CURLBar::SetPos(RECT rc)
{
	// Put all elements out of sight
    CControlUI::SetPos(rc);
	CalHideItems();

	RECT rcNull = { 0 };
	for( int i = 0; i < m_nHidenItems; i++ ) 
		static_cast<CControlUI*>(m_items[i])->SetPos(rcNull);

	// Position this control
	rc = m_rcItem;

	// Adjust for inset
	rc.left += m_rcInset.left;
	rc.top += m_rcInset.top;
	rc.right -= m_rcInset.right;
	rc.bottom -= m_rcInset.bottom;

	if( m_items.GetSize() == 0) {
		ProcessScrollBar(rc, 0, 0);
		return;
	}

	if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) rc.right -= m_pVerticalScrollBar->GetFixedWidth();
	if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();

	// Determine the width of elements that are sizeable
	SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };
	if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) 
		szAvailable.cx += m_pHorizontalScrollBar->GetScrollRange();

	int nAdjustables = 0;
	int cxFixed = 0;
	int nEstimateNum = 0;
	for( int it1 = m_nHidenItems; it1 < m_items.GetSize(); it1++ ) {
		CControlUI* pControl = static_cast<CControlUI*>(m_items[it1]);
		if( !pControl->IsVisible() ) continue;
		if( pControl->IsFloat() ) continue;
		SIZE sz = pControl->EstimateSize(szAvailable);
		if( sz.cx == 0 ) {
			nAdjustables++;
		}
		else {
			if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();
			if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();
		}
		cxFixed += sz.cx +  pControl->GetPadding().left + pControl->GetPadding().right;
		nEstimateNum++;
	}
	cxFixed += (nEstimateNum - 1) * m_iChildPadding;

	int cxExpand = 0;
	if( nAdjustables > 0 ) cxExpand = MAX(0, (szAvailable.cx - cxFixed) / nAdjustables);
	// Position the elements
	SIZE szRemaining = szAvailable;
	int iPosX = rc.left;
	m_rcDropBtn = rc;
	m_rcDropBtn.right = rc.left;
	if (m_bShowDropBtn)
	{
		iPosX += m_uDropBtnWidth;
		m_rcDropBtn.right = rc.left + m_uDropBtnWidth  + GetPadding().left;
		if (m_DropBtnSize.cy != 0)
		{
			int nlen = rc.bottom - rc.top  - m_DropBtnSize.cy;
			if (nlen > 0)
			{
				m_rcDropBtn.top = rc.top + (rc.bottom - rc.top  - m_DropBtnSize.cy) /2;
				m_rcDropBtn.bottom = m_rcDropBtn.top + m_DropBtnSize.cy;
			}
		}
	}
	if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) {
		iPosX -= m_pHorizontalScrollBar->GetScrollPos();
	}
	int iAdjustable = 0;
	int cxFixedRemaining = cxFixed;
	for( int it2 = m_nHidenItems; it2 < m_items.GetSize(); it2++ ) {
		CControlUI* pControl = static_cast<CControlUI*>(m_items[it2]);
		if( !pControl->IsVisible() ) continue;
		if( pControl->IsFloat() ) {
			SetFloatPos(it2);
			continue;
		}
		RECT rcPadding = pControl->GetPadding();
		szRemaining.cx -= rcPadding.left;
		SIZE sz = pControl->EstimateSize(szRemaining);
		if( sz.cx == 0 ) {
			iAdjustable++;
			sz.cx = cxExpand;
			// Distribute remaining to last element (usually round-off left-overs)
			if( iAdjustable == nAdjustables ) {
				sz.cx = MAX(0, szRemaining.cx - rcPadding.right - cxFixedRemaining);
			}
			if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();
			if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();
		}
		else {
			if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();
			if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();

			cxFixedRemaining -= sz.cx;
		}

		sz.cy = pControl->GetFixedHeight();
		if( sz.cy == 0 ) sz.cy = rc.bottom - rc.top - rcPadding.top - rcPadding.bottom;
		if( sz.cy < 0 ) sz.cy = 0;
		if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
		if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();
		RECT rcCtrl = { iPosX + rcPadding.left, rc.top + rcPadding.top, iPosX + sz.cx + rcPadding.left + rcPadding.right, rc.top + rcPadding.top + sz.cy};
		IntersectRect(&rcCtrl, &rcCtrl, &m_rcItem);
		pControl->SetPos(rcCtrl);
		iPosX += sz.cx + m_iChildPadding + rcPadding.left + rcPadding.right;
		szRemaining.cx -= sz.cx + m_iChildPadding + rcPadding.right;
	}

	// Process the scrollbar
	ProcessScrollBar(rc, 0, 0);
}

void CURLBar::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if( _tcscmp(pstrName, _T("textpadding")) == 0 ) {
		RECT rcTextPadding = { 0 };
		LPTSTR pstr = NULL;
		rcTextPadding.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
		rcTextPadding.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
		rcTextPadding.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
		rcTextPadding.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);    
		SetTextPadding(rcTextPadding);
	}
	else if(_tcscmp(pstrName, _T("dropboxsize")) == 0)    //add by lighten 
	{
		SIZE szbox = {0};
		LPTSTR pstr = NULL;
		szbox.cx = _tcstol(pstrValue, &pstr, 10);
		ASSERT(pstr);
		szbox.cy = _tcstol(pstr + 1, &pstr, 10);
		SetDropBoxSize(szbox);
	}
	else if (_tcscmp(pstrName, _T("arrowbtnsize")) == 0)
	{
		SIZE szbox = {0};
		LPTSTR pstr = NULL;
		szbox.cx = _tcstol(pstrValue, &pstr, 10);
		ASSERT(pstr);
		szbox.cy = _tcstol(pstr + 1, &pstr, 10);
		SetArrowBtnSize(szbox);
	}
	else if (_tcscmp(pstrName, _T("dropbtnsize")) == 0)
	{
		SIZE szbox = {0};
		LPTSTR pstr = NULL;
		szbox.cx = _tcstol(pstrValue, &pstr, 10);
		ASSERT(pstr);
		szbox.cy = _tcstol(pstr + 1, &pstr, 10);
		SetDropBtnSize(szbox);
	}
	else if (_tcscmp(pstrName, _T("arrow_normalimage")) == 0 ) SetArrowBtnNormalImage(pstrValue);
	else if (_tcscmp(pstrName, _T("arrow_width")) == 0 )
	{
		LPTSTR pstr = NULL;
		m_uArrowWith = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
	}
	else if (_tcscmp(pstrName, _T("itemheight")) == 0 )
	{
		LPTSTR pstr = NULL;
		int nHeight = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
		SetItemHeight(nHeight);
	}
	else if (_tcscmp(pstrName, _T("arrow_width")) == 0 )
	{
		LPTSTR pstr = NULL;
		m_uArrowWith = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
	}
	else if( _tcscmp(pstrName, _T("normalimage")) == 0 ) SetNormalImage(pstrValue);
	else if( _tcscmp(pstrName, _T("hotimage")) == 0 ) SetHotImage(pstrValue);
	else if( _tcscmp(pstrName, _T("pushedimage")) == 0 ) SetPushedImage(pstrValue);
	else if( _tcscmp(pstrName, _T("focusedimage")) == 0 ) SetFocusedImage(pstrValue);
	else if( _tcscmp(pstrName, _T("disabledimage")) == 0 ) SetDisabledImage(pstrValue);
	else if( _tcscmp(pstrName, _T("dropbox")) == 0 ) SetDropBoxAttributeList(pstrValue);
	else if( _tcscmp(pstrName, _T("itemfont")) == 0 ) m_ListInfo.nFont = _ttoi(pstrValue);
	else if( _tcscmp(pstrName, _T("itemalign")) == 0 ) {
		if( _tcsstr(pstrValue, _T("left")) != NULL ) {
			m_ListInfo.uTextStyle &= ~(DT_CENTER | DT_RIGHT);
			m_ListInfo.uTextStyle |= DT_LEFT;
		}
		if( _tcsstr(pstrValue, _T("center")) != NULL ) {
			m_ListInfo.uTextStyle &= ~(DT_LEFT | DT_RIGHT);
			m_ListInfo.uTextStyle |= DT_CENTER;
		}
		if( _tcsstr(pstrValue, _T("right")) != NULL ) {
			m_ListInfo.uTextStyle &= ~(DT_LEFT | DT_CENTER);
			m_ListInfo.uTextStyle |= DT_RIGHT;
		}
		if( _tcsstr(pstrValue, _T("top")) != NULL ) {
			m_ListInfo.uTextStyle &= ~(DT_VCENTER | DT_BOTTOM);
			m_ListInfo.uTextStyle |= DT_TOP;
		}
		if( _tcsstr(pstrValue, _T("bottom")) != NULL ) {
			m_ListInfo.uTextStyle &= ~(DT_VCENTER | DT_TOP);
			m_ListInfo.uTextStyle |= DT_BOTTOM;
		}
	}
	else if( _tcscmp(pstrName, _T("itemendellipsis")) == 0 ) {
		if( _tcscmp(pstrValue, _T("true")) == 0 )
		{
			m_ListInfo.uTextStyle &= ~DT_PATH_ELLIPSIS;
			m_ListInfo.uTextStyle |= DT_END_ELLIPSIS;
		}
		else m_ListInfo.uTextStyle &= ~DT_END_ELLIPSIS;
	}    
	if( _tcscmp(pstrName, _T("itemtextpadding")) == 0 ) {
		RECT rcTextPadding = { 0 };
		LPTSTR pstr = NULL;
		rcTextPadding.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
		rcTextPadding.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
		rcTextPadding.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
		rcTextPadding.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);    
		SetItemTextPadding(rcTextPadding);
	}
	else if( _tcscmp(pstrName, _T("itemtextcolor")) == 0 ) {
		if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		LPTSTR pstr = NULL;
		DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
		SetItemTextColor(clrColor);
	}
	else if( _tcscmp(pstrName, _T("itembkcolor")) == 0 ) {
		if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		LPTSTR pstr = NULL;
		DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
		SetItemBkColor(clrColor);
	}
	else if( _tcscmp(pstrName, _T("itembkimage")) == 0 ) SetItemBkImage(pstrValue);
	else if( _tcscmp(pstrName, _T("itemaltbk")) == 0 ) SetAlternateBk(_tcscmp(pstrValue, _T("true")) == 0);
	else if( _tcscmp(pstrName, _T("itemselectedtextcolor")) == 0 ) {
		if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		LPTSTR pstr = NULL;
		DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
		SetSelectedItemTextColor(clrColor);
	}
	else if( _tcscmp(pstrName, _T("itemselectedbkcolor")) == 0 ) {
		if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		LPTSTR pstr = NULL;
		DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
		SetSelectedItemBkColor(clrColor);
	}
	else if( _tcscmp(pstrName, _T("itemselectedimage")) == 0 ) SetSelectedItemImage(pstrValue);
	else if( _tcscmp(pstrName, _T("itemhottextcolor")) == 0 ) {
		if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		LPTSTR pstr = NULL;
		DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
		SetHotItemTextColor(clrColor);
	}
	else if( _tcscmp(pstrName, _T("itemhotbkcolor")) == 0 ) {
		if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		LPTSTR pstr = NULL;
		DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
		SetHotItemBkColor(clrColor);
	}
	else if( _tcscmp(pstrName, _T("itemhotimage")) == 0 ) SetHotItemImage(pstrValue);
	else if( _tcscmp(pstrName, _T("itemdisabledtextcolor")) == 0 ) {
		if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		LPTSTR pstr = NULL;
		DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
		SetDisabledItemTextColor(clrColor);
	}
	else if( _tcscmp(pstrName, _T("itemdisabledbkcolor")) == 0 ) {
		if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		LPTSTR pstr = NULL;
		DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
		SetDisabledItemBkColor(clrColor);
	}
	else if( _tcscmp(pstrName, _T("itemdisabledimage")) == 0 ) SetDisabledItemImage(pstrValue);
	else if( _tcscmp(pstrName, _T("itemlinecolor")) == 0 ) {
		if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		LPTSTR pstr = NULL;
		DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
		SetItemLineColor(clrColor);
	}
	else if( _tcscmp(pstrName, _T("itemshowhtml")) == 0 ) SetItemShowHtml(_tcscmp(pstrValue, _T("true")) == 0);
	else CContainerUI::SetAttribute(pstrName, pstrValue);
}

void CURLBar::DoPaint(HDC hDC, const RECT& rcPaint)
{
	CControlUI::DoPaint(hDC, rcPaint);
	for (int it1 = m_nHidenItems; it1 < m_items.GetSize(); it1++)
	{
		CControlUI* pControl = static_cast<CControlUI*>(m_items[it1]);
		if( !pControl->IsVisible() ) continue;
		if( pControl->IsFloat() ) continue;
		pControl->DoPaint(hDC, rcPaint);
	}
}

void CURLBar::PaintStatusImage(HDC hDC)
{
	if ( m_dwBackColor != 0 ) {
		CRenderEngine::DrawColor(hDC, m_pManager, m_rcItem, GetAdjustColor(m_dwBackColor));
	}

	if( !m_sBkImage.IsEmpty() ) {
		if( !DrawImage(hDC, (LPCTSTR)m_sBkImage) ) m_sBkImage.Empty();
	}
	
	if (!m_bShowDropBtn)
	{
		return;
	}

	if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
	else m_uButtonState &= ~ UISTATE_FOCUSED;
	if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
	else m_uButtonState &= ~ UISTATE_DISABLED;

	if( (m_uButtonState & UISTATE_DISABLED) != 0 ) {
		if( !m_sDisabledImage.IsEmpty() ) {
			if( !DrawImage(hDC, (LPCTSTR)m_sDisabledImage, m_rcDropBtn) ) m_sDisabledImage.Empty();
			else return;
		}
	}
	else if( (m_uButtonState & UISTATE_PUSHED) != 0 ) {
		if( !m_sPushedImage.IsEmpty() ) {
			if( !DrawImage(hDC, (LPCTSTR)m_sPushedImage, m_rcDropBtn) ) m_sPushedImage.Empty();
			else return;
		}
	}
	else if( (m_uButtonState & UISTATE_HOT) != 0 ) {
		if( !m_sHotImage.IsEmpty() ) {
			if( !DrawImage(hDC, (LPCTSTR)m_sHotImage, m_rcDropBtn) ) m_sHotImage.Empty();
			else return;
		}
	}
	else if( (m_uButtonState & UISTATE_FOCUSED) != 0 ) {
		if( !m_sFocusedImage.IsEmpty() ) {
			if( !DrawImage(hDC, (LPCTSTR)m_sFocusedImage, m_rcDropBtn) ) m_sFocusedImage.Empty();
			else return;
		}
	}

	if( !m_sNormalImage.IsEmpty() ) {
		if( !DrawImage(hDC, (LPCTSTR)m_sNormalImage, m_rcDropBtn) ) m_sNormalImage.Empty();
		else return;
	}
}

void CURLBar::PaintText(HDC hDC)
{
// 	RECT rcText = m_rcItem;
// 	rcText.left += m_rcTextPadding.left;
// 	rcText.right -= m_rcTextPadding.right;
// 	rcText.top += m_rcTextPadding.top;
// 	rcText.bottom -= m_rcTextPadding.bottom;
// 
// 	if( m_iCurSel >= 0 ) {
// 		CControlUI* pControl = static_cast<CControlUI*>(m_items[m_iCurSel]);
// 		IListItemUI* pElement = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
// 		if( pElement != NULL ) {
// 			pElement->DrawItemText(hDC, rcText);
// 		}
// 		else {
// 			RECT rcOldPos = pControl->GetPos();
// 			pControl->SetPos(rcText);
// 			pControl->DoPaint(hDC, rcText);
// 			pControl->SetPos(rcOldPos);
// 		}
// 	}
}

//获取需要隐藏的项目数
int CURLBar::GetHidenItems() const
{
	return m_nHidenItems;
}

//计算需要隐藏的项目数
void CURLBar::CalHideItems()
{
	if (m_items.GetSize() <= 1)   //只有一项时，认识时候都显示
	{
		int nItemWidth = GetItemNeedLen(m_items.GetSize() - 1);
		m_bShowDropBtn = false;
		m_nHidenItems = 0;
		return;
	}
	
	int nWidth = m_rcItem.right - m_rcItem.left;
	int nSize = m_items.GetSize() - 1;
	int nItemWidth = 0;
	nItemWidth = GetItemNeedLen(nSize);
	nWidth -= nItemWidth;
	nSize --;
	int nTemp = nWidth;
	while ((nSize >= 0) && (nWidth > 0))
	{
		nItemWidth = GetItemNeedLen(nSize);
		nWidth -= nItemWidth;
		if (nWidth > 0)
		{
			nWidth -= m_uArrowWith;
		}
		if ((nWidth > 0)&& (nSize >=0))
		{
			SetItemNeedLen(nSize, nItemWidth + m_uArrowWith);
			nTemp = nWidth;
			nSize --;
		}
	}
	
	if (nWidth > 0)
	{
		m_bShowDropBtn = false;
		m_nHidenItems = 0;
		return;
	}
	else
	{
		m_bShowDropBtn = true;
		m_nHidenItems = nSize + 1;
	}

	//较正显示的数据
	if (m_nHidenItems ==  m_items.GetSize() - 2)   //如果是只显示一项，一定显示
	{
		return ;
	}
	
	if (nTemp < m_uDropBtnWidth)   //剩下的空间不能够完全显示下拉按钮
	{
		GetItemNeedLen(m_nHidenItems);
		m_nHidenItems ++;
	}
	for (int i = 0; i < m_nHidenItems; i++)
	{
		GetItemNeedLen(i);
	}
}

//获取指定的项显示需要的长度
int CURLBar::GetItemNeedLen(int nIndex)
{	
	if (nIndex < 0)
	{
		return 0;
	}
	if (nIndex >= m_items.GetSize())
	{
		return 0;
	}

	CControlUI *pControl = static_cast<CControlUI *>(m_items.GetAt(nIndex));
	if (pControl == NULL)
	{
		return 0;
	}
	CURLElement *pElement = static_cast<CURLElement *>(pControl->GetInterface(_T("URLElment")));
	if (pElement == NULL)
	{
		return 0;
	}
	return pElement->GetControlNeedLen();
}

//获取指定的项显示需要的长度
void CURLBar::SetItemNeedLen(int nIndex, int nLen)
{	
	if (nLen <= 0)
	{
		return;
	}
	if (nIndex < 0)
	{
		return ;
	}
	if (nIndex >= m_items.GetSize())
	{
		return ;
	}

	CControlUI *pControl = static_cast<CControlUI *>(m_items.GetAt(nIndex));
	if (pControl == NULL)
	{
		return ;
	}
	CURLElement *pElement = static_cast<CURLElement *>(pControl->GetInterface(_T("URLElment")));
	if (pElement == NULL)
	{
		return ;
	}
	return pElement->SetControlNeedLen(nLen);
}


void CURLBar::SetArrowBtnNormalImage(CStdString strNormalImage)   //设置箭头按钮普通状态
{
	m_strImageArrowBtn_Normal = strNormalImage;
}

CStdString CURLBar::GetArrowBtnNormalImage() const                //获取箭头按钮普通状态
{
	return m_strImageArrowBtn_Normal;
}

void CURLBar::SetArrowBtnSize(SIZE szBtn)                         //设置指向按钮大小
{
	m_ArrowBtnSize = szBtn;
}

SIZE CURLBar::GetArrowBtnSize() const                             //获取指向按钮大小
{
	return m_ArrowBtnSize;
}

UINT CURLBar::GetArrowWidth() const
{
	return m_uArrowWith;
}

void CURLBar::SetDropBtnSize(SIZE szBtn)                          //设置下拉按钮的大小 
{
	m_DropBtnSize = szBtn;
	m_uDropBtnWidth = szBtn.cx;
}

SIZE CURLBar::GetDropBtnSize() const                              //获取下拉按钮的大小 
{
	return m_DropBtnSize;
}

void CURLBar::SetItemHeight(int nHeight)           //设置项目高度
{
	if (nHeight > 0)
	{
		m_nItemHeight = nHeight;
		for (int i = 0; i< m_items.GetSize(); i++)
		{
			CControlUI *pControl = static_cast<CControlUI *>(m_items.GetAt(i));
			if (pControl)
			{
				pControl->SetFixedHeight(m_nItemHeight);
			}
		}
	}
}

int CURLBar::GetItemHeight() const                 //获取项目高度
{
	return m_nItemHeight;
}


CURLElement::CURLElement()
{
	m_uTextNeedLen = 0;
	m_bSelected = false;
	m_iIndex = -1;
	m_pOwner = NULL;
	m_uButtonState = 0;
	m_bTextChanged = false;
	m_uControlNeedLen = 0;
	m_rcBtnPos.Empty();
}

LPCTSTR CURLElement::GetClass() const
{
	return _T("URLBar");
}

LPVOID CURLElement::GetInterface(LPCTSTR pstrName)
{
	if( _tcscmp(pstrName, _T("URLElment")) == 0 ) return static_cast<CURLElement*>(this);
	if( _tcscmp(pstrName, _T("ListItem")) == 0 ) return static_cast<IListItemUI*>(this);
	return CButtonUI::GetInterface(pstrName);
}

UINT CURLElement::GetControlFlags() const
{
	return UIFLAG_TABSTOP;
}

CURLBar* CURLElement::GetOwner()
{
	return m_pOwner;
}

void CURLElement::SetOwner(CControlUI* pOwner)
{
	m_pOwner = static_cast<CURLBar*>(pOwner->GetInterface(_T("URLBar")));
}

void CURLElement::SetVisible(bool bVisible)
{
	if( !IsVisible() && m_bSelected)
	{
		m_bSelected = false;
		if( m_pOwner != NULL ) m_pOwner->SelectItem(-1);
	}
}

void CURLElement::SetPos(RECT rc)
{
	CControlUI::SetPos(rc);
	m_rcBtnPos.Empty();
	if (m_uTextNeedLen == m_uControlNeedLen)
	{
		return;
	}
	if (m_pOwner == NULL)
	{
		return;
	}
	RECT rcParent = m_pOwner->GetPos();  //父窗口的大小比较，是否需要按钮
	if (m_rcItem.right >= rcParent.right)
	{
		return;
	}

	SIZE szArrowBtn = m_pOwner->GetArrowBtnSize();
	RECT rcTemp = m_rcItem;

	rcTemp.left += m_uTextNeedLen;
	m_rcBtnPos = rcTemp;
	if (szArrowBtn.cx != 0)
	{
		int nLen = rcTemp.right - rcTemp.left - szArrowBtn.cx;
		if (nLen > 0)
		{
			m_rcBtnPos.left = rcTemp.left + nLen/2;
			m_rcBtnPos.right = m_rcBtnPos.left + szArrowBtn.cx;
		}
	}
	if (szArrowBtn.cy != 0)
	{
		int nlen = rcTemp.bottom - rcTemp.top - szArrowBtn.cy;
		if (nlen > 0)
		{
			m_rcBtnPos.top = rcTemp.top + nlen/2;
			m_rcBtnPos.bottom = m_rcBtnPos.top + szArrowBtn.cy;
		}
	}
}

void CURLElement::SetEnabled(bool bEnable)
{
	CControlUI::SetEnabled(bEnable);
	if( !IsEnabled() ) {
		m_uButtonState = 0;
	}
}


SIZE CURLElement::EstimateSize(SIZE szAvailable)
{
	SIZE cXY = {0, 0};
	if( m_pOwner == NULL ) return cXY;
	TListInfoUI* pInfo = m_pOwner->GetListInfo();
	if (pInfo == NULL)
	{
		return cXY;
	}
	CRect rcTextPadding = m_pOwner->GetTextPadding();

	//计算字符需要的长度
	RECT rcText = { 0, 0, MAX(szAvailable.cx, m_cxyFixed.cx), 9999 };
	HWND hDestopWnd = ::GetDesktopWindow();
	if (hDestopWnd != NULL)
	{
		HDC hdesktopDc = ::GetDC(hDestopWnd);   // 使用桌面窗口上下文句柄，计算文本大小
		CRenderEngine::DrawText(hdesktopDc, m_pManager, rcText, m_sText, pInfo->dwTextColor, pInfo->nFont, DT_CALCRECT | pInfo->uTextStyle);
		::ReleaseDC(hDestopWnd, hdesktopDc);
		cXY.cx = rcText.right - rcText.left + rcTextPadding.left + rcTextPadding.right;
		cXY.cy = rcText.bottom - rcText.left + rcTextPadding.top + rcTextPadding.bottom;
	}
//	CRenderEngine::DrawText(m_pManager->GetPaintDC(), m_pManager, rcText, m_sText, pInfo->dwTextColor, pInfo->nFont, DT_CALCRECT | pInfo->uTextStyle);
	m_uTextNeedLen = cXY.cx;
	m_uControlNeedLen = MAX(cXY.cx, m_uControlNeedLen);
	cXY.cx = m_uControlNeedLen;

	if( m_cxyFixed.cy != 0 ) cXY.cy = m_cxyFixed.cy;
	return cXY;
}

int CURLElement::GetIndex() const
{
	return m_iIndex;
}

void CURLElement::SetIndex(int iIndex)
{
	m_iIndex = iIndex;
}

bool CURLElement::Activate()
{
	if( !CControlUI::Activate() ) return false;
	if (m_pOwner)
	{
		m_pOwner->SelectItem(m_iIndex);
	}
	if( m_pManager != NULL ) m_pManager->SendNotify(this, _T("itemactivate"), DUILIB_LIST_ITEMACTIVE);
	return true;
}

bool CURLElement::IsSelected() const
{
	return m_bSelected;
}

bool CURLElement::Select(bool bSelect /* = true */, bool bInvalidate /* = true */)
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

bool CURLElement::IsExpanded() const
{
	return false;
}

bool CURLElement::Expand(bool /*bExpand = true*/)
{
	return false;
}

void CURLElement::DoEvent(TEventUI& event)
{
	if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
		if( m_pOwner != NULL ) m_pOwner->DoEvent(event);
		else CControlUI::DoEvent(event);
		return;
	}

	if( event.Type == UIEVENT_DBLCLICK )
	{
		if( ::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled() ) 
		{
			if (m_pOwner)
			{
				m_pOwner->SelectItem(m_iIndex);
			}
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
				m_pOwner->SelectItem(m_iIndex);
			}
		}
		return;
	}
	if( event.Type == UIEVENT_BUTTONUP ) 
	{
		if ((m_uButtonState & UISTATE_CAPTURED) != 0)
		{
			m_uButtonState &= ~(UISTATE_PUSHED | UISTATE_CAPTURED);
			Invalidate();
		}
		return;
	}
	if( event.Type == UIEVENT_MOUSEMOVE )
	{
		return;
	}
	if( event.Type == UIEVENT_MOUSEENTER )
	{
		if (IsEnabled())
		{
			m_uButtonState |= UISTATE_HOT;
			Invalidate();
		}
		return;
	}
	if( event.Type == UIEVENT_MOUSELEAVE )
	{
		if (IsEnabled())
		{
			m_uButtonState &= ~UISTATE_HOT;
			Invalidate();
		}
		return;
	}

	// An important twist: The list-item will send the event not to its immediate
	// parent but to the "attached" list. A list may actually embed several components
	// in its path to the item, but key-presses etc. needs to go to the actual list.
	if( m_pOwner != NULL ) m_pOwner->DoEvent(event); else CControlUI::DoEvent(event);
}

void CURLElement::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if( _tcscmp(pstrName, _T("selected")) == 0 ) Select();
	else CControlUI::SetAttribute(pstrName, pstrValue);
}

void CURLElement::DoPaint(HDC hDC, const RECT& rcPaint)
{
	if( !::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem) ) return;
	RECT rcBkItem = m_rcItem;
	if (!m_rcBtnPos.IsNull())
	{
		if (m_pOwner)
		{
			rcBkItem.right = rcBkItem.right - m_pOwner->GetArrowWidth();
		}
	}
	DrawItemBk(hDC, rcBkItem);
	DrawItemText(hDC, m_rcItem);
	m_bTextChanged = false;

	//绘制指向按钮
	if (!m_rcBtnPos.IsNull() && m_pOwner->GetArrowBtnNormalImage())
	{
		int nHideNums = m_pOwner->GetHidenItems();
		if (m_iIndex >= nHideNums)   //收起来的不绘制按钮
		{
			if( !DrawImage(hDC, (LPCTSTR)m_pOwner->GetArrowBtnNormalImage(), m_rcBtnPos) ) 
				m_pOwner->SetArrowBtnNormalImage(_T(""));
			else return;
		}
	}

}

void CURLElement::DrawItemText(HDC hDC, const RECT& rcItem)
{
	if( m_sText.IsEmpty() ) return;

	if( m_pOwner == NULL ) return;
	TListInfoUI* pInfo = m_pOwner->GetListInfo();
	int nHideNums = m_pOwner->GetHidenItems();
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
	if ((nHideNums != 0) && (m_iIndex < nHideNums))
	{
		rcText.left += pInfo->rcTextPadding.left;
		rcText.right -= pInfo->rcTextPadding.right;
		rcText.top += pInfo->rcTextPadding.top;
		rcText.bottom -= pInfo->rcTextPadding.bottom;
	}
	else
	{
		RECT rcTextPadding = m_pOwner->GetTextPadding();
		rcText.left += rcTextPadding.left;
		rcText.right -= rcTextPadding.right;
		rcText.top += rcTextPadding.top;
		rcText.bottom -= rcTextPadding.bottom;

	}
	if( pInfo->bShowHtml )
		CRenderEngine::DrawHtmlText(hDC, m_pManager, rcText, m_sText, iTextColor, \
		NULL, NULL, nLinks, DT_SINGLELINE | pInfo->uTextStyle);
	else
		CRenderEngine::DrawText(hDC, m_pManager, rcText, m_sText, iTextColor, \
		pInfo->nFont, DT_SINGLELINE | pInfo->uTextStyle);
	return;
}

void CURLElement::DrawItemBk(HDC hDC, const RECT& rcItem)
{
	ASSERT(m_pOwner);
	if( m_pOwner == NULL ) return;

	TListInfoUI* pInfo = m_pOwner->GetListInfo();
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
		CRenderEngine::DrawColor(hDC, m_pManager, rcItem, GetAdjustColor(iBackColor));
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
			else return;
		}
	}

	if( (m_uButtonState & UISTATE_HOT) != 0 ) {
		if( !pInfo->sHotImage.IsEmpty() ) {
			if( !DrawImage(hDC, (LPCTSTR)pInfo->sHotImage) ) pInfo->sHotImage.Empty();
			else return;
		}
	}

	if( !m_sBkImage.IsEmpty() ) {
			if( !DrawImage(hDC, (LPCTSTR)m_sBkImage) ) m_sBkImage.Empty();
	}

	if( m_sBkImage.IsEmpty() ) {
		if( !pInfo->sBkImage.IsEmpty() ) {
			if( !DrawImage(hDC, (LPCTSTR)pInfo->sBkImage) ) pInfo->sBkImage.Empty();
			else return;
		}
	}

	if ( pInfo->dwLineColor != 0 ) {
		RECT rcLine = { rcItem.left, rcItem.bottom - 1, rcItem.right, rcItem.bottom - 1 };
		CRenderEngine::DrawLine(hDC, rcLine, 1, GetAdjustColor(pInfo->dwLineColor));
	}

}

UINT CURLElement::GetControlNeedLen() 
{
	if( m_pOwner == NULL )
		return m_uTextNeedLen;
	TListInfoUI* pInfo = m_pOwner->GetListInfo();
	if (pInfo == NULL)
	{
		return m_uTextNeedLen;
	}
	CRect rcTextPadding = m_pOwner->GetTextPadding();

	//计算字符需要的长度
	RECT rcText = { 0, 0, 9999, 9999 };
//	CRenderEngine::DrawText(m_pManager->GetPaintDC(), m_pManager, rcText, m_sText, pInfo->dwTextColor, pInfo->nFont, DT_CALCRECT | pInfo->uTextStyle & ~DT_RIGHT & ~DT_CENTER);
	HWND hDestopWnd = ::GetDesktopWindow();
	if (hDestopWnd != NULL)
	{
		HDC hdesktopDc = ::GetDC(hDestopWnd);   // 使用桌面窗口上下文句柄，计算文本大小
		CRenderEngine::DrawText(hdesktopDc, m_pManager, rcText, m_sText, pInfo->dwTextColor, pInfo->nFont, DT_CALCRECT | pInfo->uTextStyle);
		::ReleaseDC(hDestopWnd, hdesktopDc);
		m_uTextNeedLen = rcText.right - rcText.left + rcTextPadding.left + rcTextPadding.right;
		m_uControlNeedLen = m_uTextNeedLen;
	}
// 	m_uTextNeedLen = rcText.right - rcText.left + rcTextPadding.left + rcTextPadding.right;
// 	m_uControlNeedLen = m_uTextNeedLen;
	return m_uTextNeedLen;
}

void CURLElement::SetControlNeedLen(int nLen)
{
	if (nLen > 0)
	{
		m_uControlNeedLen = nLen;
	}
}

void CURLElement::SetText(LPCTSTR pstrText)
{
	if( m_sText == pstrText ) return;

	m_bTextChanged = true;
	m_sText = pstrText;
	Invalidate();
}

}