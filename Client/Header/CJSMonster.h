#pragma once
#include "CGameObject.h"
#include "CJSBodyPart.h"

class CJSMonster : public CGameObject
{
private:
    explicit CJSMonster(LPDIRECT3DDEVICE9 pGraphicDev);
    virtual ~CJSMonster();

public:
    HRESULT Ready_GameObject(_vec3 vStartPos);
    virtual _int    Update_GameObject(const _float& fTimeDelta) override;
    virtual void    LateUpdate_GameObject(const _float& fTimeDelta) override;
    virtual void    Render_GameObject() override;

public:
    virtual bool Is_Dead() override { return m_bDead; }
    void Set_Dead() { m_bDead = true; }

private:
    HRESULT Add_Component();
    HRESULT Ready_BodyParts();
    void    Update_BodyParts(const _float& fTimeDelta);
    void    LateUpdate_BodyParts(const _float& fTimeDelta);
    void    Update_RunAnimation(const _float& fTimeDelta);

private:
    CTransform* m_pTransformCom = nullptr;

    CJSBodyPart* m_pHead = nullptr;
    CJSBodyPart* m_pBody = nullptr;
    CJSBodyPart* m_pArmL = nullptr;
    CJSBodyPart* m_pArmR = nullptr;
    CJSBodyPart* m_pLegL = nullptr;
    CJSBodyPart* m_pLegR = nullptr;

    _float  m_fSpeed = 20.f;
    _float  m_fAnimTime = 0.f;
    _float  m_fAnimSpeed = 10.f;

    _bool   m_bDead = false;

public:
    static CJSMonster* Create(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vStartPos);

private:
    virtual void Free();
};

