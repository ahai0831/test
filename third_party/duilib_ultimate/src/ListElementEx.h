#ifndef  _LISTELEMENT_HEADER__
#define  _LISTELEMENT_HEADER__

//////////////////////////////////////////////////////////////////////////
//
//带按钮的列表子项目
#include <vector>
using namespace std;


#pragma once
namespace DuiLib {
class UILIB_API CListElementEx : public CHorizontalLayoutUI
{
public:
	CListElementEx();
	virtual ~CListElementEx();

	LPCTSTR GetClass() const;
	LPVOID GetInterface(LPCTSTR pstrName);
	bool Add(CControlUI* pControl);

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
	void SetShowLines(int nShowLines = 1);
	int  GetShowLines() const;
	void SetLineSpace(int nLineSpace = 5);
	int GetLineSpace() const;

	LPCTSTR GetForeImage();
	void SetForeImage(LPCTSTR pStrImage);
	LPCTSTR GetFgBkImage();
	void SetFgBkImage(LPCTSTR pStrImage);

	void PaintStatusImage(HDC hDC);
	void PaintText(HDC hDC);

	SIZE EstimateSize(SIZE szAvailable);
	void DoEvent(TEventUI& event);
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

	void SetPos(RECT rc);

	void SetChildItemVisible(int nIndex, bool bVisible = true);   //设置指定的子项是否显示
	void SetChildAirItemVisible(int nIndex, bool bVisible = true); //设置指定的悬浮子项是否显示

	void ChildVisible(bool bVisible = true);   //子项的显示和隐藏
	void SetChildItemText(int nIndex, CStdString strText);        //设置指定的子项的文本
	CControlUI * GetChildItemControl(int nIndex);                            // 获取指定索引的控件的位置
	
protected:
	CStdString m_sForeImage;
	CStdString m_sFgBkImage;    //背景的前置图
	DWORD m_dwTextColor;
	DWORD m_dwDisabledTextColor;
	int m_iFont;
	UINT m_uTextStyle;
	RECT m_rcTextPadding;
	bool m_bShowHtml;
	int m_nLeavePos;
	vector<bool> m_VisibleArray;
	int m_nButtonSize;
	
	BOOL m_bSignalLine;    // 是否一行显示
	int m_nLineSpace;      // 行距
	int m_nShowLines;      // 显示行数
	
	bool m_bShowLeft_to_right;          // 从左到右显示
	

};
}
#endif //_LISTELEMENT_HEADER__

