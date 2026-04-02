#include "pch.h"
#include "CTJLevelUpUI.h"
#include "CRenderer.h"

CTJLevelUpUI::CTJLevelUpUI(LPDIRECT3DDEVICE9 pGraphicDev)
    : CGameObject(pGraphicDev)
{
    D3DXMatrixIdentity(&m_matOriginView);
    D3DXMatrixIdentity(&m_matOriginProj);
}

CTJLevelUpUI::~CTJLevelUpUI()
{
}

HRESULT CTJLevelUpUI::Ready_GameObject()
{
    Engine::CComponent* pComponent = nullptr;

    pComponent = m_pBufferCom = dynamic_cast<Engine::CRcTex*>(
        CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RcTex"));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Buffer", pComponent });

    // 능력별 레벨별 텍스처
    const wchar_t* texTags[(int)ETJAbility::ABILITY_END][3] = {
        { L"Proto_Arrow1Texture",   L"Proto_Arrow2Texture",   L"Proto_Arrow3Texture"   },  // ARROW_PLUS
        { L"Proto_TNT1Texture",     L"Proto_TNT2Texture",     L"Proto_TNT3Texture"     },  // TNT_LAUNCHER
        { L"Proto_HP1Texture",      L"Proto_HP2Texture",      L"Proto_HP3Texture"      },  // HP_REGEN
        { L"Proto_Thunder1Texture", L"Proto_Thunder2Texture", L"Proto_Thunder3Texture" },  // LIGHTNING
        { L"Proto_Fire1Texture",    L"Proto_Fire2Texture",    L"Proto_Fire3Texture"    },  // FIRE_TRAIL
        { L"Proto_Blade1Texture",   L"Proto_Blade2Texture",   L"Proto_Blade3Texture"   },  // BLADE_ORBIT
    };

    for (int i = 0; i < (int)ETJAbility::ABILITY_END; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            m_pCardTexture[i][j] = dynamic_cast<Engine::CTexture*>(
                CProtoMgr::GetInstance()->Clone_Prototype(texTags[i][j]));
            if (!m_pCardTexture[i][j]) return E_FAIL;
        }
    }

    return S_OK;
}

_int CTJLevelUpUI::Update_GameObject(const _float& fTimeDelta)
{
    if (!m_bVisible) return 0;

    _int iExit = CGameObject::Update_GameObject(fTimeDelta);

    float fCard0X = 240.f;
    float fCard1X = 490.f;
    float fCard2X = 740.f;

    bool bLBtn = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;

    if (bLBtn && !m_bPrevLBtn)
    {
        if (IsMouseInCard(fCard0X, m_fCardY, m_fCardW, m_fCardH))
        {
            m_iSelected = 0;
            m_eSelectedAbility = m_eCardAbility[0];
        }
        else if (IsMouseInCard(fCard1X, m_fCardY, m_fCardW, m_fCardH))
        {
            m_iSelected = 1;
            m_eSelectedAbility = m_eCardAbility[1];
        }
        else if (IsMouseInCard(fCard2X, m_fCardY, m_fCardW, m_fCardH))
        {
            m_iSelected = 2;
            m_eSelectedAbility = m_eCardAbility[2];
        }
    }
    m_bPrevLBtn = bLBtn;

    CRenderer::GetInstance()->Add_RenderGroup(RENDER_UI, this);

    return iExit;
}

void CTJLevelUpUI::LateUpdate_GameObject(const _float& fTimeDelta)
{
    CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CTJLevelUpUI::Render_GameObject()
{
    if (!m_bVisible) return;

    float fCardX[3] = { 240.f, 490.f, 740.f };

    Render_BeginUI();
    for (int i = 0; i < 3; ++i)
        Render_Card(i, fCardX[i], m_fCardY, m_fCardW, m_fCardH);
    Render_EndUI();
}

void CTJLevelUpUI::Render_Card(int iIndex, float fX, float fY, float fW, float fH)
{
    float fNDCX = (fX + fW * 0.5f) / (WINCX * 0.5f) - 1.f;
    float fNDCY = 1.f - (fY + fH * 0.5f) / (WINCY * 0.5f);

    _matrix matWorld;
    D3DXMatrixTransformation2D(&matWorld,
        nullptr, 0.f,
        &_vec2(fW / WINCX, fH / WINCY),
        nullptr, 0.f,
        &_vec2(fNDCX, fNDCY));

    m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);

    int iAbilityIdx = (int)m_eCardAbility[iIndex];
    int iLevel = m_pCurPlayer ? m_pCurPlayer->Get_AbilityLevel(m_eCardAbility[iIndex]) : 0;
    iLevel = min(iLevel, 2); // 0~2

    if (m_pCardTexture[iAbilityIdx][iLevel])
        m_pCardTexture[iAbilityIdx][iLevel]->Set_Texture(0);

    m_pBufferCom->Render_Buffer();
}

bool CTJLevelUpUI::IsMouseInCard(float fX, float fY, float fW, float fH)
{
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(g_hWnd, &pt);
    return (pt.x >= fX && pt.x <= fX + fW &&
        pt.y >= fY && pt.y <= fY + fH);
}

void CTJLevelUpUI::Show(CTJPlayer* pPlayer)
{
    m_pCurPlayer = pPlayer;
    Pick_RandomAbilities(pPlayer);
    m_bVisible = true;
    m_iSelected = -1;
    m_eSelectedAbility = ETJAbility::ABILITY_END;
}

void CTJLevelUpUI::Hide()
{
    m_bVisible = false;
}

void CTJLevelUpUI::Pick_RandomAbilities(CTJPlayer* pPlayer)
{
    // 선택 가능한 능력 목록
    vector<ETJAbility> vecAvailable;
    for (int i = 0; i < (int)ETJAbility::ABILITY_END; ++i)
    {
        ETJAbility eAbility = (ETJAbility)i;
        if (!pPlayer->Is_AbilityMaxed(eAbility))
            vecAvailable.push_back(eAbility);
    }

    // 셔플
    for (int i = (int)vecAvailable.size() - 1; i > 0; --i)
    {
        int j = rand() % (i + 1);
        swap(vecAvailable[i], vecAvailable[j]);
    }

    // 최대 3개 선택
    for (int i = 0; i < 3; ++i)
    {
        if (i < (int)vecAvailable.size())
            m_eCardAbility[i] = vecAvailable[i];
        else
            m_eCardAbility[i] = vecAvailable[0];
    }
}

void CTJLevelUpUI::Render_BeginUI()
{
    m_pGraphicDev->GetTransform(D3DTS_VIEW, &m_matOriginView);
    m_pGraphicDev->GetTransform(D3DTS_PROJECTION, &m_matOriginProj);

    _matrix matIdentity;
    D3DXMatrixIdentity(&matIdentity);
    m_pGraphicDev->SetTransform(D3DTS_VIEW, &matIdentity);
    m_pGraphicDev->SetTransform(D3DTS_PROJECTION, &matIdentity);

    m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    m_pGraphicDev->SetRenderState(D3DRS_ZENABLE, FALSE);
    m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    m_pGraphicDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    m_pGraphicDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
    m_pGraphicDev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
    m_pGraphicDev->SetRenderState(D3DRS_ALPHAREF, 0x10);
}

void CTJLevelUpUI::Render_EndUI()
{
    m_pGraphicDev->SetTransform(D3DTS_VIEW, &m_matOriginView);
    m_pGraphicDev->SetTransform(D3DTS_PROJECTION, &m_matOriginProj);

    m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
    m_pGraphicDev->SetRenderState(D3DRS_ZENABLE, TRUE);
    m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
}

CTJLevelUpUI* CTJLevelUpUI::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
    CTJLevelUpUI* pUI = new CTJLevelUpUI(pGraphicDev);

    if (FAILED(pUI->Ready_GameObject()))
    {
        Safe_Release(pUI);
        MSG_BOX("CTJLevelUpUI Create Failed");
        return nullptr;
    }

    return pUI;
}

void CTJLevelUpUI::Free()
{
    CGameObject::Free();
}