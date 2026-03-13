#include "pch.h"
#include "CLogo.h"
#include "CBackGround.h"
#include "CProtoMgr.h"
#include "CMonster.h"
#include "CPlayer.h"
#include "CStage.h"
#include "CSquidCoast.h"
#include "CFontMgr.h"
#include "CDInputMgr.h"
#include "CEditor.h"
#include "CBlockMgr.h"
#include "CSceneChanger.h"
#include "CRenderer.h"
#include "CUIMetadataMgr.h"
#include "CUITexture.h"
#include "CUIObject.h"

CLogo::CLogo(LPDIRECT3DDEVICE9 pGraphicDev)
    : Engine::CScene(pGraphicDev), m_pEditor(nullptr)
{
}

CLogo::~CLogo()
{
}

HRESULT CLogo::Ready_Scene()
{
    if (FAILED(Ready_Prototype()))
        return E_FAIL;

    if (FAILED(Ready_Environment_Layer(L"Environment_Layer")))
        return E_FAIL;

    return S_OK;
}

_int CLogo::Update_Scene(const _float& fTimeDelta)
{
    bool bF1 = CDInputMgr::GetInstance()->Get_DIKeyState(DIK_F1);

    //������ ���� ����
    if (bF1 && !m_bF1Toggle)
    {
        if (!m_pEditor)
        {
            //���� ������ ���� ī�޶� 
            m_pEditor = CEditor::Create(m_pGraphicDev);
            if (!m_pEditor)
                return -1;
        }
        //ó�� ���� ���� ��۸� ����
        m_pEditor->SetEditorMode(!m_pEditor->IsEditorMode());
    }

    m_bF1Toggle = bF1;

    if (m_pEditor && m_pEditor->IsEditorMode())
    {
        m_pEditor->Update_Scene(fTimeDelta);
        return 0;
    }

    //�ΰ� �� ������Ʈ
    _int iExit = Engine::CScene::Update_Scene(fTimeDelta);

    if (GetAsyncKeyState(VK_RETURN))
    {
        //Render Group Clear Before Change Scene!!!!
        CRenderer::GetInstance()->Clear_RenderGroup();
        if (FAILED(CSceneChanger::ChangeScene(m_pGraphicDev, eSceneType::SCENE_SQUIDCOAST)))
        {
            MSG_BOX("SquidCoast Create Failed");
            return -1;
        }
    }
         
    return iExit;
}

void CLogo::LateUpdate_Scene(const _float& fTimeDelta)
{
    if (m_pEditor && m_pEditor->IsEditorMode())
    {
        m_pEditor->LateUpdate_Scene(fTimeDelta);
        return;
    }

    Engine::CScene::LateUpdate_Scene(fTimeDelta);
}

void CLogo::Render_Scene()
{
    if (m_pEditor && m_pEditor->IsEditorMode())
    {
        m_pEditor->Render_Scene();
        return;
    }
}

HRESULT CLogo::Ready_Environment_Layer(const _tchar* pLayerTag)
{
    CLayer* pLayer = CLayer::Create();

    if (nullptr == pLayer)
        return E_FAIL;

    // ������Ʈ �߰�
    CGameObject* pGameObject = nullptr;

    // back ground
    pGameObject = CBackGround::Create(m_pGraphicDev, L"Proto_MainMenuTexture");

    if (nullptr == pGameObject)
        return E_FAIL;

    if (FAILED(pLayer->Add_GameObject(L"BackGround", pGameObject)))
        return E_FAIL;

	// UI Test Object: Positioned at Bottom-Middle
	// pGameObject = CUIObject::Create(m_pGraphicDev, 100.f, 500.f, 618.f, 96.f, L"Proto_UITexture_Hotbar");
	// if (nullptr != pGameObject)
	// 	pLayer->Add_GameObject(L"UI_Hotbar", pGameObject);

    m_mapLayer.insert({ pLayerTag, pLayer });

    return S_OK;
}

HRESULT CLogo::Ready_Prototype()
{   
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RcTex", Engine::CRcTex::Create(m_pGraphicDev))))
        return E_FAIL;
 
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_MainMenuTexture",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Logo/MainMenu_Screen2.png"))))
        return E_FAIL;

    //�ε��� ���
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_SquidCoastLoadingTexture",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Logo/Loading_Screen_Squid_Coast.png"))))
        return E_FAIL;

	// UI Test Load
	//if (FAILED(Engine::CUIMetadataMgr::GetInstance()->Ready_MetaData(L"../Bin/Resource/Texture/UI/Materials/HotBar2/hotbarBackground.json", L"Texture_HotbarBackground")))
	//	return E_FAIL;
    //
	//if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_UITexture_Hotbar", 
	//	Engine::CUITexture::Create(m_pGraphicDev, L"../Bin/Resource/Texture/UI/Materials/HotBar2/hotbarBackground.png", L"Texture_HotbarBackground"))))
	//	return E_FAIL;

    return S_OK;
}

CLogo* CLogo::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
    CLogo* pLogo = new CLogo(pGraphicDev);

    if (FAILED(pLogo->Ready_Scene()))
    {
        Safe_Release(pLogo);
        MSG_BOX("Logo Create Failed");
        return nullptr;
    }

    return pLogo;
}

void CLogo::Free()
{
    Safe_Release(m_pEditor);
    CScene::Free();
}
