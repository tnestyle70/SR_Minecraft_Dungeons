#pragma once

#include "CBase.h"
#include "Engine_Define.h"

class CScreenFX : public CBase
{
    DECLARE_SINGLETON(CScreenFX)

private:
    explicit CScreenFX();
    virtual ~CScreenFX();

public:
    HRESULT Ready(LPDIRECT3DDEVICE9 pDev, _uint iWidth, _uint iHeight);
    void Update(const _float& fTimeDelta);

    void Begin_Capture();
    void End_Capture();
    void Apply_Effect();

    void SetBreathActive(bool bActive, float fIntensity = 1.f);
    void Set_BreathActive(bool bActive, float fIntensity = 1.f);
    void Trigger_Hit(float fIntensity = 1.f);
    void Trigger_TailHit(float fIntensity = 1.f);

private:
    HRESULT Create_RenderTarget(_uint iW, _uint iH);
    HRESULT Create_DepthBuffer(_uint iW, _uint iH);
    HRESULT Create_QuadVB();
    HRESULT Create_NoiseTex();
    HRESULT Load_Effect();
    void Set_EffectParams();

private:
    LPDIRECT3DDEVICE9 m_pGraphicDev = nullptr;

    IDirect3DTexture9* m_pRT = nullptr;
    IDirect3DSurface9* m_pRTSurface = nullptr;
    IDirect3DSurface9* m_pRTDepth = nullptr;

    IDirect3DSurface9* m_pOrigRT = nullptr;
    IDirect3DSurface9* m_pOrigDepth = nullptr;

    ID3DXEffect* m_pEffect = nullptr;
    IDirect3DTexture9* m_pNoiseTex = nullptr;
    IDirect3DVertexBuffer9* m_pQuadVB = nullptr;

    float m_fTime = 0.f;
    float m_fNoiseAmt = 0.f;
    float m_fDistortAmt = 0.f;
    float m_fShakeX = 0.f;
    float m_fShakeY = 0.f;
    D3DXVECTOR4 m_vVoidTint = { 0.f, 0.f, 0.f, 0.f };

    float m_fNoiseDuration = 0.f;
    float m_fNoisePeak = 0.f;
    float m_fDistortDuration = 0.f;
    float m_fDistortPeak = 0.f;
    float m_fShakeDuration = 0.f;
    float m_fShakePeak = 0.f;

    bool m_bBreathActive = false;
    float m_fBreathIntensity = 0.f;
    float m_fBreathCurrent = 0.f;

    _uint m_iWidth = 0;
    _uint m_iHeight = 0;
    bool m_bReady = false;

public:
    virtual void Free() override;
};
