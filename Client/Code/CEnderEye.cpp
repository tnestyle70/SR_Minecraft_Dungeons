#include "pch.h"
#include "CEnderEye.h"
#include "CRenderer.h"
#include "CPlayerArrow.h"

CEnderEye::CEnderEye(LPDIRECT3DDEVICE9 pGraphicDev)
    : CGameObject(pGraphicDev)
    , m_pBufferCom(nullptr)
    , m_pTransformCom(nullptr)
    , m_pTextureCom(nullptr)
    , m_pColliderCom(nullptr)
{
}

CEnderEye::~CEnderEye()
{
}

HRESULT CEnderEye::Ready_GameObject()
{
    if (FAILED(Add_Component()))
        return E_FAIL;

    m_pTransformCom->Set_Scale(2.f);

    return S_OK;
}

_int CEnderEye::Update_GameObject(const _float& fTimeDelta)
{
    _int iExit = CGameObject::Update_GameObject(fTimeDelta);

    _vec3 vPos;
    m_pTransformCom->Get_Info(INFO_POS, &vPos);
    m_pColliderCom->Update_AABB(vPos);

    if (m_bFlickering)
    {
        // 알파 깜빡임
        m_fFlickerTime += fTimeDelta;
        m_fAlpha = (sinf(m_fFlickerTime * 5.f) + 1.f) * 0.5f;

        // 화살 충돌 체크
        Check_ArrowCollision();
    }

    CRenderer::GetInstance()->Add_RenderGroup(RENDER_ALPHA, this);

    return iExit;
}

void CEnderEye::Check_ArrowCollision()
{
    if (!m_pPlayer) return;

    const vector<CPlayerArrow*>& vecArrows = m_pPlayer->Get_Arrows();

    for (auto* pArrow : vecArrows)
    {
        if (!pArrow || pArrow->Is_Dead()) continue;

        CCollider* pArrowCol = dynamic_cast<CCollider*>
            (pArrow->Get_Component(ID_STATIC, L"Com_Collider"));
        if (!pArrowCol) continue;

        if (m_pColliderCom->IsColliding(pArrowCol->Get_AABB()))
        {
            // 충돌 → 깜빡임 OFF
            m_bFlickering = false;
            m_fAlpha = 1.f;
            m_fFlickerTime = 0.f;
            return;
        }
    }
}

void CEnderEye::LateUpdate_GameObject(const _float& fTimeDelta)
{
    CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CEnderEye::Render_GameObject()
{
    m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    m_pGraphicDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    m_pGraphicDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

    // TEXTUREFACTOR로 알파값 직접 설정
    DWORD dwAlpha = (DWORD)(m_fAlpha * 255.f);
    m_pGraphicDev->SetRenderState(D3DRS_TEXTUREFACTOR,
        D3DCOLOR_ARGB(dwAlpha, 255, 255, 255));

    m_pGraphicDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);

    m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);

    _matrix matWorld = *m_pTransformCom->Get_World();
    _matrix matView;
    m_pGraphicDev->GetTransform(D3DTS_VIEW, &matView);
    matView._41 = matView._42 = matView._43 = 0.f;
    D3DXMatrixInverse(&matView, NULL, &matView);

    _vec3 vScale = m_pTransformCom->Get_Scale();
    matWorld._11 = matView._11 * vScale.x;
    matWorld._12 = matView._12 * vScale.x;
    matWorld._13 = matView._13 * vScale.x;
    matWorld._21 = matView._21 * vScale.y;
    matWorld._22 = matView._22 * vScale.y;
    matWorld._23 = matView._23 * vScale.y;
    matWorld._31 = matView._31 * vScale.z;
    matWorld._32 = matView._32 * vScale.z;
    matWorld._33 = matView._33 * vScale.z;

    m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);
    m_pTextureCom->Set_Texture(0);
    m_pBufferCom->Render_Buffer();

    // 원래 상태 복구
    m_pGraphicDev->SetRenderState(D3DRS_TEXTUREFACTOR, 0xFFFFFFFF);
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}
HRESULT CEnderEye::Add_Component()
{
    CComponent* pComponent = nullptr;

    pComponent = m_pBufferCom = dynamic_cast<CRcTex*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RcTex"));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Buffer", pComponent });

    pComponent = m_pTextureCom = dynamic_cast<CTexture*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_EnderEyeTexture"));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Texture", pComponent });

    pComponent = m_pTransformCom = dynamic_cast<CTransform*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_DYNAMIC].insert({ L"Com_Transform", pComponent });

    m_pColliderCom = CCollider::Create(m_pGraphicDev,
        _vec3{ 3.0f, 3.0f, 3.0f }, _vec3{ 0.f, 0.f, 0.f });
    if (!m_pColliderCom) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Collider", m_pColliderCom });

    return S_OK;
}

CEnderEye* CEnderEye::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
    CEnderEye* pEnderEye = new CEnderEye(pGraphicDev);
    if (FAILED(pEnderEye->Ready_GameObject()))
    {
        Safe_Release(pEnderEye);
        MSG_BOX("EnderEye Create Failed");
        return nullptr;
    }
    return pEnderEye;
}

void CEnderEye::Free()
{
    CGameObject::Free();
}