#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"

struct PartDesc
{
    _vec3   vOffset = {};
    _float  fSizeX = 1.f;
    _float  fSizeY = 1.f;
    _float  fSizeZ = 1.f;

    CJSBodyBuffer::FaceUV front = {};
    CJSBodyBuffer::FaceUV back = {};
    CJSBodyBuffer::FaceUV left = {};
    CJSBodyBuffer::FaceUV right = {};
    CJSBodyBuffer::FaceUV top = {};
    CJSBodyBuffer::FaceUV bottom = {};
};

class CJSBodyPart : public CGameObject
{
private:
    explicit CJSBodyPart(LPDIRECT3DDEVICE9 pGraphicDev);
    virtual ~CJSBodyPart();

public:
    HRESULT Ready_GameObject(CTransform* pParent, const PartDesc& desc);

    virtual _int    Update_GameObject(const _float& fTimeDelta) override;
    virtual void    LateUpdate_GameObject(const _float& fTimeDelta) override;
    virtual void    Render_GameObject() override;

private:
    HRESULT Add_Component();

private:
    CJSBodyBuffer* m_pBufferCom = nullptr;
    CTransform* m_pTransformCom = nullptr;
    CTexture* m_pTextureCom = nullptr;
    _vec3           m_vOffset = {};

public:
    CTransform* Get_Transform() { return m_pTransformCom; }

public:
    static CJSBodyPart* Create(LPDIRECT3DDEVICE9 pGraphicDev, CTransform* pParent, const PartDesc& desc);

private:
    virtual void Free() override;
};

