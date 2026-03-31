#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"

class CTorch : public CGameObject
{
private:
    explicit CTorch(LPDIRECT3DDEVICE9 pGraphicDev);
    virtual ~CTorch();

public:
    virtual HRESULT Ready_GameObject();
    virtual _int    Update_GameObject(const _float& fTimeDelta);
    virtual void    LateUpdate_GameObject(const _float& fTimeDelta);
    virtual void    Render_GameObject();

private:
    HRESULT Add_Component();

private:
    CTexture* m_pTextureCom = nullptr;
    CTransform* m_pTransformCom = nullptr;
    CCollider* m_pColliderCom = nullptr;
    Engine::CRcTex* m_pBufferCom = nullptr;
    _float          m_fFlicker = 0.f;

public:
    static CTorch* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
    virtual void Free();
};