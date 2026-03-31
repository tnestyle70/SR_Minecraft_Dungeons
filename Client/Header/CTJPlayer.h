#pragma once
#include "CPlayer.h"
#include "CTNT.h"

class CTJPlayer : public CPlayer
{
private:
    explicit CTJPlayer(LPDIRECT3DDEVICE9 pGraphicDev);
    virtual ~CTJPlayer();

public:
    virtual HRESULT Ready_GameObject() override;
    virtual _int    Update_GameObject(const _float& fTimeDelta) override;
    virtual void    LateUpdate_GameObject(const _float& fTimeDelta) override;
    virtual void    Render_GameObject() override;

public:
    // 레벨링 시스템
    void    Add_Exp(int iExp);
    int     Get_Level()  const { return m_iLevel; }
    int     Get_Exp()    const { return m_iExp; }
    int     Get_MaxExp() const { return m_iMaxExp; }
    bool    Is_LevelUp() const { return m_bLevelUp; }
    void    Set_LevelUp(bool b) { m_bLevelUp = b; }

    // 능력 적용
    void    Apply_Ability(int iAbility); // 0=TNT오라, 1=멀티샷+1, 2=펫

    // 능력 상태 조회
    bool    Has_TNTAura()    const { return m_bTNTAura; }
    bool    Has_Pet()        const { return m_bPet; }
    int     Get_ArrowCount() const { return m_iArrowCount; }

public:
    static CTJPlayer* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
    virtual void Free();

private:
    //레벨링
    int     m_iLevel = 1;
    int     m_iExp = 0;
    int     m_iMaxExp = 10;
    bool    m_bLevelUp = false;

    // TNT 오라
    bool    m_bTNTAura = false;
    float   m_fTNTTimer = 0.f;
    float   m_fTNTInterval = 3.f;
    vector<CTNT*> m_vecTNTAura;

    // 멀티샷
    int     m_iArrowCount = 1;

    // 펫
    bool    m_bPet = false;
};