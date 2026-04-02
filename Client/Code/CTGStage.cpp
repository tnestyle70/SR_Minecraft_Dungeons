#include "pch.h"
#include "CTGStage.h"
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
#include "CTGSkyBox.h"
#include "CTJSpawnMgr.h"
#include "CTJLevelUpUI.h"


CTGStage::CTGStage(LPDIRECT3DDEVICE9 pGraphicDev)
	:CScene(pGraphicDev)
{}

CTGStage::~CTGStage()
{}

HRESULT CTGStage::Ready_Scene()
{
	if (FAILED(Ready_Light()))
		return E_FAIL;

	if (FAILED(Ready_Environment_Layer(L"Environment_Layer")))
		return E_FAIL;

	if (FAILED(Ready_GameLogic_Layer(L"GameLogic_Layer")))
		return E_FAIL;

	if (FAILED(Ready_UI_Layer(L"UI_Layer")))
		return E_FAIL;

	if (FAILED(Ready_StageData(L"../Bin/Data/Stage6.dat")))
		MSG_BOX("StageData Load Failed");

	return S_OK;
}

_int CTGStage::Update_Scene(const _float& fTimeDelta)
{
	//인벤토리 먼저 업데이트
	CInventoryMgr::GetInstance()->Update(fTimeDelta);

	//Inventory 활성화되었을 경우 게임 업데이트 중지
	if (CInventoryMgr::GetInstance()->IsActive())
		return 0;

	// 레벨업 UI 활성화 시 게임 정지
	if (m_pLevelUpUI && m_pLevelUpUI->Is_Visible())
	{
		m_pLevelUpUI->Update_GameObject(fTimeDelta);
		m_pLevelUpUI->LateUpdate_GameObject(fTimeDelta);

		if (m_pLevelUpUI->Is_Selected())
		{
			m_pTJPlayer->Apply_Ability(m_pLevelUpUI->Get_Selected());
			m_pLevelUpUI->Reset_Selected();
			m_pLevelUpUI->Hide();
		}
		return 0;
	}
	// 레벨업 체크
	if (m_pTJPlayer && m_pTJPlayer->Is_LevelUp())
	{
		m_pTJPlayer->Set_LevelUp(false);
		m_pLevelUpUI->Show(m_pTJPlayer);
	}


	_int iExit = CScene::Update_Scene(fTimeDelta);

	CBlockMgr::GetInstance()->Update(fTimeDelta);

	CTriggerBoxMgr::GetInstance()->Update(fTimeDelta);

	CIronBarMgr::GetInstance()->Update(fTimeDelta);

	CMonsterMgr::GetInstance()->Update(fTimeDelta);

	CTJSpawnMgr::GetInstance()->Update(fTimeDelta);

	CParticleMgr::GetInstance()->Update(fTimeDelta);

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
		CTJSpawnMgr::GetInstance()->Clear();
		CTJSpawnMgr::GetInstance()->Clear();
		if (FAILED(CSceneChanger::ChangeScene(m_pGraphicDev, eSceneType::SCENE_CY)))
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

void CTGStage::LateUpdate_Scene(const _float& fTimeDelta)
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

void CTGStage::Render_Scene()
{
	if (CInventoryMgr::GetInstance()->IsActive())
	{
		CInventoryMgr::GetInstance()->Render();
		return;
	}

	CBlockMgr::GetInstance()->Render();

	CParticleMgr::GetInstance()->Render();
}

void CTGStage::Render_UI()
{
	if (CInventoryMgr::GetInstance()->IsActive())
	{
		CInventoryMgr::GetInstance()->Render();
		return;
	}
}

HRESULT CTGStage::Ready_Environment_Layer(const _tchar* pLayerTag)
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

	pDynamicCam->SetActionCam();

	if (!pGameObject)
		return E_FAIL;

	if (FAILED(pLayer->Add_GameObject(L"DynamicCamera", pGameObject)))
		return E_FAIL;

	pGameObject = CTGSkyBox::Create(m_pGraphicDev);
	if (nullptr == pGameObject)
		return E_FAIL;
	if (FAILED(pLayer->Add_GameObject(L"SkyBox", pGameObject)))
		return E_FAIL;


	m_mapLayer.insert({ pLayerTag, pLayer });

	return S_OK;
}

HRESULT CTGStage::Ready_GameLogic_Layer(const _tchar* pLayerTag)
{
	CLayer* pLayer = CLayer::Create();

	if (!pLayer)
		return E_FAIL;

	CGameObject* pGameObject = nullptr;

	//Player
	pGameObject = CTJPlayer::Create(m_pGraphicDev);

	if (!pGameObject)
		return E_FAIL;

	if (FAILED(pLayer->Add_GameObject(L"Player", pGameObject)))
		return E_FAIL;

	CTJPlayer* pPlayer = dynamic_cast<CTJPlayer*>(pGameObject);

	m_pTJPlayer = pPlayer;

	pPlayer->Get_Transform()->Set_Pos(0.f, 5.f, 0.f);
	pPlayer->Set_MoveSpeed(10.f);
	//HUD
	pGameObject = CHUD::Create(m_pGraphicDev);

	if (nullptr == pGameObject)
		return E_FAIL;

	if (FAILED(pLayer->Add_GameObject(L"HUD", pGameObject)))
		return E_FAIL;

	CHUD* pHUD = dynamic_cast<CHUD*>(pGameObject);

	pHUD->Set_Player(pPlayer);

	//레벨업 능력카드
	m_pLevelUpUI = CTJLevelUpUI::Create(m_pGraphicDev);
	if (!m_pLevelUpUI) return E_FAIL;
	if (FAILED(pLayer->Add_GameObject(L"LevelUpUI", m_pLevelUpUI)))
		return E_FAIL;

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

	CTJSpawnMgr::GetInstance()->Set_GraphicDev(m_pGraphicDev);
	CTJSpawnMgr::GetInstance()->Set_Player(pPlayer);
	CTJSpawnMgr::GetInstance()->Set_TJPlayer(pPlayer);

	//고정카메라 추가
	if (m_pDynamicCamera)
		m_pDynamicCamera->SetFollowTarget(
			dynamic_cast<Engine::CTransform*>(pPlayer->Get_Component(ID_DYNAMIC, L"Com_Transform")));


	m_mapLayer.insert({ pLayerTag, pLayer });


	return S_OK;
}

HRESULT CTGStage::Ready_UI_Layer(const _tchar* pLayerTag)
{
	CLayer* pLayer = CLayer::Create();
	if (!pLayer)
		return E_FAIL;

	m_mapLayer.insert({ pLayerTag, pLayer });
	return S_OK;
}

HRESULT CTGStage::Ready_Light()
{
	return S_OK;
}

HRESULT CTGStage::Ready_StageData(const _tchar* szPath)
{
	FILE* pFile = nullptr;
	_wfopen_s(&pFile, szPath, L"rb");
	if (!pFile)
		return E_FAIL;

	// 1. 블럭 (LoadBlocks 내부에서 RebuildBatchMesh까지)

	//BlockMgr


	CBlockMgr::GetInstance()->SetRenderMode(eRenderMode::RENDER_BATCH); // 먼저 모드 설정

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

CTGStage* CTGStage::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
	CTGStage* pCamp = new CTGStage(pGraphicDev);

	if (FAILED(pCamp->Ready_Scene()))
	{
		Safe_Release(pCamp);
		MSG_BOX("TGStage Create Failed");
		return nullptr;
	}

	return pCamp;
}

void CTGStage::Free()
{
	m_pLevelUpUI = nullptr;
	m_pTJPlayer = nullptr;
	CScene::Free();
}
