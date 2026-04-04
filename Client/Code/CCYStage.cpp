#include "pch.h"
#include "CCYStage.h"
#include "CMonster.h"
#include "CCYMonster.h"
#include "CPlayer.h"
#include "CIronBar.h"
#include "CTriggerBox.h"
#include "CMonsterAnim.h"
#include "CEditor.h"
#include "CBlockMgr.h"
#include "CTriggerBoxMgr.h"
#include "CIronBarMgr.h"
#include "CMonsterMgr.h"
#include "CSceneChanger.h"
#include "CRenderer.h"
#include "StageData.h"
#include "CAncientGuardian.h"
#include "CHUD.h"
#include "CInventoryMgr.h"
#include "CJumpingTrapMgr.h"
#include "CTorch.h"
#include "CObjectEditor.h"
#include "CNormalCubeTex.h"
#include "CCYCamera.h"
#include "CDamageMgr.h"
#include "CSoundMgr.h"

CCYStage::CCYStage(LPDIRECT3DDEVICE9 pGraphicDev)
    : CScene(pGraphicDev)
{
}

CCYStage::~CCYStage()
{
}

HRESULT CCYStage::Ready_Scene()
{
    D3DXCreateFont(m_pGraphicDev, 40, 0, FW_BOLD, 1, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Arial", &m_pFont); 

    CSoundMgr::GetInstance()->StopSound(SOUND_BGM);
    CSoundMgr::GetInstance()->PlayBGM(L"BGM/CCYStageBGM.wav", 0.5f);

    if (FAILED(Ready_Light()))                                 return E_FAIL;
    if (FAILED(Ready_Environment_Layer(L"Environment_Layer"))) return E_FAIL;
    if (FAILED(Ready_GameLogic_Layer(L"GameLogic_Layer")))     return E_FAIL;
    if (FAILED(Ready_UI_Layer(L"UI_Layer")))                   return E_FAIL;

    Ready_StageData(L"../Bin/Data/Stage7.dat");

    return S_OK;
}

_int CCYStage::Update_Scene(const _float& fTimeDelta)
{
    CInventoryMgr::GetInstance()->Update(fTimeDelta);

    if (CInventoryMgr::GetInstance()->IsActive())
        return 0;

    _int iExit = CScene::Update_Scene(fTimeDelta);

    // 타이머 업데이트
    if (!m_bGameOver)
    {
        m_fTimer -= fTimeDelta;
        if (m_fTimer <= 0.f)
        {
            m_fTimer = 0.f;
            m_bGameOver = true;
        }
    }

    // +3초 표시 타이머
    if (m_bShowAddTime)
    {
        m_fAddTimeShow -= fTimeDelta;
        if (m_fAddTimeShow <= 0.f)
            m_bShowAddTime = false;
    }

    // ===== 좌클릭 처리 =====
    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
    {
        _vec3 vCamEye = m_pCYCamera->Get_Eye();

        // 낙하 트리거
        if (!m_bFallTriggered)
        {
            _vec3 vDiff = _vec3{ -2.f, 19.f, 452.f } - vCamEye;
            vDiff.y = 0.f;
            if (D3DXVec3Length(&vDiff) < 3.f)
            {
                m_bFallTriggered = true;
                m_pCYCamera->Start_Fall(-24.f);
            }
        }

        // 아이언바 트리거
        static const _vec3 vIronBarPos[] = {
            { -3.f,  19.f,  40.f },
            { -3.f,  19.f, 100.f },
            { -3.f,  19.f, 195.f },
            { -3.f,  19.f, 355.f },
            { -3.f, -26.f, 350.f },
            { -3.f, -26.f, 208.f },
            { -3.f, -26.f,  94.f },
        };

        for (int i = 0; i < 7; ++i)
        {
            if (m_setIronBarTriggered.find(i) != m_setIronBarTriggered.end()) continue;
            _vec3 vDiff = vIronBarPos[i] - vCamEye;
            vDiff.y = 0.f;
            if (D3DXVec3Length(&vDiff) < 3.f)
            {
                CIronBarMgr::GetInstance()->Close(i + 1);
                m_setIronBarTriggered.insert(i);
            }
        }
    }

    CBlockMgr::GetInstance()->Update(fTimeDelta);
    CTriggerBoxMgr::GetInstance()->Update(fTimeDelta);
    CIronBarMgr::GetInstance()->Update(fTimeDelta);

   
    _vec3 vCamEye = m_pCYCamera->Get_Eye();
    for (auto& pair : CMonsterMgr::GetInstance()->Get_MonsterGroups())
    {
        for (auto* pMonster : pair.second.vecMonsters)
        {
            if (!pMonster->IsActive()) continue;
            pMonster->Set_TargetPos(vCamEye);
        }
    }

    CMonsterMgr::GetInstance()->Update(fTimeDelta);

    // 근접 공격
    if (m_pCYPlayer && m_pCYPlayer->Get_AtkColliderActive())
    {
        CCollider* pAtkCol = m_pCYPlayer->Get_AtkCollider();
        if (pAtkCol)
        {
            bool bHit = false;
            for (auto& pair : CMonsterMgr::GetInstance()->Get_MonsterGroups())
            {
                for (auto* pMonster : pair.second.vecMonsters)
                {
                    if (!pMonster->IsActive() || pMonster->Is_Dead()) continue;
                    CCollider* pMonCol = pMonster->Get_Collider();
                    if (!pMonCol) continue;
                    if (pAtkCol->IsColliding(pMonCol->Get_AABB()))
                    {
                        bHit = true;
                        if (!m_bPrevAtkColliding)
                        {
                            pMonster->Take_Damage(10);
                            if (pMonster->Get_Hp() <= 0)
                            {
                                pMonster->SetActive(false);
                                Add_Time(3.f);
                            }
                        }
                    }
                }
            }
            m_bPrevAtkColliding = bHit;
        }
    }
    else
        m_bPrevAtkColliding = false;

    // 화살 충돌
    for (auto& pair : CMonsterMgr::GetInstance()->Get_MonsterGroups())
    {
        for (auto* pMonster : pair.second.vecMonsters)
        {
            if (!pMonster->IsActive() || pMonster->Is_Dead()) continue;
            CCollider* pMonCol = pMonster->Get_Collider();
            if (!pMonCol) continue;

            for (auto& pArrow : m_pCYPlayer->Get_Arrows())
            {
                if (!pArrow || pArrow->Is_Dead()) continue;
                CCollider* pArrowCol = pArrow->Get_Collider();
                if (!pArrowCol) continue;
                if (pMonCol->IsColliding(pArrowCol->Get_AABB()))
                {
                    pMonster->Take_Damage((int)pArrow->Get_Damage());
                    if (pMonster->Get_Hp() <= 0)
                    {
                        pMonster->SetActive(false);
                        Add_Time(3.f);
                    }
                    pArrow->Set_Dead();
                    break;
                }
            }
        }
    }

    // 몬스터 공격 - 카메라 거리 기반 피격 감지
    bool bHit = false;
    for (auto& pair : CMonsterMgr::GetInstance()->Get_MonsterGroups())
    {
        for (auto* pMonster : pair.second.vecMonsters)
        {
            if (!pMonster->IsActive() || pMonster->Is_Dead()) continue;

            CCollider* pMonCol = pMonster->Get_Collider();
            if (!pMonCol) continue;

            AABB tAABB = pMonCol->Get_AABB();
            _vec3 vMonPos = (tAABB.vMin + tAABB.vMax) * 0.5f;

            _vec3 vDiff = vCamEye - vMonPos;
            vDiff.y = 0.f;
            float fDist = D3DXVec3Length(&vDiff);

            if (fDist <= 3.f)
            {
                bHit = true;
                if (!m_bPrevPlayerHit)
                {
                    m_pCYPlayer->Hit(10.f);
                    m_fTimer -= 3.f;
                    if (m_fTimer < 0.f) m_fTimer = 0.f;
                }
            }
        }
    }
    m_bPrevPlayerHit = bHit;

    if ((GetAsyncKeyState(VK_F6) & 0x8000) || CTriggerBoxMgr::GetInstance()->IsSceneChanged())
    {
        CTriggerBoxMgr::GetInstance()->SetSceneChanged(false);
        CRenderer::GetInstance()->Clear_RenderGroup();
        CTriggerBoxMgr::GetInstance()->Clear();
        CIronBarMgr::GetInstance()->Clear();
        CMonsterMgr::GetInstance()->Clear();
        CParticleMgr::GetInstance()->Clear_Emitters();
        CInventoryMgr::GetInstance()->Clear_Player();
        if (FAILED(CSceneChanger::ChangeScene(m_pGraphicDev, eSceneType::SCENE_CY)))
        {
            MSG_BOX("Scene Change Failed");
            return -1;
        }
        return iExit;
    }

    auto iter = m_mapLayer.find(L"GameLogic_Layer");
    if (iter != m_mapLayer.end())
        iter->second->Delete_GameObject(fTimeDelta);

    return iExit;
}

void CCYStage::LateUpdate_Scene(const _float& fTimeDelta)
{
    if (CInventoryMgr::GetInstance()->IsActive())
    {
        CInventoryMgr::GetInstance()->LateUpdate(fTimeDelta);
        return;
    }

    CScene::LateUpdate_Scene(fTimeDelta);
    CTriggerBoxMgr::GetInstance()->LateUpdate(fTimeDelta);
    CIronBarMgr::GetInstance()->LateUpdate(fTimeDelta);
    CMonsterMgr::GetInstance()->LateUpdate(fTimeDelta);
}

void CCYStage::Render_Scene()
{
    if (CInventoryMgr::GetInstance()->IsActive())
    {
        CInventoryMgr::GetInstance()->Render();
        return;
    }

    m_pGraphicDev->SetRenderState(D3DRS_LIGHTING, TRUE);
    Update_TorchLights();

    D3DMATERIAL9 mat;
    ZeroMemory(&mat, sizeof(mat));
    mat.Diffuse = { 1.f, 1.f, 1.f, 1.f };
    mat.Ambient = { 1.f, 1.f, 1.f, 1.f };
    m_pGraphicDev->SetMaterial(&mat);

    CBlockMgr::GetInstance()->Render();

    m_pGraphicDev->SetRenderState(D3DRS_LIGHTING, FALSE);

    CIronBarMgr::GetInstance()->Render();
    CMonsterMgr::GetInstance()->Render();

    for (auto* pTorch : m_vecTorches)
        pTorch->Render_GameObject();

    for (auto& pair : CMonsterMgr::GetInstance()->Get_MonsterGroups())
    {
        for (auto* pMonster : pair.second.vecMonsters)
        {
            if (!pMonster->IsActive()) continue;
            pMonster->Render_Arrows();
        }
    }

    if (m_pCYPlayer)
        m_pCYPlayer->Render_GameObject();

    CRenderer::GetInstance()->Clear_RenderGroup();
}

void CCYStage::Render_UI()
{
    if (CInventoryMgr::GetInstance()->IsActive())
    {
        CInventoryMgr::GetInstance()->Render();
        return;
    }

    if (!m_pFont) return;

    TCHAR szTimer[32];
    wsprintf(szTimer, L"%d", (int)m_fTimer);

    RECT rcTimer = { WINCX - 120, 20, WINCX - 20, 80 };
    D3DCOLOR color = (m_fTimer <= 3.f) ?
        D3DCOLOR_ARGB(255, 255, 50, 50) :
        D3DCOLOR_ARGB(255, 255, 255, 255);
    m_pFont->DrawText(nullptr, szTimer, -1, &rcTimer, DT_CENTER | DT_VCENTER, color);

    if (m_bShowAddTime)
    {
        RECT rcAdd = { WINCX - 150, 80, WINCX - 20, 130 };
        m_pFont->DrawText(nullptr, L"+3", -1, &rcAdd,
            DT_CENTER | DT_VCENTER, D3DCOLOR_ARGB(255, 100, 255, 100));
    }
}

HRESULT CCYStage::Ready_Environment_Layer(const _tchar* pLayerTag)
{
    CLayer* pLayer = CLayer::Create();
    if (!pLayer) return E_FAIL;

    _vec3 vEye{ -3.f, 21.f, 19.f };
    _vec3 vAt{ -3.f, 21.f, 20.f };
    _vec3 vUp{ 0.f,  1.f,  0.f };

    m_pCYCamera = CCYCamera::Create(m_pGraphicDev, &vEye, &vAt, &vUp);
    if (!m_pCYCamera) return E_FAIL;

    if (FAILED(pLayer->Add_GameObject(L"DynamicCamera", m_pCYCamera)))
        return E_FAIL;

    m_mapLayer.insert({ pLayerTag, pLayer });
    return S_OK;
}

HRESULT CCYStage::Ready_GameLogic_Layer(const _tchar* pLayerTag)
{
    CLayer* pLayer = CLayer::Create();
    if (!pLayer) return E_FAIL;

    m_pCYPlayer = CCYPlayer::Create(m_pGraphicDev);
    if (!m_pCYPlayer) return E_FAIL;

    if (FAILED(pLayer->Add_GameObject(L"Player", m_pCYPlayer)))
        return E_FAIL;

    m_pCYPlayer->Set_Camera(m_pCYCamera);
    CDamageMgr::GetInstance()->Ready_Component();

    m_mapLayer.insert({ pLayerTag, pLayer });

    if (CInventoryMgr::GetInstance()->Ready_InventoryMgr(m_pGraphicDev))
        return E_FAIL;

    CCollider* pCollider = dynamic_cast<CCollider*>
        (m_pCYPlayer->Get_Component(ID_STATIC, L"Com_Collider"));
    if (pCollider)
        CTriggerBoxMgr::GetInstance()->SetPlayerCollider(pCollider);

    m_mapLayer.insert({ pLayerTag, pLayer });

    return S_OK;
}

HRESULT CCYStage::Ready_UI_Layer(const _tchar* pLayerTag)
{
    CLayer* pLayer = CLayer::Create();
    if (!pLayer) return E_FAIL;

    CGameObject* pGameObject = CHUD::Create(m_pGraphicDev);
    if (!pGameObject) return E_FAIL;

    if (FAILED(pLayer->Add_GameObject(L"HUD", pGameObject)))
        return E_FAIL;

    m_mapLayer.insert({ pLayerTag, pLayer });
    return S_OK;
}

HRESULT CCYStage::Ready_Light()
{
    m_pGraphicDev->SetRenderState(D3DRS_LIGHTING, TRUE);
    m_pGraphicDev->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(20, 20, 20));
    return S_OK;
}

HRESULT CCYStage::Ready_StageData(const _tchar* szPath)
{
    CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_NormalCubeTex",
        Engine::CNormalCubeTex::Create(m_pGraphicDev, 1.f, 1.f, 1.f));

    CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_StoneGradientTexture",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL,
            L"../Bin/Resource/Texture/blocks/stone_gradient_12.dds"));

    FILE* pFile = nullptr;
    _wfopen_s(&pFile, szPath, L"rb");
    if (!pFile) return E_FAIL;

    if (FAILED(CBlockMgr::GetInstance()->Ready_BlockMgr(m_pGraphicDev)))
        return E_FAIL;
    CBlockMgr::GetInstance()->SetRenderMode(eRenderMode::RENDER_BATCH);
    CBlockMgr::GetInstance()->ClearBlocks();
    CMonsterMgr::GetInstance()->Ready_MonsterMgr(m_pGraphicDev);
    CMonsterMgr::GetInstance()->SetPlayer(nullptr);
    CBlockMgr::GetInstance()->LoadBlocks(pFile);

    int iCount = 0;

    fread(&iCount, sizeof(int), 1, pFile);
    for (int i = 0; i < iCount; ++i)
    {
        MonsterData tData;
        fread(&tData, sizeof(MonsterData), 1, pFile);
        _vec3 vPos = { (float)tData.x, (float)tData.y, (float)tData.z };
        CGameObject* pMonster = CCYMonster::Create(
            m_pGraphicDev, (EMonsterType)tData.iMonsterType, vPos);
        if (pMonster)
            CMonsterMgr::GetInstance()->AddMonster(pMonster, tData.iTriggerID, vPos);
    }

    fread(&iCount, sizeof(int), 1, pFile);
    for (int i = 0; i < iCount; ++i)
    {
        IronBarData tData;
        fread(&tData, sizeof(IronBarData), 1, pFile);
        _vec3 vPos = { (float)tData.x, (float)tData.y, (float)tData.z };
        CGameObject* pIronBar = CIronBar::Create(m_pGraphicDev, vPos);
        if (pIronBar)
            CIronBarMgr::GetInstance()->AddIronBar(pIronBar, tData.iTriggerID);
    }

    fread(&iCount, sizeof(int), 1, pFile);
    for (int i = 0; i < iCount; ++i)
    {
        TriggerBoxData tData;
        fread(&tData, sizeof(TriggerBoxData), 1, pFile);
        _vec3 vPos = { (float)tData.x, (float)tData.y, (float)tData.z };
        CGameObject* pTriggerBox = CTriggerBox::Create(
            m_pGraphicDev, vPos, tData.iTriggerID, (eTriggerBoxType)tData.iTriggerBoxType);
        if (pTriggerBox)
            CTriggerBoxMgr::GetInstance()->AddTriggerBox(pTriggerBox);
    }

    fread(&iCount, sizeof(int), 1, pFile);
    for (int i = 0; i < iCount; ++i)
    {
        JumpingTrapData tData;
        fread(&tData, sizeof(JumpingTrapData), 1, pFile);
        _vec3 vPos = { (float)tData.x, (float)tData.y, (float)tData.z };
        CGameObject* pJumpingTrap = CJumpingTrap::Create(m_pGraphicDev, vPos);
        if (pJumpingTrap)
            CJumpingTrapMgr::GetInstance()->Add_JumpingTrap(pJumpingTrap, tData.iTriggerID);
    }

    fclose(pFile);

    FILE* pObjFile = nullptr;
    fopen_s(&pObjFile, "../Bin/Data/Stage7Object.dat", "rb");
    if (pObjFile)
    {
        int iObjCount = 0;
        fread(&iObjCount, sizeof(int), 1, pObjFile);
        for (int i = 0; i < iObjCount; ++i)
        {
            OBJECT_DATA data;
            fread(&data, sizeof(OBJECT_DATA), 1, pObjFile);

            if (data.eType == OBJECT_TORCH)
            {
                CTorch* pTorch = CTorch::Create(m_pGraphicDev);
                if (pTorch)
                {
                    CTransform* pTrans = dynamic_cast<CTransform*>
                        (pTorch->Get_Component(ID_DYNAMIC, L"Com_Transform"));
                    if (pTrans)
                        pTrans->Set_Pos(data.vPos[0], data.vPos[1], data.vPos[2]);

                    m_mapLayer[L"GameLogic_Layer"]->Add_GameObject(L"Torch", pTorch);
                    m_vecTorches.push_back(pTorch);
                }
            }
        }
        fclose(pObjFile);
    }

    return S_OK;
}

_vec3 CCYStage::Get_PlayerPos()
{
    if (!m_pCYPlayer) return { 0.f, 0.f, 0.f };

    Engine::CTransform* pTrans = dynamic_cast<Engine::CTransform*>
        (m_pCYPlayer->Get_Component(ID_DYNAMIC, L"Com_Transform"));
    if (!pTrans) return { 0.f, 0.f, 0.f };

    _vec3 vPos;
    pTrans->Get_Info(INFO_POS, &vPos);
    return vPos;
}

void CCYStage::Update_TorchLights()
{
    if (m_vecTorches.empty()) return;

    _vec3 vPlayerPos = Get_PlayerPos();

    for (int i = 0; i < 8; ++i)
        m_pGraphicDev->LightEnable(i, FALSE);

    vector<pair<float, CTorch*>> vecSorted;
    for (auto* pTorch : m_vecTorches)
    {
        _vec3 vDiff = pTorch->Get_Pos() - vPlayerPos;
        float fDist = D3DXVec3Length(&vDiff);
        vecSorted.push_back({ fDist, pTorch });
    }

    sort(vecSorted.begin(), vecSorted.end(),
        [](const pair<float, CTorch*>& a, const pair<float, CTorch*>& b)
        { return a.first < b.first; });

    int iIdx = 0;
    for (auto& p : vecSorted)
    {
        if (iIdx >= 8) break;
        p.second->Set_LightIndex(iIdx);
        p.second->Apply_Light();
        ++iIdx;
    }
}

CCYStage* CCYStage::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
    CCYStage* pCamp = new CCYStage(pGraphicDev);
    if (FAILED(pCamp->Ready_Scene()))
    {
        Safe_Release(pCamp);
        MSG_BOX("pCamp Create Failed");
        return nullptr;
    }
    return pCamp;
}

void CCYStage::Free()
{ 
    CSoundMgr::GetInstance()->StopSound(SOUND_BGM);
    if (m_pFont)
    {
        m_pFont->Release();
        m_pFont = nullptr;
    }

    CDamageMgr::GetInstance()->Clear_Boss();
    m_vecTorches.clear();
    CScene::Free();
}