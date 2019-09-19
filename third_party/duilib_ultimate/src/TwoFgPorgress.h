#ifndef __UITWOFGPROGRESS_H__
#define __UITWOFGPROGRESS_H__
#pragma once

namespace DuiLib {

class UILIB_API CTwoFgPorgressUI :public CProgressUI
{
public:
	CTwoFgPorgressUI(void);
	~CTwoFgPorgressUI(void);

	LPCTSTR GetClass() const;
	LPVOID GetInterface(LPCTSTR pstrName);

	int GetTwoValue() const;
	void SetTwoValue(int nValue);

	LPCTSTR GetTwoFgImage() const;
	void SetTwoFgImage(LPCTSTR pStrImage, bool bInvalidate = false);

	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	void PaintStatusImage(HDC hDC);
private:

	int m_nTwoValue;
	CStdString m_sTwoFgImage;
	CStdString m_sTwoFgImageModify;

};
}

#endif  //__UITWOFGPROGRESS_H__
