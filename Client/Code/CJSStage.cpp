#include "pch.h"
#include "CJSStage.h"
#include "CSceneChanger.h"
#include "CRenderer.h"
#include "CJSCamera.h"
#include "CJSChunkMgr.h"
#include "CJSPlayer.h"
#include "CManagement.h"
#include "CSoundMgr.h"
#include "CLightMgr.h"
#include "CJSScoreUI.h"
#include "CJSScoreMgr.h"
#include "CJSSkyBox.h"
#include "CJSWaterPlane.h"
#include "CScreenfx.h"

CJSStage::CJSStage(LPDIRECT3DDEVICE9 pGraphicDev)
	:CScene(pGraphicDev)
{
}

CJSStage::~CJSStage()
{
}

HRESULT CJSStage::Ready_Scene()
{
	CSoundMgr::GetInstance()->StopAll();
	CSoundMgr::GetInstance()->PlayBGM(L"JS/1-02.-Background-Music.wav", 0.8f);

	if (FAILED(Ready_Light()))
		return E_FAIL;

	if (FAILED(Ready_Fog()))
		return E_FAIL;

	if (FAILED(Ready_Environment_Layer(L"Environment_Layer")))
		return E_FAIL;

	if (FAILED(Ready_GameLogic_Layer(L"GameLogic_Layer")))
		return E_FAIL;

	if (FAILED(Ready_UI_Layer(L"UI_Layer")))
		return E_FAIL;

	return S_OK;
}

_int CJSStage::Update_Scene(const _float& fTimeDelta)
{
	_vec3 vPlayerPos;
	CTransform* pPlayerTrans = dynamic_cast<CTransform*>(CManagement::GetInstance()->Get_Component(ID_DYNAMIC, L"GameLogic_Layer", L"JSPlayer", L"Com_Transform"));
	pPlayerTrans->Get_Info(INFO_POS, &vPlayerPos);

	if (m_pWaterPlane)
	{
		m_pWaterPlane->Set_PlayerPos(vPlayerPos);
	}

	_int iExit = CScene::Update_Scene(fTimeDelta);

	CJSChunkMgr::GetInstance()->Update_Manager(fTimeDelta, vPlayerPos);

	Clear_DeadObject(L"Environment_Layer", fTimeDelta);

	return iExit;
}

void CJSStage::LateUpdate_Scene(const _float& fTimeDelta)
{
	CScene::LateUpdate_Scene(fTimeDelta);
}

void CJSStage::Render_Scene()
{
}

HRESULT CJSStage::Ready_Environment_Layer(const _tchar* pLayerTag)
{
	CLayer* pLayer = CLayer::Create();

	if (!pLayer)
		return E_FAIL;

	CGameObject* pGameObject = nullptr;

	//camera
	_vec3 vEye{ 0.f, 8.f, -10.f };
	_vec3 vAt{ 0.f, 0.f, 1.f };
	_vec3 vUp{ 0.f, 1.f, 0.f };

	pGameObject = CJSCamera::Create(m_pGraphicDev, &vEye, &vAt, &vUp);

	if (!pGameObject)
		return E_FAIL;

	if (FAILED(pLayer->Add_GameObject(L"JSCamera", pGameObject)))
		return E_FAIL;

	// SkyBox
	pGameObject = CJSSkyBox::Create(m_pGraphicDev);

	if (!pGameObject)
		return E_FAIL;

	if (FAILED(pLayer->Add_GameObject(L"JSSkyBox", pGameObject)))
		return E_FAIL;

	// Water
	m_pWaterPlane = CJSWaterPlane::Create(m_pGraphicDev);

	if (!m_pWaterPlane)
		return E_FAIL;

	if (FAILED(pLayer->Add_GameObject(L"WaterPlane", m_pWaterPlane)))
		return E_FAIL;

	// Chunk
	if (FAILED(CJSChunkMgr::GetInstance()->Ready_Manager(m_pGraphicDev, pLayer)))
		return E_FAIL;

	m_mapLayer.insert({ pLayerTag, pLayer });

	return S_OK;
}

HRESULT CJSStage::Ready_GameLogic_Layer(const _tchar* pLayerTag)
{
	CLayer* pLayer = CLayer::Create();

	if (!pLayer)
		return E_FAIL;

	CGameObject* pGameObject = nullptr;

	pGameObject = CJSPlayer::Create(m_pGraphicDev);

	if (!pGameObject)
		return E_FAIL;

	if (FAILED(pLayer->Add_GameObject(L"JSPlayer", pGameObject)))
		return E_FAIL;

	m_mapLayer.insert({ pLayerTag, pLayer });

	return S_OK;
}

HRESULT CJSStage::Ready_UI_Layer(const _tchar* pLayerTag)
{
	CLayer* pLayer = CLayer::Create();

	if (!pLayer)
		return E_FAIL;

	CGameObject* pGameObject = nullptr;

	pGameObject = CJSScoreUI::Create(m_pGraphicDev);

	if (!pGameObject)
		return E_FAIL;

	if (FAILED(pLayer->Add_GameObject(L"JSUI", pGameObject)))
		return E_FAIL;

	m_mapLayer.insert({ pLayerTag, pLayer });

	return S_OK;
}

HRESULT CJSStage::Ready_Light()
{
	D3DLIGHT9   tLightInfo;
	ZeroMemory(&tLightInfo, sizeof(D3DLIGHT9));

	tLightInfo.Type = D3DLIGHT_DIRECTIONAL;

	tLightInfo.Diffuse = D3DXCOLOR(1.f, 1.f, 1.f, 1.f);
	tLightInfo.Specular = D3DXCOLOR(1.f, 1.f, 1.f, 1.f);
	tLightInfo.Ambient = D3DXCOLOR(1.f, 1.f, 1.f, 1.f);

	tLightInfo.Direction = { 1.f, -1.f, 1.f };

	if (FAILED(CLightMgr::GetInstance()->Ready_Light(m_pGraphicDev, &tLightInfo, 0)))
		return E_FAIL;

	return S_OK;
}

HRESULT CJSStage::Ready_Fog()
{
	// 포그 활성화
	m_pGraphicDev->SetRenderState(D3DRS_FOGENABLE, TRUE);

	// 포그 색상 (어두운 남색 계열)
	m_pGraphicDev->SetRenderState(D3DRS_FOGCOLOR, D3DCOLOR_XRGB(50, 87, 90));

	// 선형 포그 (시작, 끝 거리 설정)
	m_pGraphicDev->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_LINEAR);

	_float fStart = 30.f;   // 포그 시작 거리
	_float fEnd = 80.f;   // 포그 끝 거리 (완전히 가려짐)

	m_pGraphicDev->SetRenderState(D3DRS_FOGSTART, *(DWORD*)&fStart);
	m_pGraphicDev->SetRenderState(D3DRS_FOGEND, *(DWORD*)&fEnd);

	return S_OK;
}

void CJSStage::Clear_DeadObject(const _tchar* pLayerTag, const _float& fTimeDelta)
{
	auto iter = m_mapLayer.find(pLayerTag);
	if (iter != m_mapLayer.end())
	{
		iter->second->Delete_GameObject(fTimeDelta);
	}
}

CJSStage* CJSStage::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
	CJSStage* pJSStage = new CJSStage(pGraphicDev);

	if (FAILED(pJSStage->Ready_Scene()))
	{
		Safe_Release(pJSStage);
		MSG_BOX("pJSStage Create Failed");
		return nullptr;
	}

	return pJSStage;
}

void CJSStage::Free()
{
	m_pGraphicDev->SetRenderState(D3DRS_FOGENABLE, FALSE);

	if (m_pWaterPlane)
		Safe_Release(m_pWaterPlane);

	CJSScoreMgr::DestroyInstance();
	CJSChunkMgr::DestroyInstance();
	CScene::Free();
}
