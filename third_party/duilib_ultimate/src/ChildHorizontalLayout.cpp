#include "stdafx.h"
namespace DuiLib {

CChildHorizontalLayout::CChildHorizontalLayout() : CHorizontalLayoutUI()
{
}

LPCTSTR CChildHorizontalLayout::GetClass() const
{
	return _T("ChildHorizontalLayout");
}

LPVOID CChildHorizontalLayout::GetInterface(LPCTSTR pstrName)
{
	if( _tcscmp(pstrName, _T("ChildHorizontalLayout")) == 0 ) 
		return static_cast<CChildHorizontalLayout*>(this);
	return CHorizontalLayoutUI::GetInterface(pstrName);
}

// 获取容器长度
SIZE CChildHorizontalLayout::EstimateSize(SIZE szAvailable)
{
	int cxFixed = 0;
	int cyFixed = 0;
	for( int it = 0; it < GetCount(); it++ ) {
		CControlUI* pControl = static_cast<CControlUI*>(GetItemAt(it));
		if( !pControl->IsVisible() ) continue;
		SIZE sz = pControl->EstimateSize(szAvailable);
		cxFixed += sz.cx;
		if( cyFixed < sz.cy )
			cyFixed = sz.cy;
	}
	if (cyFixed > m_cxyFixed.cy)
	{
		cyFixed = m_cxyFixed.cy; 
	}
	return CSize(cxFixed, cyFixed);

}


}