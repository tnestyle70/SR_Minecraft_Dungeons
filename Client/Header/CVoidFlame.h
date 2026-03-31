#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"

class CVoidFlame : public CGameObject
{
public:
    CVoidFlame(LPDIRECT3DDEVICE9 pGraphicDev);
    virtual ~CVoidFlame();

public:
    virtual HRESULT Ready_GameObject()                              override;
    virtual _int    Update_GameObject(const _float& fTimeDelta)    override;
    virtual void    LateUpdate_GameObject(const _float& fTimeDelta) override;
    virtual void    Render_GameObject()                             override;
    virtual void    Free()                                          override;

    bool       Is_Dead()    const { return m_bDead; }
    void       Set_Dead() { m_bDead = true; }
    float      Get_Damage() const { return m_fDamage; }
    CCollider* Get_Collider() const { return m_pColliderCom; }

private:
    HRESULT Add_Component();

private:
    CTransform* m_pTransformCom = nullptr;
    CRcTex* m_pBufferCom = nullptr;
    CCollider* m_pColliderCom = nullptr;

    _vec3 m_vDir = {};
    float m_fSpeed = 25.f;
    float m_fDamage = 0.f;
    float m_fLifeTime = 0.f;
    bool  m_bDead = false;

    float m_fTime = 0.f;

public:
    static CVoidFlame* Create(LPDIRECT3DDEVICE9 pGraphicDev,
        const _vec3& vPos,
        const _vec3& vDir,
        float        fDamage);
};
