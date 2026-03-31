#include "pch.h"
#include "CCYStage.h"
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
#include "CJumpingTrapMgr.h"

CCYStage::CCYStage(LPDIRECT3DDEVICE9 pGraphicDev)
	:CScene(pGraphicDev)
{}

CCYStage::~CCYStage()
{}

HRESULT CCYStage::Ready_Scene()
{
	if (FAILED(Ready_Light()))
		return E_FAIL;

	if (FAILED(Ready_Environment_Layer(L"Environment_Layer")))
		return E_FAIL;

	if (FAILED(Ready_GameLogic_Layer(L"GameLogic_Layer")))
		return E_FAIL;

	if (FAILED(Ready_UI_Layer(L"UI_Layer")))
		return E_FAIL;

	Ready_StageData(L"../Bin/Data/Stage7.dat");

	return S_OK;
}

_int CCYStage::Update_Scene(const _float& fTimeDelta)
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
		if (FAILED(CSceneChanger::ChangeScene(m_pGraphicDev, eSceneType::SCENE_NETWORK)))
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

	D3DMATERIAL9 mat;
	ZeroMemory(&mat, sizeof(mat));
	mat.Diffuse = { 1.f, 1.f, 1.f, 1.f };
	mat.Ambient = { 1.f, 1.f, 1.f, 1.f };
	m_pGraphicDev->SetMaterial(&mat);

	CBlockMgr::GetInstance()->Render();
} 

void CCYStage::Render_UI()
{
	if (CInventoryMgr::GetInstance()->IsActive())
	{
		CInventoryMgr::GetInstance()->Render();
		return;
	}
}

HRESULT CCYStage::Ready_Environment_Layer(const _tchar* pLayerTag)
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

	//SkyBox 추가


	m_mapLayer.insert({ pLayerTag, pLayer });

	return S_OK;
}

HRESULT CCYStage::Ready_GameLogic_Layer(const _tchar* pLayerTag)
{
	CLayer* pLayer = CLayer::Create();

	if (!pLayer)
		return E_FAIL;

	CGameObject* pGameObject = nullptr;

	//Player
	pGameObject = CPlayer::Create(m_pGraphicDev);

	if (!pGameObject)
		return E_FAIL;

	if (FAILED(pLayer->Add_GameObject(L"Player", pGameObject)))
		return E_FAIL;

	CPlayer* pPlayer = dynamic_cast<CPlayer*>(pGameObject);

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

	//고정카메라 추가
	if (m_pDynamicCamera)
		m_pDynamicCamera->SetFollowTarget(
			dynamic_cast<Engine::CTransform*>(pPlayer->Get_Component(ID_DYNAMIC, L"Com_Transform")));


	m_mapLayer.insert({ pLayerTag, pLayer });


	return S_OK;
}

HRESULT CCYStage::Ready_UI_Layer(const _tchar* pLayerTag)
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

HRESULT CCYStage::Ready_Light()
{
	m_pGraphicDev->SetRenderState(D3DRS_LIGHTING, TRUE);
	m_pGraphicDev->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(10, 10, 10));

	return S_OK;
}

HRESULT CCYStage::Ready_StageData(const _tchar* szPath)
{
	 //CNormalCubeTex, StoneGradient 프로토타입 등록 (게임 플레이 시 에디터를 안 거치면 없을 수 있음)
	if(FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_NormalCubeTex",
		Engine::CNormalCubeTex::Create(m_pGraphicDev, 1.f, 1.f, 1.f))))
		return E_FAIL;
	
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_StoneGradientTexture",
		Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL,
			L"../Bin/Resource/Texture/blocks/stone_gradient_12.dds"))))
		return E_FAIL;

	FILE* pFile = nullptr;
	_wfopen_s(&pFile, szPath, L"rb");
	if (!pFile)
		return E_FAIL;

	// 1. 블럭 (LoadBlocks 내부에서 RebuildBatchMesh까지)

	//BlockMgr
	//if (FAILED(CBlockMgr::GetInstance()->Ready_BlockMgr(m_pGraphicDev)))
	//{
	//	MSG_BOX("block mgr create failed");
	//	return E_FAIL;
	//}

	CBlockMgr::GetInstance()->SetRenderMode(eRenderMode::RENDER_EDITOR); // 먼저 모드 설정

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
		if (pMonster)
			CMonsterMgr::GetInstance()->AddMonster(pMonster, tData.iTriggerID);
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
	}

	//5. 점핑 트랩
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
	return S_OK;

	//
	//int iCount = 0;
	//fread(&iCount, sizeof(int), 1, pFile);
	//for (int i = 0; i < iCount; ++i)
	//{
	//	MonsterData tData;
	//	fread(&tData, sizeof(MonsterData), 1, pFile);
	//	_vec3 vPos = { (float)tData.x, (float)tData.y, (float)tData.z };
	//	CGameObject* pMonster = CMonster::Create(
	//		m_pGraphicDev, (EMonsterType)tData.iMonsterType, vPos);
	//	CMonsterMgr::GetInstance()->AddMonster(pMonster, tData.iTriggerID);
	//}
	//
	//fread(&iCount, sizeof(int), 1, pFile);
	//for (int i = 0; i < iCount; ++i)
	//{
	//	IronBarData tData;
	//	fread(&tData, sizeof(IronBarData), 1, pFile);
	//	_vec3 vPos = { (float)tData.x, (float)tData.y, (float)tData.z };
	//	CGameObject* pIronBar = CIronBar::Create(m_pGraphicDev, vPos);
	//	if (pIronBar)
	//		CIronBarMgr::GetInstance()->AddIronBar(pIronBar, tData.iTriggerID);
	//}
	//
	//fread(&iCount, sizeof(int), 1, pFile);
	//for (int i = 0; i < iCount; ++i)
	//{
	//	TriggerBoxData tData;
	//	fread(&tData, sizeof(TriggerBoxData), 1, pFile);
	//	_vec3 vPos = { (float)tData.x, (float)tData.y, (float)tData.z };
	//	CGameObject* pTriggerBox = CTriggerBox::Create(m_pGraphicDev, vPos, tData.iTriggerID, (eTriggerBoxType)tData.iTriggerBoxType);
	//	if (pTriggerBox)
	//		CTriggerBoxMgr::GetInstance()->AddTriggerBox(pTriggerBox);
	//}
	//
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
	
	CScene::Free();
}
