#include "pch.h"
#include "CTJMagnet.h"
#include "CRenderer.h"
#include "CMonsterMgr.h"
#include "CPlayer.h"
#include "CTJSpawnMgr.h"

CTJMagnet::CTJMagnet(LPDIRECT3DDEVICE9 pGraphicDev)
    : CGameObject(pGraphicDev)
{
}

CTJMagnet::~CTJMagnet()
{
}

HRESULT CTJMagnet::Ready_GameObject(_vec3 vPos)
{
    if (FAILED(Add_Component()))
        return E_FAIL;

    m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);

    return S_OK;
}

_int CTJMagnet::Update_GameObject(const _float& fTimeDelta)
{
    if (m_bDead) return 0;

    _int iExit = CGameObject::Update_GameObject(fTimeDelta);

    CPlayer* pPlayer = CMonsterMgr::GetInstance()->Get_Player();
    if (!pPlayer) return iExit;

    _vec3 vPos, vPlayerPos;
    m_pTransformCom->Get_Info(INFO_POS, &vPos);
    pPlayer->Get_Transform()->Get_Info(INFO_POS, &vPlayerPos);

    _vec3 vDiff = vPlayerPos - vPos;
    vDiff.y = 0.f;
    if (D3DXVec3Length(&vDiff) < m_fPickupRange)
    {
        m_bDead = true;
        // 모든 경험치 오브 흡수
        for (auto& pOrb : CTJSpawnMgr::GetInstance()->Get_ExpOrbs())
        {
            if (!pOrb->Is_Dead())
                pOrb->Set_AbsorbAll();
        }
    }

    m_pColliderCom->Update_AABB(vPos);
    CRenderer::GetInstance()->Add_RenderGroup(RENDER_ALPHA, this);

    return iExit;
}

void CTJMagnet::LateUpdate_GameObject(const _float& fTimeDelta)
{
    CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CTJMagnet::Render_GameObject()
{
    _vec3 vPos;
    m_pTransformCom->Get_Info(INFO_POS, &vPos);

    _matrix matScale, matTrans, matWorld;
    _matrix matView;
    m_pGraphicDev->GetTransform(D3DTS_VIEW, &matView);
    matView._41 = matView._42 = matView._43 = 0.f;
    D3DXMatrixInverse(&matView, NULL, &matView);

    D3DXMatrixScaling(&matScale, 2.f, 2.f, 2.f);
    D3DXMatrixTranslation(&matTrans, vPos.x, vPos.y + 1.f, vPos.z);
    matWorld = matScale * matView * matTrans;

    m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);

    m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    m_pGraphicDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    m_pGraphicDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

    m_pTextureCom->Set_Texture(0);
    m_pBufferCom->Render_Buffer();

    m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
}

HRESULT CTJMagnet::Add_Component()
{
    Engine::CComponent* pComponent = nullptr;

    pComponent = m_pBufferCom = dynamic_cast<Engine::CRcTex*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RcTex"));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Buffer", pComponent });

    pComponent = m_pTextureCom = dynamic_cast<Engine::CTexture*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_MagnetTexture"));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Texture", pComponent });

    pComponent = m_pTransformCom = dynamic_cast<Engine::CTransform*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_DYNAMIC].insert({ L"Com_Transform", pComponent });

    m_pColliderCom = CCollider::Create(m_pGraphicDev,
        _vec3(1.f, 1.f, 1.f), _vec3(0.f, 0.f, 0.f));
    if (!m_pColliderCom) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Collider", m_pColliderCom });

    return S_OK;
}

CTJMagnet* CTJMagnet::Create(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vPos)
{
    CTJMagnet* pMagnet = new CTJMagnet(pGraphicDev);
    if (FAILED(pMagnet->Ready_GameObject(vPos)))
    {
        Safe_Release(pMagnet);
        MSG_BOX("CTJMagnet Create Failed");
        return nullptr;
    }
    return pMagnet;
}

void CTJMagnet::Free()
{
    CGameObject::Free();
}