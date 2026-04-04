#include "pch.h"
#include "CTJExpOrb.h"
#include "CRenderer.h"
#include "CMonsterMgr.h"
#include "CPlayer.h"
#include "CSoundMgr.h"

CTJExpOrb::CTJExpOrb(LPDIRECT3DDEVICE9 pGraphicDev)
    : CGameObject(pGraphicDev)
{
}

CTJExpOrb::~CTJExpOrb()
{
}

HRESULT CTJExpOrb::Ready_GameObject(_vec3 vPos, int iExp)
{
    if (FAILED(Add_Component()))
        return E_FAIL;

    m_iExp = iExp;
    m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
    m_pTransformCom->m_vScale = { 1.f, 1.f, 1.f };

    return S_OK;
}

_int CTJExpOrb::Update_GameObject(const _float& fTimeDelta)
{
    if (m_bDead)
        return 0;

    _int iExit = CGameObject::Update_GameObject(fTimeDelta);

    CPlayer* pPlayer = CMonsterMgr::GetInstance()->Get_Player();
    if (!pPlayer)
        return iExit;

    _vec3 vPos, vPlayerPos;
    m_pTransformCom->Get_Info(INFO_POS, &vPos);
    pPlayer->Get_Transform()->Get_Info(INFO_POS, &vPlayerPos);

    _vec3 vDir = vPlayerPos - vPos;
    float fDist = D3DXVec3Length(&vDir);

    // 흡수 범위 안이면 플레이어 쪽으로 이동
    if (fDist < m_fAbsorbRange)
    {
        if (fDist < 0.5f)
        {
            m_bDead = true;
            CSoundMgr::GetInstance()->PlayEffect(L"Emerald/sfx_item_emeraldBurstOutPing-001_soundWave.wav", 1.0f);
            return iExit;
        }
        D3DXVec3Normalize(&vDir, &vDir);
        vPos += vDir * m_fMoveSpeed * fTimeDelta;
        m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
    }

    m_pColliderCom->Update_AABB(vPos);
    CRenderer::GetInstance()->Add_RenderGroup(RENDER_ALPHA, this);

    return iExit;
}

void CTJExpOrb::LateUpdate_GameObject(const _float& fTimeDelta)
{
    CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CTJExpOrb::Render_GameObject()
{
    _vec3 vPos;
    m_pTransformCom->Get_Info(INFO_POS, &vPos);

    _matrix matScale, matTrans, matWorld;
    D3DXMatrixScaling(&matScale, 1.f, 1.f, 1.f);
    D3DXMatrixTranslation(&matTrans, vPos.x, vPos.y + 0.5f, vPos.z);
    matWorld = matScale * matTrans;

    m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);

    m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    m_pGraphicDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    m_pGraphicDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
    m_pGraphicDev->SetRenderState(D3DRS_ALPHAREF, 128);
    m_pGraphicDev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);

    m_pTextureCom->Set_Texture(0);
    m_pBufferCom->Render_Buffer();

    m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
}

HRESULT CTJExpOrb::Add_Component()
{
    Engine::CComponent* pComponent = nullptr;

    pComponent = m_pBufferCom = dynamic_cast<Engine::CRcTex*>(
        CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RcTex"));
    if (!pComponent)
        return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Buffer", pComponent });

    pComponent = m_pTextureCom = dynamic_cast<Engine::CTexture*>(
        CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_ExpOrbTexture"));
    if (!pComponent)
        return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Texture", pComponent });

    pComponent = m_pTransformCom = dynamic_cast<Engine::CTransform*>(
        CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));
    if (!pComponent)
        return E_FAIL;
    m_mapComponent[ID_DYNAMIC].insert({ L"Com_Transform", pComponent });

    m_pColliderCom = CCollider::Create(m_pGraphicDev,
        _vec3(0.5f, 0.5f, 0.5f), _vec3(0.f, 0.f, 0.f));
    if (!m_pColliderCom)
        return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Collider", m_pColliderCom });

    return S_OK;
}

CTJExpOrb* CTJExpOrb::Create(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vPos, int iExp)
{
    CTJExpOrb* pOrb = new CTJExpOrb(pGraphicDev);

    if (FAILED(pOrb->Ready_GameObject(vPos, iExp)))
    {
        Safe_Release(pOrb);
        MSG_BOX("CTJExpOrb Create Failed");
        return nullptr;
    }

    return pOrb;
}

void CTJExpOrb::Free()
{
    CGameObject::Free();
}