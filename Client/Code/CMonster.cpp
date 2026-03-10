#include "pch.h"
#include "CMonster.h"
#include "CManagement.h"
#include "CProtoMgr.h"
#include "CRenderer.h"

CMonster::CMonster(LPDIRECT3DDEVICE9 pGraphicDev)
    : CGameObject(pGraphicDev)
{
}

CMonster::CMonster(const CGameObject& rhs)
    : CGameObject(rhs)
{
}

CMonster::~CMonster()
{
}

HRESULT CMonster::Ready_GameObject()
{
    if (FAILED(Add_Component()))
        return E_FAIL;

    m_pTransformCom->Set_Pos(5.f, 10.f, 5.f);

    return S_OK;
}

_int CMonster::Update_GameObject(const _float& fTimeDelta)
{
    _int iExit = CGameObject::Update_GameObject(fTimeDelta);

    CMonsterAnim* pAnim = dynamic_cast<CMonsterAnim*>(m_pBodyCom->Get_Anim());

    // 사망 시 Transform 전체 X축 회전
    if (pAnim && pAnim->Get_DeadRotX() != 0.f)
    {
        D3DXMATRIX matRot, matWorld;
        D3DXMatrixRotationX(&matRot, pAnim->Get_DeadRotX());

        // 위치만 뽑아서 회전행렬에 위치 덮어씌우기
        _vec3 vPos;
        m_pTransformCom->Get_Info(INFO_POS, &vPos);
        matWorld = matRot;
        matWorld._41 = vPos.x;
        matWorld._42 = vPos.y;
        matWorld._43 = vPos.z;

        m_pTransformCom->Set_World(&matWorld);
    }

    // 넉백 처리
    if (pAnim && pAnim->Get_KnockbackDelta() > 0.f)
    {
        _vec3 vPos;
        m_pTransformCom->Get_Info(INFO_POS, &vPos);
        vPos.z -= pAnim->Get_KnockbackDelta();
        m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
    }

    m_pBodyCom->Update_Body(fTimeDelta, m_bIsMoving, false);
    CRenderer::GetInstance()->Add_RenderGroup(RENDER_NONALPHA, this);

    return iExit;
}

void CMonster::LateUpdate_GameObject(const _float& fTimeDelta)
{
    CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CMonster::Render_GameObject()
{
    CMonsterAnim* pAnim = dynamic_cast<CMonsterAnim*>(m_pBodyCom->Get_Anim());

    if (pAnim && pAnim->Is_HitFlash())
    {
        m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
        m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
        m_pGraphicDev->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_XRGB(255, 0, 0));
    }


    m_pBodyCom->Render_Body(m_pTransformCom->Get_World(), m_pTextureCom);
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
}

HRESULT CMonster::Add_Component()
{
    Engine::CComponent* pComponent = nullptr;


    pComponent = m_pTransformCom = dynamic_cast<Engine::CTransform*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_DYNAMIC].insert({ L"Com_Transform", pComponent });


    const _tchar* pTexTag = nullptr;
    switch (m_eType)
    {
    case EMonsterType::ZOMBIE: pTexTag = L"Proto_ZombieTexture"; break;
    default: return E_FAIL;
    }

    pComponent = m_pTextureCom = dynamic_cast<Engine::CTexture*>
        (CProtoMgr::GetInstance()->Clone_Prototype(pTexTag));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Texture", pComponent });


    m_pBodyCom = CMonsterBody::Create(m_pGraphicDev, m_eType);
    if (!m_pBodyCom) return E_FAIL;

    return S_OK;
}

CMonster* CMonster::Create(LPDIRECT3DDEVICE9 pGraphicDev, EMonsterType eType)
{
    CMonster* pMonster = new CMonster(pGraphicDev);
    pMonster->m_eType = eType;

    if (FAILED(pMonster->Ready_GameObject()))
    {
        Safe_Release(pMonster);
        MSG_BOX("pMonster Create Failed");
        return nullptr;
    }

    return pMonster;
}

void CMonster::Free()
{
    Safe_Release(m_pBodyCom);
    CGameObject::Free();
}