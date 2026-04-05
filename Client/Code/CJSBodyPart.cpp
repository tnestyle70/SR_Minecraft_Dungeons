#include "pch.h"
#include "CJSBodyPart.h"
#include "CRenderer.h"

CJSBodyPart::CJSBodyPart(LPDIRECT3DDEVICE9 pGraphicDev)
    : CGameObject(pGraphicDev)
    , m_pBufferCom(nullptr)
    , m_pTransformCom(nullptr)
    , m_pTextureCom(nullptr)
{
}

CJSBodyPart::~CJSBodyPart()
{
}

HRESULT CJSBodyPart::Ready_GameObject(CTransform* pParent, const PartDesc& desc)
{
    m_vOffset = desc.vOffset;
    m_pTexProto = desc.pTexProto;

    if (FAILED(Add_Component()))
        return E_FAIL;

    m_pTransformCom->Set_Parent(pParent);
    m_pTransformCom->Set_Pos(desc.vOffset.x, desc.vOffset.y, desc.vOffset.z);

    m_pBufferCom->Set_SizeAndUVs(
        desc.fSizeX, desc.fSizeY, desc.fSizeZ,
        desc.front, desc.back,
        desc.left, desc.right,
        desc.top, desc.bottom);

    return S_OK;
}

_int CJSBodyPart::Update_GameObject(const _float& fTimeDelta)
{
    return CGameObject::Update_GameObject(fTimeDelta);
}

void CJSBodyPart::LateUpdate_GameObject(const _float& fTimeDelta)
{
    CGameObject::LateUpdate_GameObject(fTimeDelta);
    CRenderer::GetInstance()->Add_RenderGroup(RENDER_NONALPHA, this);
}

void CJSBodyPart::Render_GameObject()
{
    m_pGraphicDev->SetTransform(D3DTS_WORLD, m_pTransformCom->Get_World());
    m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    m_pTextureCom->Set_Texture(0);
    m_pBufferCom->Render_Buffer();
    m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}

HRESULT CJSBodyPart::Add_Component()
{
    CComponent* pComponent = nullptr;

    // Buffer
    m_pBufferCom = CJSBodyBuffer::Create(m_pGraphicDev);
    if (!m_pBufferCom) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Buffer", m_pBufferCom });

    // Transform
    pComponent = m_pTransformCom = dynamic_cast<CTransform*>(
        CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_DYNAMIC].insert({ L"Com_Transform", pComponent });

    // Texture
    pComponent = m_pTextureCom = dynamic_cast<CTexture*>(
        CProtoMgr::GetInstance()->Clone_Prototype(m_pTexProto));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Texture", pComponent });

    return S_OK;
}

CJSBodyPart* CJSBodyPart::Create(LPDIRECT3DDEVICE9 pGraphicDev, CTransform* pParent, const PartDesc& desc)
{
    CJSBodyPart* pPart = new CJSBodyPart(pGraphicDev);
    if (FAILED(pPart->Ready_GameObject(pParent, desc)))
    {
        Safe_Release(pPart);
        MSG_BOX("JSBodyPart Create Failed");
        return nullptr;
    }
    return pPart;
}

void CJSBodyPart::Free()
{
    CGameObject::Free();
}