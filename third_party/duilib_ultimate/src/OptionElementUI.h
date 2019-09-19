#pragma once
namespace DuiLib{
	class UILIB_API COptionElementUI : public COptionUI
{
public:
	COptionElementUI(void);

	LPCTSTR GetClass() const;
	LPVOID GetInterface(LPCTSTR pstrName);
	UINT GetControlFlags() const;

	bool Activate();
	void DoEvent(TEventUI& event);

	void PaintText(HDC hDC);
	void PaintStatusImage(HDC hDC);

protected:
	TEventUI *m_pEvent;
};
}