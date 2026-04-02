#include "pch.h"
#include "CJSScoreUI.h"
#include "CFontMgr.h"
#include "CRenderer.h"
#include "CJSScoreMgr.h"

CJSScoreUI::CJSScoreUI(LPDIRECT3DDEVICE9 pGraphicDev)
    : CGameObject(pGraphicDev)
{
}

CJSScoreUI::~CJSScoreUI()
{
}

HRESULT CJSScoreUI::Ready_GameObject()
{
    if (FAILED(CFontMgr::GetInstance()->Ready_Font(
        m_pGraphicDev,
        L"Font_Score",
        L"Minecraft",
        20, 40, 700)))
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