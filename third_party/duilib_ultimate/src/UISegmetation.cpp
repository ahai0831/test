#include "stdafx.h"
namespace DuiLib{


	CSegmetationUI::CSegmetationUI()
	{

	}

	LPCTSTR CSegmetationUI::GetClass() const
	{
		return _T("SegmetationUI");
	}

	LPVOID CSegmetationUI::GetInterface(LPCTSTR pstrName)
	{
		if (_tcscmp(pstrName, _T("Segmetation")) == 0) return this;
		return CHorizontalLayoutUI::GetInterface(pstrName);
	}

	SIZE CSegmetationUI::EstimateSize(SIZE szAvailable)
	{
		SIZE cXY = szAvailable;
		/*if (cXY.cy == 0 && m_pManager != NULL) {
			for (int it = 0; it < m_items.GetSize(); it++) {
				cXY.cy = MAX(cXY.cy, static_cast<CControlUI*>(m_items[it])->EstimateSize(szAvailable).cy);
			}
			int nMin = m_pManager->GetDefaultFontInfo()->tm.tmHeight + 6;
			cXY.cy = MAX(cXY.cy, nMin);
		}

		for (int it = 0; it < m_items.GetSize(); it++) {
			cXY.cx += static_cast<CControlUI*>(m_items[it])->EstimateSize(szAvailable).cx;
		}*/

		return cXY;
	}

	void CSegmetationUI::SetPos(RECT rc)
	{
		CControlUI::SetPos(rc);
		rc = m_rcItem;

		// Adjust for inset
		rc.left += m_rcInset.left;
		rc.top += m_rcInset.top;
		rc.right -= m_rcInset.right;
		rc.bottom -= m_rcInset.bottom;

		if (m_items.GetSize() == 0) {
			return;
		}


		// Determine the width of elements that are sizeable
		SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };
		int nMinwidth = 0;
		int nAdjustables = 0;
		int cxFixed = 0;
		int nEstimateNum = 0;
		for (int it1 = 0; it1 < m_items.GetSize(); it1++) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it1]);
			if (!pControl->IsVisible()) continue;
			if (pControl->IsFloat()) continue;
			SIZE sz = pControl->EstimateSize(szAvailable);
			if (sz.cx == 0) {
				nAdjustables++;
			}
			else {
				if (sz.cx < pControl->GetMinWidth()) sz.cx = pControl->GetMinWidth();
				if (sz.cx > pControl->GetMaxWidth()) sz.cx = pControl->GetMaxWidth();
			}
			nMinwidth += pControl->GetMinWidth();
			cxFixed += sz.cx + pControl->GetPadding().left + pControl->GetPadding().right;
			nEstimateNum++;
		}
		cxFixed += (nEstimateNum - 1) * m_iChildPadding;

		int cxExpand = 0;
		int cxNeeded = 0;
		if (nAdjustables > 0) cxExpand = MAX(0, (szAvailable.cx - cxFixed) / nAdjustables);
		// Position the elements
		if (szAvailable.cx == cxFixed)
		{
			cxNeeded = AllItemPos(szAvailable, rc, cxFixed, cxExpand, nAdjustables, cxNeeded, nEstimateNum);
		}
		else
		{
			if (szAvailable.cx > cxFixed)
			{
				int nCount = m_items.GetSize();
				CControlUI* pControl = static_cast<CControlUI*>(m_items[nCount - 1]);
				int nWidth = pControl->GetFixedWidth();
				pControl->SetFixedWidth(nWidth + szAvailable.cx - cxFixed);
				cxNeeded = AllItemPos(szAvailable, rc, cxFixed, cxExpand, nAdjustables, cxNeeded, nEstimateNum);
			}
			else
			{
				if (szAvailable.cx >= nMinwidth)
				{
					int nValue = cxFixed - szAvailable.cx;
					int nCount = m_items.GetSize();
					for (int i = nCount - 1; i >= 0; i--)
					{
						CControlUI* pControl = static_cast<CControlUI*>(m_items[i]);
						int nWidth = pControl->GetFixedWidth();
						int nMinWidth = pControl->GetMinWidth();
						if ((nWidth - nValue) > nMinWidth)
						{
							pControl->SetFixedWidth(nWidth - nValue);
							break;
						}
						else
						{
							nValue = nValue - nWidth + nMinWidth;
							pControl->SetFixedWidth(nMinWidth);
						}
					}
					cxNeeded = AllItemPos(szAvailable, rc, cxFixed, cxExpand, nAdjustables, cxNeeded, nEstimateNum);
				}
			}
		}
		
	}

	void CSegmetationUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		CHorizontalLayoutUI::SetAttribute(pstrName, pstrValue);
	}

	void CSegmetationUI::DoEvent(TEventUI& event)
	{
		if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND) {
			if (m_pParent != NULL) m_pParent->DoEvent(event);
			else CContainerUI::DoEvent(event);
			return;
		}

		if (event.Type == UIEVENT_MOUSEMOVE)
		{
			int nCount = m_items.GetSize();
			if (nCount > 1)
			{
				SIZE sMoveSize = { 0, 0 };
				vector<SIZE> vcallItem;
				bool bReCal = false;
				int nIndex = 0;
				int nAllWidth = 0;
				for (int i = 0; i < nCount; i++)
				{
					CControlUI* pContrl = static_cast<CControlUI*>(m_items.GetAt(i));
					SIZE nSize = pContrl->EstimateSize({ 0, 0 });
					vcallItem.push_back(nSize);
					nAllWidth += nSize.cx;
					if (pContrl == event.pSender)
					{
						sMoveSize = nSize;
						bReCal = true;
						nIndex = i;
					}
				}
				if (bReCal)
				{
						RECT rc = m_rcItem;
						rc.left += m_rcInset.left;
						rc.top += m_rcInset.top;
						rc.right -= m_rcInset.right;
						rc.bottom -= m_rcInset.bottom;
						int nWidth = rc.right - rc.left;
						int nVale = nWidth - nAllWidth;
						if (nVale > 0)
						{
							CControlUI * pContrl = static_cast<CControlUI*>(m_items.GetAt(nIndex));
							int nMinWidth = pContrl->GetMinWidth();
							//SIZE nSize = vcallItem.at(nIndex + 1);
							if (sMoveSize.cx  > nMinWidth)
							{
								CControlUI *pContrl2 = static_cast<CControlUI*>(m_items.GetAt(nIndex + 1));
								SIZE nSize = vcallItem.at(nIndex + 1);
								nSize.cx += nVale;
								pContrl2->SetFixedWidth(nSize.cx);
								NeedUpdate();

							}
							else
							{
								pContrl->SetFixedWidth(nMinWidth);
								int nChange = 0;
								nChange += nMinWidth;
								for (int k = 0; k < nCount; k++)
								{
									if (k != nIndex && k != nIndex + 1)
									{
										nChange += vcallItem.at(k).cx;
									}
								}
								CControlUI * pContrl2 = static_cast<CControlUI*>(m_items.GetAt(nIndex + 1));
								pContrl2->SetFixedWidth(nWidth - nChange);
								NeedUpdate();
							}
						}
						else
						{
							CControlUI * pContrl = static_cast<CControlUI*>(m_items.GetAt(nIndex + 1));
							SIZE nSize = vcallItem.at(nIndex + 1);
							int nMinWidth = pContrl->GetMinWidth();
							if ((nSize.cx + nVale) > nMinWidth)
							{
								pContrl->SetFixedWidth(nSize.cx + nVale);
								NeedUpdate();
							}
							else
							{
								pContrl->SetFixedWidth(nMinWidth);
								CControlUI *pContrl2 = static_cast<CControlUI*>(m_items.GetAt(nIndex));
								int nChange = 0;
								nChange += nMinWidth;
								for (int k = 0; k < nCount; k++)
								{
									if (k != nIndex && k != nIndex + 1)
									{
										nChange += vcallItem.at(k).cx;
									}
								}
								pContrl2->SetFixedWidth(nWidth - nChange);
								NeedUpdate();
							}
						}
					}
				
				vcallItem.clear();
			}
			
			return;
		}

		CContainerUI::DoEvent(event);
	}

	int CSegmetationUI::AllItemPos(SIZE szAvailable, RECT &rc, int cxFixed, int cxExpand, int nAdjustables, int cxNeeded, int nEstimateNum)
	{
		SIZE szRemaining = szAvailable;
		int iPosX = rc.left;

		int iAdjustable = 0;
		int cxFixedRemaining = cxFixed;

		int nHeaderWidth = GetWidth();
		for (int it2 = 0; it2 < m_items.GetSize(); it2++) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it2]);
			if (!pControl->IsVisible()) continue;
			if (pControl->IsFloat()) {
				SetFloatPos(it2);
				continue;
			}
			RECT rcPadding = pControl->GetPadding();
			szRemaining.cx -= rcPadding.left;

			SIZE sz = { 0, 0 };

			sz = pControl->EstimateSize(szRemaining);

			if (sz.cx == 0) {
				iAdjustable++;
				sz.cx = cxExpand;
				// Distribute remaining to last element (usually round-off left-overs)
				if (iAdjustable == nAdjustables) {
					sz.cx = MAX(0, szRemaining.cx - rcPadding.right - cxFixedRemaining);
				}
				if (sz.cx < pControl->GetMinWidth()) sz.cx = pControl->GetMinWidth();
				if (sz.cx > pControl->GetMaxWidth()) sz.cx = pControl->GetMaxWidth();
			}
			else {
				if (sz.cx < pControl->GetMinWidth()) sz.cx = pControl->GetMinWidth();
				if (sz.cx > pControl->GetMaxWidth()) sz.cx = pControl->GetMaxWidth();

				cxFixedRemaining -= sz.cx;
			}

			sz.cy = pControl->GetFixedHeight();
			if (sz.cy == 0) sz.cy = rc.bottom - rc.top - rcPadding.top - rcPadding.bottom;
			if (sz.cy < 0) sz.cy = 0;
			if (sz.cy < pControl->GetMinHeight()) sz.cy = pControl->GetMinHeight();
			if (sz.cy > pControl->GetMaxHeight()) sz.cy = pControl->GetMaxHeight();


			RECT rcCtrl = { iPosX + rcPadding.left, rc.top + rcPadding.top, iPosX + sz.cx + rcPadding.left + rcPadding.right, rc.top + rcPadding.top + sz.cy };
			pControl->SetPos(rcCtrl);
			iPosX += sz.cx + m_iChildPadding + rcPadding.left + rcPadding.right;
			cxNeeded += sz.cx + rcPadding.left + rcPadding.right;
			szRemaining.cx -= sz.cx + m_iChildPadding + rcPadding.right;
		}
		cxNeeded += (nEstimateNum - 1) * m_iChildPadding;	return cxNeeded;
	}


	CSegmetationItemUI::CSegmetationItemUI() : m_bDragable(false), m_uButtonState(0), m_iSepWidth(4)
	{
		ptLastMouse.x = ptLastMouse.y = 0;
		SetMinWidth(16);
	}

	LPCTSTR CSegmetationItemUI::GetClass() const
	{
		return _T("SegmetationItemUI");
	}

	LPVOID CSegmetationItemUI::GetInterface(LPCTSTR pstrName)
	{
		if (_tcscmp(pstrName, _T("SegmetationItem")) == 0) return this;
		return CContainerUI::GetInterface(pstrName);
	}

	UINT CSegmetationItemUI::GetControlFlags() const
	{
		if (IsEnabled() && m_iSepWidth != 0) return UIFLAG_SETCURSOR;
		else return 0;
	}

	void CSegmetationItemUI::SetEnabled(bool bEnable /*= true*/)
	{
		CContainerUI::SetEnabled(bEnable);
		if (!IsEnabled()) {
			m_uButtonState = 0;
		}
	}

	bool CSegmetationItemUI::IsDragable() const
	{
		return m_bDragable;
	}

	void CSegmetationItemUI::SetDragable(bool bDragable)
	{
		m_bDragable = bDragable;
		if (!m_bDragable) m_uButtonState &= ~UISTATE_CAPTURED;
	}

	DWORD CSegmetationItemUI::GetSepWidth() const
	{
		return m_iSepWidth;
	}

	void CSegmetationItemUI::SetSepWidth(int iWidth)
	{
		m_iSepWidth = iWidth;
	}

	LPCTSTR CSegmetationItemUI::GetSepImage() const
	{
		return m_sSepImage;
	}

	void CSegmetationItemUI::SetSepImage(LPCTSTR pStrImage)
	{
		m_sSepImage = pStrImage;
		Invalidate();
	}

	void CSegmetationItemUI::DoEvent(TEventUI& event)
	{
		if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND) {
			if (m_pParent != NULL) m_pParent->DoEvent(event);
			else CContainerUI::DoEvent(event);
			return;
		}

		if (event.Type == UIEVENT_SETFOCUS)
		{
			Invalidate();
		}
		if (event.Type == UIEVENT_KILLFOCUS)
		{
			Invalidate();
		}
		if (event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK)
		{
			if (!IsEnabled()) return;
			RECT rcSeparator = GetThumbRect();
			if (m_iSepWidth >= 0)//111024 by cddjr, 增加分隔符区域，方便用户拖动
				rcSeparator.left -= 4;
			else
				rcSeparator.right += 4;
			if (::PtInRect(&rcSeparator, event.ptMouse)) {
				if (m_bDragable) {
					m_uButtonState |= UISTATE_CAPTURED;
					ptLastMouse = event.ptMouse;
				}
			}
			return;
		}
		if (event.Type == UIEVENT_BUTTONUP)
		{
			if ((m_uButtonState & UISTATE_CAPTURED) != 0) {
				m_uButtonState &= ~UISTATE_CAPTURED;
				if (GetParent())
					GetParent()->NeedParentUpdate();
			}
			return;
		}
		if (event.Type == UIEVENT_MOUSEMOVE)
		{
			if ((m_uButtonState & UISTATE_CAPTURED) != 0) {
				RECT rc = m_rcItem;
				if (m_iSepWidth >= 0) {
					rc.right -= ptLastMouse.x - event.ptMouse.x;
				}
				else {
					rc.left -= ptLastMouse.x - event.ptMouse.x;
				}

				if (rc.right - rc.left > GetMinWidth()) {
					TEventUI Segevent = { 0 };
					memcpy(&Segevent, &event, sizeof(Segevent));
					Segevent.pSender = static_cast<CControlUI*>(this);
					m_cxyFixed.cx = rc.right - rc.left;
					ptLastMouse = event.ptMouse;
					if (GetParent())
					{
						GetParent()->DoEvent(Segevent);
					}	
				}
			}
			RECT rcSeparator = GetThumbRect();
			if (m_iSepWidth >= 0)//111024 by cddjr, 增加分隔符区域，方便用户拖动
				rcSeparator.left -= 10;
			else
				rcSeparator.right += 10;
			if (IsEnabled() && m_bDragable && ::PtInRect(&rcSeparator, event.ptMouse)) {
				::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEWE)));
			}
			return;
		}
		//if (event.Type == UIEVENT_SETCURSOR)
		//{
		//	RECT rcSeparator = GetThumbRect();
		//	if (m_iSepWidth >= 0)//111024 by cddjr, 增加分隔符区域，方便用户拖动
		//		rcSeparator.left -= 4;
		//	else
		//		rcSeparator.right += 4;
		//	if (IsEnabled() && m_bDragable && ::PtInRect(&rcSeparator, event.ptMouse)) {
		//		::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEWE)));
		//		return;
		//	}
		//}
		CContainerUI::DoEvent(event);
	}

	SIZE CSegmetationItemUI::EstimateSize(SIZE szAvailable)
	{
		return m_cxyFixed;
	}

	void CSegmetationItemUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if (_tcscmp(pstrName, _T("dragable")) == 0) SetDragable(_tcscmp(pstrValue, _T("true")) == 0);
		else if (_tcscmp(pstrName, _T("sepwidth")) == 0) SetSepWidth(_ttoi(pstrValue));
		else if (_tcscmp(pstrName, _T("sepimage")) == 0) SetSepImage(pstrValue);
		else CContainerUI::SetAttribute(pstrName, pstrValue);
	}

	RECT CSegmetationItemUI::GetThumbRect() const
	{
		if (m_iSepWidth >= 0) return CRect(m_rcItem.right - m_iSepWidth, m_rcItem.top, m_rcItem.right, m_rcItem.bottom);
		else return CRect(m_rcItem.left, m_rcItem.top, m_rcItem.left - m_iSepWidth, m_rcItem.bottom);
	}

	void CSegmetationItemUI::PaintStatusImage(HDC hDC)
	{
		if (!m_sSepImage.IsEmpty()) {
			RECT rcThumb = GetThumbRect();
			rcThumb.left -= m_rcItem.left;
			rcThumb.top -= m_rcItem.top;
			rcThumb.right -= m_rcItem.left;
			rcThumb.bottom -= m_rcItem.top;

			m_sSepImageModify.Empty();
			m_sSepImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), rcThumb.left, rcThumb.top, rcThumb.right, rcThumb.bottom);
			if (!DrawImage(hDC, (LPCTSTR)m_sSepImage, (LPCTSTR)m_sSepImageModify)) m_sSepImage.Empty();
		}
	}
}
