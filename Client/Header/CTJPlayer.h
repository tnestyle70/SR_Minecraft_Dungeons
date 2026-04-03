#pragma once
#include "CPlayer.h"
#include "CTNT.h"

enum class ETJAbility
{
    ARROW_PLUS,  
    TNT_LAUNCHER,
    HP_REGEN,    
    LIGHTNING,   
    FIRE_TRAIL,  
    BLADE_ORBIT, 
    ABILITY_END
};

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
    _int     Get_Level()  const { return m_iLevel; }
    _int     Get_Exp()    const { return m_iExp; }
    _int     Get_MaxExp() const { return m_iMaxExp; }
    _bool    Is_LevelUp() const { return m_bLevelUp; }
    void    Set_LevelUp(bool b) { m_bLevelUp = b; }

    // 능력 상태 조회
    bool    Has_TNTAura()    const { return m_bTNTAura; }
    int     Get_ArrowCount() const { return m_iArrowCount; }

    void Apply_Ability(ETJAbility eAbility);
    _int  Get_AbilityLevel(ETJAbility eAbility) const { return m_iAbilityLevel[(int)eAbility]; }
    _int  Get_AbilityMaxLevel(ETJAbility eAbility) const { return m_iAbilityMaxLevel[(int)eAbility]; }
    _bool Is_AbilityMaxed(ETJAbility eAbility) const { return m_iAbilityLevel[(int)eAbility] >= m_iAbilityMaxLevel[(int)eAbility]; }

    virtual void Key_Input(const _float& fTimeDelta) override;

public:
    static CTJPlayer* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
    virtual void Free();

private:
    //레벨링
    _int     m_iLevel = 1;
    _int     m_iExp = 0;
    _int     m_iMaxExp = 1.f;
    _bool    m_bLevelUp = false;

    // TNT 오라
    _bool    m_bTNTAura = false;
    _float   m_fTNTTimer = 0.f;
    _float   m_fTNTInterval = 3.f;
    vector<CTNT*> m_vecTNTAura;

    // 멀티샷
    _int     m_iArrowCount = 1;

    // HP 회복
    _bool    m_bHPRegen = false;
    _float   m_fRegenTimer = 0.f;
    _float m_fRegenInterval = 1.f;
    _float   m_fRegenAmount = 1.f;

    // 번개
    _bool    m_bLightning = false;
    _float   m_fLightningTimer = 0.f;
    _float   m_fLightningInterval = 4.f;

    // 번개 이펙트
    Engine::CRcTex* m_pThunderBufferCom = nullptr;
    Engine::CTexture* m_pLightningTextureCom = nullptr;
    Engine::CTexture* m_pSparkTextureCom = nullptr;
    bool m_bThunderEffect = false;
    float m_fThunderEffectTimer = 0.f;
    float m_fThunderEffectDuration = 0.5f;
    int m_iLightningFrame = 0;
    int m_iSparkFrame = 0;
    float m_fFrameTimer = 0.f;
    float m_fFrameInterval = 0.1f;
    _vec3 m_vThunderPos = {};

    // 화염 장판
    struct TJFireTrail
    {
        _vec3 vPos;
        _float fLifeTime = 3.f;
        _float fDmgTimer = 0.f;
    };

    // 화염 장판
    _bool m_bFireTrail = false;
    vector<TJFireTrail> m_vecFireTrails;
    _float m_fFireSpawnTimer = 0.f;
    _float m_fFireSpawnInterval = 0.3f;
    Engine::CRcTex* m_pFireBufferCom = nullptr;
    Engine::CTexture* m_pFireTextureCom = nullptr;
    float m_fFireDamage = 5.f;

    // 회전 칼날
    _bool    m_bBladeOrbit = false;
    _int     m_iBladeCount = 1;  
    _float m_fBladeAngle = 0.f;      // 현재 공전 각도
    _float m_fBladeOrbitRadius = 3.f; // 공전 반지름
    _float m_fBladeOrbitSpeed = 180.f; // 초당 회전 각도
    _float m_fBladeHitTimer = 0.f;
    _float m_fBladeHitInterval = 0.5f; // 타격 간격
    
    float m_fBladeSelfAngle = 0.f; // 자전 각도
    float m_fBladeSelfSpeed = 360.f; // 초당 자전 각도

    Engine::CRcTex* m_pBladeBufferCom = nullptr;
    Engine::CTexture* m_pBladeTextureCom = nullptr;


    _int m_iAbilityLevel[(int)ETJAbility::ABILITY_END] = {};
    _int m_iAbilityMaxLevel[(int)ETJAbility::ABILITY_END] = { 3, 3, 3, 3, 3, 3 };
};