#include "pch.h"
#include "CJSStage.h"
#include "CDynamicCamera.h"
#include "CSceneChanger.h"
#include "CRenderer.h"

CJSStage::CJSStage(LPDIRECT3DDEVICE9 pGraphicDev)
	:CScene(pGraphicDev)
{
}

CJSStage::~CJSStage()
{
}

HRESULT CJSStage::Ready_Scene()
{
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

	//dynamic camera
	_vec3 vEye{ 0.f, 10.f, -10.f };
	_vec3 vAt{ 0.f, 0.f, 1.f };
	_vec3 vUp{ 0.f, 1.f, 0.f };

	if (FAILED(pLayer->Add_GameObject(L"DynamicCamera", pGameObject)))
		return E_FAIL;

	//SkyBox 추가

	m_mapLayer.insert({ pLayerTag, pLayer });

	return S_OK;
}

HRESULT CJSStage::Ready_GameLogic_Layer(const _tchar* pLayerTag)
{
	CLayer* pLayer = CLayer::Create();

	if (!pLayer)
		return E_FAIL;

	CGameObject* pGameObject = nullptr;

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
	CScene::Free();
}
