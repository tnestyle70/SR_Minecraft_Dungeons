#include "pch.h"
#include "CJSScoreUI.h"
#include "CFontMgr.h"
#include "CRenderer.h"
#include "CJSScoreMgr.h"
#include "CJSChunkMgr.h"
#include "CDInputMgr.h"
#include "CSceneChanger.h"

CJSScoreUI::CJSScoreUI(LPDIRECT3DDEVICE9 pGraphicDev)
    : CGameObject(pGraphicDev)
{
}

CJSScoreUI::~CJSScoreUI()
{
}

HRESULT CJSScoreUI::Ready_GameObject()
{
    if (FAILED(Add_Component()))
        return E_FAIL;

    if (FAILED(CFontMgr::GetInstance()->Ready_Font(
        m_pGraphicDev,
        L"Font_Score",
        L"Minecraft",
        20, 40, 700)))
        return E_FAIL;

    if (FAILED(CFontMgr::GetInstance()->Ready_Font(
        m_pGraphicDev,
        L"Font_Countdown",
        L"Minecraft",
        80, 160, 700)))
        return E_FAIL;

    return S_OK;
}

_int CJSScoreUI::Update_GameObject(const _float& fTimeDelta)
{
    _int iExit = CGameObject::Update_GameObject(fTimeDelta);

    if (CJSScoreMgr::GetInstance()->Is_GameOver())
    {
        if (CDInputMgr::GetInstance()->Get_DIMouseState(DIM_LB))
            CJSScoreMgr::GetInstance()->Set_ExitReserved();
    }

    return iExit;
}

void CJSScoreUI::LateUpdate_GameObject(const _float& fTimeDelta)
{
    CGameObject::LateUpdate_GameObject(fTimeDelta);

    CRenderer::GetInstance()->Add_RenderGroup(RENDER_UI, this);
}

void CJSScoreUI::Render_GameObject()
{
    //Render_Overlay();

    if (CJSScoreMgr::GetInstance()->Is_GameOver())
    {
        Render_GameOver();
        return;
    }

    JSGAMESTAGE eStage = CJSScoreMgr::GetInstance()->Get_Stage();

    if (eStage == JSSTAGE_COUNTDOWN)
    {
        _int iCount = CJSScoreMgr::GetInstance()->Get_Countdown();
        TCHAR szCount[8];

        if (iCount > 0)
            wsprintf(szCount, L"%d", iCount);
        else
            wsprintf(szCount, L"GO!");

        _vec2 vPos = { WINCX * 0.5f - 20.f, WINCY * 0.5f - 40.f };
        CFontMgr::GetInstance()->Render_Font(L"Font_Countdown", szCount, &vPos, D3DXCOLOR(1.f, 1.f, 0.f, 1.f));
        return;
    }

    if (eStage != JSSTAGE_PLAY)
        return;

    m_iScore = CJSScoreMgr::GetInstance()->Get_Score();
    m_fDistance = CJSScoreMgr::GetInstance()->Get_Distance();
    m_fSpeed = CJSScoreMgr::GetInstance()->Get_Speed();

    //TCHAR szScore[64];
    //wsprintf(szScore, L"Score: %d", m_iScore);
    //_vec2 vScorePos = { 10.f, 10.f };
    //CFontMgr::GetInstance()->Render_Font(L"Font_Score", szScore, &vScorePos, D3DXCOLOR(0.f, 1.f, 0.f, 1.f));

    Render_Score();

    TCHAR szDist[64];
    wsprintf(szDist, L"Distance: %d m", (_int)m_fDistance);
    _vec2 vDistPos = { 10.f, 10.f };
    CFontMgr::GetInstance()->Render_Font(L"Font_Score", szDist, &vDistPos, D3DXCOLOR(1.f, 1.f, 1.f, 1.f));

    TCHAR szSpeed[64];
    wsprintf(szSpeed, L"Speed: %d", (_int)m_fSpeed);
    _vec2 vSpeedPos = { 10.f, 60.f };
    CFontMgr::GetInstance()->Render_Font(L"Font_Score", szSpeed, &vSpeedPos, D3DXCOLOR(1.f, 1.f, 1.f, 1.f));
}

HRESULT CJSScoreUI::Add_Component()
{
    CComponent* pComponent = nullptr;

    pComponent = m_pEmeraldBuf = dynamic_cast<CRcTex*>(
        CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RcTex"));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_EmeraldBuf", pComponent });

    pComponent = m_pEmeraldTex = dynamic_cast<CTexture*>(
        CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_EmeraldTexture"));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_EmeraldTex", pComponent });

    pComponent = m_pGameOverBuf = dynamic_cast<CRcTex*>(
        CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RcTex"));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_GameOverBuf", pComponent });

    pComponent = m_pGameOverTex = dynamic_cast<CTexture*>(
        CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_JSGameOverTexture"));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_GameOverTex", pComponent });

    return S_OK;
}

void CJSScoreUI::Render_Score()
{
    _matrix matViewOld, matProjOld;
    m_pGraphicDev->GetTransform(D3DTS_VIEW, &matViewOld);
    m_pGraphicDev->GetTransform(D3DTS_PROJECTION, &matProjOld);

    _matrix matView, matProj;
    D3DXMatrixIdentity(&matView);
    D3DXMatrixOrthoLH(&matProj, (_float)WINCX, (_float)WINCY, 0.f, 1.f);
    m_pGraphicDev->SetTransform(D3DTS_VIEW, &matView);
    m_pGraphicDev->SetTransform(D3DTS_PROJECTION, &matProj);

    _matrix matWorld;
    D3DXMatrixScaling(&matWorld, 40.f, 40.f, 1.f);
    matWorld._41 = 30.f - WINCX * 0.5f + 20.f;   // x 더 오른쪽으로
    matWorld._42 = WINCY * 0.5f - 120.f - 20.f;  // y 그대로

    m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);

    m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    m_pGraphicDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    m_pGraphicDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    m_pEmeraldTex->Set_Texture(0);
    m_pEmeraldBuf->Render_Buffer();
    m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

    TCHAR szScore[32];
    wsprintf(szScore, L"x %d", m_iScore);
    // 스코어 위치 조정
    _vec2 vScorePos = { 85.f, 120.f };  // x 더 오른쪽, y 살짝 아래
    CFontMgr::GetInstance()->Render_Font(L"Font_Score", szScore, &vScorePos, D3DXCOLOR(1.f, 1.f, 1.f, 1.f));

    m_pGraphicDev->SetTransform(D3DTS_VIEW, &matViewOld);
    m_pGraphicDev->SetTransform(D3DTS_PROJECTION, &matProjOld);
}

void CJSScoreUI::Render_GameOver()
{
    _matrix matViewOld, matProjOld;
    m_pGraphicDev->GetTransform(D3DTS_VIEW, &matViewOld);
    m_pGraphicDev->GetTransform(D3DTS_PROJECTION, &matProjOld);

    _matrix matView, matProj;
    D3DXMatrixIdentity(&matView);
    D3DXMatrixOrthoLH(&matProj, (_float)WINCX, (_float)WINCY, 0.f, 1.f);
    m_pGraphicDev->SetTransform(D3DTS_VIEW, &matView);
    m_pGraphicDev->SetTransform(D3DTS_PROJECTION, &matProj);

    // 게임오버 텍스쳐 화면 중앙에
    _matrix matWorld;
    D3DXMatrixScaling(&matWorld, 400.f, 200.f, 1.f);
    matWorld._41 = 0.f;
    matWorld._42 = 0.f;

    m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);
    m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    m_pGraphicDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    m_pGraphicDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    m_pGameOverTex->Set_Texture(0);
    m_pGameOverBuf->Render_Buffer();
    m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

    // 최종 스코어 표시
    m_iScore = CJSScoreMgr::GetInstance()->Get_Score();
    m_fDistance = CJSScoreMgr::GetInstance()->Get_Distance();
    m_fSpeed = CJSScoreMgr::GetInstance()->Get_Speed();

    TCHAR szDist[64];
    wsprintf(szDist, L"%d m", (_int)m_fDistance);
    _vec2 vDistPos = { WINCX * 0.5f + 80.f, WINCY * 0.5f - 80.f };
    CFontMgr::GetInstance()->Render_Font(L"Font_Score", szDist, &vDistPos, D3DXCOLOR(1.f, 1.f, 1.f, 1.f));

    TCHAR szScore[64];
    wsprintf(szScore, L"%d", m_iScore);
    _vec2 vScorePos = { WINCX * 0.5f + 115.f, WINCY * 0.5f - 17.f };
    CFontMgr::GetInstance()->Render_Font(L"Font_Score", szScore, &vScorePos, D3DXCOLOR(1.f, 1.f, 1.f, 1.f));

    TCHAR szSpeed[64];
    wsprintf(szSpeed, L"%d", (_int)m_fSpeed);
    _vec2 vSpeedPos = { WINCX * 0.5f + 115.f, WINCY * 0.5f + 45.f };
    CFontMgr::GetInstance()->Render_Font(L"Font_Score", szSpeed, &vSpeedPos, D3DXCOLOR(1.f, 1.f, 1.f, 1.f));

    TCHAR szExit[32];
    wsprintf(szExit, L"Exit");
    _vec2 vExitPos = { WINCX * 0.5f - 50.f, WINCY * 0.5f + 120.f };
    CFontMgr::GetInstance()->Render_Font(L"Font_Score", szExit, &vExitPos, D3DXCOLOR(1.f, 1.f, 0.f, 1.f));

    m_pGraphicDev->SetTransform(D3DTS_VIEW, &matViewOld);
    m_pGraphicDev->SetTransform(D3DTS_PROJECTION, &matProjOld);
}

//void CJSScoreUI::Render_Overlay()
//{
//    // 뷰/프로젝션 행렬 저장
//    _matrix matViewOld, matProjOld;
//    m_pGraphicDev->GetTransform(D3DTS_VIEW, &matViewOld);
//    m_pGraphicDev->GetTransform(D3DTS_PROJECTION, &matProjOld);
//
//    // 직교 투영으로 변경
//    _matrix matView, matProj;
//    D3DXMatrixIdentity(&matView);
//    D3DXMatrixOrthoLH(&matProj, (_float)WINCX, (_float)WINCY, 0.f, 1.f);
//    m_pGraphicDev->SetTransform(D3DTS_VIEW, &matView);
//    m_pGraphicDev->SetTransform(D3DTS_PROJECTION, &matProj);
//
//    // 화면 전체 크기로 스케일
//    _matrix matWorld;
//    D3DXMatrixScaling(&matWorld, (_float)WINCX * 0.5f, (_float)WINCY * 0.5f, 1.f);
//    m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);
//
//    m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
//    m_pGraphicDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
//    m_pGraphicDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
//
//    m_pTextureCom->Set_Texture(0);
//    m_pBufferCom->Render_Buffer();
//
//    m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
//
//    // 원래 행렬 복구
//    m_pGraphicDev->SetTransform(D3DTS_VIEW, &matViewOld);
//    m_pGraphicDev->SetTransform(D3DTS_PROJECTION, &matProjOld);
//}

CJSScoreUI* CJSScoreUI::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
    CJSScoreUI* pUI = new CJSScoreUI(pGraphicDev);

    if (FAILED(pUI->Ready_GameObject()))
    {
        Safe_Release(pUI);
        MSG_BOX("JSScoreUI Create Failed");
        return nullptr;
    }

    return pUI;
}

void CJSScoreUI::Free()
{
    CGameObject::Free();
}