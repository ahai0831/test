#ifndef __UILABELEX_H__
#define __UILABELEX_H__

#include "AnimationGIF.h"
#pragma once

namespace DuiLib {

class UILIB_API CGIFUI: public CLabelUI, public CAnimationGIF
{
public:
	CGIFUI(void);
	~CGIFUI(void);

	 void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
public:
	void PaintBkImage(HDC hDC);
//	void PaintStatusImage(HDC hDC);

	void DoEvent(TEventUI& event);
	void OnTimer( int nTimerID );

	virtual void OnAnimationStart(INT nAnimationID, BOOL bFirstLoop) {}
	virtual void OnAnimationStep(INT nTotalFrame, INT nCurFrame, INT nAnimationID);
	virtual void OnAnimationStop(INT nAnimationID);

};

class UILIB_API CGIFUIEX : public CLabelUI, public CAnimationGIF
{
public:
	CGIFUIEX(void);
	~CGIFUIEX(void);

	int GetMinValue() const;
	void SetMinValue(int nMin);
	int GetMaxValue() const;
	void SetMaxValue(int nMax);
	int GetValue() const;
	void SetValue(int nValue);
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
public:
	void PaintBkImage(HDC hDC);

	void DoEvent(TEventUI& event);
	void OnTimer(int nTimerID);

	virtual void OnAnimationStart(INT nAnimationID, BOOL bFirstLoop) {}
	virtual void OnAnimationStep(INT nTotalFrame, INT nCurFrame, INT nAnimationID);
	virtual void OnAnimationStop(INT nAnimationID);
protected:
	int m_nMax;
	int m_nMin;
	int m_nValue;

	RECT m_rcPaintPos;
};

}

#endif //__UILABELEX_H__
