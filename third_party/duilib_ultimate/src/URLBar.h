#ifndef _URL_BAR_HEADER__
#define _URL_BAR_HEADER__
#pragma once
namespace DuiLib {

class CURLBar;

class UILIB_API CComboWndEx : public CWindowWnd
{
public:
	void Init(CURLBar* pOwner);
	LPCTSTR GetWindowClassName() const;
	void OnFinalMessage(HWND hWnd);

	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

	void EnsureVisible(int iIndex);
	void Scroll(int dx, int dy);

public:
	CPaintManagerUI m_pm;
	CURLBar* m_pOwner;
	CVerticalLayoutUI* m_pLayout;
	int m_iOldSel;
};

class UILIB_API CURLBar: public CHorizontalLayoutUI, public IListOwnerUI
{
	friend CComboWndEx;
public:
	CURLBar(void);
	virtual ~CURLBar(void);

	LPCTSTR GetClass() const;
	UINT GetControlFlags() const;
	LPVOID GetInterface(LPCTSTR pstrName);

	CStdString GetText() const;
	void SetEnabled(bool bEnable = true);

	CStdString GetDropBoxAttributeList();
	void SetDropBoxAttributeList(LPCTSTR pstrList);
	SIZE GetDropBoxSize() const;
	void SetDropBoxSize(SIZE szDropBox);

	int GetCurSel() const;  
	bool SelectItem(int iIndex, bool bTakeFocus = false);

	bool SetItemIndex(CControlUI* pControl, int iIndex);
	bool AddItem(CStdString strText);
	bool Add(CControlUI* pControl);
	bool AddAt(CControlUI* pControl, int iIndex);
	bool Remove(CControlUI* pControl);
	bool RemoveAt(int iIndex);
	void RemoveAll();

	bool Activate();

	RECT GetTextPadding() const;
	void SetTextPadding(RECT rc);
	LPCTSTR GetNormalImage() const;
	void SetNormalImage(LPCTSTR pStrImage);
	LPCTSTR GetHotImage() const;
	void SetHotImage(LPCTSTR pStrImage);
	LPCTSTR GetPushedImage() const;
	void SetPushedImage(LPCTSTR pStrImage);
	LPCTSTR GetFocusedImage() const;
	void SetFocusedImage(LPCTSTR pStrImage);
	LPCTSTR GetDisabledImage() const;
	void SetDisabledImage(LPCTSTR pStrImage);

	TListInfoUI* GetListInfo();
	void SetItemFont(int index);
	void SetItemTextStyle(UINT uStyle);
	RECT GetItemTextPadding() const;
	void SetItemTextPadding(RECT rc);
	DWORD GetItemTextColor() const;
	void SetItemTextColor(DWORD dwTextColor);
	DWORD GetItemBkColor() const;
	void SetItemBkColor(DWORD dwBkColor);
	LPCTSTR GetItemBkImage() const;
	void SetItemBkImage(LPCTSTR pStrImage);
	bool IsAlternateBk() const;
	void SetAlternateBk(bool bAlternateBk);
	DWORD GetSelectedItemTextColor() const;
	void SetSelectedItemTextColor(DWORD dwTextColor);
	DWORD GetSelectedItemBkColor() const;
	void SetSelectedItemBkColor(DWORD dwBkColor);
	LPCTSTR GetSelectedItemImage() const;
	void SetSelectedItemImage(LPCTSTR pStrImage);
	DWORD GetHotItemTextColor() const;
	void SetHotItemTextColor(DWORD dwTextColor);
	DWORD GetHotItemBkColor() const;
	void SetHotItemBkColor(DWORD dwBkColor);
	LPCTSTR GetHotItemImage() const;
	void SetHotItemImage(LPCTSTR pStrImage);
	DWORD GetDisabledItemTextColor() const;
	void SetDisabledItemTextColor(DWORD dwTextColor);
	DWORD GetDisabledItemBkColor() const;
	void SetDisabledItemBkColor(DWORD dwBkColor);
	LPCTSTR GetDisabledItemImage() const;
	void SetDisabledItemImage(LPCTSTR pStrImage);
	DWORD GetItemLineColor() const;
	void SetItemLineColor(DWORD dwLineColor);
	bool IsItemShowHtml();
	void SetItemShowHtml(bool bShowHtml = true);

	SIZE EstimateSize(SIZE szAvailable);
	void SetPos(RECT rc);
	void DoEvent(TEventUI& event);
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

	void DoPaint(HDC hDC, const RECT& rcPaint);
	void PaintText(HDC hDC);
	void PaintStatusImage(HDC hDC);

	int GetHidenItems() const;   //获取需要隐藏的项目数
	void CalHideItems();         //计算需要隐藏的项目数
	int GetItemNeedLen(int nIndex); 	//获取指定的项显示需要的长度
	void SetItemNeedLen(int nIndex, int nLen); //获取指定的项显示需要的长度
	
	void SetArrowBtnNormalImage(CStdString strNormalImage);   //设置指向按钮普通状态
	CStdString GetArrowBtnNormalImage() const;                //获取指向按钮普通状态
	void SetArrowBtnSize(SIZE szBtn);                         //设置指向按钮大小
	SIZE GetArrowBtnSize() const;                             //获取指向按钮大小
	UINT GetArrowWidth() const;

	void SetDropBtnSize(SIZE szBtn);                          //设置下拉按钮的大小 
	SIZE GetDropBtnSize() const;                              //获取下拉按钮的大小 
	
	void SetItemHeight(int nHeight);           //设置项目高度
	int GetItemHeight() const;                 //获取项目高度
protected:
	CComboWndEx* m_pWindow;

	int m_iCurSel;
	RECT m_rcTextPadding;
	CStdString m_sDropBoxAttributes;
	SIZE m_szDropBox;
	UINT m_uButtonState;

	CStdString m_sNormalImage;
	CStdString m_sHotImage;
	CStdString m_sPushedImage;
	CStdString m_sFocusedImage;
	CStdString m_sDisabledImage;

	TListInfoUI m_ListInfo;
	int  m_nHidenItems;          //能够显示的项
	UINT m_uArrowWith;           //指向图标的宽度
	UINT m_uDropBtnWidth;        //下拉按钮的宽度
	bool m_bShowDropBtn;         //是否显示下来按钮
	CRect m_rcDropBtn;           //下拉按钮的区域
	SIZE m_DropBtnSize;          //下拉按钮大小 

	SIZE m_ArrowBtnSize;                    //箭头按钮大小
	CStdString m_strImageArrowBtn_Normal;   //箭头按钮普通状态图标
	int m_nItemHeight;
};

class UILIB_API CURLElement : public CButtonUI, public IListItemUI
{
public:
	CURLElement();
	LPCTSTR GetClass() const;
	LPVOID GetInterface(LPCTSTR pstrName);
	UINT GetControlFlags() const;

	SIZE EstimateSize(SIZE szAvailable);
	void SetPos(RECT rc);

	int GetIndex() const;
	void SetIndex(int iIndex);
	virtual void SetText(LPCTSTR pstrText);

	CURLBar* GetOwner();
	void SetOwner(CControlUI* pOwner);
	void SetVisible(bool bVisible = true);
	void SetEnabled(bool bEnable = true);

	bool IsSelected() const;
	bool Select(bool bSelect = true, bool bInvalidate = true);
	bool IsExpanded() const;
	bool Expand(bool bExpand = true);

	//	void Invalidate(); // 直接CControl::Invalidate会导致滚动条刷新，重写减少刷新区域
	bool Activate();

	void DoEvent(TEventUI& event);
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	void DoPaint(HDC hDC, const RECT& rcPaint);

	void DrawItemText(HDC hDC, const RECT& rcItem);    
	void DrawItemBk(HDC hDC, const RECT& rcItem);

	UINT GetControlNeedLen();           //获取文本长度
	void SetControlNeedLen(int nLen);   //设置控件需要的长度

protected:
	int m_iIndex;
	bool m_bSelected;
	CURLBar* m_pOwner;

	UINT m_uTextNeedLen;      //显示文本需要的长度  
	UINT m_uControlNeedLen;   //控件需要的长度
	CRect m_rcBtnPos;
	UINT m_uButtonState;

	bool m_bTextChanged;
};

}
#endif
