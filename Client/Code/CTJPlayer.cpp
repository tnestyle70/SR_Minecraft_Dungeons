#include "pch.h"
#include "CTJPlayer.h"
#include "CTNT.h"
#include "CDInputMgr.h"
#include "CSoundMgr.h"
#include "CMonsterMgr.h"
#include "CTJSpawnMgr.h"

CTJPlayer::CTJPlayer(LPDIRECT3DDEVICE9 pGraphicDev)
    : CPlayer(pGraphicDev)
{
}

CTJPlayer::~CTJPlayer()
{
}

HRESULT CTJPlayer::Ready_GameObject()
{
    if (FAILED(CPlayer::Ready_GameObject()))
        return E_FAIL;

    // 칼날 버퍼
    m_pBladeBufferCom = dynamic_cast<Engine::CRcTex*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RcTex"));
    if (nullptr == m_pBladeBufferCom)
        return E_FAIL;

    // 칼날 텍스처
    m_pBladeTextureCom = dynamic_cast<Engine::CTexture*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_BladeEffectTexture"));
    if (nullptr == m_pBladeTextureCom)
        return E_FAIL;

    // 번개 버퍼
    m_pThunderBufferCom = dynamic_cast<Engine::CRcTex*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RcTex"));
    if (nullptr == m_pThunderBufferCom)
        return E_FAIL;

    // 번개 텍스처
    m_pLightningTextureCom = dynamic_cast<Engine::CTexture*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_LightningTexture"));
    if (nullptr == m_pLightningTextureCom)
        return E_FAIL;

    // 스파크 텍스처
    m_pSparkTextureCom = dynamic_cast<Engine::CTexture*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_SparkTexture"));
    if (nullptr == m_pSparkTextureCom)
        return E_FAIL;

    // 화염 버퍼
    m_pFireBufferCom = dynamic_cast<Engine::CRcTex*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RcTex"));
    if (nullptr == m_pFireBufferCom)
        return E_FAIL;

    // 화염 텍스처
    m_pFireTextureCom = dynamic_cast<Engine::CTexture*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_FireTrailEffectTexture"));
    if (nullptr == m_pFireTextureCom)
        return E_FAIL;

    return S_OK;
}

_int CTJPlayer::Update_GameObject(const _float& fTimeDelta)
{
    _int iExit = CPlayer::Update_GameObject(fTimeDelta);

    // TNT 오라
    if (m_bTNTAura)
    {
        m_fTNTTimer += fTimeDelta;
        if (m_fTNTTimer >= m_fTNTInterval)
        {
            m_fTNTTimer = 0.f;

            _vec3 vPos;
            Get_Transform()->Get_Info(INFO_POS, &vPos);
            vPos.y += 1.5f;

            // 랜덤 방향
            float fAngle = ((float)(rand() % 360)) * D3DX_PI / 180.f;
            _vec3 vDir = { cosf(fAngle), 0.f, sinf(fAngle) };
            D3DXVec3Normalize(&vDir, &vDir);

            CTNT* pTNT = CTNT::Create(m_pGraphicDev, vPos);
            if (pTNT)
            {
                pTNT->Throw(vPos, vDir, 8.f);
                m_vecTNTAura.push_back(pTNT);
                Add_TNT(pTNT);
            }
        }
    }

    // TNT 업데이트 및 삭제
    for (auto& pTNT : m_vecTNTAura)
        pTNT->Update_GameObject(fTimeDelta);

    m_vecTNTAura.erase(
        remove_if(m_vecTNTAura.begin(), m_vecTNTAura.end(),
            [](CTNT* p) {
                return p->Is_Dead() && !p->Is_Exploding();
            }),
        m_vecTNTAura.end());

    //체력회복
    m_fRegenTimer += fTimeDelta;
    if (m_fRegenTimer >= m_fRegenInterval)
    {
        m_fRegenTimer = 0.f;
        float fHp = Get_Hp();
        float fMax = Get_MaxHp();
        Set_Hp(min(fHp + m_fRegenAmount, fMax));
    }

    // 화살 - 보스 충돌
    if (m_pTJBoss && !m_pTJBoss->Is_Dead())
    {
        CCollider* pBossCol = m_pTJBoss->Get_Collider();
        if (pBossCol)
        {
            AABB tBossAABB = pBossCol->Get_AABB();
            for (auto& pArrow : m_vecArrows)
            {
                if (!pArrow || pArrow->Is_Dead())
                    continue;
                CCollider* pArrowCol = pArrow->Get_Collider();
                if (!pArrowCol)
                    continue;
                AABB tArrowAABB = pArrowCol->Get_AABB();
                if (tBossAABB.vMin.x <= tArrowAABB.vMax.x && tBossAABB.vMax.x >= tArrowAABB.vMin.x &&
                    tBossAABB.vMin.y <= tArrowAABB.vMax.y && tBossAABB.vMax.y >= tArrowAABB.vMin.y &&
                    tBossAABB.vMin.z <= tArrowAABB.vMax.z && tBossAABB.vMax.z >= tArrowAABB.vMin.z)
                {
                    m_pTJBoss->Take_Damage(pArrow->Get_Damage());
                    pArrow->Set_Dead();
                }
            }
        }
    }

    // 번개
    if (m_bLightning)
    {
        m_fLightningTimer += fTimeDelta;
        if (m_fLightningTimer >= m_fLightningInterval)
        {
            m_fLightningTimer = 0.f;

            vector<CMonster*> vecActive;
            for (auto& pair : CMonsterMgr::GetInstance()->Get_MonsterGroups())
                for (auto& pMonster : pair.second.vecMonsters)
                    if (pMonster->IsActive())
                        vecActive.push_back(pMonster);

            bool bTargetBoss = m_pTJBoss && !m_pTJBoss->Is_Dead() &&
                (vecActive.empty() || rand() % 2 == 0);

            if (bTargetBoss)
            {
                CCollider* pCol = m_pTJBoss->Get_Collider();
                if (pCol)
                {
                    AABB tAABB = pCol->Get_AABB();
                    m_vThunderPos = (tAABB.vMin + tAABB.vMax) * 0.5f;
                    m_pTJBoss->Take_Damage(30.f);
                    m_bThunderEffect = true;
                    m_fThunderEffectTimer = 0.f;
                    CSoundMgr::GetInstance()->PlayEffect(L"Player/ThunderSound.wav", 1.f);
                }
            }
            else if (!vecActive.empty())
            {
                CMonster* pTarget = vecActive[rand() % vecActive.size()];
                CCollider* pCol = pTarget->Get_Collider();
                if (pCol)
                {
                    AABB tAABB = pCol->Get_AABB();
                    m_vThunderPos = (tAABB.vMin + tAABB.vMax) * 0.5f;
                    pTarget->Take_Damage(30);
                    m_bThunderEffect = true;
                    m_fThunderEffectTimer = 0.f;
                    CSoundMgr::GetInstance()->PlayEffect(L"Player/ThunderSound.wav", 1.f);
                }
            }
        }
    }

    //번개 이펙트
    if (m_bThunderEffect)
    {
        m_fThunderEffectTimer += fTimeDelta;
        m_fFrameTimer += fTimeDelta;

        if (m_fFrameTimer >= m_fFrameInterval)
        {
            m_fFrameTimer = 0.f;
            m_iLightningFrame++;
            m_iSparkFrame++;
        }

        if (m_fThunderEffectTimer >= m_fThunderEffectDuration)
        {
            m_bThunderEffect = false;
            m_fThunderEffectTimer = 0.f;
            m_iLightningFrame = 0;
            m_iSparkFrame = 0;
            m_fFrameTimer = 0.f;
        }
    }

    // 화염 장판
    if (m_bFireTrail && m_bMoving)
    {
        m_fFireSpawnTimer += fTimeDelta;
        if (m_fFireSpawnTimer >= m_fFireSpawnInterval)
        {
            m_fFireSpawnTimer = 0.f;
            _vec3 vPos;
            Get_Transform()->Get_Info(INFO_POS, &vPos);
            vPos.y += 0.1f;
            TJFireTrail tFire;
            tFire.vPos = vPos;
            m_vecFireTrails.push_back(tFire);
        }
    }

    // 화염 업데이트
    for (auto& tFire : m_vecFireTrails)
    {
        tFire.fLifeTime -= fTimeDelta;
        tFire.fDmgTimer += fTimeDelta;

        if (tFire.fDmgTimer >= 0.5f)
        {
            tFire.fDmgTimer = 0.f;
            for (auto& pair : CMonsterMgr::GetInstance()->Get_MonsterGroups())
            {
                for (auto& pMonster : pair.second.vecMonsters)
                {
                    if (!pMonster->IsActive())
                        continue;
                    CCollider* pCol = pMonster->Get_Collider();
                    if (!pCol)
                        continue;
                    AABB tAABB = pCol->Get_AABB();
                    _vec3 vCenter = (tAABB.vMin + tAABB.vMax) * 0.5f;
                    _vec3 vDiff = vCenter - tFire.vPos;
                    vDiff.y = 0.f;
                    if (D3DXVec3Length(&vDiff) < 2.f)
                        pMonster->Take_Damage((int)m_fFireDamage);

                    if (m_pTJBoss && !m_pTJBoss->Is_Dead())
                    {
                        CCollider* pCol = m_pTJBoss->Get_Collider();
                        if (pCol)
                        {
                            AABB tAABB = pCol->Get_AABB();
                            _vec3 vCenter = (tAABB.vMin + tAABB.vMax) * 0.5f;
                            _vec3 vDiff = vCenter - tFire.vPos;
                            vDiff.y = 0.f;
                            if (D3DXVec3Length(&vDiff) < 2.f)
                                m_pTJBoss->Take_Damage(m_fFireDamage);
                        }
                    }
                }
            }
        }
    }

    //화염 제거
    m_vecFireTrails.erase(
        remove_if(m_vecFireTrails.begin(), m_vecFireTrails.end(),
            [](const TJFireTrail& t) { return t.fLifeTime <= 0.f; }),
        m_vecFireTrails.end());

    // 회전 칼날
    if (m_bBladeOrbit)
    {
        m_fBladeAngle += m_fBladeOrbitSpeed * fTimeDelta;
        if (m_fBladeAngle >= 360.f) m_fBladeAngle -= 360.f;

        m_fBladeSelfAngle += m_fBladeSelfSpeed * fTimeDelta;
        if (m_fBladeSelfAngle >= 360.f) m_fBladeSelfAngle -= 360.f;

        m_fBladeHitTimer += fTimeDelta;
        if (m_fBladeHitTimer >= m_fBladeHitInterval)
        {
            m_fBladeHitTimer = 0.f;

            _vec3 vPos;
            Get_Transform()->Get_Info(INFO_POS, &vPos);
            vPos.y += 1.f;

            for (int i = 0; i < m_iBladeCount; i++)
            {
                float fAngle = D3DXToRadian(m_fBladeAngle + (360.f / m_iBladeCount) * i);
                _vec3 vBladePos;
                vBladePos.x = vPos.x + cosf(fAngle) * m_fBladeOrbitRadius;
                vBladePos.y = vPos.y;
                vBladePos.z = vPos.z + sinf(fAngle) * m_fBladeOrbitRadius;

                // 몬스터 피격 체크
                for (auto& pair : CMonsterMgr::GetInstance()->Get_MonsterGroups())
                {
                    for (auto& pMonster : pair.second.vecMonsters)
                    {
                        if (!pMonster->IsActive())
                            continue;
                        CCollider* pCol = pMonster->Get_Collider();
                        if (!pCol)
                            continue;
                        AABB tAABB = pCol->Get_AABB();
                        _vec3 vCenter = (tAABB.vMin + tAABB.vMax) * 0.5f;
                        _vec3 vDiff = vCenter - vBladePos;
                        vDiff.y = 0.f;
                        if (D3DXVec3Length(&vDiff) < 5.f)
                            pMonster->Take_Damage(15);

                        if (m_pTJBoss && !m_pTJBoss->Is_Dead())
                        {
                            CCollider* pCol = m_pTJBoss->Get_Collider();
                            if (pCol)
                            {
                                AABB tAABB = pCol->Get_AABB();
                                _vec3 vCenter = (tAABB.vMin + tAABB.vMax) * 0.5f;
                                _vec3 vDiff = vCenter - vBladePos;
                                vDiff.y = 0.f;
                                if (D3DXVec3Length(&vDiff) < 5.f)
                                    m_pTJBoss->Take_Damage(15.f);
                            }
                        }
                    }
                }
            }
        }
    }

    return iExit;
}

void CTJPlayer::LateUpdate_GameObject(const _float& fTimeDelta)
{
    CPlayer::LateUpdate_GameObject(fTimeDelta);

    for (auto& pTNT : m_vecTNTAura)
        pTNT->LateUpdate_GameObject(fTimeDelta);
}

void CTJPlayer::Render_GameObject()
{
    CPlayer::Render_GameObject();

    for (auto& pTNT : m_vecTNTAura)
        pTNT->Render_GameObject();

    // 회전 칼날 렌더링
    if (m_bBladeOrbit && m_pBladeBufferCom && m_pBladeTextureCom)
    {
        _vec3 vPos;
        Get_Transform()->Get_Info(INFO_POS, &vPos);
        vPos.y += 1.f;

        m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        m_pGraphicDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        m_pGraphicDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
        m_pGraphicDev->SetRenderState(D3DRS_ALPHAREF, 0x10);
        m_pGraphicDev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);

        for (int i = 0; i < m_iBladeCount; i++)
        {
            float fAngle = D3DXToRadian(m_fBladeAngle + (360.f / m_iBladeCount) * i);
            _vec3 vBladePos;
            vBladePos.x = vPos.x + cosf(fAngle) * m_fBladeOrbitRadius;
            vBladePos.y = vPos.y;
            vBladePos.z = vPos.z + sinf(fAngle) * m_fBladeOrbitRadius;

            _matrix matWorld, matScale, matRot, matRotX, matTrans;
            D3DXMatrixScaling(&matScale, 2.f, 2.f, 2.f);
            D3DXMatrixRotationY(&matRot, fAngle + D3DX_PI);
            D3DXMatrixRotationX(&matRotX, D3DX_PI * 0.5f);
            D3DXMatrixTranslation(&matTrans, vBladePos.x, vBladePos.y, vBladePos.z);
            matWorld = matScale * matRotX * matRot * matTrans;

            m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);
            m_pGraphicDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            m_pGraphicDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            m_pBladeTextureCom->Set_Texture(0);
            m_pBladeBufferCom->Render_Buffer();
        }

        m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    }

    // 번개 이펙트
    if (m_bThunderEffect && m_pThunderBufferCom && m_pLightningTextureCom && m_pSparkTextureCom)
    {
        m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        m_pGraphicDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        m_pGraphicDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        m_pGraphicDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        m_pGraphicDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);

        // UV 변환으로 프레임 자르기
        m_pGraphicDev->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);

        // 번개 — 5프레임, 가로 스프라이트
        {
            int iFrame = min(m_iLightningFrame, 4);
            _matrix matUV;
            D3DXMatrixIdentity(&matUV);
            matUV._11 = 1.f / 5.f;         // U 스케일
            matUV._31 = iFrame * (1.f / 5.f); // U 오프셋

            m_pGraphicDev->SetTransform(D3DTS_TEXTURE0, &matUV);

            _matrix matScale, matTrans, matWorld;
            D3DXMatrixScaling(&matScale, 3.f, 10.f, 1.f);
            D3DXMatrixTranslation(&matTrans, m_vThunderPos.x, m_vThunderPos.y + 5.f, m_vThunderPos.z);
            matWorld = matScale * matTrans;
            m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);
            m_pLightningTextureCom->Set_Texture(0);
            m_pThunderBufferCom->Render_Buffer();
        }

        // 스파크 — 4프레임
        {
            int iFrame = min(m_iSparkFrame, 3);
            _matrix matUV;
            D3DXMatrixIdentity(&matUV);
            matUV._11 = 1.f / 4.f;
            matUV._31 = iFrame * (1.f / 4.f);

            m_pGraphicDev->SetTransform(D3DTS_TEXTURE0, &matUV);

            _matrix matView;
            m_pGraphicDev->GetTransform(D3DTS_VIEW, &matView);
            matView._41 = matView._42 = matView._43 = 0.f;
            D3DXMatrixInverse(&matView, NULL, &matView);

            _matrix matScale, matTrans, matWorld;
            D3DXMatrixScaling(&matScale, 3.f, 3.f, 3.f);
            D3DXMatrixTranslation(&matTrans, m_vThunderPos.x, m_vThunderPos.y, m_vThunderPos.z);
            matWorld = matScale * matView * matTrans;
            m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);
            m_pSparkTextureCom->Set_Texture(0);
            m_pThunderBufferCom->Render_Buffer();
        }

        // 복원
        m_pGraphicDev->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
        m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    }

    // 화염 장판 렌더링
    if (!m_vecFireTrails.empty() && m_pFireBufferCom && m_pFireTextureCom)
    {
        m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        m_pGraphicDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        m_pGraphicDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
        m_pGraphicDev->SetRenderState(D3DRS_ALPHAREF, 0x10);
        m_pGraphicDev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
        m_pGraphicDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        m_pGraphicDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
        m_pGraphicDev->SetRenderState(D3DRS_ZENABLE, FALSE);

        m_pFireTextureCom->Set_Texture(0);

        for (auto& tFire : m_vecFireTrails)
        {
            _matrix matScale, matRotX, matTrans, matWorld;
            D3DXMatrixScaling(&matScale, 3.f, 3.f, 3.f);
            D3DXMatrixRotationX(&matRotX, D3DX_PI * 0.5f);
            D3DXMatrixTranslation(&matTrans, tFire.vPos.x, tFire.vPos.y + 0.05f, tFire.vPos.z);
            matWorld = matScale * matRotX * matTrans;
            m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);
            m_pFireBufferCom->Render_Buffer();
        }

        m_pGraphicDev->SetRenderState(D3DRS_ZENABLE, TRUE);
        m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
        m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
        m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    }
}

void CTJPlayer::Add_Exp(int iExp)
{
    if (m_iLevel >= m_iMaxLevel)
        return;

    m_iExp += iExp;
    if (m_iExp >= m_iMaxExp)
    {
        m_iExp -= m_iMaxExp;
        m_iLevel++;
        m_iMaxExp = (int)(m_iMaxExp * 1.2f);
        m_bLevelUp = true;

        CTJSpawnMgr::GetInstance()->Set_SpawnInterval(
            max(0.3f, 2.f * powf(0.7f, (float)m_iLevel)));
    }
}

void CTJPlayer::Apply_Ability(ETJAbility eAbility)
{
    int iIdx = (int)eAbility;
    if (m_iAbilityLevel[iIdx] >= m_iAbilityMaxLevel[iIdx])
        return;

    m_iAbilityLevel[iIdx]++;
    int iLevel = m_iAbilityLevel[iIdx];

    switch (eAbility)
    {
    case ETJAbility::ARROW_PLUS:
        m_iArrowCount++;
        break;

    case ETJAbility::TNT_LAUNCHER:
        m_bTNTAura = true;
        if (iLevel == 2) m_fTNTInterval = 2.f;  
        if (iLevel == 3) m_fTNTInterval = 1.f;  
        break;

    case ETJAbility::HP_REGEN:
        if (iLevel == 1) m_fRegenAmount = 2.f;
        if (iLevel == 2) m_fRegenAmount = 3.f;
        if (iLevel == 3) m_fRegenAmount = 5.f;
        break;

    case ETJAbility::LIGHTNING:
        m_bLightning = true;
        if (iLevel == 2) m_fLightningInterval = 2.f;
        if (iLevel == 3) m_fLightningInterval = 0.5f;
        break;

    case ETJAbility::FIRE_TRAIL:
        m_bFireTrail = true;
        if (iLevel == 2) m_fFireDamage = 7.f;
        if (iLevel == 3) m_fFireDamage = 10.f;
        break;

    case ETJAbility::BLADE_ORBIT:
        m_bBladeOrbit = true;
        if (iLevel == 2) m_iBladeCount = 2;
        if (iLevel == 3) m_iBladeCount = 3;
        break;
    }
}

void CTJPlayer::Key_Input(const _float& fTimeDelta)
{
    m_bMoving = false;

    // 우클릭 화살
    bool bRClick = (GetAsyncKeyState(VK_RBUTTON) & 0x8000);

    if (bRClick && m_fBowCooldown <= 0.f)
    {
        if (!m_bCharging)
        {
            int iIdx = rand() % 3 + 1;
            TCHAR szKey[MAX_PATH];
            wsprintf(szKey, L"Player/sfx_item_CrossBowLoadTwang-%03d.wav", iIdx);
            CSoundMgr::GetInstance()->PlayEffect(szKey, 1.f);
        }
        m_bCharging = true;
        m_fCharge += fTimeDelta;
        if (m_fCharge > m_fMaxCharge)
            m_fCharge = m_fMaxCharge;

        // 마우스 방향으로 조준
        _vec3 vPos;
        m_pTransformCom->Get_Info(INFO_POS, &vPos);
        _vec3 vPickPos = Picking_OnBlock();
        _vec3 vDir = vPickPos - vPos;
        vDir.y = 0.f;
        if (D3DXVec3Length(&vDir) > 0.1f)
        {
            D3DXVec3Normalize(&vDir, &vDir);
            m_vBowDir = vDir;
            m_pTransformCom->m_vAngle.y = D3DXToDegree(atan2f(vDir.x, vDir.z)) + 180.f;
        }
    }
    else if (m_bCharging)
    {
        _vec3 vPos;
        m_pTransformCom->Get_Info(INFO_POS, &vPos);
        vPos.y += 1.0f;
        m_fLastChargeRatio = m_fCharge / m_fMaxCharge;

        int iIdx = rand() % 3 + 1;
        TCHAR szKey[MAX_PATH];
        wsprintf(szKey, L"Player/sfx_item_bowShoot-%03d_soundWave.wav", iIdx);
        CSoundMgr::GetInstance()->PlayEffect(szKey, 1.f);

        int iCount = m_iArrowCount;
        float fSpread = 15.f;
        float fStartAngle = -fSpread * (iCount - 1) / 2.f;

        for (int i = 0; i < iCount; i++)
        {
            float fAngle = D3DXToRadian(fStartAngle + fSpread * i);
            _matrix matRot;
            D3DXMatrixRotationY(&matRot, fAngle);
            _vec3 vDir;
            D3DXVec3TransformNormal(&vDir, &m_vBowDir, &matRot);
            D3DXVec3Normalize(&vDir, &vDir);

            CPlayerArrow* pArrow = CPlayerArrow::Create(m_pGraphicDev, vPos, vDir, Get_BowDmg());
            if (pArrow)
                m_vecArrows.push_back(pArrow);
        }

        m_fBowCooldown = 1.f;
        m_fCharge = 0.f;
        m_bCharging = false;
    }

    // WASD 이동
    _vec3 vMoveDir = { 0.f, 0.f, 0.f };
    if (GetAsyncKeyState('W') & 0x8000) vMoveDir.z += 1.f;
    if (GetAsyncKeyState('S') & 0x8000) vMoveDir.z -= 1.f;
    if (GetAsyncKeyState('A') & 0x8000) vMoveDir.x -= 1.f;
    if (GetAsyncKeyState('D') & 0x8000) vMoveDir.x += 1.f;

    if (D3DXVec3Length(&vMoveDir) > 0.1f)
    {
        D3DXVec3Normalize(&vMoveDir, &vMoveDir);
        if (!m_bCharging)
            m_pTransformCom->m_vAngle.y = D3DXToDegree(atan2f(vMoveDir.x, vMoveDir.z)) + 180.f;
        m_pTransformCom->Move_Pos(&vMoveDir, m_fMoveSpeed, fTimeDelta);
        m_bMoving = true;
        m_bHasTarget = false;
    }

    // 구르기
    if (CDInputMgr::GetInstance()->Get_DIKeyState(DIK_SPACE))
    {
        if (!m_bRolling && m_fRollCooldown <= 0.f)
        {
            if (m_bMoving)
                m_vRollDir = vMoveDir;
            else
            {
                _vec3 vLook;
                m_pTransformCom->Get_Info(INFO_LOOK, &vLook);
                m_vRollDir = -vLook;
                m_vRollDir.y = 0.f;
                D3DXVec3Normalize(&m_vRollDir, &m_vRollDir);
            }
            m_bRolling = true;
            m_fRollTime = 0.f;
            m_bHasTarget = false;
        }
    }
}

CTJPlayer* CTJPlayer::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
    CTJPlayer* pPlayer = new CTJPlayer(pGraphicDev);

    if (FAILED(pPlayer->Ready_GameObject()))
    {
        Safe_Release(pPlayer);
        MSG_BOX("CTJPlayer Create Failed");
        return nullptr;
    }

    return pPlayer;
}

void CTJPlayer::Free()
{
    for (auto& pTNT : m_vecTNTAura)
        Safe_Release(pTNT);
    m_vecTNTAura.clear();

    CGameObject::Free();
}