#include "pch.h"
#include "CJSStage.h"
#include "CSceneChanger.h"
#include "CRenderer.h"
#include "CJSCamera.h"
#include "CJSChunkMgr.h"
#include "CJSPlayer.h"
#include "CManagement.h"
#include "CSoundMgr.h"

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
	CSoundMgr::GetInstance()->PlayBGM(L"BGM/BGM_JSStage.wav", 0.6f);

	if (FAILED(Ready_Light()))
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
	_int iExit = CScene::Update_Scene(fTimeDelta);

	_vec3 vPlayerPos;
	CTransform* pPlayerTrans = dynamic_cast<CTransform*>(CManagement::GetInstance()->Get_Component(ID_DYNAMIC, L"GameLogic_Layer", L"JSPlayer", L"Com_Transform"));
	pPlayerTrans->Get_Info(INFO_POS, &vPlayerPos);

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
	_vec3 vEye{ 0.f, 10.f, -10.f };
	_vec3 vAt{ 0.f, 0.f, 1.f };
	_vec3 vUp{ 0.f, 1.f, 0.f };

	pGameObject = CJSCamera::Create(m_pGraphicDev, &vEye, &vAt, &vUp);

	if (!pGameObject)
		return E_FAIL;

	if (FAILED(pLayer->Add_GameObject(L"JSCamera", pGameObject)))
		return E_FAIL;

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

	m_mapLayer.insert({ pLayerTag, pLayer });

	return S_OK;
}

HRESULT CJSStage::Ready_Light()
{
	return S_OK;
}

void CJSStage::Clear_DeadObject(const _tchar* pLayerTag, const _float& fTimeDelta)
{
	auto iter = m_mapLayer.find(pLayerTag);
	if (iter != m_mapLayer.end())
	{
		iter->second->Delete_GameObject(fTimeDelta);

		// 사이즈 확인
		TCHAR szBuf[64];
		wsprintf(szBuf, L"MapObject Size: %d", iter->second->Get_MapSize());
		OutputDebugString(szBuf);
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
	CJSChunkMgr::DestroyInstance();
	CScene::Free();
}
