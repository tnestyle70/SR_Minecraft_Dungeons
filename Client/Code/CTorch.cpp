#include "pch.h"
#include "CTorch.h"
#include "CRenderer.h"

CTorch::CTorch(LPDIRECT3DDEVICE9 pGraphicDev)
    : CGameObject(pGraphicDev)
{
}

CTorch::~CTorch()
{
}

HRESULT CTorch::Ready_GameObject()
{
    if (FAILED(Add_Component()))
        return E_FAIL;

    m_pTransformCom->Set_Pos(0.f, 0.f, 0.f);
    m_pTransformCom->m_vScale = { 0.5f, 1.f, 0.5f };

    return S_OK;
}

_int CTorch::Update_GameObject(const _float& fTimeDelta)
{
    m_fFlicker += fTimeDelta;

    _int iExit = CGameObject::Update_GameObject(fTimeDelta);

    _vec3 vPos;
    m_pTransformCom->Get_Info(INFO_POS, &vPos);
    m_pColliderCom->Update_AABB(vPos);

    CRenderer::GetInstance()->Add_RenderGroup(RENDER_ALPHA, this);

    return iExit;
} 

void CTorch::LateUpdate_GameObject(const _float& fTimeDelta)
{
    CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CTorch::Render_GameObject()
{
    _vec3 vPos;
    m_pTransformCom->Get_Info(INFO_POS, &vPos);

    D3DLIGHT9 tLight;
    ZeroMemory(&tLight, sizeof(tLight));
    tLight.Type = D3DLIGHT_POINT;
    tLight.Position = { vPos.x, vPos.y + 0.5f, vPos.z };
    tLight.Diffuse = { 1.f, 0.6f, 0.1f, 1.f };
    tLight.Ambient = { 0.1f, 0.06f, 0.01f, 1.f };
    tLight.Range = 8.f + sinf(m_fFlicker * 3.f) * 1.5f;
    tLight.Attenuation1 = 0.1f;

    static int s_iLightIdx = 5;
    m_pGraphicDev->SetLight(s_iLightIdx, &tLight);
    m_pGraphicDev->LightEnable(s_iLightIdx, TRUE);

    _matrix matView;
    m_pGraphicDev->GetTransform(D3DTS_VIEW, &matView);

    _matrix matBill;
    D3DXMatrixIdentity(&matBill);
    matBill._11 = matView._11; matBill._13 = matView._31;
    matBill._31 = matView._13; matBill._33 = matView._33;
    matBill._41 = vPos.x;
    matBill._42 = vPos.y + 0.5f;
    matBill._43 = vPos.z;

    _matrix matScale;
    D3DXMatrixScaling(&matScale,
        m_pTransformCom->m_vScale.x,
        m_pTransformCom->m_vScale.y,
        m_pTransformCom->m_vScale.z);

    _matrix matWorld = matScale * matBill;
    m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);

    D3DMATERIAL9 mtrl;
    ZeroMemory(&mtrl, sizeof(mtrl));
    mtrl.Diffuse = { 1.f, 1.f, 1.f, 1.f };
    mtrl.Ambient = { 1.f, 1.f, 1.f, 1.f };
    mtrl.Emissive = { 1.f, 0.8f, 0.3f, 1.f };
    m_pGraphicDev->SetMaterial(&mtrl);

    m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    m_pGraphicDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    m_pGraphicDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
    m_pGraphicDev->SetRenderState(D3DRS_ALPHAREF, 128);
    m_pGraphicDev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
    m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);  

    m_pGraphicDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);


    m_pTextureCom->Set_Texture(0);
    m_pBufferCom->Render_Buffer();

    m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW); 

    D3DMATERIAL9 defaultMtrl;
    ZeroMemory(&defaultMtrl, sizeof(defaultMtrl));
    defaultMtrl.Diffuse = { 1.f, 1.f, 1.f, 1.f };
    defaultMtrl.Ambient = { 1.f, 1.f, 1.f, 1.f };
    defaultMtrl.Emissive = { 0.f, 0.f, 0.f, 0.f };
    m_pGraphicDev->SetMaterial(&defaultMtrl);

    // 추가 - 조명 끄기
    m_pGraphicDev->LightEnable(s_iLightIdx, FALSE);

    // 추가 - 텍스처 스테이지 원상복구
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
}

HRESULT CTorch::Add_Component()
{
    CComponent* pComponent = nullptr;

    pComponent = m_pTransformCom = dynamic_cast<CTransform*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_DYNAMIC].insert({ L"Com_Transform", pComponent });

    pComponent = m_pBufferCom = dynamic_cast<Engine::CRcTex*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RcTex"));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Buffer", pComponent });

    pComponent = m_pTextureCom = dynamic_cast<CTexture*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_TorchTexture"));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Texture", pComponent });

    m_pColliderCom = CCollider::Create(m_pGraphicDev,
        _vec3(0.5f, 1.f, 0.5f), _vec3(0.f, 0.5f, 0.f));
    if (!m_pColliderCom) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Collider", m_pColliderCom });

    return S_OK;
}

CTorch* CTorch::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
    CTorch* pTorch = new CTorch(pGraphicDev);
    if (FAILED(pTorch->Ready_GameObject()))
    {
        Safe_Release(pTorch);
        MSG_BOX("Torch Create Failed");
        return nullptr;
    }
    return pTorch;
}

void CTorch::Free()
{
    CGameObject::Free();
}