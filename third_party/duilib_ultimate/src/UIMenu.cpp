#include "StdAfx.h"
#include "UIMenu.h"

namespace DuiLib {

/////////////////////////////////////////////////////////////////////////////////////
//
ContextMenuObserver s_context_menu_observer;

// MenuUI
const TCHAR* const kMenuUIClassName = _T("MenuUI");
const TCHAR* const kMenuUIInterfaceName = _T("Menu");


CMenuUI::CMenuUI()
{
	if (GetHeader() != NULL)
		GetHeader()->SetVisible(false);
}

CMenuUI::~CMenuUI()
{
}

LPCTSTR CMenuUI::GetClass() const
{
    return kMenuUIClassName;
}

LPVOID CMenuUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcsicmp(pstrName, kMenuUIInterfaceName) == 0 ) return static_cast<CMenuUI*>(this);
    return CListUI::GetInterface(pstrName);
}

void CMenuUI::DoEvent(TEventUI& event)
{
	return __super::DoEvent(event);
}

bool CMenuUI::Add(CControlUI* pControl)
{
	CMenuElementUI* pMenuItem = static_cast<CMenuElementUI*>(pControl->GetInterface(kMenuElementUIInterfaceName));
	if (pMenuItem == NULL)
		return false;

	for (int i = 0; i < pMenuItem->GetCount(); ++i)
	{
		if (pMenuItem->GetItemAt(i)->GetInterface(kMenuElementUIInterfaceName) != NULL)
		{
			(static_cast<CMenuElementUI*>(pMenuItem->GetItemAt(i)->GetInterface(kMenuElementUIInterfaceName)))->SetInternVisible(false);
		}
	}
	return CListUI::Add(pControl);
}

bool CMenuUI::AddAt(CControlUI* pControl, int iIndex)
{
	CMenuElementUI* pMenuItem = static_cast<CMenuElementUI*>(pControl->GetInterface(kMenuElementUIInterfaceName));
	if (pMenuItem == NULL)
		return false;

	for (int i = 0; i < pMenuItem->GetCount(); ++i)
	{
		if (pMenuItem->GetItemAt(i)->GetInterface(kMenuElementUIInterfaceName) != NULL)
		{
			(static_cast<CMenuElementUI*>(pMenuItem->GetItemAt(i)->GetInterface(kMenuElementUIInterfaceName)))->SetInternVisible(false);
		}
	}
	return CListUI::AddAt(pControl, iIndex);
}

int CMenuUI::GetItemIndex(CControlUI* pControl) const
{
	CMenuElementUI* pMenuItem = static_cast<CMenuElementUI*>(pControl->GetInterface(kMenuElementUIInterfaceName));
	if (pMenuItem == NULL)
		return -1;

	return __super::GetItemIndex(pControl);
}

bool CMenuUI::SetItemIndex(CControlUI* pControl, int iIndex)
{
	CMenuElementUI* pMenuItem = static_cast<CMenuElementUI*>(pControl->GetInterface(kMenuElementUIInterfaceName));
	if (pMenuItem == NULL)
		return false;

	return __super::SetItemIndex(pControl, iIndex);
}

bool CMenuUI::Remove(CControlUI* pControl)
{
	CMenuElementUI* pMenuItem = static_cast<CMenuElementUI*>(pControl->GetInterface(kMenuElementUIInterfaceName));
	if (pMenuItem == NULL)
		return false;

	return __super::Remove(pControl);
}

bool CMenuUI::GetShadow()
{
	return m_bShadow;
}

SIZE CMenuUI::EstimateSize(SIZE szAvailable)
{
	int cxFixed = 0;
    int cyFixed = 0;
    for( int it = 0; it < GetCount(); it++ ) {
        CControlUI* pControl = static_cast<CControlUI*>(GetItemAt(it));
        if( !pControl->IsVisible() ) continue;
        SIZE sz = pControl->EstimateSize(szAvailable);
        cyFixed += sz.cy;
		if( cxFixed < sz.cx )
			cxFixed = sz.cx;
    }
    return CSize(cxFixed, cyFixed);
}

void CMenuUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcscmp(pstrName, _T("shadow")) == 0) {
		if (_tcsstr(pstrValue, _T("true")) != NULL) {
			m_bShadow = true;
		}
	}

	CListUI::SetAttribute(pstrName, pstrValue);
}

/////////////////////////////////////////////////////////////////////////////////////
//
class CMenuBuilderCallback: public IDialogBuilderCallback
{
	CControlUI* CreateControl(LPCTSTR pstrClass)
	{
		if (_tcsicmp(pstrClass, kMenuUIInterfaceName) == 0)
		{
			return new CMenuUI();
		}
		else if (_tcsicmp(pstrClass, kMenuElementUIInterfaceName) == 0)
		{
			return new CMenuElementUI();
		}
		return NULL;
	}
};

//CStdString CMenuWnd::m_strResClickMenuName;

CMenuWnd::CMenuWnd(HWND hParent /* = NULL */, CStdString *strRes /* = NULL */):
m_hParent(hParent),
m_pOwner(NULL),
m_pLayout(),
m_xml(_T("")),
m_pSenderPaintManager(NULL)
{
	m_bShowMode = false;
	m_pStrResClickMenuName = strRes;
}

CMenuWnd::~CMenuWnd()
{
	if ((m_pOwner == NULL) && m_pStrResClickMenuName)
	{
		delete m_pStrResClickMenuName;
		m_pStrResClickMenuName = NULL;
	}
}

BOOL CMenuWnd::Receive(ContextMenuParam param)
{
	switch (param.wParam)
	{
	case 1:
		{
			Close();
		}
		break;
	case 2:
		{
			HWND hParent = GetParent(m_hWnd);
			while (hParent != NULL)
			{
				if (hParent == param.hWnd)
				{
					Close();
					break;
				}
				hParent = GetParent(hParent);
			}
		}
		break;
	default:
		break;
	}

	return TRUE;
}

void CMenuWnd::SetParentManager(CPaintManagerUI *pPaintManager)
{
	if (pPaintManager != NULL)
	{
		m_pSenderPaintManager = pPaintManager;
	}
}


//设置指定名称的菜单项是否显示
void CMenuWnd::SetItemVisible(CStdString strName, bool bVisible /* = true */)
{
	CMenuElementUI *pControl = static_cast<CMenuElementUI *>(m_pm.FindControl(strName));
	if (pControl != NULL)
	{
		pControl->SetVisible(bVisible);
	}
}

//设置某个单项的tooltip
void CMenuWnd::SetItemToolTips(CStdString strName, CStdString strToolTip)
{
	CMenuElementUI *pControl = static_cast<CMenuElementUI *>(m_pm.FindControl(strName));
	if (pControl != NULL)
	{
		pControl->SetToolTip(strToolTip);
	}
}

//设置某个单项是否选中
void CMenuWnd::CheckMenuItem(CStdString strName, bool bchecked /* = true */)
{
	CMenuElementUI *pControl = static_cast<CMenuElementUI *>(m_pm.FindControl(strName));
	if (pControl != NULL)
	{
		pControl->CheckItem(bchecked);
	}

}

//设置指定名称的菜单项的文本
void CMenuWnd::SetItemText(CStdString strName, CStdString strText)
{
	CControlUI *pControl = m_pm.FindControl(strName);
	if (pControl != NULL)
	{
		pControl->SetText(strText);
	}
}

//设置指定名称的菜单项的FagImage 来显示不同会员图标；
void CMenuWnd::SetItemFagImage(CStdString strName, CStdString strImage)
{
	auto pControl = static_cast<CMenuElementUI*>(m_pm.FindControl(strName));
	if (nullptr  != pControl)
	{
		pControl->SetFagImage(strImage.GetData());
	}
}

//设置指定名称的菜单项是否可用
void CMenuWnd::SetItemEnable(CStdString strName, bool bEnable /* = true */)
{
	CControlUI *pControl = m_pm.FindControl(strName);
	if (pControl != NULL)
	{
		pControl->SetEnabled(bEnable);
	}
}

void CMenuWnd::Init(CMenuElementUI* pOwner, STRINGorID xml, LPCTSTR pSkinType, POINT point)
{
	m_BasedPoint = point;
    m_pOwner = pOwner;
    m_pLayout = NULL;

	if (m_pOwner == NULL)
	{
		m_pStrResClickMenuName = NULL;
		m_pStrResClickMenuName = new CStdString();
	}

	if (pSkinType != NULL)
		m_sType = pSkinType;

	m_xml = xml;
	CStdString strXml = pSkinType;
	if (!strXml.IsEmpty())
	{
		if (m_pStrResClickMenuName)
		{
			m_pStrResClickMenuName->Empty();
		}
	}
	s_context_menu_observer.AddReceiver(this);

	Create((m_pOwner == NULL) ? m_hParent : m_pOwner->GetManager()->GetPaintWindow(), NULL, WS_POPUP, WS_EX_TOOLWINDOW | WS_EX_TOPMOST, CRect());
    // HACK: Don't deselect the parent's caption
    HWND hWndParent = m_hWnd;
    while( ::GetParent(hWndParent) != NULL ) hWndParent = ::GetParent(hWndParent);

//	m_Shadow.Create(m_hWnd);

//    ::ShowWindow(m_hWnd, SW_SHOW);
#if defined(WIN32) && !defined(UNDER_CE)
    ::SendMessage(hWndParent, WM_NCACTIVATE, TRUE, 0L);
#endif	
}

LPCTSTR CMenuWnd::GetWindowClassName() const
{
    return _T("MenuWnd");
}

void CMenuWnd::OnFinalMessage(HWND hWnd)
{
	RemoveObserver();
// 	m_pm.RemoveNotifier(this);
	m_pm.ReapObjects(m_pLayout);
	if( m_pOwner != NULL ) {
		for( int i = 0; i < m_pOwner->GetCount(); i++ ) {
			if( static_cast<CMenuElementUI*>(m_pOwner->GetItemAt(i)->GetInterface(kMenuElementUIInterfaceName)) != NULL ) {
				(static_cast<CMenuElementUI*>(m_pOwner->GetItemAt(i)))->SetOwner(m_pOwner->GetParent());
				(static_cast<CMenuElementUI*>(m_pOwner->GetItemAt(i)))->SetVisible(false);
				(static_cast<CMenuElementUI*>(m_pOwner->GetItemAt(i)->GetInterface(kMenuElementUIInterfaceName)))->SetInternVisible(false);
			}
		}
		m_pOwner->m_pWindow = NULL;
		m_pOwner->m_uButtonState &= ~ UISTATE_PUSHED;
		m_pOwner->Invalidate();
		
		if (m_pLayout && m_pLayout->GetHeader())
		{
			delete m_pLayout->GetHeader();
		}
		if (m_pLayout && m_pLayout->GetList())
		{
			m_pLayout->GetList()->SetAutoDestroy(false);
			delete m_pLayout->GetList();
		}

		delete this;   //自弹出窗口删除自己

	}
}

LRESULT CMenuWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if( uMsg == WM_CREATE ) {
		if( m_pOwner != NULL) {
			LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
			styleValue &= ~WS_CAPTION;
			::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
			RECT rcClient;
			::GetClientRect(*this, &rcClient);
			::SetWindowPos(*this, NULL, rcClient.left, rcClient.top, rcClient.right - rcClient.left, \
				rcClient.bottom - rcClient.top, SWP_FRAMECHANGED);

			m_pm.Init(m_hWnd);

			// The trick is to add the items to the new container. Their owner gets
			// reassigned by this operation - which is why it is important to reassign
			// the items back to the righfull owner/manager when the window closes.
 			m_pLayout = new CMenuUI();
			if (m_pLayout == NULL)
			{
				return 0;
			}
			m_pm.UseParentResource(m_pOwner->GetManager());
			m_pLayout->SetManager(&m_pm, NULL, true);
			m_pLayout->SetBkColor(0xFFFFFFFF);
			m_pLayout->SetBorderColor(0xFF85E4FF);
			m_pLayout->SetBorderSize(0);
			m_pLayout->SetAutoDestroy(false);
//			m_pLayout->EnableScrollBar();

			LPCTSTR pDefaultAttributes = m_pOwner->GetManager()->GetDefaultAttributeList(kMenuUIInterfaceName);
			if( pDefaultAttributes ) {
				m_pLayout->ApplyAttributeList(pDefaultAttributes);
			}
			if (m_pLayout->GetShadow())
			{
				CWndShadow::Initialize(CPaintManagerUI::GetInstance());
				m_Shadow.Create(m_hWnd);
				m_Shadow.SetSize(20);
				m_Shadow.SetSharpness(20);
				m_Shadow.SetDarkness(20);
				m_Shadow.SetPosition(5, 5);
				m_Shadow.SetColor(RGB(0, 0, 0));
			}

			for( int i = 0; i < m_pOwner->GetCount(); i++ ) {
				if(m_pOwner->GetItemAt(i)->GetInterface(kMenuElementUIInterfaceName) != NULL ){
					(static_cast<CMenuElementUI*>(m_pOwner->GetItemAt(i)))->SetOwner(m_pLayout);
					(static_cast<CMenuElementUI*>(m_pOwner->GetItemAt(i)))->SetSenderManager(m_pSenderPaintManager);
					(static_cast<CMenuElementUI*>(m_pOwner->GetItemAt(i)))->SetResReceiveString(m_pStrResClickMenuName);
					m_pLayout->Add(static_cast<CControlUI*>(m_pOwner->GetItemAt(i)));
				}
			}
			m_pm.AttachDialog(m_pLayout);
//			m_pm.SetUpdateTransparent();
			// Position the popup window in absolute space
			RECT rcOwner = m_pOwner->GetPos();
			RECT rc = rcOwner;

			int cxFixed = 0;
			int cyFixed = 0;

			MONITORINFO oMonitor = {}; 
			oMonitor.cbSize = sizeof(oMonitor);
//			::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTOPRIMARY), &oMonitor);
			if (m_hParent == NULL)
			{
				::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTOPRIMARY), &oMonitor);
			}
			else
			{
				HMONITOR hMonitor = MonitorFromWindow(m_hParent, MONITOR_DEFAULTTONEAREST);
				if (hMonitor != INVALID_HANDLE_VALUE)
				{
					GetMonitorInfo(hMonitor, &oMonitor);
				}
			}

			CRect rcWork = oMonitor.rcWork;

			SIZE szAvailable = { rcWork.right - rcWork.left, rcWork.bottom - rcWork.top };

			for( int it = 0; it < m_pOwner->GetCount(); it++ ) {
				if(m_pOwner->GetItemAt(it)->GetInterface(kMenuElementUIInterfaceName) != NULL ){
					CControlUI* pControl = static_cast<CControlUI*>(m_pOwner->GetItemAt(it));
					SIZE sz = pControl->EstimateSize(szAvailable);
					cyFixed += sz.cy;

					if( cxFixed < sz.cx )
						cxFixed = sz.cx;
				}
			}
			cyFixed += 4;
			cxFixed += 4;

			RECT rcWindow;
			GetWindowRect(m_pOwner->GetManager()->GetPaintWindow(), &rcWindow);

			rc.top = rcOwner.top;
			rc.bottom = rc.top + cyFixed;
			::MapWindowRect(m_pOwner->GetManager()->GetPaintWindow(), HWND_DESKTOP, &rc);
			rc.left = rcWindow.right;
			rc.right = rc.left + cxFixed;
			rc.right += 2;

			bool bReachBottom = false;
			bool bReachRight = false;
			LONG chRightAlgin = 0;
			LONG chBottomAlgin = 0;

			RECT rcPreWindow = {0};
			ContextMenuObserver::Iterator<BOOL, ContextMenuParam> iterator(s_context_menu_observer);
			ReceiverImplBase<BOOL, ContextMenuParam>* pReceiver = iterator.next();
			while( pReceiver != NULL ) {
				CMenuWnd* pContextMenu = dynamic_cast<CMenuWnd*>(pReceiver);
				if( pContextMenu != NULL ) {
					GetWindowRect(pContextMenu->GetHWND(), &rcPreWindow);

					bReachRight = rcPreWindow.left >= rcWindow.right;
					bReachBottom = rcPreWindow.top >= rcWindow.bottom;
					if( pContextMenu->GetHWND() == m_pOwner->GetManager()->GetPaintWindow() 
						||  bReachBottom || bReachRight )
						break;
				}
				pReceiver = iterator.next();
			}

			POINT pt;
			GetCursorPos(&pt);

			if (bReachBottom)
			{
//				rc.bottom = rcWindow.top;
				rc.bottom = pt.y;
				rc.top = rc.bottom - cyFixed;
			}

			if (bReachRight)
			{
				rc.right = rcWindow.left;
				rc.left = rc.right - cxFixed;
// 				POINT pt;
// 				GetCursorPos(&pt);
				if (pt.y  + cyFixed > rcWindow.bottom)
				{
//					rc.bottom = rcWindow.bottom;
					rc.bottom = pt.y;
					rc.top = rc.bottom - cyFixed;
				}
				else
				{
					rc.bottom = rc.top + cyFixed;
				}

			}

			if( rc.bottom > rcWork.bottom )
			{
// 				POINT pt;
// 				GetCursorPos(&pt);
				if (pt.y  + cyFixed > rcWindow.bottom)
				{
					rc.bottom = pt.y;
//					rc.bottom = rcWindow.bottom;
					rc.top = rc.bottom - cyFixed;
				}
				else
				{
					rc.bottom = rc.top + cyFixed;
				}
			}

			if (rc.right > rcWork.right)
			{

				rc.right = rcWindow.left;
				rc.left = rc.right - cxFixed;
// 				rc.top = rcWindow.bottom;
// 				rc.bottom = rc.top + cyFixed;
// 				rc.bottom = rcWindow.bottom;
// 				rc.top = rc.bottom - cyFixed;
// 				POINT pt;
// 				GetCursorPos(&pt);

				if (pt.y  + cyFixed > rcWindow.bottom)
				{
//					rc.bottom = rcWindow.bottom;
					rc.bottom = pt.y;
					rc.top = rc.bottom - cyFixed;
				}
				else
				{
					rc.bottom = rc.top + cyFixed;
				}

			}

			if( rc.top < rcWork.top )
			{
				rc.top = rcOwner.top;
				rc.bottom = rc.top + cyFixed;
			}

			if (rc.left < rcWork.left)
			{
				rc.left = rcWindow.right;
				rc.right = rc.left + cxFixed;
			}

			SetWindowPos(m_hWnd, HWND_TOPMOST, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_SHOWWINDOW);

			//MoveWindow(m_hWnd, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, FALSE);

//			AdjustPostion();
		}
		else {
			m_pm.Init(m_hWnd);

			CDialogBuilder builder;
			CMenuBuilderCallback menuCallback;
			
			CControlUI* pRoot = builder.Create(m_xml, m_sType.GetData(), &menuCallback, &m_pm);
			ASSERT(pRoot);
			m_pLayout = static_cast<CMenuUI *>(pRoot);
			if (m_pLayout == NULL)
			{
				return 0;
			}
			m_pm.AttachDialog(m_pLayout);
			if (m_pLayout->GetShadow())
			{
				CWndShadow::Initialize(CPaintManagerUI::GetInstance());
				m_Shadow.Create(m_hWnd);
				m_Shadow.SetSize(3);
				m_Shadow.SetSharpness(21);
				m_Shadow.SetDarkness(60);
				m_Shadow.SetPosition(0, 5);
				m_Shadow.SetColor(RGB(0, 0, 0));
			}
			for (int i = 0; i< m_pLayout->GetCount(); i++)
			{
				(static_cast<CMenuElementUI*>(m_pLayout->GetItemAt(i)))->SetSenderManager(m_pSenderPaintManager);
				(static_cast<CMenuElementUI*>(m_pLayout->GetItemAt(i)))->SetResReceiveString(m_pStrResClickMenuName);
			}
//			m_pm.SetUpdateTransparent();
// 			m_pm.AddNotifier(this);

#if defined(WIN32) && !defined(UNDER_CE)
			MONITORINFO oMonitor = {}; 
			oMonitor.cbSize = sizeof(oMonitor);
//			::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTOPRIMARY), &oMonitor);
			if (m_hParent == NULL)
			{
				::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTOPRIMARY), &oMonitor);
			}
			else
			{
				HMONITOR hMonitor = MonitorFromWindow(m_hParent, MONITOR_DEFAULTTONEAREST);
				if (hMonitor != INVALID_HANDLE_VALUE)
				{
					GetMonitorInfo(hMonitor, &oMonitor);
				}
			}
			CRect rcWork = oMonitor.rcWork;
#else
			CRect rcWork;
			GetWindowRect(m_pOwner->GetManager()->GetPaintWindow(), &rcWork);
#endif
			SIZE szAvailable = { rcWork.right - rcWork.left, rcWork.bottom - rcWork.top };
			szAvailable = pRoot->EstimateSize(szAvailable);
			m_pm.SetInitSize(szAvailable.cx, szAvailable.cy);

			DWORD dwAlignment = eMenuAlignment_Left | eMenuAlignment_Top;

			SIZE szInit = m_pm.GetInitSize();
			CRect rc;
			CPoint point = m_BasedPoint;
			rc.left = point.x;
			rc.top = point.y;
			rc.right = rc.left + szInit.cx;
			rc.bottom = rc.top + szInit.cy;

			int nWidth = rc.GetWidth();
			int nHeight = rc.GetHeight();

			if (dwAlignment & eMenuAlignment_Right)
			{
				rc.right = point.x;
				rc.left = rc.right - nWidth;
			}

			if (dwAlignment & eMenuAlignment_Bottom)
			{
				rc.bottom = point.y;
				rc.top = rc.bottom - nHeight;
			}

//			SetForegroundWindow(m_hWnd);
//			MoveWindow(m_hWnd, rc.left, rc.top, rc.GetWidth(), rc.GetHeight(), FALSE);
			SetWindowPos(m_hWnd, HWND_TOPMOST, rc.left, rc.top, rc.GetWidth(), rc.GetHeight(), SWP_SHOWWINDOW);
			AdjustPostion();

			SIZE szRoundCorner = m_pm.GetRoundCorner();
			if (!::IsIconic(m_hWnd) && (szRoundCorner.cx != 0 || szRoundCorner.cy != 0)) {
				CRect rcWnd;
				rcWnd = rc;
				rcWnd.Offset(-rcWnd.left, -rcWnd.top);
				rcWnd.right++; rcWnd.bottom++;

				szRoundCorner.cx += 2;
				HRGN hRgn2 = ::CreateRoundRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom /*- szRoundCorner.cx*/, szRoundCorner.cx, szRoundCorner.cy);
				::SetWindowRgn(m_hWnd, hRgn2, TRUE);
				::DeleteObject(hRgn2);
			}

		}

		return 0;
    }
    else if( uMsg == WM_CLOSE ) {
// 		CStdString str;
// 		str.Format(_T("The message type 4 is 0x%0x\n"), uMsg);
// 		OutputDebugStr(str);
		if( m_pOwner != NULL )
		{
			m_pOwner->SetManager(m_pOwner->GetManager(), m_pOwner->GetParent(), false);
			m_pOwner->SetPos(m_pOwner->GetPos());
			m_pOwner->SetFocus();
		}
	}
	else if( uMsg == WM_RBUTTONDOWN || uMsg == WM_CONTEXTMENU || uMsg == WM_RBUTTONUP || uMsg == WM_RBUTTONDBLCLK )
	{
// 		CStdString str;
// 		str.Format(_T("The message type 3 is 0x%0x\n"), uMsg);
// 		OutputDebugStr(str);
		return 0L;
	}
	else if( uMsg == WM_KILLFOCUS )
	{
// 		CStdString str;
// 		str.Format(_T("The message type 2 is 0x%0x\n"), uMsg);
// 		OutputDebugStr(str);
		HWND hFocusWnd = (HWND)wParam;

		BOOL bInMenuWindowList = FALSE;
		ContextMenuParam param;
		param.hWnd = GetHWND();

		ContextMenuObserver::Iterator<BOOL, ContextMenuParam> iterator(s_context_menu_observer);
		ReceiverImplBase<BOOL, ContextMenuParam>* pReceiver = iterator.next();
		while( pReceiver != NULL ) {
			CMenuWnd* pContextMenu = dynamic_cast<CMenuWnd*>(pReceiver);
			if( pContextMenu != NULL && pContextMenu->GetHWND() ==  hFocusWnd ) {
				bInMenuWindowList = TRUE;
				break;
			}
			pReceiver = iterator.next();
		}

		if( !bInMenuWindowList ) {
			param.wParam = 1;
			s_context_menu_observer.RBroadcast(param);

			return 0;
		}
	}
	else if( uMsg == WM_KEYDOWN)
	{
		if( wParam == VK_ESCAPE)
		{
			Close();
		}
	}
// 	else 
// 	{
// 		CStdString str;
// 		str.Format(_T("The message 1 type is %0x\n"), uMsg);
// 		OutputDebugStr(str);
// 	}

    LRESULT lRes = 0;
    if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;
    return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
}

void CMenuWnd::ResizeMenu()        //重新计算Menu的大小
{
	CControlUI *pRoot = m_pm.GetRoot();
	if (pRoot == NULL)
	{
		return;
	}

	MONITORINFO oMonitor = {}; 
	oMonitor.cbSize = sizeof(oMonitor);
//	::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTOPRIMARY), &oMonitor);
	if (m_hParent == NULL)
	{
		::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTOPRIMARY), &oMonitor);
	}
	else
	{
		HMONITOR hMonitor = MonitorFromWindow(m_hParent, MONITOR_DEFAULTTONEAREST);
		if (hMonitor != INVALID_HANDLE_VALUE)
		{
			GetMonitorInfo(hMonitor, &oMonitor);
		}
	}
	CRect rcWork = oMonitor.rcWork;
	SIZE szAvailable = { rcWork.right - rcWork.left, rcWork.bottom - rcWork.top };
	szAvailable = pRoot->EstimateSize(szAvailable);
	m_pm.SetInitSize(szAvailable.cx, szAvailable.cy);

	DWORD dwAlignment = eMenuAlignment_Left | eMenuAlignment_Top;

	SIZE szInit = m_pm.GetInitSize();
	CRect rc;
	CPoint point = m_BasedPoint;
	rc.left = point.x;
	rc.top = point.y;
	rc.right = rc.left + szInit.cx;
	rc.bottom = rc.top + szInit.cy;

	int nWidth = rc.GetWidth();
	int nHeight = rc.GetHeight();
	wstring wsConfig = _T("Menu\\mainMenu\\topMenu.xml");
	if (string::npos != wsConfig.find(m_xml.m_lpstr))
	{
		dwAlignment &= 0;
		dwAlignment = eMenuAlignment_Right | eMenuAlignment_Top;
		//调整下拉菜单宽度；2019.1.17
		nWidth = 250 > nWidth ? 250 : nWidth;
	}

	if (dwAlignment & eMenuAlignment_Right)
	{
		rc.right = point.x;
		rc.left = rc.right - nWidth;
	}

	if (dwAlignment & eMenuAlignment_Bottom)
	{
		rc.bottom = point.y;
		rc.top = rc.bottom - nHeight;
	}

	SetForegroundWindow(m_hWnd);
	//			MoveWindow(m_hWnd, rc.left, rc.top, rc.GetWidth(), rc.GetHeight(), FALSE);
	SetWindowPos(m_hWnd, HWND_TOPMOST, rc.left, rc.top, nWidth, rc.GetHeight(), SWP_SHOWWINDOW);
	AdjustPostion();

	SIZE szRoundCorner = m_pm.GetRoundCorner();
	if (!::IsIconic(m_hWnd) && (szRoundCorner.cx != 0 || szRoundCorner.cy != 0)) {
		CRect rcWnd;
		rcWnd = rc;
		rcWnd.Offset(-rcWnd.left, -rcWnd.top);
		rcWnd.right++; rcWnd.bottom++;

		szRoundCorner.cx += 2;
		HRGN hRgn2 = ::CreateRoundRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom /*- szRoundCorner.cx*/, szRoundCorner.cx, szRoundCorner.cy);
		::SetWindowRgn(m_hWnd, hRgn2, TRUE);
		::DeleteObject(hRgn2);
	}
}

CStdString CMenuWnd::DoModal()
{
	m_bShowMode = true;
	ASSERT(::IsWindow(m_hWnd));
	HWND hWndParent = GetWindowOwner(m_hWnd);
	::ShowWindow(m_hWnd, SW_SHOWNORMAL);
	::EnableWindow(hWndParent, FALSE);
	MSG msg = { 0 };
	while( ::IsWindow(m_hWnd) && ::GetMessage(&msg, NULL, 0, 0) ) {
		if( msg.message == WM_CLOSE && msg.hwnd == m_hWnd ) {
			::EnableWindow(hWndParent, TRUE);
			::SetFocus(hWndParent);
		}
		//if (msg.message == 0x118)
		//{
		//	Close();
		//}
		if( !CPaintManagerUI::TranslateMessage(&msg) ) {
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		if( msg.message == WM_QUIT ) break;
	}
	::EnableWindow(hWndParent, TRUE);
	::SetFocus(hWndParent);
	if( msg.message == WM_QUIT )
		::PostQuitMessage(msg.wParam);
	CStdString strResClickMenuName;
	if (m_pStrResClickMenuName)
	{
		strResClickMenuName = *m_pStrResClickMenuName;
		delete m_pStrResClickMenuName;
		m_pStrResClickMenuName = NULL;
	}

	return strResClickMenuName;
}

//计算位置
void CMenuWnd::AdjustPostion() {
	CRect rcWnd;
	GetWindowRect(m_hWnd, &rcWnd);
	int nWidth = rcWnd.GetWidth();
	int nHeight = rcWnd.GetHeight();
	rcWnd.left = m_BasedPoint.x;
	rcWnd.top = m_BasedPoint.y;
	rcWnd.right = rcWnd.left + nWidth;
	rcWnd.bottom = rcWnd.top + nHeight;
	wstring wsConfig = _T("Menu\\mainMenu\\topMenu.xml");//下拉菜单位置调整；2018.12.07
	if (string::npos != wsConfig.find(m_xml.m_lpstr))
	{
		//调整下拉菜单宽度；2019.1.17
		nWidth = 250 > nWidth ? 250 : nWidth;

		rcWnd.right = m_BasedPoint.x;
		rcWnd.top = m_BasedPoint.y;
		rcWnd.left = rcWnd.right - nWidth;
		rcWnd.bottom = rcWnd.top + nHeight;
	}
	MONITORINFO oMonitor = {};
	oMonitor.cbSize = sizeof(oMonitor);
//	::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTOPRIMARY), &oMonitor);
	if (m_hParent == NULL)
	{
		::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTOPRIMARY), &oMonitor);
	}
	else
	{
		HMONITOR hMonitor = MonitorFromWindow(m_hParent, MONITOR_DEFAULTTONEAREST);
		if (hMonitor != INVALID_HANDLE_VALUE)
		{
			GetMonitorInfo(hMonitor, &oMonitor);
		}
	}

	CRect rcWork = oMonitor.rcWork;

	if( rcWnd.bottom > rcWork.bottom ) {
		if( nHeight >= rcWork.GetHeight() ) {
			rcWnd.top = m_BasedPoint.y;
			rcWnd.bottom = rcWnd.top + nHeight;
		}
		else {
//			rcWnd.bottom = rcWork.bottom;
			rcWnd.bottom = m_BasedPoint.y;
			rcWnd.top = rcWnd.bottom - nHeight;
		}
	}
	if( rcWnd.right > rcWork.right ) {
		if( nWidth >= rcWork.GetWidth() ) {
			rcWnd.left = m_BasedPoint.x;
			rcWnd.right = rcWnd.left + nWidth;
//			rcWnd.left = 0;
//			rcWnd.right = nWidth;
		}
		else {
			rcWnd.right = m_BasedPoint.x;
//			rcWnd.right = rcWork.right;
			rcWnd.left = rcWnd.right - nWidth;
		}
	}
	::SetWindowPos(m_hWnd, NULL, rcWnd.left, rcWnd.top, rcWnd.GetWidth(), rcWnd.GetHeight(), SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
}

void CMenuWnd::SetPaintManDelegate(std::function<void(const CPaintManagerUI &)> a)
{
	a(m_pm);
}

/////////////////////////////////////////////////////////////////////////////////////
//

// MenuElementUI
const TCHAR* const kMenuElementUIClassName = _T("MenuElementUI");
const TCHAR* const kMenuElementUIInterfaceName = _T("MenuElement");

CMenuElementUI::CMenuElementUI():
m_pWindow(NULL)
{
	m_cxyFixed.cy = 25;
	m_bMouseChildEnabled = true;
	m_pSenderManager = NULL;
	m_bChecked = false;
	m_strResMenuName = NULL;
	SetMouseChildEnabled(false);
}

CMenuElementUI::~CMenuElementUI()
{
}

LPCTSTR CMenuElementUI::GetClass() const
{
	return kMenuElementUIClassName;
}

LPVOID CMenuElementUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcsicmp(pstrName, kMenuElementUIInterfaceName) == 0 ) return static_cast<CMenuElementUI*>(this);    
    return CListContainerElementUI::GetInterface(pstrName);
}

void CMenuElementUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
    if( !::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem) ) return;
	CMenuElementUI::DrawItemBk(hDC, m_rcItem);
	DrawItemText(hDC, m_rcItem);
	for (int i = 0; i < GetCount(); ++i)
	{
		if (GetItemAt(i)->GetInterface(kMenuElementUIInterfaceName) == NULL)
			GetItemAt(i)->DoPaint(hDC, rcPaint);
	}

	if (m_bChecked && !m_strCheckedImage.IsEmpty())
	{
		if (!DrawImage(hDC, (LPCTSTR)m_strCheckedImage))
		{
			m_strCheckedImage.Empty();
		}
		else
			return;
	}
	if ((m_uButtonState & UISTATE_HOT) != 0 && !m_strHotImage.IsEmpty()) {
		if (!DrawImage(hDC, (LPCTSTR)m_strHotImage))
		{
			m_strHotImage.Empty();
		}
		else
			return;
	}
	if (!m_strNormalImage.IsEmpty())
	{
		if (!DrawImage(hDC, (LPCTSTR)m_strNormalImage))
			m_strNormalImage.Empty();
	}
	if (!m_strFagImage.IsEmpty())
	{
		if( !DrawImage(hDC, (LPCTSTR)m_strFagImage) )
			m_strFagImage.Empty();
	}


}

void CMenuElementUI::DrawItemBk(HDC hDC, const RECT& rcItem)
{
	ASSERT(m_pOwner);
	if( m_pOwner == NULL ) return;
	TListInfoUI* pInfo = m_pOwner->GetListInfo();
	DWORD iBackColor = 0;
	iBackColor = pInfo->dwBkColor;

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
		if(! pInfo->sSelectedImage.IsEmpty() ) {
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
		RECT rcLine = { m_rcItem.left, m_rcItem.bottom - 1, m_rcItem.right, m_rcItem.bottom - 1 };
		CRenderEngine::DrawLine(hDC, rcLine, 1, GetAdjustColor(pInfo->dwLineColor));
	}
}

void CMenuElementUI::DrawItemText(HDC hDC, const RECT& rcItem)
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

    if( pInfo->bShowHtml )
        CRenderEngine::DrawHtmlText(hDC, m_pManager, rcText, m_sText, iTextColor, \
        NULL, NULL, nLinks, DT_SINGLELINE | pInfo->uTextStyle);
    else
        CRenderEngine::DrawText(hDC, m_pManager, rcText, m_sText, iTextColor, \
        pInfo->nFont, DT_SINGLELINE | pInfo->uTextStyle);
}


SIZE CMenuElementUI::EstimateSize(SIZE szAvailable)
{
	SIZE cXY = {0};
	for( int it = 0; it < GetCount(); it++ ) {
		CControlUI* pControl = static_cast<CControlUI*>(GetItemAt(it));
		if( !pControl->IsVisible() ) continue;
		SIZE sz = pControl->EstimateSize(szAvailable);
		cXY.cy += sz.cy;
		if( cXY.cx < sz.cx )
			cXY.cx = sz.cx;
	}
	if(cXY.cy == 0) {
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

		RECT rcText = { 0, 0, MAX(szAvailable.cx, m_cxyFixed.cx), 9999 };
		rcText.left += pInfo->rcTextPadding.left;
		rcText.right -= pInfo->rcTextPadding.right;
		if( pInfo->bShowHtml ) {   
			int nLinks = 0;
			CRenderEngine::DrawHtmlText(m_pManager->GetPaintDC(), m_pManager, rcText, m_sText, iTextColor, NULL, NULL, nLinks, DT_CALCRECT | pInfo->uTextStyle);
		}
		else {
			CRenderEngine::DrawText(m_pManager->GetPaintDC(), m_pManager, rcText, m_sText, iTextColor, pInfo->nFont, DT_CALCRECT | pInfo->uTextStyle);
		}
		cXY.cx = rcText.right - rcText.left + pInfo->rcTextPadding.left + pInfo->rcTextPadding.right + 20;
		cXY.cy = rcText.bottom - rcText.top + pInfo->rcTextPadding.top + pInfo->rcTextPadding.bottom;
	}

	if( m_cxyFixed.cy != 0 ) cXY.cy = m_cxyFixed.cy;
	if (m_cxyFixed.cx != 0) cXY.cx = m_cxyFixed.cx;
	return cXY;
}

void CMenuElementUI::DoEvent(TEventUI& event)
{
	if( event.Type == UIEVENT_MOUSEENTER )
	{
		CListContainerElementUI::DoEvent(event);
		if( m_pWindow ) return;
		bool hasSubMenu = false;
		for( int i = 0; i < GetCount(); ++i )
		{
			if( GetItemAt(i)->GetInterface(kMenuElementUIInterfaceName) != NULL )
			{
				(static_cast<CMenuElementUI*>(GetItemAt(i)->GetInterface(kMenuElementUIInterfaceName)))->SetVisible(true);
				(static_cast<CMenuElementUI*>(GetItemAt(i)->GetInterface(kMenuElementUIInterfaceName)))->SetInternVisible(true);
				(static_cast<CMenuElementUI*>(GetItemAt(i)->GetInterface(kMenuElementUIInterfaceName)))->SetSenderManager(m_pSenderManager);
				(static_cast<CMenuElementUI*>(GetItemAt(i)->GetInterface(kMenuElementUIInterfaceName)))->SetResReceiveString(m_strResMenuName);
				hasSubMenu = true;
			}
		}
		if( hasSubMenu )
		{
//			m_pOwner->SelectItem(GetIndex(), true);
			CreateMenuWnd();
		}
		else
		{
			ContextMenuParam param;
			param.hWnd = m_pManager->GetPaintWindow();
			param.wParam = 2;
			s_context_menu_observer.RBroadcast(param);
// 			if (m_pOwner)
// 			{
// 				m_pOwner->GetManager()->SendNotify(this, _T("itemclick"), DUILIB_LIST_ITEMCLICK);
// 			}
		}
		return;
	}

	if( event.Type == UIEVENT_BUTTONDOWN )
	{
		if( IsEnabled() ){
			bool hasSubMenu = false;
			for( int i = 0; i < GetCount(); ++i ) {
				if( GetItemAt(i)->GetInterface(kMenuElementUIInterfaceName) != NULL ) {
					(static_cast<CMenuElementUI*>(GetItemAt(i)->GetInterface(kMenuElementUIInterfaceName)))->SetVisible(true);
					(static_cast<CMenuElementUI*>(GetItemAt(i)->GetInterface(kMenuElementUIInterfaceName)))->SetInternVisible(true);

					hasSubMenu = true;
				}
			}
			if (!hasSubMenu)
			{
				CListContainerElementUI::DoEvent(event);
			}

			if( m_pWindow ) return;

			if( hasSubMenu )
			{
				CreateMenuWnd();
			}
			else
			{
				ContextMenuParam param;
				param.hWnd = m_pManager->GetPaintWindow();
				param.wParam = 1;
				s_context_menu_observer.RBroadcast(param);
				
				if (m_items.GetSize() > 0)
				{
					return;
				}
				if (m_strResMenuName != NULL)
				{
					*m_strResMenuName = GetName();
				}
				if (m_pSenderManager)
				{
					m_pSenderManager->SendNotify(NULL, _T("menuclick"), DUILIB_MENU_CLICK, (WPARAM)(LPCTSTR)GetName(), 0, false);
				}
// 				else if (m_pManager)
// 				{
// 					m_pManager->SendNotify(NULL, _T("menuclick"), DUILIB_MENU_CLICK, (WPARAM)(LPCTSTR)GetName());
// 				}
				m_pOwner->SelectItem(GetIndex(), true);
			}
        }
        return;
    }

    CListContainerElementUI::DoEvent(event);
}

bool CMenuElementUI::Activate()
{
	if (CListContainerElementUI::Activate() && m_bSelected)
	{
		if( m_pWindow ) return true;
		bool hasSubMenu = false;
		for (int i = 0; i < GetCount(); ++i)
		{
			if (GetItemAt(i)->GetInterface(kMenuElementUIInterfaceName) != NULL)
			{
				(static_cast<CMenuElementUI*>(GetItemAt(i)->GetInterface(kMenuElementUIInterfaceName)))->SetVisible(true);
				(static_cast<CMenuElementUI*>(GetItemAt(i)->GetInterface(kMenuElementUIInterfaceName)))->SetInternVisible(true);

				hasSubMenu = true;
			}
		}
		if (hasSubMenu)
		{
			CreateMenuWnd();
		}
		else
		{
			ContextMenuParam param;
			param.hWnd = m_pManager->GetPaintWindow();
			param.wParam = 1;
			s_context_menu_observer.RBroadcast(param);
		}

		return true;
	}
	return false;
}

CMenuWnd* CMenuElementUI::GetMenuWnd()
{
	return m_pWindow;
}

void CMenuElementUI::CreateMenuWnd()
{
	if( m_pWindow ) return;

	m_pWindow = new CMenuWnd(m_pManager->GetPaintWindow(), m_strResMenuName);
	ASSERT(m_pWindow);
	if (m_pSenderManager != NULL)
	{
		m_pWindow->SetParentManager(m_pSenderManager);
	}
	else
		m_pWindow->SetParentManager(m_pManager);
//	m_pManager->SetUpdateTransparent();
	ContextMenuParam param;
	param.hWnd = m_pManager->GetPaintWindow();
	param.wParam = 2;
	s_context_menu_observer.RBroadcast(param);

	m_pWindow->Init(static_cast<CMenuElementUI*>(this), _T(""), _T(""), CPoint());
	m_pWindow->ShowWindow(true);
}

//设置消息发送的接受句柄
void CMenuElementUI::SetSenderManager(CPaintManagerUI *pPaintManager)
{
	if (pPaintManager != NULL)
	{
		m_pSenderManager = pPaintManager;
	}
}

//设置阻塞模式下，返回的菜单名
void CMenuElementUI::SetResReceiveString(CStdString *strRes)  
{
	if (strRes != NULL)
	{
		m_strResMenuName = strRes;
	}
}

void CMenuElementUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcscmp(pstrName, _T("FagImage")) == 0)
	{
		m_strFagImage = pstrValue;
	}
	else if (_tcscmp(pstrName, _T("checkedimage")) == 0)
	{
		m_strCheckedImage = pstrValue;
	}
	else if (_tcscmp(pstrName, _T("normalimage")) == 0)
	{
		SetNormalImage(pstrValue);
	}
	else if (_tcscmp(pstrName, _T("hotimage")) == 0)
	{
		SetHotImage(pstrValue);
	}
	CListContainerElementUI::SetAttribute(pstrName, pstrValue);
}

void CMenuElementUI::CheckItem(bool bCheck /* = true */)
{
	m_bChecked = bCheck;
}

void CMenuElementUI::SetNormalImage(LPCTSTR pStrImage)
{
	m_strNormalImage = pStrImage;
}

LPCTSTR CMenuElementUI::GetNormalImage()
{
	return m_strNormalImage;
}

void CMenuElementUI::SetHotImage(LPCTSTR pStrImage)
{
	m_strHotImage = pStrImage;
}

LPCTSTR CMenuElementUI::GetHotImage()
{
	return m_strHotImage;
}

void CMenuElementUI::SetFagImage(LPCTSTR pStrImage)
{
	m_strFagImage = pStrImage;
}

LPCTSTR CMenuElementUI::GetFagImage()
{
	return m_strFagImage;
}

} // namespace DuiLib
