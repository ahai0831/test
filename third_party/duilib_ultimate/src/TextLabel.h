
#pragma once
namespace DuiLib {

	class CTextLabel: public CLabelUI
	{
	public:
		CTextLabel(void);
		~CTextLabel(void);

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		SIZE EstimateSize(SIZE szAvailable);

	};

}