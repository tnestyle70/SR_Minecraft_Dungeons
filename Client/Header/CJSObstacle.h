#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"

class CJSObstacle : public CGameObject
{
private:
    explicit CJSObstacle(LPDIRECT3DDEVICE9 pGraphicDev);
    virtual ~CJSObstacle();

public:
    HRESULT Ready_GameObject(_vec3 vPos);
    virtual _int    Update_GameObject(const _float& fTimeDelta) override;
    virtual void    LateUpdate_GameObject(const _float& fTimeDelta) override;
    virtual void    Render_GameObject() override;

public:
    virtual bool    Is_Dead() override { return m_bDead; }
    void            Set_Dead() { m_bDead = true; }
    CJSCollider* Get_Collider() { return m_pColliderCom; }

private:
    HRESULT Add_Component();

private:
    CJSSpriteBuffer* m_pBufferCom = nullptr;
    CTransform* m_pTransformCom = nullptr;
    CTexture* m_pTextureCom = nullptr;
    CJSCollider* m_pColliderCom = nullptr;

    _float  m_fFrameTime = 0.f;
    _float  m_fFrameSpeed = 0.05f;   // 프레임 속도 (조절 가능)
    _uint   m_iCurFrame = 0;
    _uint   m_iTotalFrame = 32;

    _bool   m_bDead = false;

public:
    static CJSObstacle* Create(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vPos);
    virtual void Free();
};

