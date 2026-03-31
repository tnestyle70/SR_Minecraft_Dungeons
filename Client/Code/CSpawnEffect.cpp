#include "pch.h"
#include "CSpawnEffect.h"

CSpawnEffect::CSpawnEffect(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vPos)
    : m_pGraphicDev(pGraphicDev), m_vPos(vPos)
{
    m_pGraphicDev->AddRef();

    m_pSpawnCube = Engine::CCubeTex::Create(m_pGraphicDev);

    D3DLIGHT9 tLight;
    ZeroMemory(&tLight, sizeof(D3DLIGHT9));
    tLight.Type = D3DLIGHT_POINT;
    tLight.Diffuse = { 0.7f, 0.4f, 1.0f, 1.0f };
    tLight.Ambient = { 0.f, 0.f, 0.f, 1.f };
    tLight.Position = { m_vPos.x, m_vPos.y + 1.5f, m_vPos.z };
    tLight.Range = 0.f;
    tLight.Attenuation0 = 0.f;
    tLight.Attenuation1 = 0.08f;
    tLight.Attenuation2 = 0.f;

    m_pGraphicDev->SetLight(m_iLightIndex, &tLight);
    m_pGraphicDev->LightEnable(m_iLightIndex, TRUE);
}

CSpawnEffect::~CSpawnEffect()
{
    m_pGraphicDev->LightEnable(m_iLightIndex, FALSE);
    Safe_Release(m_pSpawnCube);
    Safe_Release(m_pGraphicDev);
}

void CSpawnEffect::Update(const _float& fTimeDelta)
{
    if (m_bDone) return;

    m_fTimer += fTimeDelta;

    _float      fRange = 0.f;
    D3DXCOLOR   tColor = { 0.7f, 0.4f, 1.0f, 1.0f };

    if (m_fTimer < m_fExpandEnd)
    {
        // Phase 1: 빠른 확장 (0 ~ 0.15s)
        _float fRatio = m_fTimer / m_fExpandEnd;
        fRange = m_fMaxRange * fRatio;
        tColor = { 0.7f, 0.4f, 1.0f, 1.0f };
    }
    else if (m_fTimer < m_fGlowEnd)
    {
        // Phase 2: 유지 + 밝아짐 (0.15 ~ 0.50s)
        _float fRatio = (m_fTimer - m_fExpandEnd) / (m_fGlowEnd - m_fExpandEnd);
        fRange = m_fMaxRange;
        _float fR = 0.7f + 0.3f * fRatio;   // 0.7 -> 1.0
        _float fG = 0.4f + 0.3f * fRatio;   // 0.4 -> 0.7
        tColor = { fR, fG, 1.0f, 1.0f };
    }
    else if (m_fTimer < m_fDecayEnd)
    {
        // Phase 3: 감쇠 (0.50 ~ 1.20s)
        _float fRatio = (m_fTimer - m_fGlowEnd) / (m_fDecayEnd - m_fGlowEnd);
        fRange = m_fMaxRange * (1.f - fRatio);
        _float fR = 1.0f - 0.5f * fRatio;   // 1.0 -> 0.5
        _float fG = 0.7f - 0.5f * fRatio;   // 0.7 -> 0.2
        tColor = { fR, fG, 1.0f, 1.0f };
    }
    else
    {
        m_bDone = true;
        return;
    }

    D3DLIGHT9 tLight;
    ZeroMemory(&tLight, sizeof(D3DLIGHT9));
    tLight.Type = D3DLIGHT_POINT;
    tLight.Diffuse = tColor;
    tLight.Position = { m_vPos.x, m_vPos.y + 1.5f, m_vPos.z };
    tLight.Range = fRange;
    tLight.Attenuation0 = 0.f;
    tLight.Attenuation1 = 0.08f;
    tLight.Attenuation2 = 0.f;

    m_pGraphicDev->SetLight(m_iLightIndex, &tLight);
}

void CSpawnEffect::Render()
{
    if (m_fTimer >= m_fCubeEnd || !m_pSpawnCube) return;

    _float fRatio = m_fTimer / m_fCubeEnd;
    _float fScale = m_fMaxScale * fRatio;
    _float fAlpha = m_fMaxAlpha * (1.f - fRatio);
    DWORD  dwAlpha = (DWORD)(fAlpha * 255.f);

    D3DXMATRIX matScale, matTrans, matWorld;
    D3DXMatrixScaling(&matScale, fScale, fScale, fScale);
    D3DXMatrixTranslation(&matTrans, m_vPos.x, m_vPos.y + 1.5f, m_vPos.z);
    matWorld = matScale * matTrans;

    // 렌더 스테이트 저장
    DWORD dwOldAlpha, dwOldSrc, dwOldDst, dwOldZWrite,
        dwOldLighting, dwOldCull;
    m_pGraphicDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwOldAlpha);
    m_pGraphicDev->GetRenderState(D3DRS_SRCBLEND, &dwOldSrc);
    m_pGraphicDev->GetRenderState(D3DRS_DESTBLEND, &dwOldDst);
    m_pGraphicDev->GetRenderState(D3DRS_ZWRITEENABLE, &dwOldZWrite);
    m_pGraphicDev->GetRenderState(D3DRS_LIGHTING, &dwOldLighting);
    m_pGraphicDev->GetRenderState(D3DRS_CULLMODE, &dwOldCull);

    // 블렌딩 설정 (Additive)
    m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    m_pGraphicDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    m_pGraphicDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
    m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    m_pGraphicDev->SetRenderState(D3DRS_LIGHTING, FALSE);
    m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

    // 텍스처 팩터 (연한 보라색)
    m_pGraphicDev->SetTexture(0, nullptr);
    m_pGraphicDev->SetRenderState(D3DRS_TEXTUREFACTOR,
        D3DCOLOR_ARGB(dwAlpha, 180, 100, 255));
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);

    m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);
    m_pSpawnCube->Render_Buffer();

    // 렌더 스테이트 복원
    m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, dwOldAlpha);
    m_pGraphicDev->SetRenderState(D3DRS_SRCBLEND, dwOldSrc);
    m_pGraphicDev->SetRenderState(D3DRS_DESTBLEND, dwOldDst);
    m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, dwOldZWrite);
    m_pGraphicDev->SetRenderState(D3DRS_LIGHTING, dwOldLighting);
    m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, dwOldCull);
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
}
