#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"
#include "CCollider.h"

class CTJSpawnMgr;

class CTJMagnet : public CGameObject
{
private:
    explicit CTJMagnet(LPDIRECT3DDEVICE9 pGraphicDev);
    virtual ~CTJMagnet();
public:
    virtual HRESULT Ready_GameObject(_vec3 vPos);
    virtual _int    Update_GameObject(const _float& fTimeDelta);
    virtual void    LateUpdate_GameObject(const _float& fTimeDelta);
    virtual void    Render_GameObject();
private:
    HRESULT Add_Component();
public:
    bool Is_Dead() const { return m_bDead; }
    static CTJMagnet* Create(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vPos);
private:
    virtual void Free();
private:
    Engine::CRcTex* m_pBufferCom = nullptr;
    Engine::CTexture* m_pTextureCom = nullptr;
    Engine::CTransform* m_pTransformCom = nullptr;
    Engine::CCollider* m_pColliderCom = nullptr;
    bool    m_bDead = false;
    float   m_fPickupRange = 5.f;
};