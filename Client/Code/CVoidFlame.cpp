#include "pch.h"
#include "CVoidFlame.h"
#include "CRenderer.h"
#include "CBlockMgr.h"
#include "CScreenFX.h"

CVoidFlame::CVoidFlame(LPDIRECT3DDEVICE9 pGraphicDev)
    : CGameObject(pGraphicDev)
{}

CVoidFlame::~CVoidFlame()
{}

HRESULT CVoidFlame::Ready_GameObject()
{
    if (FAILED(Add_Component()))
        return E_FAIL;
    return S_OK;
}

HRESULT CVoidFlame::Add_Component()
{
    Engine::CComponent* pComponent = nullptr;

    pComponent = m_pBufferCom = dynamic_cast<Engine::CRcTex*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RcTex"));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Buffer", pComponent });

    pComponent = m_pTransformCom = dynamic_cast<Engine::CTransform*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_DYNAMIC].insert({ L"Com_Transform", pComponent });

    m_pColliderCom = CCollider::Create(m_pGraphicDev,
        _vec3(5.f, 5.f, 5.f),
        _vec3(0.f, 0.f, 0.f));
    if (!m_pColliderCom) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Collider", m_pColliderCom });

    return S_OK;
}

_int CVoidFlame::Update_GameObject(const _float& fTimeDelta)
{
    _int iExit = CGameObject::Update_GameObject(fTimeDelta);

    if (m_bDead) return -1;

    m_fTime += fTimeDelta;
    m_fLifeTime += fTimeDelta;
    if (m_fLifeTime >= 3.f)
    {
        m_bDead = true;
        return -1;
    }

    _vec3 vPos;
    m_pTransformCom->Get_Info(INFO_POS, &vPos);
    vPos += m_vDir * m_fSpeed * fTimeDelta;
    m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
    m_pColliderCom->Update_AABB(vPos);

    BlockPos tBlockPos = { (int)vPos.x, (int)vPos.y, (int)vPos.z };
    if (CBlockMgr::GetInstance()->HasBlock(tBlockPos))
    {
        m_bDead = true;
        return -1;
    }

    CRenderer::GetInstance()->Add_RenderGroup(RENDER_ALPHA, this);

    return iExit;
}

void CVoidFlame::LateUpdate_GameObject(const _float& fTimeDelta)
{
    CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CVoidFlame::Render_GameObject()
{
    if (m_bDead)
        return;

    _vec3 vPos;
    m_pTransformCom->Get_Info(INFO_POS, &vPos);

    // 빌보드 WVP / WV 계산
    _matrix matView, matProj, matScale, matBill, matTrans, matWorld, matWVP, matWV;
    m_pGraphicDev->GetTransform(D3DTS_VIEW, &matView);
    m_pGraphicDev->GetTransform(D3DTS_PROJECTION, &matProj);
    D3DXMatrixIdentity(&matBill);
    matBill._11 = matView._11; matBill._12 = matView._21; matBill._13 = matView._31;
    matBill._21 = matView._12; matBill._22 = matView._22; matBill._23 = matView._32;
    matBill._31 = matView._13; matBill._32 = matView._23; matBill._33 = matView._33;
    D3DXMatrixScaling(&matScale, 2.6f, 2.6f, 2.6f);
    D3DXMatrixTranslation(&matTrans, vPos.x, vPos.y, vPos.z);
    matWorld = matScale * matBill * matTrans;
    matWVP = matWorld * matView * matProj;
    matWV = matWorld * matView;              // eyeVec Fresnel용 (Proj 제외)

    CScreenFX::GetInstance()->Apply_VoidFlame(matWVP, matWV);

    m_pColliderCom->Render_Collider();
}

CVoidFlame* CVoidFlame::Create(LPDIRECT3DDEVICE9 pGraphicDev,
    const _vec3& vPos, const _vec3& vDir, float fDamage)
{
    CVoidFlame* pFlame = new CVoidFlame(pGraphicDev);
    pFlame->m_vDir = vDir;
    pFlame->m_fDamage = fDamage;

    if (FAILED(pFlame->Ready_GameObject()))
    {
        Safe_Release(pFlame);
        MSG_BOX("CVoidFlame Create Failed");
        return nullptr;
    }

    pFlame->m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);

    return pFlame;
}

void CVoidFlame::Free()
{
    CGameObject::Free();
}
