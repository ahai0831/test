#ifndef __UIOPTION_H__
#define __UIOPTION_H__

#pragma once

namespace DuiLib
{
	class UILIB_API CCustomizationOptionUI : public CButtonUI
	{
	public:
		CCustomizationOptionUI();
		~CCustomizationOptionUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		UINT GetControlFlags() const;

		void SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit = true);

		bool Activate();
		void SetEnabled(bool bEnable = true);

		LPCTSTR GetSelectedImage();
		void SetSelectedImage(LPCTSTR pStrImage);

		LPCTSTR GetSelectedHotImage();
		void SetSelectedHotImage(LPCTSTR pStrImage);

		LPCTSTR GetSelectedPushedImage();
		void SetSelectedPushedImage(LPCTSTR pStrImage);

		void SetSelectedTextColor(DWORD dwTextColor);
		DWORD GetSelectedTextColor();

		void SetSelectedBkColor(DWORD dwBkColor);
		DWORD GetSelectBkColor();

		LPCTSTR GetForeImage();
		void SetForeImage(LPCTSTR pStrImage);

		LPCTSTR GetSelectedForedImage();
		void SetSelectedForedImage(LPCTSTR pStrImage);

		LPCTSTR GetGroup() const;
		void SetGroup(LPCTSTR pStrGroupName = NULL);
		bool IsSelected() const;
		void Selected(bool bSelected,bool bNotify = true);

		SIZE EstimateSize(SIZE szAvailable);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

		void PaintStatusImage(HDC hDC);
		void PaintText(HDC hDC);

	protected:
		bool			m_bSelected;
		CStdString		m_sGroupName;

		DWORD			m_dwSelectedBkColor;
		DWORD			m_dwSelectedTextColor;

		CStdString		m_sSelectedImage;
		CStdString		m_sSelectedHotImage;
		CStdString		m_sSelectedPushedImage;
		CStdString		m_sForeImage;
		CStdString		m_sSelectedForeImage;
	};

} // namespace DuiLib

#endif // __UIOPTION_H__