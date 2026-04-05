#pragma once
#include "CAncientGuardian.h"
#include "CBeam.h"

class CCYGuardian : public CAncientGuardian
{
public:
    explicit CCYGuardian(LPDIRECT3DDEVICE9 pGraphicDev);
    virtual ~CCYGuardian();

public:
    virtual _int Update_GameObject(const _float& fTimeDelta) override;
    virtual void LateUpdate_GameObject(const _float& fTimeDelta) override;
    virtual void Render_GameObject() override;

public:
    void Set_TargetPos(_vec3 vPos) { m_vTargetPos = vPos; }
    bool IsActive() { return m_bActive; }
    void SetActive(bool bActive) { m_bActive = bActive; } 
    void Take_Damage(int iDamage); 
    void Set_Hp(int iHp) { m_iHp = iHp; m_iMaxHp = iHp; }
    bool IsDead() { return m_iHp <= 0; }

private:
    void Update_CCY_AI(const _float& fTimeDelta);

private:
    _vec3 m_vTargetPos = { 0.f, 0.f, 0.f };
    bool  m_bActive = false;
    float m_fMoveSpeed = 5.f;
    float m_fAtkTimer = 0.f;
    float m_fAtkInterval = 3.f;
    float m_fIdleSoundTimer = 0.f;
    static constexpr float m_fIdleSoundInterval = 5.f;
    vector<CBeam*> m_vecBeams; 

    
   

public:
    static CCYGuardian* Create(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vPos);

protected:
    virtual void Free();
};