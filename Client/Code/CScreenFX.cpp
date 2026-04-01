#include "pch.h"
#include "CScreenFX.h"
#include <string>

IMPLEMENT_SINGLETON(CScreenFX)

namespace
{
    struct QUAD_VERTEX
    {
        float x, y, z;
        float u, v;
    };

    const DWORD QUAD_FVF = D3DFVF_XYZ | D3DFVF_TEX1;

    const QUAD_VERTEX kQuad[4] =
    {
        { -1.f,  1.f, 0.f, 0.f, 0.f },
        {  1.f,  1.f, 0.f, 1.f, 0.f },
        { -1.f, -1.f, 0.f, 0.f, 1.f },
        {  1.f, -1.f, 0.f, 1.f, 1.f }
    };
}

CScreenFX::CScreenFX() {}
CScreenFX::~CScreenFX() { Free(); }

HRESULT CScreenFX::Ready(LPDIRECT3DDEVICE9 pDev, _uint iWidth, _uint iHeight)
{
    if (!pDev) return E_FAIL;

    m_pGraphicDev = pDev;
    m_pGraphicDev->AddRef();
    m_iWidth = iWidth;
    m_iHeight = iHeight;
    m_fScreenW = static_cast<float>(iWidth);
    m_fScreenH = static_cast<float>(iHeight);

    if (FAILED(Create_RenderTarget(iWidth, iHeight)))
        return E_FAIL;
    if (FAILED(Create_PingPongRT(iWidth, iHeight)))
        return E_FAIL;
    if (FAILED(Create_DepthBuffer(iWidth, iHeight)))
        return E_FAIL;
    if (FAILED(Create_QuadVB()))
        return E_FAIL;
    if (FAILED(Create_NoiseTex()))
        return E_FAIL;
    if (FAILED(Load_Effect()))
        return E_FAIL;

    m_bReady = true;
    return S_OK;
}

HRESULT CScreenFX::Create_RenderTarget(_uint iW, _uint iH)
{
    if (FAILED(D3DXCreateTexture(
        m_pGraphicDev, iW, iH, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pRT)))
    {
        MSG_BOX("CScreenFX RT texture create failed");
        return E_FAIL;
    }

    if (FAILED(m_pRT->GetSurfaceLevel(0, &m_pRTSurface)))
        return E_FAIL;

    return S_OK;
}

HRESULT CScreenFX::Create_PingPongRT(_uint iW, _uint iH)
{
    if (FAILED(D3DXCreateTexture(
        m_pGraphicDev, iW, iH, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pRT_B)))
    {
        MSG_BOX("CScreenFX ping-pong RT texture create failed");
        return E_FAIL;
    }

    if (FAILED(m_pRT_B->GetSurfaceLevel(0, &m_pRTSurface_B)))
        return E_FAIL;

    return S_OK;
}

HRESULT CScreenFX::Create_DepthBuffer(_uint iW, _uint iH)
{
    if (FAILED(m_pGraphicDev->CreateDepthStencilSurface(
        iW, iH, D3DFMT_D24S8, D3DMULTISAMPLE_NONE, 0, TRUE, &m_pRTDepth, nullptr)))
    {
        MSG_BOX("CScreenFX depth create failed");
        return E_FAIL;
    }
    return S_OK;
}

HRESULT CScreenFX::Create_QuadVB()
{
    if (FAILED(m_pGraphicDev->CreateVertexBuffer(
        sizeof(QUAD_VERTEX) * 4, D3DUSAGE_WRITEONLY, QUAD_FVF, D3DPOOL_MANAGED, &m_pQuadVB, nullptr)))
    {
        MSG_BOX("CScreenFX quad vb create failed");
        return E_FAIL;
    }

    QUAD_VERTEX* pVerts = nullptr;
    m_pQuadVB->Lock(0, 0, reinterpret_cast<void**>(&pVerts), 0);
    memcpy(pVerts, kQuad, sizeof(kQuad));
    m_pQuadVB->Unlock();
    return S_OK;
}

HRESULT CScreenFX::Create_NoiseTex()
{
    const _uint kSize = 64;

    if (FAILED(D3DXCreateTexture(
        m_pGraphicDev, kSize, kSize, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &m_pNoiseTex)))
    {
        MSG_BOX("CScreenFX noise texture create failed");
        return E_FAIL;
    }

    D3DLOCKED_RECT lr{};
    m_pNoiseTex->LockRect(0, &lr, nullptr, 0);
    DWORD* pBits = reinterpret_cast<DWORD*>(lr.pBits);

    srand(12345u);
    for (_uint y = 0; y < kSize; ++y)
    {
        for (_uint x = 0; x < kSize; ++x)
        {
            BYTE r = static_cast<BYTE>(rand() & 0xFF);
            BYTE g = static_cast<BYTE>(rand() & 0xFF);
            BYTE b = static_cast<BYTE>(rand() & 0xFF);
            pBits[y * (lr.Pitch / 4) + x] = D3DCOLOR_ARGB(0xFF, r, g, b);
        }
    }
    m_pNoiseTex->UnlockRect(0);
    return S_OK;
}

HRESULT CScreenFX::Load_Effect()
{
    ID3DXBuffer* pErrBuf = nullptr;
    HRESULT hr = D3DXCreateEffectFromFile(
        m_pGraphicDev, L"../Bin/Shader/PostProcess.fx", nullptr, nullptr, 0, nullptr, &m_pEffect, &pErrBuf);

    if (FAILED(hr))
    {
        if (pErrBuf)
        {
            const char* pMsg = static_cast<const char*>(pErrBuf->GetBufferPointer());
            int len = MultiByteToWideChar(CP_ACP, 0, pMsg, -1, nullptr, 0);
            std::wstring wMsg(len, L'\0');
            MultiByteToWideChar(CP_ACP, 0, pMsg, -1, &wMsg[0], len);
            MessageBoxW(nullptr, wMsg.c_str(), L"PostProcess.fx compile error", MB_OK | MB_ICONERROR);
            pErrBuf->Release();
        }
        else
        {
            MSG_BOX("PostProcess.fx load failed");
        }
        return E_FAIL;
    }

    if (pErrBuf) pErrBuf->Release();
    return S_OK;
}

void CScreenFX::Update(const _float& fTimeDelta)
{
    if (!m_bReady) return;

    m_fTime += fTimeDelta;

    if (m_fNoiseDuration > 0.f)
    {
        m_fNoiseDuration -= fTimeDelta;
        float t = max(0.f, m_fNoiseDuration);
        m_fNoiseAmt = m_fNoisePeak * (t / max(m_fNoisePeak * 0.5f + 0.001f, t + 0.001f));
        m_fNoiseAmt = max(0.f, m_fNoiseAmt);
    }
    else
    {
        m_fNoiseAmt = 0.f;
    }

    if (m_fDistortDuration > 0.f)
    {
        m_fDistortDuration -= fTimeDelta;
        float t = max(0.f, m_fDistortDuration);
        m_fDistortAmt = m_fDistortPeak * (t / max(m_fDistortPeak * 0.5f + 0.001f, t + 0.001f));
        m_fDistortAmt = max(0.f, m_fDistortAmt);
    }
    else
    {
        m_fDistortAmt = 0.f;
    }

    if (m_fShakeDuration > 0.f)
    {
        m_fShakeDuration -= fTimeDelta;
        float ratio = max(0.f, m_fShakeDuration) * m_fShakePeak;
        float amp = ratio * 0.012f;
        m_fShakeX = ((rand() % 1000) / 1000.f * 2.f - 1.f) * amp;
        m_fShakeY = ((rand() % 1000) / 1000.f * 2.f - 1.f) * amp;
    }
    else
    {
        m_fShakeX = 0.f;
        m_fShakeY = 0.f;
    }

    // Grain noise decay
    if (m_fGrainDuration > 0.f)
    {
        m_fGrainDuration -= fTimeDelta;
        float t = max(0.f, m_fGrainDuration);
        m_fGrainAmt = m_fGrainPeak * (t / max(m_fGrainPeak * 0.5f + 0.001f, t + 0.001f));
        m_fGrainAmt = max(0.f, m_fGrainAmt);
    }
    else
    {
        m_fGrainAmt = 0.f;
    }

    // Glass break decay
    if (m_fGlassDuration > 0.f)
    {
        m_fGlassDuration -= fTimeDelta;
        float t = max(0.f, m_fGlassDuration);
        m_fGlassAmt = m_fGlassPeak * (t / max(m_fGlassPeak * 0.5f + 0.001f, t + 0.001f));
        m_fGlassAmt = max(0.f, m_fGlassAmt);
    }
    else
    {
        m_fGlassAmt = 0.f;
    }

    // Wave decay
    if (m_fWaveDuration > 0.f)
    {
        m_fWaveDuration -= fTimeDelta;
        float t = max(0.f, m_fWaveDuration);
        m_fWaveAmt = m_fWavePeak * (t / max(m_fWavePeak * 0.5f + 0.001f, t + 0.001f));
        m_fWaveAmt = max(0.f, m_fWaveAmt);
    }
    else
    {
        m_fWaveAmt = 0.f;
    }

    // Noise phase for per-frame grain variation
    m_fNoisePhase += fTimeDelta * 60.f;

    float fBreathTarget = m_bBreathActive ? m_fBreathIntensity : 0.f;
    const float fBreathSpeed = 2.5f;
    if (m_fBreathCurrent < fBreathTarget)
        m_fBreathCurrent = min(fBreathTarget, m_fBreathCurrent + fBreathSpeed * fTimeDelta);
    else
        m_fBreathCurrent = max(fBreathTarget, m_fBreathCurrent - fBreathSpeed * fTimeDelta);

    float fPulse = sinf(m_fTime * 2.5f) * 0.15f + 0.85f;
    m_vVoidTint.x = 0.3f * m_fBreathCurrent;
    m_vVoidTint.y = 0.f;
    m_vVoidTint.z = 0.6f * m_fBreathCurrent * fPulse;
    m_vVoidTint.w = m_fBreathCurrent;
}

void CScreenFX::Begin_Capture()
{
    if (!m_bReady) return;

    m_pGraphicDev->GetRenderTarget(0, &m_pOrigRT);
    m_pGraphicDev->GetDepthStencilSurface(&m_pOrigDepth);
    m_pGraphicDev->SetRenderTarget(0, m_pRTSurface);
    m_pGraphicDev->SetDepthStencilSurface(m_pRTDepth);

    m_pGraphicDev->Clear(
        0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, D3DCOLOR_ARGB(255, 0, 0, 0), 1.f, 0);
}

void CScreenFX::End_Capture()
{
    if (!m_bReady) return;

    m_pGraphicDev->SetRenderTarget(0, m_pOrigRT);
    m_pGraphicDev->SetDepthStencilSurface(m_pOrigDepth);
    if (m_pOrigRT) { m_pOrigRT->Release(); m_pOrigRT = nullptr; }
    if (m_pOrigDepth) { m_pOrigDepth->Release(); m_pOrigDepth = nullptr; }
}

void CScreenFX::ApplyPass(const char* szTechnique, IDirect3DTexture9* pSrc, IDirect3DSurface9* pWriteSurf)
{
    // Redirect output to pWriteSurf
    IDirect3DSurface9* pPrevRT = nullptr;
    m_pGraphicDev->GetRenderTarget(0, &pPrevRT);
    m_pGraphicDev->SetRenderTarget(0, pWriteSurf);

    // Bind source scene
    m_pEffect->SetTexture("SceneMap", pSrc);
    m_pEffect->SetTexture("NoiseMap", m_pNoiseTex);
    m_pEffect->SetTechnique(szTechnique);

    UINT passes = 0;
    m_pEffect->Begin(&passes, 0);
    for (UINT p = 0; p < passes; ++p)
    {
        m_pEffect->BeginPass(p);
        m_pGraphicDev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
        m_pEffect->EndPass();
    }
    m_pEffect->End();

    // Restore original RT
    m_pGraphicDev->SetRenderTarget(0, pPrevRT);
    if (pPrevRT) pPrevRT->Release();
}

void CScreenFX::Apply_Effect()
{
    if (!m_bReady || !m_pEffect)
        return;

    Set_EffectParams();

    // Set per-technique shader params
    m_pEffect->SetFloat("gNoiseAmt", m_fGrainAmt);
    m_pEffect->SetFloat("gNoisePhase", m_fNoisePhase);
    m_pEffect->SetFloat("gGlassAmt", m_fGlassAmt);
    m_pEffect->SetFloat("gGridSize", 10.f);
    m_pEffect->SetFloat("gWaveAmpX", 0.012f * m_fWaveAmt);
    m_pEffect->SetFloat("gWaveAmpY", 0.008f * m_fWaveAmt);
    m_pEffect->SetFloat("gWaveFreqX", 12.f);
    m_pEffect->SetFloat("gWaveFreqY", 14.f);
    m_pEffect->SetFloat("gWaveSpeedX", 2.5f);
    m_pEffect->SetFloat("gWaveSpeedY", 1.8f);

    IDirect3DVertexDeclaration9* pPrevDecl = nullptr;
    IDirect3DVertexShader9* pPrevVS = nullptr;
    m_pGraphicDev->GetVertexDeclaration(&pPrevDecl);
    m_pGraphicDev->GetVertexShader(&pPrevVS);

    m_pGraphicDev->SetFVF(QUAD_FVF);
    m_pGraphicDev->SetStreamSource(0, m_pQuadVB, 0, sizeof(QUAD_VERTEX));

    bool bMultiPass = (m_fGrainAmt > 0.001f || m_fGlassAmt > 0.001f || m_fWaveAmt > 0.001f);

    if (!bMultiPass)
    {
        // Single pass: existing PostProcess technique directly to backbuffer
        m_pEffect->SetTechnique("PostProcess");
        UINT passes = 0;
        m_pEffect->Begin(&passes, 0);
        for (UINT p = 0; p < passes; ++p)
        {
            m_pEffect->BeginPass(p);
            m_pGraphicDev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
            m_pEffect->EndPass();
        }
        m_pEffect->End();
    }
    else
    {
        // Multi-pass: ping-pong between m_pRT and m_pRT_B
        // pRead = current source texture, pWriteSurf = current destination surface
        IDirect3DTexture9*  pTexA  = m_pRT;
        IDirect3DSurface9*  pSurfA = m_pRTSurface;
        IDirect3DTexture9*  pTexB  = m_pRT_B;
        IDirect3DSurface9*  pSurfB = m_pRTSurface_B;

        IDirect3DTexture9*  pRead  = pTexA;
        IDirect3DSurface9*  pWrite = pSurfB;
        bool bLastIsB = true;

        // Pass 0: Base PostProcess (shake + existing distort + existing noise + voidtint)
        ApplyPass("PostProcess", pRead, pWrite);
        // Swap: result is in pTexB, next read from B, write to A
        pRead  = pTexB;
        pWrite = pSurfA;
        bLastIsB = false;

        // Pass 1: Grain noise
        if (m_fGrainAmt > 0.001f)
        {
            ApplyPass("TechNoise", pRead, pWrite);
            // Swap
            if (bLastIsB) { pRead = pTexB; pWrite = pSurfA; bLastIsB = false; }
            else          { pRead = pTexA; pWrite = pSurfB; bLastIsB = true;  }
        }

        // Pass 2: Glass break
        if (m_fGlassAmt > 0.001f)
        {
            ApplyPass("TechGlassBreak", pRead, pWrite);
            if (bLastIsB) { pRead = pTexB; pWrite = pSurfA; bLastIsB = false; }
            else          { pRead = pTexA; pWrite = pSurfB; bLastIsB = true;  }
        }

        // Pass 3: Wave
        if (m_fWaveAmt > 0.001f)
        {
            ApplyPass("TechWave", pRead, pWrite);
            if (bLastIsB) { pRead = pTexB; pWrite = pSurfA; bLastIsB = false; }
            else          { pRead = pTexA; pWrite = pSurfB; bLastIsB = true;  }
        }

        // Final blit: pRead contains the final result, draw to backbuffer
        m_pEffect->SetTexture("SceneMap", pRead);
        m_pEffect->SetTexture("NoiseMap", m_pNoiseTex);

        // Use a simple pass-through to blit (reuse PostProcess with all effects zeroed)
        // Instead, set a minimal blit: zero out effects and just sample SceneMap
        m_pEffect->SetFloat("fNoiseAmt", 0.f);
        m_pEffect->SetFloat("fDistortAmt", 0.f);
        m_pEffect->SetFloat("fShakeX", 0.f);
        m_pEffect->SetFloat("fShakeY", 0.f);
        D3DXVECTOR4 vZero(0.f, 0.f, 0.f, 0.f);
        m_pEffect->SetVector("vVoidTint", &vZero);
        m_pEffect->SetTechnique("PostProcess");

        UINT passes = 0;
        m_pEffect->Begin(&passes, 0);
        for (UINT p = 0; p < passes; ++p)
        {
            m_pEffect->BeginPass(p);
            m_pGraphicDev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
            m_pEffect->EndPass();
        }
        m_pEffect->End();
    }

    m_pGraphicDev->SetVertexDeclaration(pPrevDecl);
    m_pGraphicDev->SetVertexShader(pPrevVS);
    if (pPrevDecl) pPrevDecl->Release();
    if (pPrevVS) pPrevVS->Release();

    m_pGraphicDev->SetTexture(0, nullptr);
    m_pGraphicDev->SetTexture(1, nullptr);
}

void CScreenFX::Set_EffectParams()
{
    m_pEffect->SetTexture("SceneMap", m_pRT);
    m_pEffect->SetTexture("NoiseMap", m_pNoiseTex);
    m_pEffect->SetFloat("fTime", m_fTime);
    m_pEffect->SetFloat("fNoiseAmt", m_fNoiseAmt);
    m_pEffect->SetFloat("fDistortAmt", m_fDistortAmt);
    m_pEffect->SetFloat("fShakeX", m_fShakeX);
    m_pEffect->SetFloat("fShakeY", m_fShakeY);
    m_pEffect->SetVector("vVoidTint", &m_vVoidTint);
    m_pEffect->SetFloat("fFlameAmt", m_fFlameAmt);
    m_pEffect->SetFloat("iScreenW", m_fScreenW);
    m_pEffect->SetFloat("iScreenH", m_fScreenH);
}

void CScreenFX::Trigger_Hit(float fIntensity)
{
    fIntensity = max(0.f, min(1.f, fIntensity));

    float fNoiseDur = 0.35f + fIntensity * 0.3f;
    if (fNoiseDur > m_fNoiseDuration || fIntensity > m_fNoisePeak)
    {
        m_fNoiseDuration = fNoiseDur;
        m_fNoisePeak = fIntensity;
    }

    float fShakeDur = 0.2f + fIntensity * 0.3f;
    if (fShakeDur > m_fShakeDuration || fIntensity > m_fShakePeak)
    {
        m_fShakeDuration = fShakeDur;
        m_fShakePeak = fIntensity;
    }
}

void CScreenFX::Trigger_TailHit(float fIntensity)
{
    fIntensity = max(0.f, min(1.f, fIntensity));

    float fDistortDur = 0.4f + fIntensity * 0.4f;
    if (fDistortDur > m_fDistortDuration || fIntensity > m_fDistortPeak)
    {
        m_fDistortDuration = fDistortDur;
        m_fDistortPeak = fIntensity;
    }

    float fNoiseDur = 0.3f + fIntensity * 0.25f;
    if (fNoiseDur > m_fNoiseDuration || fIntensity > m_fNoisePeak)
    {
        m_fNoiseDuration = fNoiseDur;
        m_fNoisePeak = fIntensity * 0.8f;
    }

    float fShakeDur = 0.35f + fIntensity * 0.4f;
    if (fShakeDur > m_fShakeDuration || fIntensity > m_fShakePeak)
    {
        m_fShakeDuration = fShakeDur;
        m_fShakePeak = fIntensity * 1.2f;
    }

    // Chain new effects for testing
    Trigger_GlassBreak(fIntensity, 0.4f);
    Trigger_Noise(fIntensity * 0.8f, 0.5f);
}

void CScreenFX::Trigger_Noise(float fIntensity, float fDuration)
{
    fIntensity = max(0.f, min(1.f, fIntensity));
    if (fDuration > m_fGrainDuration || fIntensity > m_fGrainPeak)
    {
        m_fGrainDuration = fDuration;
        m_fGrainPeak = fIntensity;
    }
}

void CScreenFX::Trigger_GlassBreak(float fIntensity, float fDuration)
{
    fIntensity = max(0.f, min(1.f, fIntensity));
    if (fDuration > m_fGlassDuration || fIntensity > m_fGlassPeak)
    {
        m_fGlassDuration = fDuration;
        m_fGlassPeak = fIntensity;
    }
}

void CScreenFX::Trigger_Wave(float fIntensity, float fDuration)
{
    fIntensity = max(0.f, min(1.f, fIntensity));
    if (fDuration > m_fWaveDuration || fIntensity > m_fWavePeak)
    {
        m_fWaveDuration = fDuration;
        m_fWavePeak = fIntensity;
    }
}

void CScreenFX::Trigger_VoidFlame(bool bActive)
{
    m_bFlameActive = bActive;
}

void CScreenFX::Apply_VoidFlame(const D3DXMATRIX& matWVP, const D3DXMATRIX& matWV)
{
    if (!m_bReady || !m_pEffect)
        return;

    // FVF / StreamSource 는 effect Begin/End 가 복원하지 않으므로 직접 저장
    DWORD dwPrevFVF = 0;
    IDirect3DVertexBuffer9* pPrevVB = nullptr;
    UINT uPrevOffset = 0, uPrevStride = 0;
    m_pGraphicDev->GetFVF(&dwPrevFVF);
    m_pGraphicDev->GetStreamSource(0, &pPrevVB, &uPrevOffset, &uPrevStride);

    DWORD prevAlphaTest = 0;
    m_pGraphicDev->GetRenderState(D3DRS_ALPHATESTENABLE, &prevAlphaTest);
    m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

    m_pEffect->SetTechnique("VoidFlame");

    // 행렬
    m_pEffect->SetMatrix("matWVP", &matWVP);
    m_pEffect->SetMatrix("matWV", &matWV);   // eyeVec Fresnel용

    // 스칼라
    m_pEffect->SetFloat("fFlameAmt", 1.f);
    m_pEffect->SetFloat("fTime", m_fTime);
    m_pEffect->SetFloat("fRefractionIndex", 0.025f);
    m_pEffect->SetFloat("iScreenW", m_fScreenW);
    m_pEffect->SetFloat("iScreenH", m_fScreenH);

    // 텍스처 (Begin 전 반드시 바인딩)
    m_pEffect->SetTexture("NoiseMap", m_pNoiseTex);   // fBm 3옥타브
    if (m_pRT)
        m_pEffect->SetTexture("SceneMap", m_pRT);     // Screen 굴절 (캡처 활성 시)

    m_pGraphicDev->SetFVF(QUAD_FVF);
    m_pGraphicDev->SetStreamSource(0, m_pQuadVB, 0, sizeof(QUAD_VERTEX));

    UINT passes = 0;
    m_pEffect->Begin(&passes, 0);
    for (UINT p = 0; p < passes; ++p)
    {
        m_pEffect->BeginPass(p);
        m_pGraphicDev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
        m_pEffect->EndPass();
    }
    m_pEffect->End();

    // 복원
    m_pGraphicDev->SetStreamSource(0, pPrevVB, uPrevOffset, uPrevStride);
    m_pGraphicDev->SetFVF(dwPrevFVF);
    if (pPrevVB) pPrevVB->Release();

    m_pEffect->SetTechnique("PostProcess");
    m_pGraphicDev->SetTexture(0, nullptr);
    m_pGraphicDev->SetTexture(1, nullptr);

    m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, prevAlphaTest);
}

void CScreenFX::SetBreathActive(bool bActive, float fIntensity)
{
    m_bBreathActive = bActive;
    m_fBreathIntensity = max(0.f, min(1.f, fIntensity));
}

void CScreenFX::Set_BreathActive(bool bActive, float fIntensity)
{
    SetBreathActive(bActive, fIntensity);
}

void CScreenFX::Free()
{
    if (m_pEffect) { m_pEffect->Release(); m_pEffect = nullptr; }
    if (m_pNoiseTex) { m_pNoiseTex->Release(); m_pNoiseTex = nullptr; }
    if (m_pQuadVB) { m_pQuadVB->Release(); m_pQuadVB = nullptr; }
    if (m_pRTSurface_B) { m_pRTSurface_B->Release(); m_pRTSurface_B = nullptr; }
    if (m_pRT_B) { m_pRT_B->Release(); m_pRT_B = nullptr; }
    if (m_pRTSurface) { m_pRTSurface->Release(); m_pRTSurface = nullptr; }
    if (m_pRTDepth) { m_pRTDepth->Release(); m_pRTDepth = nullptr; }
    if (m_pRT) { m_pRT->Release(); m_pRT = nullptr; }
    if (m_pOrigRT) { m_pOrigRT->Release(); m_pOrigRT = nullptr; }
    if (m_pOrigDepth) { m_pOrigDepth->Release(); m_pOrigDepth = nullptr; }
    if (m_pGraphicDev) { m_pGraphicDev->Release(); m_pGraphicDev = nullptr; }
}
