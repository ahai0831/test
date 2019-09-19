#ifndef  _EDIT_COMBOUI_HEADER_
#define _EDIT_COMBOUI_HEADER_

#pragma once

namespace DuiLib {

#define  DUILIB_COMBO_ITMEDELETE 0x22
class CComboEditWnd;

class UILIB_API CEditComboUI :public CComboUI, public IComboExUI
{
	friend CComboEditWnd;
public:
	CEditComboUI();
	virtual ~CEditComboUI();

	LPCTSTR GetClass() const;
	UINT GetControlFlags() const;
	LPVOID GetInterface(LPCTSTR pstrName);

	bool ClickItem(int iIndex, bool bTakeFocus = false);
	bool CheckItem(int iIndex, bool bTakeFocus = false);
	bool RemoveItem(int iIndex);

	void SetText(LPCTSTR pstrText);
	CStdString GetText() const;
	CStdString GetSelText() const;

	void SetMaxChar(UINT uMax);
	UINT GetMaxChar();
	void SetReadOnly(bool bReadOnly);
	bool IsReadOnly() const;
	void SetPasswordMode(bool bPasswordMode);
	bool IsPasswordMode() const;
	void SetPasswordChar(TCHAR cPasswordChar);
	TCHAR GetPasswordChar() const;
	void SetNumberOnly(bool bNumberOnly);
	bool IsNumberOnly() const;
	void SetNativeEditBkColor(DWORD dwBkColor);
	DWORD GetNativeEditBkColor() const;

	void SetEditOffSet(RECT rc);     // edit �ؼ�ƫ����
	RECT GetEditRect() const;

	void SetDefaultText(LPCTSTR pStrDefaultText);
	LPCTSTR GetDefaultText();

	LPCTSTR GetEditHotImage() const;
	void SetEditHotImage(LPCTSTR pStrImage);
	LPCTSTR GetEditFocusedImage() const;
	void SetEditFocusedImage(LPCTSTR pStrImage);

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
	void SetKeyState(bool bVkReturn);   // ����״̬�������Ƿ���Ҫ������
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	bool RemoveAt(int iIndex);

	void PaintText(HDC hDC);

	void SetPos(RECT rc);
	void DoEvent(TEventUI& event);
	void DoPaint(HDC hDC, const RECT& rcPaint);
	void DoPostPaint(HDC hDC, const RECT& rcPaint);
	void PaintStatusImage(HDC hDC);
	void SetEditFocus();   // ����Edit focus״̬


protected:
	CComboEditWnd* m_pEditWindow;

	UINT m_uMaxChar;
	bool m_bReadOnly;
	bool m_bPasswordMode;
	TCHAR m_cPasswordChar;
	bool m_bNumberOnly;
	DWORD m_dwEditbkColor;
	RECT  m_editOffSet;  
	RECT  m_rcEdit;

	// Ĭ������
	CStdString m_strDefaultText;   //Ĭ�ϵ���ʾ�ַ���Ϊ��ʱ����ʾ
	DWORD m_dwDefaultTextColor;
	int  m_iDefaultTextFont;

	bool m_bMouseInEdit = false;
	CStdString m_sEditHotImage;
	CStdString m_sEditFocusedImage;

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
	bool m_bVK_Return = false;
};

}
#endif //_EDIT_COMBOUI_HEADER_
