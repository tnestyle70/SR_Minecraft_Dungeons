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
#include "CTJBoss.h"
#include "CEventBus.h"

CTGStage::CTGStage(LPDIRECT3DDEVICE9 pGraphicDev)
	:CScene(pGraphicDev)
{}

CTGStage::~CTGStage()
{}

HRESULT CTGStage::Ready_Scene()
{
	if (FAILED(Ready_Light()))
		return E_FAIL;

	D3DXCreateFont(m_pGraphicDev, 24, 0, FW_BOLD, 1, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, L"Arial", &m_pFont);

	D3DXCreateLine(m_pGraphicDev, &m_pLine);

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

	// 최대 레벨 달성 시 보스 스폰
	if (m_pTJPlayer && !m_bBossSpawned && m_pTJPlayer->Get_Level() >= 18)
	{
		m_bBossSpawned = true;
		_vec3 vPlayerPos;
		m_pTJPlayer->Get_Transform()->Get_Info(INFO_POS, &vPlayerPos);
		_vec3 vBossPos = { vPlayerPos.x + 15.f, vPlayerPos.y, vPlayerPos.z + 15.f };
		m_pTJBoss = CTJBoss::Create(m_pGraphicDev, vBossPos);
		if (m_pTJBoss)
		{
			m_pTJBoss->Set_Camera(m_pDynamicCamera);
			m_mapLayer[L"GameLogic_Layer"]->Add_GameObject(L"TJBoss", m_pTJBoss);
		}
	}

	m_pTJPlayer->Set_TJBoss(m_pTJBoss);

	// 보스 처치 시 클리어
	if (m_bBossSpawned && m_pTJBoss && m_pTJBoss->Is_Dead() && !m_bDoorSpawned)
	{
		m_bDoorSpawned = true;

		// 몬스터 전체 삭제
		for (auto& pair : CMonsterMgr::GetInstance()->Get_MonsterGroups())
			for (auto& pMonster : pair.second.vecMonsters)
				if (pMonster->IsActive())
					pMonster->Take_Damage(9999);
		CTJSpawnMgr::GetInstance()->Stop_Spawn();

		// 보스 위치에 문 생성
		m_pTJBoss->Get_Transform()->Get_Info(INFO_POS, &m_vDoorPos);
		m_vDoorPos.y = 0.1f;
	}

	_int iExit = CScene::Update_Scene(fTimeDelta);

	CBlockMgr::GetInstance()->Update(fTimeDelta);

	CTriggerBoxMgr::GetInstance()->Update(fTimeDelta);

	CIronBarMgr::GetInstance()->Update(fTimeDelta);

	CMonsterMgr::GetInstance()->Update(fTimeDelta);

	CTJSpawnMgr::GetInstance()->Update(fTimeDelta);

	CParticleMgr::GetInstance()->Update(fTimeDelta);

	// 문 통과 감지
	if (m_bDoorSpawned && m_pTJPlayer)
	{
		_vec3 vPlayerPos;
		m_pTJPlayer->Get_Transform()->Get_Info(INFO_POS, &vPlayerPos);
		_vec3 vDiff = vPlayerPos - m_vDoorPos;
		vDiff.y = 0.f;
		if (D3DXVec3Length(&vDiff) < 4.f)
		{
			m_fDoorTimer += fTimeDelta;
			if (m_fDoorTimer >= 3.f)
			{
				CRenderer::GetInstance()->Clear_RenderGroup();
				CTriggerBoxMgr::GetInstance()->Clear();
				CIronBarMgr::GetInstance()->Clear();
				CMonsterMgr::GetInstance()->Clear();
				CParticleMgr::GetInstance()->Clear_Emitters();
				CInventoryMgr::GetInstance()->Clear_Player();
				CTJSpawnMgr::GetInstance()->Clear();
				CBlockMgr::GetInstance()->ClearBlocks();

				if (FAILED(CSceneChanger::ChangeScene(m_pGraphicDev, eSceneType::SCENE_CAMP)))
				{
					MSG_BOX("Scene Change Failed");
					return -1;
				}
				return iExit;
			}
		}
		else
		{
			m_fDoorTimer = 0.f;
		}
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
	//문 렌더링
	if (m_bDoorSpawned && m_pDoorBufferCom && m_pDoorTextureCom)
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

		_matrix matScale, matRotX, matTrans, matWorld;
		D3DXMatrixScaling(&matScale, 8.f, 8.f, 8.f);
		D3DXMatrixRotationX(&matRotX, D3DX_PI * 0.5f);
		D3DXMatrixTranslation(&matTrans, m_vDoorPos.x, m_vDoorPos.y, m_vDoorPos.z);
		matWorld = matScale * matRotX * matTrans;

		m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);
		m_pDoorTextureCom->Set_Texture(0);
		m_pDoorBufferCom->Render_Buffer();

		m_pGraphicDev->SetRenderState(D3DRS_ZENABLE, TRUE);
		m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	}
}

void CTGStage::Render_UI()
{
	if (CInventoryMgr::GetInstance()->IsActive())
	{
		CInventoryMgr::GetInstance()->Render();
		return;
	}

	// 보스 체력바
	if (m_bBossSpawned && m_pTJBoss && !m_pTJBoss->Is_Dead() && m_pLine)
	{
		RECT rc = { (LONG)(WINCX - 400) / 2, (LONG)20, (LONG)(WINCX + 400) / 2, (LONG)50 };
		m_pFont->DrawText(nullptr, L"좀비킹", -1, &rc, DT_CENTER | DT_VCENTER, D3DCOLOR_RGBA(255, 255, 255, 255));

		float fRatio = m_pTJBoss->Get_HpRatio();

		float fBarWidth = 400.f;
		float fBarHeight = 20.f;
		float fBarX = (WINCX - fBarWidth) * 0.5f;
		float fBarY = 50.f;

		// 배경바 (회색)
		D3DXVECTOR2 bgVerts[2] = {
			{ fBarX, fBarY + fBarHeight * 0.5f },
			{ fBarX + fBarWidth, fBarY + fBarHeight * 0.5f }
		};
		m_pLine->SetWidth(fBarHeight);
		m_pLine->Begin();
		m_pLine->Draw(bgVerts, 2, D3DCOLOR_RGBA(80, 80, 80, 200));
		m_pLine->End();

		// 체력바 (빨강)
		D3DXVECTOR2 hpVerts[2] = {
			{ fBarX, fBarY + fBarHeight * 0.5f },
			{ fBarX + fBarWidth * fRatio, fBarY + fBarHeight * 0.5f }
		};
		m_pLine->Begin();
		m_pLine->Draw(hpVerts, 2, D3DCOLOR_RGBA(220, 30, 30, 220));
		m_pLine->End();
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

	//pDynamicCam->SetActionCam();

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
	CEventBus::GetInstance()->Unsubscribe(eEventType::MISSION_COMPLETE, pHUD);
	//레벨업 능력카드
	m_pLevelUpUI = CTJLevelUpUI::Create(m_pGraphicDev);
	if (!m_pLevelUpUI) return E_FAIL;
	if (FAILED(pLayer->Add_GameObject(L"LevelUpUI", m_pLevelUpUI)))
		return E_FAIL;
	//문 세팅
	m_pDoorBufferCom = dynamic_cast<Engine::CRcTex*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RcTex"));

	m_pMagnetBufferCom = dynamic_cast<Engine::CRcTex*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RcTex"));
	m_pMagnetTextureCom = dynamic_cast<Engine::CTexture*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_MagnetTexture"));

	m_pDoorTextureCom = dynamic_cast<Engine::CTexture*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_TJDoorTexture"));

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
	if (m_pLine)
	{
		m_pLine->Release();
		m_pLine = nullptr;
	}
	if (m_pFont)
	{
		m_pFont->Release();
		m_pFont = nullptr;
	}
	Safe_Release(m_pDoorBufferCom);
	Safe_Release(m_pDoorTextureCom);
	Safe_Release(m_pTJBoss);
	m_pLevelUpUI = nullptr;
	m_pTJPlayer = nullptr;
	CScene::Free();
}
