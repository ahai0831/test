#ifndef __UISEGMETATION_H__
#define __UISEGMETATION_H__

#pragma once

namespace DuiLib
{
	/////////////////////////////////////////////////////////////////////////////////////
	//

	class UILIB_API CSegmetationUI : public CHorizontalLayoutUI
	{
	public:
		CSegmetationUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		SIZE EstimateSize(SIZE szAvailable);
		void SetPos(RECT rc);

		int AllItemPos(SIZE szAvailable, RECT &rc, int cxFixed, int cxExpand, int nAdjustables, int cxNeeded, int nEstimateNum);

		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

		void DoEvent(TEventUI& event);
	private:
	};


	/////////////////////////////////////////////////////////////////////////////////////
	//

	class UILIB_API CSegmetationItemUI : public CContainerUI
	{
	public:
		CSegmetationItemUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		UINT GetControlFlags() const;

		void SetEnabled(bool bEnable = true);

		bool IsDragable() const;
		void SetDragable(bool bDragable);
		DWORD GetSepWidth() const;
		void SetSepWidth(int iWidth);
		LPCTSTR GetSepImage() const;
		void SetSepImage(LPCTSTR pStrImage);

		void DoEvent(TEventUI& event);
		SIZE EstimateSize(SIZE szAvailable);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		RECT GetThumbRect() const;

		void PaintStatusImage(HDC hDC);

	protected:
		POINT ptLastMouse;
		bool m_bDragable;
		UINT m_uButtonState;
		int m_iSepWidth;
		CStdString m_sSepImage;
		CStdString m_sSepImageModify;
		int m_nScale;
	};
}	// namespace DuiLib

#endif // __UISEGMETATION_H__