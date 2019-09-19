#include "stdafx.h"
#include "ListUIEx.h"
#include "Winbase.h"

namespace DuiLib {

CListUIEx::CListUIEx() : m_pCallback(NULL), m_bScrollSelect(false), /*m_iCurSel(-1),*/ m_iExpandedItem(-1)
{
	m_pList = new CListBodyUIEx(this);
	ASSERT(m_pList);
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

	//add by ligthen
	m_nTopIndex = 0;
	m_nItemHeight = 20;
	m_nItems = 0;
	m_nShowItems = 0;
	m_dequeueReusableItem.Empty();
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

	m_pMoveItem = NULL;
	m_bMoveItemEffect = true;
	m_bNeedRefresh = false;
	m_uMoveSelectIndex = -1;
	m_bHasMoveItem = false;

	m_uTextStyle = DT_VCENTER;
	m_dwTextColor = 1;
	m_dwDisabledTextColor = 1;
	m_iFont = -1;
	m_bShowHtml = false;
	::ZeroMemory(&m_rcTextPadding, sizeof(m_rcTextPadding));
	m_bPaintEmptyImage = false;
	m_bSelTopIndex = false;
	m_nEditIndex = -1;
	m_ntoHeaderHeight = 0;
	m_bStyleAddItem = false;
	m_bWindowsStyle = false;
	m_nLeftWidth = 0;
}

CListUIEx::~CListUIEx(void)
{
	m_dequeueReusableItem.Empty();

	if (m_pMoveItem != NULL)   //释放移动窗口
	{
		m_pMoveItem->DestroyWindow();
		delete m_pMoveItem;
		m_pMoveItem = NULL;
	}
}

//关闭移动窗口
void CListUIEx::CloseMoveWindow()
{
	if (m_pMoveItem != NULL)   //释放移动窗口
	{
		m_pMoveItem->DestroyWindow();
		delete m_pMoveItem;
		m_pMoveItem = NULL;
	}
}

//设置是否需要移动效果
void CListUIEx::SetMoveItemEffect(bool bMoveItem /* = true */)
{
	m_bMoveItemEffect = bMoveItem;
}

//返回移动效果在值
bool CListUIEx::IsMoveItemEffect() const
{
	return m_bMoveItemEffect;
}

int CListUIEx::FindIndexbyPoint(POINT &pt)   //通过指定的点找到对应的索引
{
	int nSelIndex = -1;
	for (int i = 0; i< GetCount(); i++)
	{
		CListContainerElementUIEx *pcontrol = static_cast<CListContainerElementUIEx *>(GetItemAt(i));
		if (pcontrol == NULL)
		{
			continue;
		}
		RECT rc = pcontrol->GetPos();
		if (::PtInRect(&rc, pt))
		{
// 			SelectMoveTemp(i);
			nSelIndex = i;
			break;
		}
	}

	return nSelIndex;
}


int CListUIEx::MoveToPoint(POINT &pt)     //移动到指定的点
{
	if (m_pMoveItem != NULL)
	{
		m_pMoveItem->MoveWindowFollowPoint(pt);
	}
	return 0;
}

//设置是否需要刷新数据
bool CListUIEx::NeedFreshData(bool bFRresh /* = true */)
{
	bool bold = m_bNeedRefresh;
	m_bNeedRefresh = bFRresh;
	return bold;
}

LPCTSTR CListUIEx::GetClass() const
{
	return _T("ListUIEx");
}

void CListUIEx::SetItemHeight(int nItemHeigh)
{

 	if (nItemHeigh < m_pManager->GetDefaultFontInfo()->tm.tmHeight + 8)
 	{
		m_nItemHeight =  m_pManager->GetDefaultFontInfo()->tm.tmHeight + 8;
 	}
	else
		m_nItemHeight = nItemHeigh - 1;

	CalShowItemNum();
}

//获取项高度
int  CListUIEx::GetItemHeight() const           
{
	return m_nItemHeight;
}

void CListUIEx::SetItemCount(int nNums)
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
int  CListUIEx::GetItemCount() const
{
	return m_nItems;
}

//获取顶部索引
int  CListUIEx::GetTopIndex() const
{
	return m_nTopIndex;
}

//获取可以显示的Item数
int  CListUIEx::GetShowItems() const
{
	return m_nShowItems;
}

int  CListUIEx::GetListDataSize()               // 获取数据源当前大小
{
	if (m_pListDataSource)
	{
		return m_pListDataSource->numberOfRows();
	}
	else
		return 0;
}

// 添加选中项目，不刷新
void CListUIEx::AddSelItem(int nIndex)
{
	m_aSelItems.Add((LPVOID)(nIndex));
}

// 清空选中项目，不刷新 
void CListUIEx::EmptySelItem()
{
	m_aSelItems.Empty();
}


//获取可复用的项
CListContainerElementUIEx *CListUIEx::GetReuseableItem()
{
	CListContainerElementUIEx *pListItem = NULL;
	if (m_dequeueReusableItem.GetSize() > 0)
	{
		pListItem = static_cast<CListContainerElementUIEx *>(m_dequeueReusableItem.GetAt(0));
		m_dequeueReusableItem.Remove(0);
	}

	return pListItem;
}

//获取指定索引的项
CControlUI* CListUIEx::GetItemExAt(int iIndex) const
{
	if (m_pListDataSource == NULL)
		return NULL;
	return static_cast<CControlUI *>(m_pListDataSource->listItemForIndex(iIndex));
}


//获取总项数 
int CListUIEx::GetAllCount() const
{
	return m_nItems;
}

// DWORD WINAPI FuncThread_ScrollDown(LPVOID lpThreadParameter)
// {
// 	CListUIEx *plist = (CListUIEx *)lpThreadParameter;
// 	if (plist == NULL)
// 	{
// 		return 0L;
// 	}
// 
// 	return plist->ScrollDownLine();
// }

//查询是否有数据隐藏在前面
bool CListUIEx::IsHiddenItemInTop()   
{
	return m_pList->IsHiddenItemInTop();
}

//在底部是否有隐藏的项
bool CListUIEx::IsHiddenItemInBottom()
{
	return m_pList->IsHiddenItemInBottom();
}

bool CListUIEx::ScrollDownLine()
{
	m_bNeedCheckListData = false;
	if (m_pListDataSource == NULL)
	{
		return false;
	}
	if (m_nTopIndex + m_nShowItems + NUM_LEFT + 1> m_nItems)
	{
		if (IsHiddenItemInBottom())
		{
//			m_nTopIndex ++;
			return true;
		}
		return false;
	}
//  	TCHAR wstr[256] = {0};
//  	::wsprintf(wstr, _T("CListUIEx::ScrollDownLine The TopIndex is %d\n"), m_nTopIndex);
//  	OutputDebugStr(wstr);
	CListContainerElementUIEx *pListElement = static_cast<CListContainerElementUIEx *>(m_pListDataSource->listItemForIndex(m_nTopIndex + m_nShowItems + NUM_LEFT));
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
// 	CControlUI *pListItem = GetItemAt(0);
// 	if (pListItem != NULL)
// 	{
//		m_dequeueReusableItem.Add(pListItem);
//	}

	RemoveAt(0);
	m_nTopIndex++;
	
	return true;
}

//往上走一行
UINT CListUIEx::ScrollUpLine()
{
	if (m_pListDataSource == NULL)
	{
		return 0;
	}

	if (m_nTopIndex <= 0)   //少于预留
	{
		return 0;
	}

	if (m_pList && IsHiddenItemInTop())   //有数据还在前面，先显示数据
	{
//		m_nTopIndex--;
		return m_pList->GetHiddenItemInTop();
// 		return true;
	}
// 	TCHAR wstr[256] = {0};
// 	::wsprintf(wstr, _T("CListUIEx::ScrollUpLine The TopIndex is %d\n"), m_nTopIndex);
// 	OutputDebugStr(wstr);

	m_nTopIndex--;
	CListContainerElementUIEx *pListElement = static_cast<CListContainerElementUIEx *>(m_pListDataSource->listItemForIndex(m_nTopIndex));
	if (pListElement == NULL)
	{
		return 0;
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
// 	CControlUI *pListItem = GetItemAt(GetCount() - 1);
// 	if (pListItem != NULL)
// 	{
// 		m_dequeueReusableItem.Add(pListItem);
// 	}
	RemoveAt(GetCount() - 1);
// 	m_dequeueReusableItem.Add(m_pListDataSource->listItemForIndex(m_nTopIndex + m_nShowItems));
	return 0;
}

//根据指定索引刷新pos和数据
bool CListUIEx::RefreshPos(int nTopIndex, bool bInit /* = true */)
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
 	SIZE sz = {0, 0};
 	sz.cy = (m_nTopIndex * m_nItemHeight) + 1;
 	SetVerticalScrollPos(sz.cy);

// 	CStdString str;
// 	str.Format(_T("CListUIEx::InsetIndexData Index=%d, m_nTopIndex=%d, m_nItems=%d, m_nShowItems=%d, itemcount=%d vpos=%d\n"), nTopIndex, m_nTopIndex, m_nItems, m_nShowItems, GetCount(), sz.cy);
// 	OutputDebugStr(str);

	return true;
}

//删除指定的数据
bool CListUIEx::DeleteIndexData(int Index)        
{
	m_bFreshed = false;
	m_aSelItems.Empty();
	m_bSelTopIndex = false;
	RefreshPos(m_nTopIndex);
	return true;

}

bool CListUIEx::DeleteDataAt(int nIndex)          //删除指定的数据(不刷新全部数据）
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


	m_nItems --;
// 	CStdString str;
// 	str.Format(_T("CListUIEx::DeleteDataAt  Index=%d, m_nTopIndex=%d, m_nItems=%d, m_nShowItems=%d, itemcount=%d\n"), nIndex, m_nTopIndex, m_nItems, m_nShowItems, GetCount());
// 	OutputDebugStr(str);

	SIZE szPos = m_pList->GetScrollPos();
	if (m_nItems > m_nShowItems)   //如果显示不全
	{
		m_pList->SetAllNeedHeight(m_nItems * m_nItemHeight + 1 + m_ntoHeaderHeight);
	}
	else
		m_pList->SetAllNeedHeight(0);

	if (nIndex < m_nTopIndex)   //删除显示项目前面的项
	{
		if (m_nTopIndex > 0)
		{
			m_nTopIndex --;
		}
	}
	else if (nIndex == m_nTopIndex)
	{
// 		if (m_nTopIndex > 0)
// 		{
// 			m_nTopIndex --;
// 		}
		RemoveAt(0);
		if ((m_nTopIndex + m_nShowItems - 1) < m_nItems)
		{
			CListContainerElementUIEx *pListItem = static_cast<CListContainerElementUIEx *>(m_pListDataSource->listItemForIndex(m_nTopIndex + m_nShowItems - 1));
			if (pListItem == NULL)
			{
				return false;
			}
			Add(pListItem);

// 			if (m_pListDataSource)
// 			{
// 				CListContainerElementUIEx *pListItem = static_cast<CListContainerElementUIEx *>(m_pListDataSource->listItemForIndex(m_nTopIndex + m_nShowItems -1));
// 				if (pListItem == NULL)
// 				{
// 					return false;
// 				}
// 				Add(pListItem);
// 			}
		}
		else if((m_nTopIndex + m_nShowItems - 1) == m_nItems)
		{
			if (m_nTopIndex > 0)
			{
				m_nTopIndex --;
				CListContainerElementUIEx *pListItem = static_cast<CListContainerElementUIEx *>(m_pListDataSource->listItemForIndex(m_nTopIndex));
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
					CListContainerElementUIEx *pListItem = static_cast<CListContainerElementUIEx *>(m_pListDataSource->listItemForIndex(m_nTopIndex + m_nShowItems -1));
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
						m_nTopIndex --;
// 					}
// 					if (m_pListDataSource)
// 					{
						CListContainerElementUIEx *pListItem = static_cast<CListContainerElementUIEx *>(m_pListDataSource->listItemForIndex(m_nTopIndex));
						if (pListItem == NULL)
						{
							return false;
						}
						AddAt(pListItem, 0);
					}
				}
			}
// 			else
// 			{
// 				m_pList->SetPos(m_pList->GetPos());
// 			}
		}
	}

	m_pList->SetPos(m_pList->GetPos());
	m_pList->SetVerticalScrollPos((m_nTopIndex * m_nItemHeight) + 1 + m_ntoHeaderHeight);
	m_aSelItems.Empty();
	m_bSelTopIndex = false;
	Invalidate();
	return true;
}

bool CListUIEx::InsetIndexData(int Index)         //在制定索引之前插入数据
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

// 	CStdString str;
// 	str.Format(_T("CListUIEx::InsetIndexData Index=%d, m_nTopIndex=%d, m_nItems=%d, m_nShowItems=%d, itemcount=%d\n"), Index, m_nTopIndex, m_nItems, m_nShowItems, GetCount());
// 	OutputDebugStr(str);

	if (Index < m_nTopIndex)
	{
		m_nTopIndex ++;
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
	return true;

}

bool CListUIEx::InsetDataAt(int nIndex)
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

	m_nItems ++;
//  	CStdString str;
//  	str.Format(_T("CListUIEx::InsetDataAt Index=%d, m_nTopIndex=%d, m_nItems=%d, m_nShowItems=%d, itemcount=%d\n"), nIndex, m_nTopIndex, m_nItems, m_nShowItems, GetCount());
//  	OutputDebugStr(str);

	SIZE szPos = m_pList->GetScrollPos();
	if (m_nItems > m_nShowItems)   //如果显示不全
	{
		m_pList->SetAllNeedHeight((m_nItems * m_nItemHeight) + 1 + m_ntoHeaderHeight);
	}
	else
		m_pList->SetAllNeedHeight(0);

	if (nIndex < m_nTopIndex)   //插入到前面显示
	{
		m_nTopIndex ++;
//		m_pList->SetPos(m_pList->GetPos());
	}
	else if (nIndex == m_nTopIndex)
	{
		if (GetCount() < m_nShowItems)
		{
			if (m_pListDataSource)
			{
				CListContainerElementUIEx *pListItem = static_cast<CListContainerElementUIEx *>(m_pListDataSource->listItemForIndex(nIndex));
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
				CListContainerElementUIEx *pListItem = static_cast<CListContainerElementUIEx *>(m_pListDataSource->listItemForIndex(nIndex));
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
				CListContainerElementUIEx *pListItem = static_cast<CListContainerElementUIEx *>(m_pListDataSource->listItemForIndex(nIndex));
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
					CListContainerElementUIEx *pListItem = static_cast<CListContainerElementUIEx *>(m_pListDataSource->listItemForIndex(nIndex));
					if (pListItem == NULL)
					{
						return false;
					}
					AddAt(pListItem, nIndex - m_nTopIndex);
					RemoveAt(GetCount() - 1);
				}
			}
// 			else
// 			{
// 				m_pList->SetPos(m_pList->GetPos());
// 			}

		}
	}

	for (int i = 0; i< m_aSelItems.GetSize(); i++)
	{
		int nSel = (int)m_aSelItems.GetAt(i);
		if (nSel >= nIndex)
		{
			m_aSelItems.SetAt(i, (LPVOID)(nSel + 1));
		}
	}
	m_pList->SetPos(m_pList->GetPos());
	m_pList->SetVerticalScrollPos((m_nItemHeight * m_nTopIndex) + 1 + m_ntoHeaderHeight);
	Invalidate();
	return true;
}

// 刷新数据，保留选中项
bool CListUIEx::RefreshData(CStdPtrArray & deleteList, CStdPtrArray & InsertList)
{
	if (m_aSelItems.GetSize() == 0)    //没有选中项目
	{
		return RefreshListData(m_nTopIndex);
	}

	int nDeleteCount = deleteList.GetSize();
	if (nDeleteCount > 0)     // 有删除的项，先检测是否有选中项目被删除
	{
		for (int i = 0; i < nDeleteCount; i++)
		{
			int nIndex = (int)deleteList.GetAt(i);
			for (int j = 0; j < m_aSelItems.GetSize(); j++)
			{
				int nSelIndex = (int)m_aSelItems.GetAt(j);
				if (nSelIndex > nIndex)    // 删除当前选中项目前面的项目，当前选中项目索引-1
				{
					m_aSelItems.SetAt(j, (LPVOID)(nSelIndex - 1));
				}
				else if (nSelIndex == nIndex)       // 删除当前选中项
				{
					m_aSelItems.Remove(j);
					j--;         // 移除当前项后，叠加-1，保证数据都检测到
				}
			}
		}
	}

	int nInsertCount = InsertList.GetSize();
	for (int i = 0; i< nInsertCount; i++)
	{
		int nIndex = (int)InsertList.GetAt(i);
		int nSelCount = m_aSelItems.GetSize();
		for (int j = 0; j < nSelCount; j++)
		{
			int nSelIndex = (int)m_aSelItems.GetAt(j);
			if (nSelIndex >= nIndex)    // 插入当前选中项目前面，当前选中项目索引+1
			{
				m_aSelItems.SetAt(j, (LPVOID)(nSelIndex + 1));
			}
		}
	}

	return RefreshListData(m_nTopIndex);
}


//根据指定索引刷新数据
// 不刷新滚动条
bool CListUIEx::RefreshListData(int nTopIndex, bool bInit /* = true */)
{
// 	CStdString str;
// 	str.Format(_T("CListUIEx::RefreshListData index=%d\n"), nTopIndex);
// 	OutputDebugStr(str);
	m_bFreshed = true;
	if (m_pListDataSource == NULL)
	{
		return false;
	}

	if (bInit)
	{
		Add(m_pListDataSource->listHeader());
		SetItemCount(m_pListDataSource->numberOfRows());
		CalShowItemNum();
		if (m_nItems > m_nShowItems)   //如果显示不全
		{
			m_pList->SetAllNeedHeight((m_nItems * m_nItemHeight) + 1 + m_ntoHeaderHeight);
		}
		else
			m_pList->SetAllNeedHeight(0);
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

//	m_aSelItems.Empty();
	//清空数据
// 	for (int i = 0; i< m_nShowItems + NUM_LEFT; i++)
// 	{
// 		CControlUI *pListItem = GetItemAt(i);
// 		if (pListItem != NULL)
// 		{
// 			m_dequeueReusableItem.Add(pListItem);
// 		}
// 	}
	RemoveAll();

	//重新生成数据
	for (int i = 0; i< m_nShowItems + NUM_LEFT; i++)
	{
		if (i + m_nTopIndex >= m_nItems)
		{
			break;
		}
		CListContainerElementUIEx *pListItem = static_cast<CListContainerElementUIEx *>(m_pListDataSource->listItemForIndex(m_nTopIndex + i));
		if (pListItem == NULL)
		{
			return false;
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

//显示指定的项
void CListUIEx::EnsureVisible(int iIndex)
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
	int iOffset = (m_nTopIndex - nOldTop)*m_nItemHeight;
	if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() )
		iOffset -= m_pHorizontalScrollBar->GetFixedHeight();
	sz.cy += iOffset;
	SetScrollPos(sz);
	
}

//计算可以显示的项数
void CListUIEx::CalShowItemNum()
{
	if (m_nItemHeight == 0)
	{
		m_nShowItems = 0;
		return;
	}

	RECT rc = m_pList->GetPos();
	int nHeight = rc.bottom - rc.top;
	if (nHeight == 0)
	{
		m_bNeedRefresh = true;    //需要在计算pos的时候重新刷新
		m_nShowItems = 0; 
		return;
	}
	m_nShowItems = (nHeight - 1)/m_nItemHeight;
	if ((nHeight - 1) % m_nItemHeight != 0)  //不是刚刚好全部显示
	{
		m_nShowItems ++;
	}
}

// 设置数据源，不做任何界面操作
void CListUIEx::SetDataSource(IListDataSource *ListDataSource)
{
	m_pListDataSource = ListDataSource;
}

//设置数据来源并刷新
void CListUIEx::SetListDataSource(IListDataSource *ListDataSource, bool bChangeTopIndex /* = true */)
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
		RefreshListData(m_nTopIndex);
		SIZE sz = {0, 0};
		sz.cy = m_nTopIndex *m_nItemHeight + 1;
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
		nNumInset =  m_nShowItems;

	for (int i = 0; i< nNumInset + NUM_LEFT; i++)
	{
		if (i >= m_nItems)
		{
			break;
		}
		CListContainerElementUIEx *pListItem = static_cast<CListContainerElementUIEx *>(m_pListDataSource->listItemForIndex(i));
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


void CListUIEx::SetAllNeedHeight(int nHeight)
{
	if (m_pList && (nHeight >= 0))
	{
		m_pList->SetAllNeedHeight(nHeight);
	}
}

void CListUIEx::SetVerticalScrollPos(int nPos)
{
	if (m_pList && (nPos >= 0))
	{
		m_pList->SetVerticalScrollPos(nPos);
	}
}

bool CListUIEx::IsSelectTopIndex()
{
	return m_bSelTopIndex;
}

void CListUIEx::Init()
{
	if (m_pListDataSource != NULL)
	{
		Add(m_pListDataSource->listHeader());
		SetItemCount(m_pListDataSource->numberOfRows());
		CalShowItemNum();
		if (m_nItems > m_nShowItems)   //如果显示不全
		{
			m_pList->SetAllNeedHeight((m_nItems * m_nItemHeight) + 1 + m_ntoHeaderHeight );
		}
		else
			m_pList->SetAllNeedHeight(0);
	}
}

UINT CListUIEx::GetControlFlags() const
{
	return UIFLAG_TABSTOP;
}

LPVOID CListUIEx::GetInterface(LPCTSTR pstrName)
{
	if( _tcscmp(pstrName, _T("ListEx")) == 0 ) return static_cast<CListUIEx*>(this);
	if( _tcscmp(pstrName, _T("IList")) == 0 ) return static_cast<IListUI*>(this);
	if( _tcscmp(pstrName, _T("IListOwner")) == 0 ) return static_cast<IListOwnerUI*>(this);
	return CVerticalLayoutUI::GetInterface(pstrName);
}

CControlUI* CListUIEx::GetItemAt(int iIndex) const
{
	return m_pList->GetItemAt(iIndex);
}

int CListUIEx::GetItemIndex(CControlUI* pControl) const
{
	if( pControl->GetInterface(_T("ListHeader")) != NULL ) return CVerticalLayoutUI::GetItemIndex(pControl);
	// We also need to recognize header sub-items
	if( _tcsstr(pControl->GetClass(), _T("ListHeaderItemUI")) != NULL ) return m_pHeader->GetItemIndex(pControl);

	return m_pList->GetItemIndex(pControl);
}

bool CListUIEx::SetItemIndex(CControlUI* pControl, int iIndex)
{
	if( pControl->GetInterface(_T("ListHeader")) != NULL ) return CVerticalLayoutUI::SetItemIndex(pControl, iIndex);
	// We also need to recognize header sub-items
	if( _tcsstr(pControl->GetClass(), _T("ListHeaderItemUI")) != NULL ) return m_pHeader->SetItemIndex(pControl, iIndex);

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

int CListUIEx::GetCount() const
{
	return m_pList->GetCount();
}

bool CListUIEx::Add(CControlUI* pControl)
{
	// Override the Add() method so we can add items specifically to
	// the intended widgets. Headers are assumed to be
	// answer the correct interface so we can add multiple list headers.

	if (pControl == NULL)
	{
		return false;
	}

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

bool CListUIEx::AddAt(CControlUI* pControl, int iIndex)
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
/*	if( m_iCurSel >= iIndex ) m_iCurSel += 1;*/
	return true;
}

bool CListUIEx::Remove(CControlUI* pControl)
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

// 	if( iIndex == m_iCurSel && m_iCurSel >= 0 ) {
// 		int iSel = m_iCurSel;
// 		m_iCurSel = -1;
// 		SelectItem(FindSelectable(iSel, false));
// 	}
// 	else if( iIndex < m_iCurSel ) m_iCurSel -= 1;
	return true;
}

bool CListUIEx::RemoveAt(int iIndex)
{
	if (!m_pList->RemoveAt(iIndex)) return false;

	for(int i = iIndex; i < m_pList->GetCount(); ++i) {
		CControlUI* p = m_pList->GetItemAt(i);
		IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(_T("ListItem")));
		if( pListItem != NULL ) pListItem->SetIndex(i);
	}

// 	if( iIndex == m_iCurSel && m_iCurSel >= 0 ) {
// 		int iSel = m_iCurSel;
// 		m_iCurSel = -1;
// 		SelectItem(FindSelectable(iSel, false));
// 	}
// 	else if( iIndex < m_iCurSel ) m_iCurSel -= 1;
	return true;
}

void CListUIEx::RemoveAll()
{
/*	m_iCurSel = -1;*/
	m_iExpandedItem = -1;
	m_pList->RemoveAll();
}

void CListUIEx::SetPos(RECT rc)
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
// 	if (m_bNeedRefresh)
// 	{
		RefreshListData(m_nTopIndex);
//		m_bNeedRefresh = false;
		//重新计算滚动条的位置
		//if (m_pList)
		//{
		//	SIZE szPos = m_pList->GetScrollPos();
		//	szPos.cy = m_nTopIndex*m_nItemHeight;
		//	m_pList->SetScrollLinePos(szPos);
		//}

//	}

	int iOffset = m_pList->GetScrollPos().cx;
	for( int i = 0; i < m_ListInfo.nColumns; i++ ) {
		CControlUI* pControl = static_cast<CControlUI*>(m_pHeader->GetItemAt(i));
		if( !pControl->IsVisible() ) continue;
		if( pControl->IsFloat() ) continue;

		RECT rcPos = pControl->GetPos();
		if( iOffset > 0 ) {
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

void CListUIEx::DoEvent(TEventUI& event)
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
	if (event.Type == UIEVENT_DBLCLICK)
	{
		if (!IsEnabled())
		{
			return;
		}
		UnSelectAllItems(true);
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
		m_bLButtonDown = true;
		UnSelectAllItems(true);
		
		SetMCaptured(this);
		m_pStartPoint = event.ptMouse;
		RECT rc = m_pList->GetPos();
		int ndy = m_pStartPoint.y - rc.top - m_ntoHeaderHeight;
		if (ndy < 0)
		{
			return;
		}
		m_nStartSelItem = m_nTopIndex + (ndy - 1)/ m_nItemHeight;
		if (m_nStartSelItem < 0)
		{
			m_nStartSelItem = 0;
		}
		m_StartPos = GetScrollPos();
		m_pTempPoint = m_pStartPoint;
		m_pEndPoint = m_pStartPoint;
		m_bLButtonDown = true;
		if (m_pManager)
		{
			m_pManager->SendNotify(this, _T("itemclick"), DUILIB_LIST_ITEMCLICK, -1, 0, 0);
		}

	}
	if (event.Type == UIEVENT_BUTTONUP)
	{
		if (!IsEnabled())
		{
			return;
		}
		m_bLButtonDown = false;
		m_bLButtonDown_ClickInData = false;
		ReleaseMCaptured();
		Invalidate();
	}
	if (event.Type == UIEVENT_MOUSEMOVE)
	{
		if (!IsEnabled())
		{
			return;
		}
		if (m_bLButtonDown_ClickInData && m_bMoveItemEffect)
		{
// 			if (m_pMoveItem == NULL)
// 			{
// 				m_pMoveItem = new CMoveItemWnd();
// 				ASSERT(m_pMoveItem);
// 				m_pMoveItem->Init(this);
// 				m_pMoveItem->ShowWindow(false);
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
			return;
		}
		if (m_bLButtonDown_ClickInData)
		{
			return;
		}

		if (!m_bLButtonDown && !m_bLButtonDown_ClickInData)
			return;

		m_pEndPoint = event.ptMouse;
		RECT rcListboby = m_pList->GetPos();

		if ((m_pEndPoint.y < m_rcItem.bottom) && (m_pEndPoint.y > rcListboby.top))
		{
			//计算最后鼠标所在项，并点击
			RECT rc = m_pList->GetPos();
			int ndy = m_pEndPoint.y  - rc.top - m_ntoHeaderHeight;
			if (ndy < 0)
			{
				return;
			}
			int nItem = (ndy - 1)/m_nItemHeight;
			
// 			CStdString str1;
// 			str1.Format(_T("ndy is %d, nItem is %d\n"),ndy, m_rcItem.top);
// 			OutputDebugStr(str1);

			if ((ndy - 1) % m_nItemHeight != 0)
			{
				nItem ++;
			}
			SetEndItem(nItem - 1);   //从0开始统计
			Invalidate();
			return;
		}
		if (m_pStartPoint.y == 0)
		{
			m_pStartPoint = m_pEndPoint;
			m_StartPos = GetScrollPos();
			m_pTempPoint = m_pStartPoint;
			return;
		}
		if (((m_pEndPoint.y - m_pTempPoint.y) > 5) && (m_pEndPoint.y > m_rcItem.bottom))   //向下
		{
			if (IsFocused() && IsVisible())
			{
				//判断是否需要向下走一行
				int nIndex = m_nEndSelItem - m_nTopIndex;
				if (IsHiddenItemInBottom() && nIndex == m_nShowItems - 1 - 1)
				{
					if (m_nTopIndex + m_nShowItems + NUM_LEFT + 1<= m_nItems)
					{
						nIndex = nIndex - 1;
					}
					LineDown();
				}
				else if (nIndex == m_nShowItems - 1)
				{
					LineDown();
				}

				//选择
				if (m_nEndSelItem < m_nItems - 1)
				{
					m_nEndSelItem ++;
				}
				SetItemSelect(m_nStartSelItem, m_nEndSelItem, true);
			}
			m_pTempPoint = event.ptMouse;
		}
		else if ((m_pTempPoint.y -  m_pEndPoint.y) > 5 && (m_pEndPoint.y < rcListboby.top))   //向上
		{
			if (IsFocused() && IsVisible())
			{
				int nIndex = m_nEndSelItem - m_nTopIndex;
				if (IsHiddenItemInTop() && nIndex == 1)
				{
					LineUp();
				}
				else if (nIndex <= 0)
				{
					LineUp();
				}
				if (m_nEndSelItem > 0)
				{
					m_nEndSelItem --;
				}
				SetItemSelect(m_nStartSelItem, m_nEndSelItem, true);
			}
			m_pTempPoint = event.ptMouse;
		}
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
						m_nFocusItem --;
					}
					m_nEndSelItem = m_nFocusItem;
					SetItemSelect(m_nStartSelItem, m_nEndSelItem, true);
				}

			}
			else if ((!m_bSingleSel) &&(GetKeyState(VK_CONTROL) < 0))
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
					if (m_nTopIndex + m_nShowItems + NUM_LEFT + 1<= m_nItems)
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
						m_nFocusItem ++;
					}
					m_nEndSelItem = m_nFocusItem;
					SetItemSelect(m_nStartSelItem, m_nEndSelItem, true);
				}
			}
			else if ((!m_bSingleSel) &&((event.wKeyState & MK_CONTROL) == MK_CONTROL))
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
					if (m_nTopIndex + m_nShowItems + NUM_LEFT + 1<= m_nItems)
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
			if (m_pList->GetCount() > 0)
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
				break;
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

CListHeaderUI* CListUIEx::GetHeader() const
{
	return m_pHeader;
}

CContainerUI* CListUIEx::GetList() const
{
	return m_pList;
}

bool CListUIEx::GetScrollSelect()
{
	return m_bScrollSelect;
}

void CListUIEx::SetScrollSelect(bool bScrollSelect)
{
	m_bScrollSelect = bScrollSelect;
}

int CListUIEx::GetCurSel() const
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

int CListUIEx::GetMinSelItemIndex()
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

int CListUIEx::GetMaxSelItemIndex()
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

bool CListUIEx::IsListDataChanged()   // 判断当前数据是否修改
{
	if (m_bNeedCheckListData)
	{
		int nItem = m_nItems;
		if (m_bStyleAddItem)
		{
			nItem--;
		}
		if (m_pListDataSource)
		{
			if (nItem > m_pListDataSource->numberOfRows())   //当前总项目数大于数据源总数
			{
				return true;
			}
		}
	}
	return false;
}


bool CListUIEx::SelectItem(int iIndex, bool bTakeFocus /* = false */)
{
	if (iIndex < 0) return false;
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
		m_pManager->SendNotify(this, _T("itemselect"), DUILIB_LIST_ITEMSELECT, iIndex);
	}

	return true;
}

//拖动时，选择的项目
bool CListUIEx::SelectMoveTemp(int iIndex)
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

	IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
	if( pListItem == NULL ) return false;
	if( !pListItem->Select(true) ) 
	{		
		return false;
	}
	if (m_uMoveSelectIndex != iIndex)
	{
		m_uMoveSelectIndex = iIndex;
		if( m_pManager != NULL ) {
			m_pManager->SendNotify(this, _T("itemmove"), DUILIB_LIST_ITEM_MOVE, iIndex + m_nTopIndex);
		}
	}

	//	EnsureVisible(iIndex);
	return true;
}
//拖动时，取消选择的项目
bool CListUIEx::UnSelectMoveTemp()
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

bool CListUIEx::SelectMultiItem(int iIndex, bool bTakeFocus /* = false */, bool bSendMessage/* = true*/)
{
	if (m_bSingleSel)
	{
		return SelectItem(iIndex, bTakeFocus);
	}

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

//该函数只用于多选，单选不可用
bool CListUIEx::SetItemSelect(int nStart, int nEnd, bool bTakeFocus /* = false */)
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
		if (m_bStyleAddItem && (iIndex == m_nItems - 1))
		{
			continue;
		}
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
				continue;
			}
		}
	}

	//消息发送
	if( m_pManager != NULL ) {
		m_pManager->SendNotify(this, _T("itemselect"), DUILIB_LIST_ITEMSELECT, (WPARAM)m_aSelItems.GetAt(0));
	}

	//焦点跟随
	if (bTakeFocus)
	{
		m_nFocusItem = nEnd;
		CControlUI* pControl = GetItemAt(nEnd - m_nTopIndex);
		if( pControl == NULL ) return false;
		if( !pControl->IsVisible() ) return false;
		if( !pControl->IsEnabled() ) return false;
		pControl->SetFocus();
	}

	Invalidate();

	return true;
}

void CListUIEx::SetSingleSelect(bool bSingleSel)
{
	m_bSingleSel = bSingleSel;
	UnSelectAllItems(true);
}

bool CListUIEx::GetSingleSelect() const
{
	return m_bSingleSel;
}

bool CListUIEx::UnSelectItem(int iIndex, bool bSendMessage /*= true*/)
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
//	if (m_aSelItems.IsEmpty())   // 取消选中，发送消息
	{
		if (m_pManager != NULL) {
			if (bSendMessage)
				m_pManager->SendNotify(this, _T("itemunselect"), DUILIB_LIST_ITEMUNSELECT, 0, 0);
		}
	}
	return true;
}

void CListUIEx::SelectAllItems()
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

void CListUIEx::UnSelectAllItems()
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

void CListUIEx::UnSelectAllItems(bool bSendMessage)
{
	UnSelectAllItems();
	if (bSendMessage)
	{
		if (m_pManager != NULL) {
			m_pManager->SendNotify(this, _T("itemunselect"), DUILIB_LIST_ITEMUNSELECT, 0, 0);
		}
	}
}


int CListUIEx::GetSelectItemCount() const
{
	return m_aSelItems.GetSize();
}

int CListUIEx::GetNextSelItem(int nItem) const
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

int CListUIEx::SetFocusItem(int nItem, bool bTakeFocus /*= false*/)
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
int CListUIEx::SetEndItem(int nItem, bool bTakeFocus/* = false*/)
{
	if ((nItem < 0) || (nItem > (GetCount() - 1)))
	{
		if (m_nStartSelItem > GetCount() - 1)
		{
			UnSelectAllItems();
		}
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
	else
	{
		if (IsHiddenItemInTop() && (nItem != 0 ))
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

int CListUIEx::GetEndItem() const
{
	return m_nEndSelItem;
}

bool CListUIEx::SetLButton_ClickData(bool bDown)   //设置鼠标是否点击在数据区域
{
	bool bOldState = m_bLButtonDown_ClickInData;
	m_bLButtonDown_ClickInData = bDown;
	if (m_bLButtonDown_ClickInData == false)
	{
		CloseMoveWindow();
		m_bLButtonDown = false;
		UnSelectMoveTemp();
		m_bHasMoveItem = false;
	}
	return bOldState;
}

bool CListUIEx::GetLButtonClickData() const
{
	return m_bLButtonDown_ClickInData;
}

//是否已经移动了选中项
bool CListUIEx::IsMovedItem() const       
{
	return m_bHasMoveItem;
}


//设置鼠标是否被按下
bool CListUIEx::SetLButtonState(bool bDown)
{
	bool bOldState = m_bLButtonDown;
	m_bLButtonDown = bDown;
	return bOldState;
}

bool CListUIEx::GetLButtonState() const
{
	return m_bLButtonDown;
}

void CListUIEx::SetStartPoint(POINT pt)
{
	m_pStartPoint = pt;	
	m_pEndPoint = m_pStartPoint;
	m_pTempPoint = m_pEndPoint;
	m_StartPos = GetScrollPos();
}

 //获取选中的数组
CStdPtrArray CListUIEx::GetSelArray() const
{
	return m_aSelItems;
}

TListInfoUI* CListUIEx::GetListInfo()
{
	return &m_ListInfo;
}

int CListUIEx::GetChildPadding() const
{
	return m_pList->GetChildPadding();
}

void CListUIEx::SetChildPadding(int iPadding)
{
	m_pList->SetChildPadding(iPadding);
}

void CListUIEx::SetItemFont(int index)
{
	m_ListInfo.nFont = index;
	NeedUpdate();
}

void CListUIEx::SetItemTextStyle(UINT uStyle)
{
	m_ListInfo.uTextStyle = uStyle;
	NeedUpdate();
}

void CListUIEx::SetItemTextPadding(RECT rc)
{
	m_ListInfo.rcTextPadding = rc;
	NeedUpdate();
}

RECT CListUIEx::GetItemTextPadding() const
{
	return m_ListInfo.rcTextPadding;
}

void CListUIEx::SetItemTextColor(DWORD dwTextColor)
{
	m_ListInfo.dwTextColor = dwTextColor;
	Invalidate();
}

void CListUIEx::SetItemBkColor(DWORD dwBkColor)
{
	m_ListInfo.dwBkColor = dwBkColor;
	Invalidate();
}

void CListUIEx::SetItemBkImage(LPCTSTR pStrImage)
{
	m_ListInfo.sBkImage = pStrImage;
	Invalidate();
}

void CListUIEx::SetAlternateBk(bool bAlternateBk)
{
	m_ListInfo.bAlternateBk = bAlternateBk;
	Invalidate();
}

DWORD CListUIEx::GetItemTextColor() const
{
	return m_ListInfo.dwTextColor;
}

DWORD CListUIEx::GetItemBkColor() const
{
	return m_ListInfo.dwBkColor;
}

LPCTSTR CListUIEx::GetItemBkImage() const
{
	return m_ListInfo.sBkImage;
}

bool CListUIEx::IsAlternateBk() const
{
	return m_ListInfo.bAlternateBk;
}

void CListUIEx::SetSelectedItemTextColor(DWORD dwTextColor)
{
	m_ListInfo.dwSelectedTextColor = dwTextColor;
	Invalidate();
}

void CListUIEx::SetSelectedItemBkColor(DWORD dwBkColor)
{
	m_ListInfo.dwSelectedBkColor = dwBkColor;
	Invalidate();
}

void CListUIEx::SetSelectedItemImage(LPCTSTR pStrImage)
{
	m_ListInfo.sSelectedImage = pStrImage;
	Invalidate();
}

DWORD CListUIEx::GetSelectedItemTextColor() const
{
	return m_ListInfo.dwSelectedTextColor;
}

DWORD CListUIEx::GetSelectedItemBkColor() const
{
	return m_ListInfo.dwSelectedBkColor;
}

LPCTSTR CListUIEx::GetSelectedItemImage() const
{
	return m_ListInfo.sSelectedImage;
}

void CListUIEx::SetHotItemTextColor(DWORD dwTextColor)
{
	m_ListInfo.dwHotTextColor = dwTextColor;
	Invalidate();
}

void CListUIEx::SetHotItemBkColor(DWORD dwBkColor)
{
	m_ListInfo.dwHotBkColor = dwBkColor;
	Invalidate();
}

void CListUIEx::SetHotItemImage(LPCTSTR pStrImage)
{
	m_ListInfo.sHotImage = pStrImage;
	Invalidate();
}

DWORD CListUIEx::GetHotItemTextColor() const
{
	return m_ListInfo.dwHotTextColor;
}
DWORD CListUIEx::GetHotItemBkColor() const
{
	return m_ListInfo.dwHotBkColor;
}

LPCTSTR CListUIEx::GetHotItemImage() const
{
	return m_ListInfo.sHotImage;
}

void CListUIEx::SetDisabledItemTextColor(DWORD dwTextColor)
{
	m_ListInfo.dwDisabledTextColor = dwTextColor;
	Invalidate();
}

void CListUIEx::SetDisabledItemBkColor(DWORD dwBkColor)
{
	m_ListInfo.dwDisabledBkColor = dwBkColor;
	Invalidate();
}

void CListUIEx::SetDisabledItemImage(LPCTSTR pStrImage)
{
	m_ListInfo.sDisabledImage = pStrImage;
	Invalidate();
}

void CListUIEx::SetMoveSelectImage(LPCTSTR pStrImage)
{
	m_strMoveSelectImage = pStrImage;
}

LPCTSTR CListUIEx::GetMoveSelectImage()
{
	return m_strMoveSelectImage;
}

void CListUIEx::SetEmptyImage(LPCTSTR pStrImage)
{	
	m_strEmptyImage = pStrImage;
}

void CListUIEx::PaintEmptyImage(bool bPaint/* = true*/)
{
	m_bPaintEmptyImage = bPaint;
	Invalidate();
}

LPCTSTR CListUIEx::GetEmptyImage()
{
	return m_strEmptyImage;
}

DWORD CListUIEx::GetDisabledItemTextColor() const
{
	return m_ListInfo.dwDisabledTextColor;
}

DWORD CListUIEx::GetDisabledItemBkColor() const
{
	return m_ListInfo.dwDisabledBkColor;
}

LPCTSTR CListUIEx::GetDisabledItemImage() const
{
	return m_ListInfo.sDisabledImage;
}

DWORD CListUIEx::GetItemLineColor() const
{
	return m_ListInfo.dwLineColor;
}

void CListUIEx::SetItemLineColor(DWORD dwLineColor)
{
	m_ListInfo.dwLineColor = dwLineColor;
	Invalidate();
}

bool CListUIEx::IsItemShowHtml()
{
	return m_ListInfo.bShowHtml;
}

void CListUIEx::SetItemShowHtml(bool bShowHtml)
{
	if( m_ListInfo.bShowHtml == bShowHtml ) return;

	m_ListInfo.bShowHtml = bShowHtml;
	NeedUpdate();
}

void CListUIEx::SetMultiExpanding(bool bMultiExpandable)
{
	m_ListInfo.bMultiExpandable = bMultiExpandable;
}

bool CListUIEx::ExpandItem(int iIndex, bool bExpand /*= true*/)
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

int CListUIEx::GetExpandedItem() const
{
	return m_iExpandedItem;
}

void CListUIEx::Scroll(int dx, int dy)
{
	if( dx == 0 && dy == 0 ) return;
	SIZE sz = m_pList->GetScrollPos();
	m_pList->SetScrollPos(CSize(sz.cx + dx, sz.cy + dy));
}

void CListUIEx::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if( _tcscmp(pstrName, _T("header")) == 0 ) GetHeader()->SetVisible(_tcscmp(pstrValue, _T("hidden")) != 0);
	else if( _tcscmp(pstrName, _T("headerbkimage")) == 0 ) GetHeader()->SetBkImage(pstrValue);
	else if (_tcscmp(pstrName, _T("toheaderheight")) == 0) SetToHeaderHeight(_ttoi(pstrValue));
	else if( _tcscmp(pstrName, _T("scrollselect")) == 0 ) SetScrollSelect(_tcscmp(pstrValue, _T("true")) == 0);
	else if( _tcscmp(pstrName, _T("multiexpanding")) == 0 ) SetMultiExpanding(_tcscmp(pstrValue, _T("true")) == 0);
	else if( _tcscmp(pstrName, _T("itemfont")) == 0 ) m_ListInfo.nFont = _ttoi(pstrValue);
	else if ( _tcscmp(pstrName, _T("moveselectimage")) == 0 ) SetMoveSelectImage(pstrValue);
	else if ( _tcscmp(pstrName, _T("emptyimage")) == 0 ) SetEmptyImage(pstrValue);
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
	else if (_tcscmp(pstrName, _T("additemstyle")) == 0)          //add by chenjinfeng 2016.08.09， 是否有额外的添加选项
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
	else if (_tcscmp(pstrName, _T("itemstyle")) == 0)
	{
		if (_tcscmp(pstrValue, _T("true")) == 0)
		{
			SetWindowsStyle(true);
		}
		else
			SetWindowsStyle(false);
	}
	else if (_tcscmp(pstrName, _T("itemleftwidth")) == 0)
	{
		SetLeftWidth(_ttoi(pstrValue));
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
	else if( _tcscmp(pstrName, _T("showhtml")) == 0 ) m_bShowHtml = _tcscmp(pstrValue, _T("true")) == 0?true:false;
	else CVerticalLayoutUI::SetAttribute(pstrName, pstrValue);
}

void CListUIEx::PaintSelectItem(HDC hDC, const RECT &rcPaint)
{
	if (m_pList == NULL)
	{
		return;
	}
	int dx = rcPaint.left - m_rcItem.left;
	int dy = rcPaint.top - m_rcItem.top ;
	for (int i =0; i< m_pList->GetCount(); i++)
	{
		CListContainerElementUIEx *pListItem = static_cast<CListContainerElementUIEx*>(m_pList->GetItemAt(i));
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

void CListUIEx::PaintEmptyImage(HDC hDC)   // 绘制为空时图片
{
	if (m_nItems != 0)
	{
		return;
	}
	if (m_bPaintEmptyImage == false)
	{
		return;
	}
	if (m_strEmptyImage.IsEmpty())
	{
		return;
	}
	RECT rc = m_rcItem;
	const TImageInfo* data = NULL;
	if (m_pManager)
	{
		data = m_pManager->GetImageEx(m_strEmptyImage, NULL, 0);
	}
	if (data != 0)
	{
		int nWidth = data->nX;
		int nHeight = data->nY;
		rc.left =( m_rcItem.right - m_rcItem.left - nWidth)/2 + m_rcItem.left;
		rc.right = rc.left + nWidth;
		rc.top = ( m_rcItem.bottom - m_rcItem.top - nWidth)/2 + m_rcItem.top;
		rc.bottom = rc.top + nHeight;
		if (!DrawImage(hDC, m_strEmptyImage, rc, NULL))
		{
			m_strEmptyImage.Empty();
		}			
	}



}

void CListUIEx::PaintText(HDC hDC)
{
	if (m_pListDataSource == NULL)
	{
		return;
	}
	if (m_nItems != 0)
	{
		return;
	}
	PaintEmptyImage(hDC);

	if (m_sText.IsEmpty() || m_bPaintEmptyImage)
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


void CListUIEx::DoPaint(HDC hDC, const RECT& rcPaint)
{
// 	CStdString str1;
// 	str1.Format(_T("Paint the elements\n"));
// 	OutputDebugStr(str1);
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

IListCallbackUI* CListUIEx::GetTextCallback() const
{
	return m_pCallback;
}

void CListUIEx::SetTextCallback(IListCallbackUI* pCallback)
{
	m_pCallback = pCallback;
}

SIZE CListUIEx::GetScrollPos() const
{
	return m_pList->GetScrollPos();
}

SIZE CListUIEx::GetScrollRange() const
{
	return m_pList->GetScrollRange();
}

void CListUIEx::SetScrollPos(SIZE szPos)
{
	m_pList->SetScrollPos(szPos);
}

void CListUIEx::LineUp()
{
	m_pList->LineUp();
}

void CListUIEx::LineDown()
{
	m_pList->LineDown();
}

void CListUIEx::PageUp()
{
	m_pList->PageUp();
}

void CListUIEx::PageDown()
{
	m_pList->PageDown();
}

void CListUIEx::HomeUp()
{
	m_pList->HomeUp();
}

void CListUIEx::EndDown()
{
	m_pList->EndDown();
}

void CListUIEx::LineLeft()
{
	m_pList->LineLeft();
}

void CListUIEx::LineRight()
{
	m_pList->LineRight();
}

void CListUIEx::PageLeft()
{
	m_pList->PageLeft();
}

void CListUIEx::PageRight()
{
	m_pList->PageRight();
}

void CListUIEx::HomeLeft()
{
	m_pList->HomeLeft();
}

void CListUIEx::EndRight()
{
	m_pList->EndRight();
}

void CListUIEx::EnableScrollBar(bool bEnableVertical, bool bEnableHorizontal)
{
	m_pList->EnableScrollBar(bEnableVertical, bEnableHorizontal);
}

CScrollBarUI* CListUIEx::GetVerticalScrollBar() const
{
	return m_pList->GetVerticalScrollBar();
}

CScrollBarUI* CListUIEx::GetHorizontalScrollBar() const
{
	return m_pList->GetHorizontalScrollBar();
}

void CListUIEx::SetEditIndex(int iIndex)
{
	m_nEditIndex = iIndex;
}

int CListUIEx::GetEditiIndex()
{
	return m_nEditIndex;
}

void CListUIEx::SetToHeaderHeight(int nHeight)
{
	m_ntoHeaderHeight = nHeight;
}

int CListUIEx::GetToHeaderHeight()
{
	return m_ntoHeaderHeight;
}

void CListUIEx::SetAddItemStyle(bool bAddItem /*= false*/)
{
	m_bStyleAddItem = bAddItem;
}

void CListUIEx::SetWindowsStyle(bool bStyle /*= false*/)
{
	m_bWindowsStyle = bStyle;
}

bool CListUIEx::GetWindowsStyle() const
{
	return m_bWindowsStyle;
}

void CListUIEx::SetLeftWidth(int nWidth)
{
	m_nLeftWidth = nWidth;
}

int CListUIEx::GetLeftWidth() const
{
	return m_nLeftWidth;
}

/////////////////////////////////////////////////////////////////////////////////////
//
//


CListBodyUIEx::CListBodyUIEx(CListUIEx* pOwner) : m_pOwner(pOwner)
{
	ASSERT(m_pOwner);
	SetInset(CRect(0,0,0,0));
	m_nLeavePos = 0;
}

LPCTSTR CListBodyUIEx::GetClass()
{
	if (m_pOwner)
	{
		return m_pOwner->GetClass();
	}
	return NULL;
}

void CListBodyUIEx::SetVerticalScrollPos(int nPos)
{
	if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() )
	{
		m_pVerticalScrollBar->SetScrollPos(nPos);
	}
}

void CListBodyUIEx::SetVerticalScrollRange(int nRange)
{
	if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() )
	{
		m_pVerticalScrollBar->SetScrollPos(nRange);
	}
}

int CListBodyUIEx::GetVerticalFixWidth()
{
	if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() )
		return m_pVerticalScrollBar->GetFixedWidth();
	return 0;
}

void CListBodyUIEx::LineUp()
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
	if( m_pManager ) 
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

// 	TCHAR wstr[256] = {0};
// 	::wsprintf(wstr, _T("CListBodyUIEx::LineUp The sz.cy is %d, nRes is %d\n"), sz.cy, nRes);
// 	OutputDebugStr(wstr);

}

//在底部是否有隐藏的项
bool CListBodyUIEx::IsHiddenItemInBottom()
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

UINT CListBodyUIEx::GetHiddenItemInBootom()        //获取底部隐藏的像素
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
	if( !pControl->IsVisible() )
	{
		int i = 0;
	}
	if( pControl->IsFloat() ) 
		return 0;
	RECT rect;
	rect = pControl->GetPos();
	if (rect.bottom > m_rcItem.bottom)
	{
		return rect.bottom - m_rcItem.bottom;
	}
	return 0;
}


//在头部是否有隐藏的项
bool CListBodyUIEx::IsHiddenItemInTop()
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

//获取头图隐藏的像素
UINT CListBodyUIEx::GetHiddenItemInTop()
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
	if( !pControl->IsVisible() )
	{
		int i = 0;
	}
	if( pControl->IsFloat() ) 
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


void CListBodyUIEx::PageUp()
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

void CListBodyUIEx::PageDown()
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

void CListBodyUIEx::HomeUp()
{
	SIZE sz = GetScrollPos();
	sz.cy = 0;
	SetScrollPos(sz);
}

void CListBodyUIEx::EndDown()
{
	SIZE sz = GetScrollPos();
	sz.cy = GetScrollRange().cy;
	SetScrollPos(sz);
}

void CListBodyUIEx::LineDown()
{
	bool bRefreshData = true;
	if (m_pOwner)
		bRefreshData = m_pOwner->ScrollDownLine();

	if (!bRefreshData && m_pOwner && m_pOwner->IsListDataChanged())
	{		
		m_pOwner->RefreshListData(0, true);      // 重新刷新数据，并将top置为0 
		return;
	}

	int cyLine = 8;
	if( m_pManager )
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

 	//TCHAR wstr[256] = {0};
 	//::wsprintf(wstr, _T("CListBodyUIEx::LineDown The sz.cy is %d, range is %d\n"), sz.cy, GetScrollRange().cy);
 	//OutputDebugStr(wstr);
}

void CListBodyUIEx::SetScrollLinePos(SIZE szPos, bool bScrollItem /* = true */)
{
	int cx = 0;
	int cy = 0;
	int nRange = 0;
	if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
		int iLastScrollPos = m_pVerticalScrollBar->GetScrollPos();
		m_pVerticalScrollBar->SetScrollPos(szPos.cy);
		cy = m_pVerticalScrollBar->GetScrollPos() - iLastScrollPos;
		nRange = m_pVerticalScrollBar->GetScrollRange();
	}

// 	if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) {
// 		int iLastScrollPos = m_pHorizontalScrollBar->GetScrollPos();
// 		m_pHorizontalScrollBar->SetScrollPos(szPos.cx);
// 		cx = m_pHorizontalScrollBar->GetScrollPos() - iLastScrollPos;
// 	}

	if( /*cx == 0 &&*/ cy == 0) return;

	bool bEnd = false;
	int nPos = szPos.cy;
	if (szPos.cy < 0)
	{
		nPos = 0;
	}
	if (szPos.cy > nRange)
		nPos = nRange;

	int nItemHeight = 0;

	if (m_pOwner)
	{
		nItemHeight = m_pOwner->GetItemHeight();
		int nStep = (nPos - 1) / nItemHeight;
		int noldIndex = m_pOwner->GetTopIndex();
		if ((nPos - 1) % nItemHeight != 0)   //校正数据
		{
			if (nPos == nRange)
			{
				int nHeight = 0;
				if (m_pOwner)
				{
					nHeight  = m_pOwner->GetToHeaderHeight();
				}
				m_nLeavePos = (nPos - nHeight)%nItemHeight;
			}
			else
				m_nLeavePos = 0;
		}
	}

	if (!bScrollItem)
	{
		Invalidate();
		return;
	}

	//计算位置
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

// 	if( cx != 0 && m_pOwner ) {
// 		CListHeaderUI* pHeader = m_pOwner->GetHeader();
// 		if( pHeader == NULL ) return;
// 		TListInfoUI* pInfo = m_pOwner->GetListInfo();
// 		pInfo->nColumns = MIN(pHeader->GetCount(), UILIST_MAX_COLUMNS);
// 
// 		if( !pHeader->IsVisible() ) {
// 			for( int it = 0; it < pHeader->GetCount(); it++ ) {
// 				static_cast<CControlUI*>(pHeader->GetItemAt(it))->SetInternVisible(true);
// 			}
// 		}
// 		for( int i = 0; i < pInfo->nColumns; i++ ) {
// 			CControlUI* pControl = static_cast<CControlUI*>(pHeader->GetItemAt(i));
// 			if( !pControl->IsVisible() ) continue;
// 			if( pControl->IsFloat() ) continue;
// 
// 			RECT rcPos = pControl->GetPos();
// 			rcPos.left -= cx;
// 			rcPos.right -= cx;
// 			pControl->SetPos(rcPos);
// 			pInfo->rcColumn[i] = pControl->GetPos();
// 		}
// 		if( !pHeader->IsVisible() ) {
// 			for( int it = 0; it < pHeader->GetCount(); it++ ) {
// 				static_cast<CControlUI*>(pHeader->GetItemAt(it))->SetInternVisible(false);
// 			}
// 		}
// 	}
}


void CListBodyUIEx::SetScrollPos(SIZE szPos)
{
	int cx = 0;
	int cy = 0;
	int m_nRange = 0;
	if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
		int iLastScrollPos = m_pVerticalScrollBar->GetScrollPos();
		m_pVerticalScrollBar->SetScrollPos(szPos.cy);
		cy = m_pVerticalScrollBar->GetScrollPos() - iLastScrollPos;
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
	   nPos = m_nRange;

	int nItemHeight = 0; 

	if (m_pOwner)
	{
		nItemHeight = m_pOwner->GetItemHeight();
		int nStep = (nPos - 1)/nItemHeight;
		int noldIndex = m_pOwner->GetTopIndex();
		if ((nPos - 1)%nItemHeight != 0)   //校正数据
		{
			if (nPos == m_nRange)
			{
				int nHeight = 0;
				if (m_pOwner)
				{
					nHeight = m_pOwner->GetToHeaderHeight();
				}
				m_nLeavePos = (nPos - nHeight) % nItemHeight;
			}
			else
				m_nLeavePos = 0;
			nStep ++;
		}
		m_pOwner->RefreshListData(nStep);
	}
	
	//计算位置
	cy = 0;
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
 
	//计算Header的位置
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

void CListBodyUIEx::SetPos(RECT rc)
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
	int iPosY = rc.top - m_nLeavePos;
	if (m_pOwner)
	{
		iPosY += m_pOwner->GetToHeaderHeight();
	}
// 	if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
// 		iPosY -= m_pVerticalScrollBar->GetScrollPos();
// 	}
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
		bool bWindowsStyle = false;
		if (m_pOwner)
		{
			bWindowsStyle = m_pOwner->GetWindowsStyle();
		}
		if (bWindowsStyle)
		{
			sz.cx = cxNeeded;
		}
		else
		{
			sz.cx = MAX(cxNeeded, szAvailable.cx - rcPadding.left - rcPadding.right);
		}
		if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();
		if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();

		RECT rcCtrl = { iPosX + rcPadding.left, iPosY + rcPadding.top, iPosX + rcPadding.left + sz.cx, iPosY + sz.cy + rcPadding.top + rcPadding.bottom };
		pControl->SetPos(rcCtrl);

		iPosY += sz.cy + m_iChildPadding + rcPadding.top + rcPadding.bottom - 1;
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

void CListBodyUIEx::DoEvent(TEventUI& event)
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

CListContainerElementUIEx::CListContainerElementUIEx() : 
m_iIndex(-1),
m_pOwner(NULL), 
m_bSelected(false),
m_uButtonState(0)
{
	m_uScrollshowControl = -1;
	m_bNeedSelected = false;
	m_bShade = false;
	m_dwShadeColor = 0x78535353;
	m_bvisibleEdit = false;
	m_bCanSel = true;
}

LPCTSTR CListContainerElementUIEx::GetClass() const
{
	if (m_pOwner)
	{
		return m_pOwner->GetClass();
	}
	return _T("ListContainerElementUIEx");
}

UINT CListContainerElementUIEx::GetControlFlags() const
{
	return UIFLAG_WANTRETURN;
}

LPVOID CListContainerElementUIEx::GetInterface(LPCTSTR pstrName)
{
	if( _tcscmp(pstrName, _T("ListItem")) == 0 ) return static_cast<IListItemUI*>(this);
	if( _tcscmp(pstrName, _T("ListContainerElementEx")) == 0 ) return static_cast<CListContainerElementUIEx*>(this);
	return CContainerUI::GetInterface(pstrName);
}

CListUIEx* CListContainerElementUIEx::GetOwner()
{
	return m_pOwner;
}

void CListContainerElementUIEx::SetOwner(CControlUI* pOwner)
{
	m_pOwner = static_cast<CListUIEx*>(pOwner->GetInterface(_T("ListEx")));
}

void CListContainerElementUIEx::SetVisible(bool bVisible)
{
	CContainerUI::SetVisible(bVisible);
	if( !IsVisible() && m_bSelected)
	{
		m_bSelected = false;
		if( m_pOwner != NULL ) 
			m_pOwner->SelectItem(-1);
	}
}

void CListContainerElementUIEx::SetEnabled(bool bEnable)
{
	CControlUI::SetEnabled(bEnable);
	if( !IsEnabled() ) {
		m_uButtonState = 0;
	}
}

int CListContainerElementUIEx::GetIndex() const
{
	return m_iIndex;
}

void CListContainerElementUIEx::SetIndex(int iIndex)
{
	m_iIndex = iIndex;
}

void CListContainerElementUIEx::Invalidate()
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

bool CListContainerElementUIEx::Activate()
{
	if( !CContainerUI::Activate() ) return false;
	if( m_pManager != NULL ) m_pManager->SendNotify(this, _T("itemactivate"), DUILIB_LIST_ITEMACTIVE, m_iIndex);
	return true;
}

bool CListContainerElementUIEx::IsSelected() const
{
	return m_bSelected;
}

bool CListContainerElementUIEx::Select(bool bSelect /* = true */, bool bInvalidate /* = true */)
{
	if (!IsEnabled()) return false;

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

bool CListContainerElementUIEx::IsExpanded() const
{
	return false;
}

bool CListContainerElementUIEx::Expand(bool /*bExpand = true*/)
{
	return false;
}

void CListContainerElementUIEx::DoEvent(TEventUI& event)
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
		if (m_pOwner == NULL)
		{
			return;
		}
		if (!m_pOwner->IsEnabled())
		{
			return;
		}
		if( ::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled() )    //鼠标点解在区域
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
	if( event.Type == UIEVENT_BUTTONUP ) 
	{
		if (m_pOwner == NULL)
		{
			return;
		}
		if (!m_pOwner->IsEnabled())
		{
			return;
		}
		if (m_bNeedSelected && ::PtInRect(&m_rcItem, event.ptMouse) && !m_pOwner->IsMovedItem())
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
				if (!m_pOwner->IsMovedItem())
				{
					return;
				}
				if ((m_uScrollshowControl >= 0) &&  (m_items.GetSize() > m_uScrollshowControl))
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
	if( event.Type == UIEVENT_MOUSEENTER )
	{
		if (m_pOwner && !m_pOwner->IsEnabled())
		{
			return;
		}
		if (m_pOwner && m_pOwner->GetLButtonClickData())
		{
			m_pOwner->SelectMoveTemp(m_iIndex);
			return;
		}
		if ((m_uScrollshowControl >= 0) &&  (m_items.GetSize() > m_uScrollshowControl))
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
		if( IsEnabled() ) {
			m_uButtonState |= UISTATE_HOT;
			m_pManager->SendNotify(this, _T("item_mouseenter"), DUILIB_LIST_ITEM_MOUSEENTER);
			Invalidate();
		}
		return;
	}
	if( event.Type == UIEVENT_MOUSELEAVE )
	{
		//if (::PtInRect(&m_rcItem, event.ptMouse))    // 如果鼠标在当前项目中，离开子控件消息，不处理
		//{
		//	return;
		//}

		if (m_pOwner && !m_pOwner->IsEnabled())
		{
			return;
		}
		if (m_pOwner && m_pOwner->GetLButtonClickData())
		{
			m_pOwner->UnSelectMoveTemp();
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
		if( (m_uButtonState & UISTATE_HOT) != 0 ) {
			m_uButtonState &= ~UISTATE_HOT;
			Invalidate();
			m_pManager->SendNotify(this, _T("item_mouseleave"), DUILIB_LIST_ITEM_MOUSELEAVE);
		}
		return;
	}

	// An important twist: The list-item will send the event not to its immediate
	// parent but to the "attached" list. A list may actually embed several components
	// in its path to the item, but key-presses etc. needs to go to the actual list.
	if( m_pOwner != NULL ) m_pOwner->DoEvent(event); else CControlUI::DoEvent(event);
}

void CListContainerElementUIEx::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if( _tcscmp(pstrName, _T("selected")) == 0 ) Select();
	else if (_tcscmp(pstrName, _T("scrollShowControl")) == 0)  
	{
		m_uScrollshowControl = _tcstol(pstrValue, NULL, 10);
	}
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
	else CContainerUI::SetAttribute(pstrName, pstrValue);
}

void CListContainerElementUIEx::DoPaint(HDC hDC, const RECT& rcPaint)
{
	if( !::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem) ) return;
	bool bWindowsStyle = false;
	int nWidth = 0;
	if (m_pOwner)
	{
		bWindowsStyle = m_pOwner->GetWindowsStyle();
		nWidth = m_pOwner->GetLeftWidth();
	}
	RECT rcBkItem = m_rcItem;
	if (bWindowsStyle)
	{
		rcBkItem.left = rcBkItem.left + nWidth;
	}
	DrawItemBk(hDC, rcBkItem);
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

void CListContainerElementUIEx::DrawItemText(HDC hDC, const RECT& rcItem)
{
	return;
}

void CListContainerElementUIEx::DrawItemBk(HDC hDC, const RECT& rcItem)
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
			if (!DrawImage(hDC, (LPCTSTR)pInfo->sSelectedImage, rcItem)) pInfo->sSelectedImage.Empty();
			else return;
		}
	}
	if( (m_uButtonState & UISTATE_HOT) != 0 ) {
		if( !pInfo->sHotImage.IsEmpty() ) {
			if (!DrawImage(hDC, (LPCTSTR)pInfo->sHotImage, rcItem)) pInfo->sHotImage.Empty();
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
		RECT rcLine = { rcItem.left, rcItem.bottom - 1, rcItem.right, rcItem.bottom - 1 };
		CRenderEngine::DrawLine(hDC, rcLine, 1, GetAdjustColor(pInfo->dwLineColor));
	}
}


//add by lighten
SIZE CListContainerElementUIEx::EstimateSize(SIZE szAvailable)
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

void CListContainerElementUIEx::SetShade(bool bShade /* = false */)
{
	m_bShade = bShade;
}

bool CListContainerElementUIEx::GetShade() const
{
	return m_bShade;
}


void CListContainerElementUIEx::SetShadeColor(DWORD dwShadeColor)
{
	if (dwShadeColor != 0)
	{
		m_dwShadeColor = dwShadeColor;
	}
}

DWORD CListContainerElementUIEx::GetShadeColor() const
{
	return m_dwShadeColor;
}

void CListContainerElementUIEx::SetShadeImage(CStdString strImage)
{
	m_strShadeImage = strImage;
}

LPCTSTR CListContainerElementUIEx::GetShadeImage() const
{
	return m_strShadeImage;
}


void CListContainerElementUIEx::SetPos(RECT rc)
{
	if (m_pOwner == NULL)
		return;

	CControlUI::SetPos(rc);
	if( m_items.IsEmpty() ) return;
	rc.left += m_rcInset.left;
	rc.top += m_rcInset.top;
	rc.right -= m_rcInset.right;
	rc.bottom -= m_rcInset.bottom;
	m_rcNotData = m_rcItem;

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

void CListContainerElementUIEx::SetbVisibleEdit(bool bVisible)
{
	if (m_bvisibleEdit == bVisible) return;
	m_bvisibleEdit = bVisible;
	CStdString sName;
	if (bVisible)
	{
		CListElementEx * pListElement = static_cast<CListElementEx*>(m_items[0]);
		if (pListElement)
		{
			CLabelElement * pfileName = static_cast<CLabelElement*>(pListElement->GetChildItemControl(4));
			pListElement->SetChildAirItemVisible(10, false);
			pListElement->SetChildAirItemVisible(11, false);
			pListElement->SetChildAirItemVisible(12, false);
			pListElement->SetChildAirItemVisible(13, false);
			pListElement->SetChildAirItemVisible(14, false);
			pListElement->SetChildAirItemVisible(15, false);
			pListElement->SetChildItemVisible(10, false);
			pListElement->SetChildItemVisible(11, false);
			pListElement->SetChildItemVisible(12, false);
			pListElement->SetChildItemVisible(13, false);
			pListElement->SetChildItemVisible(14, false);
			pListElement->SetChildItemVisible(15, false);
			if (pfileName)
			{
				sName = pfileName->GetText();
				pfileName->SetVisible(false);
			}
			CEditUIEx *pEdit = static_cast<CEditUIEx*>(pListElement->GetChildItemControl(5));
			if (pEdit)
			{
				pEdit->SetVisible(true);
				int nPos = sName.ReverseFind('.');
				if (nPos != -1)
				{
					sName = sName.Left(nPos);
				}
				pEdit->SetText(sName);
			}
			pListElement->GetChildItemControl(6)->SetVisible(true);
			pListElement->GetChildItemControl(7)->SetVisible(true);
			pListElement->GetChildItemControl(8)->SetVisible(true);
			pListElement->GetChildItemControl(9)->SetVisible(true);
			
		}
	}
	else
	{
		CListElementEx * pListElement = static_cast<CListElementEx*>(m_items[0]);
		if (pListElement)
		{
			CLabelElement * pfileName = static_cast<CLabelElement*>(pListElement->GetChildItemControl(4));

			if (pfileName)
			{
				pfileName->SetVisible(true);
			}
			CEditUIEx *pEdit = static_cast<CEditUIEx*>(pListElement->GetChildItemControl(5));
			if (pEdit)
			{
				pEdit->SetVisible(false);
			}
			pListElement->GetChildItemControl(6)->SetVisible(false);
			pListElement->GetChildItemControl(7)->SetVisible(false);
			pListElement->GetChildItemControl(8)->SetVisible(false);
			pListElement->GetChildItemControl(9)->SetVisible(false);
		}

	}
}
void CListContainerElementUIEx::SetbVisibleEdit(bool bVisible, bool bIsFolder)
{
	if (m_bvisibleEdit == bVisible) return;
	m_bvisibleEdit = bVisible;
	CStdString sName;
	if (bVisible)
	{
		CListElementEx * pListElement = static_cast<CListElementEx*>(m_items[0]);
		if (pListElement)
		{
			CLabelElement * pfileName = static_cast<CLabelElement*>(pListElement->GetChildItemControl(4));
			pListElement->SetChildAirItemVisible(10, false);
			pListElement->SetChildAirItemVisible(11, false);
			pListElement->SetChildAirItemVisible(12, false);
			pListElement->SetChildAirItemVisible(13, false);
			pListElement->SetChildAirItemVisible(14, false);
			pListElement->SetChildAirItemVisible(15, false);
			pListElement->SetChildAirItemVisible(16, false);
			pListElement->SetChildAirItemVisible(17, false);
			pListElement->SetChildItemVisible(10, false);
			pListElement->SetChildItemVisible(11, false);
			pListElement->SetChildItemVisible(12, false);
			pListElement->SetChildItemVisible(13, false);
			pListElement->SetChildItemVisible(14, false);
			pListElement->SetChildItemVisible(15, false);
			pListElement->SetChildItemVisible(16, false);
			pListElement->SetChildItemVisible(17, false);
			if (pfileName)
			{
				sName = pfileName->GetText();
				pfileName->SetVisible(false);
			}
			CEditUIEx *pEdit = static_cast<CEditUIEx*>(pListElement->GetChildItemControl(5));
			if (pEdit)
			{
				pEdit->SetVisible(true);
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
			pListElement->GetChildItemControl(6)->SetVisible(true);
			pListElement->GetChildItemControl(7)->SetVisible(true);
			pListElement->GetChildItemControl(8)->SetVisible(true);
			pListElement->GetChildItemControl(9)->SetVisible(true);

		}
	}
	else
	{
		CListElementEx * pListElement = static_cast<CListElementEx*>(m_items[0]);
		if (pListElement)
		{
			CLabelElement * pfileName = static_cast<CLabelElement*>(pListElement->GetChildItemControl(4));

			if (pfileName)
			{
				pfileName->SetVisible(true);
			}
			CEditUIEx *pEdit = static_cast<CEditUIEx*>(pListElement->GetChildItemControl(5));
			if (pEdit)
			{
				pEdit->SetVisible(false);
			}
			pListElement->GetChildItemControl(6)->SetVisible(false);
			pListElement->GetChildItemControl(7)->SetVisible(false);
			pListElement->GetChildItemControl(8)->SetVisible(false);
			pListElement->GetChildItemControl(9)->SetVisible(false);
		}

	}
}

void CListContainerElementUIEx::SetCanSel(bool bCanSel /*= true*/)
{
	m_bCanSel = bCanSel;
}

bool CListContainerElementUIEx::GetCanSel() const
{
	return m_bCanSel;
}


CIgnoreListContainerElementUI::CIgnoreListContainerElementUI()
{

}

LPCTSTR CIgnoreListContainerElementUI::GetClass() const
{
	if (m_pOwner)
	{
		return m_pOwner->GetClass();
	}
	return _T("IgnoreListContainerElementUI");
}

UINT CIgnoreListContainerElementUI::GetControlFlags() const
{
	return UIFLAG_WANTRETURN;
}

LPVOID CIgnoreListContainerElementUI::GetInterface(LPCTSTR pstrName)
{
	if (_tcscmp(pstrName, _T("ListItem")) == 0) return static_cast<IListItemUI*>(this);
	if (_tcscmp(pstrName, _T("IgnoreListContainerElement")) == 0) return static_cast<CIgnoreListContainerElementUI*>(this);
	return CContainerUI::GetInterface(pstrName);
}

void CIgnoreListContainerElementUI::DoEvent(TEventUI& event)
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
					m_pOwner->SelectItem(m_iIndex);*/
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
			/*if (!IsSelected())
				m_pOwner->SelectItem(m_iIndex);*/
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
						m_bNeedSelected = true;
					}
					else if (!IsSelected() || ::PtInRect(&m_rcNotData, event.ptMouse))
					{
						m_pOwner->SelectMultiItem(m_iIndex);
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
		if (event.wParam == UISTATE_SELECTED)
		{
			CListElementEx *pItems = static_cast<CListElementEx *>(m_items.GetAt(0));
			if (pItems != NULL)
			{
				COptionElementUI * pOption = (COptionElementUI*)pItems->GetItemAt(0);
				if (pOption->IsSelected())
				{
					m_pOwner->SelectMultiItem(m_iIndex);
				}
				else
				{
					m_pOwner->UnSelectItem(m_iIndex);
				}
			}

			Invalidate();
			return;
		}
		if (m_bNeedSelected && ::PtInRect(&m_rcItem, event.ptMouse) && !m_pOwner->IsMovedItem())
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
		m_bNeedSelected = false;
		if (IsEnabled() && m_pOwner)
		{
			m_pOwner->SetLButtonState(false);
			if (m_pOwner->GetLButtonClickData())
			{
				m_pOwner->SetLButton_ClickData(false);
				if (!m_pOwner->IsMovedItem())
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
		if (m_pOwner && m_pOwner->GetLButtonClickData())
		{
			m_pOwner->SelectMoveTemp(m_iIndex);
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
			m_pManager->SendNotify(this, _T("item_mouseenter"), DUILIB_LIST_ITEM_MOUSEENTER);
			Invalidate();
		}
		return;
	}
	if (event.Type == UIEVENT_MOUSELEAVE)
	{
		//if (::PtInRect(&m_rcItem, event.ptMouse))    // 如果鼠标在当前项目中，离开子控件消息，不处理
		//{
		//	return;
		//}

		if (m_pOwner && !m_pOwner->IsEnabled())
		{
			return;
		}
		if (m_pOwner && m_pOwner->GetLButtonClickData())
		{
			m_pOwner->UnSelectMoveTemp();
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
			m_pManager->SendNotify(this, _T("item_mouseleave"), DUILIB_LIST_ITEM_MOUSELEAVE);
		}
		return;
	}

	// An important twist: The list-item will send the event not to its immediate
	// parent but to the "attached" list. A list may actually embed several components
	// in its path to the item, but key-presses etc. needs to go to the actual list.
	if (m_pOwner != NULL) m_pOwner->DoEvent(event); else CControlUI::DoEvent(event);
}

bool CIgnoreListContainerElementUI::Select(bool bSelect /*= true*/, bool bInvalidate /*= true*/)
{
	if (!IsEnabled()) return false;
	if (bSelect == m_bSelected) return true;
	m_bSelected = bSelect;
	// 	if( bSelect && m_pOwner != NULL ) 
	// 		m_pOwner->SelectItem(m_iIndex);
	if (m_bSelected == true)
	{
		CListElementEx *pItems = static_cast<CListElementEx *>(m_items.GetAt(0));
		if (pItems != NULL)
		{
			COptionElementUI * pOption = (COptionElementUI*)pItems->GetItemAt(0);
			if (pOption)
			{
				pOption->Selected(true,false);
			}
		}
	}
	else
	{
		CListElementEx *pItems = static_cast<CListElementEx *>(m_items.GetAt(0));
		if (pItems != NULL)
		{
			COptionElementUI * pOption = (COptionElementUI*)pItems->GetItemAt(0);
			if (pOption)
			{
				pOption->Selected(false, false);
			}
		}
	}
	if (bInvalidate)
	{
		Invalidate();
	}

	return true;
}

} // namespace DuiLib
