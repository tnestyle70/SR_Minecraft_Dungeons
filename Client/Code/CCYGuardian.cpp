#include "pch.h"
#include "CCYGuardian.h"
#include "CRenderer.h"
#include "CSoundMgr.h"

CCYGuardian::CCYGuardian(LPDIRECT3DDEVICE9 pGraphicDev)
    : CAncientGuardian(pGraphicDev)
{
}

CCYGuardian::~CCYGuardian()
{
}

_int CCYGuardian::Update_GameObject(const _float& fTimeDelta)
{
    if (!m_bActive) return 0;

    Update_CCY_AI(fTimeDelta);

    CGameObject::Update_GameObject(fTimeDelta);

    if (m_pBodyCom)
        m_pBodyCom->Update_Body(fTimeDelta, true, false);

    CRenderer::GetInstance()->Add_RenderGroup(RENDER_NONALPHA, this);

    return 0;
}

void CCYGuardian::LateUpdate_GameObject(const _float& fTimeDelta)
{
    if (!m_bActive) return;
    CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CCYGuardian::Render_GameObject()
{
    if (!m_bActive) return;

    for (auto* pBeam : m_vecBeams)
    {
        if (pBeam && !pBeam->Is_Dead())
            pBeam->Render_GameObject();
    }

    CAncientGuardian::Render_GameObject();
}   

void CCYGuardian::Take_Damage(int iDamage)
{
    if (m_iHp <= 0) return;
    m_iHp -= iDamage;

    if (m_iHp <= 0)
    {
        m_iHp = 0;
        m_bActive = false;
        CSoundMgr::GetInstance()->PlayEffect(L"Monster/AG_DEAD.wav", 1.f);
    }
    else
    {
        CSoundMgr::GetInstance()->PlayEffect(L"Monster/AG_Attack1.wav", 1.f);
    }
}


void CCYGuardian::Update_CCY_AI(const _float& fTimeDelta)
{
    Engine::CTransform* pTrans = dynamic_cast<Engine::CTransform*>
        (Get_Component(ID_DYNAMIC, L"Com_Transform"));
    if (!pTrans) return;

    _vec3 vMyPos;
    pTrans->Get_Info(INFO_POS, &vMyPos);

    // Y축은 카메라 Eye 높이로 서서히 맞춤
    float fTargetY = m_vTargetPos.y + 2.f;
    vMyPos.y += (fTargetY - vMyPos.y) * 2.f * fTimeDelta;

    // 수평 이동
    _vec3 vDir = m_vTargetPos - vMyPos;
    vDir.y = 0.f;
    float fDist = D3DXVec3Length(&vDir);

    if (fDist > 2.f)
    {
        D3DXVec3Normalize(&vDir, &vDir);
        vMyPos.x += vDir.x * m_fMoveSpeed * fTimeDelta;
        vMyPos.z += vDir.z * m_fMoveSpeed * fTimeDelta;
        pTrans->m_vAngle.y = D3DXToDegree(atan2f(vDir.x, vDir.z)) + 180.f;
    }

    pTrans->Set_Pos(vMyPos.x, vMyPos.y, vMyPos.z);

    // 콜라이더를 앞머리 위치로 갱신
    CCollider* pCol = dynamic_cast<CCollider*>
        (Get_Component(ID_STATIC, L"Com_Collider"));
    if (pCol)
    {
        _vec3 vLook;
        pTrans->Get_Info(INFO_LOOK, &vLook);
        D3DXVec3Normalize(&vLook, &vLook);
        _vec3 vHeadPos = vMyPos + vLook * 2.f;
        vHeadPos.y += 1.5f;
        pCol->Update_AABB(vHeadPos);
    }

    // Idle 사운드 타이머
    m_fIdleSoundTimer += fTimeDelta;
    if (m_fIdleSoundTimer >= m_fIdleSoundInterval)
    {
        m_fIdleSoundTimer = 0.f;
        CSoundMgr::GetInstance()->PlayEffect(L"Monster/AG_IDLE.wav", 1.f);
    }

    // 공격 타이머 - 빔 발사
    m_fAtkTimer += fTimeDelta;
    if (m_fAtkTimer >= m_fAtkInterval)
    {
        m_fAtkTimer = 0.f;
        CSoundMgr::GetInstance()->PlayEffect(L"Monster/AG_Attack1.wav", 1.f);

        _vec3 vLook;
        pTrans->Get_Info(INFO_LOOK, &vLook);
        _vec3 vHeadPos = vMyPos + vLook * 1.5f;
        vHeadPos.y += 1.5f;

        _vec3 vFireDir = m_vTargetPos - vHeadPos;
        D3DXVec3Normalize(&vFireDir, &vFireDir);

        CBeam* pBeam = CBeam::Create(m_pGraphicDev, vHeadPos, vFireDir);
        if (pBeam)
            m_vecBeams.push_back(pBeam);
    }

    // 빔 업데이트
    for (auto iter = m_vecBeams.begin(); iter != m_vecBeams.end();)
    {
        (*iter)->Update_GameObject(fTimeDelta);
        (*iter)->LateUpdate_GameObject(fTimeDelta);

        if ((*iter)->Is_Dead())
        {
            Safe_Release(*iter);
            iter = m_vecBeams.erase(iter);
        }
        else
            ++iter;
    }
}

CCYGuardian* CCYGuardian::Create(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vPos)
{
    CCYGuardian* pGuardian = new CCYGuardian(pGraphicDev);

    if (FAILED(pGuardian->Ready_GameObject()))
    {
        Safe_Release(pGuardian);
        MSG_BOX("CCYGuardian Create Failed");
        return nullptr;
    }

    Engine::CTransform* pTrans = dynamic_cast<Engine::CTransform*>
        (pGuardian->Get_Component(ID_DYNAMIC, L"Com_Transform"));
    if (pTrans)
        pTrans->Set_Pos(vPos.x, vPos.y, vPos.z);

    return pGuardian;
}

void CCYGuardian::Free()
{
    for (auto* pBeam : m_vecBeams)
        Safe_Release(pBeam);
    m_vecBeams.clear();

    CAncientGuardian::Free();
}