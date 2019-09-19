#ifndef  _COMBOUI_EX_HEADER_
#define _COMBOUI_EX_HEADER_

#pragma once

namespace DuiLib {

#define  DUILIB_COMBO_ITMEDELETE 0x22

class CComboElementUI;
class IComboExUI
{
public:
	virtual bool ClickItem(int iIndex, bool bTakeFocus = false) = 0;
	virtual bool CheckItem(int iIndex, bool bTakeFocus = false) = 0;
	virtual bool RemoveItem(int iIndex) = 0;
	virtual void SetBtnOffset(RECT rc) = 0;   //����Btnƫ�������ұߵ��������½�ƫ����
	virtual RECT GetBtnOffset() = 0;

	virtual SIZE GetBtnSize() = 0;      //��ť��С

	virtual void SetBtnNormalImage(LPCTSTR lpstr) = 0;
	virtual LPCTSTR GetBtnNormalImage() = 0;
	virtual void SetBtnHotImage(LPCTSTR lpstr) = 0;
	virtual LPCTSTR GetBtnHotImage() = 0;
	virtual void SetBtnPushedImage(LPCTSTR lpstr) = 0;
	virtual LPCTSTR GetBtnPushedImage() = 0;

	//��ʾ
	virtual void SetIconOffset(RECT rc) = 0;   //����Iconƫ�������ұߵ��������½�ƫ����
	virtual RECT GetIconOffset() = 0;

	virtual SIZE GetIconSize() = 0;      //��ť��С

	virtual void SetIconImage(LPCTSTR lpstr) = 0;
	virtual LPCTSTR GetIconImage() = 0;
	virtual void SetDescIconImage(LPCTSTR lpstr) = 0;
	virtual LPCTSTR GetDescIconImage() = 0;


};
class UILIB_API CComboUIEx :public CComboUI, public IComboExUI
{
public:
	CComboUIEx(void);
	~CComboUIEx(void);

	LPCTSTR GetClass() const;
	UINT GetControlFlags() const;
	LPVOID GetInterface(LPCTSTR pstrName);

	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

	virtual bool ClickItem(int iIndex, bool bTakeFocus = false);
	virtual bool CheckItem(int iIndex, bool bTakeFocus = false);
	virtual bool RemoveItem(int iIndex);

	void SetBtnOffset(RECT rc);   //����Btnƫ�������ұߵ��������½�ƫ����
	RECT GetBtnOffset();

	SIZE GetBtnSize();      //��ť��С
	
	void SetBtnNormalImage(LPCTSTR lpstr);
	LPCTSTR GetBtnNormalImage();
	void SetBtnHotImage(LPCTSTR lpstr);
	LPCTSTR GetBtnHotImage();
	void SetBtnPushedImage(LPCTSTR lpstr);
	LPCTSTR GetBtnPushedImage();

	//��ʾ
	void SetIconOffset(RECT rc);   //����Iconƫ�������ұߵ��������½�ƫ����
	RECT GetIconOffset();

	SIZE GetIconSize();      //��ť��С

	void SetIconImage(LPCTSTR lpstr);
	LPCTSTR GetIconImage();
	void SetDescIconImage(LPCTSTR lpstr);
	LPCTSTR GetDescIconImage();

	void PaintText(HDC hDC);

	//����
	void SetTextStyle(UINT uStyle);
	UINT GetTextStyle() const;
	void SetTextColor(DWORD dwTextColor);
	DWORD GetTextColor() const;
	void SetDisabledTextColor(DWORD dwTextColor);
	DWORD GetDisabledTextColor() const;
	void SetFont(int index);
	int GetFont() const;
	RECT GetTextPadding() const;
	void SetTextPadding(RECT rc);
	bool IsShowHtml();
	void SetShowHtml(bool bShowHtml = true);

	void SetItemShowIcon(int nIndex);
	void DoEvent(TEventUI& event);
	void DoPaint(HDC hDC, const RECT& rcPaint);
	void DoPostPaint(HDC hDC, const RECT& rcPaint);


protected:
	RECT m_rcBtnPos;
	RECT m_rcBtnOffset;
	SIZE m_szBtn;

	CStdString m_sBtnNormalImage;
	CStdString m_sBtnHotImage;
	CStdString m_sBtnPushedImage;
	int m_nLastSel;

	DWORD m_dwTextColor;
	DWORD m_dwDisabledTextColor;
	int m_iFont;
	UINT m_uTextStyle;
	RECT m_rcTextPadding;
	bool m_bShowHtml;


	CStdString m_sIconImage;         //���ͼ��
	CStdString m_sDescIconImage;     //������ͼ��

	RECT m_rcIconOffset;
	SIZE m_szIcon;
	bool m_bShowIcon;
	bool m_bDelayHover;   // ����״̬��ʱ����

};

class UILIB_API CComboElementUI : public CControlUI, public IListItemUI
{
public:
	CComboElementUI();

	LPCTSTR GetClass() const;
	UINT GetControlFlags() const;
	LPVOID GetInterface(LPCTSTR pstrName);

	int GetIndex() const;
	void SetIndex(int iIndex);

	IListOwnerUI* GetOwner();
	void SetOwner(CControlUI* pOwner);
	void SetVisible(bool bVisible = true);
	void SetEnabled(bool bEnable = true);

	bool IsSelected() const;
	bool Select(bool bSelect = true, bool bInvalidate = true);
	bool IsExpanded() const;
	bool Expand(bool bExpand = true);

	void Invalidate(); // ֱ��CControl::Invalidate�ᵼ�¹�����ˢ�£���д����ˢ������
	bool Activate();

	void DoEvent(TEventUI& event);
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	void DoPaint(HDC hDC, const RECT& rcPaint);

	void DrawItemText(HDC hDC, const RECT& rcItem);    
	void DrawItemBk(HDC hDC, const RECT& rcItem);
	void DrawBtn(HDC hDC, const RECT& rcItem);
	void DrawIcon(HDC hDC, const RECT& rcItem);


	//add by lighten
	SIZE EstimateSize(SIZE szAvailable);
	void SetPos(RECT rc);

	void CalBtnPos();
	void CalIconPos();
	void SetShowIcon(bool bShow = true);

protected:
	int m_iIndex;
	bool m_bSelected;
	UINT m_uButtonState;
	IComboExUI* m_pOwner;
	IListOwnerUI *m_pListOwner = NULL;
	RECT m_rcBtnPos;
	RECT m_rcIconPos;
	bool m_bShowButton;
	bool m_bShowIcon;
	UINT m_nShowCount;
};
}
#endif //_COMBOUI_EX_HEADER_
