#include "pch.h"
#include "CJSScoreUI.h"
#include "CFontMgr.h"
#include "CRenderer.h"
#include "CJSScoreMgr.h"
#include "CJSChunkMgr.h"

CJSScoreUI::CJSScoreUI(LPDIRECT3DDEVICE9 pGraphicDev)
    : CGameObject(pGraphicDev)
{
}

CJSScoreUI::~CJSScoreUI()
{
}

HRESULT CJSScoreUI::Ready_GameObject()
{
    //if (FAILED(Add_Component()))
    //    return E_FAIL;

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
        60, 120, 700)))
        return E_FAIL;

    return S_OK;
}

_int CJSScoreUI::Update_GameObject(const _float& fTimeDelta)
{
    _int iExit = CGameObject::Update_GameObject(fTimeDelta);

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

    TCHAR szScore[64];
    wsprintf(szScore, L"Score: %d", m_iScore);
    _vec2 vScorePos = { 10.f, 10.f };
    CFontMgr::GetInstance()->Render_Font(L"Font_Score", szScore, &vScorePos, D3DXCOLOR(0.f, 1.f, 0.f, 1.f));

    TCHAR szDist[64];
    wsprintf(szDist, L"Distance: %d m", (_int)m_fDistance);
    _vec2 vDistPos = { 10.f, 60.f };
    CFontMgr::GetInstance()->Render_Font(L"Font_Score", szDist, &vDistPos, D3DXCOLOR(1.f, 1.f, 1.f, 1.f));

    TCHAR szSpeed[64];
    wsprintf(szSpeed, L"Speed: %d", (_int)m_fSpeed);
    _vec2 vSpeedPos = { 10.f, 110.f };
    CFontMgr::GetInstance()->Render_Font(L"Font_Score", szSpeed, &vSpeedPos, D3DXCOLOR(1.f, 1.f, 1.f, 1.f));
}

//HRESULT CJSScoreUI::Add_Component()
//{
//    CComponent* pComponent = nullptr;
//
//    pComponent = m_pBufferCom = dynamic_cast<CRcTex*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RcTex"));
//
//    if (!pComponent)
//        return E_FAIL;
//
//    m_mapComponent[ID_STATIC].insert({ L"Com_Buffer", pComponent });
//
//    pComponent = m_pTextureCom = dynamic_cast<CTexture*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_JSUITexture"));
//
//    if (!pComponent)
//        return E_FAIL;
//
//    m_mapComponent[ID_STATIC].insert({ L"Com_Texture", pComponent });
//
//    return S_OK;
//}

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