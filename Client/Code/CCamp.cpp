#include "pch.h"
#include "CCamp.h"
#include "CMonster.h"
#include "CPlayer.h"
#include "CIronBar.h"
#include "CTriggerBox.h"
#include "CMonsterAnim.h"
#include "CEditor.h"
#include "CBlockMgr.h"
#include "CTriggerBoxMgr.h"
#include "CIronBarMgr.h"
#include "CMonsterMgr.h"
#include "CDynamicCamera.h"
#include "CSceneChanger.h"
#include "CRenderer.h"
#include "StageData.h" 
#include "CAncientGuardian.h"
#include "CHUD.h"
#include "CInventoryMgr.h"
#include "CNPC.h"
#include "CSoundMgr.h"

CCamp::CCamp(LPDIRECT3DDEVICE9 pGraphicDev)
	:CScene(pGraphicDev)
{
}

CCamp::~CCamp()
{
}

HRESULT CCamp::Ready_Scene()
{
	if (FAILED(Ready_Light()))
		return E_FAIL;

	if (FAILED(Ready_Environment_Layer(L"Environment_Layer")))
		return E_FAIL;

	if (FAILED(Ready_GameLogic_Layer(L"GameLogic_Layer")))
		return E_FAIL;

	if (FAILED(Ready_UI_Layer(L"UI_Layer")))
		return E_FAIL;

	Ready_StageData(L"../Bin/Data/Stage2.dat");

	CSoundMgr::GetInstance()->PlayBGM(L"BGM/BGM_MainStage.wav", 0.5f);

	return S_OK;
}

_int CCamp::Update_Scene(const _float& fTimeDelta)
{
	//인벤토리 먼저 업데이트
	CInventoryMgr::GetInstance()->Update(fTimeDelta);

	//Inventory 활성화되었을 경우 게임 업데이트 중지
	if (CInventoryMgr::GetInstance()->IsActive())
		return 0;

	_int iExit = CScene::Update_Scene(fTimeDelta);

	CBlockMgr::GetInstance()->Update(fTimeDelta);

	CTriggerBoxMgr::GetInstance()->Update(fTimeDelta);

	CIronBarMgr::GetInstance()->Update(fTimeDelta);

	CMonsterMgr::GetInstance()->Update(fTimeDelta); 

	if (CTriggerBoxMgr::GetInstance()->IsSceneChanged())
	{
		CTriggerBoxMgr::GetInstance()->SetSceneChanged(false);
		CRenderer::GetInstance()->Clear_RenderGroup();
		CTriggerBoxMgr::GetInstance()->Clear();
		CIronBarMgr::GetInstance()->Clear();
		CMonsterMgr::GetInstance()->Clear();
		CParticleMgr::GetInstance()->Clear_Emitters();
		CInventoryMgr::GetInstance()->Clear_Player();

		eSceneType eNext = eSceneType::SCENE_CAMP_PLAY;
		int iID = CTriggerBoxMgr::GetInstance()->Get_TriggeredID();

		switch (iID)
		{
		case 1: eNext = eSceneType::SCENE_NETWORK; break;  // GB
		case 2: eNext = eSceneType::SCENE_JS;      break;  // JS
		case 3: eNext = eSceneType::SCENE_TJ;      break;  // TG
		case 4: eNext = eSceneType::SCENE_CY;      break;  // CY
		}

		if (FAILED(CSceneChanger::ChangeScene(m_pGraphicDev, eNext)))
		{
			MSG_BOX("Scene Change Failed");
			return -1;
		}
		return iExit;
	}

	if (GetAsyncKeyState(VK_RETURN) || CTriggerBoxMgr::GetInstance()->IsSceneChanged())
	{
		//Render Group Clear Before Change Scene!!!!
		CTriggerBoxMgr::GetInstance()->SetSceneChanged(false); 
		CRenderer::GetInstance()->Clear_RenderGroup();
		CTriggerBoxMgr::GetInstance()->Clear();
		CIronBarMgr::GetInstance()->Clear();
		CMonsterMgr::GetInstance()->Clear();
		CParticleMgr::GetInstance()->Clear_Emitters();
		CInventoryMgr::GetInstance()->Clear_Player();
		if (FAILED(CSceneChanger::ChangeScene(m_pGraphicDev, eSceneType::SCENE_TJ)))
		{
			MSG_BOX("RedStone Create Failed");
			return -1;
		}
		return iExit;
	}
	auto iter = m_mapLayer.find(L"GameLogic_Layer");
	if (iter != m_mapLayer.end())
		iter->second->Delete_GameObject(fTimeDelta);

	return iExit;
}

void CCamp::LateUpdate_Scene(const _float& fTimeDelta)
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

void CCamp::Render_Scene()
{
	if (CInventoryMgr::GetInstance()->IsActive())
	{
		CInventoryMgr::GetInstance()->Render();
		return;
	}
	m_pGraphicDev->SetRenderState(D3DRS_LIGHTING, FALSE); 
	CBlockMgr::GetInstance()->Render();
}

void CCamp::Render_UI()
{
	if (CInventoryMgr::GetInstance()->IsActive())
	{
		CInventoryMgr::GetInstance()->Render();
		return;
	}
}

HRESULT CCamp::Ready_Environment_Layer(const _tchar* pLayerTag)
{
	CLayer* pLayer = CLayer::Create();

	if (!pLayer)
		return E_FAIL;

	CGameObject* pGameObject = nullptr;

	//dynamic camera
	_vec3 vEye{ 0.f, 10.f, -10.f };
	_vec3 vAt{ 0.f, 0.f, 1.f };
	_vec3 vUp{ 0.f, 1.f, 0.f };

	m_pDynamicCamera = CDynamicCamera::Create(m_pGraphicDev, &vEye, &vAt, &vUp);

	pGameObject = m_pDynamicCamera;

	CDynamicCamera* pDynamicCam = dynamic_cast<CDynamicCamera*>(pGameObject);

	if (!pDynamicCam)
		return E_FAIL;

	//pDynamicCam->SetActionCam();

	if (!pGameObject)
		return E_FAIL;

	if (FAILED(pLayer->Add_GameObject(L"DynamicCamera", pGameObject)))
		return E_FAIL;

	//SkyBox 추가
	

	m_mapLayer.insert({ pLayerTag, pLayer });

	return S_OK;
}

HRESULT CCamp::Ready_GameLogic_Layer(const _tchar* pLayerTag)
{
	CLayer* pLayer = CLayer::Create();

	if (!pLayer)
		return E_FAIL;

	CGameObject* pGameObject = nullptr;

	//Player
	pGameObject = CPlayer::Create(m_pGraphicDev);
	if (!pGameObject) return E_FAIL;

	if (FAILED(pLayer->Add_GameObject(L"Player", pGameObject)))
		return E_FAIL;

	CPlayer* pPlayer = dynamic_cast<CPlayer*>(pGameObject);


	Engine::CTransform* pTrans = dynamic_cast<Engine::CTransform*>
		(pPlayer->Get_Component(ID_DYNAMIC, L"Com_Transform"));
	if (pTrans)
		pTrans->Set_Pos(-2.f, 2.f, 0.f);
	pTrans->Update_Component(0.016f);

	//플레이어 고정카메라 추가
	if (m_pDynamicCamera)
	{
		m_pDynamicCamera->SetFollowTarget(
			dynamic_cast<Engine::CTransform*>(pPlayer->Get_Component(ID_DYNAMIC, L"Com_Transform")));
		m_pDynamicCamera->SnapToTarget();
	}


	//HUD
	pGameObject = CHUD::Create(m_pGraphicDev);

	if (nullptr == pGameObject)
		return E_FAIL;

	if (FAILED(pLayer->Add_GameObject(L"HUD", pGameObject)))
		return E_FAIL;

	CHUD* pHUD = dynamic_cast<CHUD*>(pGameObject);

	pHUD->Set_Player(pPlayer);

	m_mapLayer.insert({ pLayerTag, pLayer });

	//Inventory 세팅
	if (CInventoryMgr::GetInstance()->Ready_InventoryMgr(m_pGraphicDev))
		return E_FAIL;

	CInventoryMgr::GetInstance()->Set_Player(pPlayer);

	//TriggerBoxMgr
	CCollider* pCollider = dynamic_cast<CCollider*>(pPlayer->Get_Component(ID_STATIC, L"Com_Collider"));
	if (!pCollider)
	{
		MSG_BOX("Player Collider Set Failed");
	}

	CTriggerBoxMgr::GetInstance()->SetPlayerCollider(pCollider);

	CMonsterMgr::GetInstance()->SetPlayer(pPlayer);


	CDialogueBox* pDialogueBox = CDialogueBox::Create(m_pGraphicDev);
	if (!pDialogueBox) return E_FAIL;
	if (FAILED(pLayer->Add_GameObject(L"DialogueBox", pDialogueBox))) return E_FAIL;
	pGameObject = CNPC::Create(m_pGraphicDev, _vec3(-35.f, 2.f, 29.f));
	if (!pGameObject) return E_FAIL; 
	if(FAILED(pLayer->Add_GameObject(L"NPC3", pGameObject))) return E_FAIL;
	CNPC* pNPC3 = dynamic_cast<CNPC*>(pGameObject);
	if (pNPC3 && pPlayer)
	{
		pPlayer->Add_NPC(pNPC3);
		pNPC3->Set_NPCType(eNPCType::NPC_MiNiGame1);
		pNPC3->Set_DialogueBox(pDialogueBox);
	}
	pGameObject = CNPC::Create(m_pGraphicDev, _vec3(-35.f, 2.f, 29.f));
	if (!pGameObject) return E_FAIL;
	if (FAILED(pLayer->Add_GameObject(L"NPC3", pGameObject))) return E_FAIL;



	pGameObject = CNPC::Create(m_pGraphicDev, _vec3(-5.f, 2.f, 29.f));
	if (!pGameObject) return E_FAIL;
	if (FAILED(pLayer->Add_GameObject(L"NPC4", pGameObject))) return E_FAIL;
	CNPC* pNPC4 = dynamic_cast<CNPC*>(pGameObject);
	if (pNPC4 && pPlayer)
	{
		pPlayer->Add_NPC(pNPC4);
		pNPC4->Set_NPCType(eNPCType::NPC_MiNiGame2);
		pNPC4->Set_DialogueBox(pDialogueBox);
	}

	pGameObject = CNPC::Create(m_pGraphicDev, _vec3(24.f, 2.f, 29.f));
	if (!pGameObject) return E_FAIL;
	if (FAILED(pLayer->Add_GameObject(L"NPC5", pGameObject))) return E_FAIL;
	CNPC* pNPC5 = dynamic_cast<CNPC*>(pGameObject);
	if (pNPC5 && pPlayer)
	{
		pPlayer->Add_NPC(pNPC5);
		pNPC5->Set_NPCType(eNPCType::NPC_MiNiGame3);
		pNPC5->Set_DialogueBox(pDialogueBox);
	}

	pGameObject = CNPC::Create(m_pGraphicDev, _vec3(13.f, 2.f, -28.f));
	if (!pGameObject) return E_FAIL;
	if (FAILED(pLayer->Add_GameObject(L"NPC6", pGameObject))) return E_FAIL;
	CNPC* pNPC6 = dynamic_cast<CNPC*>(pGameObject);
	if (pNPC6 && pPlayer)
	{ 
		pNPC6->Get_Transform()->m_vAngle.y = 180.f;
		pPlayer->Add_NPC(pNPC6);
		pNPC6->Set_NPCType(eNPCType::NPC_MiNiGame4);
		pNPC6->Set_DialogueBox(pDialogueBox);
	} 

	pGameObject = CNPC::Create(m_pGraphicDev, _vec3(-16.f, 2.f, -28.f));
	if (!pGameObject) return E_FAIL;
	if (FAILED(pLayer->Add_GameObject(L"NPC7", pGameObject))) return E_FAIL;
	CNPC* pNPC7 = dynamic_cast<CNPC*>(pGameObject);
	if (pNPC7 && pPlayer)
	{ 
		pNPC7->Get_Transform()->m_vAngle.y = 180.f;
		pPlayer->Add_NPC(pNPC7);
		pNPC7->Set_NPCType(eNPCType::NPC_MiNiGame5);
		pNPC7->Set_DialogueBox(pDialogueBox);
	} 

	pGameObject = CNPC::Create(m_pGraphicDev, _vec3(-45.f, 2.f, -28.f));
	if (!pGameObject) return E_FAIL;
	if (FAILED(pLayer->Add_GameObject(L"NPC8", pGameObject))) return E_FAIL;
	CNPC* pNPC8 = dynamic_cast<CNPC*>(pGameObject);
	if (pNPC8 && pPlayer)
	{ 
		pNPC8->Get_Transform()->m_vAngle.y = 180.f;
		pPlayer->Add_NPC(pNPC8);
		pNPC8->Set_NPCType(eNPCType::NPC_MiNiGame6);
		pNPC8->Set_DialogueBox(pDialogueBox);
	}


	m_mapLayer.insert({ pLayerTag, pLayer });


	return S_OK;
}

HRESULT CCamp::Ready_UI_Layer(const _tchar* pLayerTag)
{
	CLayer* pLayer = CLayer::Create();

	if (!pLayer)
		return E_FAIL;

	CGameObject* pGameObject = nullptr;
	//HUD
	pGameObject = CHUD::Create(m_pGraphicDev);

	if (nullptr == pGameObject)
		return E_FAIL;

	if (FAILED(pLayer->Add_GameObject(L"HUD", pGameObject)))
		return E_FAIL;

	m_mapLayer.insert({ pLayerTag, pLayer });

	return S_OK;
}

HRESULT CCamp::Ready_Light()
{
	m_pGraphicDev->SetRenderState(D3DRS_LIGHTING, FALSE);
	return S_OK;
}

HRESULT CCamp::Ready_StageData(const _tchar* szPath)
{
	FILE* pFile = nullptr;
	_wfopen_s(&pFile, szPath, L"rb");
	if (!pFile)
		return E_FAIL;

	// 1. 블럭 (LoadBlocks 내부에서 RebuildBatchMesh까지)

	//BlockMgr
	if (FAILED(CBlockMgr::GetInstance()->Ready_BlockMgr(m_pGraphicDev)))
	{
		MSG_BOX("block mgr create failed");
		return E_FAIL;
	}

	CBlockMgr::GetInstance()->SetRenderMode(eRenderMode::RENDER_BATCH); // 먼저 모드 설정
	CBlockMgr::GetInstance()->ClearBlocks();
	CBlockMgr::GetInstance()->LoadBlocks(pFile);

	// 2. 몬스터 - map에 안 담고 레이어에 바로 추가
	int iCount = 0;
	fread(&iCount, sizeof(int), 1, pFile);
	for (int i = 0; i < iCount; ++i)
	{
		MonsterData tData;
		fread(&tData, sizeof(MonsterData), 1, pFile);
		_vec3 vPos = { (float)tData.x, (float)tData.y, (float)tData.z };

		CGameObject* pMonster = CMonster::Create(
			m_pGraphicDev, (EMonsterType)tData.iMonsterType, vPos);
		//MonsterMgr 쪽에 추가
		CMonsterMgr::GetInstance()->AddMonster(pMonster, tData.iTriggerID, vPos);
	}

	// 3. 창살
	fread(&iCount, sizeof(int), 1, pFile);
	for (int i = 0; i < iCount; ++i)
	{
		IronBarData tData;
		fread(&tData, sizeof(IronBarData), 1, pFile);
		_vec3 vPos = { (float)tData.x, (float)tData.y, (float)tData.z };

		CGameObject* pIronBar = CIronBar::Create(m_pGraphicDev, vPos);
		if (pIronBar)
			CIronBarMgr::GetInstance()->AddIronBar(pIronBar, tData.iTriggerID);
		//m_mapLayer[L"GameLogic_Layer"]->Add_GameObject(L"IronBar", pIronBar);
	}

	// 4. 트리거박스
	fread(&iCount, sizeof(int), 1, pFile);
	for (int i = 0; i < iCount; ++i)
	{
		TriggerBoxData tData;
		fread(&tData, sizeof(TriggerBoxData), 1, pFile);
		_vec3 vPos = { (float)tData.x, (float)tData.y, (float)tData.z };

		CGameObject* pTriggerBox = CTriggerBox::Create(m_pGraphicDev, vPos, tData.iTriggerID, (eTriggerBoxType)tData.iTriggerBoxType);
		if (pTriggerBox)
			CTriggerBoxMgr::GetInstance()->AddTriggerBox(pTriggerBox);
		//m_mapLayer[L"GameLogic_Layer"]->Add_GameObject(L"TriggerBox", pTriggerBox);
	}

	fclose(pFile);
	return S_OK;
}

CCamp* CCamp::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
	CCamp* pCamp = new CCamp(pGraphicDev);
	
	if (FAILED(pCamp->Ready_Scene()))
	{
		Safe_Release(pCamp);
		MSG_BOX("pCamp Create Failed");
		return nullptr;
	}

	return pCamp;
}

void CCamp::Free()
{
	CScene::Free();
}
