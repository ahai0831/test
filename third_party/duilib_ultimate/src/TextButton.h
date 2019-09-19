#pragma once
#pragma once
namespace DuiLib {

class CTextButton: public CButtonUI
{
public:
	CTextButton(void);
	~CTextButton(void);

	LPCTSTR GetClass() const;
	LPVOID GetInterface(LPCTSTR pstrName);
	UINT GetControlFlags() const;

	SIZE EstimateSize(SIZE szAvailable);

};

}
