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

	void SetEditOffSet(RECT rc);     // edit 控件偏移量
	RECT GetEditRect() const;

	void SetDefaultText(LPCTSTR pStrDefaultText);
	LPCTSTR GetDefaultText();

	LPCTSTR GetEditHotImage() const;
	void SetEditHotImage(LPCTSTR pStrImage);
	LPCTSTR GetEditFocusedImage() const;
	void SetEditFocusedImage(LPCTSTR pStrImage);

	void SetBtnOffset(RECT rc);   //设置Btn偏移量，右边的设置右下角偏移量
	RECT GetBtnOffset();

	SIZE GetBtnSize();      //按钮大小

	void SetBtnNormalImage(LPCTSTR lpstr);
	LPCTSTR GetBtnNormalImage();
	void SetBtnHotImage(LPCTSTR lpstr);
	LPCTSTR GetBtnHotImage();
	void SetBtnPushedImage(LPCTSTR lpstr);
	LPCTSTR GetBtnPushedImage();

	//显示
	void SetIconOffset(RECT rc);   //设置Icon偏移量，右边的设置右下角偏移量
	RECT GetIconOffset();

	SIZE GetIconSize();      //按钮大小

	void SetIconImage(LPCTSTR lpstr);
	LPCTSTR GetIconImage();
	void SetDescIconImage(LPCTSTR lpstr);
	LPCTSTR GetDescIconImage();

	//字体
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
	void SetKeyState(bool bVkReturn);   // 设置状态，控制是否需要最后绘制
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	bool RemoveAt(int iIndex);

	void PaintText(HDC hDC);

	void SetPos(RECT rc);
	void DoEvent(TEventUI& event);
	void DoPaint(HDC hDC, const RECT& rcPaint);
	void DoPostPaint(HDC hDC, const RECT& rcPaint);
	void PaintStatusImage(HDC hDC);
	void SetEditFocus();   // 设置Edit focus状态


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

	// 默认字体
	CStdString m_strDefaultText;   //默认的显示字符，为空时，显示
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


	CStdString m_sIconImage;         //标记图像
	CStdString m_sDescIconImage;     //降序标记图像

	RECT m_rcIconOffset;
	SIZE m_szIcon;
	bool m_bShowIcon;
	bool m_bDelayHover;   // 高亮状态延时绘制
	bool m_bVK_Return = false;
};

}
#endif //_EDIT_COMBOUI_HEADER_
