#include "pch.h"
#include "CLoadingScene.h"
#include "CFontMgr.h"
#include "CRenderer.h"
#include "CLoadingBlock.h"
#include "CLoadingTexture.h"

CLoadingScene::CLoadingScene(LPDIRECT3DDEVICE9 pGraphicDev)
	:CScene(pGraphicDev)
{
}

CLoadingScene::~CLoadingScene()
{
}

HRESULT CLoadingScene::Ready_Scene()
{
	//Initialize Camera!!
	_matrix matView, matProj;
	D3DXMatrixIdentity(&matView);
	D3DXMatrixIdentity(&matProj);
	
	// 현재 윈도우 해상도를 가져옵니다 (예: 1280, 720)
	float fWidth = 1280.f;
	float fHeight = 720.f;
	float fAspect = fWidth / fHeight;

	// 가로 종횡비를 반영한 직교 투영 행렬 생성
	// 화면 중앙을 (0,0)으로 하고, 가로 범위를 -fAspect ~ fAspect로 설정
	D3DXMatrixOrthoLH(&matProj, 2.0f * fAspect, 2.0f, 0.f, 1.f);

	m_pGraphicDev->SetTransform(D3DTS_PROJECTION, &matProj);
	//m_pGraphicDev->SetTransform(D3DTS_PROJECTION, &matProj);
	m_pGraphicDev->SetTransform(D3DTS_VIEW, &matView);

	//로딩 텍스쳐
	m_pLoadingTexture = CBackGround::Create(m_pGraphicDev, m_pTextureName);
	if (!m_pLoadingTexture)
		return E_FAIL;

	//로딩 블럭
	m_pLoadingBlock = CLoadingBlock::Create(m_pGraphicDev);
	if (!m_pLoadingBlock)
		return E_FAIL;

	//백그라운드 로딩 쓰레드 돌리기
	m_pLoading = CLoading::Create(m_pGraphicDev, m_eLoadingID);
	if (!m_pLoading)
		return E_FAIL;

	//NextScene에 따른 텍스쳐 ProtoName 설정
	const wchar_t* pTextTexture = nullptr;
	
	switch (m_eNextScene)
	{
	case SCENE_SQUIDCOAST_PLAY:
		pTextTexture = L"Proto_SquidCoastText";
		break;
	case SCENE_CAMP_PLAY:
		pTextTexture = L"Proto_CampText";
		break;
	case SCENE_JS_PLAY:
		pTextTexture = L"Proto_JSText";
		break;
	case SCENE_TJ_PLAY:
		pTextTexture = L"Proto_TJText";
		break;
	case SCENE_CY_PLAY:
		pTextTexture = L"Proto_CYText";
		break;
	case SCENE_NETWORK_PLAY:
		pTextTexture = L"Proto_GBText";
		break;
	case SCENE_END:
		break;
	default:
		break;
	}

	m_pTextTexture = CLoadingTexture::Create(m_pGraphicDev, pTextTexture);

	if (!m_pTextTexture)
		return E_FAIL;
	
	return S_OK;
}

_int CLoadingScene::Update_Scene(const _float& fTimeDelta)
{
	//로딩 블럭
	m_pLoadingBlock->Update_GameObject(fTimeDelta);

	//로딩 텍스쳐
	//CRenderer::GetInstance()->Add_RenderGroup(RENDER_NONALPHA, m_pLoadingTexture);
	
	if (m_bRenderOnce)
	{
		m_fDisplayTimer += fTimeDelta;
	}

	if (m_bRenderOnce &&  !m_bSceneChanged && m_pLoading->Get_Finish() && m_fDisplayTimer >= m_fMinDisplayTime)
	{
		m_bSceneChanged = true;

		//Render Group Clear Before Change Scene!!!!
		CRenderer::GetInstance()->Clear_RenderGroup();

		if (FAILED(CSceneChanger::ChangeScene(m_pGraphicDev, m_eNextScene)))
		{
			MSG_BOX("CLoadingScene: Scene Change Failed");
			return -1;
		}
	}

	return 0;
}

void CLoadingScene::LateUpdate_Scene(const _float& fTimeDelta)
{
	//로딩 블럭
	m_pLoadingBlock->LateUpdate_GameObject(fTimeDelta);
}

void CLoadingScene::Render_Scene()
{	
	m_bRenderOnce = true;

	m_pLoadingTexture->Render_GameObject();

	m_pTextTexture->Render_GameObject();

	m_pLoadingBlock->Render_GameObject();
	
	//로딩 진행 텍스트
	//_vec2 vPos{ 0.f, 0.f };
	//CFontMgr::GetInstance()->Render_Font(
	//	L"Font_Minecraft", m_pLoading->Get_String(), &vPos, D3DXCOLOR(1.f, 1.f, 1.f, 1.f));
}

void CLoadingScene::Render_UI()
{
	_vec2 vStagePos;

	switch (m_eNextScene)
	{
	case SCENE_SQUIDCOAST_PLAY:
		vStagePos = { 0.f, 0.f };
		break;
	case SCENE_CAMP_PLAY:
		vStagePos = { 0.f, 0.f };
		break;
	case SCENE_REDSTONE_PLAY:
		vStagePos = { 0.f, 0.f };
		break;
	case SCENE_OBSIDIAN_PLAY:
		vStagePos = { 0.f, 0.f };
		break;
	default:
		break;
	}
}

CLoadingScene* CLoadingScene::Create(LPDIRECT3DDEVICE9 pGraphicDev,
	CLoading::LOADINGID eLoadingID,
	eSceneType eNextScene,
	const _tchar* pLoadingTexture)
{
	CLoadingScene* pScene = new CLoadingScene(pGraphicDev);

	pScene->m_eLoadingID = eLoadingID;
	pScene->m_eNextScene = eNextScene;
	pScene->m_pTextureName = pLoadingTexture;

	if (FAILED(pScene->Ready_Scene()))
	{
		Safe_Release(pScene);
		MSG_BOX("CLoadingScene Create Failed");
		return nullptr;
	}

	return pScene;
}

void CLoadingScene::Free()
{
	m_pLoadingBlock = nullptr;

	Safe_Release(m_pLoading);
	Safe_Release(m_pLoadingTexture);
	CScene::Free();
}
