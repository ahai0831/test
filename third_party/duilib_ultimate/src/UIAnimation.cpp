#include "StdAfx.h"
#include <algorithm>

namespace DuiLib {

	CUIAnimation::CUIAnimation(CControlUI* pOwner)
	{
		ASSERT(pOwner != NULL);
		m_pControl = pOwner;
	}

	BOOL CUIAnimation::StartAnimation(int nElapse, int nTotalFrame, int nAnimationID /*= 0*/, BOOL bLoop/* = FALSE*/)
	{
		CAnimationData* pData = GetAnimationDataByID(nAnimationID);
		if( NULL != pData 
			|| nElapse <= 0
			|| nTotalFrame <= 0
			|| NULL == m_pControl )
		{
			ASSERT(FALSE);
			return FALSE;
		}

		CAnimationData* pAnimation = new CAnimationData(nElapse, nTotalFrame, nAnimationID, bLoop);
		if( NULL == pAnimation ) return FALSE;
		
		if(m_pControl->GetManager()->SetTimer( m_pControl, nAnimationID, nElapse ))
		{
			m_arAnimations.push_back(pAnimation);
			return TRUE;
		}
		return FALSE;
	}

	VOID CUIAnimation::StopAnimation(int nAnimationID /*= 0*/)
	{
		if(m_pControl == NULL) return;

		if(nAnimationID  != 0)
		{
			CAnimationData* pData = GetAnimationDataByID(nAnimationID);
			if( NULL != pData )
			{
				m_pControl->GetManager()->KillTimer( m_pControl, nAnimationID );
				m_arAnimations.erase(std::remove(m_arAnimations.begin(), m_arAnimations.end(), pData), m_arAnimations.end());
				delete pData;
				pData = NULL;
				return;
			}
		}
		else
		{
			int nCount = m_arAnimations.size();
			for(int i=0; i<nCount; ++i)
			{
				m_pControl->GetManager()->KillTimer( m_pControl, m_arAnimations[i]->m_nAnimationID );
			}
			FreeAnimationData(0);
			m_arAnimations.clear();
		}
	}

	BOOL CUIAnimation::IsAnimationRunning(int nAnimationID)
	{
		CAnimationData* pData = GetAnimationDataByID(nAnimationID);
		return NULL != pData;
	}

	int CUIAnimation::GetCurrentFrame(int nAnimationID/* = 0*/)
	{
		CAnimationData* pData = GetAnimationDataByID(nAnimationID);
		if( NULL == pData )
		{
//			ASSERT(FALSE);
			return -1;
		}
		return pData->m_nCurFrame;
	}

	BOOL CUIAnimation::SetCurrentFrame(int nFrame, int nAnimationID/* = 0*/)
	{
		CAnimationData* pData = GetAnimationDataByID(nAnimationID);
		if( NULL == pData)
		{
//			ASSERT(FALSE);
			return FALSE;
		}

		if(nFrame >= 0 && nFrame <= pData->m_nTotalFrame)
		{
			pData->m_nCurFrame = nFrame;
			return TRUE;
		}
		else
		{
			ASSERT(FALSE);
		}
		return FALSE;
	}

	VOID CUIAnimation::OnAnimationElapse(int nAnimationID)
	{
		if(m_pControl == NULL) return;

		CAnimationData* pData = GetAnimationDataByID(nAnimationID);
		if( NULL == pData ) return;

		int nCurFrame = pData->m_nCurFrame;
		if(nCurFrame == 0)
		{
			OnAnimationStart(nAnimationID, pData->m_bFirstLoop);
			pData->m_bFirstLoop = FALSE;
		}

		OnAnimationStep(pData->m_nTotalFrame, nCurFrame, nAnimationID);

		if(nCurFrame >= pData->m_nTotalFrame)
		{
			if(pData->m_bLoop)
			{
				pData->m_nCurFrame = 0;
				return;
			}
			else
			{
				OnAnimationStop(nAnimationID);
				m_pControl->GetManager()->KillTimer( m_pControl, nAnimationID );
				m_arAnimations.erase(std::remove(m_arAnimations.begin(), m_arAnimations.end(), pData), m_arAnimations.end());
				delete pData;
				pData = NULL;
			}
		}

		if( NULL != pData )
		{
			++(pData->m_nCurFrame);
		}
	}

	CAnimationData* CUIAnimation::GetAnimationDataByID(int nAnimationID)
	{
		CAnimationData* pRet = NULL;
		int nCount = m_arAnimations.size();
		for(int i=0; i<nCount; ++i)
		{
			if(m_arAnimations[i]->m_nAnimationID == nAnimationID)
			{
				pRet = m_arAnimations[i];
				break;
			}
		}

		return pRet;
	}
	
	void CUIAnimation::FreeAnimationData(int nAnimationID)
	{
		CAnimationData* pRet = NULL;
		int nCount = m_arAnimations.size();
		if (nAnimationID > 0)
		{
			for(int i=0; i<nCount; ++i)
			{
				if(m_arAnimations[i]->m_nAnimationID == nAnimationID)
				{
					pRet = m_arAnimations[i];
					delete pRet;
					pRet = NULL;
					return;
				}
			}
		}
		else
		{
			for(int i=0; i<nCount; ++i)
			{
				pRet = m_arAnimations[i];
				delete pRet;
				pRet = NULL;
			}
		}
		return;
	}

	VOID CUIAnimation::PlayAnimation(int nAnimationID)
	{
		if (m_pControl == NULL) return;

		if (nAnimationID != 0)
		{
			CAnimationData* pData = GetAnimationDataByID(nAnimationID);
			if (NULL != pData)
			{ 
				if (!pData->m_bPlaying)
				{
					m_pControl->GetManager()->SetTimer(m_pControl, nAnimationID, pData->m_nElapse);
					pData->m_bPlaying = TRUE;
				}

				return;
			}
		}
		else
		{
			int nCount = m_arAnimations.size();
			for (int i = 0; i < nCount; ++i)
			{
				if (!m_arAnimations[i]->m_bPlaying)
				{
					m_pControl->GetManager()->SetTimer(m_pControl, m_arAnimations[i]->m_nAnimationID, m_arAnimations[i]->m_nElapse);
					m_arAnimations[i]->m_bPlaying = TRUE;
				}
				
			}
		}
	}

	VOID CUIAnimation::PasedAnimation(int nAnimationID)
	{
		if (m_pControl == NULL) return;

		if (nAnimationID != 0)
		{
			CAnimationData* pData = GetAnimationDataByID(nAnimationID);
			if (NULL != pData)
			{
				if (pData->m_bPlaying)
				{
					m_pControl->GetManager()->KillTimer(m_pControl, nAnimationID);
					pData->m_bPlaying = FALSE;
				}
				
				return;
			}
		}
		else
		{
			int nCount = m_arAnimations.size();
			for (int i = 0; i < nCount; ++i)
			{
				if (m_arAnimations[i]->m_bPlaying)
				{
					m_pControl->GetManager()->KillTimer(m_pControl, m_arAnimations[i]->m_nAnimationID);
					m_arAnimations[i]->m_bPlaying = FALSE;
				}
				
			}
		}
	}

} // namespace DuiLib