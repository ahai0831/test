#include "stdafx.h"
#include "IConListUIEx.h"
#include <windows.h>
#include <tlhelp32.h>
namespace DuiLib {
//////////////////////////////////////////////////////////////////////////
//
	void CMoveVerticalLayoutUI::DoPaint(HDC hDC, const RECT& rcPaint)
	{
		CVerticalLayoutUI::DoPaint(hDC, rcPaint);
		CRect rc;
		rc.left = m_point.x - 10;
		rc.right = rc.left + 20;
		rc.top = m_point.y - 10;
		rc.bottom = rc.top + 20;
		if (m_pOwner != NULL)
		{
			CIConListUIEx *pList = static_cast<CIConListUIEx*>(m_pOwner->GetInterface(_T("IconListEx")));
			if (pList != NULL)
			{
				pList->PaintSelectItem(hDC, rcPaint);
				CRenderEngine::DrawColor(hDC, m_pManager, rc, 0xFFFFFFFF);
				return;
			}
			CListUIEx *ptemp = static_cast<CListUIEx*>(m_pOwner->GetInterface(_T("ListEx")));
			if (ptemp != NULL)
			{
				ptemp->PaintSelectItem(hDC, rcPaint);
				POINT pt;
				CRenderEngine::DrawColor(hDC, m_pManager, rc, 0xFFFFFFFF);
				return;
			}
		}
	}

	void CMoveVerticalLayoutUI::SetOwner(CControlUI *pOwner)
	{
		m_pOwner = pOwner;
	}

/////////////////////////////////////////////////////////////////////////////////////
//
//
	void CMoveItemWnd::Init(CControlUI* pOwner)
	{
		m_pOwner = pOwner;
		m_pLayout = NULL;
		m_point = m_pOwner->GetManager()->GetMousePos();
		// Position the popup window in absolute space
		RECT rcOwner;
		GetClientRect(pOwner->GetManager()->GetPaintWindow(), &rcOwner);
		RECT rc = rcOwner;

		::MapWindowRect(pOwner->GetManager()->GetPaintWindow(), HWND_DESKTOP, &rc);

// 		Create(pOwner->GetManager()->GetPaintWindow(), NULL, WS_POPUP, WS_EX_TOOLWINDOW, rc);
		Create(NULL, NULL, WS_POPUP, WS_EX_TOOLWINDOW, rc);

		// add by lighten , visible the sel item
		POINT pt={rc.left, rc.top};
		::ScreenToClient(m_pm.GetPaintWindow(), &pt);
		RECT rcClient;
		rcClient.left = pt.x;
		rcClient.top = pt.y;
		rcClient.right = rcClient.left + (rc.right - rc.left);
		rcClient.bottom = rcClient.top + (rc.bottom - rc.top);
		m_pLayout->SetPos(rcClient);

		HWND hWndParent = m_hWnd;
		while( ::GetParent(hWndParent) != NULL ) hWndParent = ::GetParent(hWndParent);
		SetWindowPos(m_hWnd, HWND_TOPMOST, rc.left, rc.top, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, SWP_SHOWWINDOW | SWP_NOSIZE);

//		::ShowWindow(m_hWnd, SW_SHOW);
//		::SendMessage(hWndParent, WM_NCACTIVATE, TRUE, 0L);
	}

	LPCTSTR CMoveItemWnd::GetWindowClassName() const
	{
		return _T("MoveItemWnd");
	}

	void CMoveItemWnd::MoveWindowFollowPoint(POINT pt)
	{
		RECT rc;
		GetWindowRect(m_hWnd, &rc);
		int nWidth = rc.right  - rc.left;
		int nHeight = rc.bottom - rc.top;
		rc.top = rc.top + pt.y - m_point.y;
		rc.left = rc.left + pt.x - m_point.x;
		if (!MoveWindow(m_hWnd, rc.left, rc.top, nWidth, nHeight, true))
		{
			DWORD dwError = GetLastError();
			int i = 0;
		}
		
		m_point = pt;
		m_pLayout->m_point = m_point;
	}

	void CMoveItemWnd::OnFinalMessage(HWND hWnd)
	{
		m_pm.ReapObjects(m_pm.GetRoot());
// 		m_pOwner->Invalidate();
		if (m_pOwner != NULL)
		{
			CIConListUIEx *pList = static_cast<CIConListUIEx*>(m_pOwner->GetInterface(_T("IconListEx")));
			if (pList != NULL)
			{
				pList->m_bItemLButtonDown = false;
				pList->m_pMoveItem = NULL;
			}
			CListUIEx *ptemp = static_cast<CListUIEx*>(m_pOwner->GetInterface(_T("ListEx")));
			if (ptemp != NULL)
			{
				ptemp->m_bLButtonDown_ClickInData = false;
				ptemp->m_pMoveItem = NULL;
			}
		}
		delete this;
	}

	LRESULT CMoveItemWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if( uMsg == WM_CREATE ) {
			m_pm.Init(m_hWnd);
			// The trick is to add the items to the new container. Their owner gets
			// reassigned by this operation - which is why it is important to reassign
			// the items back to the righfull owner/manager when the window closes.
			m_pLayout = new CMoveVerticalLayoutUI;
			if (m_pLayout == NULL)
			{
				return 0;
			}
			m_pLayout->SetOwner(m_pOwner);
			m_pLayout->m_point = m_point;
			m_pm.UseParentResource(m_pOwner->GetManager());
			m_pLayout->SetManager(&m_pm, NULL, true);
			m_pLayout->SetBkColor(0xFFFFFFFF);
			m_pm.AttachDialog(m_pLayout);
			m_pm.SetTransparentAndColor(125, 0xFFFFFFFF);

			return 0;
		}
		else if( uMsg == WM_CLOSE || uMsg == WM_DESTROY) {

			m_pOwner->SetManager(m_pOwner->GetManager(), m_pOwner->GetParent(), false);
			m_pOwner->SetPos(m_pOwner->GetPos());
			m_pOwner->SetFocus();
		}
		else if( uMsg == WM_KEYDOWN ) {
			switch( wParam ) {
		case VK_ESCAPE:
			{
				PostMessage(WM_CLOSE);
			}
			break;
		case VK_RETURN:
			{
			PostMessage(WM_CLOSE);
// 				if (m_pOwner != NULL)
// 				{
// 					CIConListUIEx *pList = static_cast<CIConListUIEx*>(m_pOwner);
// 					if (pList != NULL)
// 					{
// 						pList->CloseMoveWindow();
// 						break;
// 					}
// 					CListUIEx *ptemp = static_cast<CListUIEx*>(m_pOwner);
// 					if (ptemp != NULL)
// 					{
// 						ptemp->CloseMoveWindow();
// 						break;
// 					}
// 				}
			}
			break;
		default:
			TEventUI event;
			event.Type = UIEVENT_KEYDOWN;
			event.chKey = (TCHAR)wParam;
			m_pOwner->DoEvent(event);
			return 0;
			}
		}
// 		else if( uMsg == WM_KILLFOCUS ) {
// 			if( m_hWnd != (HWND) wParam ) PostMessage(WM_CLOSE);
// 		}

		LRESULT lRes = 0;
		if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;
		return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	}



//////////////////////////////////////////////////////////////////////////
//
CIConListUIEx::CIConListUIEx() : m_pCallback(NULL), m_bScrollSelect(false), /*m_iCurSel(-1),*/ m_iExpandedItem(-1)
{
	m_pList = new CIConListBodyUIEx(this);
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

	//add by ligthen
	m_nTopIndex = 0;
	m_nItemHeight = 20;
	m_nItemWidth = 20;
	m_nItems = 0;
	m_nShowItemsInColumn = 0;
	m_nShowItemsInLine = 0;
	m_nShowNeedLines = 0;
	m_nShowNeedHeights = 0;
	m_dequeueReusableItem.Empty();
	m_pListDataSource = NULL;
	Init();

	m_bSingleSel = false;
	m_nFocusItem = -1;
	m_nStartSelItem = 0;
	m_nEndSelItem = -1;
	m_bLButtonDown = false;
	m_bItemLButtonDown = false;
	m_bHasMoveItem = false;
	m_bMoveSelect = false;
	memset(&m_pStartPoint, 0x00, sizeof(POINT));
	memset(&m_pEndPoint, 0x00, sizeof(POINT));
	m_StartPos.cx = 0;
	m_StartPos.cy = 0;

	m_pMoveItem = NULL;
	m_bMoveItemEffect = true;
	m_bNeedRefresh = false;
	m_uMoveSelectIndex = -1;

	m_uTextStyle = DT_VCENTER;
	m_dwTextColor = 1;
	m_dwDisabledTextColor = 1;
	m_iFont = -1;
	m_bShowHtml = false;
	::ZeroMemory(&m_rcTextPadding, sizeof(m_rcTextPadding));

	m_bStyleAddItem = false;
	m_bSelTopIndex = false;
	m_nEditIndex = -1;
}

CIConListUIEx::~CIConListUIEx(void)
{
	m_dequeueReusableItem.Empty();
	if (m_pMoveItem != NULL)
	{
		m_pMoveItem->DestroyWindow();
		if (m_pMoveItem)
		{
			delete m_pMoveItem;
			m_pMoveItem = NULL;
		}
//		m_pMoveItem = NULL;
	}
}

void CIConListUIEx::CloseMoveWindow()                       //关闭移动窗口
{
	if (m_pMoveItem != NULL)
	{
		m_pMoveItem->DestroyWindow();
		delete m_pMoveItem;
		m_pMoveItem = NULL;
	}
}

//设置是否需要移动效果
void CIConListUIEx::SetMoveItemEffect(bool bMoveItem /* = true */)
{
	m_bMoveItemEffect = bMoveItem;
}

//设置是否有额外的添加选项目
void CIConListUIEx::SetAddItemStyle(bool bAddItem /* = false */)
{
	m_bStyleAddItem = bAddItem;
// 	if (m_bStyleAddItem)
// 	{
// 		SetDelayedDestroy(true);
// 	}
}

void CIConListUIEx::SetVerticalScrollPos(int nPos)
{
	if (m_pList && (nPos >= 0))
	{
		m_pList->SetVerticalScrollPos(nPos);
	}
}

bool CIConListUIEx::IsSelectTopIndex()
{
	return m_bSelTopIndex;
}

int CIConListUIEx::FindIndexbyPoint(POINT &pt)   //通过指定的点找到对应的索引
{
	int nSelIndex = -1;
	for (int i = 0; i< GetCount(); i++)
	{
		CIConListContainerElementUIEx *pcontrol = static_cast<CIConListContainerElementUIEx *>(GetItemAt(i));
		if (pcontrol == NULL)
		{
			continue;
		}
		RECT rc = pcontrol->GetPos();
		if (::PtInRect(&rc, pt))
		{
			SelectMoveTemp(i);
			nSelIndex = i;
			break;
		}
	}
	return nSelIndex;
}

int CIConListUIEx::MoveToPoint(POINT &pt)     //移动到指定的点
{
	if (m_pMoveItem != NULL)
	{
		m_pMoveItem->MoveWindowFollowPoint(pt);
	}
	return 0;
}

//返回移动效果在值
bool CIConListUIEx::IsMoveItemEffect() const
{
	return m_bMoveItemEffect;
}

//设置是否需要刷新数据
bool CIConListUIEx::NeedFreshData(bool bFRresh /* = true */)
{
	bool bold = m_bNeedRefresh;
	m_bNeedRefresh = bFRresh;
	return bold;
}

LPCTSTR CIConListUIEx::GetClass() const
{
	return _T("IConListUIEx");
}

void CIConListUIEx::SetItemWidth(int nItemWidth)    //设置项宽度
{
	if (nItemWidth > 0)
	{
		m_nItemWidth = nItemWidth;
	}
	CalShowItemNum();
}


void CIConListUIEx::SetItemHeight(int nItemHeigh)
{
	if (nItemHeigh < m_pManager->GetDefaultFontInfo()->tm.tmHeight + 8)
	{
		m_nItemHeight =  m_pManager->GetDefaultFontInfo()->tm.tmHeight + 8;
	}
	else
		m_nItemHeight = nItemHeigh;

	CalShowItemNum();
}

//获取项高度
int  CIConListUIEx::GetItemHeight() const           
{
	return m_nItemHeight;
}

void CIConListUIEx::SetItemCount(int nNums)
{
	if (nNums >= 0)
	{
		m_nItems = nNums;
		if (m_bStyleAddItem)
		{
			m_nItems++;
		}
	}
}

//获取总项数
int  CIConListUIEx::GetItemCount() const
{
	return m_nItems;
}

//获取顶部索引
int  CIConListUIEx::GetTopIndex() const
{
	return m_nTopIndex;
}

//获取可以显示的Item数
int  CIConListUIEx::GetShowItems() const
{
	return m_nShowItems;
}

// 添加选中项目，不刷新
void CIConListUIEx::AddSelItem(int nIndex)
{
	m_aSelItems.Add((LPVOID)(nIndex));
}

// 清空选中项目，不刷新 
void CIConListUIEx::EmptySelItem()
{
	m_aSelItems.Empty();
}


//获取可复用的项
CIConListContainerElementUIEx *CIConListUIEx::GetReuseableItem()
{
	CIConListContainerElementUIEx *pListItem = NULL;
	if (m_dequeueReusableItem.GetSize() > 0)
	{
		pListItem = static_cast<CIConListContainerElementUIEx *>(m_dequeueReusableItem.GetAt(0));
		m_dequeueReusableItem.Remove(0);
	}

	return pListItem;
}

//获取指定索引的项
CControlUI* CIConListUIEx::GetItemExAt(int iIndex) const
{
	if (iIndex >= m_nItems)
	{
		return NULL;
	}
	if (m_pListDataSource == NULL)
		return NULL;
	return static_cast<CControlUI*>(m_pListDataSource->listItemForIndex(iIndex));
}


//获取总项数 
int CIConListUIEx::GetAllCount() const
{
	return m_nItems;
}

// DWORD WINAPI FuncThread_ScrollDown(LPVOID lpThreadParameter)
// {
// 	CIConListUIEx *plist = (CIConListUIEx *)lpThreadParameter;
// 	if (plist == NULL)
// 	{
// 		return 0L;
// 	}
// 
// 	return plist->ScrollDownLine();
// }

//查询是否有数据隐藏在前面
bool CIConListUIEx::IsHiddenItemInTop()   
{
	return m_pList->IsHiddenItemInTop();
}

//在底部是否有隐藏的项
bool CIConListUIEx::IsHiddenItemInBottom()
{
	return m_pList->IsHiddenItemInBottom();
}

bool CIConListUIEx::ScrollDownLine()
{
	m_bNeedCheckListData = false;
	if (m_pListDataSource == NULL)
	{
		return false;
	}
	if (m_nTopIndex + m_nShowItems + 1> m_nItems)
	{
		if (IsHiddenItemInBottom())
		{
			//			m_nTopIndex ++;
			return true;
		}
		return false;
	}

	for (int i = 0; i< m_nShowItemsInColumn;i++)
	{
		CControlUI *pListItem = GetItemAt(0);
		if (pListItem != NULL)
		{
			m_dequeueReusableItem.Add(pListItem);
		}

		RemoveAt(0);
		
		if (m_nTopIndex + m_nShowItems >= m_nItems)
		{
			m_nTopIndex++;
			continue;
		}

		CIConListContainerElementUIEx *pListElement = static_cast<CIConListContainerElementUIEx *>(m_pListDataSource->listItemForIndex(m_nTopIndex + m_nShowItems));
		if (pListElement == NULL)
		{
			m_bNeedCheckListData = true;
			return false;
		}
		if ((m_nTopIndex + m_nShowItems) == m_nEditIndex)
		{
			pListElement->SetbVisibleEdit(true);
		}
		else
		{
			pListElement->SetbVisibleEdit(false);
		}
		int nindx = m_aSelItems.Find((LPVOID)(m_nTopIndex + m_nShowItems + NUM_LEFT));   //插入时检查是否选中项
		if (nindx >= 0)
		{
			pListElement->Select(true);
		}
		else
			pListElement->Select(false);

		Add(pListElement);
		m_nTopIndex++;
	}
	return true;
}

//往上走一行
bool CIConListUIEx::ScrollUpLine()
{
	if (m_pListDataSource == NULL)
	{
		return false;
	}

	if (m_nTopIndex <= 0)   //少于预留
	{
		return false;
	}
	
	if (IsHiddenItemInTop())   //有数据还在前面，先显示数据
	{
		//		m_nTopIndex--;
		return true;
	}
	
	if (m_pList == NULL)
	{
		return false;
	}

	for (int i = 0; i < m_pList->GetColumns(); i++)
	{
		m_nTopIndex--;
		if (m_nTopIndex >=0 )
		{
			CIConListContainerElementUIEx *pListElement = static_cast<CIConListContainerElementUIEx *>(m_pListDataSource->listItemForIndex(m_nTopIndex));
			if (pListElement == NULL)
			{
				return false;
			}
			if (m_nTopIndex == m_nEditIndex)
			{
				pListElement->SetbVisibleEdit(true);
			}
			else
			{
				pListElement->SetbVisibleEdit(false);
			}
			int nindx = m_aSelItems.Find((LPVOID)(m_nTopIndex));   //插入时检查是否选中项
			if (nindx >= 0)
			{
				pListElement->Select(true);
			}
			else
				pListElement->Select(false);

			AddAt(pListElement, 0);
		}
		
		if (m_nTopIndex + m_nShowItems < m_nItems)
		{
			CControlUI *pListItem = GetItemAt(GetCount() - 1);
			if (pListItem != NULL)
			{
				m_dequeueReusableItem.Add(pListItem);
			}

			RemoveAt(GetCount() - 1);
		}

	}

	// 	m_dequeueReusableItem.Add(m_pListDataSource->listItemForIndex(m_nTopIndex + m_nShowItems));
	return true;
}

// bool CIConListUIEx::ScrollDown()
// {
// 	DWORD dwThreadID = 0;
// 	HANDLE hThread = CreateThread(NULL,0, &FuncThread_ScrollDown, (LPVOID)this, 0, &dwThreadID);
// 	if (hThread == NULL)
// 	{
// 		return false;
// 	}
// 	return true;
// }

//根据指定索引刷新pos和数据
bool CIConListUIEx::RefreshPos(int nTopIndex, bool bInit /* = true */)
{
	if (m_pListDataSource == NULL)
	{
		return false;
	}
	if (bInit)
	{
		Init();
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
	if (m_pList)
	{
		m_pList->setUp(TRUE);
	}
// 	if (nTopIndex == 0)
// 	{
		RefreshListData(m_nTopIndex, false);
// 	}
// 	else
// 	{
	    SIZE szSrc = GetScrollPos();
		SIZE sz = {0, 0};
		//int nLines = m_nTopIndex/m_nShowItemsInColumn;
		int nLines = (m_nShowItemsInColumn == 0) ? m_nTopIndex : m_nTopIndex / m_nShowItemsInColumn;//www 2016.03.21 m_nShowItemsInColumn可能为0不能除
 		sz.cy = nLines * (m_nItemHeight + GetChildPadding());
		SetVerticalScrollPos(sz.cy);
// 		if (sz.cy != szSrc.cy)
// 		{
// 			SetScrollPos(sz);
// 		}
// 		else
// 			RefreshListData(m_nTopIndex, false);
// 	}
// 	if (!m_bFreshed)
// 	{
// 		RefreshListData(nTopIndex);
// 	}
	return true;
}

//删除指定的数据
bool CIConListUIEx::DeleteIndexData(int Index)        
{
	m_bFreshed = false;
	m_aSelItems.Empty();
	m_bSelTopIndex = false;
	RefreshPos(m_nTopIndex);
	return true;

}

bool CIConListUIEx::InsetIndexData(int Index)         //在制定索引之前插入数据
{
	if (Index < 0)
	{
		return false;
	}
	Init();
	if (Index >= m_nItems)
	{
		return false;
	}
	if (Index < m_nTopIndex)
	{
		m_nTopIndex += m_nShowItemsInColumn;
	}

	for (int i = 0; i< m_aSelItems.GetSize(); i++)
	{
		int nSel = (int)m_aSelItems.GetAt(i);
		if (nSel >= Index)
		{
			m_aSelItems.SetAt(i, (LPVOID)(nSel + 1));
		}
	}

	RefreshPos(m_nTopIndex, false);
//	RefreshListData(m_nTopIndex, false);
	return true;
}

// 插入最后
bool CIConListUIEx::PushBackData(int nIndex)
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

	if (nIndex != (m_nItems - 1))   // 是否插入最后一项判断
	{
		m_nItems--;
		return false;
	}
	
	Init(); 
	
	//SIZE szPos = m_pList->GetScrollPos();
	//if (m_nItems > m_nShowItems)   // 如果显示不全
	//{
	//	m_pList->SetAllNeedHeight(m_nItems * m_nItemHeight);
	//}
	//else
	//	m_pList->SetAllNeedHeight(0);
	
	if (GetCount() < m_nShowItems)
	{
		if (m_pListDataSource)
		{
			CIConListContainerElementUIEx *pListItem = static_cast<CIConListContainerElementUIEx *>(m_pListDataSource->listItemForIndex(nIndex));
			if (pListItem == NULL)
			{
				return false;
			}
			if (nIndex == m_nEditIndex)
			{
				pListItem->SetbVisibleEdit(true);
			}
			else
			{
				pListItem->SetbVisibleEdit(false);
			}
			Add(pListItem);
		}
	}
	
	//m_pList->SetPos(m_pList->GetPos());
	//m_pList->SetVerticalScrollPos(m_nItemHeight * m_nTopIndex);
	//Invalidate();
	return true;
}



bool CIConListUIEx::RefreshListDatabyLine(int nLines) //根据指定行刷新数据
 {
	m_bFreshed = true;
	if (nLines >= m_nShowNeedLines - m_nShowItemsInLine)
	{
		nLines = m_nShowNeedLines - m_nShowItemsInLine;
	}
	if (nLines < 0)
	{
		nLines = 0;
	}
	
// 	if (IsHiddenItemInBottom())
// 	{
// 		nLines ++;
// 	}

	m_nTopIndex = nLines * m_nShowItemsInColumn;
	RemoveAll();

// 	TCHAR wstr[256] = {0};
// 	::wsprintf(wstr, _T("CIConListUIEx::RefreshListDatabyLine m_nTopIndex = %d, m_nItems = %d, Current ThreadID=%d\n"), m_nTopIndex, m_nItems, GetCurrentThreadId());
// 	OutputDebugStr(wstr);

	//重新生成数据
	for (int i = 0; i< m_nShowItems + NUM_LEFT; i++)
	{
		if (m_nTopIndex + i > m_nItems - 1)
		{
			break;
		}
// 		TCHAR wstr[256] = {0};
// 		::wsprintf(wstr, _T("CIConListUIEx::RefreshListDatabyLine listItemForIndex nIndex = %d\n"), m_nTopIndex + i);
// 		OutputDebugStr(wstr);

		CIConListContainerElementUIEx *pListItem = static_cast<CIConListContainerElementUIEx *>(m_pListDataSource->listItemForIndex(m_nTopIndex + i));
		if (pListItem == NULL)
		{
			continue;
		}
		if ((m_nTopIndex + i) == m_nEditIndex)
		{
			pListItem->SetbVisibleEdit(true);
		}
		else
		{
			pListItem->SetbVisibleEdit(false);
		}
		int nindx = m_aSelItems.Find((LPVOID)(m_nTopIndex + i));   //插入时检查是否选中项
		if (nindx >= 0)
		{
			pListItem->Select(true);
		}
		else
			pListItem->Select(false);

		Add(pListItem);
	}
	return true;

}

//根据指定索引刷新数据
bool CIConListUIEx::RefreshListData(int nTopIndex, bool bInit /* = true */)
{
	m_bFreshed = true;
	if (m_pListDataSource == NULL)
	{
		return false;
	}
	if (bInit)
	{
		Init();
	}
	
	if (nTopIndex < 0)
	{
		nTopIndex = 0;
	}
	int nlines = (m_nShowItemsInColumn == 0) ? nTopIndex : nTopIndex / m_nShowItemsInColumn;//www 2016.02.26 m_nShowItemsInColumn可能为0不能除
	//OutputDebugStr(_T("CIConListUIEx::RefreshListData RefreshListDatabyLine \n"));
	RefreshListDatabyLine(nlines);
 	return true;
}

//显示指定的项
void CIConListUIEx::EnsureVisible(int iIndex)
{
	if( iIndex < 0 || iIndex >= m_nItems) 
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
	int iOffset = (m_nTopIndex - nOldTop)*(m_nItemHeight + GetChildPadding());
	if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() )
		iOffset -= m_pHorizontalScrollBar->GetFixedHeight();
	sz.cy += iOffset;
	SetScrollPos(sz);

}

//计算可以显示的项数
void CIConListUIEx::CalShowItemNum()
{
	if (m_nItemHeight == 0)
	{
		m_nShowItems = 0;
		return;
	}
	if (m_pList == NULL)
	{
		return;
	}

	RECT rc = m_pList->GetPos();
	int nHeight = rc.bottom - rc.top;
	if ((nHeight == 0) && (rc.right - rc.left == 0))
	{
		m_nShowItemsInColumn = m_nShowItemsInLine = 0;
		m_nShowItems = 0;
		m_bNeedRefresh = true;   //需要刷新
		return;
	}
	int nItemHeight = m_nItemHeight + m_pList->GetChildPadding();
	m_nShowItemsInLine = nHeight/nItemHeight;
	int nLeave = nHeight%nItemHeight;
	if (nHeight%nItemHeight != 0)  //不是刚刚好全部显示
	{
		m_pList->SetLeavePos(m_nItemHeight - nLeave);
		m_nShowItemsInLine ++;
	}
	else
	{
		m_pList->SetLeavePos(0);
	}
	int iDeskWidth = m_nItemWidth + m_pList->GetChildPadding();
	int column = (rc.right - rc.left) / iDeskWidth ;
	if( column < 1 ) column = 1;
	m_pList->SetColumns(column);
	m_nShowItemsInColumn = m_pList->GetColumns();
	m_nShowItems = m_nShowItemsInColumn*m_nShowItemsInLine;
}

// 设置数据源，不做任何界面操作
void CIConListUIEx::SetDataSource(IListDataSource *ListDataSource)
{
	m_pListDataSource = ListDataSource;
}

//设置数据来源并刷新
void CIConListUIEx::SetListDataSource(IListDataSource *ListDataSource, bool bChangeTopIndex /* = false */)
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
		if (m_pManager != NULL) {
			m_pManager->SendNotify(this, _T("itemunselect"), DUILIB_LIST_ITEMUNSELECT, 0, 0);
		}
		m_bSelTopIndex = false;
	}
	m_dequeueReusableItem.Empty();
	if (!bChangeTopIndex)
	{
//		RefreshListData(m_nTopIndex);
		RefreshPos(m_nTopIndex);
		return;
	}
	Init();
	
	if (m_nShowItems == 0)   //显示数据为0
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
		nNumInset =  m_nShowItems;

	for (int i = 0; i< nNumInset + NUM_LEFT; i++)
	{
		if (i >= m_nItems)
		{
			break;
		}
		CIConListContainerElementUIEx *pListItem = static_cast<CIConListContainerElementUIEx *>(m_pListDataSource->listItemForIndex(i));
		if (pListItem == NULL)
		{
			return;
		}
		if (i == m_nEditIndex)
		{
			pListItem->SetbVisibleEdit(true);
		}
		else
		{
			pListItem->SetbVisibleEdit(false);
		}
		Add(pListItem);
	}
	SIZE sz;
	sz.cx = sz.cy = 0;
	SetScrollPos(sz);
}

void CIConListUIEx::SetAllNeedHeight(int nHeight)
{
	if (m_pList && (nHeight >= 0))
	{
		m_pList->SetAllNeedHeight(nHeight);
	}
}

void CIConListUIEx::Init(bool bRefreshSize /* = true */)
{
	if ((m_pListDataSource != NULL) && (m_pList != NULL))
	{
		if (bRefreshSize)
		{
			SetItemCount(m_pListDataSource->numberOfRows());
		}
		CalShowItemNum();
		if (m_nShowItems == 0)
		{
			return;
		}
		if (m_nItems > m_nShowItems)
		{
			m_nShowNeedLines = m_nItems/m_nShowItemsInColumn;
			if (m_nItems%m_nShowItemsInColumn != 0)
			{
				m_nShowNeedLines ++;
			}
			int nHeight = m_pList->GetChildPadding() + m_nItemHeight;
			m_nShowNeedHeights = m_nShowNeedLines * nHeight/* - m_pList->GetChildPadding()*/;
			m_pList->SetAllNeedHeight(m_nShowNeedHeights);
			if (m_pList->GetVerticalScrollBar())
			{
				m_pList->GetVerticalScrollBar()->SetScrollRange(m_nShowNeedHeights - m_rcItem.bottom + m_rcItem.top);
			}
		}
		else
		{
			int nHeight = m_pList->GetChildPadding() + m_nItemHeight;
			int nLines = 0;
			if (m_nShowItemsInColumn != 0)
			{
				nLines = m_nItems/m_nShowItemsInColumn;
			}
			if (m_nItems%m_nShowItemsInColumn != 0)
			{
				nLines ++;
			}
			if (nHeight * nLines - m_pList->GetChildPadding() > m_rcItem.bottom - m_rcItem.top)
			{
				m_nShowNeedHeights = m_nShowItemsInLine*nHeight/* - m_pList->GetChildPadding()*/;
				m_pList->SetAllNeedHeight(m_nShowNeedHeights);
				if (m_pList->GetVerticalScrollBar())
				{
					m_pList->GetVerticalScrollBar()->SetScrollRange(m_nShowNeedHeights - m_rcItem.bottom + m_rcItem.top);
				}
			}
			else
			{
				m_nShowNeedHeights = 0;
				m_pList->SetAllNeedHeight(0);
				if (m_pList->GetVerticalScrollBar())
				{
					m_pList->GetVerticalScrollBar()->SetScrollRange(m_nShowNeedHeights);
				}
			}
		}
	}
}

UINT CIConListUIEx::GetControlFlags() const
{
	return UIFLAG_TABSTOP;
}

LPVOID CIConListUIEx::GetInterface(LPCTSTR pstrName)
{
	if( _tcscmp(pstrName, _T("IconListEx")) == 0 ) return static_cast<CIConListUIEx*>(this);
	if( _tcscmp(pstrName, _T("IList")) == 0 ) return static_cast<IListUI*>(this);
	if( _tcscmp(pstrName, _T("IListOwner")) == 0 ) return static_cast<IListOwnerUI*>(this);
	return CVerticalLayoutUI::GetInterface(pstrName);
}

CControlUI* CIConListUIEx::GetItemAt(int iIndex) const
{
	return m_pList->GetItemAt(iIndex);
}

int CIConListUIEx::GetItemIndex(CControlUI* pControl) const
{
	return m_pList->GetItemIndex(pControl);
}

bool CIConListUIEx::SetItemIndex(CControlUI* pControl, int iIndex)
{

	int iOrginIndex = m_pList->GetItemIndex(pControl);
	if( iOrginIndex == -1 ) return false;
	if( iOrginIndex == iIndex ) return true;

	IListItemUI* pSelectedListItem = NULL;
	// 	if( m_iCurSel >= 0 ) 
	// 		pSelectedListItem = static_cast<IListItemUI*>(GetItemAt(m_iCurSel)->GetInterface(_T("ListItem")));
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
	// 	if( m_iCurSel >= 0 && pSelectedListItem != NULL ) m_iCurSel = pSelectedListItem->GetIndex();
	return true;
}

int CIConListUIEx::GetCount() const
{
	return m_pList->GetCount();
}

bool CIConListUIEx::Add(CControlUI* pControl)
{
	// Override the Add() method so we can add items specifically to
	// the intended widgets. Headers are assumed to be
	// answer the correct interface so we can add multiple list headers.

	if (pControl == NULL)
	{
		return false;
	}
	// The list items should know about us
	IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
	if( pListItem != NULL ) {
		pListItem->SetOwner(this);
		pListItem->SetIndex(GetCount());
	}
	return m_pList->Add(pControl);
}

bool CIConListUIEx::AddAt(CControlUI* pControl, int iIndex)
{
	// Override the AddAt() method so we can add items specifically to
	if (pControl == NULL)
	{
		return false;
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
	/*	if( m_iCurSel >= iIndex ) m_iCurSel += 1;*/
	return true;
}

bool CIConListUIEx::Remove(CControlUI* pControl)
{

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

	return true;
}

bool CIConListUIEx::RemoveAt(int iIndex)
{
	if (!m_pList->RemoveAt(iIndex)) return false;

	for(int i = iIndex; i < m_pList->GetCount(); ++i) {
		CControlUI* p = m_pList->GetItemAt(i);
		IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(_T("ListItem")));
		if( pListItem != NULL ) pListItem->SetIndex(i);
	}

	return true;
}

void CIConListUIEx::RemoveAll()
{
	/*	m_iCurSel = -1;*/
	m_iExpandedItem = -1;
	m_pList->RemoveAll();
}

void CIConListUIEx::SetPos(RECT rc)
{
	if (!m_pList)
	{
		return;
	}
	int nNeedPos =m_nShowNeedHeights;
	SIZE szPos = m_pList->GetScrollPos();
	int nHeight = m_rcItem.bottom - m_rcItem.top;
	CVerticalLayoutUI::SetPos(rc);
	// Determine general list information and the size of header columns
	if (m_pList)
	{
		m_pList->SetPos(rc);
	}
	Init();
	int nNeedPos2 = m_nShowNeedHeights;
	int nHeight2 = m_rcItem.bottom - m_rcItem.top;
	if ((szPos.cy == 0) || (nNeedPos == 0))
	{
		RefreshListData(m_nTopIndex);
	}
	else
	{
		SIZE szPos2;
		szPos2.cx = szPos.cx;
		float fRate = ((float)(szPos.cy + nHeight))/((float)nNeedPos);
		float fPosCy = nNeedPos2*fRate + 0.5;
		szPos2.cy = (int)fPosCy;
		szPos2.cy -= nHeight2;
		m_pList->FreshDataBySize(szPos2);
	}
	

// 	if (m_bNeedRefresh)
// 	{
// 		RefreshListData(m_nTopIndex);
// 		m_bNeedRefresh = false;
// 		if (m_pList)
// 		{
// 			SIZE sz = m_pList->GetScrollPos();
// 			if (m_nTopIndex > 0)
// 			{
// 				int nLines = m_nTopIndex/m_nShowItemsInColumn;
// 				sz.cy = nLines*m_nItemHeight + (nLines - 1)*GetChildPadding();
// 			}
// 			else
// 			{
// 				sz.cy = 0;
// 			}
// 
// 			m_pList->SetScrollLinePos(sz);
// 		}
// 
//	}
}

void CIConListUIEx::DoEvent(TEventUI& event)
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
	if (event.Type == UIEVENT_RBUTTONDOWN)   //右键消息
	{
		if (!IsEnabled())
		{
			return;
		}
		UnSelectAllItems(true);
		if (m_pManager)
		{
			m_pManager->SendNotify(this, _T("itemrbuttonclick"), DUILIB_LIST_ITEM_RBUTTON_CLICK, -1, 0, true);
		}
		return;
	}
	if (event.Type == UIEVENT_BUTTONDOWN)
	{
		if (!IsEnabled())
		{
			return;
		}
		m_bLButtonDown = true;
		
 		m_nStartSelItem = m_nTopIndex - 1;
 		if (m_nStartSelItem < 0)
 		{
			m_nStartSelItem = 0;
 		}
		m_bLButtonDown = true;
		SetMCaptured(this);
		m_pStartPoint = event.ptMouse;
		m_StartPos = GetScrollPos();
		m_pTempPoint = m_pStartPoint;
		m_pEndPoint = m_pStartPoint;
		CalStartPoint();
		
	}
	if (event.Type == UIEVENT_BUTTONUP)
	{
		if (!IsEnabled())
		{
			return;
		}
		m_bLButtonDown = false;
		if (!m_bMoveSelect)
		{
			
			UnSelectAllItems(true);
		}
		else
		{
			m_bMoveSelect = false;
		}
		ReleaseMCaptured();
		Invalidate();
	}
	if (event.Type == UIEVENT_MOUSEMOVE)
	{
		if (!IsEnabled())
		{
			return;
		}
		if (m_bItemLButtonDown && m_bMoveItemEffect)
		{
// 			if (m_pMoveItem == NULL)
// 			{
// 				m_pMoveItem = new CMoveItemWnd();
// 				ASSERT(m_pMoveItem);
// 				m_pMoveItem->Init(this);
// 				m_bHasMoveItem = true;
// 				if (m_pManager)
// 				{
// 					m_pManager->SendNotify(this, _T("startmoveitem"), DUILIB_LIST_MOVE_ITEM, (WPARAM)&m_aSelItems, 0);
// 				}
// 			}//Modify by www 2015.03.23; Look at the following!
			if (!m_bHasMoveItem)
			{
				m_bHasMoveItem = true;
				if (m_pManager)
				{
					m_pManager->SendNotify(this, _T("startmoveitem"), DUILIB_LIST_MOVE_ITEM, (WPARAM)&m_aSelItems, 0);
				}
			}
// 			if (m_pMoveItem != NULL)
// 			{
// 				m_pMoveItem->MoveWindowFollowPoint(event.ptMouse);
// 				return;
// 			}
		}
		if (!m_bLButtonDown)
			return;

		m_pEndPoint = event.ptMouse;
		if (m_pStartPoint.y == 0)
		{
			m_pStartPoint = m_pEndPoint;
			m_StartPos = GetScrollPos();
			m_pTempPoint = m_pStartPoint;
			return;
		}
		if (m_pEndPoint.y < m_rcItem.bottom && m_pEndPoint.y > m_rcItem.top)
		{
			m_bMoveSelect = true;
			SelectbyDrag(true);
			Invalidate();
			return;
		}
		if (((m_pEndPoint.y - m_pTempPoint.y) > 5) && (m_pEndPoint.y > m_rcItem.bottom))   //向下
		{
			if (IsFocused() && IsVisible())
			{
				//判断是否需要向下走一行
				int nIndex = m_nEndSelItem - m_nTopIndex;
				if (IsHiddenItemInBottom() && nIndex >= m_nShowItems - m_nShowItemsInColumn*2)
				{
					LineDown();
				}
				else if (nIndex >= m_nShowItems - m_nShowItemsInColumn)
				{
					LineDown();
				}

				//选择
				if (m_nEndSelItem <= m_nItems - m_nShowItemsInColumn)
				{
					m_nEndSelItem += m_nShowItemsInColumn;
				}
			}
			m_pTempPoint = event.ptMouse;
		}
		else if ((m_pTempPoint.y -  m_pEndPoint.y) > 5 && (m_pEndPoint.y < m_rcItem.top))   //向上
		{
			if (IsFocused() && IsVisible())
			{
				int nIndex = m_nEndSelItem - m_nTopIndex;
				if (IsHiddenItemInTop() && nIndex < m_nShowItemsInColumn*2)
				{
					LineUp();
				}
				else if (nIndex < m_nShowItemsInColumn)
				{
					LineUp();
				}
				if (m_nEndSelItem >= m_nShowItemsInColumn)
				{
					m_nEndSelItem -= m_nShowItemsInColumn;
				}
			}
			m_pTempPoint = event.ptMouse;
		}
		SelectbyDrag(true);
		Invalidate();
		return;
	}
	switch( event.Type ) {
	case UIEVENT_KEYDOWN:
		switch( event.chKey ) {
	case VK_UP:
		if (m_aSelItems.GetSize() > 0)
		{
			if (!IsEnabled())
			{
				return;
			}
			if ((!m_bSingleSel) && (GetKeyState(VK_SHIFT)< 0))
			{
				if (IsFocused() && IsVisible())
				{
					int nIndex = m_nEndSelItem - m_nTopIndex;
					if (IsHiddenItemInTop() && nIndex < m_nShowItemsInColumn*2)
					{
						LineUp();
					}
					else if (nIndex < m_nShowItemsInColumn)
					{
						LineUp();
					}

					if (m_nFocusItem >= m_nShowItemsInColumn)
					{
						m_nFocusItem -= m_nShowItemsInColumn;
					}
					m_nEndSelItem = m_nFocusItem;
					SetItemSelect(m_nStartSelItem, m_nEndSelItem, true);
				}

			}
			else if ((!m_bSingleSel) &&(GetKeyState(VK_CONTROL) < 0))
			{
			}
			else
			{
				int nIndex = GetMinSelItemIndex() - m_nTopIndex;
				if (IsHiddenItemInTop() && nIndex < m_nShowItemsInColumn*2)  //上面有隐藏时，第二行数据
				{
					LineUp();
				}
				else if (nIndex < m_nShowItemsInColumn)             //第一行
				{
					nIndex += m_nShowItemsInColumn;    //第一行，相对nIndex不改变
					LineUp();
				}

				nIndex = nIndex - m_nShowItemsInColumn;
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
				if (IsHiddenItemInBottom() && nIndex >= m_nShowItems - m_nShowItemsInColumn*2)
				{
// 					if (m_nTopIndex + m_nShowItems + NUM_LEFT + 1<= m_nItems)
// 					{
// 						nIndex = nIndex - 1;
// 					}
					LineDown();
				}
				else if (nIndex >= m_nShowItems - m_nShowItemsInColumn)
				{
					LineDown();
				}

				if (IsFocused() && IsVisible())
				{
					if (m_nFocusItem <= m_nItems - m_nShowItemsInColumn)
					{
						m_nFocusItem += m_nShowItemsInColumn;
					}
					m_nEndSelItem = m_nFocusItem;
					SetItemSelect(m_nStartSelItem, m_nEndSelItem, true);
				}
			}
			else if ((!m_bSingleSel) &&((event.wKeyState & MK_CONTROL) == MK_CONTROL))
			{
			}
			else
			{
				int nIndex = GetMaxSelItemIndex() - m_nTopIndex;
				if (m_nTopIndex + nIndex >= m_nItems - m_nShowItemsInColumn)   //最后一行，不需要再变动
				{
					return;
				}

				if (IsHiddenItemInBottom() && nIndex >= m_nShowItems - m_nShowItemsInColumn*2)
				{
// 					if (m_nTopIndex + m_nShowItems <= m_nItems )
// 					{
// 						nIndex = nIndex - m_nShowItemsInColumn;
// 					}
					LineDown();
				}
				else if (nIndex >= m_nShowItems - m_nShowItemsInColumn)   //最后一行
				{ 
					nIndex -= m_nShowItemsInColumn;   //相对位置不变
					LineDown();
				}
				nIndex = nIndex + m_nShowItemsInColumn;
				if ( (nIndex + 1) > m_pList->GetCount())
				{
					SelectItem(GetCount() - 1, true) ;
				}
				else
				{
					SelectItem(nIndex, true);
				}	
			}
			Invalidate();
		}
		return;
	case VK_LEFT:
		{
			if (!IsEnabled())
			{
				return;
			}
			if ((!m_bSingleSel) && (GetKeyState(VK_SHIFT) < 0))  //多选
			{
				int nIndex = m_nEndSelItem - m_nTopIndex;
				if (nIndex%m_nShowItemsInColumn > 0)
				{
					m_nFocusItem --;
					m_nEndSelItem = m_nFocusItem;
					SetItemSelect(m_nStartSelItem, m_nEndSelItem, true);
				}
			}
			else if ((!m_bSingleSel) && (GetKeyState(VK_CONTROL) < 0))
			{
			}
			else
			{
				int nIndex = GetMaxSelItemIndex() - m_nTopIndex;
				if (nIndex%m_nShowItemsInColumn > 0)
				{
					nIndex--;
// 					m_nEndSelItem = nIndex;
					SelectItem(nIndex);
				}
			}

		}
		return;
	case VK_RIGHT:
		{
			if (!IsEnabled())
			{
				return;
			}
			if ((!m_bSingleSel) && (GetKeyState(VK_SHIFT) < 0))  //多选
			{
				int nIndex = m_nEndSelItem - m_nTopIndex;
				if (nIndex%m_nShowItemsInColumn < m_nShowItemsInColumn - 1)
				{
					m_nFocusItem ++;
					m_nEndSelItem = m_nFocusItem;
					SetItemSelect(m_nStartSelItem, m_nEndSelItem, true);
				}
			}
			else if ((!m_bSingleSel) && (GetKeyState(VK_CONTROL) < 0))
			{
			}
			else
			{
				int nIndex = GetMaxSelItemIndex() - m_nTopIndex;
				if (nIndex%m_nShowItemsInColumn < m_nShowItemsInColumn - 1)
				{
					nIndex++;
//					m_nEndSelItem = nIndex;
					SelectItem(nIndex);
				}
			}
		}
		return;
	case VK_PRIOR:
		{		
			if (!IsEnabled())
			{
				return;
			}
			int nIndex = GetMinSelItemIndex() - m_nTopIndex;
//			PageUp();
			if (m_pList != NULL)
			{
				RECT rc = m_pList->GetPos();
				int nLines = (rc.bottom - rc.top)/(m_nItemHeight + m_pList->GetChildPadding());
				for (int i = 0; i< nLines; i++)
				{
					LineUp();
				}
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
	case VK_NEXT:
		{
			if (!IsEnabled())
			{
				return;
			}
			int nIndex = GetMaxSelItemIndex() - m_nTopIndex;
//			PageDown();
			if (m_pList != NULL)
			{
				RECT rc = m_pList->GetPos();
				int nLines = (rc.bottom - rc.top)/(m_nItemHeight + m_pList->GetChildPadding());
				for (int i = 0; i< nLines; i++)
				{
					LineDown();
				}
				if ( (nIndex + 1) > m_pList->GetCount())
				{
					SelectItem(GetCount() - 1, true) ;
				}
				else
				{
					SelectItem(nIndex, true);
				}	
			}
		}
		return;
	case VK_HOME:
		{
			if (!IsEnabled())
			{
				return;
			}
			HomeUp();
			if (m_pList->GetCount() > 0)
				SelectItem(0, true);
		}
		return;
	case VK_END:
		{
			if (!IsEnabled())
			{
				return;
			}
			EndDown();
			if (m_pList->GetCount() > 0)
				SelectItem(m_pList->GetCount() - 1, true);
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
				return;
			}
			switch( LOWORD(event.wParam) ) 
			{
			case SB_LINEUP:
				if( m_bScrollSelect && m_bSingleSel) 
					SelectItem(FindSelectable((int)m_aSelItems.GetAt(0) - m_nTopIndex - 1, false));
				else 
				{
					LineUp();
					if (!IsFocused() || (m_pManager && m_pManager->GetFocus() != this))
					{
						SetFocus();
					}
				}
				return;
			case SB_LINEDOWN:
				if( m_bScrollSelect && m_bSingleSel) 
					SelectItem(FindSelectable((int)m_aSelItems.GetAt(0) - m_nTopIndex + 1, true));
				else 
				{
					LineDown();
					if (!IsFocused() || (m_pManager && m_pManager->GetFocus() != this))
					{
						SetFocus();
					}
				}
				return;
			}
		}
		break;
	}
	CVerticalLayoutUI::DoEvent(event);
}

CListHeaderUI* CIConListUIEx::GetHeader() const
{
	return NULL;
}

CContainerUI* CIConListUIEx::GetList() const
{
	return m_pList;
}

bool CIConListUIEx::GetScrollSelect()
{
	return m_bScrollSelect;
}

void CIConListUIEx::SetScrollSelect(bool bScrollSelect)
{
	m_bScrollSelect = bScrollSelect;
}

int CIConListUIEx::GetCurSel() const
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

int CIConListUIEx::GetMinSelItemIndex()
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

int CIConListUIEx::GetMaxSelItemIndex()
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

void CIConListUIEx::CalStartPoint()      //计算开始拖动选择时的状态
{
	if (m_pList == NULL)
	{
		return;
	}
	if (GetCount() == 0)
	{
		return;
	}

	if ((m_pStartPoint.x == 0) && (m_pStartPoint.y == 0))
	{
		m_nStartLine = 0;
		m_nStartColumn = 0;
		return;
	}
	
	//计算所在列
	int nChildPadding = m_pList->GetChildPadding();
	int nDx = m_pStartPoint.x - m_rcItem.left;
	int nItemWidth = m_nItemWidth + nChildPadding;
	if (nDx <= nChildPadding/2)
	{
		m_nStartColumn = 0;
	}
	else
	{
		m_nStartColumn = 0;
		nDx = nDx - nChildPadding/2;
		while(nDx > 0)
		{
			m_nStartColumn ++;
			nDx = nDx - m_nItemWidth;
			if (nDx > 0)
			{
				m_nStartColumn ++;
				nDx = nDx - nChildPadding;
			}
		}
	}

	if (m_nStartColumn > m_nShowItemsInColumn*2)
	{
		m_nStartColumn = m_nShowItemsInColumn*2;
	}

	//计算所在行
	if (!IsHiddenItemInTop())
	{
		int ndy = m_pStartPoint.y - m_rcItem.top;
		if (ndy > m_rcItem.bottom - m_rcItem.top)
		{
			ndy = m_rcItem.bottom - m_rcItem.top;
		}
		if (ndy <= m_nItemHeight)
		{
			m_nStartLine = m_nTopIndex/m_nShowItemsInColumn*2;
		}
		else
		{
			m_nStartLine = m_nTopIndex/m_nShowItemsInColumn*2;
			ndy -= m_nItemHeight;
			while (ndy > 0)
			{
				m_nStartLine ++;
				ndy = ndy - nChildPadding;
				if (ndy > 0)
				{
					m_nStartLine++;
					ndy = ndy - m_nItemHeight;
				}
			}
		}
	}
	else
	{
		int ndy = m_rcItem.bottom - m_pStartPoint.y;
		if (ndy > m_rcItem.bottom - m_rcItem.top)
		{
			ndy = m_rcItem.bottom - m_rcItem.top;
		}

		if (ndy <= m_nItemHeight)
		{
			m_nStartLine = m_nTopIndex/m_nShowItemsInColumn*2 + m_nShowItemsInLine*2 - 1;
		}
		else
		{
			m_nStartLine = m_nTopIndex/m_nShowItemsInColumn*2 + m_nShowItemsInLine*2 - 1;
			ndy -= m_nItemHeight;
			while (ndy > 0)
			{
				m_nStartLine --;
				ndy = ndy - nChildPadding;
				if (ndy > 0 )
				{
					m_nStartLine --;
					ndy -= m_nItemHeight;
				}
			}
		}

	}
}

void CIConListUIEx::CalEndPoint()
{
	if (m_pList == NULL)
	{
		return;
	}
	if (GetCount() == 0)
	{
		return;
	}

	if ((m_pEndPoint.x == 0) && (m_pEndPoint.y == 0))
	{
		m_nEndLine = 0;
		m_nEndColumn = 0;
		return;
	}

	//计算所在列
	int nChildPadding = m_pList->GetChildPadding();
	int nItemWidth = m_nItemWidth + nChildPadding;
	int nDx = m_pEndPoint.x - m_rcItem.left;
	if (nDx <= nChildPadding/2)
	{
		m_nEndColumn = 0;
	}
	else
	{
		m_nEndColumn = 0;
		nDx = nDx - nChildPadding/2;
		while(nDx > 0)
		{
			m_nEndColumn ++;
			nDx = nDx - m_nItemWidth;
			if (nDx > 0)
			{
				m_nEndColumn ++;
				nDx = nDx - nChildPadding;
			}
		}
	}
	
	if (m_nEndColumn < 0)
	{
		m_nEndColumn = 0;
	}
	if (m_nEndColumn > m_nShowItemsInColumn*2)
	{
		m_nEndColumn = m_nShowItemsInColumn*2;
	}


	//计算所在行
	if (!IsHiddenItemInTop())
	{
		int ndy = m_pEndPoint.y - m_rcItem.top;
		if (ndy > m_rcItem.bottom - m_rcItem.top)
		{
			ndy = m_rcItem.bottom - m_rcItem.top;
		}
		if (ndy <= m_nItemHeight)
		{
			m_nEndLine = m_nTopIndex/m_nShowItemsInColumn*2;
		}
		else
		{
			m_nEndLine = m_nTopIndex/m_nShowItemsInColumn*2;
			ndy -= m_nItemHeight;
			while (ndy > 0)
			{
				m_nEndLine ++;
				ndy = ndy - nChildPadding;
				if (ndy > 0)
				{
					m_nEndLine++;
					ndy = ndy - m_nItemHeight;
				}
			}
		}
	}
	else
	{
		int ndy = m_rcItem.bottom - m_pEndPoint.y;
		if (ndy > m_rcItem.bottom - m_rcItem.top)
		{
			ndy = m_rcItem.bottom - m_rcItem.top;
		}

		if (ndy <= m_nItemHeight)
		{
			m_nEndLine = m_nTopIndex/m_nShowItemsInColumn*2 + m_nShowItemsInLine*2 - 1;
		}
		else
		{
			m_nEndLine = m_nTopIndex/m_nShowItemsInColumn*2 + m_nShowItemsInLine*2 - 1;
			ndy -= m_nItemHeight;
			while (ndy > 0)
			{
				m_nEndLine --;
				ndy = ndy - nChildPadding;
				if (ndy > 0 )
				{
					m_nEndLine --;
					ndy -= m_nItemHeight;
				}
			}
		}

	}
}


bool CIConListUIEx::IsListDataChanged()   // 判断当前数据是否修改
{
	if (m_bNeedCheckListData)
	{
		int nItem = m_nItems;
		if (m_bStyleAddItem)
		{
			nItem--;
		}
		if (m_pListDataSource )
		{
			if (nItem > m_pListDataSource->numberOfRows())   //当前总项目数大于数据源总数
			{
				return true;
			}
		}
	}
	return false;
}

bool CIConListUIEx::SelectItem(int iIndex, bool bTakeFocus /* = false */)
{
	if( iIndex < 0 ) return false;
	if (m_bStyleAddItem && (m_nTopIndex + iIndex == m_nItems - 1))
	{
		return false;
	}
	CControlUI* pControl = GetItemAt(iIndex);
	if( pControl == NULL ) return false;
	if( !pControl->IsVisible() ) return false;
	if( !pControl->IsEnabled() ) return false;

	if (bTakeFocus == false)
	{
		m_nFocusItem = iIndex + m_nTopIndex;
		m_nStartSelItem = m_nEndSelItem = iIndex + m_nTopIndex;
	}

	if(m_bSingleSel && m_aSelItems.GetSize() > 0) {
		CControlUI* pControl = GetItemAt((int)m_aSelItems.GetAt(0) - m_nTopIndex);
		if( pControl != NULL) {
			IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
			if( pListItem != NULL ) pListItem->Select(false);
		}		
	}	

	UnSelectAllItems();

	IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
	if( pListItem == NULL ) return false;
	if( !pListItem->Select(true) ) {
		if (m_pManager != NULL) {
			m_pManager->SendNotify(this, _T("itemunselect"), DUILIB_LIST_ITEMUNSELECT, 0, 0);
		}
		return false;
	}

	m_aSelItems.Add((LPVOID)(m_nTopIndex + iIndex));
	m_bSelTopIndex = (iIndex == 0) ? true : m_bSelTopIndex;

	//	EnsureVisible(iIndex);
	if( bTakeFocus ) 
		SetFocus();
	if( m_pManager != NULL ) {
		m_pManager->SendNotify(this, _T("itemselect"), DUILIB_LIST_ITEMSELECT, iIndex + m_nTopIndex);
	}

	return true;
}

bool CIConListUIEx::SelectMultiItembyDrag(int iIndex, bool bTakeFocus /* = false */)
{
	if( iIndex < 0 ) return false;
	if (iIndex >= m_nItems) return false;

	if (m_aSelItems.Find((LPVOID)(iIndex )) >= 0)
		return false;
	
	if (m_bStyleAddItem && (iIndex == m_nItems - 1))
	{
		// 最后一项不被选中
		return false;
	}
	m_aSelItems.Add((LPVOID)(iIndex));
	m_bSelTopIndex = (iIndex == 0) ? true : m_bSelTopIndex;

	CControlUI* pControl = GetItemAt(iIndex - m_nTopIndex);
	if( pControl == NULL ) return false;
	if( !pControl->IsVisible() ) return false;
	if( !pControl->IsEnabled() ) return false;

	if (bTakeFocus == false)
	{
		m_nFocusItem = iIndex ;
		m_nStartSelItem = m_nEndSelItem = iIndex;
	}

	IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
	if( pListItem == NULL ) return false;
	if( !pListItem->Select(true) ) 
	{
		int nFind = m_aSelItems.Find((LPVOID)iIndex);
		if (-1 != nFind)
		{
			m_aSelItems.Remove(nFind);
		}
		return false;
	}

	//	EnsureVisible(iIndex);
	if( bTakeFocus ) pControl->SetFocus();
	if( m_pManager != NULL ) {
		m_pManager->SendNotify(this, _T("itemselect"), DUILIB_LIST_ITEMSELECT, (WPARAM)iIndex + m_nTopIndex,(LPARAM)m_aSelItems.GetSize());
	}

	return true;
}


bool CIConListUIEx::SelectMultiItem(int iIndex, bool bTakeFocus /* = false */, bool bSendMessage/* = true*/)
{
	if (m_bStyleAddItem && (m_nTopIndex + iIndex == m_nItems - 1))
	{
		return FALSE;
	}
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
		m_nFocusItem = iIndex + m_nTopIndex;
		m_nStartSelItem = m_nEndSelItem = iIndex + m_nTopIndex;
	}

	if (m_aSelItems.Find((LPVOID)(iIndex + m_nTopIndex)) >= 0)
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

	m_aSelItems.Add((LPVOID)(iIndex + m_nTopIndex));
	m_bSelTopIndex = (iIndex == 0) ? true : m_bSelTopIndex;

	//	EnsureVisible(iIndex);
	if( bTakeFocus ) pControl->SetFocus();
	if( m_pManager != NULL ) {
		if (bSendMessage)
			m_pManager->SendNotify(this, _T("itemselect"), DUILIB_LIST_ITEMSELECT, iIndex + m_nTopIndex);
	}

	return true;
}

//拖动时，选择的项目
bool CIConListUIEx::SelectMoveTemp(int iIndex)
{
	if (m_bSingleSel)
	{
		return SelectItem(iIndex, false);
	}
	
	if (m_uMoveSelectIndex != iIndex)
	{
		UnSelectMoveTemp();
	}

	if( iIndex < 0 ) return false;
	CControlUI* pControl = GetItemAt(iIndex);
	if( pControl == NULL ) return false;
	if( !pControl->IsVisible() ) return false;
	if( !pControl->IsEnabled() ) return false;

	if (m_aSelItems.Find((LPVOID)(iIndex + m_nTopIndex)) >= 0)
		return false;

	if (m_uMoveSelectIndex != iIndex)
	{
		IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
		if( pListItem == NULL ) return false;
		if( !pListItem->Select(true) ) 
		{		
			return false;
		}
		m_uMoveSelectIndex = iIndex;
		if( m_pManager != NULL ) {
			m_pManager->SendNotify(this, _T("itemmove"), DUILIB_LIST_ITEM_MOVE, iIndex + m_nTopIndex);
		}
	}

	return true;
}
//拖动时，取消选择的项目
bool CIConListUIEx::UnSelectMoveTemp()
{
	int iIndex = m_uMoveSelectIndex;
	if( iIndex < 0 ) return false;
	CControlUI* pControl = GetItemAt(iIndex);
	if( pControl == NULL ) return false;
	if( !pControl->IsVisible() ) return false;
	if( !pControl->IsEnabled() ) return false;
	IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
	if( pListItem == NULL ) return false;

	iIndex += m_nTopIndex;
	int aIndex = m_aSelItems.Find((LPVOID)iIndex);
	if (aIndex >= 0)
		return false;

	if( !pListItem->Select(false) ) {		
		return false;
	}
	return true;
}


//拖动选择
void CIConListUIEx::SelectbyDrag(bool bTakeFocus/* = false*/)
{
	if(m_bSingleSel )
	{
		return;
	}
	if (GetCount() == 0)
	{
		return;
	}

	CalEndPoint();

	int nStartLine = m_nStartLine;
	int nEndLine = m_nEndLine;
	if (nStartLine > m_nEndLine)
	{
		nStartLine = m_nEndLine;
		nEndLine = m_nStartLine;
	}
	
	int nStartColumn = m_nStartColumn;
	int nEndColumn = m_nEndColumn;
	if (nStartColumn > nEndColumn)
	{
		nStartColumn = m_nEndColumn;
		nEndColumn = m_nStartColumn;
	}

	UnSelectAllItems();
	if (nEndColumn > m_nShowItemsInColumn*2)
	{
		nEndColumn = m_nShowItemsInColumn*2;
	}

	int nTopLine = m_nTopIndex/m_nShowItemsInColumn;
	for (int nLine = nStartLine; nLine<= nEndLine; nLine++)
	{
		//if ((nLine%2 != 0) && IsHiddenItemInBottom())
		//{
		//	continue;
		//}
		//else if ((nLine%2 == 0) && IsHiddenItemInTop())
		//{
		//	continue;
		//}
		for (int nCol = nStartColumn; nCol <= nEndColumn; nCol++)
		{
			if (nCol%2 != 0)
			{
				SelectMultiItembyDrag((nLine/2)*m_nShowItemsInColumn + nCol/2, bTakeFocus);
			}
		}
	}
	if (m_aSelItems.IsEmpty())
	{
		if (m_pManager != NULL) {
			m_pManager->SendNotify(this, _T("itemunselect"), DUILIB_LIST_ITEMUNSELECT, 0, 0);
		}
	}
}


//该函数只用于多选，单选不可用
bool CIConListUIEx::SetItemSelect(int nStart, int nEnd, bool bTakeFocus /* = false */)
{
	if(m_bSingleSel )
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
	if (nEndTemp >= m_nItems)
	{
		return false;
	}


	UnSelectAllItems();

	//记录所有选中项
	for (int iIndex = nStartTemp; iIndex <= nEndTemp; iIndex++)
	{
		if (m_bStyleAddItem && (iIndex == m_nItems - 1))
		{
			continue;
		}
		m_aSelItems.Add((LPVOID)iIndex);
		m_bSelTopIndex = (iIndex == 0) ? true : m_bSelTopIndex;
	}

	//焦点跟随
	if (bTakeFocus)
	{
		m_nFocusItem = nEnd;
// 		CControlUI* pControl = GetItemAt(nEnd - m_nTopIndex);
// 		if( pControl == NULL ) return false;
// 		if( !pControl->IsVisible() ) return false;
// 		if( !pControl->IsEnabled() ) return false;
		this->SetFocus();
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

		for (int i = npos; i<= nStep; i++)
		{
			if (m_bStyleAddItem && (m_nTopIndex + i == m_nItems - 1))
			{
				continue;
			}
			CControlUI* pControl = GetItemAt(i);
			if( pControl == NULL ) continue;
			if( !pControl->IsVisible() ) continue;
			if( !pControl->IsEnabled() ) continue;


			IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
			if( pListItem == NULL ) 
				continue;
			if( !pListItem->Select(true, false) ) 
			{
				int nFind = m_aSelItems.Find((LPVOID)i);
				if (-1 != nFind)
				{
					m_aSelItems.Remove(nFind);
				}
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

		for (int i = nPosStart; i<= nStep; i++)
		{
			if (m_bStyleAddItem && (m_nTopIndex + i == m_nItems - 1))
			{
				continue;
			}
			CControlUI* pControl = GetItemAt(i);
			if( pControl == NULL ) continue;
			if( !pControl->IsVisible() ) continue;
			if( !pControl->IsEnabled() ) continue;


			IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
			if( pListItem == NULL ) 
				continue;
			if( !pListItem->Select(true, false) ) 
			{		
				int nFind = m_aSelItems.Find((LPVOID)i);
				if (-1 != nFind)
				{
					m_aSelItems.Remove(nFind);
				}
				continue;
			}
		}
	}

	Invalidate();

	if( m_pManager != NULL ) {
		m_pManager->SendNotify(this, _T("itemselect"), DUILIB_LIST_ITEMSELECT, (WPARAM)m_aSelItems.GetAt(0),(LPARAM)m_aSelItems.GetSize());
	}

	return true;
}

void CIConListUIEx::SetSingleSelect(bool bSingleSel)
{
	m_bSingleSel = bSingleSel;
	UnSelectAllItems(true);
}

bool CIConListUIEx::GetSingleSelect() const
{
	return m_bSingleSel;
}

bool CIConListUIEx::UnSelectItem(int iIndex, bool bSendMessage/* = true*/)
{
	if( iIndex < 0 ) return false;
	CControlUI* pControl = GetItemAt(iIndex);
	if( pControl == NULL ) return false;
	if( !pControl->IsVisible() ) return false;
	if( !pControl->IsEnabled() ) return false;
	IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
	if( pListItem == NULL ) return false;

	iIndex += m_nTopIndex;
	int aIndex = m_aSelItems.Find((LPVOID)iIndex);
	if (aIndex < 0)
		return false;

	// 	m_nFocusItem = aIndex;
	// 	m_nStartSelItem = m_nEndSelItem = aIndex;

	if( !pListItem->Select(false) ) {		
		return false;
	}

	m_aSelItems.Remove(aIndex);
	m_bSelTopIndex = (iIndex == 0) ? false : m_bSelTopIndex;
	//if (m_aSelItems.IsEmpty())   // 取消选中，发送消息
	{
		if (m_pManager != NULL) {
			if (bSendMessage)
				if (bSendMessage)
					m_pManager->SendNotify(this, _T("itemunselect"), DUILIB_LIST_ITEMUNSELECT, iIndex + 1, 0);
		}
	}

	return true;
}

void CIConListUIEx::SelectAllItems()
{
	UnSelectAllItems();
	CControlUI* pControl;

	for (int i = 0; i < GetCount(); ++i)   //更新当前所有项的状态
	{
		if (m_bStyleAddItem && (m_nTopIndex + i == m_nItems - 1))
		{
			continue;
		}
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
	}

	for (int i = 0; i < m_nItems; i++)     //所有可选
	{
		if (m_bStyleAddItem && (i == m_nItems - 1))
		{
			break;
		}
		m_aSelItems.Add((LPVOID)i);
	}
	m_bSelTopIndex = true;
	m_nFocusItem = 0;
	m_nStartSelItem = m_nEndSelItem = 0;
}

void CIConListUIEx::UnSelectAllItems()
{
	CControlUI* pControl;
	for (int i = 0; !m_bSingleSel && i < GetCount(); ++i)    //更新当前项状态为不可选
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
		if( !pListItem->Select(false))
			continue;		
	}
	m_aSelItems.Empty();
	m_bSelTopIndex = false;
}

void CIConListUIEx::UnSelectAllItems(bool bSendMessage)
{
	UnSelectAllItems();
	if (bSendMessage)
	{
		if (m_pManager != NULL) {
			m_pManager->SendNotify(this, _T("itemunselect"), DUILIB_LIST_ITEMUNSELECT, 0, 0);
		}
	}
}


int CIConListUIEx::GetSelectItemCount() const
{
	return m_aSelItems.GetSize();
}

int CIConListUIEx::GetNextSelItem(int nItem) const
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

int CIConListUIEx::SetFocusItem(int nItem, bool bTakeFocus /*= false*/)
{
	if ((nItem < 0) || (nItem > (GetCount() - 1)))
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

//设置多选中结束项
int CIConListUIEx::SetEndItem(int nItem, bool bTakeFocus/* = false*/)
{
	if ((nItem < 0) || (nItem > (GetCount() - 1)))
	{
		if (m_pManager != NULL) {
			m_pManager->SendNotify(this, _T("itemunselect"), DUILIB_LIST_ITEMUNSELECT, 0, 0);
		}
		return  -1;
	}
	int oldItem = m_nEndSelItem;
	m_nEndSelItem = nItem + m_nTopIndex;	
	if (bTakeFocus)
	{
		m_nFocusItem =	nItem + m_nTopIndex;
	}
	SetItemSelect(m_nStartSelItem, m_nEndSelItem, true);
	return oldItem;	
}

int CIConListUIEx::GetEndItem() const
{
	return m_nEndSelItem;
}

//设置鼠标是否被按下
bool CIConListUIEx::SetLButtonState(bool bDown)
{
	bool bOldState = m_bItemLButtonDown;
	m_bItemLButtonDown = bDown;
	if (m_bItemLButtonDown == false)
	{
		if (m_pMoveItem)
		{
			m_pMoveItem->DestroyWindow();
			delete m_pMoveItem;
			m_pMoveItem = NULL;
		}
		m_bHasMoveItem = false;
		UnSelectMoveTemp();
		m_bLButtonDown = false;
	}
	return bOldState;
}

//是否已经移动了选中项
bool CIConListUIEx::IsMovedItem() const       
{
	return m_bHasMoveItem;
}


bool CIConListUIEx::GetLButtonState() const
{
	return m_bItemLButtonDown;
}

void CIConListUIEx::SetStartPoint(POINT pt)
{
	m_pStartPoint = pt;	
	m_pEndPoint = pt;
	m_pTempPoint = pt;
	m_StartPos = GetScrollPos();
	CalStartPoint();
}

//获取选中的数组
CStdPtrArray CIConListUIEx::GetSelArray() const
{
	return m_aSelItems;
}


TListInfoUI* CIConListUIEx::GetListInfo()
{
	return &m_ListInfo;
}

int CIConListUIEx::GetChildPadding() const
{
	return m_pList->GetChildPadding();
}

void CIConListUIEx::SetChildPadding(int iPadding)
{
	m_pList->SetChildPadding(iPadding);
}

void CIConListUIEx::SetItemFont(int index)
{
	m_ListInfo.nFont = index;
	NeedUpdate();
}

void CIConListUIEx::SetItemTextStyle(UINT uStyle)
{
	m_ListInfo.uTextStyle = uStyle;
	NeedUpdate();
}

void CIConListUIEx::SetItemTextPadding(RECT rc)
{
	m_ListInfo.rcTextPadding = rc;
	NeedUpdate();
}

RECT CIConListUIEx::GetItemTextPadding() const
{
	return m_ListInfo.rcTextPadding;
}

void CIConListUIEx::SetItemTextColor(DWORD dwTextColor)
{
	m_ListInfo.dwTextColor = dwTextColor;
	Invalidate();
}

void CIConListUIEx::SetItemBkColor(DWORD dwBkColor)
{
	m_ListInfo.dwBkColor = dwBkColor;
	Invalidate();
}

void CIConListUIEx::SetItemBkImage(LPCTSTR pStrImage)
{
	m_ListInfo.sBkImage = pStrImage;
	Invalidate();
}

void CIConListUIEx::SetAlternateBk(bool bAlternateBk)
{
	m_ListInfo.bAlternateBk = bAlternateBk;
	Invalidate();
}

DWORD CIConListUIEx::GetItemTextColor() const
{
	return m_ListInfo.dwTextColor;
}

DWORD CIConListUIEx::GetItemBkColor() const
{
	return m_ListInfo.dwBkColor;
}

LPCTSTR CIConListUIEx::GetItemBkImage() const
{
	return m_ListInfo.sBkImage;
}

bool CIConListUIEx::IsAlternateBk() const
{
	return m_ListInfo.bAlternateBk;
}

void CIConListUIEx::SetSelectedItemTextColor(DWORD dwTextColor)
{
	m_ListInfo.dwSelectedTextColor = dwTextColor;
	Invalidate();
}

void CIConListUIEx::SetSelectedItemBkColor(DWORD dwBkColor)
{
	m_ListInfo.dwSelectedBkColor = dwBkColor;
	Invalidate();
}

void CIConListUIEx::SetSelectedItemImage(LPCTSTR pStrImage)
{
	m_ListInfo.sSelectedImage = pStrImage;
	Invalidate();
}

DWORD CIConListUIEx::GetSelectedItemTextColor() const
{
	return m_ListInfo.dwSelectedTextColor;
}

DWORD CIConListUIEx::GetSelectedItemBkColor() const
{
	return m_ListInfo.dwSelectedBkColor;
}

LPCTSTR CIConListUIEx::GetSelectedItemImage() const
{
	return m_ListInfo.sSelectedImage;
}

void CIConListUIEx::SetHotItemTextColor(DWORD dwTextColor)
{
	m_ListInfo.dwHotTextColor = dwTextColor;
	Invalidate();
}

void CIConListUIEx::SetHotItemBkColor(DWORD dwBkColor)
{
	m_ListInfo.dwHotBkColor = dwBkColor;
	Invalidate();
}

void CIConListUIEx::SetHotItemImage(LPCTSTR pStrImage)
{
	m_ListInfo.sHotImage = pStrImage;
	Invalidate();
}

DWORD CIConListUIEx::GetHotItemTextColor() const
{
	return m_ListInfo.dwHotTextColor;
}
DWORD CIConListUIEx::GetHotItemBkColor() const
{
	return m_ListInfo.dwHotBkColor;
}

LPCTSTR CIConListUIEx::GetHotItemImage() const
{
	return m_ListInfo.sHotImage;
}

void CIConListUIEx::SetDisabledItemTextColor(DWORD dwTextColor)
{
	m_ListInfo.dwDisabledTextColor = dwTextColor;
	Invalidate();
}

void CIConListUIEx::SetDisabledItemBkColor(DWORD dwBkColor)
{
	m_ListInfo.dwDisabledBkColor = dwBkColor;
	Invalidate();
}

void CIConListUIEx::SetDisabledItemImage(LPCTSTR pStrImage)
{
	m_ListInfo.sDisabledImage = pStrImage;
	Invalidate();
}

void CIConListUIEx::SetMoveSelectImage(LPCTSTR pStrImage)
{
	m_strMoveSelectImage = pStrImage;
}

LPCTSTR CIConListUIEx::GetMoveSelectImage()
{
	return m_strMoveSelectImage;
}

DWORD CIConListUIEx::GetDisabledItemTextColor() const
{
	return m_ListInfo.dwDisabledTextColor;
}

DWORD CIConListUIEx::GetDisabledItemBkColor() const
{
	return m_ListInfo.dwDisabledBkColor;
}

LPCTSTR CIConListUIEx::GetDisabledItemImage() const
{
	return m_ListInfo.sDisabledImage;
}

DWORD CIConListUIEx::GetItemLineColor() const
{
	return m_ListInfo.dwLineColor;
}

void CIConListUIEx::SetItemLineColor(DWORD dwLineColor)
{
	m_ListInfo.dwLineColor = dwLineColor;
	Invalidate();
}

bool CIConListUIEx::IsItemShowHtml()
{
	return m_ListInfo.bShowHtml;
}

void CIConListUIEx::SetItemShowHtml(bool bShowHtml)
{
	if( m_ListInfo.bShowHtml == bShowHtml ) return;

	m_ListInfo.bShowHtml = bShowHtml;
	NeedUpdate();
}

void CIConListUIEx::SetMultiExpanding(bool bMultiExpandable)
{
	m_ListInfo.bMultiExpandable = bMultiExpandable;
}

bool CIConListUIEx::ExpandItem(int iIndex, bool bExpand /*= true*/)
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

int CIConListUIEx::GetExpandedItem() const
{
	return m_iExpandedItem;
}

void CIConListUIEx::Scroll(int dx, int dy)
{
	if( dx == 0 && dy == 0 ) return;
	SIZE sz = m_pList->GetScrollPos();
	m_pList->SetScrollPos(CSize(sz.cx + dx, sz.cy + dy));
}

void CIConListUIEx::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if( _tcscmp(pstrName, _T("header")) == 0 ) GetHeader()->SetVisible(_tcscmp(pstrValue, _T("hidden")) != 0);
	else if( _tcscmp(pstrName, _T("headerbkimage")) == 0 ) GetHeader()->SetBkImage(pstrValue);
	else if( _tcscmp(pstrName, _T("scrollselect")) == 0 ) SetScrollSelect(_tcscmp(pstrValue, _T("true")) == 0);
	else if( _tcscmp(pstrName, _T("multiexpanding")) == 0 ) SetMultiExpanding(_tcscmp(pstrValue, _T("true")) == 0);
	else if ( _tcscmp(pstrName, _T("moveselectimage")) == 0 ) SetMoveSelectImage(pstrValue);
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
// 	else if( _tcscmp(pstrName, _T("itemendellipsis")) == 0 ) {
// 		if( _tcscmp(pstrValue, _T("true")) == 0 ) m_ListInfo.uTextStyle |= DT_END_ELLIPSIS;
// 		else m_ListInfo.uTextStyle &= ~DT_END_ELLIPSIS;
// 	}    
	else if( _tcscmp(pstrName, _T("endellipsis")) == 0 ) {
		if( _tcscmp(pstrValue, _T("true")) == 0 )
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
	else if (_tcscmp(pstrName, _T("moveitem")) == 0)          //add by lighten 2013.04.02， 是否需要移动效果
	{
		if (_tcscmp(pstrValue, _T("true")) == 0)
		{
			SetMoveItemEffect(true);
		}
		else
			SetMoveItemEffect(false);
	}
	else if (_tcscmp(pstrName, _T("additemstyle")) == 0)          //add by lighten 2014.05.08， 是否有额外的添加选项
	{
		if (_tcscmp(pstrValue, _T("true")) == 0)
		{
			SetAddItemStyle(true);
		}
		else
			SetAddItemStyle(false);
	}
	else if( _tcscmp(pstrName, _T("itemtextpadding")) == 0 ) {
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
	else if( _tcscmp(pstrName, _T("align")) == 0 ) {
		if( _tcsstr(pstrValue, _T("left")) != NULL ) {
			m_uTextStyle &= ~(DT_CENTER | DT_RIGHT | DT_TOP | DT_BOTTOM);
			m_uTextStyle |= DT_LEFT;
		}
		if( _tcsstr(pstrValue, _T("center")) != NULL ) {
			m_uTextStyle &= ~(DT_LEFT | DT_RIGHT | DT_TOP | DT_BOTTOM);
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
	else if( _tcscmp(pstrName, _T("font")) == 0 ) m_iFont = _ttoi(pstrValue);
	else if( _tcscmp(pstrName, _T("textcolor")) == 0 ) {
		if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		LPTSTR pstr = NULL;
		DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
		m_dwTextColor = clrColor;
	}
	else if( _tcscmp(pstrName, _T("disabledtextcolor")) == 0 ) {
		if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		LPTSTR pstr = NULL;
		DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
		m_dwDisabledTextColor = clrColor;
	}
	else if( _tcscmp(pstrName, _T("textpadding")) == 0 ) {
		RECT rcTextPadding = { 0 };
		LPTSTR pstr = NULL;
		rcTextPadding.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
		rcTextPadding.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
		rcTextPadding.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
		rcTextPadding.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);    
		m_rcTextPadding = rcTextPadding;
	}
	else if( _tcscmp(pstrName, _T("showhtml")) == 0 ) m_bShowHtml = _tcscmp(pstrValue, _T("true")) == 0?true:false;	else CVerticalLayoutUI::SetAttribute(pstrName, pstrValue);
}

void CIConListUIEx::PaintSelectItem(HDC hDC, const RECT &rcPaint)
{
	if (m_pList == NULL)
	{
		return;
	}
	int dx = rcPaint.left - m_rcItem.left;
	int dy = rcPaint.top - m_rcItem.top ;
	for (int i =0; i< m_pList->GetCount(); i++)
	{
		CIConListContainerElementUIEx *pListItem = static_cast<CIConListContainerElementUIEx*>(m_pList->GetItemAt(i));
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

void CIConListUIEx::PaintText(HDC hDC)
{
	if (m_nItems != 0)
	{
		return;
	}
	if( m_dwTextColor == 0 ) m_dwTextColor = m_pManager->GetDefaultFontColor();
	if( m_dwDisabledTextColor == 0 ) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

	if( m_sText.IsEmpty() ) return;
	int nLinks = 0;
	RECT rc = m_rcItem;
	m_rcTextPadding.top = (m_rcItem.bottom - m_rcItem.top)*2/5;
	rc.left += m_rcTextPadding.left;
	rc.right -= m_rcTextPadding.right;
	rc.top += m_rcTextPadding.top;
	rc.bottom -= m_rcTextPadding.bottom;
	if( IsEnabled() ) {
		if( m_bShowHtml )
			CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, m_sText, m_dwTextColor, \
			NULL, NULL, nLinks, DT_SINGLELINE | m_uTextStyle);
		else
			CRenderEngine::DrawText(hDC, m_pManager, rc, m_sText, m_dwTextColor, \
			m_iFont, DT_SINGLELINE | m_uTextStyle);
	}
	else {
		if( m_bShowHtml )
			CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, m_sText, m_dwDisabledTextColor, \
			NULL, NULL, nLinks, DT_SINGLELINE | m_uTextStyle);
		else
			CRenderEngine::DrawText(hDC, m_pManager, rc, m_sText, m_dwDisabledTextColor, \
			m_iFont, DT_SINGLELINE | m_uTextStyle);
	}
}


void CIConListUIEx::DoPaint(HDC hDC, const RECT& rcPaint)
{
	if (!m_bLButtonDown)   //鼠标放开，取消选择框
	{
		CContainerUI::DoPaint(hDC, rcPaint);
		return;
	}

	CContainerUI::DoPaint(hDC, rcPaint);
	PaintText(hDC);

	if ((m_pStartPoint.x == 0) && (m_pStartPoint.y == 0))
	{
		return;
	}

	RECT rcTemp;
	SIZE szCurPos = GetScrollPos();
	int ndy = szCurPos.cy - m_StartPos.cy;
	if (m_pStartPoint.x > m_pEndPoint.x)
	{
		rcTemp.left = m_pEndPoint.x;
		rcTemp.right = m_pStartPoint.x;
	}
	else
	{
		rcTemp.left = m_pStartPoint.x;
		rcTemp.right = m_pEndPoint.x;
	}

	if (m_pStartPoint.y - ndy > m_pEndPoint.y)
	{
		rcTemp.top = m_pEndPoint.y;
		rcTemp.bottom = m_pStartPoint.y - ndy;
	}
	else
	{
		rcTemp.top = m_pStartPoint.y - ndy;
		rcTemp.bottom = m_pEndPoint.y;
	}
	RECT rcList;
	if (m_pList != NULL)
	{
		rcList = m_pList->GetPos();
		rcList.right -= m_pList->GetVerticalFixWidth();
	}
	
	RECT rc;
	if (IntersectRect(&rc, &rcTemp, &rcList))
	{
		if (!m_strMoveSelectImage.IsEmpty())
		{
			if (!DrawImage(hDC, m_strMoveSelectImage, rc, NULL))
			{
				m_strMoveSelectImage.Empty();
			}			
		}
		else
			CRenderEngine::DrawRect(hDC, rc, 1, 0xDDFF00FF);
	}
}

IListCallbackUI* CIConListUIEx::GetTextCallback() const
{
	return m_pCallback;
}

void CIConListUIEx::SetTextCallback(IListCallbackUI* pCallback)
{
	m_pCallback = pCallback;
}

SIZE CIConListUIEx::GetScrollPos() const
{
	return m_pList->GetScrollPos();
}

SIZE CIConListUIEx::GetScrollRange() const
{
	return m_pList->GetScrollRange();
}

void CIConListUIEx::SetScrollPos(SIZE szPos)
{
	m_pList->SetScrollPos(szPos);
}

void CIConListUIEx::LineUp()
{
	m_pList->LineUp();
}

void CIConListUIEx::LineDown()
{
	m_pList->LineDown();
}

void CIConListUIEx::PageUp()
{
	m_pList->PageUp();
}

void CIConListUIEx::PageDown()
{
	m_pList->PageDown();
}

void CIConListUIEx::HomeUp()
{
	m_pList->HomeUp();
}

void CIConListUIEx::EndDown()
{
	m_pList->EndDown();
}

void CIConListUIEx::LineLeft()
{
	m_pList->LineLeft();
}

void CIConListUIEx::LineRight()
{
	m_pList->LineRight();
}

void CIConListUIEx::PageLeft()
{
	m_pList->PageLeft();
}

void CIConListUIEx::PageRight()
{
	m_pList->PageRight();
}

void CIConListUIEx::HomeLeft()
{
	m_pList->HomeLeft();
}

void CIConListUIEx::EndRight()
{
	m_pList->EndRight();
}

void CIConListUIEx::EnableScrollBar(bool bEnableVertical, bool bEnableHorizontal)
{
	m_pList->EnableScrollBar(bEnableVertical, bEnableHorizontal);
}

CScrollBarUI* CIConListUIEx::GetVerticalScrollBar() const
{
	return m_pList->GetVerticalScrollBar();
}

CScrollBarUI* CIConListUIEx::GetHorizontalScrollBar() const
{
	return m_pList->GetHorizontalScrollBar();
}

void CIConListUIEx::SetEditIndex(int iIndex)
{
	m_nEditIndex = iIndex;
}

int CIConListUIEx::GetEditiIndex()
{
	return m_nEditIndex;
}


//////////////////////////////////////////////////////////////////////////////////////////////
/////

CIConPhotoBackupListUI::CIConPhotoBackupListUI()
{

}

CIConPhotoBackupListUI::~CIConPhotoBackupListUI(void)
{

}

LPCTSTR CIConPhotoBackupListUI::GetClass() const
{
	return _T("IConPhotoBackupListUI");
}

LPVOID CIConPhotoBackupListUI::GetInterface(LPCTSTR pstrName)
{
	if (_tcscmp(pstrName, _T("IConPhotoBackupList"))) return static_cast<CIConPhotoBackupListUI*>(this);
	if (_tcscmp(pstrName, _T("IconListEx")) == 0) return static_cast<CIConListUIEx*>(this);
	if (_tcscmp(pstrName, _T("IList")) == 0) return static_cast<IListUI*>(this);
	if (_tcscmp(pstrName, _T("IListOwner")) == 0) return static_cast<IListOwnerUI*>(this);
	return CVerticalLayoutUI::GetInterface(pstrName);
}

void CIConPhotoBackupListUI::DoEvent(TEventUI& event)
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
		return;
	}
	if (event.Type == UIEVENT_BUTTONDOWN)
	{
		if (!IsEnabled())
		{
			return;
		}
		m_bLButtonDown = true;
		/*UnSelectAllItems(true);
		m_nStartSelItem = m_nTopIndex - 1;
		if (m_nStartSelItem < 0)
		{
			m_nStartSelItem = 0;
		}
		m_bLButtonDown = true;*/
		SetMCaptured(this);
		m_pStartPoint = event.ptMouse;
		m_StartPos = GetScrollPos();
		m_pTempPoint = m_pStartPoint;
		m_pEndPoint = m_pStartPoint;
		CalStartPoint();

	}
	if (event.Type == UIEVENT_BUTTONUP)
	{
		if (!IsEnabled())
		{
			return;
		}
		m_bLButtonDown = false;
		ReleaseMCaptured();
		Invalidate();
	}
	if (event.Type == UIEVENT_MOUSEMOVE)
	{
		if (!IsEnabled())
		{
			return;
		}
		if (m_bItemLButtonDown && m_bMoveItemEffect)
		{
			// 			if (m_pMoveItem == NULL)
			// 			{
			// 				m_pMoveItem = new CMoveItemWnd();
			// 				ASSERT(m_pMoveItem);
			// 				m_pMoveItem->Init(this);
			// 				m_bHasMoveItem = true;
			// 				if (m_pManager)
			// 				{
			// 					m_pManager->SendNotify(this, _T("startmoveitem"), DUILIB_LIST_MOVE_ITEM, (WPARAM)&m_aSelItems, 0);
			// 				}
			// 			}//Modify by www 2015.03.23; Look at the following!
			if (!m_bHasMoveItem)
			{
				m_bHasMoveItem = true;
				if (m_pManager)
				{
					m_pManager->SendNotify(this, _T("startmoveitem"), DUILIB_LIST_MOVE_ITEM, (WPARAM)&m_aSelItems, 0);
				}
			}
			// 			if (m_pMoveItem != NULL)
			// 			{
			// 				m_pMoveItem->MoveWindowFollowPoint(event.ptMouse);
			// 				return;
			// 			}
		}
		if (!m_bLButtonDown)
			return;

		m_pEndPoint = event.ptMouse;
		if (m_pStartPoint.y == 0)
		{
			m_pStartPoint = m_pEndPoint;
			m_StartPos = GetScrollPos();
			m_pTempPoint = m_pStartPoint;
			return;
		}
		if (m_pEndPoint.y < m_rcItem.bottom && m_pEndPoint.y > m_rcItem.top)
		{
			SelectbyDrag(true);
			Invalidate();
			return;
		}
		if (((m_pEndPoint.y - m_pTempPoint.y) > 5) && (m_pEndPoint.y > m_rcItem.bottom))   //向下
		{
			if (IsFocused() && IsVisible())
			{
				//判断是否需要向下走一行
				int nIndex = m_nEndSelItem - m_nTopIndex;
				if (IsHiddenItemInBottom() && nIndex >= m_nShowItems - m_nShowItemsInColumn * 2)
				{
					LineDown();
				}
				else if (nIndex >= m_nShowItems - m_nShowItemsInColumn)
				{
					LineDown();
				}

				//选择
			/*	if (m_nEndSelItem <= m_nItems - m_nShowItemsInColumn)
				{
					m_nEndSelItem += m_nShowItemsInColumn;
				}*/
			}
			m_pTempPoint = event.ptMouse;
		}
		else if ((m_pTempPoint.y - m_pEndPoint.y) > 5 && (m_pEndPoint.y < m_rcItem.top))   //向上
		{
			if (IsFocused() && IsVisible())
			{
				int nIndex = m_nEndSelItem - m_nTopIndex;
				if (IsHiddenItemInTop() && nIndex < m_nShowItemsInColumn * 2)
				{
					LineUp();
				}
				else if (nIndex < m_nShowItemsInColumn)
				{
					LineUp();
				}
				if (m_nEndSelItem >= m_nShowItemsInColumn)
				{
					m_nEndSelItem -= m_nShowItemsInColumn;
				}
			}
			m_pTempPoint = event.ptMouse;
		}
		SelectbyDrag(true);
		Invalidate();
		return;
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
						if (IsHiddenItemInTop() && nIndex < m_nShowItemsInColumn * 2)
						{
							LineUp();
						}
						else if (nIndex < m_nShowItemsInColumn)
						{
							LineUp();
						}

						/*if (m_nFocusItem >= m_nShowItemsInColumn)
						{
						m_nFocusItem -= m_nShowItemsInColumn;
						}
						m_nEndSelItem = m_nFocusItem;
						SetItemSelect(m_nStartSelItem, m_nEndSelItem, true);*/
					}

				}
				else if ((!m_bSingleSel) && (GetKeyState(VK_CONTROL) < 0))
				{
				}
				else
				{
					int nIndex = GetMinSelItemIndex() - m_nTopIndex;
					if (IsHiddenItemInTop() && nIndex < m_nShowItemsInColumn * 2)  //上面有隐藏时，第二行数据
					{
						LineUp();
					}
					else if (nIndex < m_nShowItemsInColumn)             //第一行
					{
						nIndex += m_nShowItemsInColumn;    //第一行，相对nIndex不改变
						LineUp();
					}

					/*nIndex = nIndex - m_nShowItemsInColumn;
					if (nIndex > 0)
					{
					SelectItem(nIndex, true);
					}
					else
					{
					SelectItem(0, true);
					}*/
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
					if (IsHiddenItemInBottom() && nIndex >= m_nShowItems - m_nShowItemsInColumn * 2)
					{
						// 					if (m_nTopIndex + m_nShowItems + NUM_LEFT + 1<= m_nItems)
						// 					{
						// 						nIndex = nIndex - 1;
						// 					}
						LineDown();
					}
					else if (nIndex >= m_nShowItems - m_nShowItemsInColumn)
					{
						LineDown();
					}

					/*if (IsFocused() && IsVisible())
					{
					if (m_nFocusItem <= m_nItems - m_nShowItemsInColumn)
					{
					m_nFocusItem += m_nShowItemsInColumn;
					}
					m_nEndSelItem = m_nFocusItem;
					SetItemSelect(m_nStartSelItem, m_nEndSelItem, true);
					}*/
				}
				else if ((!m_bSingleSel) && ((event.wKeyState & MK_CONTROL) == MK_CONTROL))
				{
				}
				else
				{
					int nIndex = GetMaxSelItemIndex() - m_nTopIndex;
					if (m_nTopIndex + nIndex >= m_nItems - m_nShowItemsInColumn)   //最后一行，不需要再变动
					{
						return;
					}

					if (IsHiddenItemInBottom() && nIndex >= m_nShowItems - m_nShowItemsInColumn * 2)
					{
						// 					if (m_nTopIndex + m_nShowItems <= m_nItems )
						// 					{
						// 						nIndex = nIndex - m_nShowItemsInColumn;
						// 					}
						LineDown();
					}
					else if (nIndex >= m_nShowItems - m_nShowItemsInColumn)   //最后一行
					{
						nIndex -= m_nShowItemsInColumn;   //相对位置不变
						LineDown();
					}
					/*nIndex = nIndex + m_nShowItemsInColumn;
					if ((nIndex + 1) > m_pList->GetCount())
					{
					SelectItem(GetCount() - 1, true);
					}
					else
					{
					SelectItem(nIndex, true);
					}*/
				}
				Invalidate();
			}
			return;
		case VK_LEFT:
		{
			if (!IsEnabled())
			{
				return;
			}
			if ((!m_bSingleSel) && (GetKeyState(VK_SHIFT) < 0))  //多选
			{
				/*int nIndex = m_nEndSelItem - m_nTopIndex;
				if (nIndex%m_nShowItemsInColumn > 0)
				{
					m_nFocusItem--;
					m_nEndSelItem = m_nFocusItem;
					SetItemSelect(m_nStartSelItem, m_nEndSelItem, true);
				}*/
			}
			else if ((!m_bSingleSel) && (GetKeyState(VK_CONTROL) < 0))
			{
			}
			else
			{
				//int nIndex = GetMaxSelItemIndex() - m_nTopIndex;
				//if (nIndex%m_nShowItemsInColumn > 0)
				//{
				//	nIndex--;
				//	// 					m_nEndSelItem = nIndex;
				//	SelectItem(nIndex);
				//}
			}

		}
		return;
		case VK_RIGHT:
		{
			if (!IsEnabled())
			{
				return;
			}
			if ((!m_bSingleSel) && (GetKeyState(VK_SHIFT) < 0))  //多选
			{
				/*int nIndex = m_nEndSelItem - m_nTopIndex;
				if (nIndex%m_nShowItemsInColumn < m_nShowItemsInColumn - 1)
				{
					m_nFocusItem++;
					m_nEndSelItem = m_nFocusItem;
					SetItemSelect(m_nStartSelItem, m_nEndSelItem, true);
				}*/
			}
			else if ((!m_bSingleSel) && (GetKeyState(VK_CONTROL) < 0))
			{
			}
			else
			{
				//int nIndex = GetMaxSelItemIndex() - m_nTopIndex;
				//if (nIndex%m_nShowItemsInColumn < m_nShowItemsInColumn - 1)
				//{
				//	nIndex++;
				//	//					m_nEndSelItem = nIndex;
				//	SelectItem(nIndex);
				//}
			}
		}
		return;
		case VK_PRIOR:
		{
			if (!IsEnabled())
			{
				return;
			}
			int nIndex = GetMinSelItemIndex() - m_nTopIndex;
			//			PageUp();
			if (m_pList != NULL)
			{
				RECT rc = m_pList->GetPos();
				int nLines = (rc.bottom - rc.top) / (m_nItemHeight + m_pList->GetChildPadding());
				for (int i = 0; i < nLines; i++)
				{
					LineUp();
				}
				/*if (nIndex > 0)
				{
				SelectItem(nIndex, true);
				}
				else
				{
				SelectItem(0, true);
				}*/
			}
		}
		return;
		case VK_NEXT:
		{
			if (!IsEnabled())
			{
				return;
			}
			int nIndex = GetMaxSelItemIndex() - m_nTopIndex;
			//			PageDown();
			if (m_pList != NULL)
			{
				RECT rc = m_pList->GetPos();
				int nLines = (rc.bottom - rc.top) / (m_nItemHeight + m_pList->GetChildPadding());
				for (int i = 0; i< nLines; i++)
				{
					LineDown();
				}
				/*if ((nIndex + 1) > m_pList->GetCount())
				{
				SelectItem(GetCount() - 1, true);
				}
				else
				{
				SelectItem(nIndex, true);
				}*/
			}
		}
		return;
		case VK_HOME:
		{
			if (!IsEnabled())
			{
				return;
			}
			HomeUp();
			/*if (m_pList->GetCount() > 0)
				SelectItem(0, true);*/
		}
		return;
		case VK_END:
		{
			if (!IsEnabled())
			{
				return;
			}
			EndDown();
			/*if (m_pList->GetCount() > 0)
				SelectItem(m_pList->GetCount() - 1, true);*/
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
			return;
		}
		switch (LOWORD(event.wParam))
		{
		case SB_LINEUP:
			if (m_bScrollSelect && m_bSingleSel)
				SelectItem(FindSelectable((int)m_aSelItems.GetAt(0) - m_nTopIndex - 1, false));
			else
			{
				LineUp();
			}
			return;
		case SB_LINEDOWN:
			if (m_bScrollSelect && m_bSingleSel)
				SelectItem(FindSelectable((int)m_aSelItems.GetAt(0) - m_nTopIndex + 1, true));
			else
			{
				LineDown();
			}
			return;
		}
	}
	break;
	}
	CVerticalLayoutUI::DoEvent(event);
}

//bool CIConPhotoBackupListUI::SetItemSelect(int nStart, int nEnd, bool bTakeFocus /*= false*/)
//{
//	if (m_bSingleSel)
//	{
//		return false;
//	}
//
//	int nStartTemp = nStart;
//	int nEndTemp = nEnd;
//	if (nStart > nEnd)
//	{
//		int nTemp = nStartTemp;
//		nStartTemp = nEndTemp;
//		nEndTemp = nTemp;
//	}
//
//	//判断数据有效性
//	if (nStartTemp < 0)
//	{
//		return false;
//	}
//	if (nEndTemp >= m_nItems)
//	{
//		return false;
//	}
//
//
//	UnSelectAllItems();
//
//	//记录所有选中项
//	for (int iIndex = nStartTemp; iIndex <= nEndTemp; iIndex++)
//	{
//		if (m_bStyleAddItem && (iIndex == m_nItems - 1))
//		{
//			continue;
//		}
//		m_aSelItems.Add((LPVOID)iIndex);
//		m_bSelTopIndex = (iIndex == 0) ? true : m_bSelTopIndex;
//	}
//
//	//焦点跟随
//	if (bTakeFocus)
//	{
//		m_nFocusItem = nEnd;
//		// 		CControlUI* pControl = GetItemAt(nEnd - m_nTopIndex);
//		// 		if( pControl == NULL ) return false;
//		// 		if( !pControl->IsVisible() ) return false;
//		// 		if( !pControl->IsEnabled() ) return false;
//		this->SetFocus();
//	}
//
//	//更新当前选中显示项状态
//	if (nStart > nEnd)
//	{
//		int npos = nEnd - m_nTopIndex;  //计算开始位置
//		if (npos < 0)
//		{
//			npos = 0;
//		}
//
//		int nStep = nStart - m_nTopIndex;   //计算结束位置
//		if (nStep > m_nShowItems)
//		{
//			nStep = m_nShowItems;
//		}
//
//		for (int i = npos; i <= nStep; i++)
//		{
//			if (m_bStyleAddItem && (m_nTopIndex + i == m_nItems - 1))
//			{
//				continue;
//			}
//			CControlUI* pControl = GetItemAt(i);
//			if (pControl == NULL) continue;
//			if (!pControl->IsVisible()) continue;
//			if (!pControl->IsEnabled()) continue;
//
//
//			IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
//			if (pListItem == NULL)
//				continue;
//			if (!pListItem->Select(true, false))
//			{
//				int nFind = m_aSelItems.Find((LPVOID)i);
//				if (nFind < 0)
//				{
//					m_aSelItems.Remove(nFind);
//				}
//				continue;
//			}
//		}
//	}
//	else
//	{
//		int nStep = nEnd - m_nTopIndex;   //计算步长
//		if (nStep > m_nShowItems)
//		{
//			nStep = m_nShowItems;
//		}
//
//		int nPosStart = nStart - m_nTopIndex;
//		if (nPosStart < 0)    //已经走过了nStart项
//		{
//			nPosStart = 0;
//		}
//
//		for (int i = nPosStart; i <= nStep; i++)
//		{
//			if (m_bStyleAddItem && (m_nTopIndex + i == m_nItems - 1))
//			{
//				continue;
//			}
//			CControlUI* pControl = GetItemAt(i);
//			if (pControl == NULL) continue;
//			if (!pControl->IsVisible()) continue;
//			if (!pControl->IsEnabled()) continue;
//
//
//			IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
//			if (pListItem == NULL)
//				continue;
//			if (!pListItem->Select(true, false))
//			{
//				int nFind = m_aSelItems.Find((LPVOID)i);
//				if (nFind < 0)
//				{
//					m_aSelItems.Remove(nFind);
//				}
//				continue;
//			}
//		}
//	}
//
//	Invalidate();
//
//	if (m_pManager != NULL) {
//		m_pManager->SendNotify(this, _T("itemselect"), DUILIB_LIST_ITEMSELECT, (WPARAM)m_aSelItems.GetAt(0),(LPARAM)m_aSelItems.GetSize());
//	}
//
//	return true;
//}

void CIConPhotoBackupListUI::AddSelItemIndex(int Index)
{
	int nFind = m_aSelItems.Find((LPVOID)Index);
	if (nFind < 0)
	{
		m_aSelItems.Add((LPVOID)Index);
	}
}

void CIConPhotoBackupListUI::ReMoveSelItemIndex(int Index)
{
	int nFind = m_aSelItems.Find((LPVOID)Index);
	if (nFind  >= 0)
	{
		m_aSelItems.Remove(nFind);
	}
}




/////////////////////////////////////////////////////////////////////////////////////
//
CGridLayoutUI::CGridLayoutUI()
{

}

CGridLayoutUI::~CGridLayoutUI()
{

}

void CGridLayoutUI::SetPos(RECT rc)
{
	CControlUI::SetPos(rc);
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

	// Position the elements
	int cyNeeded = 0;
// 	int cxWidth = (rc.right - rc.left) / m_nColumns;
// 	if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) 
// 		cxWidth = (rc.right - rc.left + m_pHorizontalScrollBar->GetScrollRange() ) / m_nColumns;
	int cxWidth = GetItemAt(0)->GetFixedWidth() + m_iChildPadding;

// 	TCHAR tchar[MAX_PATH] = {0};
// 	::wsprintf(tchar, _T("the current width is %d\r\n"), cxWidth);
// 	OutputDebugStr(tchar);

	int cyHeight = 0;
	int iCount = 0;
	POINT ptTile = { rc.left, rc.top };
	if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
		ptTile.y -= m_pVerticalScrollBar->GetScrollPos();
	}
 	int iPosX = rc.left;
// 	if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) {
// 		iPosX -= m_pHorizontalScrollBar->GetScrollPos();
// 		ptTile.x = iPosX;
// 	}
	for( int it1 = 0; it1 < m_items.GetSize(); it1++ ) {
		CControlUI* pControl = static_cast<CControlUI*>(m_items[it1]);
		if( !pControl->IsVisible() ) continue;
		if( pControl->IsFloat() ) {
			SetFloatPos(it1);
			continue;
		}

		// Determine size
		RECT rcTile = { ptTile.x, ptTile.y, ptTile.x + cxWidth, ptTile.y };
		if( (iCount % m_nColumns) == 0 )
		{
			int iIndex = iCount;
			for( int it2 = it1; it2 < m_items.GetSize(); it2++ ) {
				CControlUI* pLineControl = static_cast<CControlUI*>(m_items[it2]);
				if( !pLineControl->IsVisible() ) continue;
				if( pLineControl->IsFloat() ) continue;

				RECT rcPadding = pLineControl->GetPadding();
				SIZE szAvailable = { rcTile.right - rcTile.left - rcPadding.left - rcPadding.right, 9999 };
				if( iIndex == iCount || (iIndex + 1) % m_nColumns == 0 ) {
					szAvailable.cx -= m_iChildPadding / 2;
				}
				else {
					szAvailable.cx -= m_iChildPadding;
				}

				if( szAvailable.cx < pControl->GetMinWidth() ) szAvailable.cx = pControl->GetMinWidth();
				if( szAvailable.cx > pControl->GetMaxWidth() ) szAvailable.cx = pControl->GetMaxWidth();

				SIZE szTile = pLineControl->EstimateSize(szAvailable);
				if( szTile.cx < pControl->GetMinWidth() ) szTile.cx = pControl->GetMinWidth();
				if( szTile.cx > pControl->GetMaxWidth() ) szTile.cx = pControl->GetMaxWidth();
				if( szTile.cy < pControl->GetMinHeight() ) szTile.cy = pControl->GetMinHeight();
				if( szTile.cy > pControl->GetMaxHeight() ) szTile.cy = pControl->GetMaxHeight();

				cyHeight = MAX(cyHeight, szTile.cy + rcPadding.top + rcPadding.bottom);
				if( (++iIndex % m_nColumns) == 0) break;
			}
		}

		RECT rcPadding = pControl->GetPadding();

		rcTile.left += rcPadding.left + m_iChildPadding / 2;
		rcTile.right -= rcPadding.right + m_iChildPadding / 2;
		if( (iCount % m_nColumns) == 0 ) {
			rcTile.left -= m_iChildPadding / 2;
		}

		if( ( (iCount + 1) % m_nColumns) == 0 ) {
			rcTile.right += m_iChildPadding / 2;
		}

		// Set position
		rcTile.top = ptTile.y + rcPadding.top;
		rcTile.bottom = ptTile.y + cyHeight;

		SIZE szAvailable = { rcTile.right - rcTile.left, rcTile.bottom - rcTile.top };
		SIZE szTile = pControl->EstimateSize(szAvailable);
		if( szTile.cx == 0 ) szTile.cx = szAvailable.cx;
		if( szTile.cy == 0 ) szTile.cy = szAvailable.cy;
		if( szTile.cx < pControl->GetMinWidth() ) szTile.cx = pControl->GetMinWidth();
		if( szTile.cx > pControl->GetMaxWidth() ) szTile.cx = pControl->GetMaxWidth();
		if( szTile.cy < pControl->GetMinHeight() ) szTile.cy = pControl->GetMinHeight();
		if( szTile.cy > pControl->GetMaxHeight() ) szTile.cy = pControl->GetMaxHeight();
		RECT rcPos = {(rcTile.left + rcTile.right - szTile.cx) / 2, (rcTile.top + rcTile.bottom - szTile.cy) / 2,
			(rcTile.left + rcTile.right - szTile.cx) / 2 + szTile.cx, (rcTile.top + rcTile.bottom - szTile.cy) / 2 + szTile.cy};
		pControl->SetPos(rcPos);

		if( (++iCount % m_nColumns) == 0 ) {
			ptTile.x = iPosX;
			ptTile.y += cyHeight + m_iChildPadding;
			cyHeight = 0;
		}
		else {
			cxWidth = MIN(cxWidth, (rcPos.right - rcPos.left) + m_iChildPadding);
			ptTile.x += cxWidth;
		}
		cyNeeded = rcTile.bottom - rc.top;
		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) cyNeeded = m_pVerticalScrollBar->GetScrollPos();
	}

	// Process the scrollbar
	ProcessScrollBar(rc, 0, cyNeeded);
}


/////////////////////////////////////////////////////////////////////////////////////
//
//


CIConListBodyUIEx::CIConListBodyUIEx(CIConListUIEx* pOwner) : m_pOwner(pOwner)
{
	ASSERT(m_pOwner);
	SetInset(CRect(0,0,0,0));
	m_nLeavePos = 0;
	m_nPosOffset = 0;
	m_bUp = false;
	m_bPosChange = false;
}

LPCTSTR CIConListBodyUIEx::GetClass()
{
	if (m_pOwner)
	{
		return m_pOwner->GetClass();
	}
	return NULL;
}

void CIConListBodyUIEx::SetVerticalScrollPos(int nPos)
{
	if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() )
	{
		m_pVerticalScrollBar->SetScrollPos(nPos);
	}
}



int CIConListBodyUIEx::GetVerticalFixWidth()
{
	if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() )
		return m_pVerticalScrollBar->GetFixedWidth();
	return 0;
}

void CIConListBodyUIEx::setUp(bool up)
{
	m_bUp = up;
}

void CIConListBodyUIEx::LineUp()
{
	m_bUp = true;
	m_bPosChange = false;
	if (!IsHiddenItemInTop())   //在上面没有没显示全的项
	{
		bool bRefreshData = true;
		if (m_pOwner)
		{
			bRefreshData = m_pOwner->ScrollUpLine();
		}
		int cyLine = 8;
		if( m_pManager ) 
			cyLine = m_pManager->GetDefaultFontInfo()->tm.tmHeight + 8;
		if (m_pOwner)
			cyLine = m_pOwner->GetItemHeight() + GetChildPadding();

		SIZE sz = GetScrollPos();
		sz.cy -= cyLine;
		if (sz.cy < 0)
		{
			sz.cy = 0;
		}
		SetScrollLinePos(sz, bRefreshData);
	}
	else    ////在上面有没显示全的项,先滚动让其显示全
	{
		int cyLine = 8;
		if( m_pManager ) 
			cyLine = m_pManager->GetDefaultFontInfo()->tm.tmHeight + 8;
		if (m_pOwner)
			cyLine = m_pOwner->GetItemHeight() + GetChildPadding();
		
		RECT rc = GetItemAt(0)->GetPos();
		int nNeed = m_rcItem.top - rc.top;

		SIZE sz = GetScrollPos();
		sz.cy -= nNeed;
		if (sz.cy < 0)
		{
			sz.cy = 0;
		}

// 		sz.cy += nNeed + m_iChildPadding;
		SetScrollLinePos(sz);
	}

}

//在底部是否有隐藏的项
bool CIConListBodyUIEx::IsHiddenItemInBottom()
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
	if( !pControl->IsVisible() )
	{
		int i = 0;
	}
	if( pControl->IsFloat() ) 
		return false;
	RECT rect;
	rect = pControl->GetPos();
	if (rect.bottom > m_rcItem.bottom)
	{
		return true;
	}
	return false;
}

//在头部是否有隐藏的项
bool CIConListBodyUIEx::IsHiddenItemInTop()
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
	if( !pControl->IsVisible() )
	{
		int i = 0;
	}
	if( pControl->IsFloat() ) 
		return false;
	RECT rect;
	rect = pControl->GetPos();
	if (rect.top < m_rcItem.top)
	{
		return true;
	}
	return false;

}

//设置为显示全的项隐藏的像素大小
void CIConListBodyUIEx::SetLeavePos(int nLeavePos)
{
	if ((nLeavePos >= 0) && (nLeavePos <= m_rcItem.bottom - m_rcItem.top))
	{
		m_nLeavePos = nLeavePos;
	}
	else
		m_nLeavePos = 0;
}

void CIConListBodyUIEx::PageUp()
{

	int iOffset = m_rcItem.bottom - m_rcItem.top - m_rcInset.top - m_rcInset.bottom;
	// 	if (m_pOwner)
	// 	{
	// 		iOffset = m_pOwner->GetItemHeight() *m_pOwner->GetShowItems();
	// 	}

	SIZE sz = GetScrollPos();
	if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) 
		iOffset -= m_pHorizontalScrollBar->GetFixedHeight();
	sz.cy -= iOffset;
	SetScrollPos(sz);
}

void CIConListBodyUIEx::PageDown()
{
	SIZE sz = GetScrollPos();
	int iOffset = m_rcItem.bottom - m_rcItem.top - m_rcInset.top - m_rcInset.bottom;
	// 	if (m_pOwner)
	// 	{
	// 		iOffset = m_pOwner->GetItemHeight() *m_pOwner->GetShowItems();
	// 	}
	if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() )
		iOffset -= m_pHorizontalScrollBar->GetFixedHeight();
	sz.cy += iOffset;
	SetScrollPos(sz);
}

void CIConListBodyUIEx::HomeUp()
{
	SIZE sz = GetScrollPos();
	sz.cy = 0;
	SetScrollPos(sz);
}

void CIConListBodyUIEx::EndDown()
{
	SIZE sz = GetScrollPos();
	sz.cy = GetScrollRange().cy;
	SetScrollPos(sz);
}

void CIConListBodyUIEx::LineDown()
{
	m_bUp = false;
	m_bPosChange = false;
	if (!IsHiddenItemInBottom())
	{
		bool bRefreshData = true;
		if (m_pOwner)
		{
			bRefreshData = m_pOwner->ScrollDownLine();
		}

		if (!bRefreshData && m_pOwner && m_pOwner->IsListDataChanged())
		{
			m_pOwner->RefreshListData(0, true);      // 重新刷新数据，并将top置为0 
			return;
		}


		int cyLine = 8;
		if( m_pManager )
			cyLine = m_pManager->GetDefaultFontInfo()->tm.tmHeight + 8;

		if (m_pOwner)
			cyLine = m_pOwner->GetItemHeight() + GetChildPadding();

		SIZE sz = GetScrollPos();
		sz.cy += cyLine;

		SetScrollLinePos(sz, bRefreshData);
	}
	else
	{
		int cyLine = 8;
		if( m_pManager )
			cyLine = m_pManager->GetDefaultFontInfo()->tm.tmHeight + 8;

		if (m_pOwner)
			cyLine = m_pOwner->GetItemHeight() + GetChildPadding();

		SIZE sz = GetScrollPos();
//		sz.cy += cyLine;

		RECT rc = GetItemAt(GetCount() - 1)->GetPos();
		int nNeed = rc.bottom - m_rcItem.bottom;
		m_nLeavePos = nNeed;
		sz.cy += nNeed;

		SetScrollLinePos(sz);
	}

}

void CIConListBodyUIEx::SetScrollLinePos(SIZE szPos, bool bScrollItem /* = true */)
{
	int cx = 0;
	int cy = 0;
	if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
		int iLastScrollPos = m_pVerticalScrollBar->GetScrollPos();
		m_pVerticalScrollBar->SetScrollPos(szPos.cy);
		cy = m_pVerticalScrollBar->GetScrollPos() - iLastScrollPos;
	}

	if( /*cx == 0 &&*/ cy == 0) return;
	
	if (!bScrollItem)
	{
		Invalidate();
		return;
	}
// 	//计算位置
	RECT rcPos;
	for( int it2 = 0; it2 < m_items.GetSize(); it2++ ) {
		CControlUI* pControl = static_cast<CControlUI*>(m_items[it2]);
		if (pControl == NULL)
		{
			continue;
		}
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

}

void CIConListBodyUIEx::FreshDataBySize(SIZE szPos)     //位置变化时，刷新界面数据
{
	m_bPosChange = true;
	int cx = 0;
	int cy = 0;
	int m_nRange = 0;
	if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
		int iLastScrollPos = m_pVerticalScrollBar->GetScrollPos();
		m_pVerticalScrollBar->SetScrollPos(szPos.cy);
//		cy = m_pVerticalScrollBar->GetScrollPos() - iLastScrollPos;
		m_nRange = m_pVerticalScrollBar->GetScrollRange();
	}

	if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) {
		int iLastScrollPos = m_pHorizontalScrollBar->GetScrollPos();
		m_pHorizontalScrollBar->SetScrollPos(szPos.cx);
//		cx = m_pHorizontalScrollBar->GetScrollPos() - iLastScrollPos;
	}
	
	//纵向计算滚动
	bool bEnd = false;
	int nPos = szPos.cy;
	if (szPos.cy < 0)
	{
		nPos = 0;
	}
// 	if( szPos.cy > m_nRange )
// 	{
// 		nPos = m_nRange;
// 	}
	if (m_pOwner)
	{
		int nItemHeight = m_pOwner->GetItemHeight();
		int nStep = nPos/(nItemHeight + m_iChildPadding);
		cy = nPos%(nItemHeight + m_iChildPadding);
		if (cy > nItemHeight)
		{
			nStep ++;
			cy -= nItemHeight;
		}
//		OutputDebugStr(_T("CIConListBodyUIEx::FreshDataBySize RefreshListDatabyLine \n"));
		m_pOwner->RefreshListDatabyLine(nStep);
// 		SetPos(m_rcItem);
		if (m_nRange - nPos > nItemHeight)
		{
			cy = 0;
		}
	}
	if (cy > m_nLeavePos)
	{
		cy = m_nLeavePos;
	}
	m_nPosOffset = cy;

 	if( cx == 0 && cy == 0)
	{
		Invalidate();
		return;
	}
	//计算位置
	//	cy = 0;
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
}


void CIConListBodyUIEx::SetScrollPos(SIZE szPos)
{
	m_bPosChange = false;
	int cx = 0;
	int cy = 0;
	int m_nRange = 0;
	if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
		int iLastScrollPos = m_pVerticalScrollBar->GetScrollPos();
		m_pVerticalScrollBar->SetScrollPos(szPos.cy);
		cy = m_pVerticalScrollBar->GetScrollPos() - iLastScrollPos;
		if (cy < 0)
		{
			m_bUp = true;
		}
		else m_bUp = false;
		m_nRange = m_pVerticalScrollBar->GetScrollRange();
	}

	if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) {
		int iLastScrollPos = m_pHorizontalScrollBar->GetScrollPos();
		m_pHorizontalScrollBar->SetScrollPos(szPos.cx);
		cx = m_pHorizontalScrollBar->GetScrollPos() - iLastScrollPos;
	}

	if( cx == 0 && cy == 0) return;
	//纵向计算滚动
	bool bEnd = false;
	int nPos = szPos.cy;
	if (szPos.cy < 0)
	{
		nPos = 0;
	}
	if( szPos.cy > m_nRange )
	{
		nPos = m_nRange;
	}


// 	TCHAR wstr[256] = {0};
// 	::wsprintf(wstr, _T("The pos is %d, cy=%d\n"), nPos, cy);
// 	OutputDebugStr(wstr);

	if (m_pOwner)
	{
		int nItemHeight = m_pOwner->GetItemHeight();
		int nStep = nPos/(nItemHeight + m_iChildPadding);
		int nLeft = nPos%(nItemHeight + m_iChildPadding);
		if (nLeft > nItemHeight)
		{
			nStep ++;
		}
		m_pOwner->RefreshListDatabyLine(nStep);
	}

	//计算位置
//	cy = 0;
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
}

void CIConListBodyUIEx::SetPos(RECT rc)
{
        if( GetCount() > 0 ) {
            int iDeskWidth = GetItemAt(0)->GetFixedWidth() + m_iChildPadding;
            int column = (rc.right - rc.left) / iDeskWidth ;
            if( column < 1 ) column = 1;
			if( column <= 0 ) return;
			m_nColumns = column;
        }

//         CGridLayoutUI::SetPos(rc);
		CControlUI::SetPos(rc);
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

		// Position the elements
		int cyNeeded = 0;
		int cxWidth = GetItemAt(0)->GetFixedWidth() + m_iChildPadding;

		int cyHeight = 0;
		int iCount = 0;
		POINT ptTile = { rc.left, rc.top };
		if (m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible())
		{
			int nPosy = m_pVerticalScrollBar->GetScrollPos();
			int nRange = m_pVerticalScrollBar->GetScrollRange();
			if ((m_nLeavePos > 0) && ((nPosy > 0) && (nPosy <= nRange)) && !m_bUp)
			{
				ptTile.y -= m_nLeavePos;
			}
			else if ((m_bPosChange > 0) && ((nPosy > 0) && (nPosy <= nRange)) && m_bPosChange)
			{
				ptTile.y -= m_nPosOffset;
			}
		}
		int iPosX = rc.left;

		for( int it1 = 0; it1 < m_items.GetSize(); it1++ ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it1]);
			if( !pControl->IsVisible() ) continue;
			if( pControl->IsFloat() ) {
				SetFloatPos(it1);
				continue;
			}
			
			RECT rcOldPos = pControl->GetPos();
			// Determine size
			RECT rcTile = { ptTile.x, ptTile.y, ptTile.x + cxWidth, ptTile.y };
			if( (iCount % m_nColumns) == 0 )
			{
				int iIndex = iCount;
				for( int it2 = it1; it2 < m_items.GetSize(); it2++ ) {
					CControlUI* pLineControl = static_cast<CControlUI*>(m_items[it2]);
					if( !pLineControl->IsVisible() ) continue;
					if( pLineControl->IsFloat() ) continue;

					RECT rcPadding = pLineControl->GetPadding();
					SIZE szAvailable = { rcTile.right - rcTile.left - rcPadding.left - rcPadding.right, 9999 };
					if( iIndex == iCount || (iIndex + 1) % m_nColumns == 0 ) {
						szAvailable.cx -= m_iChildPadding / 2;
					}
					else {
						szAvailable.cx -= m_iChildPadding;
					}

					if( szAvailable.cx < pControl->GetMinWidth() ) szAvailable.cx = pControl->GetMinWidth();
					if( szAvailable.cx > pControl->GetMaxWidth() ) szAvailable.cx = pControl->GetMaxWidth();

					SIZE szTile = pLineControl->EstimateSize(szAvailable);
					if( szTile.cx < pControl->GetMinWidth() ) szTile.cx = pControl->GetMinWidth();
					if( szTile.cx > pControl->GetMaxWidth() ) szTile.cx = pControl->GetMaxWidth();
					if( szTile.cy < pControl->GetMinHeight() ) szTile.cy = pControl->GetMinHeight();
					if( szTile.cy > pControl->GetMaxHeight() ) szTile.cy = pControl->GetMaxHeight();

					cyHeight = MAX(cyHeight, szTile.cy + rcPadding.top + rcPadding.bottom);
					if( (++iIndex % m_nColumns) == 0) break;
				}
			}

			RECT rcPadding = pControl->GetPadding();

			rcTile.left += rcPadding.left /*+ m_iChildPadding / 2*/;
			rcTile.right -= rcPadding.right/* + m_iChildPadding / 2*/;
// 			if( (iCount % m_nColumns) == 0 ) {
// 				rcTile.left -= m_iChildPadding / 2;
// 			}
// 
// 			if( ( (iCount + 1) % m_nColumns) == 0 ) {
// 				rcTile.right += m_iChildPadding / 2;
// 			}

			// Set position
			rcTile.top = ptTile.y + rcPadding.top;
			rcTile.bottom = ptTile.y + cyHeight;

			SIZE szAvailable = { rcTile.right - rcTile.left, rcTile.bottom - rcTile.top };
			SIZE szTile = pControl->EstimateSize(szAvailable);
			if( szTile.cx == 0 ) szTile.cx = szAvailable.cx;
			if( szTile.cy == 0 ) szTile.cy = szAvailable.cy;
			if( szTile.cx < pControl->GetMinWidth() ) szTile.cx = pControl->GetMinWidth();
			if( szTile.cx > pControl->GetMaxWidth() ) szTile.cx = pControl->GetMaxWidth();
			if( szTile.cy < pControl->GetMinHeight() ) szTile.cy = pControl->GetMinHeight();
			if( szTile.cy > pControl->GetMaxHeight() ) szTile.cy = pControl->GetMaxHeight();
			RECT rcPos = {(rcTile.left + rcTile.right - szTile.cx) / 2, (rcTile.top + rcTile.bottom - szTile.cy) / 2,
				(rcTile.left + rcTile.right - szTile.cx) / 2 + szTile.cx, (rcTile.top + rcTile.bottom - szTile.cy) / 2 + szTile.cy};
// 			if (rcOldPos.top != 0)
// 			{
// 				rcPos.top -= m_rcItem.top - rcOldPos.top;
// 				rcPos.bottom -=m_rcItem.top - rcOldPos.top;
// 				ptTile.y -= m_rcItem.top - rcOldPos.top;;
// 			}
			rcPos.left += m_iChildPadding / 2;
			rcPos.right += m_iChildPadding / 2;
			rcPos.top += m_iChildPadding;
			rcPos.bottom += m_iChildPadding;
			pControl->SetPos(rcPos);

			if( (++iCount % m_nColumns) == 0 ) {
				ptTile.x = iPosX;
				ptTile.y += cyHeight + m_iChildPadding;
				cyHeight = 0;
			}
			else {
				cxWidth = MIN(cxWidth, (rcPos.right - rcPos.left) + m_iChildPadding);
				ptTile.x += cxWidth;
			}
			cyNeeded = rcTile.bottom - rc.top;
		}
		
		if (cyNeeded < 0)
		{
			cyNeeded = 0;
		}
		int cyCurPos = 0;
		// Process the scrollbar
		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) 
			int cyCurPos = m_pVerticalScrollBar->GetScrollPos();
		if(cyNeeded != cyCurPos)
		{
			ProcessScrollBar(rc, 0, cyNeeded);
		}
}

void CIConListBodyUIEx::DoEvent(TEventUI& event)
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

CIConListContainerElementUIEx::CIConListContainerElementUIEx() : 
m_iIndex(-1),
m_pOwner(NULL), 
m_bSelected(false),
m_uButtonState(0)
{
	m_bNeedSelect = false;
	m_bShade = false;
	m_dwShadeColor = 0x77535353;
	m_bCanSel = true;
	m_bvisibleEdit = false;
}

LPCTSTR CIConListContainerElementUIEx::GetClass() const
{
	if (m_pOwner)
	{
		return m_pOwner->GetClass();
	}
	return _T("ListContainerElementUIEx");
}

UINT CIConListContainerElementUIEx::GetControlFlags() const
{
	return UIFLAG_WANTRETURN;
}

LPVOID CIConListContainerElementUIEx::GetInterface(LPCTSTR pstrName)
{
	if( _tcscmp(pstrName, _T("ListItem")) == 0 ) return static_cast<IListItemUI*>(this);
	if( _tcscmp(pstrName, _T("IconListContainerElementEx")) == 0 ) return static_cast<CIConListContainerElementUIEx*>(this);
	return CContainerUI::GetInterface(pstrName);
}

CIConListUIEx* CIConListContainerElementUIEx::GetOwner()
{
	return m_pOwner;
}

void CIConListContainerElementUIEx::SetOwner(CControlUI* pOwner)
{
	m_pOwner = static_cast<CIConListUIEx*>(pOwner->GetInterface(_T("IconListEx")));
}

void CIConListContainerElementUIEx::SetVisible(bool bVisible)
{
	CContainerUI::SetVisible(bVisible);
	if( !IsVisible() && m_bSelected)
	{
		m_bSelected = false;
		if( m_pOwner != NULL ) 
			m_pOwner->SelectItem(-1);
	}
}

void CIConListContainerElementUIEx::SetEnabled(bool bEnable)
{
	CControlUI::SetEnabled(bEnable);
	if( !IsEnabled() ) {
		m_uButtonState = 0;
	}
}

int CIConListContainerElementUIEx::GetIndex() const
{
	return m_iIndex;
}

void CIConListContainerElementUIEx::SetIndex(int iIndex)
{
	m_iIndex = iIndex;
}

void CIConListContainerElementUIEx::Invalidate()
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

bool CIConListContainerElementUIEx::Activate()
{
	if( !CContainerUI::Activate() ) return false;
	if( m_pManager != NULL ) m_pManager->SendNotify(this, _T("itemactivate"), DUILIB_LIST_ITEMACTIVE, m_iIndex);
	return true;
}

bool CIConListContainerElementUIEx::IsSelected() const
{
	return m_bSelected;
}

bool CIConListContainerElementUIEx::Select(bool bSelect /* = true */, bool bInvalidate /* = true */)
{
	if( !IsEnabled() ) return false;

	if (m_bCanSel)
	{
		if (bSelect == m_bSelected) return true;
		m_bSelected = bSelect;
		// 	if( bSelect && m_pOwner != NULL ) 
		// 		m_pOwner->SelectItem(m_iIndex);
		if (bInvalidate)
		{
			Invalidate();
		}
	}
	else
	{
		m_bSelected = false;
		return false;
	}
	return true;
}

bool CIConListContainerElementUIEx::IsExpanded() const
{
	return false;
}

bool CIConListContainerElementUIEx::Expand(bool /*bExpand = true*/)
{
	return false;
}

void CIConListContainerElementUIEx::DoEvent(TEventUI& event)
{
	if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
		if( m_pOwner != NULL ) m_pOwner->DoEvent(event);
		else CContainerUI::DoEvent(event);
		return;
	}

	if( event.Type == UIEVENT_DBLCLICK )
	{	
		if (m_pOwner && !m_pOwner->IsEnabled())
		{
			return;
		}
		if( IsEnabled() ) {
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
	if( event.Type == UIEVENT_KEYDOWN && IsEnabled() )
	{
		if (m_pOwner && !m_pOwner->IsEnabled())
		{
			return;
		}
		if( event.chKey == VK_RETURN ) {
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
	if( event.Type == UIEVENT_BUTTONDOWN /*|| event.Type == UIEVENT_RBUTTONDOWN */)
	{
		if (m_pOwner && !m_pOwner->IsEnabled())
		{
			return;
		}
		if( ::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled() ) 
		{
			m_pManager->SendNotify(this, _T("itemclick"), DUILIB_LIST_ITEMCLICK);
			if (m_pOwner == NULL)
			{
				return;
			}
			POINT pt;
			pt.x = 0;
			pt.y = 0;
			m_pOwner->SetStartPoint(pt);
			if (m_pOwner->GetSingleSelect())
			{
				if (!IsSelected())
					m_pOwner->SelectItem(m_iIndex);
			}				
			else
			{
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
					m_pOwner->SetEndItem(m_iIndex, true);
				}
				else
				{
					if ((m_pOwner->GetSelectItemCount() == 1 && IsSelected()))
					{								
					}
					else if (!IsSelected())
					{
						m_pOwner->SelectItem(m_iIndex);
					}
					else
					{
						m_bNeedSelect = true;
					}
				}
// 				m_pOwner->SetMCaptured(m_pOwner);
			}
			m_pOwner->SetLButtonState(true);
			Invalidate();
		}
		return;
	}
	if( event.Type == UIEVENT_BUTTONUP ) 
	{
		if (m_pOwner && !m_pOwner->IsEnabled())
		{
			return;
		}
		if (m_pOwner == NULL)
		{
			return;
		}
		
		if (m_bNeedSelect && ::PtInRect(&m_rcItem, event.ptMouse) && (!m_pOwner->GetLButtonState()))
		{
			m_pOwner->SelectItem(m_iIndex);
		}
		else if (m_bNeedSelect && ::PtInRect(&m_rcItem, event.ptMouse) && (m_pOwner->GetSelectItemCount() > 1))
		{
			m_pOwner->SelectItem(m_iIndex);
		}
		m_bNeedSelect = false;
		if (IsEnabled() && m_pOwner)
		{
			m_pOwner->SetLButtonState(false);
		}
		return;
	}
	if( event.Type == UIEVENT_MOUSEENTER )
	{
		if (m_pOwner && !m_pOwner->IsEnabled())
		{
			return;
		}
		if (m_pOwner && m_pOwner->GetLButtonState())
		{
			m_pOwner->SelectMoveTemp(m_iIndex);
			return;
		}
		if (!IsLButtonDown() || (!IsFocused() && !m_pOwner->IsFocused()))
		{
			m_pOwner->SetLButtonState(false);
		}
		if( IsEnabled() ) {
			m_uButtonState |= UISTATE_HOT;
			Invalidate();
		}
		return;
	}
	if( event.Type == UIEVENT_MOUSELEAVE )
	{
		if (::PtInRect(&m_rcItem, event.ptMouse))    // 如果鼠标在当前项目中，离开子控件消息，不处理
		{
			return;
		}
	
		if (m_pOwner && !m_pOwner->IsEnabled())
		{
			return;
		}
		if (m_pOwner && m_pOwner->GetLButtonState())
		{
			m_pOwner->UnSelectMoveTemp();
			return;
		}

		if( (m_uButtonState & UISTATE_HOT) != 0 ) {
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

void CIConListContainerElementUIEx::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if( _tcscmp(pstrName, _T("selected")) == 0 ) Select();
	else if (_tcscmp(pstrName, _T("shadecolor")) == 0)
	{
		if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
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
	else if (_tcscmp(pstrName, _T("canselect")) == 0)
	{
		if (_tcscmp(pstrValue, _T("true")) == 0)
		{
			SetCanSel(true);
		}
		else
			SetCanSel(false);
	}
	else if (_tcscmp(pstrName, _T("normaliamge")) == 0)
	{
		SetNormalImage(pstrValue);
	}
	else if (_tcscmp(pstrName, _T("hotimage")) == 0)
	{
		SetHotImage(pstrValue);
	}
	else if (_tcscmp(pstrName, _T("selectimage")) == 0)
	{
		SetSelectImage(pstrValue);
	}
	else CContainerUI::SetAttribute(pstrName, pstrValue);
}

void CIConListContainerElementUIEx::PaintSelect(HDC hDC, const RECT&rcItem)
{
	ASSERT(m_pOwner);
	if( m_pOwner == NULL ) return;
	TListInfoUI* pInfo = m_pOwner->GetListInfo();
	DWORD iBackColor = 0;
	if( IsSelected() ) {
		iBackColor = pInfo->dwSelectedBkColor;
	}
	if ( iBackColor != 0 ) {
		CRenderEngine::DrawColor(hDC, m_pManager, rcItem, GetAdjustColor(iBackColor));
	}

	if( IsSelected() ) {
		if( !pInfo->sSelectedImage.IsEmpty() ) {
			if( !DrawImage(hDC, (LPCTSTR)pInfo->sSelectedImage, rcItem) )
				pInfo->sSelectedImage.Empty();
			else return;
		}
	}
	CContainerUI::DoPaint(hDC, rcItem);
}

void CIConListContainerElementUIEx::SetShade(bool bShade /* = false */)
{
	m_bShade = bShade;
}

bool CIConListContainerElementUIEx::GetShade() const
{
	return m_bShade;
}


void CIConListContainerElementUIEx::SetShadeColor(DWORD dwShadeColor)
{
	if (dwShadeColor != 0)
	{
		m_dwShadeColor = dwShadeColor;
	}
}

DWORD CIConListContainerElementUIEx::GetShadeColor() const
{
	return m_dwShadeColor;
}

void CIConListContainerElementUIEx::SetShadeImage(CStdString strImage)
{
	m_strShadeImage = strImage;
}

LPCTSTR CIConListContainerElementUIEx::GetShadeImage() const
{
	return m_strShadeImage;
}


void CIConListContainerElementUIEx::SetCanSel(bool bCanSel/* = true*/)
{
	m_bCanSel = bCanSel;
}

bool CIConListContainerElementUIEx::GetCanSel() const
{
	return m_bCanSel;
}

void CIConListContainerElementUIEx::SetNormalImage(CStdString strImage)
{
	m_strNormalImage = strImage;
}

LPCTSTR CIConListContainerElementUIEx::GetNormalImage() const
{
	return m_strNormalImage;
}

void CIConListContainerElementUIEx::SetHotImage(CStdString strImage)
{
	m_strHotImage = strImage;
}

LPCTSTR CIConListContainerElementUIEx::GetHotImage() const
{
	return m_strHotImage;
}

void CIConListContainerElementUIEx::SetSelectImage(CStdString strImage)
{
	m_strSelectImage = strImage;
}

LPCTSTR CIConListContainerElementUIEx::GetSelectImage() const
{
	return m_strSelectImage;
}

void CIConListContainerElementUIEx::DoPaint(HDC hDC, const RECT& rcPaint)
{
	if( !::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem) ) return;
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
			CRenderEngine::DrawColor(hDC, m_pManager, m_rcItem, m_dwShadeColor);
	}
}

void CIConListContainerElementUIEx::DrawItemText(HDC hDC, const RECT& rcItem)
{
	return;
}

void CIConListContainerElementUIEx::DrawItemBk(HDC hDC, const RECT& rcItem)
{
	ASSERT(m_pOwner);
	if( m_pOwner == NULL ) return;
	TListInfoUI* pInfo = m_pOwner->GetListInfo();
	DWORD iBackColor = 0;
	/*if( m_iIndex % 2 == 0 )*/ iBackColor = pInfo->dwBkColor;

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
			if( !DrawImage(hDC, (LPCTSTR)pInfo->sDisabledImage) ) 
				pInfo->sDisabledImage.Empty();
			else return;
		}
	}
	if( IsSelected() ) {
		if (!m_strSelectImage.IsEmpty())
		{
			if (!DrawImage(hDC, (LPCTSTR)m_strSelectImage))
				m_strSelectImage.Empty();
			else goto DRAWLINE;
		}
		if (!pInfo->sSelectedImage.IsEmpty()) {
			if (!DrawImage(hDC, (LPCTSTR)pInfo->sSelectedImage))
				pInfo->sSelectedImage.Empty();
			else goto DRAWLINE;
		}
	}
	if( (m_uButtonState & UISTATE_HOT) != 0 ) {
		if (!m_strHotImage.IsEmpty())
		{
			if (!DrawImage(hDC, (LPCTSTR)m_strHotImage))
				m_strHotImage.Empty();
			else goto DRAWLINE;
		}
		if( !pInfo->sHotImage.IsEmpty() ) {
			if( !DrawImage(hDC, (LPCTSTR)pInfo->sHotImage) ) pInfo->sHotImage.Empty();
			else goto DRAWLINE;
		}
		
	}
	if( !m_sBkImage.IsEmpty() ) {
		if( m_iIndex % 2 == 0 ) {
			if( !DrawImage(hDC, (LPCTSTR)m_sBkImage))
				m_sBkImage.Empty();
		}
	}

	if( m_sBkImage.IsEmpty() ) {
		if( !pInfo->sBkImage.IsEmpty() ) {
			if( !DrawImage(hDC, (LPCTSTR)pInfo->sBkImage) )
				pInfo->sBkImage.Empty();
			else goto DRAWLINE;
		}
	}
	if (!m_strNormalImage.IsEmpty())
	{
		if (!DrawImage(hDC, (LPCTSTR)m_strNormalImage))
			m_strNormalImage.Empty();
		else goto DRAWLINE;
	}
DRAWLINE:
	if (pInfo->dwLineColor != 0) {
		RECT rcLine = { m_rcItem.left, m_rcItem.bottom - 1, m_rcItem.right, m_rcItem.bottom - 1 };
		CRenderEngine::DrawLine(hDC, rcLine, 1, GetAdjustColor(pInfo->dwLineColor));
	}
}

void CIConListContainerElementUIEx::SetbVisibleEdit(bool bVisible)
{
	if (m_bvisibleEdit == bVisible) return;
	m_bvisibleEdit = bVisible;
	CStdString sName;
	if (bVisible)
	{
		CListElementEx * pListElement = static_cast<CListElementEx*>(m_items[1]);
		if (pListElement)
		{
			pListElement->SetVisible(false);
			sName = pListElement->GetText();
		}
		pListElement = static_cast<CListElementEx*>(m_items[2]);
		if (pListElement)
		{
			pListElement->SetVisible(true);
			CEditUIEx *pEdit = (CEditUIEx*)pListElement->GetItemAt(4);
			if(pEdit)
			{
				int nPos = sName.ReverseFind('.');
				if (nPos != -1)
				{
					sName = sName.Left(nPos);
				}
				pEdit->SetText(sName);
			}	
		}
	}
	else
	{
		CListElementEx * pListElement = static_cast<CListElementEx*>(m_items[2]);
		if (pListElement)
		{
			pListElement->SetVisible(false);
			CEditUIEx *pEdit = (CEditUIEx*)pListElement->GetItemAt(4);
		}
		 pListElement = static_cast<CListElementEx*>(m_items[1]);
		if (pListElement)
		{
			pListElement->SetVisible(true);
		}
		
	}
}

void CIConListContainerElementUIEx::SetbVisibleEdit(bool bVisible, bool bIsFolder)
{
	if (m_bvisibleEdit == bVisible) return;
	m_bvisibleEdit = bVisible;
	CStdString sName;
	if (bVisible)
	{
		CListElementEx * pListElement = static_cast<CListElementEx*>(m_items[1]);
		if (pListElement)
		{
			pListElement->SetVisible(false);
			sName = pListElement->GetText();
		}
		pListElement = static_cast<CListElementEx*>(m_items[2]);
		if (pListElement)
		{
			pListElement->SetVisible(true);
			CEditUIEx *pEdit = (CEditUIEx*)pListElement->GetItemAt(4);
			if (pEdit)
			{
				if (!bIsFolder)
				{
					int nPos = sName.ReverseFind('.');
					if (nPos != -1)
					{
						sName = sName.Left(nPos);
					}
				}
				pEdit->SetText(sName);
			}
		}
	}
	else
	{
		CListElementEx * pListElement = static_cast<CListElementEx*>(m_items[2]);
		if (pListElement)
		{
			pListElement->SetVisible(false);
			CEditUIEx *pEdit = (CEditUIEx*)pListElement->GetItemAt(4);
		}
		pListElement = static_cast<CListElementEx*>(m_items[1]);
		if (pListElement)
		{
			pListElement->SetVisible(true);
		}

	}
}



CIConPhotoBackupListContainerElementUI::CIConPhotoBackupListContainerElementUI()
{

}

LPCTSTR CIConPhotoBackupListContainerElementUI::GetClass() const
{
	if (m_pOwner)
	{
		return m_pOwner->GetClass();
	}
	return _T("IConPhotoBackupListContainerElementUI");
}

LPVOID CIConPhotoBackupListContainerElementUI::GetInterface(LPCTSTR pstrName)
{
	if (_tcscmp(pstrName, _T("ListItem")) == 0) return static_cast<IListItemUI*>(this);
	if (_tcscmp(pstrName, _T("IConPhotoBackupListContainerElement")) == 0) return static_cast<CIConPhotoBackupListContainerElementUI*>(this);
	return CContainerUI::GetInterface(pstrName);
}

void CIConPhotoBackupListContainerElementUI::DoEvent(TEventUI& event)
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
				/*if (!IsSelected())
					m_pOwner->SelectMultiItem(m_iIndex);
				else
				{
					m_pOwner->UnSelectItem(m_iIndex);
				}*/
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
				m_pOwner->SelectMultiItem(m_iIndex);
			/*else
			{
			m_pOwner->UnSelectItem(m_iIndex);
			}*/
		}
		Invalidate();
		return;
	}
	if (event.Type == UIEVENT_BUTTONDOWN /*|| event.Type == UIEVENT_RBUTTONDOWN */)
	{
		if (m_pOwner && !m_pOwner->IsEnabled())
		{
			return;
		}
		if (::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled())
		{
			m_pManager->SendNotify(this, _T("itemclick"), DUILIB_LIST_ITEMCLICK);
			if (m_pOwner == NULL)
			{
				return;
			}
			POINT pt;
			pt.x = 0;
			pt.y = 0;
			m_pOwner->SetStartPoint(pt);
			if (m_pOwner->GetSingleSelect())
			{
				if (!IsSelected())
					m_pOwner->SelectItem(m_iIndex);
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
						m_pOwner->UnSelectItem(m_iIndex);
					}
					else if (!IsSelected())
					{
						m_pOwner->SelectMultiItem(m_iIndex);
					}
					else
					{
						m_bNeedSelect = true;
					}
				}
				// 				m_pOwner->SetMCaptured(m_pOwner);
			}
			m_pOwner->SetLButtonState(true);
			Invalidate();
		}
		return;
	}
	if (event.Type == UIEVENT_BUTTONUP)
	{
		if (m_pOwner && !m_pOwner->IsEnabled())
		{
			return;
		}
		if (m_pOwner == NULL)
		{
			return;
		}

		if (m_bNeedSelect && ::PtInRect(&m_rcItem, event.ptMouse) && (!m_pOwner->GetLButtonState()))
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
		else if (m_bNeedSelect && ::PtInRect(&m_rcItem, event.ptMouse) && (m_pOwner->GetSelectItemCount() > 1))
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
		m_bNeedSelect = false;
		if (IsEnabled() && m_pOwner)
		{
			m_pOwner->SetLButtonState(false);
		}
		return;
	}
	if (event.Type == UIEVENT_MOUSEENTER)
	{
		if (m_pOwner && !m_pOwner->IsEnabled())
		{
			return;
		}
		if (m_pOwner && m_pOwner->GetLButtonState())
		{
			m_pOwner->SelectMoveTemp(m_iIndex);
			return;
		}
		if (!IsLButtonDown() || (!IsFocused() && !m_pOwner->IsFocused()))
		{
			m_pOwner->SetLButtonState(false);
		}
		if (IsEnabled()) {
			m_uButtonState |= UISTATE_HOT;
			Invalidate();
		}
		return;
	}
	if (event.Type == UIEVENT_MOUSELEAVE)
	{
		if (::PtInRect(&m_rcItem, event.ptMouse))    // 如果鼠标在当前项目中，离开子控件消息，不处理
		{
			return;
		}

		if (m_pOwner && !m_pOwner->IsEnabled())
		{
			return;
		}
		if (m_pOwner && m_pOwner->GetLButtonState())
		{
			m_pOwner->UnSelectMoveTemp();
			return;
		}

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

} // namespace DuiLib
