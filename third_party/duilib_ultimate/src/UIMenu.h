#ifndef __UIMENU_H__
#define __UIMENU_H__

#ifdef _MSC_VER
#pragma once
#endif

#include "observer_impl_base.hpp"
#include "WndShadow.h"
#include <functional>

namespace DuiLib {

/////////////////////////////////////////////////////////////////////////////////////
//
struct ContextMenuParam
{
	// 1: remove all
	// 2: remove the sub menu
	WPARAM wParam;
	HWND hWnd;
};

enum MenuAlignment
{
	eMenuAlignment_Left = 1 << 1,
	eMenuAlignment_Top = 1 << 2,
	eMenuAlignment_Right = 1 << 3,
	eMenuAlignment_Bottom = 1 << 4,
};

typedef class ObserverImpl<BOOL, ContextMenuParam> ContextMenuObserver;
typedef class ReceiverImpl<BOOL, ContextMenuParam> ContextMenuReceiver;

extern ContextMenuObserver s_context_menu_observer;

// MenuUI
extern const TCHAR* const kMenuUIClassName;// = _T("MenuUI");
extern const TCHAR* const kMenuUIInterfaceName;// = _T("Menu");

class CListUI;
class UILIB_API CMenuUI : public CListUI
{
public:
	CMenuUI();
	~CMenuUI();

    LPCTSTR GetClass() const;
    LPVOID GetInterface(LPCTSTR pstrName);

	virtual void DoEvent(TEventUI& event);

    virtual bool Add(CControlUI* pControl);
    virtual bool AddAt(CControlUI* pControl, int iIndex);

    virtual int GetItemIndex(CControlUI* pControl) const;
    virtual bool SetItemIndex(CControlUI* pControl, int iIndex);
    virtual bool Remove(CControlUI* pControl);

	virtual bool GetShadow();
	

	SIZE EstimateSize(SIZE szAvailable);

	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
private:
	bool m_bShadow = false;
};

/////////////////////////////////////////////////////////////////////////////////////
//

// MenuElementUI
extern const TCHAR* const kMenuElementUIClassName;// = _T("MenuElementUI");
extern const TCHAR* const kMenuElementUIInterfaceName;// = _T("MenuElement);

class CMenuElementUI;

class UILIB_API CMenuWnd : public CWindowWnd, public ContextMenuReceiver
{
public:
	CMenuWnd(HWND hParent = NULL, CStdString *strRes = NULL);
	virtual ~CMenuWnd();
    void Init(CMenuElementUI* pOwner, STRINGorID xml, LPCTSTR pSkinType, POINT point);
    LPCTSTR GetWindowClassName() const;
	void OnFinalMessage(HWND hWnd);

    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

	BOOL Receive(ContextMenuParam param);

	void SetParentManager(CPaintManagerUI *pPaintManager);
	
	void SetItemVisible(CStdString strName, bool bVisible = true);   //设置指定名称的菜单项是否显示
	void SetItemText(CStdString strName, CStdString strText);   //设置指定名称的菜单项的文本
	void SetItemFagImage(CStdString strName, CStdString strImage);   //设置指定名称的菜单项的FagImage;2018.12.04
	void SetItemEnable(CStdString strName, bool bEnable = true);   //设置指定名称的菜单项是否可用
	void SetItemToolTips(CStdString strName, CStdString strToolTip);  //设置某个单项的tooltip
	void CheckMenuItem(CStdString strName, bool bchecked = true);  //设置某个单项是否选中
	void SetPaintManDelegate(std::function<void(const CPaintManagerUI &)> a);		//界面库内设置paintmanager；2019.1.30

	void ResizeMenu();        //重新计算Menu的大小

	CStdString DoModal();   //阻塞式显示菜单

protected:
	void AdjustPostion();
public:
	HWND m_hParent;
	POINT m_BasedPoint;
	STRINGorID m_xml;
	CStdString m_sType;
    CPaintManagerUI m_pm;
    CMenuElementUI* m_pOwner;
    CMenuUI* m_pLayout;
	CPaintManagerUI *m_pSenderPaintManager;
	CWndShadow m_Shadow;
	CStdString *m_pStrResClickMenuName;
	bool m_bShowMode;             //显示模式
};

class CListContainerElementUI;
class UILIB_API CMenuElementUI : public CListContainerElementUI
{
	friend CMenuWnd;
public:
    CMenuElementUI();
	~CMenuElementUI();

    LPCTSTR GetClass() const;
    LPVOID GetInterface(LPCTSTR pstrName);

	void SetNormalImage(LPCTSTR pStrImage);
	LPCTSTR GetNormalImage();

	void SetHotImage(LPCTSTR pStrImage);
	LPCTSTR GetHotImage();

	void SetFagImage(LPCTSTR pStrImage);
	LPCTSTR GetFagImage();
	
	void DoPaint(HDC hDC, const RECT& rcPaint);

	void DrawItemText(HDC hDC, const RECT& rcItem);

    void DrawItemBk(HDC hDC, const RECT& rcItem);

	SIZE EstimateSize(SIZE szAvailable);

	bool Activate();

	void DoEvent(TEventUI& event);

	CMenuWnd* GetMenuWnd();

	void CreateMenuWnd();

	void SetSenderManager(CPaintManagerUI *pPaintManager);  //设置消息发送的接受句柄

	void SetResReceiveString(CStdString *strRes);  //设置阻塞模式下，返回的菜单名

	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

	void CheckItem(bool bCheck = true);

protected:
	CMenuWnd* m_pWindow;
	CPaintManagerUI *m_pSenderManager;
	CStdString *m_strResMenuName;
	CStdString m_strFagImage;
	CStdString m_strCheckedImage;
	CStdString m_strNormalImage;
	CStdString m_strHotImage;
	bool m_bChecked;
};

} // namespace DuiLib

#endif // __UIMENU_H__
