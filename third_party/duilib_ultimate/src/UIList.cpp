#include "StdAfx.h"

namespace DuiLib {

/////////////////////////////////////////////////////////////////////////////////////
//
//
#define  EXPAND_TIMER 0X112

CListUI::CListUI() : m_pCallback(NULL), m_bScrollSelect(false),/* m_iCurSel(-1), */m_iExpandedItem(-1)
{
    m_pList = new CListBodyUI(this);
    m_pHeader = new CListHeaderUI;

    Add(m_pHeader);
    CVerticalLayoutUI::Add(m_pList);

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

	//add by lighten
	m_bSingleSel = false;
	m_nFocusItem = -1;
	m_nStartSelItem = 0;
	m_nEndSelItem = -1;
	m_bLButtonDown = false;
	m_uMoveSelectIndex = -1;
}

LPCTSTR CListUI::GetClass() const
{
    return _T("ListUI");
}

UINT CListUI::GetControlFlags() const
{
    return UIFLAG_TABSTOP;
}

LPVOID CListUI::GetInterface(LPCTSTR pstrName)
{
	if( _tcscmp(pstrName, _T("List")) == 0 ) return static_cast<CListUI*>(this);
    if( _tcscmp(pstrName, _T("IList")) == 0 ) return static_cast<IListUI*>(this);
    if( _tcscmp(pstrName, _T("IListOwner")) == 0 ) return static_cast<IListOwnerUI*>(this);
    return CVerticalLayoutUI::GetInterface(pstrName);
}

CControlUI* CListUI::GetItemAt(int iIndex) const
{
    return m_pList->GetItemAt(iIndex);
}

int CListUI::GetItemIndex(CControlUI* pControl) const
{
    if( pControl->GetInterface(_T("ListHeader")) != NULL ) return CVerticalLayoutUI::GetItemIndex(pControl);
    // We also need to recognize header sub-items
    if( _tcsstr(pControl->GetClass(), _T("ListHeaderItemUI")) != NULL ) return m_pHeader->GetItemIndex(pControl);

    return m_pList->GetItemIndex(pControl);
}

bool CListUI::SetItemIndex(CControlUI* pControl, int iIndex)
{
    if( pControl->GetInterface(_T("ListHeader")) != NULL ) return CVerticalLayoutUI::SetItemIndex(pControl, iIndex);
    // We also need to recognize header sub-items
    if( _tcsstr(pControl->GetClass(), _T("ListHeaderItemUI")) != NULL ) return m_pHeader->SetItemIndex(pControl, iIndex);

    int iOrginIndex = m_pList->GetItemIndex(pControl);
    if( iOrginIndex == -1 ) return false;
    if( iOrginIndex == iIndex ) return true;

    IListItemUI* pSelectedListItem = NULL;
//     if( m_iCurSel >= 0 ) pSelectedListItem = 
//         static_cast<IListItemUI*>(GetItemAt(m_iCurSel)->GetInterface(_T("ListItem")));
    if( !m_pList->SetItemIndex(pControl, iIndex) ) return false;
    int iMinIndex = min(iOrginIndex, iIndex);
    int iMaxIndex = max(iOrginIndex, iIndex);
    for(int i = iMinIndex; i < iMaxIndex + 1; ++i) {
        CControlUI* p = m_pList->GetItemAt(i);
        IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(_T("ListItem")));
        if( pListItem != NULL ) {
            pListItem->SetIndex(i);
        }
    }
//  if( m_iCurSel >= 0 && pSelectedListItem != NULL ) m_iCurSel = pSelectedListItem->GetIndex();
	UnSelectAllItems();
    return true;
}

int CListUI::GetCount() const
{
    return m_pList->GetCount();
}

bool CListUI::Add(CControlUI* pControl)
{
	if (pControl == NULL)
	{
		return false;
	}
    // Override the Add() method so we can add items specifically to
    // the intended widgets. Headers are assumed to be
    // answer the correct interface so we can add multiple list headers.
    if( pControl->GetInterface(_T("ListHeader")) != NULL ) {
        if( m_pHeader != pControl && m_pHeader->GetCount() == 0 ) {
            CVerticalLayoutUI::Remove(m_pHeader);
            m_pHeader = static_cast<CListHeaderUI*>(pControl);
        }
        m_ListInfo.nColumns = MIN(m_pHeader->GetCount(), UILIST_MAX_COLUMNS);
        return CVerticalLayoutUI::AddAt(pControl, 0);
    }
    // We also need to recognize header sub-items
    if( _tcsstr(pControl->GetClass(), _T("ListHeaderItemUI")) != NULL ) {
        bool ret = m_pHeader->Add(pControl);
        m_ListInfo.nColumns = MIN(m_pHeader->GetCount(), UILIST_MAX_COLUMNS);
        return ret;
    }
    // The list items should know about us
    IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
    if( pListItem != NULL ) {
        pListItem->SetOwner(this);
        pListItem->SetIndex(GetCount());
    }
    return m_pList->Add(pControl);
}

bool CListUI::AddAt(CControlUI* pControl, int iIndex)
{
    // Override the AddAt() method so we can add items specifically to
    // the intended widgets. Headers and are assumed to be
    // answer the correct interface so we can add multiple list headers.
    if( pControl->GetInterface(_T("ListHeader")) != NULL ) {
        if( m_pHeader != pControl && m_pHeader->GetCount() == 0 ) {
            CVerticalLayoutUI::Remove(m_pHeader);
            m_pHeader = static_cast<CListHeaderUI*>(pControl);
        }
        m_ListInfo.nColumns = MIN(m_pHeader->GetCount(), UILIST_MAX_COLUMNS);
        return CVerticalLayoutUI::AddAt(pControl, 0);
    }
    // We also need to recognize header sub-items
    if( _tcsstr(pControl->GetClass(), _T("ListHeaderItemUI")) != NULL ) {
        bool ret = m_pHeader->AddAt(pControl, iIndex);
        m_ListInfo.nColumns = MIN(m_pHeader->GetCount(), UILIST_MAX_COLUMNS);
        return ret;
    }
    if (!m_pList->AddAt(pControl, iIndex)) return false;

    // The list items should know about us
    IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
    if( pListItem != NULL ) {
        pListItem->SetOwner(this);
        pListItem->SetIndex(iIndex);
    }

    for(int i = iIndex + 1; i < m_pList->GetCount(); ++i) {
        CControlUI* p = m_pList->GetItemAt(i);
        pListItem = static_cast<IListItemUI*>(p->GetInterface(_T("ListItem")));
        if( pListItem != NULL ) {
            pListItem->SetIndex(i);
        }
    }
//     if( m_iCurSel >= iIndex ) m_iCurSel += 1;
	UnSelectAllItems();
    return true;
}

bool CListUI::Remove(CControlUI* pControl)
{
    if( pControl->GetInterface(_T("ListHeader")) != NULL ) return CVerticalLayoutUI::Remove(pControl);
    // We also need to recognize header sub-items
    if( _tcsstr(pControl->GetClass(), _T("ListHeaderItemUI")) != NULL ) return m_pHeader->Remove(pControl);

    int iIndex = m_pList->GetItemIndex(pControl);
    if (iIndex == -1) return false;

    if (!m_pList->RemoveAt(iIndex)) return false;

    for(int i = iIndex; i < m_pList->GetCount(); ++i) {
        CControlUI* p = m_pList->GetItemAt(i);
        IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(_T("ListItem")));
        if( pListItem != NULL ) {
            pListItem->SetIndex(i);
        }
    }

//     if( iIndex == m_iCurSel && m_iCurSel >= 0 ) {
//         int iSel = m_iCurSel;
//         m_iCurSel = -1;
//         SelectItem(FindSelectable(iSel, false));
//     }
//     else if( iIndex < m_iCurSel ) m_iCurSel -= 1;
	m_aSelItems.Remove(m_aSelItems.Find((LPVOID)iIndex));
    return true;
}

bool CListUI::RemoveAt(int iIndex)
{
    if (!m_pList->RemoveAt(iIndex)) return false;

    for(int i = iIndex; i < m_pList->GetCount(); ++i) {
        CControlUI* p = m_pList->GetItemAt(i);
        IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(_T("ListItem")));
        if( pListItem != NULL ) pListItem->SetIndex(i);
    }

//     if( iIndex == m_iCurSel && m_iCurSel >= 0 ) {
//         int iSel = m_iCurSel;
//         m_iCurSel = -1;
//         SelectItem(FindSelectable(iSel, false));
//     }
//     else if( iIndex < m_iCurSel ) m_iCurSel -= 1;
	m_aSelItems.Remove(m_aSelItems.Find((LPVOID)iIndex));
	return true;
}

void CListUI::RemoveAll()
{
//    m_iCurSel = -1;
	UnSelectAllItems();
    m_iExpandedItem = -1;
    m_pList->RemoveAll();
}

void CListUI::SetPos(RECT rc)
{
    CVerticalLayoutUI::SetPos(rc);
    if( m_pHeader == NULL ) return;
    // Determine general list information and the size of header columns
    m_ListInfo.nColumns = MIN(m_pHeader->GetCount(), UILIST_MAX_COLUMNS);
    // The header/columns may or may not be visible at runtime. In either case
    // we should determine the correct dimensions...

    if( !m_pHeader->IsVisible() ) {
        for( int it = 0; it < m_pHeader->GetCount(); it++ ) {
            static_cast<CControlUI*>(m_pHeader->GetItemAt(it))->SetInternVisible(true);
        }
        m_pHeader->SetPos(CRect(rc.left, 0, rc.right, 0));
    }
    int iOffset = m_pList->GetScrollPos().cx;
    for( int i = 0; i < m_ListInfo.nColumns; i++ ) {
        CControlUI* pControl = static_cast<CControlUI*>(m_pHeader->GetItemAt(i));
        if( !pControl->IsVisible() ) continue;
        if( pControl->IsFloat() ) continue;

        RECT rcPos = pControl->GetPos();
        if( iOffset > 0 ) 
		{
            rcPos.left -= iOffset;
            rcPos.right -= iOffset;
            pControl->SetPos(rcPos);
        }
        m_ListInfo.rcColumn[i] = pControl->GetPos();
    }
    if( !m_pHeader->IsVisible() ) {
        for( int it = 0; it < m_pHeader->GetCount(); it++ ) {
            static_cast<CControlUI*>(m_pHeader->GetItemAt(it))->SetInternVisible(false);
        }
    }

	//add by lighten
	if ((m_pList != NULL) && (m_ListInfo.nColumns > 0))
	{
		RECT rcHead = m_ListInfo.rcColumn[0];
		RECT rcTemp={rc.left, rcHead.bottom, rc.right, rc.bottom};
		m_pList->SetPos(rcTemp);
	}

}

void CListUI::DoEvent(TEventUI& event)
{
    if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
        if( m_pParent != NULL ) m_pParent->DoEvent(event);
        else CVerticalLayoutUI::DoEvent(event);
        return;
    }

    if( event.Type == UIEVENT_SETFOCUS ) 
    {
        m_bFocused = true;
        return;
    }
    if( event.Type == UIEVENT_KILLFOCUS ) 
    {
        m_bFocused = false;
        return;
    }
	if (event.Type == UIEVENT_BUTTONDOWN)
	{
		m_bLButtonDown = true;
		UnSelectAllItems();
		if( m_pManager != NULL ) {
			m_pManager->SendNotify(this, _T("itemunselect"), DUILIB_LIST_ITEMUNSELECT, NULL);
		}
		m_nStartSelItem = GetCount() - 1;
		if (m_nStartSelItem < 0)
		{
			m_nStartSelItem = 0;
		}
		SetMCaptured(this);
	}
	if (event.Type == UIEVENT_BUTTONUP)
	{
		m_bLButtonDown = false;
		ReleaseMCaptured();
	}
	if (event.Type == UIEVENT_MOUSEMOVE)
	{
		if (!IsLButtonDown())
			return;
		static int i =0;
		i++;
//		TCHAR wstr[256] = {0};
//		::wsprintf(wstr, _T("The mouse is  moveing, the Index is %d\n"), i);
//		OutputDebugStr(wstr);

		m_pEndPoint = event.ptMouse;
		if (m_pEndPoint.y < m_rcItem.bottom && m_pEndPoint.y > m_rcItem.top)
		{
			return;
		}
		if (m_pStartPoint.y == 0)
		{
			m_pStartPoint = m_pEndPoint;
			return;
		}
		if (((m_pEndPoint.y - m_pStartPoint.y) > 5) && (m_pEndPoint.y > m_rcItem.bottom))   //向下
		{
			if (IsFocused() && IsVisible())
			{
				if (m_nEndSelItem < (m_pList->GetCount() - 1))
				{
					m_nEndSelItem ++;
				}
 				SetItemSelect(m_nStartSelItem, m_nEndSelItem, true);
			}
			m_pStartPoint = event.ptMouse;
		}
		else if ((m_pStartPoint.y -  m_pEndPoint.y) > 5 && (m_pEndPoint.y < m_rcItem.top))   //向上
		{
			if (IsFocused() && IsVisible())
			{
				if (m_nEndSelItem > 0)
				{
					m_nEndSelItem --;
				}
				SetItemSelect(m_nStartSelItem, m_nEndSelItem, true);
			}
			m_pStartPoint = event.ptMouse;
		}
		return;
	}
	if( event.Type == UIEVENT_MOUSELEAVE )
	{
//		m_bLButtonDown = false;
		m_pStartPoint = event.ptMouse;
//		OutputDebugStr(_T("mouse has leaved the list contorl!\n"));
		return;
	}

    switch( event.Type ) {
    case UIEVENT_KEYDOWN:
        switch( event.chKey ) {
        case VK_UP:
			if (m_aSelItems.GetSize() > 0)
			{
				if ((!m_bSingleSel) && (GetKeyState(VK_SHIFT)< 0))
				{
					if (IsFocused() && IsVisible())
					{
						if (m_nFocusItem > 0)
						{
							m_nFocusItem --;
						}
						m_nEndSelItem = m_nFocusItem;
						SetItemSelect(m_nStartSelItem, m_nEndSelItem, true);
					}

				}
				else if ((!m_bSingleSel) &&(GetKeyState(VK_CONTROL) < 0))
				{
					if (IsFocused() && IsVisible())
					{
						if (m_nFocusItem > 0)
						{
							m_nFocusItem --;
						}
					}
				}
				else
				{
					int index = GetMinSelItemIndex();
					UnSelectAllItems();
					if (index < m_nFocusItem)
					{
						index = m_nFocusItem;
					}
					index = index - 1;

					if (index > 0)
					{
						SelectItem(index, true);
						m_nFocusItem = index;
						m_nStartSelItem = m_nEndSelItem = m_nFocusItem;
					}
					else
					{
						SelectItem(0, true);
						m_nFocusItem = 0;
						m_nStartSelItem = m_nEndSelItem = m_nFocusItem;

					}
				}
			}
            return;
        case VK_DOWN:
			if (m_aSelItems.GetSize() > 0)
			{
				if ((!m_bSingleSel) && (GetKeyState(VK_SHIFT) < 0))
				{
					if (IsFocused() && IsVisible())
					{
						if (m_nFocusItem < (m_pList->GetCount() - 1))
						{
							m_nFocusItem ++;
						}
						m_nEndSelItem = m_nFocusItem;
						SetItemSelect(m_nStartSelItem, m_nEndSelItem, true);
					}
				}
				else if ((!m_bSingleSel) &&((event.wKeyState & MK_CONTROL) == MK_CONTROL))
				{
					if (IsFocused() && IsVisible())
					{
						if (m_nFocusItem < (m_pList->GetCount() - 1))
						{
							m_nFocusItem ++;
						}
					}
				}
				else
				{
					int index = GetMaxSelItemIndex();
					UnSelectAllItems();
					if (index > m_nFocusItem)
					{
						index = m_nFocusItem;
					}
					index = index + 1;
					if ( (index +1) > m_pList->GetCount())
					{
						SelectItem(GetCount() - 1, true) ;
						m_nFocusItem = GetCount() - 1;
						m_nStartSelItem = m_nEndSelItem = m_nFocusItem;
					}
					else
					{
						 SelectItem(index, true);
						 m_nFocusItem = index;
						 m_nStartSelItem = m_nEndSelItem = m_nFocusItem;
					}					
				}
				Invalidate();
			}
//             SelectItem(FindSelectable(m_iCurSel + 1, true));
            return;
        case VK_PRIOR:
            PageUp();
            return;
        case VK_NEXT:
            PageDown();
            return;
        case VK_HOME:
//            SelectItem(FindSelectable(0, false));
			{
				if (m_pList->GetCount() > 0)
					SelectItem(0, true);
			}
            return;
        case VK_END:
//             SelectItem(FindSelectable(GetCount() - 1, true));
			{
				if (m_pList->GetCount() > 0)
					SelectItem(m_pList->GetCount() - 1, true);
			}
            return;
//         case VK_RETURN:
//             if( m_iCurSel != -1 ) GetItemAt(m_iCurSel)->Activate();
//             return;
// 		case 0x41:
// 			{
// 				if (!m_bSingleSel && (GetKeyState(VK_CONTROL) < 0))
// 				{
// 					SelectAllItems();
// 				}
//             }
			}
        break;
    case UIEVENT_SCROLLWHEEL:
        {
            switch( LOWORD(event.wParam) )
			{
//             case SB_LINEUP:
//                 if( m_bScrollSelect ) SelectItem(FindSelectable(m_iCurSel - 1, false));
//                 else LineUp();
//                 return;
//             case SB_LINEDOWN:
//                 if( m_bScrollSelect ) SelectItem(FindSelectable(m_iCurSel + 1, true));
//                 else LineDown();
//                 return;
				case SB_LINEUP:
					if( m_bScrollSelect && m_bSingleSel)
					{					
						SelectItem(FindSelectable((int)m_aSelItems.GetAt(0) - 1, false), true);
					}
					else LineUp();
					return;
				case SB_LINEDOWN:
					if( m_bScrollSelect && m_bSingleSel) 
					{					
						SelectItem(FindSelectable((int)m_aSelItems.GetAt(0) + 1, true), true);
					}
					else LineDown();
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

CListHeaderUI* CListUI::GetHeader() const
{
    return m_pHeader;
}

CContainerUI* CListUI::GetList() const
{
    return m_pList;
}

bool CListUI::GetScrollSelect()
{
    return m_bScrollSelect;
}

void CListUI::SetScrollSelect(bool bScrollSelect)
{
    m_bScrollSelect = bScrollSelect;
}

int CListUI::GetCurSel() const
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
//bool CListUI::SelectItem(int iIndex)
//{
// 	if( iIndex == m_iCurSel ) return true;
// 
// 	int iOldSel = m_iCurSel;
// 	// We should first unselect the currently selected item
// 	if( m_iCurSel >= 0 ) {
// 		CControlUI* pControl = GetItemAt(m_iCurSel);
// 		if( pControl != NULL) {
// 			IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
// 			if( pListItem != NULL ) pListItem->Select(false);
// 		}
// 
// 		m_iCurSel = -1;
// 	}
// 	if( iIndex < 0 ) return false;
// 
// 	CControlUI* pControl = GetItemAt(iIndex);
// 	if( pControl == NULL ) return false;
// 	if( !pControl->IsVisible() ) return false;
// 	if( !pControl->IsEnabled() ) return false;
// 
// 	IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
// 	if( pListItem == NULL ) return false;
// 	m_iCurSel = iIndex;
// 	if( !pListItem->Select(true) ) {
// 		m_iCurSel = -1;
// 		return false;
// 	}
// 	EnsureVisible(m_iCurSel);
// 	if( bTakeFocus ) pControl->SetFocus();
// 	if( m_pManager != NULL ) {
// 		m_pManager->SendNotify(this, _T("itemselect"), m_iCurSel, iOldSel);
// 	}
// 	return true;
// }

//add by lighten

int CListUI::GetMinSelItemIndex()
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

int CListUI::GetMaxSelItemIndex()
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

bool CListUI::SelectItem(int iIndex, bool bTakeFocus /* = false */)
{
	if( iIndex < 0 ) return false;
	CControlUI* pControl = GetItemAt(iIndex);
	if( pControl == NULL ) return false;
	if( !pControl->IsVisible() ) return false;
	if( !pControl->IsEnabled() ) return false;
	
	
	UnSelectAllItems();

	if (bTakeFocus == false)
	{
		m_nFocusItem = iIndex;
		m_nStartSelItem = m_nEndSelItem = iIndex;
	}

	if(m_bSingleSel && m_aSelItems.GetSize() > 0) {
		CControlUI* pControl = GetItemAt((int)m_aSelItems.GetAt(0));
		if( pControl != NULL) {
			IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
			if( pListItem != NULL ) pListItem->Select(false);
		}		
	}	

	IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
	if( pListItem == NULL ) return false;
	if( !pListItem->Select(true) ) {		
		return false;
	}

	m_aSelItems.Add((LPVOID)iIndex);

	//EnsureVisible(iIndex);
	if( bTakeFocus ) pControl->SetFocus();
	if( m_pManager != NULL ) {
		m_pManager->SendNotify(this, _T("itemselect"), DUILIB_LIST_ITEMSELECT,iIndex);
	}

	return true;
}

bool CListUI::SelectMultiItem(int iIndex, bool bTakeFocus /* = false */, bool bSendMessage/* = true*/)
{
	if (m_bSingleSel)
	{
		return SelectItem(iIndex, bTakeFocus);
	}

	if( iIndex < 0 ) return false;
	CControlUI* pControl = GetItemAt(iIndex);
	if( pControl == NULL ) return false;
	if( !pControl->IsVisible() ) return false;
	if( !pControl->IsEnabled() ) return false;

	if (bTakeFocus == false)
	{
		m_nFocusItem = iIndex;
		m_nStartSelItem = m_nEndSelItem = iIndex;
	}

	if (m_aSelItems.Find((LPVOID)iIndex) >= 0)
		return false;

	if(m_bSingleSel && m_aSelItems.GetSize() > 0) {
		CControlUI* pControl = GetItemAt((int)m_aSelItems.GetAt(0));
		if( pControl != NULL) {
			IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
			if( pListItem != NULL ) pListItem->Select(false);
		}		
	}	

	IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
	if( pListItem == NULL ) return false;
	if( !pListItem->Select(true) ) 
	{		
		return false;
	}

	m_aSelItems.Add((LPVOID)iIndex);

	//EnsureVisible(iIndex);
	if( bTakeFocus ) pControl->SetFocus();
	if( m_pManager != NULL ) {
		if (bSendMessage)
			m_pManager->SendNotify(this, _T("itemselect"), DUILIB_LIST_ITEMSELECT, iIndex);
	}

	return true;
}

int CListUI::SetFocusItem(int nItem, bool bTakeFocus /*= false*/)
{
	if ((nItem < 0) || (nItem > (GetCount() - 1)))
	{
		return  -1;
	}
	int oldItem = m_nFocusItem;
	m_nFocusItem = nItem;	
	if (bTakeFocus == false)
	{
		m_nEndSelItem = m_nFocusItem;
		SetItemSelect(m_nStartSelItem, m_nEndSelItem, true);
	}
	return oldItem;
}

//设置多选中结束项
int CListUI::SetEndItem(int nItem, bool bTakeFocus/* = false*/)
{
	if ((nItem < 0) || (nItem > (GetCount() - 1)))
	{
		return  -1;
	}
	int oldItem = m_nEndSelItem;
	m_nEndSelItem = nItem;	
	if (bTakeFocus)
	{
		m_nFocusItem =	nItem;
	}
	SetItemSelect(m_nStartSelItem, m_nEndSelItem, true);
	return oldItem;	
}

int CListUI::GetEndItem() const
{
	return m_nEndSelItem;
}

//设置鼠标是否被按下
bool CListUI::SetLButtonState(bool bDown)
{
	bool bOldState = m_bLButtonDown;
	m_bLButtonDown = bDown;
	return bOldState;
}

bool CListUI::GetLButtonState() const
{
	return m_bLButtonDown;
}

//拖动时，选择的项目
bool CListUI::SelectMoveTemp(int iIndex)
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

//拖动时，取消选择的项目
bool CListUI::UnSelectMoveTemp()
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

bool CListUI::SetItemSelect(int nStart, int nEnd, bool bTakeFocus /* = false */)
{

	int nStartTemp = nStart;
	int nEndTemp = nEnd;
	if (nStart > nEnd)
	{
		int nTemp = nStartTemp;
		nStartTemp = nEndTemp;
		nEndTemp = nTemp;
	}

	if (nStartTemp < 0)
	{
		return false;
	}

	if(m_bSingleSel )
	{
		SelectItem(nStartTemp, bTakeFocus);
	}	

	UnSelectAllItems();
	for (int iIndex = nStartTemp; iIndex <= nEndTemp; iIndex++)
	{
		CControlUI* pControl = GetItemAt(iIndex);
		if( pControl == NULL ) return false;
		if( !pControl->IsVisible() ) return false;
		if( !pControl->IsEnabled() ) return false;

		if (bTakeFocus == false)
		{
			m_nFocusItem = iIndex;
		}

		IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
		if( pListItem == NULL ) 
			return false;
		if( !pListItem->Select(true, false) ) 
		{		
			return false;
		}

		m_aSelItems.Add((LPVOID)iIndex);

		if( bTakeFocus ) pControl->SetFocus();
		if( m_pManager != NULL ) {
			m_pManager->SendNotify(this, _T("itemselect"), DUILIB_LIST_ITEMSELECT, iIndex);
		}
	}
	
// 	if (nStart < nEnd)
// 	{
		//EnsureVisible(nEnd);
// 	}
// 	else
// 	{
// 		EnsureVisible(nStart);
// 	}
	return true;
}

void CListUI::SetSingleSelect(bool bSingleSel)
{
	m_bSingleSel = bSingleSel;
	UnSelectAllItems();
}

bool CListUI::GetSingleSelect() const
{
	return m_bSingleSel;
}

bool CListUI::UnSelectItem(int iIndex, bool bSendMessage /*= true*/)
{
	if( iIndex < 0 ) return false;
	CControlUI* pControl = GetItemAt(iIndex);
	if( pControl == NULL ) return false;
	if( !pControl->IsVisible() ) return false;
	if( !pControl->IsEnabled() ) return false;
	IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
	if( pListItem == NULL ) return false;

	int aIndex = m_aSelItems.Find((LPVOID)iIndex);
	if (aIndex < 0)
		return false;

// 	m_nFocusItem = aIndex;
// 	m_nStartSelItem = m_nEndSelItem = aIndex;

	if( !pListItem->Select(false) ) {		
		return false;
	}

	m_aSelItems.Remove(aIndex);

	return true;
}

void CListUI::SelectAllItems()
{
	UnSelectAllItems();
	CControlUI* pControl;
	for (int i = 0; i < GetCount(); ++i)
	{
		pControl = GetItemAt(i);
		if(pControl == NULL)
			continue;
		if(!pControl->IsVisible())
			continue;
		if(!pControl->IsEnabled())
			continue;
		IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
		if( pListItem == NULL )
			continue;
		if( !pListItem->Select(true) )
			continue;
		m_aSelItems.Add((LPVOID)i);
	}
	m_nFocusItem = 0;
	m_nStartSelItem = m_nEndSelItem = 0;
}

void CListUI::UnSelectAllItems()
{
	CControlUI* pControl;
	int itemIndex;
	for (int i = 0; i < m_aSelItems.GetSize(); ++i)
	{
		itemIndex = (int)m_aSelItems.GetAt(i);
		pControl = GetItemAt(itemIndex);
		if(pControl == NULL)
			continue;
		if(!pControl->IsVisible())
			continue;
		if(!pControl->IsEnabled())
			continue;
		IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
		if( pListItem == NULL )
			continue;
		if( !pListItem->Select(false))
			continue;		
	}
// 	m_nFocusItem = -1;
// 	m_nStartSelItem = m_nEndSelItem = -1;
	m_aSelItems.Empty();
}

int CListUI::GetSelectItemCount() const
{
	return m_aSelItems.GetSize();
}

int CListUI::GetNextSelItem(int nItem) const
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

TListInfoUI* CListUI::GetListInfo()
{
    return &m_ListInfo;
}

int CListUI::GetChildPadding() const
{
    return m_pList->GetChildPadding();
}

void CListUI::SetChildPadding(int iPadding)
{
    m_pList->SetChildPadding(iPadding);
}

void CListUI::SetItemFont(int index)
{
    m_ListInfo.nFont = index;
    NeedUpdate();
}

void CListUI::SetItemTextStyle(UINT uStyle)
{
    m_ListInfo.uTextStyle = uStyle;
    NeedUpdate();
}

void CListUI::SetItemTextPadding(RECT rc)
{
    m_ListInfo.rcTextPadding = rc;
    NeedUpdate();
}

RECT CListUI::GetItemTextPadding() const
{
	return m_ListInfo.rcTextPadding;
}

void CListUI::SetItemTextColor(DWORD dwTextColor)
{
    m_ListInfo.dwTextColor = dwTextColor;
    Invalidate();
}

void CListUI::SetItemBkColor(DWORD dwBkColor)
{
    m_ListInfo.dwBkColor = dwBkColor;
    Invalidate();
}

void CListUI::SetItemBkImage(LPCTSTR pStrImage)
{
    m_ListInfo.sBkImage = pStrImage;
    Invalidate();
}

void CListUI::SetAlternateBk(bool bAlternateBk)
{
    m_ListInfo.bAlternateBk = bAlternateBk;
    Invalidate();
}

DWORD CListUI::GetItemTextColor() const
{
	return m_ListInfo.dwTextColor;
}

DWORD CListUI::GetItemBkColor() const
{
	return m_ListInfo.dwBkColor;
}

LPCTSTR CListUI::GetItemBkImage() const
{
	return m_ListInfo.sBkImage;
}

bool CListUI::IsAlternateBk() const
{
    return m_ListInfo.bAlternateBk;
}

void CListUI::SetSelectedItemTextColor(DWORD dwTextColor)
{
    m_ListInfo.dwSelectedTextColor = dwTextColor;
    Invalidate();
}

void CListUI::SetSelectedItemBkColor(DWORD dwBkColor)
{
    m_ListInfo.dwSelectedBkColor = dwBkColor;
    Invalidate();
}

void CListUI::SetSelectedItemImage(LPCTSTR pStrImage)
{
    m_ListInfo.sSelectedImage = pStrImage;
    Invalidate();
}

DWORD CListUI::GetSelectedItemTextColor() const
{
	return m_ListInfo.dwSelectedTextColor;
}

DWORD CListUI::GetSelectedItemBkColor() const
{
	return m_ListInfo.dwSelectedBkColor;
}

LPCTSTR CListUI::GetSelectedItemImage() const
{
	return m_ListInfo.sSelectedImage;
}

void CListUI::SetHotItemTextColor(DWORD dwTextColor)
{
    m_ListInfo.dwHotTextColor = dwTextColor;
    Invalidate();
}

void CListUI::SetHotItemBkColor(DWORD dwBkColor)
{
    m_ListInfo.dwHotBkColor = dwBkColor;
    Invalidate();
}

void CListUI::SetHotItemImage(LPCTSTR pStrImage)
{
    m_ListInfo.sHotImage = pStrImage;
    Invalidate();
}

DWORD CListUI::GetHotItemTextColor() const
{
	return m_ListInfo.dwHotTextColor;
}
DWORD CListUI::GetHotItemBkColor() const
{
	return m_ListInfo.dwHotBkColor;
}

LPCTSTR CListUI::GetHotItemImage() const
{
	return m_ListInfo.sHotImage;
}

void CListUI::SetDisabledItemTextColor(DWORD dwTextColor)
{
    m_ListInfo.dwDisabledTextColor = dwTextColor;
    Invalidate();
}

void CListUI::SetDisabledItemBkColor(DWORD dwBkColor)
{
    m_ListInfo.dwDisabledBkColor = dwBkColor;
    Invalidate();
}

void CListUI::SetDisabledItemImage(LPCTSTR pStrImage)
{
    m_ListInfo.sDisabledImage = pStrImage;
    Invalidate();
}

DWORD CListUI::GetDisabledItemTextColor() const
{
	return m_ListInfo.dwDisabledTextColor;
}

DWORD CListUI::GetDisabledItemBkColor() const
{
	return m_ListInfo.dwDisabledBkColor;
}

LPCTSTR CListUI::GetDisabledItemImage() const
{
	return m_ListInfo.sDisabledImage;
}

DWORD CListUI::GetItemLineColor() const
{
	return m_ListInfo.dwLineColor;
}

void CListUI::SetItemLineColor(DWORD dwLineColor)
{
    m_ListInfo.dwLineColor = dwLineColor;
    Invalidate();
}

bool CListUI::IsItemShowHtml()
{
    return m_ListInfo.bShowHtml;
}

void CListUI::SetItemShowHtml(bool bShowHtml)
{
    if( m_ListInfo.bShowHtml == bShowHtml ) return;

    m_ListInfo.bShowHtml = bShowHtml;
    NeedUpdate();
}

void CListUI::SetMultiExpanding(bool bMultiExpandable)
{
    m_ListInfo.bMultiExpandable = bMultiExpandable;
}

bool CListUI::ExpandItem(int iIndex, bool bExpand /*= true*/)
{
    if( m_iExpandedItem >= 0 && !m_ListInfo.bMultiExpandable) {
        CControlUI* pControl = GetItemAt(m_iExpandedItem);
        if( pControl != NULL ) {
            IListItemUI* pItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
            if( pItem != NULL ) pItem->Expand(false);
        }
        m_iExpandedItem = -1;
    }
    if( bExpand ) {
        CControlUI* pControl = GetItemAt(iIndex);
        if( pControl == NULL ) return false;
        if( !pControl->IsVisible() ) return false;
        IListItemUI* pItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
        if( pItem == NULL ) return false;
        m_iExpandedItem = iIndex;
        if( !pItem->Expand(true) ) {
            m_iExpandedItem = -1;
            return false;
        }
    }
    NeedUpdate();
    return true;
}

int CListUI::GetExpandedItem() const
{
    return m_iExpandedItem;
}

void CListUI::EnsureVisible(int iIndex)
{
    if( iIndex < 0 ) return;
    RECT rcItem = m_pList->GetItemAt(iIndex)->GetPos();
    RECT rcList = m_pList->GetPos();
    RECT rcListInset = m_pList->GetInset();

    rcList.left += rcListInset.left;
    rcList.top += rcListInset.top;
    rcList.right -= rcListInset.right;
    rcList.bottom -= rcListInset.bottom;

    CScrollBarUI* pHorizontalScrollBar = m_pList->GetHorizontalScrollBar();
    if( pHorizontalScrollBar && pHorizontalScrollBar->IsVisible() ) rcList.bottom -= pHorizontalScrollBar->GetFixedHeight();

    int iPos = m_pList->GetScrollPos().cy;
    if( rcItem.top >= rcList.top && rcItem.bottom < rcList.bottom ) return;
    int dx = 0;
    if( rcItem.top < rcList.top ) dx = rcItem.top - rcList.top;
    if( rcItem.bottom > rcList.bottom ) dx = rcItem.bottom - rcList.bottom;
    Scroll(0, dx);
}

void CListUI::Scroll(int dx, int dy)
{
    if( dx == 0 && dy == 0 ) return;
    SIZE sz = m_pList->GetScrollPos();
    m_pList->SetScrollPos(CSize(sz.cx + dx, sz.cy + dy));
}

void CListUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if( _tcscmp(pstrName, _T("header")) == 0 ) GetHeader()->SetVisible(_tcscmp(pstrValue, _T("hidden")) != 0);
    else if( _tcscmp(pstrName, _T("headerbkimage")) == 0 ) GetHeader()->SetBkImage(pstrValue);
    else if( _tcscmp(pstrName, _T("scrollselect")) == 0 ) SetScrollSelect(_tcscmp(pstrValue, _T("true")) == 0);
    else if( _tcscmp(pstrName, _T("multiexpanding")) == 0 ) SetMultiExpanding(_tcscmp(pstrValue, _T("true")) == 0);
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
    }
    else if( _tcscmp(pstrName, _T("itemendellipsis")) == 0 ) {
        if( _tcscmp(pstrValue, _T("true")) == 0 ) m_ListInfo.uTextStyle |= DT_END_ELLIPSIS;
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
	else if ( _tcscmp(pstrName, _T("singleselect")) == 0 )
	{
		SetSingleSelect(_tcscmp(pstrValue, _T("true")) == 0);
	}
    else CVerticalLayoutUI::SetAttribute(pstrName, pstrValue);
}

IListCallbackUI* CListUI::GetTextCallback() const
{
    return m_pCallback;
}

void CListUI::SetTextCallback(IListCallbackUI* pCallback)
{
    m_pCallback = pCallback;
}

SIZE CListUI::GetScrollPos() const
{
    return m_pList->GetScrollPos();
}

SIZE CListUI::GetScrollRange() const
{
    return m_pList->GetScrollRange();
}

void CListUI::SetScrollPos(SIZE szPos)
{
    m_pList->SetScrollPos(szPos);
}

void CListUI::LineUp()
{
    m_pList->LineUp();
}

void CListUI::LineDown()
{
    m_pList->LineDown();
}

void CListUI::PageUp()
{
    m_pList->PageUp();
}

void CListUI::PageDown()
{
    m_pList->PageDown();
}

void CListUI::HomeUp()
{
    m_pList->HomeUp();
}

void CListUI::EndDown()
{
    m_pList->EndDown();
}

void CListUI::LineLeft()
{
    m_pList->LineLeft();
}

void CListUI::LineRight()
{
    m_pList->LineRight();
}

void CListUI::PageLeft()
{
    m_pList->PageLeft();
}

void CListUI::PageRight()
{
    m_pList->PageRight();
}

void CListUI::HomeLeft()
{
    m_pList->HomeLeft();
}

void CListUI::EndRight()
{
    m_pList->EndRight();
}

void CListUI::EnableScrollBar(bool bEnableVertical, bool bEnableHorizontal)
{
    m_pList->EnableScrollBar(bEnableVertical, bEnableHorizontal);
}

CScrollBarUI* CListUI::GetVerticalScrollBar() const
{
    return m_pList->GetVerticalScrollBar();
}

CScrollBarUI* CListUI::GetHorizontalScrollBar() const
{
    return m_pList->GetHorizontalScrollBar();
}

bool CListUI::ExpandNode()
{
	int iIndex = m_uMoveSelectIndex;
	if (iIndex < 0) return false;
	CControlUI* pControl = GetItemAt(iIndex);
	if (pControl == NULL) return false;
	if (!pControl->IsVisible()) return false;
	if (!pControl->IsEnabled()) return false;
	IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
	if (pListItem == NULL) return false;
	if (!pListItem->IsExpanded())
	{
		pListItem->Expand(true);
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////
//
//


CListBodyUI::CListBodyUI(CListUI* pOwner) : m_pOwner(pOwner)
{
    ASSERT(m_pOwner);
    SetInset(CRect(0,0,0,0));
}

void CListBodyUI::SetScrollPos(SIZE szPos)
{
    int cx = 0;
    int cy = 0;
    if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
        int iLastScrollPos = m_pVerticalScrollBar->GetScrollPos();
        m_pVerticalScrollBar->SetScrollPos(szPos.cy);
        cy = m_pVerticalScrollBar->GetScrollPos() - iLastScrollPos;
    }

    if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) {
        int iLastScrollPos = m_pHorizontalScrollBar->GetScrollPos();
        m_pHorizontalScrollBar->SetScrollPos(szPos.cx);
        cx = m_pHorizontalScrollBar->GetScrollPos() - iLastScrollPos;
    }

    if( cx == 0 && cy == 0 ) return;

    RECT rcPos;
    for( int it2 = 0; it2 < m_items.GetSize(); it2++ ) {
        CControlUI* pControl = static_cast<CControlUI*>(m_items[it2]);
        if( !pControl->IsVisible() ) continue;
        if( pControl->IsFloat() ) continue;

        rcPos = pControl->GetPos();
        rcPos.left -= cx;
        rcPos.right -= cx;
        rcPos.top -= cy;
        rcPos.bottom -= cy;
        pControl->SetPos(rcPos);
    }

    Invalidate();

    if( cx != 0 && m_pOwner ) {
        CListHeaderUI* pHeader = m_pOwner->GetHeader();
        if( pHeader == NULL ) return;
        TListInfoUI* pInfo = m_pOwner->GetListInfo();
        pInfo->nColumns = MIN(pHeader->GetCount(), UILIST_MAX_COLUMNS);

        if( !pHeader->IsVisible() ) {
            for( int it = 0; it < pHeader->GetCount(); it++ ) {
                static_cast<CControlUI*>(pHeader->GetItemAt(it))->SetInternVisible(true);
            }
        }
        for( int i = 0; i < pInfo->nColumns; i++ ) {
            CControlUI* pControl = static_cast<CControlUI*>(pHeader->GetItemAt(i));
            if( !pControl->IsVisible() ) continue;
            if( pControl->IsFloat() ) continue;

            RECT rcPos = pControl->GetPos();
            rcPos.left -= cx;
            rcPos.right -= cx;
            pControl->SetPos(rcPos);
            pInfo->rcColumn[i] = pControl->GetPos();
        }
        if( !pHeader->IsVisible() ) {
            for( int it = 0; it < pHeader->GetCount(); it++ ) {
                static_cast<CControlUI*>(pHeader->GetItemAt(it))->SetInternVisible(false);
            }
        }
    }
}

void CListBodyUI::SetPos(RECT rc)
{
    CControlUI::SetPos(rc);
    rc = m_rcItem;

    // Adjust for inset
    rc.left += m_rcInset.left;
    rc.top += m_rcInset.top;
    rc.right -= m_rcInset.right;
    rc.bottom -= m_rcInset.bottom;
    if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) rc.right -= m_pVerticalScrollBar->GetFixedWidth();
    if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();

    // Determine the minimum size
    SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };
    if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) 
        szAvailable.cx += m_pHorizontalScrollBar->GetScrollRange();

    int cxNeeded = 0;
    int nAdjustables = 0;
    int cyFixed = 0;
    int nEstimateNum = 0;
    for( int it1 = 0; it1 < m_items.GetSize(); it1++ ) {
        CControlUI* pControl = static_cast<CControlUI*>(m_items[it1]);
        if( !pControl->IsVisible() ) continue;
        if( pControl->IsFloat() ) continue;
        SIZE sz = pControl->EstimateSize(szAvailable);
        if( sz.cy == 0 ) {
            nAdjustables++;
        }
        else {
            if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
            if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();
        }
        cyFixed += sz.cy + pControl->GetPadding().top + pControl->GetPadding().bottom;

        RECT rcPadding = pControl->GetPadding();
        sz.cx = MAX(sz.cx, 0);
        if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();
        if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();
        cxNeeded = MAX(cxNeeded, sz.cx);
        nEstimateNum++;
    }
    cyFixed += (nEstimateNum - 1) * m_iChildPadding;

    if( m_pOwner ) {
        CListHeaderUI* pHeader = m_pOwner->GetHeader();
        if( pHeader != NULL && pHeader->GetCount() > 0 ) {
            cxNeeded = MAX(0, pHeader->EstimateSize(CSize(rc.right - rc.left, rc.bottom - rc.top)).cx);
        }
    }

    // Place elements
    int cyNeeded = 0;
    int cyExpand = 0;
    if( nAdjustables > 0 ) cyExpand = MAX(0, (szAvailable.cy - cyFixed) / nAdjustables);
    // Position the elements
    SIZE szRemaining = szAvailable;
    int iPosY = rc.top;
    if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
        iPosY -= m_pVerticalScrollBar->GetScrollPos();
    }
    int iPosX = rc.left;
    if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) {
        iPosX -= m_pHorizontalScrollBar->GetScrollPos();
    }
    int iAdjustable = 0;
    int cyFixedRemaining = cyFixed;
    for( int it2 = 0; it2 < m_items.GetSize(); it2++ ) {
        CControlUI* pControl = static_cast<CControlUI*>(m_items[it2]);
        if( !pControl->IsVisible() ) continue;
        if( pControl->IsFloat() ) {
            SetFloatPos(it2);
            continue;
        }

        RECT rcPadding = pControl->GetPadding();
        szRemaining.cy -= rcPadding.top;
        SIZE sz = pControl->EstimateSize(szRemaining);
        if( sz.cy == 0 ) {
            iAdjustable++;
            sz.cy = cyExpand;
            // Distribute remaining to last element (usually round-off left-overs)
            if( iAdjustable == nAdjustables ) {
                sz.cy = MAX(0, szRemaining.cy - rcPadding.bottom - cyFixedRemaining);
            } 
            if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
            if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();
        }
        else {
            if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
            if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();
            cyFixedRemaining -= sz.cy;
        }

        sz.cx = MAX(cxNeeded, szAvailable.cx - rcPadding.left - rcPadding.right);

        if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();
        if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();

        RECT rcCtrl = { iPosX + rcPadding.left, iPosY + rcPadding.top, iPosX + rcPadding.left + sz.cx, iPosY + sz.cy + rcPadding.top + rcPadding.bottom };
        pControl->SetPos(rcCtrl);

        iPosY += sz.cy + m_iChildPadding + rcPadding.top + rcPadding.bottom;
        cyNeeded += sz.cy + rcPadding.top + rcPadding.bottom;
        szRemaining.cy -= sz.cy + m_iChildPadding + rcPadding.bottom;
    }
    cyNeeded += (nEstimateNum - 1) * m_iChildPadding;

    if( m_pHorizontalScrollBar != NULL ) {
        if( cxNeeded > rc.right - rc.left ) {
            if( m_pHorizontalScrollBar->IsVisible() ) {
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
            if( m_pHorizontalScrollBar->IsVisible() ) {
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

void CListBodyUI::DoEvent(TEventUI& event)
{
    if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
        if( m_pOwner != NULL ) m_pOwner->DoEvent(event);
        else CControlUI::DoEvent(event);
        return;
    }

    if( m_pOwner != NULL ) m_pOwner->DoEvent(event); else CControlUI::DoEvent(event);
}

/////////////////////////////////////////////////////////////////////////////////////
//
//

CListHeaderUI::CListHeaderUI()
{
}

LPCTSTR CListHeaderUI::GetClass() const
{
    return _T("ListHeaderUI");
}

LPVOID CListHeaderUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcscmp(pstrName, _T("ListHeader")) == 0 ) return this;
    return CHorizontalLayoutUI::GetInterface(pstrName);
}

SIZE CListHeaderUI::EstimateSize(SIZE szAvailable)
{
    SIZE cXY = {0, m_cxyFixed.cy};
	if( cXY.cy == 0 && m_pManager != NULL ) {
		for( int it = 0; it < m_items.GetSize(); it++ ) {
			cXY.cy = MAX(cXY.cy,static_cast<CControlUI*>(m_items[it])->EstimateSize(szAvailable).cy);
		}
		int nMin = m_pManager->GetDefaultFontInfo()->tm.tmHeight + 6;
		cXY.cy = MAX(cXY.cy,nMin);
	}

    for( int it = 0; it < m_items.GetSize(); it++ ) {
        cXY.cx +=  static_cast<CControlUI*>(m_items[it])->EstimateSize(szAvailable).cx;
    }

    return cXY;
}

/////////////////////////////////////////////////////////////////////////////////////
//
//

CListHeaderItemUI::CListHeaderItemUI() : m_bDragable(TRUE), m_uButtonState(0), m_iSepWidth(4),
m_uTextStyle(DT_VCENTER | DT_CENTER | DT_SINGLELINE), m_dwTextColor(1), m_iFont(-1), m_bShowHtml(false)
{
	SetTextPadding(CRect(2, 0, 2, 0));
    ptLastMouse.x = ptLastMouse.y = 0;
	m_szIcon.cx = m_szIcon.cy = 0;
	m_rcIconOffset.left = m_rcIconOffset.right = m_rcIconOffset.top = m_rcIconOffset.bottom = 0;
    SetMinWidth(16);
	m_bSelected = false;
	m_bShowIcon = false;
	m_bButtonStyleIcon = false;
}

LPCTSTR CListHeaderItemUI::GetClass() const
{
    return _T("ListHeaderItemUI");
}

LPVOID CListHeaderItemUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcscmp(pstrName, _T("ListHeaderItem")) == 0 ) return this;
    return CControlUI::GetInterface(pstrName);
}

UINT CListHeaderItemUI::GetControlFlags() const
{
    if( IsEnabled() && m_iSepWidth != 0 ) return UIFLAG_SETCURSOR;
    else return 0;
}

void CListHeaderItemUI::SetEnabled(bool bEnable)
{
    CControlUI::SetEnabled(bEnable);
    if( !IsEnabled() ) {
        m_uButtonState = 0;
    }
}

void CListHeaderItemUI::SetSelected(bool bSelected /* = true */, bool bInvalidate /* = false */)
{
	m_bSelected = bSelected;
	if (bInvalidate)
	{
		Invalidate();
	}
}

void CListHeaderItemUI::SetShowIcon(bool bShow /* = true */, bool bInvalidate /* = false */)
{
	m_bShowIcon = bShow;
	if (bInvalidate)
	{
		Invalidate();
	}
}

bool CListHeaderItemUI::IsDragable() const
{
	return m_bDragable;
}

void CListHeaderItemUI::SetDragable(bool bDragable)
{
    m_bDragable = bDragable;
    if ( !m_bDragable ) m_uButtonState &= ~UISTATE_CAPTURED;
}

void CListHeaderItemUI::SetButtonStyleIcon(bool bButtonStyleIcon)
{
	m_bButtonStyleIcon = bButtonStyleIcon;
}

bool CListHeaderItemUI::IsButtonStyleIcon() const
{
	return m_bButtonStyleIcon;
}


DWORD CListHeaderItemUI::GetSepWidth() const
{
	return m_iSepWidth;
}

void CListHeaderItemUI::SetSepWidth(int iWidth)
{
    m_iSepWidth = iWidth;
}

DWORD CListHeaderItemUI::GetTextStyle() const
{
	return m_uTextStyle;
}

void CListHeaderItemUI::SetTextStyle(UINT uStyle)
{
    m_uTextStyle = uStyle;
    Invalidate();
}

DWORD CListHeaderItemUI::GetTextColor() const
{
	return m_dwTextColor;
}


void CListHeaderItemUI::SetTextColor(DWORD dwTextColor)
{
    m_dwTextColor = dwTextColor;
}

RECT CListHeaderItemUI::GetTextPadding() const
{
	return m_rcTextPadding;
}

void CListHeaderItemUI::SetTextPadding(RECT rc)
{
	m_rcTextPadding = rc;
	Invalidate();
}

void CListHeaderItemUI::SetFont(int index)
{
    m_iFont = index;
}

bool CListHeaderItemUI::IsShowHtml()
{
    return m_bShowHtml;
}

void CListHeaderItemUI::SetShowHtml(bool bShowHtml)
{
    if( m_bShowHtml == bShowHtml ) return;

    m_bShowHtml = bShowHtml;
    Invalidate();
}

LPCTSTR CListHeaderItemUI::GetNormalImage() const
{
	return m_sNormalImage;
}

void CListHeaderItemUI::SetNormalImage(LPCTSTR pStrImage)
{
    m_sNormalImage = pStrImage;
    Invalidate();
}

LPCTSTR CListHeaderItemUI::GetHotImage() const
{
    return m_sHotImage;
}

void CListHeaderItemUI::SetHotImage(LPCTSTR pStrImage)
{
    m_sHotImage = pStrImage;
    Invalidate();
}

LPCTSTR CListHeaderItemUI::GetPushedImage() const
{
    return m_sPushedImage;
}

void CListHeaderItemUI::SetPushedImage(LPCTSTR pStrImage)
{
    m_sPushedImage = pStrImage;
    Invalidate();
}

LPCTSTR CListHeaderItemUI::GetFocusedImage() const
{
    return m_sFocusedImage;
}

void CListHeaderItemUI::SetFocusedImage(LPCTSTR pStrImage)
{
    m_sFocusedImage = pStrImage;
    Invalidate();
}

LPCTSTR CListHeaderItemUI::GetSepImage() const
{
    return m_sSepImage;
}

void CListHeaderItemUI::SetSepImage(LPCTSTR pStrImage)
{
    m_sSepImage = pStrImage;
    Invalidate();
}

LPCTSTR CListHeaderItemUI::GetNormalIconImage() const
{
	return m_sNormalIcon;
}

void CListHeaderItemUI::SetNormalIconImage(LPCTSTR pStrImage)
{
	m_sNormalIcon = pStrImage;
	Invalidate();
}

LPCTSTR CListHeaderItemUI::GetSelectedIconImage() const
{
	return m_sSelectedIcon;
}

void CListHeaderItemUI::SetSelectedIconImage(LPCTSTR pStrImage)
{
	m_sSelectedIcon = pStrImage;
	Invalidate();
}

//显示
void CListHeaderItemUI::SetIconOffset(RECT rc)   //设置Icon偏移量，右边的设置右下角偏移量
{
	m_rcIconOffset = rc;
}

RECT CListHeaderItemUI::GetIconOffset() const
{
	return m_rcIconOffset;
}

SIZE CListHeaderItemUI::GetIconSize() const      //按钮大小
{
	return m_szIcon;
}



void CListHeaderItemUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if( _tcscmp(pstrName, _T("dragable")) == 0 ) SetDragable(_tcscmp(pstrValue, _T("true")) == 0);
    else if( _tcscmp(pstrName, _T("sepwidth")) == 0 ) SetSepWidth(_ttoi(pstrValue));
	else if ( _tcscmp(pstrName, _T("buttonsytle")) == 0 )
	{
		SetButtonStyleIcon(_tcscmp(pstrValue, _T("true")) == 0);
	}
    else if( _tcscmp(pstrName, _T("align")) == 0 ) {
        if( _tcsstr(pstrValue, _T("left")) != NULL ) {
            m_uTextStyle &= ~(DT_CENTER | DT_RIGHT);
            m_uTextStyle |= DT_LEFT;
        }
        if( _tcsstr(pstrValue, _T("center")) != NULL ) {
            m_uTextStyle &= ~(DT_LEFT | DT_RIGHT);
            m_uTextStyle |= DT_CENTER;
        }
        if( _tcsstr(pstrValue, _T("right")) != NULL ) {
            m_uTextStyle &= ~(DT_LEFT | DT_CENTER);
            m_uTextStyle |= DT_RIGHT;
        }
    }
    else if( _tcscmp(pstrName, _T("endellipsis")) == 0 ) {
        if( _tcscmp(pstrValue, _T("true")) == 0 ) m_uTextStyle |= DT_END_ELLIPSIS;
        else m_uTextStyle &= ~DT_END_ELLIPSIS;
    }    
    else if( _tcscmp(pstrName, _T("font")) == 0 ) SetFont(_ttoi(pstrValue));
    else if( _tcscmp(pstrName, _T("textcolor")) == 0 ) {
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetTextColor(clrColor);
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
    else if( _tcscmp(pstrName, _T("normalimage")) == 0 ) SetNormalImage(pstrValue);
    else if( _tcscmp(pstrName, _T("hotimage")) == 0 ) SetHotImage(pstrValue);
    else if( _tcscmp(pstrName, _T("pushedimage")) == 0 ) SetPushedImage(pstrValue);
    else if( _tcscmp(pstrName, _T("focusedimage")) == 0 ) SetFocusedImage(pstrValue);
    else if( _tcscmp(pstrName, _T("sepimage")) == 0 ) SetSepImage(pstrValue);
	else if( _tcscmp(pstrName, _T("selectedicon")) == 0 ) SetSelectedIconImage(pstrValue);
	else if( _tcscmp(pstrName, _T("normalicon")) == 0 ) SetNormalIconImage(pstrValue);
	else if (_tcscmp(pstrName, _T("iconoffset")) == 0 )
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

	else CControlUI::SetAttribute(pstrName, pstrValue);
}

void CListHeaderItemUI::DoEvent(TEventUI& event)
{
    if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
        if( m_pParent != NULL ) m_pParent->DoEvent(event);
        else CControlUI::DoEvent(event);
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
	if (event.Type == UIEVENT_RBUTTONDOWN)
	{
		return;
	}
    if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK )
    {
        if( !IsEnabled() ) return;
        RECT rcSeparator = GetThumbRect();
		if (m_iSepWidth>=0)//111024 by cddjr, 增加分隔符区域，方便用户拖动
			rcSeparator.left-=4;
		else
			rcSeparator.right+=4;

        if( ::PtInRect(&rcSeparator, event.ptMouse) ) {
            if( m_bDragable ) {
                m_uButtonState |= UISTATE_CAPTURED;
                ptLastMouse = event.ptMouse;
            }
        }
        else {
			m_bSelected = !m_bSelected;
            m_uButtonState |= UISTATE_PUSHED;
            m_pManager->SendNotify(this, _T("headerclick"), DUILIB_LIST_HEADERCLICK);
            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_BUTTONUP )
    {
        if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
            m_uButtonState &= ~UISTATE_CAPTURED;
            if( GetParent() ) 
                GetParent()->NeedParentUpdate();
        }
        else if( (m_uButtonState & UISTATE_PUSHED) != 0 ) {
            m_uButtonState &= ~UISTATE_PUSHED;
            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_MOUSEMOVE )
    {
        if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
            RECT rc = m_rcItem;
            if( m_iSepWidth >= 0 ) {
                rc.right -= ptLastMouse.x - event.ptMouse.x;
            }
            else {
                rc.left -= ptLastMouse.x - event.ptMouse.x;
            }
            
            if( rc.right - rc.left > GetMinWidth() ) {
                m_cxyFixed.cx = rc.right - rc.left;
                ptLastMouse = event.ptMouse;
                if( GetParent() ) 
                    GetParent()->NeedParentUpdate();
            }
        }
        return;
    }
    if( event.Type == UIEVENT_SETCURSOR )
    {
	    RECT rcSeparator = GetThumbRect();
        if( IsEnabled() && m_bDragable && ::PtInRect(&rcSeparator, event.ptMouse) ) {
            ::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEWE)));
            return;
        }
		else if (m_bButtonStyleIcon)
		{
			::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
			return;

		}
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
    CControlUI::DoEvent(event);
}

SIZE CListHeaderItemUI::EstimateSize(SIZE szAvailable)
{
    if( m_cxyFixed.cy == 0 ) return CSize(m_cxyFixed.cx, m_pManager->GetDefaultFontInfo()->tm.tmHeight + 14);
    return CControlUI::EstimateSize(szAvailable);
}

RECT CListHeaderItemUI::GetThumbRect() const
{
    if( m_iSepWidth >= 0 ) return CRect(m_rcItem.right - m_iSepWidth, m_rcItem.top, m_rcItem.right, m_rcItem.bottom);
    else return CRect(m_rcItem.left, m_rcItem.top, m_rcItem.left - m_iSepWidth, m_rcItem.bottom);
}

RECT CListHeaderItemUI::GetIconRect() const
{
	if (m_szIcon.cx == 0 || m_szIcon.cy == 0)
	{
		return CRect(0,0,0,0);
	}
	
	CRect rcIcon;
	rcIcon.right = m_rcItem.right - m_rcIconOffset.right;
	rcIcon.left = m_rcItem.left + m_rcIconOffset.left;
	if (m_rcIconOffset.left == 0)
	{
		rcIcon.left = rcIcon.right - m_szIcon.cx;
	}
	if (m_rcIconOffset.right == 0)
	{
		rcIcon.right = rcIcon.left + m_szIcon.cx;
	}
	if (rcIcon.right < m_rcItem.left)  //修正大小
	{
		rcIcon.right = m_rcItem.left;
	}
	if (rcIcon.left < m_rcItem.left)
	{
		rcIcon.left = m_rcItem.left;
	}

	rcIcon.bottom = m_rcItem.bottom - m_rcIconOffset.bottom;
	rcIcon.top = m_rcItem.top + m_rcIconOffset.top;

	if (m_rcIconOffset.top == 0)
	{
		rcIcon.top = rcIcon.bottom - m_szIcon.cy;
	}
	if (m_rcIconOffset.bottom == 0)
	{
		rcIcon.bottom = rcIcon.top + m_szIcon.cy;
	}
	if (rcIcon.bottom < m_rcItem.top)
	{
		rcIcon.bottom = m_rcItem.top;
	}
	if (rcIcon.top < m_rcItem.top)
	{
		rcIcon.top = m_rcItem.top;
	}
	return rcIcon;
}


void CListHeaderItemUI::PaintStatusImage(HDC hDC)
{
    if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
    else m_uButtonState &= ~ UISTATE_FOCUSED;

	RECT rcThumb = m_rcItem;
	if( !m_sSepImage.IsEmpty() ) {
		rcThumb.right --;
	}

    if( (m_uButtonState & UISTATE_PUSHED) != 0 ) {
        if( m_sPushedImage.IsEmpty() && !m_sNormalImage.IsEmpty() ) DrawImage(hDC, (LPCTSTR)m_sNormalImage, rcThumb);
        if( !DrawImage(hDC, (LPCTSTR)m_sPushedImage, rcThumb)) m_sPushedImage.Empty();
    }
    else if( (m_uButtonState & UISTATE_HOT) != 0 ) {
        if( m_sHotImage.IsEmpty() && !m_sNormalImage.IsEmpty() ) DrawImage(hDC, (LPCTSTR)m_sNormalImage, rcThumb);
        if( !DrawImage(hDC, (LPCTSTR)m_sHotImage, rcThumb)) m_sHotImage.Empty();
    }
    else if( (m_uButtonState & UISTATE_FOCUSED) != 0 ) {
        if( m_sFocusedImage.IsEmpty() && !m_sNormalImage.IsEmpty() ) DrawImage(hDC, (LPCTSTR)m_sNormalImage, rcThumb);
        if( !DrawImage(hDC, (LPCTSTR)m_sFocusedImage, rcThumb)) m_sFocusedImage.Empty();
    }
    else {
        if( !m_sNormalImage.IsEmpty() ) {
            if( !DrawImage(hDC, (LPCTSTR)m_sNormalImage, rcThumb)) m_sNormalImage.Empty();
        }
    }

    if( !m_sSepImage.IsEmpty() ) {
        RECT rcThumb = GetThumbRect();
        rcThumb.left -= m_rcItem.left;
        rcThumb.top -= m_rcItem.top;
        rcThumb.right -= m_rcItem.left;
        rcThumb.bottom -= m_rcItem.top;

        m_sSepImageModify.Empty();
        m_sSepImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), rcThumb.left, rcThumb.top, rcThumb.right, rcThumb.bottom);
        if( !DrawImage(hDC, (LPCTSTR)m_sSepImage, (LPCTSTR)m_sSepImageModify) ) m_sSepImage.Empty();
    }

	if (m_bShowIcon)
	{
		if (m_bSelected)
		{
			if( !m_sSelectedIcon.IsEmpty() ) {
				RECT rcIcon = GetIconRect();
				rcIcon.left -= m_rcItem.left;
				rcIcon.top -= m_rcItem.top;
				rcIcon.right -= m_rcItem.left;
				rcIcon.bottom -= m_rcItem.top;
				m_strIconModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), rcIcon.left, rcIcon.top, rcIcon.right, rcIcon.bottom);
				if( !DrawImage(hDC, (LPCTSTR)m_sSelectedIcon, (LPCTSTR)m_strIconModify) ) m_sSelectedIcon.Empty();
			}
		}
		else
		{
			if( !m_sNormalIcon.IsEmpty() ) {
				RECT rcIcon = GetIconRect();
				rcIcon.left -= m_rcItem.left;
				rcIcon.top -= m_rcItem.top;
				rcIcon.right -= m_rcItem.left;
				rcIcon.bottom -= m_rcItem.top;

				m_strIconModify.Empty();
				m_strIconModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), rcIcon.left, rcIcon.top, rcIcon.right, rcIcon.bottom);
				if( !DrawImage(hDC, (LPCTSTR)m_sNormalIcon, (LPCTSTR)m_strIconModify) ) m_sNormalIcon.Empty();
			}
		}
	}

}

void CListHeaderItemUI::PaintText(HDC hDC)
{
    if( m_dwTextColor == 0 ) m_dwTextColor = m_pManager->GetDefaultFontColor();

	RECT rcText = m_rcItem;
	rcText.left += m_rcTextPadding.left;
	rcText.top += m_rcTextPadding.top;
	rcText.right -= m_rcTextPadding.right;
	rcText.bottom -= m_rcTextPadding.bottom;

    if( m_sText.IsEmpty() ) return;
    int nLinks = 0;
    if( m_bShowHtml )
        CRenderEngine::DrawHtmlText(hDC, m_pManager, rcText, m_sText, m_dwTextColor, \
        NULL, NULL, nLinks, DT_SINGLELINE | m_uTextStyle);
    else
        CRenderEngine::DrawText(hDC, m_pManager, rcText, m_sText, m_dwTextColor, \
        m_iFont, DT_SINGLELINE | m_uTextStyle);
}

/////////////////////////////////////////////////////////////////////////////////////
//
//

CListElementUI::CListElementUI() : 
m_iIndex(-1),
m_pOwner(NULL), 
m_bSelected(false),
m_uButtonState(0)
{
}

LPCTSTR CListElementUI::GetClass() const
{
    return _T("ListElementUI");
}

UINT CListElementUI::GetControlFlags() const
{
    return UIFLAG_WANTRETURN;
}

LPVOID CListElementUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcscmp(pstrName, _T("ListItem")) == 0 ) return static_cast<IListItemUI*>(this);
	if( _tcscmp(pstrName, _T("ListElement")) == 0 ) return static_cast<CListElementUI*>(this);
    return CControlUI::GetInterface(pstrName);
}

IListOwnerUI* CListElementUI::GetOwner()
{
    return m_pOwner;
}

void CListElementUI::SetOwner(CControlUI* pOwner)
{
    m_pOwner = static_cast<IListOwnerUI*>(pOwner->GetInterface(_T("IListOwner")));
}

void CListElementUI::SetVisible(bool bVisible)
{
    CControlUI::SetVisible(bVisible);
    if( !IsVisible() && m_bSelected)
    {
        m_bSelected = false;
        if( m_pOwner != NULL ) m_pOwner->SelectItem(-1);
    }
}

void CListElementUI::SetEnabled(bool bEnable)
{
    CControlUI::SetEnabled(bEnable);
    if( !IsEnabled() ) {
        m_uButtonState = 0;
    }
}

int CListElementUI::GetIndex() const
{
    return m_iIndex;
}

void CListElementUI::SetIndex(int iIndex)
{
    m_iIndex = iIndex;
}

void CListElementUI::Invalidate()
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

bool CListElementUI::Activate()
{
    if( !CControlUI::Activate() ) return false;
    if( m_pManager != NULL ) m_pManager->SendNotify(this, _T("itemactivate"), DUILIB_LIST_ITEMACTIVE);
    return true;
}

bool CListElementUI::IsSelected() const
{
    return m_bSelected;
}

bool CListElementUI::Select(bool bSelect /* = true */, bool bInvalidate /* = true */)
{
    if( !IsEnabled() ) return false;
    if( bSelect == m_bSelected ) return true;
    m_bSelected = bSelect;
    if( bSelect && m_pOwner != NULL ) m_pOwner->SelectItem(m_iIndex);
	if (bInvalidate)
	{
		Invalidate();
	}

    return true;
}

bool CListElementUI::IsExpanded() const
{
    return false;
}

bool CListElementUI::Expand(bool /*bExpand = true*/)
{
    return false;
}

void CListElementUI::DoEvent(TEventUI& event)
{
    if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
        if( m_pOwner != NULL ) m_pOwner->DoEvent(event);
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
    // An important twist: The list-item will send the event not to its immediate
    // parent but to the "attached" list. A list may actually embed several components
    // in its path to the item, but key-presses etc. needs to go to the actual list.
    if( m_pOwner != NULL ) m_pOwner->DoEvent(event); else CControlUI::DoEvent(event);
}

void CListElementUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if( _tcscmp(pstrName, _T("selected")) == 0 ) Select();
    else CControlUI::SetAttribute(pstrName, pstrValue);
}

void CListElementUI::DrawItemBk(HDC hDC, const RECT& rcItem)
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
        if( m_iIndex % 2 == 0 ) {
            if( !DrawImage(hDC, (LPCTSTR)m_sBkImage) ) m_sBkImage.Empty();
        }
    }

    if( m_sBkImage.IsEmpty() ) {
        if( !pInfo->sBkImage.IsEmpty() ) {
            if( !DrawImage(hDC, (LPCTSTR)pInfo->sBkImage) ) pInfo->sBkImage.Empty();
            else return;
        }
    }

    if ( pInfo->dwLineColor != 0 ) {
        RECT rcLine = { m_rcItem.left, m_rcItem.bottom - 1, m_rcItem.right, m_rcItem.bottom - 1 };
        CRenderEngine::DrawLine(hDC, rcLine, 1, GetAdjustColor(pInfo->dwLineColor));
    }
}

/////////////////////////////////////////////////////////////////////////////////////
//
//

CListLabelElementUI::CListLabelElementUI()
: m_uTextStyle(DT_VCENTER)
, m_nEllipsisLength(-1)
{
}

LPCTSTR CListLabelElementUI::GetClass() const
{
    return _T("ListLabelElementUI");
}

LPVOID CListLabelElementUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcscmp(pstrName, _T("ListLabelElement")) == 0 ) return static_cast<CListLabelElementUI*>(this);
    return CListElementUI::GetInterface(pstrName);
}

void CListLabelElementUI::DoEvent(TEventUI& event)
{
    if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
        if( m_pOwner != NULL ) m_pOwner->DoEvent(event);
        else CListElementUI::DoEvent(event);
        return;
    }

    if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_RBUTTONDOWN )
    {
        if( IsEnabled() ) {
            m_pManager->SendNotify(this, _T("itemclick"), DUILIB_LIST_ITEMCLICK);
            Select();
            Invalidate();
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
        if( (m_uButtonState & UISTATE_HOT) != 0 ) {
            m_uButtonState &= ~UISTATE_HOT;
            Invalidate();
        }
        return;
    }
    CListElementUI::DoEvent(event);
}

SIZE CListLabelElementUI::EstimateSize(SIZE szAvailable)
{
    if( m_pOwner == NULL ) return CSize(0, 0);

    TListInfoUI* pInfo = m_pOwner->GetListInfo();
    SIZE cXY = m_cxyFixed;
    if( cXY.cy == 0 && m_pManager != NULL ) {
        cXY.cy = m_pManager->GetDefaultFontInfo()->tm.tmHeight + 8;
        cXY.cy += pInfo->rcTextPadding.top + pInfo->rcTextPadding.bottom;
    }

    if( cXY.cx == 0 && m_pManager != NULL ) {
        RECT rcText = { 0, 0, 9999, cXY.cy };
        if( pInfo->bShowHtml ) {
            int nLinks = 0;
            CRenderEngine::DrawHtmlText(m_pManager->GetPaintDC(), m_pManager, rcText, m_sText, 0, NULL, NULL, nLinks, DT_SINGLELINE | DT_CALCRECT | pInfo->uTextStyle & ~DT_RIGHT & ~DT_CENTER);
        }
        else {
//            CRenderEngine::DrawText(m_pManager->GetPaintDC(), m_pManager, rcText, m_sText, 0, pInfo->nFont, DT_SINGLELINE | DT_CALCRECT | pInfo->uTextStyle & ~DT_RIGHT & ~DT_CENTER);
			HWND hDestopWnd = ::GetDesktopWindow();
			if (hDestopWnd != NULL)
			{
				HDC hdesktopDc = ::GetDC(hDestopWnd);   // 使用桌面窗口上下文句柄，计算文本大小
				CRenderEngine::DrawText(hdesktopDc, m_pManager, rcText, m_sText, 0, pInfo->nFont, DT_CALCRECT |  pInfo->uTextStyle & ~DT_RIGHT & ~DT_CENTER);
				::ReleaseDC(hDestopWnd, hdesktopDc);
			}

        }
        cXY.cx = rcText.right - rcText.left + pInfo->rcTextPadding.left + pInfo->rcTextPadding.right;        
    }

    return cXY;
}

void CListLabelElementUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
    if( !::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem) ) return;
    DrawItemBk(hDC, m_rcItem);
    DrawItemText(hDC, m_rcItem);
}

void CListLabelElementUI::DrawItemText(HDC hDC, const RECT& rcItem)
{
    if( m_sText.IsEmpty() ) return;

    if( m_pOwner == NULL ) return;
    TListInfoUI* pInfo = m_pOwner->GetListInfo();
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
	
	if (m_nEllipsisLength > 0 && m_nEllipsisLength <= MAX_PATH)
	{
// 		TEXTMETRIC tm; 
// 		GetTextMetrics(hDC, &tm);	   //获取当个字体大小		
		TreeNode *node = (TreeNode*)GetTag();
		if (node != NULL)
		{
			CFileListUI *pControl = static_cast<CFileListUI *>(m_pOwner);
			CStdString sName = node->data()._text,sTest = _T("中文字符长度");	
			HFONT hOldFont = (HFONT)::SelectObject(hDC, m_pManager->GetFont(pInfo->nFont));
			SIZE szSpace = { 0 },szSpace1 = { 0 };
			::GetTextExtentPoint32(hDC, (LPCTSTR)sName, sName.GetLength(), &szSpace);
			::GetTextExtentPoint32(hDC, (LPCTSTR)sTest, sTest.GetLength(), &szSpace1);
			::SelectObject(hDC, hOldFont);
			int nFontWidth = (int)szSpace1.cx / sTest.GetLength();

			SIZE szExpand = {0};						
			if (szSpace.cx > (m_nEllipsisLength * nFontWidth) && pControl)
			{
				szExpand = pControl->GetSelctedImageSize();					
				rcText.right =  node->data()._LeftPos + (m_nEllipsisLength * nFontWidth) + pInfo->rcTextPadding.right + szExpand.cx /*+ 10*/;
			}
		}		
	}

    if( pInfo->bShowHtml )
        CRenderEngine::DrawHtmlText(hDC, m_pManager, rcText, m_sText, iTextColor, \
        NULL, NULL, nLinks, DT_SINGLELINE | pInfo->uTextStyle);
    else
        CRenderEngine::DrawText(hDC, m_pManager, rcText, m_sText, iTextColor, \
        pInfo->nFont, DT_SINGLELINE | pInfo->uTextStyle);
}

void CListLabelElementUI::SetEndEllipsisLength(int nNumbers)
{
	m_nEllipsisLength = (nNumbers < 0 || nNumbers > MAX_PATH) ? -1 : nNumbers;
}

/////////////////////////////////////////////////////////////////////////////////////
//
//

CListTextElementUI::CListTextElementUI() : m_nLinks(0), m_nHoverLink(-1), m_pOwner(NULL)
{
    ::ZeroMemory(&m_rcLinks, sizeof(m_rcLinks));
}

CListTextElementUI::~CListTextElementUI()
{
    CStdString* pText;
    for( int it = 0; it < m_aTexts.GetSize(); it++ ) {
        pText = static_cast<CStdString*>(m_aTexts[it]);
        if( pText ) delete pText;
    }
    m_aTexts.Empty();
}

LPCTSTR CListTextElementUI::GetClass() const
{
    return _T("ListTextElementUI");
}

LPVOID CListTextElementUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcscmp(pstrName, _T("ListTextElement")) == 0 ) return static_cast<CListTextElementUI*>(this);
    return CListLabelElementUI::GetInterface(pstrName);
}

UINT CListTextElementUI::GetControlFlags() const
{
    return UIFLAG_WANTRETURN | ( (IsEnabled() && m_nLinks > 0) ? UIFLAG_SETCURSOR : 0);
}

LPCTSTR CListTextElementUI::GetText(int iIndex) const
{
    CStdString* pText = static_cast<CStdString*>(m_aTexts.GetAt(iIndex));
    if( pText ) return pText->GetData();
    return NULL;
}

void CListTextElementUI::SetText(int iIndex, LPCTSTR pstrText)
{
    if( m_pOwner == NULL ) return;
    TListInfoUI* pInfo = m_pOwner->GetListInfo();
    if( iIndex < 0 || iIndex >= pInfo->nColumns ) return;
    while( m_aTexts.GetSize() < pInfo->nColumns ) { m_aTexts.Add(NULL); }

    CStdString* pText = static_cast<CStdString*>(m_aTexts[iIndex]);
    if( (pText == NULL && pstrText == NULL) || (pText && *pText == pstrText) ) return;

    if( pText ) delete pText;
    m_aTexts.SetAt(iIndex, new CStdString(pstrText));
    Invalidate();
}

void CListTextElementUI::SetOwner(CControlUI* pOwner)
{
    CListElementUI::SetOwner(pOwner);
    m_pOwner = static_cast<IListUI*>(pOwner->GetInterface(_T("IList")));
}

CStdString* CListTextElementUI::GetLinkContent(int iIndex)
{
    if( iIndex >= 0 && iIndex < m_nLinks ) return &m_sLinks[iIndex];
    return NULL;
}

void CListTextElementUI::DoEvent(TEventUI& event)
{
    if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
        if( m_pOwner != NULL ) m_pOwner->DoEvent(event);
        else CListLabelElementUI::DoEvent(event);
        return;
    }

    // When you hover over a link
    if( event.Type == UIEVENT_SETCURSOR ) {
        for( int i = 0; i < m_nLinks; i++ ) {
            if( ::PtInRect(&m_rcLinks[i], event.ptMouse) ) {
                ::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
                return;
            }
        }      
    }
    if( event.Type == UIEVENT_BUTTONUP && IsEnabled() ) {
        for( int i = 0; i < m_nLinks; i++ ) {
            if( ::PtInRect(&m_rcLinks[i], event.ptMouse) ) {
                m_pManager->SendNotify(this, _T("link"), DUILIB_LIST_LINK, i);
                return;
            }
        }
    }
    if( m_nLinks > 0 && event.Type == UIEVENT_MOUSEMOVE ) {
        int nHoverLink = -1;
        for( int i = 0; i < m_nLinks; i++ ) {
            if( ::PtInRect(&m_rcLinks[i], event.ptMouse) ) {
                nHoverLink = i;
                break;
            }
        }

        if(m_nHoverLink != nHoverLink) {
            Invalidate();
            m_nHoverLink = nHoverLink;
        }
    }
    if( m_nLinks > 0 && event.Type == UIEVENT_MOUSELEAVE ) {
        if(m_nHoverLink != -1) {
            Invalidate();
            m_nHoverLink = -1;
        }
    }
    CListLabelElementUI::DoEvent(event);
}

SIZE CListTextElementUI::EstimateSize(SIZE szAvailable)
{
    TListInfoUI* pInfo = NULL;
    if( m_pOwner ) pInfo = m_pOwner->GetListInfo();

    SIZE cXY = m_cxyFixed;
    if( cXY.cy == 0 && m_pManager != NULL ) {
        cXY.cy = m_pManager->GetDefaultFontInfo()->tm.tmHeight + 8;
        if( pInfo ) cXY.cy += pInfo->rcTextPadding.top + pInfo->rcTextPadding.bottom;
    }

    return cXY;
}

void CListTextElementUI::DrawItemText(HDC hDC, const RECT& rcItem)
{
    if( m_pOwner == NULL ) return;
    TListInfoUI* pInfo = m_pOwner->GetListInfo();
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
    IListCallbackUI* pCallback = m_pOwner->GetTextCallback();
    //ASSERT(pCallback);
    //if( pCallback == NULL ) return;

    m_nLinks = 0;
    int nLinks = lengthof(m_rcLinks);
    for( int i = 0; i < pInfo->nColumns; i++ )
    {
        RECT rcItem = { pInfo->rcColumn[i].left, m_rcItem.top, pInfo->rcColumn[i].right, m_rcItem.bottom };
        rcItem.left += pInfo->rcTextPadding.left;
        rcItem.right -= pInfo->rcTextPadding.right;
        rcItem.top += pInfo->rcTextPadding.top;
        rcItem.bottom -= pInfo->rcTextPadding.bottom;

        LPCTSTR pstrText = NULL;
        if( pCallback ) pstrText = pCallback->GetItemText(this, m_iIndex, i);
        else pstrText = GetText(i);
        if( pInfo->bShowHtml )
            CRenderEngine::DrawHtmlText(hDC, m_pManager, rcItem, pstrText, iTextColor, \
                &m_rcLinks[m_nLinks], &m_sLinks[m_nLinks], nLinks, DT_SINGLELINE | pInfo->uTextStyle);
        else
            CRenderEngine::DrawText(hDC, m_pManager, rcItem, pstrText, iTextColor, \
            pInfo->nFont, DT_SINGLELINE | pInfo->uTextStyle);

        m_nLinks += nLinks;
        nLinks = lengthof(m_rcLinks) - m_nLinks; 
    }
    for( int i = m_nLinks; i < lengthof(m_rcLinks); i++ ) {
        ::ZeroMemory(m_rcLinks + i, sizeof(RECT));
        ((CStdString*)(m_sLinks + i))->Empty();
    }
}

/////////////////////////////////////////////////////////////////////////////////////
//
//

CListContainerElementUI::CListContainerElementUI() : 
m_iIndex(-1),
m_pOwner(NULL), 
m_uButtonState(0)
{
	m_bSelected = false;
}

LPCTSTR CListContainerElementUI::GetClass() const
{
	if (m_pOwner)
	{
		return m_pOwner->GetClass();
	}
    return _T("ListContainerElementUI");
}

UINT CListContainerElementUI::GetControlFlags() const
{
    return UIFLAG_WANTRETURN;
}

LPVOID CListContainerElementUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcscmp(pstrName, _T("ListItem")) == 0 ) return static_cast<IListItemUI*>(this);
	if( _tcscmp(pstrName, _T("ListContainerElement")) == 0 ) return static_cast<CListContainerElementUI*>(this);
    return CContainerUI::GetInterface(pstrName);
}

CListUI* CListContainerElementUI::GetOwner()
{
    return m_pOwner;
}

void CListContainerElementUI::SetOwner(CControlUI* pOwner)
{
    m_pOwner = static_cast<CListUI*>(pOwner->GetInterface(_T("List")));
}

void CListContainerElementUI::SetVisible(bool bVisible)
{
    CContainerUI::SetVisible(bVisible);
    if( !IsVisible() && m_bSelected)
    {
        m_bSelected = false;
        if( m_pOwner != NULL ) m_pOwner->SelectItem(-1);
    }
}

void CListContainerElementUI::SetEnabled(bool bEnable)
{
    CControlUI::SetEnabled(bEnable);
    if( !IsEnabled() ) {
        m_uButtonState = 0;
    }
}

int CListContainerElementUI::GetIndex() const
{
    return m_iIndex;
}

void CListContainerElementUI::SetIndex(int iIndex)
{
    m_iIndex = iIndex;
}

void CListContainerElementUI::Invalidate()
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
            CContainerUI::Invalidate();
        }
    }
    else {
        CContainerUI::Invalidate();
    }
}

bool CListContainerElementUI::Activate()
{
    if( !CContainerUI::Activate() ) return false;
    if( m_pManager != NULL ) m_pManager->SendNotify(this, _T("itemactivate"), DUILIB_LIST_ITEMACTIVE);
    return true;
}

bool CListContainerElementUI::IsSelected() const
{
    return m_bSelected;
}

bool CListContainerElementUI::Select(bool bSelect /* = true */, bool bInvalidate /* = true */)
{
    if( !IsEnabled() ) return false;
    if( bSelect == m_bSelected )
		return true;
    m_bSelected = bSelect;
//    if( bSelect && m_pOwner != NULL ) m_pOwner->SelectItem(m_iIndex);
	if (bInvalidate == true)
	{
		Invalidate();
	}

    return true;
}

bool CListContainerElementUI::IsExpanded() const
{
    return false;
}

bool CListContainerElementUI::Expand(bool /*bExpand = true*/)
{
    return false;
}

void CListContainerElementUI::DoEvent(TEventUI& event)
{
    if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
        if( m_pOwner != NULL ) m_pOwner->DoEvent(event);
        else CContainerUI::DoEvent(event);
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
	if (event.Type == UIEVENT_RBUTTONDOWN)
	{
		m_pManager->SendNotify(this, _T("itemclick"), DUILIB_LIST_ITEMCLICK);
		if (m_pOwner != NULL)
		{
			if (!IsSelected())
				m_pOwner->SelectItem(m_iIndex);
		}
		Invalidate();
		return;
	}
    if( event.Type == UIEVENT_BUTTONDOWN /*|| event.Type == UIEVENT_RBUTTONDOWN */)
    {
		if( ::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled() ) 
		{
			/*m_uButtonState |= UISTATE_CAPTURED;*/
            m_pManager->SendNotify(this, _T("itemclick"), DUILIB_LIST_ITEMCLICK);
			if (m_pOwner)
			{
				if (m_pOwner->GetSingleSelect())
				{
					if (!IsSelected())
						m_pOwner->SelectItem(m_iIndex);
				}				
				else
				{
					m_pOwner->SetLButtonState(true);
					if ((GetKeyState(VK_CONTROL)< 0))
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
						m_pOwner->SetFocusItem(m_iIndex);
					}
					else
					{
						if ((m_pOwner->GetSelectItemCount() == 1 && IsSelected()))
						{								
						}						
						else
						{
							m_pOwner->SelectItem(m_iIndex);	
						}
					}					
				}
				m_pOwner->SetMCaptured(m_pOwner);
			}
			Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_BUTTONUP ) 
    {
		if (IsEnabled())
		{
			if (m_pOwner)
			{
// 				TCHAR wstr[256] = {0};
// 				::wsprintf(wstr, _T("The status is set to false, the Index is %d\n"), m_iIndex);
// 				OutputDebugStr(wstr);
 				m_pOwner->SetLButtonState(false);
			}
		}
//		m_pOwner->DoEvent(event);
        return;
    }
    if( event.Type == UIEVENT_MOUSEMOVE )
    {
		if (IsEnabled())
		{
			if (m_pOwner && (m_pOwner->GetLButtonState()))
			{
				if (m_pOwner->GetEndItem() != m_iIndex)
				{
// 					TCHAR wstr[256] = {0};
// 					::wsprintf(wstr, _T("The status is  true, the Index is %d\n"), m_iIndex);
// 					OutputDebugStr(wstr);
					m_pOwner->SetEndItem(m_iIndex);
				}
			}
		}
        return;
    }
    if( event.Type == UIEVENT_MOUSEENTER )
    {
		if (!IsLButtonDown() || (!IsFocused() && !m_pOwner->IsFocused()))
		{
			m_pOwner->SetLButtonState(false);
		}
        if( IsEnabled() ) {
            m_uButtonState |= UISTATE_HOT;
			m_pManager->SendNotify(this, _T("item_mouseenter"), DUILIB_LIST_ITEM_MOUSEENTER);
            Invalidate();
        }
		
        return;
    }
    if( event.Type == UIEVENT_MOUSELEAVE )
    {
        if( (m_uButtonState & UISTATE_HOT) != 0 )
		{
            m_uButtonState &= ~UISTATE_HOT;
			m_pManager->SendNotify(this, _T("item_mouseleave"), DUILIB_LIST_ITEM_MOUSELEAVE);
            Invalidate();
        }
        return;
    }

    // An important twist: The list-item will send the event not to its immediate
    // parent but to the "attached" list. A list may actually embed several components
    // in its path to the item, but key-presses etc. needs to go to the actual list.
    if( m_pOwner != NULL ) m_pOwner->DoEvent(event); else CControlUI::DoEvent(event);
}

void CListContainerElementUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if( _tcscmp(pstrName, _T("selected")) == 0 ) Select();
    else CContainerUI::SetAttribute(pstrName, pstrValue);
}

void CListContainerElementUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
    if( !::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem) ) return;
    DrawItemBk(hDC, m_rcItem);

	TListInfoUI* pInfo = m_pOwner->GetListInfo();

    CContainerUI::DoPaint(hDC, rcPaint);
}

//add by lighten
SIZE CListContainerElementUI::EstimateSize(SIZE szAvailable)
{
	SIZE cXY = {0, m_cxyFixed.cy};
	if( cXY.cy == 0 && m_pManager != NULL ) {
		for( int it = 0; it < m_items.GetSize(); it++ ) {
			cXY.cy = MAX(cXY.cy,static_cast<CControlUI*>(m_items[it])->EstimateSize(szAvailable).cy);
		}
		int nMin = m_pManager->GetDefaultFontInfo()->tm.tmHeight + 8;
		cXY.cy = MAX(cXY.cy,nMin);
	}

	for( int it = 0; it < m_items.GetSize(); it++ ) {
		cXY.cx +=  static_cast<CControlUI*>(m_items[it])->EstimateSize(szAvailable).cx;
	}
	return cXY;
}

void CListContainerElementUI::SetPos(RECT rc)
{
	CControlUI::SetPos(rc);
	if( m_items.IsEmpty() ) return;
	rc.left += m_rcInset.left;
	rc.top += m_rcInset.top;
	rc.right -= m_rcInset.right;
	rc.bottom -= m_rcInset.bottom;

	for( int it = 0; it < m_items.GetSize(); it++ ) {
		CControlUI* pControl = static_cast<CControlUI*>(m_items[it]);
		if( !pControl->IsVisible() ) continue;
		if( pControl->IsFloat() ) {
			SetFloatPos(it);
		}
		else {
			 TListInfoUI* pInfo = m_pOwner->GetListInfo();
			 if (pInfo->nColumns > 0)
			 {
				 RECT rcItem = { pInfo->rcColumn[it].left, rc.top, pInfo->rcColumn[it].right, rc.bottom };
				 pControl->SetPos(rcItem); 
			 }
			 else
			 {
				 pControl->SetPos(rc);// 所有非float子控件放大到整个客户区
			 }
		}
	}
}

void CListContainerElementUI::DrawItemText(HDC hDC, const RECT& rcItem)
{
    return;
}

void CListContainerElementUI::DrawItemBk(HDC hDC, const RECT& rcItem)
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
        if( m_iIndex % 2 == 0 ) {
            if( !DrawImage(hDC, (LPCTSTR)m_sBkImage) ) m_sBkImage.Empty();
        }
    }

    if( m_sBkImage.IsEmpty() ) {
        if( !pInfo->sBkImage.IsEmpty() ) {
            if( !DrawImage(hDC, (LPCTSTR)pInfo->sBkImage) ) pInfo->sBkImage.Empty();
            else return;
        }
    }

    if ( pInfo->dwLineColor != 0 ) {
        RECT rcLine = { m_rcItem.left, m_rcItem.bottom - 1, m_rcItem.right, m_rcItem.bottom - 1 };
        CRenderEngine::DrawLine(hDC, rcLine, 1, GetAdjustColor(pInfo->dwLineColor));
    }
}

} // namespace DuiLib
