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
    void Reset() { m_iScore = 0; m_fDistance = 0.f; }

private:
    _int m_iScore = 0;
    _float m_fDistance = 0.f;
    _float m_fSpeed = 20.f;

private:
    virtual void Free();
};

