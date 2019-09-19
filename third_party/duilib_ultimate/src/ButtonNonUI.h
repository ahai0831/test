#pragma once 
  
namespace DuiLib {

	class UILIB_API CButtonNonUI :public CButtonUI
	{
	public:
		CButtonNonUI(void);
		~CButtonNonUI(void);
		void DoEvent(TEventUI& event); 

		UINT GetControlFlags() const;
		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
	};
}
