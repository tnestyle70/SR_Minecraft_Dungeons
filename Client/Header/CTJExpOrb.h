#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"
#include "CCollider.h"

class CTJExpOrb : public CGameObject
{
private:
    explicit CTJExpOrb(LPDIRECT3DDEVICE9 pGraphicDev);
    virtual ~CTJExpOrb();

public:
    virtual HRESULT Ready_GameObject(_vec3 vPos, int iExp = 1);
    virtual _int    Update_GameObject(const _float& fTimeDelta);
    virtual void    LateUpdate_GameObject(const _float& fTimeDelta);
    virtual void    Render_GameObject();

private:
    HRESULT Add_Component();

public:
    bool Is_Dead() const { return m_bDead; }
    int  Get_Exp() const { return m_iExp; }
    void Set_AbsorbAll() { m_fAbsorbRange = 99999.f; m_fMoveSpeed = 20.f; }

public:
    static CTJExpOrb* Create(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vPos, int iExp = 1);

private:
    virtual void Free();

private:
    Engine::CRcTex* m_pBufferCom = nullptr;
    Engine::CTexture* m_pTextureCom = nullptr;
    Engine::CTransform* m_pTransformCom = nullptr;
    Engine::CCollider* m_pColliderCom = nullptr;

    int     m_iExp = 1;
    bool    m_bDead = false;


    float   m_fAbsorbRange = 5.f;
    float   m_fMoveSpeed = 14.f;
};