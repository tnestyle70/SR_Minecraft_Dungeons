#pragma once
#include "CBase.h"
#include "Engine_Define.h"

class CJSScoreMgr : public CBase
{
	DECLARE_SINGLETON(CJSScoreMgr)

public:
	explicit CJSScoreMgr();
	virtual ~CJSScoreMgr();

public:
    void Add_Score(_int iScore) { m_iScore += iScore; }
    void Set_Distance(_float fDist) { m_fDistance = fDist; }
    void Set_Speed(_float fSpeed) { m_fSpeed = fSpeed; }
    _int Get_Score() { return m_iScore; }
    _float Get_Distance() { return m_fDistance; }
    _float Get_Speed() { return m_fSpeed; }
    void Set_GameOver(DEATHTYPE eType) { m_eDeathType = eType; }
    DEATHTYPE Get_DeathType() { return m_eDeathType; }
    _bool Is_GameOver() { return m_eDeathType != DEATH_NONE; }
    void Set_Slide(_bool bSlide) { m_bSlide = bSlide; }
    _bool Is_Slide() { return m_bSlide; }
    void Set_Stage(JSGAMESTAGE eStage) { m_eStage = eStage; }
    JSGAMESTAGE Get_Stage() { return m_eStage; }
    _bool Is_Playing() { return m_eStage == JSSTAGE_PLAY; }
    void Set_Countdown(_int iCount) { m_iCountdown = iCount; }
    _int Get_Countdown() { return m_iCountdown; }

    void Reset() { m_iScore = 0; m_fDistance = 0.f; m_eDeathType = DEATH_NONE; m_eStage = JSSTAGE_INTRO;}

private:
    _int m_iScore = 0;
    _float m_fDistance = 0.f;
    _float m_fSpeed = 20.f;

    JSGAMESTAGE     m_eStage = JSSTAGE_INTRO;
    DEATHTYPE m_eDeathType = DEATH_NONE;
    _bool m_bSlide = false;
    _int    m_iCountdown = 3;

private:
    virtual void Free();
};

