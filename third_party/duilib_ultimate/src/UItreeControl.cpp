#include "StdAfx.h" 
namespace DuiLib {
	/////////////////////////////////////////////////////////////////////////////////////
	//
	//

#define  EXPAND_TIMER 0X112
	CTreeControlUI::CTreeControlUI() : m_pCallback(NULL), m_bScrollSelect(false),/* m_iCurSel(-1), */m_iExpandedItem(-1)
	{
		m_pTreeUI = new CTreeBodyUI(this);
		CVerticalLayoutUI::Add(m_pTreeUI);
		m_ListInfo.nColumns = 0;
		m_ListInfo.nFont = -1;
		m_ListInfo.uTextStyle = DT_VCENTER; // m_uTextStyle(DT_VCENTER | DT_END_ELLIPSIS)
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

		//add by ligthen
		m_nTopIndex = 0;
		m_nItemHeight = 20;
		m_nItems = 0;
		m_nShowItems = 0;
		m_pListDataSource = NULL;
		Init();

		m_bSingleSel = false;
		m_nFocusItem = -1;
		m_nStartSelItem = 0;
		m_nEndSelItem = -1;
		m_bLButtonDown = false;
		m_bLButtonDown_ClickInData = false;

		memset(&m_pStartPoint, 0x00, sizeof(POINT));
		memset(&m_pEndPoint, 0x00, sizeof(POINT));

		m_bNeedRefresh = false;
		m_uMoveSelectIndex = -1;


		m_uTextStyle = DT_VCENTER;
		m_dwTextColor = 1;
		m_dwDisabledTextColor = 1;
		m_iFont = -1;
		m_bShowHtml = false;
		::ZeroMemory(&m_rcTextPadding, sizeof(m_rcTextPadding));

		m_bSelTopIndex = false;

		
	}

	CTreeControlUI::~CTreeControlUI(void)
	{

	}

	LPCTSTR CTreeControlUI::GetClass() const
	{
		return _T("TreeUI");
	}

	UINT CTreeControlUI::GetControlFlags() const
	{
		return UIFLAG_TABSTOP;
	}

	LPVOID CTreeControlUI::GetInterface(LPCTSTR pstrName)
	{
		if (_tcscmp(pstrName, _T("Tree")) == 0) return static_cast<CTreeControlUI*>(this);
		if (_tcscmp(pstrName, _T("IList")) == 0) return static_cast<IListUI*>(this);
		if (_tcscmp(pstrName, _T("IListOwner")) == 0) return static_cast<IListOwnerUI*>(this);
		return CVerticalLayoutUI::GetInterface(pstrName);
	}

	CControlUI* CTreeControlUI::GetItemExAt(int iIndex) const
	{
		if (m_pListDataSource == NULL)
			return NULL;
		return static_cast<CControlUI *>(m_pListDataSource->listItemForIndex(iIndex));
	}

	int CTreeControlUI::GetAllCount() const
	{
		return m_nItems;
	}

	bool CTreeControlUI::GetScrollSelect()
	{
		return m_bScrollSelect;
	}

	void CTreeControlUI::SetScrollSelect(bool bScrollSelect)
	{
		m_bScrollSelect = bScrollSelect;
	}

	int CTreeControlUI::GetCurSel() const
	{
		if (m_aSelItems.GetSize() <= 0)
		{
			return -1;
		}
		else
		{
			return (int)m_aSelItems.GetAt(0);
		}

		return -1;
	}

	CContainerUI* CTreeControlUI::GetList() const
	{
		return m_pTreeUI;
	}

	TListInfoUI* CTreeControlUI::GetListInfo()
	{
		return &m_ListInfo;
	}

	CControlUI* CTreeControlUI::GetItemAt(int iIndex) const
	{
		return m_pTreeUI->GetItemAt(iIndex);
	}

	int CTreeControlUI::GetItemIndex(CControlUI* pControl) const
	{
		return m_pTreeUI->GetItemIndex(pControl);
	}

	bool CTreeControlUI::SetItemIndex(CControlUI* pControl, int iIndex)
	{
		int iOrginIndex = m_pTreeUI->GetItemIndex(pControl);
		if (iOrginIndex == -1) return false;
		if (iOrginIndex == iIndex) return true;

		IListItemUI* pSelectedListItem = NULL;
		// 	if( m_iCurSel >= 0 ) 
		// 		pSelectedListItem = static_cast<IListItemUI*>(GetItemAt(m_iCurSel)->GetInterface(_T("ListItem")));
		if (!m_pTreeUI->SetItemIndex(pControl, iIndex)) return false;
		int iMinIndex = min(iOrginIndex, iIndex);
		int iMaxIndex = max(iOrginIndex, iIndex);
		for (int i = iMinIndex; i < iMaxIndex + 1; ++i) {
			CControlUI* p = m_pTreeUI->GetItemAt(i);
			IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(_T("ListItem")));
			if (pListItem != NULL) {
				pListItem->SetIndex(i);
			}
		}
		// 	if( m_iCurSel >= 0 && pSelectedListItem != NULL ) m_iCurSel = pSelectedListItem->GetIndex();
		return true;
	}

	int CTreeControlUI::GetCount() const
	{
		return m_pTreeUI->GetCount();
	}

	bool CTreeControlUI::Add(CControlUI* pControl)
	{
		if (pControl == NULL)
		{
			return false;
		}

		// The list items should know about us
		IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
		if (pListItem != NULL) {
			pListItem->SetOwner(this);
			pListItem->SetIndex(GetCount());
		}
		return m_pTreeUI->Add(pControl);
	}

	bool CTreeControlUI::AddAt(CControlUI* pControl, int iIndex)
	{
		if (!m_pTreeUI->AddAt(pControl, iIndex)) return false;

		// The list items should know about us
		IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
		if (pListItem != NULL) {
			pListItem->SetOwner(this);
			pListItem->SetIndex(iIndex);
		}

		for (int i = iIndex + 1; i < m_pTreeUI->GetCount(); ++i) {
			CControlUI* p = m_pTreeUI->GetItemAt(i);
			pListItem = static_cast<IListItemUI*>(p->GetInterface(_T("ListItem")));
			if (pListItem != NULL) {
				pListItem->SetIndex(i);
			}
		}
		/*	if( m_iCurSel >= iIndex ) m_iCurSel += 1;*/
		return true;
	}

	bool CTreeControlUI::Remove(CControlUI* pControl)
	{

		int iIndex = m_pTreeUI->GetItemIndex(pControl);
		if (iIndex == -1) return false;

		if (!m_pTreeUI->RemoveAt(iIndex)) return false;

		for (int i = iIndex; i < m_pTreeUI->GetCount(); ++i) {
			CControlUI* p = m_pTreeUI->GetItemAt(i);
			IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(_T("ListItem")));
			if (pListItem != NULL) {
				pListItem->SetIndex(i);
			}
		}

		return true;
	}

	bool CTreeControlUI::RemoveAt(int iIndex)
	{
		if (!m_pTreeUI->RemoveAt(iIndex)) return false;

		for (int i = iIndex; i < m_pTreeUI->GetCount(); ++i) {
			CControlUI* p = m_pTreeUI->GetItemAt(i);
			IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(_T("ListItem")));
			if (pListItem != NULL) pListItem->SetIndex(i);
		}

		return true;
	}

	void CTreeControlUI::RemoveAll()
	{
		m_iExpandedItem = -1;
		m_pTreeUI->RemoveAll();
	}

	void CTreeControlUI::Scroll(int dx, int dy)
	{
		if (dx == 0 && dy == 0) return;
		SIZE sz = m_pTreeUI->GetScrollPos();
		m_pTreeUI->SetScrollPos(CSize(sz.cx + dx, sz.cy + dy));
	}

	int CTreeControlUI::GetChildPadding() const
	{
		return m_pTreeUI->GetChildPadding();
	}

	void CTreeControlUI::SetChildPadding(int iPadding)
	{
		m_pTreeUI->SetChildPadding(iPadding);
	}

	void CTreeControlUI::SetItemFont(int index)
	{
		m_ListInfo.nFont = index;
		NeedUpdate();
	}

	void CTreeControlUI::SetItemTextStyle(UINT uStyle)
	{
		m_ListInfo.uTextStyle = uStyle;
		NeedUpdate();
	}

	void CTreeControlUI::SetItemTextPadding(RECT rc)
	{
		m_ListInfo.rcTextPadding = rc;
		NeedUpdate();
	}

	void CTreeControlUI::SetItemTextColor(DWORD dwTextColor)
	{
		m_ListInfo.dwTextColor = dwTextColor;
		Invalidate();
	}

	void CTreeControlUI::SetItemBkColor(DWORD dwBkColor)
	{
		m_ListInfo.dwBkColor = dwBkColor;
		Invalidate();
	}

	void CTreeControlUI::SetItemBkImage(LPCTSTR pStrImage)
	{
		m_ListInfo.sBkImage = pStrImage;
		Invalidate();
	}

	void CTreeControlUI::SetAlternateBk(bool bAlternateBk)
	{
		m_ListInfo.bAlternateBk = bAlternateBk;
		Invalidate();
	}

	void CTreeControlUI::SetSelectedItemTextColor(DWORD dwTextColor)
	{
		m_ListInfo.dwSelectedTextColor = dwTextColor;
		Invalidate();
	}

	void CTreeControlUI::SetSelectedItemBkColor(DWORD dwBkColor)
	{
		m_ListInfo.dwSelectedBkColor = dwBkColor;
		Invalidate();
	}

	void CTreeControlUI::SetSelectedItemImage(LPCTSTR pStrImage)
	{
		m_ListInfo.sSelectedImage = pStrImage;
		Invalidate();
	}

	void CTreeControlUI::SetHotItemTextColor(DWORD dwTextColor)
	{
		m_ListInfo.dwHotTextColor = dwTextColor;
		Invalidate();
	}

	void CTreeControlUI::SetHotItemBkColor(DWORD dwBkColor)
	{
		m_ListInfo.dwHotBkColor = dwBkColor;
		Invalidate();
	}

	void CTreeControlUI::SetHotItemImage(LPCTSTR pStrImage)
	{
		m_ListInfo.sHotImage = pStrImage;
		Invalidate();
	}

	void CTreeControlUI::SetDisabledItemTextColor(DWORD dwTextColor)
	{
		m_ListInfo.dwDisabledTextColor = dwTextColor;
		Invalidate();
	}

	void CTreeControlUI::SetDisabledItemBkColor(DWORD dwBkColor)
	{
		m_ListInfo.dwDisabledBkColor = dwBkColor;
		Invalidate();
	}

	void CTreeControlUI::SetDisabledItemImage(LPCTSTR pStrImage)
	{
		m_ListInfo.sDisabledImage = pStrImage;
		Invalidate();
	}

	void CTreeControlUI::SetItemLineColor(DWORD dwLineColor)
	{
		m_ListInfo.dwLineColor = dwLineColor;
		Invalidate();
	}

	bool CTreeControlUI::IsItemShowHtml()
	{
		return m_ListInfo.bShowHtml;
	}

	void CTreeControlUI::SetItemShowHtml(bool bShowHtml /*= true*/)
	{
		if (m_ListInfo.bShowHtml == bShowHtml) return;

		m_ListInfo.bShowHtml = bShowHtml;
		NeedUpdate();
	}

	RECT CTreeControlUI::GetItemTextPadding() const
	{
		return m_ListInfo.rcTextPadding;
	}

	DWORD CTreeControlUI::GetItemTextColor() const
	{
		return m_ListInfo.dwTextColor;
	}

	DWORD CTreeControlUI::GetItemBkColor() const
	{
		return m_ListInfo.dwBkColor;
	}

	LPCTSTR CTreeControlUI::GetItemBkImage() const
	{
		return m_ListInfo.sBkImage;
	}

	bool CTreeControlUI::IsAlternateBk() const
	{
		return m_ListInfo.bAlternateBk;
	}

	DWORD CTreeControlUI::GetSelectedItemTextColor() const
	{
		return m_ListInfo.dwSelectedTextColor;
	}

	DWORD CTreeControlUI::GetSelectedItemBkColor() const
	{
		return m_ListInfo.dwSelectedBkColor;
	}

	LPCTSTR CTreeControlUI::GetSelectedItemImage() const
	{
		return m_ListInfo.sSelectedImage;
	}

	DWORD CTreeControlUI::GetHotItemTextColor() const
	{
		return m_ListInfo.dwHotTextColor;
	}

	DWORD CTreeControlUI::GetHotItemBkColor() const
	{
		return m_ListInfo.dwHotBkColor;
	}

	LPCTSTR CTreeControlUI::GetHotItemImage() const
	{
		return m_ListInfo.sHotImage;
	}

	DWORD CTreeControlUI::GetDisabledItemTextColor() const
	{
		return m_ListInfo.dwDisabledTextColor;
	}

	DWORD CTreeControlUI::GetDisabledItemBkColor() const
	{
		return m_ListInfo.dwDisabledBkColor;
	}

	LPCTSTR CTreeControlUI::GetDisabledItemImage() const
	{
		return m_ListInfo.sDisabledImage;
	}

	DWORD CTreeControlUI::GetItemLineColor() const
	{
		return m_ListInfo.dwLineColor;
	}

	void CTreeControlUI::SetMultiExpanding(bool bMultiExpandable)
	{
		m_ListInfo.bMultiExpandable = bMultiExpandable;
	}

	int CTreeControlUI::GetExpandedItem() const
	{
		return m_iExpandedItem;
	}

	bool CTreeControlUI::ExpandNode()
	{
		int iIndex = m_uMoveSelectIndex;
		if (iIndex < 0) return false;
		CControlUI* pControl = GetItemAt(iIndex); 
		if (pControl == NULL) return false;
		if (!pControl->IsVisible()) return false;
		if (!pControl->IsEnabled()) return false;
		
		
		/*if (!m_pListDataSource->isExpand(iIndex + GetTopIndex()))
		{
			m_pListDataSource->Expand(iIndex + GetTopIndex());
		}*/
		/*	IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
			if (pListItem == NULL) return false;
			if (!pListItem->IsExpanded())
			{
			pListItem->Expand(true);
			}*/
		return true;
	}

	bool CTreeControlUI::ExpandItem(int iIndex, bool bExpand /*= true*/)
	{
		if (m_iExpandedItem >= 0 && !m_ListInfo.bMultiExpandable) {
			CControlUI* pControl = GetItemAt(m_iExpandedItem);
			if (pControl != NULL) {
				IListItemUI* pItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
				if (pItem != NULL) pItem->Expand(false);
			}
			m_iExpandedItem = -1;
		}
		if (bExpand) {
			CControlUI* pControl = GetItemAt(iIndex);
			if (pControl == NULL) return false;
			if (!pControl->IsVisible()) return false;
			IListItemUI* pItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
			if (pItem == NULL) return false;
			m_iExpandedItem = iIndex;
			if (!pItem->Expand(true)) {
				m_iExpandedItem = -1;
				return false;
			}
		}
		NeedUpdate();
		return true;
	}

	void CTreeControlUI::SetPos(RECT rc)
	{
		CVerticalLayoutUI::SetPos(rc);
		
		RefreshListData(m_nTopIndex);
		//重新计算滚动条的位置
		if (m_pTreeUI)
		{
			SIZE szPos = m_pTreeUI->GetScrollPos();
			szPos.cy = m_nTopIndex*m_nItemHeight;
			m_pTreeUI->SetScrollLinePos(szPos);
		}

		//	}

		int iOffset = m_pTreeUI->GetScrollPos().cx;

		//add by lighten
		if ((m_pTreeUI != NULL) && (m_ListInfo.nColumns > 0))
		{
			RECT rcHead = m_ListInfo.rcColumn[0];
			RECT rcTemp = { rc.left, rcHead.bottom, rc.right, rc.bottom };
			m_pTreeUI->SetPos(rcTemp);
		}
	}

	void CTreeControlUI::DoEvent(TEventUI& event)
	{
		if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND) {
			if (m_pParent != NULL) m_pParent->DoEvent(event);
			else CVerticalLayoutUI::DoEvent(event);
			return;
		}
		if (event.Type == UIEVENT_SETFOCUS)
		{
			m_bFocused = true;
			return;
		}
		if (event.Type == UIEVENT_KILLFOCUS)
		{
			m_bFocused = false;
			return;
		}
		if (event.Type == UIEVENT_RBUTTONDOWN)   //右键消息
		{
			if (!IsEnabled())
			{
				return;
			}
			UnSelectAllItems();
			if (m_pManager)
			{
				m_pManager->SendNotify(this, _T("itemrbuttonclick"), DUILIB_LIST_ITEM_RBUTTON_CLICK, -1, 0, true);
			}
			return;
		}
		if (event.Type == UIEVENT_DBLCLICK)
		{
			if (!IsEnabled())
			{
				return;
			}
			UnSelectAllItems();
			if (m_pManager != NULL)
			{
				m_pManager->SendNotify(this, _T("itemdbclick"), DUILIB_LIST_ITEM_DOUBLE_CLICK, -1, 0, 0);
			}
		}
		if (event.Type == UIEVENT_BUTTONDOWN)
		{
			
			if (!IsEnabled())
			{
				return;
			} 
			UnSelectAllItems();
			if (m_pManager)
			{
				m_pManager->SendNotify(this, _T("itemunselect"), DUILIB_LIST_ITEMCLICK, -1, 0, 0);
			}

		}
		if (event.Type == UIEVENT_BUTTONUP)
		{
			if (!IsEnabled())
			{
				return;
			}
		}
		if (event.Type == UIEVENT_MOUSEMOVE)
		{
			if (!IsEnabled())
			{
				return;
			}
			
		}
		switch (event.Type) {
		case UIEVENT_KEYDOWN:
			switch (event.chKey) {
			case VK_UP:
				if (m_aSelItems.GetSize() > 0)
				{
					if (!IsEnabled())
					{
						return;
					}
					if ((!m_bSingleSel) && (GetKeyState(VK_SHIFT) < 0))
					{
						if (IsFocused() && IsVisible())
						{
							int nIndex = m_nEndSelItem - m_nTopIndex;
							if (IsHiddenItemInTop() && nIndex == 1)
							{
								LineUp();
							}
							else if (nIndex == 0)
							{
								LineUp();
							}

							if (m_nFocusItem > 0)
							{
								m_nFocusItem--;
							}
							m_nEndSelItem = m_nFocusItem;
							SetItemSelect(m_nStartSelItem, m_nEndSelItem, true);
						}

					}
					else if ((!m_bSingleSel) && (GetKeyState(VK_CONTROL) < 0))
					{
						// 				if (IsFocused() && IsVisible())
						// 				{
						// 					if (m_nFocusItem > 0)
						// 					{
						// 						m_nFocusItem --;
						// 					}
						// 				}
					}
					else
					{
						int nIndex = GetMinSelItemIndex() - m_nTopIndex;
						if (IsHiddenItemInTop() && nIndex == 1)
						{
							LineUp();
						}
						else if (nIndex == 0)
						{
							LineUp();
						}

						nIndex = nIndex - 1;
						if (nIndex > 0)
						{
							SelectItem(nIndex, true);
						}
						else
						{
							SelectItem(0, true);
						}
					}
				}
				return;
			case VK_DOWN:
				if (m_aSelItems.GetSize() > 0)
				{
					if (!IsEnabled())
					{
						return;
					}
					if ((!m_bSingleSel) && (GetKeyState(VK_SHIFT) < 0))
					{
						int nIndex = m_nEndSelItem - m_nTopIndex;
						if (IsHiddenItemInBottom() && nIndex == m_nShowItems - 1 - 1)
						{
							if (m_nTopIndex + m_nShowItems + NUM_LEFT + 1 <= m_nItems)
							{
								nIndex = nIndex - 1;
							}
							LineDown();
						}
						else if (nIndex == m_nShowItems - 1)
						{
							LineDown();
						}

						if (IsFocused() && IsVisible())
						{
							if (m_nFocusItem < m_nItems)
							{
								m_nFocusItem++;
							}
							m_nEndSelItem = m_nFocusItem;
							SetItemSelect(m_nStartSelItem, m_nEndSelItem, true);
						}
					}
					else if ((!m_bSingleSel) && ((event.wKeyState & MK_CONTROL) == MK_CONTROL))
					{
						// 				if (IsFocused() && IsVisible())
						// 				{
						// 					if (m_nFocusItem < (m_pList->GetCount() - 1))
						// 					{
						// 						m_nFocusItem ++;
						// 					}
						// 				}
					}
					else
					{
						int nIndex = GetMaxSelItemIndex() - m_nTopIndex;
						if (IsHiddenItemInBottom() && nIndex == m_nShowItems - 1 - 1)
						{
							if (m_nTopIndex + m_nShowItems + NUM_LEFT + 1 <= m_nItems)
							{
								nIndex = nIndex - 1;
							}
							LineDown();
						}
						else if (nIndex == m_nShowItems - 1)
						{
							LineDown();
						}

						nIndex = nIndex + 1;
						if ((nIndex + 1) > m_pTreeUI->GetCount())
						{
							SelectItem(GetCount() - 1, true);
						}
						else
						{
							SelectItem(nIndex, true);
						}
					}
					Invalidate();
				}
				return;
			case VK_PRIOR:
			{
				if (!IsEnabled())
				{
					return;
				}
				PageUp();
			}
			return;
			case VK_NEXT:
			{
				if (!IsEnabled())
				{
					return;
				}
				PageDown();
			}
			return;
			case VK_HOME:
			{
				if (!IsEnabled())
				{
					break;
				}
				HomeUp();
				if (m_pTreeUI->GetCount() > 0)
					SelectItem(0, true);
			}
			return;
			case VK_END:
			{
				if (!IsEnabled())
				{
					break;
				}
				EndDown();
				if (m_pTreeUI->GetCount() > 0)
					SelectItem(m_pTreeUI->GetCount() - 1, true);
			}

			return;
			// 	case VK_RETURN:
			// 		if( m_iCurSel != -1 ) GetItemAt(m_iCurSel)->Activate();
			// 		return;
			}
			break;
		case UIEVENT_SCROLLWHEEL:
		{
			if (!IsEnabled())
			{
				break;
			}
			switch (LOWORD(event.wParam))
			{
			case SB_LINEUP:
				if (m_bScrollSelect && m_bSingleSel)
					SelectItem(FindSelectable((int)m_aSelItems.GetAt(0) - m_nTopIndex - 1, false));
				else LineUp();
				return;
			case SB_LINEDOWN:
				if (m_bScrollSelect && m_bSingleSel)
					SelectItem(FindSelectable((int)m_aSelItems.GetAt(0) - m_nTopIndex + 1, true));
				else LineDown();
				return;
			}
		}
		break;
		}
		if (event.Type == UIEVENT_TIMER  && event.wParam == EXPAND_TIMER)
		{
			m_pManager->KillTimer(this, EXPAND_TIMER);
			ExpandNode();
		}
		CVerticalLayoutUI::DoEvent(event);
	}

	void CTreeControlUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if (_tcscmp(pstrName, _T("scrollselect")) == 0) SetScrollSelect(_tcscmp(pstrValue, _T("true")) == 0);
		else if (_tcscmp(pstrName, _T("multiexpanding")) == 0) SetMultiExpanding(_tcscmp(pstrValue, _T("true")) == 0);
		else if (_tcscmp(pstrName, _T("itemfont")) == 0) m_ListInfo.nFont = _ttoi(pstrValue);
		else if (_tcscmp(pstrName, _T("itemalign")) == 0) {
			if (_tcsstr(pstrValue, _T("left")) != NULL) {
				m_ListInfo.uTextStyle &= ~(DT_CENTER | DT_RIGHT);
				m_ListInfo.uTextStyle |= DT_LEFT;
			}
			if (_tcsstr(pstrValue, _T("center")) != NULL) {
				m_ListInfo.uTextStyle &= ~(DT_LEFT | DT_RIGHT);
				m_ListInfo.uTextStyle |= DT_CENTER;
			}
			if (_tcsstr(pstrValue, _T("right")) != NULL) {
				m_ListInfo.uTextStyle &= ~(DT_LEFT | DT_CENTER);
				m_ListInfo.uTextStyle |= DT_RIGHT;
			}
		}
		else if (_tcscmp(pstrName, _T("endellipsis")) == 0) {
			if (_tcscmp(pstrValue, _T("true")) == 0)
			{
				m_ListInfo.uTextStyle &= ~DT_PATH_ELLIPSIS;
				m_ListInfo.uTextStyle |= DT_END_ELLIPSIS;
			}
			else m_ListInfo.uTextStyle &= ~DT_END_ELLIPSIS;
		}
		else if (_tcscmp(pstrName, _T("pathellipsis")) == 0)    //add by lighten 2013.03.29, 支持中间省略
		{
			if (_tcscmp(pstrValue, _T("true")) == 0)
			{
				m_ListInfo.uTextStyle &= ~DT_END_ELLIPSIS;
				m_ListInfo.uTextStyle |= DT_PATH_ELLIPSIS;
			}
			else m_ListInfo.uTextStyle &= ~DT_PATH_ELLIPSIS;
		}
		else if (_tcscmp(pstrName, _T("itemtextpadding")) == 0) {
			RECT rcTextPadding = { 0 };
			LPTSTR pstr = NULL;
			rcTextPadding.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
			rcTextPadding.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
			rcTextPadding.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
			rcTextPadding.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
			SetItemTextPadding(rcTextPadding);
		}
		else if (_tcscmp(pstrName, _T("itemtextcolor")) == 0) {
			if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetItemTextColor(clrColor);
		}
		else if (_tcscmp(pstrName, _T("itembkcolor")) == 0) {
			if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetItemBkColor(clrColor);
		}
		else if (_tcscmp(pstrName, _T("itembkimage")) == 0) SetItemBkImage(pstrValue);
		else if (_tcscmp(pstrName, _T("itemaltbk")) == 0) SetAlternateBk(_tcscmp(pstrValue, _T("true")) == 0);
		else if (_tcscmp(pstrName, _T("itemselectedtextcolor")) == 0) {
			if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetSelectedItemTextColor(clrColor);
		}
		else if (_tcscmp(pstrName, _T("itemselectedbkcolor")) == 0) {
			if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetSelectedItemBkColor(clrColor);
		}
		else if (_tcscmp(pstrName, _T("itemselectedimage")) == 0) SetSelectedItemImage(pstrValue);
		else if (_tcscmp(pstrName, _T("itemhottextcolor")) == 0) {
			if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetHotItemTextColor(clrColor);
		}
		else if (_tcscmp(pstrName, _T("itemhotbkcolor")) == 0) {
			if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetHotItemBkColor(clrColor);
		}
		else if (_tcscmp(pstrName, _T("itemhotimage")) == 0) SetHotItemImage(pstrValue);
		else if (_tcscmp(pstrName, _T("itemdisabledtextcolor")) == 0) {
			if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetDisabledItemTextColor(clrColor);
		}
		else if (_tcscmp(pstrName, _T("itemdisabledbkcolor")) == 0) {
			if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetDisabledItemBkColor(clrColor);
		}
		else if (_tcscmp(pstrName, _T("itemdisabledimage")) == 0) SetDisabledItemImage(pstrValue);
		else if (_tcscmp(pstrName, _T("itemlinecolor")) == 0) {
			if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetItemLineColor(clrColor);
		}
		else if (_tcscmp(pstrName, _T("itemshowhtml")) == 0) SetItemShowHtml(_tcscmp(pstrValue, _T("true")) == 0);
		else if (_tcscmp(pstrName, _T("align")) == 0) {
			if (_tcsstr(pstrValue, _T("left")) != NULL) {
				m_uTextStyle &= ~(DT_CENTER | DT_RIGHT | DT_TOP | DT_BOTTOM);
				m_uTextStyle |= DT_LEFT;
			}
			if (_tcsstr(pstrValue, _T("center")) != NULL) {
				m_uTextStyle &= ~(DT_LEFT | DT_RIGHT | DT_TOP | DT_BOTTOM);
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
		else if (_tcscmp(pstrName, _T("font")) == 0) m_iFont = _ttoi(pstrValue);
		else if (_tcscmp(pstrName, _T("textcolor")) == 0) {
			if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			m_dwTextColor = clrColor;
		}
		else if (_tcscmp(pstrName, _T("disabledtextcolor")) == 0) {
			if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			m_dwDisabledTextColor = clrColor;
		}
		else if (_tcscmp(pstrName, _T("textpadding")) == 0) {
			RECT rcTextPadding = { 0 };
			LPTSTR pstr = NULL;
			rcTextPadding.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
			rcTextPadding.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
			rcTextPadding.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
			rcTextPadding.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
			m_rcTextPadding = rcTextPadding;
		}
		else if (_tcscmp(pstrName, _T("showhtml")) == 0) m_bShowHtml = _tcscmp(pstrValue, _T("true")) == 0 ? true : false;
		else CVerticalLayoutUI::SetAttribute(pstrName, pstrValue);
	}

	void CTreeControlUI::DoPaint(HDC hDC, const RECT& rcPaint)
	{
		CContainerUI::DoPaint(hDC, rcPaint);
		PaintText(hDC);
	}

	void CTreeControlUI::PaintSelectItem(HDC hDC, const RECT &rcPaint)
	{
		if (m_pTreeUI == NULL)
		{
			return;
		}
		int dx = rcPaint.left - m_rcItem.left;
		int dy = rcPaint.top - m_rcItem.top;
		for (int i = 0; i < m_pTreeUI->GetCount(); i++)
		{
			CTreeContainerElementUI *pListItem = static_cast<CTreeContainerElementUI*>(m_pTreeUI->GetItemAt(i));
			if (pListItem == NULL)
			{
				continue;
			}
			if (pListItem->IsSelected())
			{
				pListItem->DoPaint(hDC, rcPaint);
			}
		}
	}

	void CTreeControlUI::PaintText(HDC hDC)
	{
		if (m_pListDataSource == NULL)
		{
			return;
		}
		if (m_nItems != 0)
		{
			return;
		}

		if (m_sText.IsEmpty())
		{
			return;
		}

		if (m_dwTextColor == 0) m_dwTextColor = m_pManager->GetDefaultFontColor();
		if (m_dwDisabledTextColor == 0) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

		if (m_sText.IsEmpty()) return;
		int nLinks = 0;
		RECT rc = m_rcItem;
		m_rcTextPadding.top = (m_rcItem.bottom - m_rcItem.top) * 2 / 5;
		rc.left += m_rcTextPadding.left;
		rc.right -= m_rcTextPadding.right;
		rc.top += m_rcTextPadding.top;
		rc.bottom -= m_rcTextPadding.bottom;
		if (IsEnabled()) {
			if (m_bShowHtml)
				CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, m_sText, m_dwTextColor, \
				NULL, NULL, nLinks, DT_SINGLELINE | m_uTextStyle);
			else
				CRenderEngine::DrawText(hDC, m_pManager, rc, m_sText, m_dwTextColor, \
				m_iFont, DT_SINGLELINE | m_uTextStyle);
		}
		else {
			if (m_bShowHtml)
				CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, m_sText, m_dwDisabledTextColor, \
				NULL, NULL, nLinks, DT_SINGLELINE | m_uTextStyle);
			else
				CRenderEngine::DrawText(hDC, m_pManager, rc, m_sText, m_dwDisabledTextColor, \
				m_iFont, DT_SINGLELINE | m_uTextStyle);
		}
	}

	IListCallbackUI* CTreeControlUI::GetTextCallback() const
	{
		return m_pCallback;
	}

	void CTreeControlUI::SetTextCallback(IListCallbackUI* pCallback)
	{
		m_pCallback = pCallback;
	}

	SIZE CTreeControlUI::GetScrollPos() const
	{
		return m_pTreeUI->GetScrollPos();
	}

	SIZE CTreeControlUI::GetScrollRange() const
	{
		return m_pTreeUI->GetScrollRange();
	}

	void CTreeControlUI::SetScrollPos(SIZE szPos)
	{
		m_pTreeUI->SetScrollPos(szPos);
	}

	void CTreeControlUI::LineUp()
	{
		m_pTreeUI->LineUp();
	}

	void CTreeControlUI::LineDown()
	{
		m_pTreeUI->LineDown();
	}

	void CTreeControlUI::PageUp()
	{
		m_pTreeUI->PageUp();
	}

	void CTreeControlUI::PageDown()
	{
		m_pTreeUI->PageDown();
	}

	void CTreeControlUI::HomeUp()
	{
		m_pTreeUI->HomeUp();
	}

	void CTreeControlUI::EndDown()
	{
		m_pTreeUI->EndDown();
	}

	void CTreeControlUI::LineLeft()
	{
		m_pTreeUI->LineLeft();
	}

	void CTreeControlUI::LineRight()
	{
		m_pTreeUI->LineRight();
	}

	void CTreeControlUI::PageLeft()
	{
		m_pTreeUI->PageLeft();
	}

	void CTreeControlUI::PageRight()
	{
		m_pTreeUI->PageRight();
	}

	void CTreeControlUI::HomeLeft()
	{
		m_pTreeUI->HomeLeft();
	}

	void CTreeControlUI::EndRight()
	{
		m_pTreeUI->EndRight();
	}

	void CTreeControlUI::EnableScrollBar(bool bEnableVertical /*= true*/, bool bEnableHorizontal /*= false*/)
	{
		m_pTreeUI->EnableScrollBar(bEnableVertical, bEnableHorizontal);
	}

	CScrollBarUI* CTreeControlUI::GetVerticalScrollBar() const
	{
		return m_pTreeUI->GetVerticalScrollBar();
	}

	CScrollBarUI* CTreeControlUI::GetHorizontalScrollBar() const
	{
		return m_pTreeUI->GetHorizontalScrollBar();
	}

	void CTreeControlUI::EnsureVisible(int iIndex)
	{
		if (iIndex < 0 || iIndex >= m_nItems)
			return;
		int nOldTop = m_nTopIndex;
		if (iIndex < m_nItems - m_nShowItems - NUM_LEFT)
		{
			m_nTopIndex = iIndex;
		}
		else
			m_nTopIndex = m_nItems - m_nShowItems - NUM_LEFT;

		if (nOldTop == m_nTopIndex)
		{
			return;
		}

		SIZE sz = GetScrollPos();
		int iOffset = (m_nTopIndex - nOldTop)*m_nItemHeight;
		if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible())
			iOffset -= m_pHorizontalScrollBar->GetFixedHeight();
		sz.cy += iOffset;
		SetScrollPos(sz);
	}

	void CTreeControlUI::SetItemHeight(int nItemHeigh)
	{
		if (nItemHeigh < m_pManager->GetDefaultFontInfo()->tm.tmHeight + 8)
		{
			m_nItemHeight = m_pManager->GetDefaultFontInfo()->tm.tmHeight + 8;
		}
		else
			m_nItemHeight = nItemHeigh;

		CalShowItemNum();
	}

	int CTreeControlUI::GetItemHeight() const
	{
		return m_nItemHeight;
	}

	void CTreeControlUI::SetItemCount(int nNums)
	{
		if (nNums >= 0)
		{
			m_nItems = nNums;
		}
	}

	int CTreeControlUI::GetItemCount() const
	{
		return m_nItems;
	}

	int CTreeControlUI::GetTopIndex() const
	{
		return m_nTopIndex;
	}

	int CTreeControlUI::GetShowItems() const
	{
		return m_nShowItems;
	}

	void CTreeControlUI::CalShowItemNum()
	{
		if (m_nItemHeight == 0)
		{


			m_nShowItems = 0;
			return;
		}

		RECT rc = m_pTreeUI->GetPos();
		int nHeight = rc.bottom - rc.top;
		if (nHeight == 0)
		{
			m_bNeedRefresh = true;    //需要在计算pos的时候重新刷新
			m_nShowItems = 0;
			return;
		}
		m_nShowItems = nHeight / m_nItemHeight;
		if (nHeight%m_nItemHeight != 0)  //不是刚刚好全部显示
		{
			m_nShowItems++;
		}
	}

	void CTreeControlUI::SetListDataSource(IListDataSource *ListDataSource, bool bChangeTopIndex /*= true*/)
	{
		if (ListDataSource == NULL)
		{
			RemoveAll();
			m_nTopIndex = 0;
			m_nItems = 0;
			SetAllNeedHeight(0);
			m_pListDataSource = NULL;
			return;
		}

		m_pListDataSource = ListDataSource;
		RemoveAll();
		if (bChangeTopIndex)
		{
			m_nTopIndex = 0;
			m_aSelItems.Empty();
			m_bSelTopIndex = false;
		}

		if (!bChangeTopIndex)
		{
			RefreshListData(m_nTopIndex);
			SIZE sz = { 0, 0 };
			sz.cy = m_nTopIndex * m_nItemHeight;
			SetVerticalScrollPos(sz.cy);
			return;
		}
		Init();

		if (m_nShowItems == 0)   //需要显示的数据为0
		{
			SIZE sz;
			sz.cx = sz.cy = 0;
			SetScrollPos(sz);
			return;
		}

		//重新插入数据
		int nNumInset = m_nItems - m_nShowItems - NUM_LEFT;
		if (nNumInset < 0)
		{
			nNumInset = m_nItems;
		}
		else
			nNumInset = m_nShowItems;


		for (int i = 0; i < nNumInset + NUM_LEFT; i++)
		{
			if (i >= m_nItems)
			{
				break;
			}
			CTreeContainerElementUI *pListItem = static_cast<CTreeContainerElementUI *>(m_pListDataSource->listItemForIndex(i));
			if (pListItem == NULL)
			{
				return;
			}

			Add(pListItem);
		}
		SIZE sz;
		sz.cx = sz.cy = 0;
		SetScrollPos(sz);
	}

	bool CTreeControlUI::IsHiddenItemInTop()
	{
		return m_pTreeUI->IsHiddenItemInTop();
	}

	bool CTreeControlUI::IsHiddenItemInBottom()
	{
		return m_pTreeUI->IsHiddenItemInBottom();
	}

	bool CTreeControlUI::ScrollDownLine()
	{
		if (m_pListDataSource == NULL)
		{
			return false;
		}
		if (m_nTopIndex + m_nShowItems + NUM_LEFT + 1 > m_nItems)
		{
			if (IsHiddenItemInBottom())
			{
				return true;
			}
			return false;
		}
		CTreeContainerElementUI *pListElement = static_cast<CTreeContainerElementUI *>(m_pListDataSource->listItemForIndex(m_nTopIndex + m_nShowItems + NUM_LEFT));
		if (pListElement == NULL)
		{
			return false;
		}
		//int nindx = m_aSelItems.Find((LPVOID)(m_nTopIndex + m_nShowItems + NUM_LEFT));   //插入时检查是否选中项
		/*if (nindx >= 0)
		{*/
		pListElement->Select(m_pListDataSource->isSelected(m_nTopIndex + m_nShowItems + NUM_LEFT));
			/*}
			else
			{
			pListElement->Select(false);
			}*/
		
		/*	if (nindx >= 0)
			{
			pListElement->Select(true);
			}
			else
			pListElement->Select(false);
			*/
		Add(pListElement);

		RemoveAt(0);
		m_nTopIndex++;

		return true;
	}

	UINT CTreeControlUI::ScrollUpLine()
	{
		if (m_pListDataSource == NULL)
		{
			return 0;
		}

		if (m_nTopIndex <= 0)   //少于预留
		{
			return 0;
		}

		if (m_pTreeUI && IsHiddenItemInTop())   //有数据还在前面，先显示数据
		{
			return m_pTreeUI->GetHiddenItemInTop();
		}

		m_nTopIndex--;
		CTreeContainerElementUI *pListElement = static_cast<CTreeContainerElementUI *>(m_pListDataSource->listItemForIndex(m_nTopIndex));
		if (pListElement == NULL)
		{
			return 0;
		}
		//int nindx = m_aSelItems.Find((LPVOID)(m_nTopIndex));   //插入时检查是否选中项
		
		//if (nindx >= 0)
		//{
		pListElement->Select(m_pListDataSource->isSelected(m_nTopIndex));
		//}
		//else
		//	pListElement->Select(false);


		AddAt(pListElement, 0);
		RemoveAt(GetCount() - 1);
		return 0;
	}

	bool CTreeControlUI::RefreshListData(int nTopIndex, bool bInit /*= true*/)
	{
		m_bFreshed = true;
		if (m_pListDataSource == NULL)
		{
			return false;
		}

		if (bInit)
		{
			SetItemCount(m_pListDataSource->numberOfRows());
			CalShowItemNum();
			if (m_nItems > m_nShowItems)   //如果显示不全
			{
				m_pTreeUI->SetAllNeedHeight(m_nItems * m_nItemHeight);
			}
			else
				m_pTreeUI->SetAllNeedHeight(0);
		}

		m_nTopIndex = nTopIndex;
		if (m_nTopIndex + m_nShowItems >= m_nItems)
		{
			m_nTopIndex = m_nItems - m_nShowItems;
		}
		if (m_nTopIndex < 0)
		{
			m_nTopIndex = 0;
		}

		RemoveAll();

		//重新生成数据
		for (int i = 0; i < m_nShowItems + NUM_LEFT; i++)
		{
			if (i + m_nTopIndex >= m_nItems)
			{
				break;
			}
			CTreeContainerElementUI *pListItem = static_cast<CTreeContainerElementUI *>(m_pListDataSource->listItemForIndex(m_nTopIndex + i));
			if (pListItem == NULL)
			{
				return false;
			}
			//int nindx = m_aSelItems.Find((LPVOID)(m_nTopIndex + i));   //插入时检查是否选中项
			bool isSelected = m_pListDataSource->isSelected(m_nTopIndex + i);
			//if (isSelected)
			//{
				pListItem->Select(isSelected);
			//}
			//else
			//	pListItem->Select(false);

			Add(pListItem);
		}

		//重新根据顶部位置设置 垂直导航条的高度
		SIZE sz = GetScrollPos();
		int iOffset = m_nTopIndex*m_nItemHeight;
		if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible())
			iOffset -= m_pHorizontalScrollBar->GetFixedHeight();
		sz.cy = iOffset;
		SetScrollPos(sz); 

		return true;
	}

	bool CTreeControlUI::RefreshPos(int nTopIndex, bool bInit /*= true*/)
	{
		if (bInit)
		{
			Init();
		}
		if (m_pListDataSource == NULL)
		{
			return false;
		}
		m_nTopIndex = nTopIndex;
		if (m_nTopIndex + m_nShowItems >= m_nItems)
		{
			m_nTopIndex = m_nItems - m_nShowItems;
		}
		if (m_nTopIndex < 0)
		{
			m_nTopIndex = 0;
		}
		RefreshListData(m_nTopIndex, bInit);
		SIZE sz = { 0, 0 };
		sz.cy = m_nTopIndex * m_nItemHeight;
		SetVerticalScrollPos(sz.cy);

		return true;
	}

	bool CTreeControlUI::DeleteIndexData(int Index)
	{
		m_bFreshed = false;
		m_aSelItems.Empty();
		m_bSelTopIndex = false;
		RefreshPos(m_nTopIndex);
		return true;
	}

	bool CTreeControlUI::InsetIndexData(int Index)
	{
		if (Index < 0)
		{
			return false;
		}

		Init();
		if (Index > m_nItems)
		{
			return false;
		}

		if (Index < m_nTopIndex)
		{
			m_nTopIndex++;
		}

		for (int i = 0; i < m_aSelItems.GetSize(); i++)
		{
			int nSel = (int)m_aSelItems.GetAt(i);
			if (nSel >= Index)
			{
				m_aSelItems.SetAt(i, (LPVOID)(nSel + 1));
			}
		}
		RefreshPos(m_nTopIndex, false);
		return true;
	}

	bool CTreeControlUI::InsetDataAt(int nIndex)
	{
		if (nIndex < 0)
		{
			return false;
		}

		if (nIndex > m_nItems)
		{
			return false;
		}

		if (m_pListDataSource == NULL)
		{
			return false;
		}

		if (m_nShowItems == 0)
		{
			return false;
		}

		m_nItems++;

		SIZE szPos = m_pTreeUI->GetScrollPos();
		if (m_nItems > m_nShowItems)   //如果显示不全
		{
			m_pTreeUI->SetAllNeedHeight(m_nItems * m_nItemHeight);
		}
		else
			m_pTreeUI->SetAllNeedHeight(0);

		if (nIndex < m_nTopIndex)   //插入到前面显示
		{
			m_nTopIndex++;
		}
		else if (nIndex == m_nTopIndex)
		{
			if (GetCount() < m_nShowItems)
			{
				if (m_pListDataSource)
				{
					CTreeContainerElementUI *pListItem = static_cast<CTreeContainerElementUI *>(m_pListDataSource->listItemForIndex(nIndex));
					if (pListItem == NULL)
					{
						return false;
					}
					AddAt(pListItem, 0);
				}
			}
			else
			{
				if (m_pListDataSource)
				{
					CTreeContainerElementUI *pListItem = static_cast<CTreeContainerElementUI *>(m_pListDataSource->listItemForIndex(nIndex));
					if (pListItem == NULL)
					{
						return false;
					}
					AddAt(pListItem, 0);

					RemoveAt(GetCount() - 1);
				}
			}
		}
		else if (nIndex > m_nTopIndex)
		{
			if (GetCount() < m_nShowItems)
			{
				if (m_pListDataSource)
				{
					CTreeContainerElementUI *pListItem = static_cast<CTreeContainerElementUI *>(m_pListDataSource->listItemForIndex(nIndex));
					if (pListItem == NULL)
					{
						return false;
					}
					Add(pListItem);
				}
			}
			else
			{
				if (nIndex - m_nTopIndex < m_nShowItems)
				{
					if (m_pListDataSource)
					{
						CTreeContainerElementUI *pListItem = static_cast<CTreeContainerElementUI *>(m_pListDataSource->listItemForIndex(nIndex));
						if (pListItem == NULL)
						{
							return false;
						}
						AddAt(pListItem, nIndex - m_nTopIndex);
						RemoveAt(GetCount() - 1);
					}
				}

			}
		}

		for (int i = 0; i < m_aSelItems.GetSize(); i++)
		{
			int nSel = (int)m_aSelItems.GetAt(i);
			if (nSel >= nIndex)
			{
				m_aSelItems.SetAt(i, (LPVOID)(nSel + 1));
			}
		}
		m_pTreeUI->SetPos(m_pTreeUI->GetPos());
		m_pTreeUI->SetVerticalScrollPos(m_nItemHeight * m_nTopIndex);
		Invalidate();
		return true;
	}

	bool CTreeControlUI::DeleteDataAt(int nIndex)
	{
		if (nIndex < 0)
		{
			return false;
		}

		if (nIndex >= m_nItems)
		{
			return false;
		}

		if (m_nShowItems == 0)
		{
			return false;
		}

		if (m_pListDataSource == NULL)
		{
			return false;
		}


		m_nItems--;

		SIZE szPos = m_pTreeUI->GetScrollPos();
		if (m_nItems > m_nShowItems)   //如果显示不全
		{
			m_pTreeUI->SetAllNeedHeight(m_nItems * m_nItemHeight);
		}
		else
			m_pTreeUI->SetAllNeedHeight(0);

		if (nIndex < m_nTopIndex)   //删除显示项目前面的项
		{
			if (m_nTopIndex > 0)
			{
				m_nTopIndex--;
			}
		}
		else if (nIndex == m_nTopIndex)
		{
			RemoveAt(0);
			if ((m_nTopIndex + m_nShowItems - 1) < m_nItems)
			{
				CTreeContainerElementUI *pListItem = static_cast<CTreeContainerElementUI *>(m_pListDataSource->listItemForIndex(m_nTopIndex + m_nShowItems - 1));
				if (pListItem == NULL)
				{
					return false;
				}
				Add(pListItem);
			}
			else if ((m_nTopIndex + m_nShowItems - 1) == m_nItems)
			{
				if (m_nTopIndex > 0)
				{
					m_nTopIndex--;
					CTreeContainerElementUI *pListItem = static_cast<CTreeContainerElementUI *>(m_pListDataSource->listItemForIndex(m_nTopIndex));
					if (pListItem == NULL)
					{
						return false;
					}
					AddAt(pListItem, 0);
				}
			}
		}
		else if (nIndex > m_nTopIndex)
		{
			if (GetCount() < m_nShowItems)
			{
				RemoveAt(nIndex - m_nTopIndex);
			}
			else
			{
				if ((nIndex - m_nTopIndex) < m_nShowItems)
				{
					RemoveAt(nIndex - m_nTopIndex);
					if ((m_nTopIndex + m_nShowItems - 1) < m_nItems)
					{
						CTreeContainerElementUI *pListItem = static_cast<CTreeContainerElementUI *>(m_pListDataSource->listItemForIndex(m_nTopIndex + m_nShowItems - 1));
						if (pListItem == NULL)
						{
							return false;
						}
						Add(pListItem);
					}
					else if (m_nTopIndex + m_nShowItems - 1 == m_nItems)
					{
						if (m_nTopIndex > 0)
						{
							m_nTopIndex--;
							CTreeContainerElementUI *pListItem = static_cast<CTreeContainerElementUI *>(m_pListDataSource->listItemForIndex(m_nTopIndex));
							if (pListItem == NULL)
							{
								return false;
							}
							AddAt(pListItem, 0);
						}
					}
				}
			}
		}

		m_pTreeUI->SetPos(m_pTreeUI->GetPos());
		m_pTreeUI->SetVerticalScrollPos(m_nTopIndex * m_nItemHeight);
		m_aSelItems.Empty();
		m_bSelTopIndex = false;
		Invalidate();
		return true;
	}


	int CTreeControlUI::CurSelItemIndex()
	{
		if (m_pListDataSource)
	   { 
		   return m_pListDataSource->CurSelectItem();
	   }
		return -1;

	}


	//拖动时，取消选择的项目
	bool CTreeControlUI::UnSelectMoveTemp()
	{
		int iIndex = m_uMoveSelectIndex;
		m_uMoveSelectIndex = -1;
		if (iIndex < 0) return false;
		CControlUI* pControl = GetItemAt(iIndex);
		if (pControl == NULL) return false;
		if (!pControl->IsVisible()) return false;
		if (!pControl->IsEnabled()) return false;
		IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
		if (pListItem == NULL) return false;

		// 	iIndex += m_nTopIndex;
		// 	int aIndex = m_aSelItems.Find((LPVOID)iIndex);
		// 	if (aIndex >= 0)
		// 		return false;
		if (!pListItem->Select(false)) {
			return false;
		}
		return true;
	}


	//拖动时，选择的项目
	bool CTreeControlUI::SelectMoveTemp(int iIndex)
	{
		if (m_bSingleSel)
		{
			return SelectItem(iIndex, false);
		}

		if (m_uMoveSelectIndex != iIndex)
		{
			UnSelectMoveTemp();
		}
		if (iIndex < 0) return false;
		CControlUI* pControl = GetItemAt(iIndex);
		if (pControl == NULL) return false;
		if (!pControl->IsVisible()) return false;
		if (!pControl->IsEnabled()) return false;

		// 	if (m_aSelItems.Find((LPVOID)(iIndex + m_nTopIndex)) >= 0)
		// 		return false;

		IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
		if (pListItem == NULL) return false;
		if (!pListItem->Select(true))
		{
			return false;
		}
		if (m_uMoveSelectIndex != iIndex)
		{
			m_uMoveSelectIndex = iIndex;
			if (m_pManager != NULL) {
				m_pManager->SetTimer(this, EXPAND_TIMER, 1500);
				m_pManager->SendNotify(this, _T("itemmove"), DUILIB_LIST_ITEM_MOVE, iIndex);
			}
		}
		return true;
	}

	bool CTreeControlUI::SelectItem(int iIndex, bool bTakeFocus /*= false*/)
	{
		if (iIndex < 0) return false;
		CControlUI* pControl = GetItemAt(iIndex);
		if (pControl == NULL) return false;
		if (!pControl->IsVisible()) return false;
		if (!pControl->IsEnabled()) return false;

		if (bTakeFocus == false)
		{
			m_nFocusItem = iIndex + m_nTopIndex;
			m_nStartSelItem = m_nEndSelItem = iIndex + m_nTopIndex;
		} 

		if (m_bSingleSel && m_aSelItems.GetSize() > 0) {
			CControlUI* pControl = GetItemAt((int)m_aSelItems.GetAt(0) - m_nTopIndex);
			if (pControl != NULL) {
				IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
				if (pListItem != NULL) pListItem->Select(false);
			}
		}

		UnSelectAllItems();

		IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
		if (pListItem == NULL) return false;
		if (!pListItem->Select(true)) {
			return false;
		}

		m_aSelItems.Add((LPVOID)(m_nTopIndex + iIndex));
		//通知数据源 选中某个元素
		m_pListDataSource->SelectItem(m_nTopIndex + iIndex);
		m_bSelTopIndex = (iIndex == 0) ? true : m_bSelTopIndex;

		//	EnsureVisible(iIndex);
		if (bTakeFocus)
			SetFocus();
		if (m_pManager != NULL) {
			m_pManager->SendNotify(this, _T("itemselect"), DUILIB_LIST_ITEMSELECT, iIndex);
		}

		return true;
	}

	bool CTreeControlUI::SelectMultiItem(int iIndex, bool bTakeFocus /*= false*/, bool bSendMessge/* = true*/)
	{
		if (m_bSingleSel)
		{
			return SelectItem(iIndex, bTakeFocus);
		}

		if (iIndex < 0) return false;
		CControlUI* pControl = GetItemAt(iIndex);
		if (pControl == NULL) return false;
		if (!pControl->IsVisible()) return false;
		if (!pControl->IsEnabled()) return false;

		if (bTakeFocus == false)
		{
			m_nFocusItem = iIndex + m_nTopIndex;
			m_nStartSelItem = m_nEndSelItem = iIndex + m_nTopIndex;
		}

		if (m_aSelItems.Find((LPVOID)(iIndex + m_nTopIndex)) >= 0)
			return false;

		if (m_bSingleSel && m_aSelItems.GetSize() > 0) {
			CControlUI* pControl = GetItemAt((int)m_aSelItems.GetAt(0));
			if (pControl != NULL) {
				IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
				if (pListItem != NULL) pListItem->Select(false);
			}
		}

		IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
		if (pListItem == NULL) return false;
		if (!pListItem->Select(true))
		{
			return false;
		}

		m_aSelItems.Add((LPVOID)(iIndex + m_nTopIndex));
		m_bSelTopIndex = (iIndex == 0) ? true : m_bSelTopIndex;

		//	EnsureVisible(iIndex);
		if (bTakeFocus) pControl->SetFocus();
		if (m_pManager != NULL) {
			m_pManager->SendNotify(this, _T("itemselect"), DUILIB_LIST_ITEMSELECT, iIndex + m_nTopIndex);
		}

		return true;
	}

	bool CTreeControlUI::SetItemSelect(int nStart, int nEnd, bool bTakeFocus /*= false*/)
	{
		if (m_bSingleSel)
		{
			return false;
		}

		int nStartTemp = nStart;
		int nEndTemp = nEnd;
		if (nStart > nEnd)
		{
			int nTemp = nStartTemp;
			nStartTemp = nEndTemp;
			nEndTemp = nTemp;
		}

		//判断数据有效性
		if (nStartTemp < 0)
		{
			return false;
		}
		if (nStartTemp >= m_nItems)
		{
			nStartTemp = m_nItems - 1;
		}

		if (nEndTemp >= m_nItems)
		{
			nEndTemp = m_nItems - 1;
		}

		UnSelectAllItems();

		//记录所有选中项
		for (int iIndex = nStartTemp; iIndex <= nEndTemp; iIndex++)
		{
			m_aSelItems.Add((LPVOID)iIndex);
			m_bSelTopIndex = (iIndex == 0) ? true : m_bSelTopIndex;
		}

		//更新当前选中显示项状态
		if (nStart > nEnd)
		{
			int npos = nEnd - m_nTopIndex;  //计算开始位置
			if (npos < 0)
			{
				npos = 0;
			}

			int nStep = nStart - m_nTopIndex;   //计算结束位置
			if (nStep > m_nShowItems)
			{
				nStep = m_nShowItems;
			}

			for (int i = npos; i <= nStep; i++)
			{
				CControlUI* pControl = GetItemAt(i);
				if (pControl == NULL) continue;
				if (!pControl->IsVisible()) continue;
				if (!pControl->IsEnabled()) continue;

				IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
				if (pListItem == NULL)
					continue;
				if (!pListItem->Select(true, false))
				{
					continue;
				}
			}
		}
		else
		{
			int nStep = nEnd - m_nTopIndex;   //计算步长
			if (nStep > m_nShowItems)
			{
				nStep = m_nShowItems;
			}

			int nPosStart = nStart - m_nTopIndex;
			if (nPosStart < 0)    //已经走过了nStart项
			{
				nPosStart = 0;
			}

			for (int i = nPosStart; i <= nStep; i++)
			{
				CControlUI* pControl = GetItemAt(i);
				if (pControl == NULL) continue;
				if (!pControl->IsVisible()) continue;
				if (!pControl->IsEnabled()) continue;


				IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
				if (pListItem == NULL)
					continue;
				if (!pListItem->Select(true, false))
				{
					continue;
				}
			}
		}

		//消息发送
		if (m_pManager != NULL) {
			m_pManager->SendNotify(this, _T("itemselect"), DUILIB_LIST_ITEMSELECT, (WPARAM)m_aSelItems.GetAt(0));
		}

		//焦点跟随
		if (bTakeFocus)
		{
			m_nFocusItem = nEnd;
			CControlUI* pControl = GetItemAt(nEnd - m_nTopIndex);
			if (pControl == NULL) return false;
			if (!pControl->IsVisible()) return false;
			if (!pControl->IsEnabled()) return false;
			pControl->SetFocus();
		}

		Invalidate();

		return true;
	}

	void CTreeControlUI::SetSingleSelect(bool bSingleSel)
	{
		m_bSingleSel = bSingleSel;
		UnSelectAllItems();
	}

	bool CTreeControlUI::GetSingleSelect() const
	{
		return m_bSingleSel;
	}

	bool CTreeControlUI::UnSelectItem(int iIndex, bool bSendMessge/* = true*/)
	{
		if (iIndex < 0) return false;
		CControlUI* pControl = GetItemAt(iIndex);
		if (pControl == NULL) return false;
		if (!pControl->IsVisible()) return false;
		if (!pControl->IsEnabled()) return false;
		IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
		if (pListItem == NULL) return false;

		iIndex += m_nTopIndex;
		int aIndex = m_aSelItems.Find((LPVOID)iIndex);
		if (aIndex < 0)
			return false;

		// 	m_nFocusItem = aIndex;
		// 	m_nStartSelItem = m_nEndSelItem = aIndex;

		if (!pListItem->Select(false)) {
			return false;
		}

		m_aSelItems.Remove(aIndex);
		m_bSelTopIndex = (iIndex == 0) ? false : m_bSelTopIndex;
		return true;
	}

	void CTreeControlUI::SelectAllItems()
	{
		UnSelectAllItems();
		CControlUI* pControl;

		for (int i = 0; i < GetCount(); ++i)   //更新当前所有项的状态
		{
			pControl = GetItemAt(i);
			if (pControl == NULL)
				continue;
			if (!pControl->IsVisible())
				continue;
			if (!pControl->IsEnabled())
				continue;
			IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
			if (pListItem == NULL)
				continue;
			if (!pListItem->Select(true))
				continue;
		}

		for (int i = 0; i < m_nItems; i++)     //所有可选
		{
			m_aSelItems.Add((LPVOID)i);
		}
		m_bSelTopIndex = true;
		m_nFocusItem = 0;
		m_nStartSelItem = m_nEndSelItem = 0;
	}

	void CTreeControlUI::UnSelectAllItems()
	{
		CControlUI* pControl;
		for (int i = 0; !m_bSingleSel && i < GetCount(); ++i)    //更新当前项状态为不可选
		{
			pControl = GetItemAt(i);
			if (pControl == NULL)
				continue;
			if (!pControl->IsVisible())
				continue;
			if (!pControl->IsEnabled())
				continue;
			IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
			if (pListItem == NULL)
				continue;
			if (!pListItem->Select(false))
				continue;
		}
		if (m_pListDataSource)
		{
			m_pListDataSource->UnSelectAllItems();
		}
		m_aSelItems.Empty();
		m_bSelTopIndex = false;
	}

	int CTreeControlUI::GetSelectItemCount() const
	{
		return m_aSelItems.GetSize();
	}

	int CTreeControlUI::GetNextSelItem(int nItem) const
	{
		if (m_aSelItems.GetSize() <= 0)
			return -1;

		if (nItem < 0)
		{
			return (int)m_aSelItems.GetAt(0);
		}
		int aIndex = m_aSelItems.Find((LPVOID)nItem);
		if (aIndex < 0)
			return -1;
		if (aIndex + 1 > m_aSelItems.GetSize() - 1)
			return -1;
		return (int)m_aSelItems.GetAt(aIndex + 1);
	}

	int CTreeControlUI::SetFocusItem(int nItem, bool bTakeFocus /*= false*/)
	{
		if ((nItem < 0) || (nItem >(GetCount() - 1)))
		{
			return  -1;
		}
		int oldItem = m_nFocusItem;
		m_nFocusItem = nItem + m_nTopIndex;
		if (bTakeFocus == false)
		{
			m_nEndSelItem = m_nFocusItem;
			SetItemSelect(m_nStartSelItem, m_nEndSelItem, true);
		}
		return oldItem;
	}

	int CTreeControlUI::SetEndItem(int nItem, bool bTakeFocus /*= false*/)
	{
		if ((nItem < 0) || (nItem >(GetCount() - 1)))
		{
			if (m_nStartSelItem > GetCount() - 1)
			{
				UnSelectAllItems();
			}
			return  -1;
		}
		int oldItem = m_nEndSelItem;

		m_nEndSelItem = nItem + m_nTopIndex;

		if (bTakeFocus)
		{
			m_nFocusItem = nItem + m_nTopIndex;
		}
		else
		{
			if (IsHiddenItemInTop() && (nItem != 0))
			{
				m_nEndSelItem = nItem + m_nTopIndex + 1;
			}
			else if (IsHiddenItemInBottom())
			{
				m_nEndSelItem = nItem + m_nTopIndex;
			}
		}
		SetItemSelect(m_nStartSelItem, m_nEndSelItem, true);
		return oldItem;
	}

	int CTreeControlUI::GetEndItem() const
	{
		return m_nEndSelItem;
	}

	bool CTreeControlUI::SetLButtonState(bool bDown)
	{
		bool bOldState = m_bLButtonDown;
		m_bLButtonDown = bDown;
		return bOldState;
	}

	bool CTreeControlUI::GetLButtonState() const
	{
		return m_bLButtonDown;
	}

	bool CTreeControlUI::SetLButton_ClickData(bool bDown)
	{
		bool bOldState = m_bLButtonDown_ClickInData;
		m_bLButtonDown_ClickInData = bDown;
		if (m_bLButtonDown_ClickInData == false)
		{
			//CloseMoveWindow();
			m_bLButtonDown = false;
			//UnSelectMoveTemp();
			//m_bHasMoveItem = false;
		}
		return bOldState;
	}

	bool CTreeControlUI::GetLButtonClickData() const
	{
		return m_bLButtonDown_ClickInData;
	}

	void CTreeControlUI::SetStartPoint(POINT pt)
	{
		m_pStartPoint = pt;
		m_pEndPoint = m_pStartPoint;
		m_pTempPoint = m_pEndPoint;
		m_StartPos = GetScrollPos();
	}

	 

	DuiLib::CStdPtrArray CTreeControlUI::GetSelArray() const
	{
		return m_aSelItems;
	}

	bool CTreeControlUI::NeedFreshData(bool bFRresh /*= true*/)
	{
		bool bold = m_bNeedRefresh;
		m_bNeedRefresh = bFRresh;
		return bold;
	}

	int CTreeControlUI::FindIndexbyPoint(POINT &pt)
	{
		int nSelIndex = -1;
		for (int i = 0; i < GetCount(); i++)
		{
			CTreeContainerElementUI *pcontrol = static_cast<CTreeContainerElementUI *>(GetItemAt(i));
			if (pcontrol == NULL)
			{
				continue;
			}
			RECT rc = pcontrol->GetPos();
			RECT rccilent = { 0, 0, 0, 0 };
			if (m_pTreeUI)
			{
				rccilent = m_pTreeUI->GetPos();
			}
			rc.right = rccilent.right;
			if (::PtInRect(&rc, pt))
			{
				nSelIndex = i;
				break;
			}
		}

		return nSelIndex;
	}

	void CTreeControlUI::SetAllNeedHeight(int nHeight)
	{
		if (m_pTreeUI && (nHeight >= 0))
		{
			m_pTreeUI->SetAllNeedHeight(nHeight);
		}
	}

	void CTreeControlUI::SetVerticalScrollPos(int nPos)
	{
		if (m_pTreeUI && (nPos >= 0))
		{
			m_pTreeUI->SetVerticalScrollPos(nPos);
		}
	}

	bool CTreeControlUI::IsSelectTopIndex()
	{
		return m_bSelTopIndex;
	}

	void CTreeControlUI::Init()
	{
		if (m_pListDataSource != NULL)
		{
			SetItemCount(m_pListDataSource->numberOfRows());
			CalShowItemNum();
			if (m_nItems > m_nShowItems)   //如果显示不全
			{
				m_pTreeUI->SetAllNeedHeight(m_nItems * m_nItemHeight);
			}
			else
				m_pTreeUI->SetAllNeedHeight(0);
		}
	}

	int CTreeControlUI::GetMinSelItemIndex()
	{
		if (m_aSelItems.GetSize() <= 0)
			return -1;
		int min = (int)m_aSelItems.GetAt(0);
		int index;
		for (int i = 0; i < m_aSelItems.GetSize(); ++i)
		{
			index = (int)m_aSelItems.GetAt(i);
			if (min > index)
				min = index;
		}
		return min;
	}

	int CTreeControlUI::GetMaxSelItemIndex()
	{
		if (m_aSelItems.GetSize() <= 0)
			return -1;
		int max = (int)m_aSelItems.GetAt(0);
		int index;
		for (int i = 0; i < m_aSelItems.GetSize(); ++i)
		{
			index = (int)m_aSelItems.GetAt(i);
			if (max < index)
				max = index;
		}
		return max;
	}


	CTreeBodyUI::CTreeBodyUI(CTreeControlUI* pOwner) : m_pOwner(pOwner)
	{
		ASSERT(m_pOwner);
		SetInset(CRect(0,0,0,0));
		m_nLeavePos = 0;
	}

	LPCTSTR CTreeBodyUI::GetClass()
	{
		if (m_pOwner)
		{
			return m_pOwner->GetClass();
		}
		return NULL;
	}

	void CTreeBodyUI::SetScrollPos(SIZE szPos)
	{
		int cx = 0;
		int cy = 0;
		int m_nRange = 0;
		if (m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) {
			int iLastScrollPos = m_pVerticalScrollBar->GetScrollPos();
			m_pVerticalScrollBar->SetScrollPos(szPos.cy);
			cy = m_pVerticalScrollBar->GetScrollPos() - iLastScrollPos;
			m_nRange = m_pVerticalScrollBar->GetScrollRange();
		}

		if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) {
			int iLastScrollPos = m_pHorizontalScrollBar->GetScrollPos();
			m_pHorizontalScrollBar->SetScrollPos(szPos.cx);
			cx = m_pHorizontalScrollBar->GetScrollPos() - iLastScrollPos;
		}

		if (cx == 0 && cy == 0) return;
		//纵向计算滚动
		bool bEnd = false;
		int nPos = szPos.cy;
		if (szPos.cy < 0)
		{
			nPos = 0;
		}
		if (szPos.cy > m_nRange)
			nPos = m_nRange;

		int nItemHeight = 0;

		if (m_pOwner)
		{
			nItemHeight = m_pOwner->GetItemHeight();
			int nStep = nPos / nItemHeight;
			int noldIndex = m_pOwner->GetTopIndex();
			if (nPos%nItemHeight != 0)   //校正数据
			{
				if (nPos == m_nRange)
				{
					m_nLeavePos = nPos%nItemHeight;
				}
				else
					m_nLeavePos = 0;
				nStep++;
			}
			m_pOwner->RefreshListData(nStep);
		}

		//计算位置
		cy = 0;
		RECT rcPos;
		for (int it2 = 0; it2 < m_items.GetSize(); it2++) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it2]);
			if (!pControl->IsVisible()) continue;
			if (pControl->IsFloat()) continue;

			rcPos = pControl->GetPos();
			rcPos.left -= cx;
			rcPos.right -= cx;
			rcPos.top -= cy;
			rcPos.bottom -= cy;
			pControl->SetPos(rcPos);
		}

		Invalidate();
	}

	void CTreeBodyUI::SetScrollLinePos(SIZE szPos, bool bScrollItem /*= true*/)
	{
		int cx = 0;
		int cy = 0;
		if (m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) {
			int iLastScrollPos = m_pVerticalScrollBar->GetScrollPos();
			m_pVerticalScrollBar->SetScrollPos(szPos.cy);
			cy = m_pVerticalScrollBar->GetScrollPos() - iLastScrollPos;
		}

		if ( /*cx == 0 &&*/ cy == 0) return;

		if (!bScrollItem)
		{
			Invalidate();
			return;
		}

		//计算位置
		RECT rcPos;
		for (int it2 = 0; it2 < m_items.GetSize(); it2++) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it2]);
			if (!pControl->IsVisible()) continue;
			if (pControl->IsFloat()) continue;

			rcPos = pControl->GetPos();
			rcPos.left -= cx;
			rcPos.right -= cx;
			rcPos.top -= cy;
			rcPos.bottom -= cy;
			pControl->SetPos(rcPos);
		}

		Invalidate();
	}

	void CTreeBodyUI::SetPos(RECT rc)
	{
		CControlUI::SetPos(rc);
		rc = m_rcItem;

		// Adjust for inset
		rc.left += m_rcInset.left;
		rc.top += m_rcInset.top;
		rc.right -= m_rcInset.right;
		rc.bottom -= m_rcInset.bottom;
		if (m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) rc.right -= m_pVerticalScrollBar->GetFixedWidth();
		if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();

		// Determine the minimum size
		SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };
		if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible())
			szAvailable.cx += m_pHorizontalScrollBar->GetScrollRange();

		int cxNeeded = 0;
		int nAdjustables = 0;
		int cyFixed = 0;
		int nEstimateNum = 0;
		for (int it1 = 0; it1 < m_items.GetSize(); it1++) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it1]);
			if (!pControl->IsVisible()) continue;
			if (pControl->IsFloat()) continue;
			SIZE sz = pControl->EstimateSize(szAvailable);
			if (sz.cy == 0) {
				nAdjustables++;
			}
			else {
				if (sz.cy < pControl->GetMinHeight()) sz.cy = pControl->GetMinHeight();
				if (sz.cy > pControl->GetMaxHeight()) sz.cy = pControl->GetMaxHeight();
			}
			cyFixed += sz.cy + pControl->GetPadding().top + pControl->GetPadding().bottom;

			RECT rcPadding = pControl->GetPadding();
			sz.cx = MAX(sz.cx, 0);
			if (sz.cx < pControl->GetMinWidth()) sz.cx = pControl->GetMinWidth();
			if (sz.cx > pControl->GetMaxWidth()) sz.cx = pControl->GetMaxWidth();
			cxNeeded = MAX(cxNeeded, sz.cx);
			nEstimateNum++;
		}
		cyFixed += (nEstimateNum - 1) * m_iChildPadding;


		// Place elements
		int cyNeeded = 0;
		int cyExpand = 0;
		if (nAdjustables > 0) cyExpand = MAX(0, (szAvailable.cy - cyFixed) / nAdjustables);
		// Position the elements
		SIZE szRemaining = szAvailable;
		int iPosY = rc.top - m_nLeavePos;
		// 	if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
		// 		iPosY -= m_pVerticalScrollBar->GetScrollPos();
		// 	}
		int iPosX = rc.left;
		if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) {
			iPosX -= m_pHorizontalScrollBar->GetScrollPos();
		}
		int iAdjustable = 0;
		int cyFixedRemaining = cyFixed;
		for (int it2 = 0; it2 < m_items.GetSize(); it2++) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it2]);
			if (!pControl->IsVisible()) continue;
			if (pControl->IsFloat()) {
				SetFloatPos(it2);
				continue;
			}

			RECT rcPadding = pControl->GetPadding();
			szRemaining.cy -= rcPadding.top;
			SIZE sz = pControl->EstimateSize(szRemaining);
			if (sz.cy == 0) {
				iAdjustable++;
				sz.cy = cyExpand;
				// Distribute remaining to last element (usually round-off left-overs)
				if (iAdjustable == nAdjustables) {
					sz.cy = MAX(0, szRemaining.cy - rcPadding.bottom - cyFixedRemaining);
				}
				if (sz.cy < pControl->GetMinHeight()) sz.cy = pControl->GetMinHeight();
				if (sz.cy > pControl->GetMaxHeight()) sz.cy = pControl->GetMaxHeight();
			}
			else {
				if (sz.cy < pControl->GetMinHeight()) sz.cy = pControl->GetMinHeight();
				if (sz.cy > pControl->GetMaxHeight()) sz.cy = pControl->GetMaxHeight();
				cyFixedRemaining -= sz.cy;
			}

			sz.cx = MAX(cxNeeded, szAvailable.cx - rcPadding.left - rcPadding.right);

			if (sz.cx < pControl->GetMinWidth()) sz.cx = pControl->GetMinWidth();
			if (sz.cx > pControl->GetMaxWidth()) sz.cx = pControl->GetMaxWidth();

			RECT rcCtrl = { iPosX + rcPadding.left, iPosY + rcPadding.top, iPosX + rcPadding.left + sz.cx, iPosY + sz.cy + rcPadding.top + rcPadding.bottom };
			pControl->SetPos(rcCtrl);

			iPosY += sz.cy + m_iChildPadding + rcPadding.top + rcPadding.bottom;
			cyNeeded += sz.cy + rcPadding.top + rcPadding.bottom;
			szRemaining.cy -= sz.cy + m_iChildPadding + rcPadding.bottom;
		}
		cyNeeded += nEstimateNum * m_iChildPadding;

		if (m_pHorizontalScrollBar != NULL) {
			if (cxNeeded > rc.right - rc.left) {
				if (m_pHorizontalScrollBar->IsVisible()) {
					m_pHorizontalScrollBar->SetScrollRange(cxNeeded - (rc.right - rc.left));
				}
				else {
					m_pHorizontalScrollBar->SetVisible(true);
					m_pHorizontalScrollBar->SetScrollRange(cxNeeded - (rc.right - rc.left));
					m_pHorizontalScrollBar->SetScrollPos(0);
					rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
				}
			}
			else {
				if (m_pHorizontalScrollBar->IsVisible()) {
					m_pHorizontalScrollBar->SetVisible(false);
					m_pHorizontalScrollBar->SetScrollRange(0);
					m_pHorizontalScrollBar->SetScrollPos(0);
					rc.bottom += m_pHorizontalScrollBar->GetFixedHeight();
				}
			}
		}

		// Process the scrollbar
		ProcessScrollBar(rc, cxNeeded, cyNeeded);
	}

	void CTreeBodyUI::DoEvent(TEventUI& event)
	{
		if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND) {
			if (m_pOwner != NULL) m_pOwner->DoEvent(event);
			else CControlUI::DoEvent(event);
			return;
		}

		if (m_pOwner != NULL) m_pOwner->DoEvent(event); else CControlUI::DoEvent(event);
	}

	void CTreeBodyUI::SetVerticalScrollPos(int nPos)
	{
		if (m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible())
		{
			m_pVerticalScrollBar->SetScrollPos(nPos);
		}
	}

	int CTreeBodyUI::GetVerticalFixWidth()
	{
		if (m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible())
			return m_pVerticalScrollBar->GetFixedWidth();
		return 0;
	}

	void CTreeBodyUI::SetVerticalScrollRange(int nRange)
	{
		if (m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible())
		{
			m_pVerticalScrollBar->SetScrollPos(nRange);
		}
	}

	void CTreeBodyUI::LineDown()
	{
		m_nLeavePos = 0;
		bool bRefreshData = true;
		if (m_pOwner)
			bRefreshData = m_pOwner->ScrollDownLine();

		int cyLine = 8;
		if (m_pManager)
			cyLine = m_pManager->GetDefaultFontInfo()->tm.tmHeight + 8;

		if (m_pOwner)
			cyLine = m_pOwner->GetItemHeight();

		SIZE sz = GetScrollPos();
		UINT cy = 0;
		if (bRefreshData)
		{
			cy = GetHiddenItemInBootom();
		}

		if (cy == 0)
		{
			sz.cy += cyLine;
		}
		else
			sz.cy += cy;

		SetScrollLinePos(sz, bRefreshData);
	}

	void CTreeBodyUI::LineUp()
	{
		m_nLeavePos = 0;

		int nTopIndex = 0;
		if (m_pOwner)
		{
			nTopIndex = m_pOwner->GetTopIndex();
		}
		UINT nRes;
		if (m_pOwner)
			nRes = m_pOwner->ScrollUpLine();

		int nTopIndex2 = -1;
		if (m_pOwner)
		{
			nTopIndex2 = m_pOwner->GetTopIndex();
		}

		if ((nTopIndex2 == nTopIndex) && (nRes == 0) && (nTopIndex2 != 0))
		{
			return;
		}

		int cyLine = 8;
		if (m_pManager)
			cyLine = m_pManager->GetDefaultFontInfo()->tm.tmHeight + 8;
		if (m_pOwner)
			cyLine = m_pOwner->GetItemHeight();

		if (nRes != 0)
		{
			cyLine = nRes;
		}

		SIZE sz = GetScrollPos();
		sz.cy -= cyLine;
		SetScrollLinePos(sz);
	}

	void CTreeBodyUI::PageUp()
	{
		int iOffset = m_rcItem.bottom - m_rcItem.top - m_rcInset.top - m_rcInset.bottom;

		SIZE sz = GetScrollPos();
		if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible())
			iOffset -= m_pHorizontalScrollBar->GetFixedHeight();
		sz.cy -= iOffset;
		SetScrollPos(sz);
	}

	void CTreeBodyUI::PageDown()
	{
		SIZE sz = GetScrollPos();
		int iOffset = m_rcItem.bottom - m_rcItem.top - m_rcInset.top - m_rcInset.bottom;
		if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible())
			iOffset -= m_pHorizontalScrollBar->GetFixedHeight();
		sz.cy += iOffset;
		SetScrollPos(sz);
	}

	void CTreeBodyUI::HomeUp()
	{
		SIZE sz = GetScrollPos();
		sz.cy = 0;
		SetScrollPos(sz);
	}

	void CTreeBodyUI::EndDown()
	{
		SIZE sz = GetScrollPos();
		sz.cy = GetScrollRange().cy;
		SetScrollPos(sz);
	}

	bool CTreeBodyUI::IsHiddenItemInTop()
	{
		if (m_items.GetSize() <= 0)
		{
			return false;
		}

		CControlUI* pControl = static_cast<CControlUI*>(m_items[0]);
		if (pControl == NULL)
		{
			return false;
		}
		if (!pControl->IsVisible())
		{
			int i = 0;
		}
		if (pControl->IsFloat())
			return false;
		RECT rect;
		rect = pControl->GetPos();
		if ((rect.bottom == rect.top) || (rect.right == rect.left))
		{
			return false;
		}

		if (rect.top < m_rcItem.top)
		{
			return true;
		}
		return false;
	}

	UINT CTreeBodyUI::GetHiddenItemInTop()
	{
		if (m_items.GetSize() <= 0)
		{
			return 0;
		}

		CControlUI* pControl = static_cast<CControlUI*>(m_items[0]);
		if (pControl == NULL)
		{
			return 0;
		}
		if (!pControl->IsVisible())
		{
			int i = 0;
		}
		if (pControl->IsFloat())
			return 0;
		RECT rect;
		rect = pControl->GetPos();
		if ((rect.bottom == rect.top) || (rect.right == rect.left))
		{
			return 0;
		}
		if (rect.top < m_rcItem.top)
		{
			return m_rcItem.top - rect.top;
		}
		return 0;
	}

	bool CTreeBodyUI::IsHiddenItemInBottom()
	{
		if (m_items.GetSize() <= 0)
		{
			return false;
		}

		CControlUI* pControl = static_cast<CControlUI*>(m_items[m_items.GetSize() - 1]);
		if (pControl == NULL)
		{
			return false;
		}
		if (!pControl->IsVisible())
		{
			int i = 0;
		}
		if (pControl->IsFloat())
			return false;
		RECT rect;
		rect = pControl->GetPos();
		if (rect.bottom > m_rcItem.bottom)
		{
			return true;
		}
		return false;
	}

	UINT CTreeBodyUI::GetHiddenItemInBootom()
	{
		if (m_items.GetSize() <= 0)
		{
			return 0;
		}

		CControlUI* pControl = static_cast<CControlUI*>(m_items[m_items.GetSize() - 1]);
		if (pControl == NULL)
		{
			return 0;
		}
		if (!pControl->IsVisible())
		{
			int i = 0;
		}
		if (pControl->IsFloat())
			return 0;
		RECT rect;
		rect = pControl->GetPos();
		if (rect.bottom > m_rcItem.bottom)
		{
			return rect.bottom - m_rcItem.bottom;
		}
		return 0;
	}


 
	CTreeContainerElementUI::CTreeContainerElementUI()
		:m_iIndex(-1),
		m_pOwner(NULL),
		m_bSelected(false),
		m_uButtonState(0)
	{
		m_uScrollshowControl = -1;
		m_bNeedSelected = false;
		m_bShade = false;
		m_dwShadeColor = 0x78535353;
	}

	LPCTSTR CTreeContainerElementUI::GetClass() const
	{
		if (m_pOwner)
		{
			return m_pOwner->GetClass();
		}
		return _T("TreeContainerElementUI");
	}

	UINT CTreeContainerElementUI::GetControlFlags() const
	{
		return UIFLAG_WANTRETURN;
	}

	LPVOID CTreeContainerElementUI::GetInterface(LPCTSTR pstrName)
	{
		if (_tcscmp(pstrName, _T("ListItem")) == 0) return static_cast<IListItemUI*>(this);
		if (_tcscmp(pstrName, _T("TreeContainerElement")) == 0) return static_cast<CTreeContainerElementUI*>(this);
		return CContainerUI::GetInterface(pstrName);
	}

	int CTreeContainerElementUI::GetIndex() const
	{
		return m_iIndex;
	}

	void CTreeContainerElementUI::SetIndex(int iIndex)
	{
		m_iIndex = iIndex;
	}

	CTreeControlUI* CTreeContainerElementUI::GetOwner()
	{
		return m_pOwner;
	}

	void CTreeContainerElementUI::SetOwner(CControlUI* pOwner)
	{
		m_pOwner = static_cast<CTreeControlUI*>(pOwner->GetInterface(_T("Tree")));
	}

	void CTreeContainerElementUI::SetVisible(bool bVisible /*= true*/)
	{
		CContainerUI::SetVisible(bVisible);
		if (!IsVisible() && m_bSelected)
		{
			m_bSelected = false;
			if (m_pOwner != NULL)
				m_pOwner->SelectItem(-1);
		}
	}

	void CTreeContainerElementUI::SetEnabled(bool bEnable /*= true*/)
	{
		CControlUI::SetEnabled(bEnable);
		if (!IsEnabled()) {
			m_uButtonState = 0;
		}
	}

	bool CTreeContainerElementUI::IsSelected() const
	{
		return m_bSelected;
	}

	bool CTreeContainerElementUI::Select(bool bSelect /*= true*/, bool bInvalidate /*= true*/)
	{
		if (!IsEnabled()) return false;
		if (bSelect == m_bSelected) return true;
		m_bSelected = bSelect;
		// 	if( bSelect && m_pOwner != NULL ) 
		// 		m_pOwner->SelectItem(m_iIndex);
		if (bInvalidate)
		{
			Invalidate();
		}

		return true;
	}

	bool CTreeContainerElementUI::IsExpanded() const
	{  
		return false;
	}

	bool CTreeContainerElementUI::Expand(bool bExpand /*= true*/)
	{
		return false;
	}

	void CTreeContainerElementUI::Invalidate()
	{
		if (!IsVisible()) return;

		if (GetParent()) {
			CContainerUI* pParentContainer = static_cast<CContainerUI*>(GetParent()->GetInterface(_T("Container")));
			if (pParentContainer) {
				RECT rc = pParentContainer->GetPos();
				RECT rcInset = pParentContainer->GetInset();
				rc.left += rcInset.left;
				rc.top += rcInset.top;
				rc.right -= rcInset.right;
				rc.bottom -= rcInset.bottom;
				CScrollBarUI* pVerticalScrollBar = pParentContainer->GetVerticalScrollBar();
				if (pVerticalScrollBar && pVerticalScrollBar->IsVisible()) rc.right -= pVerticalScrollBar->GetFixedWidth();
				CScrollBarUI* pHorizontalScrollBar = pParentContainer->GetHorizontalScrollBar();
				if (pHorizontalScrollBar && pHorizontalScrollBar->IsVisible()) rc.bottom -= pHorizontalScrollBar->GetFixedHeight();

				RECT invalidateRc = m_rcItem;
				if (!::IntersectRect(&invalidateRc, &m_rcItem, &rc))
				{
					return;
				}

				CControlUI* pParent = GetParent();
				RECT rcTemp;
				RECT rcParent;
				while (pParent = pParent->GetParent())
				{
					rcTemp = invalidateRc;
					rcParent = pParent->GetPos();
					if (!::IntersectRect(&invalidateRc, &rcTemp, &rcParent))
					{
						return;
					}
				}

				if (m_pManager != NULL) m_pManager->Invalidate(invalidateRc);
			}
			else {
				CContainerUI::Invalidate();
			}
		}
		else {
			CContainerUI::Invalidate();
		}
	}

	bool CTreeContainerElementUI::Activate()
	{
		if (!CContainerUI::Activate()) return false;
		if (m_pManager != NULL) m_pManager->SendNotify(this, _T("itemactivate"), DUILIB_LIST_ITEMACTIVE, m_iIndex);
		return true;
	}

	void CTreeContainerElementUI::DoEvent(TEventUI& event)
	{
		if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND) {
			if (m_pOwner != NULL) m_pOwner->DoEvent(event);
			else CContainerUI::DoEvent(event);
			return;
		}

		if (event.Type == UIEVENT_DBLCLICK)
		{
			if (m_pOwner && !m_pOwner->IsEnabled())
			{
				return;
			}
			if (IsEnabled()) {
				if (m_pOwner != NULL)
				{
					if (!IsSelected())
						m_pOwner->SelectItem(m_iIndex);
				}
				if (m_pManager != NULL)
				{
					m_pManager->SendNotify(this, _T("itemdbclick"), DUILIB_LIST_ITEM_DOUBLE_CLICK, m_iIndex, 0, true);
				}
				Invalidate();
			}
			return;
		}
		if (event.Type == UIEVENT_KEYDOWN && IsEnabled())
		{
			if (m_pOwner && !m_pOwner->IsEnabled())
			{
				return;
			}
			if (event.chKey == VK_RETURN) {
				Activate();
				Invalidate();
				return;
			}
		}
		if (event.Type == UIEVENT_RBUTTONDOWN)
		{
			if (m_pOwner && !m_pOwner->IsEnabled())
			{
				return;
			}
			if (m_pManager != NULL)
			{
				m_pManager->SendNotify(this, _T("itemrbuttonclick"), DUILIB_LIST_ITEM_RBUTTON_CLICK, m_iIndex, 0, true);
			}
			if (m_pOwner != NULL)
			{
				if (!IsSelected())
					m_pOwner->SelectItem(m_iIndex);
			}
			Invalidate();
			return;
		}
		if (event.Type == UIEVENT_BUTTONDOWN /*|| event.Type == UIEVENT_RBUTTONDOWN */)
		{
			if (m_pOwner == NULL)
			{
				return;
			}
			if (!m_pOwner->IsEnabled())
			{
				return;
			}
			if (::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled())    //鼠标点解在区域
			{
				if (event.wParam == UISTATE_SELECTED)
				{
					m_pOwner->SelectItem(m_iIndex);
					Invalidate();
					return;
				}
				m_pManager->SendNotify(this, _T("itemclick"), DUILIB_LIST_ITEMCLICK);

				if (m_pOwner->GetSingleSelect())
				{
					if (!IsSelected())
					{
						m_pOwner->SelectItem(m_iIndex);
						m_pOwner->SetLButton_ClickData(true);
					}
				}
				else
				{

					if ((GetKeyState(VK_CONTROL) < 0))
					{
						if (IsSelected())
						{
							m_pOwner->UnSelectItem(m_iIndex);
						}
						else
						{
							m_pOwner->SelectMultiItem(m_iIndex);
						}
					}
					else if (GetKeyState(VK_SHIFT) < 0)
					{
						m_pOwner->SetEndItem(m_iIndex, true);
					}
					else
					{
						if ((m_pOwner->GetSelectItemCount() == 1 && IsSelected()))
						{
						}
						else if (!IsSelected() || ::PtInRect(&m_rcNotData, event.ptMouse))
						{
							m_pOwner->SelectItem(m_iIndex);
						}
						else
						{
							//						m_pOwner->SelectItem(m_iIndex);	
							m_bNeedSelected = true;
						}
					}
					if (!::PtInRect(&m_rcNotData, event.ptMouse))    //数据所在区域，即左边存在数据的区域
					{
						//					m_pOwner->SelectItem(m_iIndex);
						m_pOwner->SetLButton_ClickData(true);
					}
					else
					{
						m_pOwner->SetStartPoint(event.ptMouse);
						m_pOwner->SetLButtonState(true);
						m_pOwner->SetMCaptured(m_pOwner);
					}

				}
				Invalidate();
			}
			return;
		}
		if (event.Type == UIEVENT_BUTTONUP)
		{
			if (m_pOwner == NULL)
			{
				return;
			}
			if (!m_pOwner->IsEnabled())
			{
				return;
			}
			if (m_bNeedSelected && ::PtInRect(&m_rcItem, event.ptMouse) )
			{
				m_pOwner->SelectItem(m_iIndex);
			}
			m_bNeedSelected = false;
			if (IsEnabled() && m_pOwner)
			{
				m_pOwner->SetLButtonState(false);
				if (m_pOwner->GetLButtonClickData())
				{
					m_pOwner->SetLButton_ClickData(false);
					if ((m_uScrollshowControl >= 0) && (m_items.GetSize() > m_uScrollshowControl))
					{
						CListElementEx *pItems = static_cast<CListElementEx *>(m_items.GetAt(m_uScrollshowControl));
						if (pItems != NULL)
						{
							pItems->ChildVisible(false);
						}
					}
					// 				for (int i = 0; i< m_items.GetSize(); i++)
					// 				{
					// 					CListElementEx *pItems = static_cast<CListElementEx *>(m_items.GetAt(i));
					// 					if ((pItems != NULL) && (pItems->GetCount() > 0))
					// 					{
					// 						pItems->ChildVisible(false);
					// 					}					
					// 				}
				}
			}
			return;
		}
		if (event.Type == UIEVENT_MOUSEENTER)
		{
			if (m_pOwner && !m_pOwner->IsEnabled())
			{
				return;
			}
			if ((m_uScrollshowControl >= 0) && (m_items.GetSize() > m_uScrollshowControl))
			{
				CListElementEx *pItems = static_cast<CListElementEx *>(m_items.GetAt(m_uScrollshowControl));
				if (pItems != NULL)
				{
					pItems->ChildVisible(true);
				}
			}
			// 		for (int i = 0; i< m_items.GetSize(); i++)
			// 		{
			// 			CListElementEx *pItems = static_cast<CListElementEx *>(m_items.GetAt(i));
			// 			if ((pItems != NULL) && (pItems->GetCount() > 0))
			// 			{
			// 				pItems->ChildVisible(true);
			// 			}					
			// 		}
			if (!IsLButtonDown() || (!IsFocused() && !m_pOwner->IsFocused()))
			{
				m_pOwner->SetLButtonState(false);
				//			m_pOwner->SetLButton_ClickData(false);
			}
			if (IsEnabled()) {
				m_uButtonState |= UISTATE_HOT;
				Invalidate();
			}
			return;
		}
		if (event.Type == UIEVENT_MOUSELEAVE)
		{
			if (m_pOwner && !m_pOwner->IsEnabled())
			{
				return;
			}
			if ((m_uScrollshowControl >= 0) && (m_items.GetSize() > m_uScrollshowControl))
			{
				CListElementEx *pItems = static_cast<CListElementEx *>(m_items.GetAt(m_uScrollshowControl));
				if (pItems != NULL)
				{
					pItems->ChildVisible(false);
				}
			}
			// 		for (int i = 0; i< m_items.GetSize(); i++)
			// 		{
			// 			CListElementEx *pItems = static_cast<CListElementEx *>(m_items.GetAt(i));
			// 			if ((pItems != NULL) && (pItems->GetCount() > 0))
			// 			{
			// 				pItems->ChildVisible(false);
			// 			}					
			// 		}
			if ((m_uButtonState & UISTATE_HOT) != 0) {
				m_uButtonState &= ~UISTATE_HOT;
				Invalidate();
			}
			return;
		}

		// An important twist: The list-item will send the event not to its immediate
		// parent but to the "attached" list. A list may actually embed several components
		// in its path to the item, but key-presses etc. needs to go to the actual list.
		if (m_pOwner != NULL) m_pOwner->DoEvent(event); else CControlUI::DoEvent(event);
	}

	void CTreeContainerElementUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if (_tcscmp(pstrName, _T("selected")) == 0) Select();
		else if (_tcscmp(pstrName, _T("scrollShowControl")) == 0)
		{
			m_uScrollshowControl = _tcstol(pstrValue, NULL, 10);
		}
		else if (_tcscmp(pstrName, _T("shadecolor")) == 0)
		{
			if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetShadeColor(clrColor);
		}
		else if (_tcscmp(pstrName, _T("shadeimage")) == 0)
		{
			SetShadeImage(pstrValue);
		}
		else if (_tcscmp(pstrName, _T("shade")) == 0)
		{
			if (_tcscmp(pstrValue, _T("true")) == 0)
			{
				SetShade(true);
			}
			else
				SetShade(false);
		}
		else CContainerUI::SetAttribute(pstrName, pstrValue);
	}

	void CTreeContainerElementUI::DoPaint(HDC hDC, const RECT& rcPaint)
	{
		if (!::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem)) return;
		DrawItemBk(hDC, m_rcItem);
		CContainerUI::DoPaint(hDC, rcPaint);

		if (m_bShade)
		{
			if (!m_strShadeImage.IsEmpty())
			{
				if (!DrawImage(hDC, (LPCTSTR)m_strShadeImage))
				{
					m_strShadeImage.Empty();
				}
			}
			else
				CRenderEngine::DrawColor(hDC,m_pManager, m_rcItem, m_dwShadeColor);
		}
	}

	void CTreeContainerElementUI::DrawItemText(HDC hDC, const RECT& rcItem)
	{
		return;
	}

	void CTreeContainerElementUI::DrawItemBk(HDC hDC, const RECT& rcItem)
	{
		ASSERT(m_pOwner);
		if (m_pOwner == NULL) return;
		TListInfoUI* pInfo = m_pOwner->GetListInfo();
		DWORD iBackColor = 0;
		if (m_iIndex % 2 == 0) iBackColor = pInfo->dwBkColor;

		if ((m_uButtonState & UISTATE_HOT) != 0) {
			iBackColor = pInfo->dwHotBkColor;
		}
		if (IsSelected()) {
			iBackColor = pInfo->dwSelectedBkColor;
		}
		if (!IsEnabled()) {
			iBackColor = pInfo->dwDisabledBkColor;
		}
		if (iBackColor != 0) {
			CRenderEngine::DrawColor(hDC, m_pManager, rcItem, GetAdjustColor(iBackColor));
		}

		if (!IsEnabled()) {
			if (!pInfo->sDisabledImage.IsEmpty()) {
				if (!DrawImage(hDC, (LPCTSTR)pInfo->sDisabledImage)) pInfo->sDisabledImage.Empty();
				else return;
			}
		}
		if (IsSelected()) {
			if (!pInfo->sSelectedImage.IsEmpty()) {
				if (!DrawImage(hDC, (LPCTSTR)pInfo->sSelectedImage)) pInfo->sSelectedImage.Empty();
				else return;
			}
		}
		if ((m_uButtonState & UISTATE_HOT) != 0) {
			if (!pInfo->sHotImage.IsEmpty()) {
				if (!DrawImage(hDC, (LPCTSTR)pInfo->sHotImage)) pInfo->sHotImage.Empty();
				else return;
			}
		}
		if (!m_sBkImage.IsEmpty()) {
			if (m_iIndex % 2 == 0) {
				if (!DrawImage(hDC, (LPCTSTR)m_sBkImage)) m_sBkImage.Empty();
			}
		}

		if (m_sBkImage.IsEmpty()) {
			if (!pInfo->sBkImage.IsEmpty()) {
				if (!DrawImage(hDC, (LPCTSTR)pInfo->sBkImage)) pInfo->sBkImage.Empty();
				else return;
			}
		}

		if (pInfo->dwLineColor != 0) {
			RECT rcLine = { m_rcItem.left, m_rcItem.bottom - 1, m_rcItem.right, m_rcItem.bottom - 1 };
			CRenderEngine::DrawLine(hDC, rcLine, 1, GetAdjustColor(pInfo->dwLineColor));
		}
	}

	SIZE CTreeContainerElementUI::EstimateSize(SIZE szAvailable)
	{
		SIZE cXY = { 0, m_cxyFixed.cy };
		if (cXY.cy == 0 && m_pManager != NULL) {
			for (int it = 0; it < m_items.GetSize(); it++) {
				cXY.cy = MAX(cXY.cy, static_cast<CControlUI*>(m_items[it])->EstimateSize(szAvailable).cy);
			}
			int nMin = m_pManager->GetDefaultFontInfo()->tm.tmHeight + 8;
			cXY.cy = MAX(cXY.cy, nMin);
		}

		for (int it = 0; it < m_items.GetSize(); it++) {
			cXY.cx += static_cast<CControlUI*>(m_items[it])->EstimateSize(szAvailable).cx;
		}
		return cXY;
	}

	void CTreeContainerElementUI::SetPos(RECT rc)
	{
		if (m_pOwner == NULL)
			return;

		CControlUI::SetPos(rc);
		if (m_items.IsEmpty()) return;
		rc.left += m_rcInset.left;
		rc.top += m_rcInset.top;
		rc.right -= m_rcInset.right;
		rc.bottom -= m_rcInset.bottom;
		m_rcNotData = m_rcItem;

		for (int it = 0; it < m_items.GetSize(); it++) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it]);
			if (!pControl->IsVisible()) continue;
			if (pControl->IsFloat()) {
				SetFloatPos(it);
			}
			else {
				TListInfoUI* pInfo = m_pOwner->GetListInfo();
				if (pInfo->nColumns > 0)
				{
					RECT rcItem = { pInfo->rcColumn[it].left, rc.top, pInfo->rcColumn[it].right, rc.bottom };
					m_rcNotData.left = rcItem.right;
					pControl->SetPos(rcItem);
				}
				else
				{
					pControl->SetPos(rc);// 所有非float子控件放大到整个客户区
				}
			}
		}
	}

	void CTreeContainerElementUI::SetShade(bool bShade /*= false*/)
	{
		m_bShade = bShade;
	}

	bool CTreeContainerElementUI::GetShade() const
	{
		return m_bShade;
	}

	void CTreeContainerElementUI::SetShadeColor(DWORD dwShadeColor)
	{
		if (dwShadeColor != 0)
		{
			m_dwShadeColor = dwShadeColor;
		}
	}

	DWORD CTreeContainerElementUI::GetShadeColor() const
	{
		return m_dwShadeColor;
	}

	void CTreeContainerElementUI::SetShadeImage(CStdString strImage)
	{
		m_strShadeImage = strImage;
	}

	LPCTSTR CTreeContainerElementUI::GetShadeImage() const
	{
		return m_strShadeImage;
	}

}