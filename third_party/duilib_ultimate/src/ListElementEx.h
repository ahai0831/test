#ifndef  _LISTELEMENT_HEADER__
#define  _LISTELEMENT_HEADER__

//////////////////////////////////////////////////////////////////////////
//
//����ť���б�����Ŀ
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

	void SetChildItemVisible(int nIndex, bool bVisible = true);   //����ָ���������Ƿ���ʾ
	void SetChildAirItemVisible(int nIndex, bool bVisible = true); //����ָ�������������Ƿ���ʾ

	void ChildVisible(bool bVisible = true);   //�������ʾ������
	void SetChildItemText(int nIndex, CStdString strText);        //����ָ����������ı�
	CControlUI * GetChildItemControl(int nIndex);                            // ��ȡָ�������Ŀؼ���λ��
	
protected:
	CStdString m_sForeImage;
	CStdString m_sFgBkImage;    //������ǰ��ͼ
	DWORD m_dwTextColor;
	DWORD m_dwDisabledTextColor;
	int m_iFont;
	UINT m_uTextStyle;
	RECT m_rcTextPadding;
	bool m_bShowHtml;
	int m_nLeavePos;
	vector<bool> m_VisibleArray;
	int m_nButtonSize;
	
	BOOL m_bSignalLine;    // �Ƿ�һ����ʾ
	int m_nLineSpace;      // �о�
	int m_nShowLines;      // ��ʾ����
	
	bool m_bShowLeft_to_right;          // ��������ʾ
	

};
}
#endif //_LISTELEMENT_HEADER__

