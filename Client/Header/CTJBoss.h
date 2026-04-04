#pragma once
#include "CGameObject.h"
#include "CMonsterBody.h"
#include "CProtoMgr.h"
#include "CCollider.h"

class CDynamicCamera;

enum class ETJBossState { IDLE, WALK, JUMP, LAND, DEAD };

class CTJBoss : public CGameObject
{
private:
    explicit CTJBoss(LPDIRECT3DDEVICE9 pGraphicDev);
    virtual ~CTJBoss();

public:
    virtual HRESULT Ready_GameObject(_vec3 vPos);
    virtual _int    Update_GameObject(const _float& fTimeDelta);
    virtual void    LateUpdate_GameObject(const _float& fTimeDelta);
    virtual void    Render_GameObject();

public:
    void Take_Damage(_float fDamage);
    bool Is_Dead() const { return m_bDead; }
    void Set_Camera(CDynamicCamera* pCam) { m_pCamera = pCam; }
    CCollider* Get_Collider() const { return m_pColliderCom; }

private:
    HRESULT Add_Component(_vec3 vPos);
    void    Update_AI(const _float& fTimeDelta);
    void    Apply_Gravity(const _float& fTimeDelta);

private:
    CMonsterBody* m_pBodyCom = nullptr;
    Engine::CTransform* m_pTransformCom = nullptr;
    Engine::CTexture* m_pTextureCom = nullptr;
    Engine::CCollider* m_pColliderCom = nullptr;
    CDynamicCamera* m_pCamera = nullptr;

    ETJBossState    m_eState = ETJBossState::IDLE;
    _float          m_fHp = 500.f;
    _float          m_fMaxHp = 500.f;
    _float          m_fMoveSpeed = 4.f;
    _bool           m_bDead = false;

    // 점프
    _float  m_fJumpTimer = 0.f;
    _float  m_fJumpInterval = 4.f;
    _float  m_fVelocityY = 0.f;
    _bool   m_bOnGround = true;
    _vec3   m_vJumpTarget = {};

    // 착지 범위 데미지
    _float  m_fLandDmgRadius = 5.f;
    _float  m_fLandDmg = 30.f;

    _float  m_fWalkTime = 0.f;

    _bool m_bHitCooldown = false;
    _float m_fHitTimer = 0.f;
    _float m_fHitInterval = 0.5f;

public:
    static CTJBoss* Create(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vPos);
    _float Get_HpRatio() const { return m_fHp / m_fMaxHp; }
    Engine::CTransform* Get_Transform() { return m_pTransformCom; }
private:
    virtual void Free();
};