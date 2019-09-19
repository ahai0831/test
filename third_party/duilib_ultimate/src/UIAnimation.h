#ifndef __UIANIMATION_H__
#define __UIANIMATION_H__

#pragma once

//class vector;
#include <vector>
using namespace std;

namespace DuiLib {

	class CControlUI;

	class UILIB_API IUIAnimation
	{
	public:
		virtual ~IUIAnimation() { NULL; }

		virtual BOOL StartAnimation(int nElapse, int nTotalFrame, int nAnimationID = 0, BOOL bLoop = FALSE) = 0;
		virtual VOID StopAnimation(int nAnimationID = 0) = 0;
		virtual BOOL IsAnimationRunning(int nAnimationID) = 0;
		virtual int GetCurrentFrame(int nAnimationID = 0) = 0;
		virtual BOOL SetCurrentFrame(int nFrame, int nAnimationID = 0) = 0;

		virtual VOID OnAnimationStep(int nTotalFrame, int nCurFrame, int nAnimationID) = 0;
		virtual VOID OnAnimationStart(int nAnimationID, BOOL bFirstLoop) = 0;
		virtual VOID OnAnimationStop(int nAnimationID) = 0;

		virtual VOID OnAnimationElapse(int nAnimationID) = 0;

		virtual VOID PlayAnimation(int nAnimationID = 0) = 0;
		virtual VOID PasedAnimation(int nAnimationID = 0) = 0;
	};

	class UILIB_API CAnimationData
	{
	public:
		CAnimationData(int nElipse, int nFrame, int nID, BOOL bLoop)
		{
			m_bFirstLoop = TRUE;
			m_nCurFrame = 0;
			m_nElapse = nElipse;
			m_nTotalFrame = nFrame;
			m_bLoop = bLoop;
			m_nAnimationID = nID;
			m_bPlaying = TRUE;
		}

	//protected:
	public:
		friend class CDUIAnimation;

		int m_nAnimationID;
		int m_nElapse;

		int m_nTotalFrame;
		int m_nCurFrame;

		BOOL m_bLoop;
		BOOL m_bFirstLoop;
		BOOL m_bPlaying;
	};

	class UILIB_API CUIAnimation: public IUIAnimation
	{
	public:
		CUIAnimation(CControlUI* pOwner);

		virtual BOOL StartAnimation(int nElapse, int nTotalFrame, int nAnimationID = 0, BOOL bLoop = FALSE);
		virtual VOID StopAnimation(int nAnimationID = 0);
		virtual BOOL IsAnimationRunning(int nAnimationID);
		virtual int GetCurrentFrame(int nAnimationID = 0);
		virtual BOOL SetCurrentFrame(int nFrame, int nAnimationID = 0);

		virtual VOID OnAnimationStart(int nAnimationID, BOOL bFirstLoop) {};
		virtual VOID OnAnimationStep(int nTotalFrame, int nCurFrame, int nAnimationID) {};
		virtual VOID OnAnimationStop(int nAnimationID) {};

		virtual VOID OnAnimationElapse(int nAnimationID);

		virtual VOID PlayAnimation(int nAnimationID = 0);
		virtual VOID PasedAnimation(int nAnimationID = 0);

	protected:
		CAnimationData* GetAnimationDataByID(int nAnimationID);
		void FreeAnimationData(int nAnimationID);
	protected:
		CControlUI* m_pControl;
		std::vector<CAnimationData*> m_arAnimations;
	};

} // namespace DuiLib

#endif // __UIANIMATION_H__