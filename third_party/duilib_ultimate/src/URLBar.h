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

	int GetHidenItems() const;   //��ȡ��Ҫ���ص���Ŀ��
	void CalHideItems();         //������Ҫ���ص���Ŀ��
	int GetItemNeedLen(int nIndex); 	//��ȡָ��������ʾ��Ҫ�ĳ���
	void SetItemNeedLen(int nIndex, int nLen); //��ȡָ��������ʾ��Ҫ�ĳ���
	
	void SetArrowBtnNormalImage(CStdString strNormalImage);   //����ָ��ť��ͨ״̬
	CStdString GetArrowBtnNormalImage() const;                //��ȡָ��ť��ͨ״̬
	void SetArrowBtnSize(SIZE szBtn);                         //����ָ��ť��С
	SIZE GetArrowBtnSize() const;                             //��ȡָ��ť��С
	UINT GetArrowWidth() const;

	void SetDropBtnSize(SIZE szBtn);                          //����������ť�Ĵ�С 
	SIZE GetDropBtnSize() const;                              //��ȡ������ť�Ĵ�С 
	
	void SetItemHeight(int nHeight);           //������Ŀ�߶�
	int GetItemHeight() const;                 //��ȡ��Ŀ�߶�
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
	int  m_nHidenItems;          //�ܹ���ʾ����
	UINT m_uArrowWith;           //ָ��ͼ��Ŀ��
	UINT m_uDropBtnWidth;        //������ť�Ŀ��
	bool m_bShowDropBtn;         //�Ƿ���ʾ������ť
	CRect m_rcDropBtn;           //������ť������
	SIZE m_DropBtnSize;          //������ť��С 

	SIZE m_ArrowBtnSize;                    //��ͷ��ť��С
	CStdString m_strImageArrowBtn_Normal;   //��ͷ��ť��ͨ״̬ͼ��
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

	//	void Invalidate(); // ֱ��CControl::Invalidate�ᵼ�¹�����ˢ�£���д����ˢ������
	bool Activate();

	void DoEvent(TEventUI& event);
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	void DoPaint(HDC hDC, const RECT& rcPaint);

	void DrawItemText(HDC hDC, const RECT& rcItem);    
	void DrawItemBk(HDC hDC, const RECT& rcItem);

	UINT GetControlNeedLen();           //��ȡ�ı�����
	void SetControlNeedLen(int nLen);   //���ÿؼ���Ҫ�ĳ���

protected:
	int m_iIndex;
	bool m_bSelected;
	CURLBar* m_pOwner;

	UINT m_uTextNeedLen;      //��ʾ�ı���Ҫ�ĳ���  
	UINT m_uControlNeedLen;   //�ؼ���Ҫ�ĳ���
	CRect m_rcBtnPos;
	UINT m_uButtonState;

	bool m_bTextChanged;
};

}
#endif
