#include "pch.h"
#include "CTJBoss.h"
#include "CDynamicCamera.h"
#include "CMonsterMgr.h"
#include "CRenderer.h"
#include "CPlayer.h"

CTJBoss::CTJBoss(LPDIRECT3DDEVICE9 pGraphicDev)
    : CGameObject(pGraphicDev)
{
}

CTJBoss::~CTJBoss()
{
}

HRESULT CTJBoss::Ready_GameObject(_vec3 vPos)
{
    if (FAILED(Add_Component(vPos)))
        return E_FAIL;

    m_pTransformCom->Set_Scale(3.f); // 거대 좀비
    m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);

    return S_OK;
}

_int CTJBoss::Update_GameObject(const _float& fTimeDelta)
{
    if (m_bDead)
        return 0;

    _int iExit = CGameObject::Update_GameObject(fTimeDelta);

    Apply_Gravity(fTimeDelta);
    Update_AI(fTimeDelta);

    if (m_bHitCooldown)
    {
        m_fHitTimer += fTimeDelta;
        if (m_fHitTimer >= m_fHitInterval)
        {
            m_fHitTimer = 0.f;
            m_bHitCooldown = false;
        }
    }

    _vec3 vPos;
    m_pTransformCom->Get_Info(INFO_POS, &vPos);
    m_pColliderCom->Update_AABB(vPos);

    CRenderer::GetInstance()->Add_RenderGroup(RENDER_NONALPHA, this);

    return iExit;
}

void CTJBoss::LateUpdate_GameObject(const _float& fTimeDelta)
{
    CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CTJBoss::Render_GameObject()
{
    if (m_bDead)
        return;

    m_pBodyCom->Render_Body(m_pTransformCom->Get_World(), m_pTextureCom);
    m_pColliderCom->Render_Collider();
}

void CTJBoss::Take_Damage(_float fDamage)
{
    if (m_bDead || m_bHitCooldown) return;
    m_fHp -= fDamage;
    if (m_fHp <= 0.f)
    {
        m_fHp = 0.f;
        m_bDead = true;
    }
    m_bHitCooldown = true;
}

void CTJBoss::Update_AI(const _float& fTimeDelta)
{
    CPlayer* pPlayer = CMonsterMgr::GetInstance()->Get_Player();
    if (!pPlayer)
        return;

    _vec3 vPos, vPlayerPos;
    m_pTransformCom->Get_Info(INFO_POS, &vPos);
    pPlayer->Get_Transform()->Get_Info(INFO_POS, &vPlayerPos);

    _vec3 vDir = vPlayerPos - vPos;
    vDir.y = 0.f;
    _float fDist = D3DXVec3Length(&vDir);

    switch (m_eState)
    {
    case ETJBossState::WALK:
    {
        // 플레이어 방향으로 이동
        if (fDist > 3.f)
        {
            D3DXVec3Normalize(&vDir, &vDir);
            m_pTransformCom->m_vAngle.y = D3DXToDegree(atan2f(vDir.x, vDir.z));
            m_pTransformCom->Move_Pos(&vDir, m_fMoveSpeed, fTimeDelta);
            m_fWalkTime += fTimeDelta;
        }

        // 점프 타이머
        m_fJumpTimer += fTimeDelta;
        _float fCurInterval = (m_fHp < m_fMaxHp * 0.5f) ? m_fJumpInterval * 0.5f : m_fJumpInterval;
        if (m_fJumpTimer >= fCurInterval)
        {
            m_fJumpTimer = 0.f;
            m_vJumpTarget = vPlayerPos;
            m_fVelocityY = 15.f;
            m_bOnGround = false;
            m_eState = ETJBossState::JUMP;
        }
        break;
    }
    case ETJBossState::JUMP:
    {
        // 목표 방향으로 수평 이동
        _vec3 vJumpDir = m_vJumpTarget - vPos;
        vJumpDir.y = 0.f;
        _float fJumpDist = D3DXVec3Length(&vJumpDir);
        if (fJumpDist > 0.1f)
        {
            D3DXVec3Normalize(&vJumpDir, &vJumpDir);
            m_pTransformCom->Move_Pos(&vJumpDir, m_fMoveSpeed * 2.f, fTimeDelta);
        }

        if (m_bOnGround)
        {
            m_eState = ETJBossState::LAND;

            // 착지 범위 데미지
            _vec3 vLandPos;
            m_pTransformCom->Get_Info(INFO_POS, &vLandPos);
            _vec3 vPlayerDiff = vPlayerPos - vLandPos;
            vPlayerDiff.y = 0.f;
            if (D3DXVec3Length(&vPlayerDiff) < m_fLandDmgRadius)
                pPlayer->Hit(m_fLandDmg);

            // 카메라 쉐이킹
            if (m_pCamera)
                m_pCamera->Start_Shake(0.5f, 1.5f);
        }
        break;
    }
    case ETJBossState::LAND:
    {
        // 0.5초 후 WALK로
        m_fJumpTimer += fTimeDelta;
        if (m_fJumpTimer >= 0.5f)
        {
            m_fJumpTimer = 0.f;
            m_eState = ETJBossState::WALK;
        }
        break;
    }
    case ETJBossState::IDLE:
        m_eState = ETJBossState::WALK;
        break;
    default:
        break;
    }
}

void CTJBoss::Apply_Gravity(const _float& fTimeDelta)
{
    m_fVelocityY += -20.f * fTimeDelta;
    if (m_fVelocityY < -20.f) m_fVelocityY = -20.f;

    _vec3 vPos;
    m_pTransformCom->Get_Info(INFO_POS, &vPos);
    vPos.y += m_fVelocityY * fTimeDelta;

    if (vPos.y <= 0.f)
    {
        vPos.y = 0.f;
        m_fVelocityY = 0.f;
        m_bOnGround = true;
    }

    m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
}

HRESULT CTJBoss::Add_Component(_vec3 vPos)
{
    Engine::CComponent* pComponent = nullptr;

    pComponent = m_pTransformCom = dynamic_cast<Engine::CTransform*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));
    if (!pComponent)
        return E_FAIL;
    m_mapComponent[ID_DYNAMIC].insert({ L"Com_Transform", pComponent });

    pComponent = m_pTextureCom = dynamic_cast<Engine::CTexture*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_ZombieTexture"));
    if (!pComponent)
        return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Texture", pComponent });

    m_pBodyCom = CMonsterBody::Create(m_pGraphicDev, EMonsterType::ZOMBIE);
    if (!m_pBodyCom)
        return E_FAIL;

    m_pColliderCom = CCollider::Create(m_pGraphicDev,
        _vec3(3.f, 9.f, 3.f),
        _vec3(0.f, 4.f, 0.f));
    if (!m_pColliderCom)
        return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Collider", m_pColliderCom });

    return S_OK;
}

CTJBoss* CTJBoss::Create(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vPos)
{
    CTJBoss* pBoss = new CTJBoss(pGraphicDev);
    if (FAILED(pBoss->Ready_GameObject(vPos)))
    {
        Safe_Release(pBoss);
        MSG_BOX("CTJBoss Create Failed");
        return nullptr;
    }
    return pBoss;
}

void CTJBoss::Free()
{
    Safe_Release(m_pBodyCom);
    Safe_Release(m_pColliderCom);
    CGameObject::Free();
}