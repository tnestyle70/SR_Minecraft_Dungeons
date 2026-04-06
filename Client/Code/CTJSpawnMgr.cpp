#include "pch.h"
#include "CTJSpawnMgr.h"
#include "CPlayer.h"
#include "CMonsterMgr.h"
#include "CTJExpOrb.h"
#include "CTJPlayer.h"

IMPLEMENT_SINGLETON(CTJSpawnMgr)

CTJSpawnMgr::CTJSpawnMgr()
{
}

CTJSpawnMgr::~CTJSpawnMgr()
{
    Free();
}

void CTJSpawnMgr::Update(const _float& fTimeDelta)
{
    if (!m_pPlayer || !m_pGraphicDev)
        return;

    if (m_bSpawnStop)
        return;

    // 스폰 타이머
    m_fSpawnTimer += fTimeDelta;
    if (m_fSpawnTimer >= m_fSpawnInterval)
    {
        m_fSpawnTimer = 0.f;
        Spawn_Monster();
    }

    // 몬스터 처치 감지
    for (auto& pair : CMonsterMgr::GetInstance()->Get_MonsterGroups())
    {
        for (auto& pMonster : pair.second.vecMonsters)
        {
            if (pMonster->Is_Dead() && !pMonster->IsActive())
            {
                auto it = find(m_vecDeadMonsters.begin(), m_vecDeadMonsters.end(), pMonster);
                if (it == m_vecDeadMonsters.end())
                {
                    m_vecDeadMonsters.push_back(pMonster);
                    Engine::CTransform* pTrans = dynamic_cast<Engine::CTransform*>(
                        pMonster->Get_Component(ID_DYNAMIC, L"Com_Transform"));
                    if (pTrans)
                    {
                        _vec3 vPos;
                        pTrans->Get_Info(INFO_POS, &vPos);
                        CTJExpOrb* pOrb = CTJExpOrb::Create(m_pGraphicDev, vPos, 1);
                        if (pOrb)
                            m_vecExpOrbs.push_back(pOrb);

                        if (rand() % 10 < 1)
                        {
                            vPos.y = 0.5f;
                            CTJMagnet* pMagnet = CTJMagnet::Create(m_pGraphicDev, vPos);
                            if (pMagnet)
                                m_vecMagnets.push_back(pMagnet);
                        }
                    }
                }
            }
        }

        // 구슬 업데이트 및 흡수
        for (auto& pOrb : m_vecExpOrbs)
            pOrb->Update_GameObject(fTimeDelta);

        // 자석 업데이트
        for (auto& pMagnet : m_vecMagnets)
            pMagnet->Update_GameObject(fTimeDelta);
        m_vecMagnets.erase(
            remove_if(m_vecMagnets.begin(), m_vecMagnets.end(),
                [](CTJMagnet* p) {
                    if (p->Is_Dead()) { Safe_Release(p); return true; }
                    return false;
                }),
            m_vecMagnets.end());

        // 구슬 흡수 시 경험치 획득
        for (auto& pOrb : m_vecExpOrbs)
        {
            if (pOrb->Is_Dead() && m_pTJPlayer)
                m_pTJPlayer->Add_Exp(pOrb->Get_Exp());
        }

        m_vecExpOrbs.erase(
            remove_if(m_vecExpOrbs.begin(), m_vecExpOrbs.end(),
                [](CTJExpOrb* p) {
                    if (p->Is_Dead()) { Safe_Release(p); return true; }
                    return false;
                }),
            m_vecExpOrbs.end());
        }

        // Y값 아래 떨어진 몬스터 처리
        for (auto& pair : CMonsterMgr::GetInstance()->Get_MonsterGroups())
        {
            for (auto& pMonster : pair.second.vecMonsters)
            {
                if (!pMonster->IsActive()) continue;
                Engine::CTransform* pTrans = dynamic_cast<Engine::CTransform*>(
                    pMonster->Get_Component(ID_DYNAMIC, L"Com_Transform"));
                if (!pTrans) continue;
                _vec3 vPos;
                pTrans->Get_Info(INFO_POS, &vPos);

                if (vPos.y < -10.f)
                {
                    static float fFallTimer = 0.f;
                    fFallTimer += fTimeDelta;
                    if (fFallTimer >= 5.f)
                    {
                        fFallTimer = 0.f;
                        pMonster->Take_Damage(100);
                    }
                }
            }
        }
    }

void CTJSpawnMgr::Spawn_Monster()
{

    int iCount = 0;
    for (auto& pair : CMonsterMgr::GetInstance()->Get_MonsterGroups())
        for (auto& pMonster : pair.second.vecMonsters)
            if (pMonster->IsActive()) iCount++;

    if (iCount >= m_iMaxMonsters)
        return;

    _vec3 vPlayerPos;
    m_pPlayer->Get_Transform()->Get_Info(INFO_POS, &vPlayerPos);

    // 플레이어 주변 랜덤
    float fAngle = ((float)(rand() % 360)) * D3DX_PI / 180.f;
    float fDist = m_fSpawnRadius + (float)(rand() % 5); 

    _vec3 vSpawnPos;
    vSpawnPos.x = vPlayerPos.x + cosf(fAngle) * fDist;
    vSpawnPos.y = vPlayerPos.y;
    vSpawnPos.z = vPlayerPos.z + sinf(fAngle) * fDist;

    // 몬스터 종류 랜덤
    int iType = rand() % 4;
    EMonsterType eType;
    switch (iType)
    {
    case 0: eType = EMonsterType::ZOMBIE;
                    break;
    case 1: eType = EMonsterType::SKELETON;
                    break;
    case 2: eType = EMonsterType::CREEPER;
                    break;
    case 3: eType = EMonsterType::SPIDER;
                    break;
    default: eType = EMonsterType::ZOMBIE;
                    break;
    }

    CGameObject* pMonster = CMonster::Create(m_pGraphicDev, eType, vSpawnPos);
    if (pMonster)
        CMonsterMgr::GetInstance()->AddMonster(pMonster, 0, vSpawnPos);
}

void CTJSpawnMgr::Render_Magnet()
{
    for (auto& pMagnet : m_vecMagnets)
        pMagnet->Render_GameObject();
}

void CTJSpawnMgr::Clear()
{
    m_pPlayer = nullptr;
    m_fSpawnTimer = 0.f;
}

void CTJSpawnMgr::Free()
{
}