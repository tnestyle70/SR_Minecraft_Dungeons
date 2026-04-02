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

    void Trigger_Noise(float fIntensity, float fDuration);
    void Trigger_GlassBreak(float fIntensity, float fDuration);
    void Trigger_Wave(float fIntensity, float fDuration);

    void Trigger_VoidFlame(bool bActive);
    void Apply_VoidFlame(const D3DXMATRIX& matWVP, const D3DXMATRIX& matWV);

    void Set_DragonBreath(bool bActive, const D3DXVECTOR2& vOriginUV,
        const D3DXVECTOR2& vDirUV, float fRadiusUV, float fLengthUV);

    ID3DXEffect* Get_Effect() const { return m_pEffect; }
    IDirect3DTexture9* Get_NoiseTex() const { return m_pNoiseTex; }

private:
    HRESULT Create_RenderTarget(_uint iW, _uint iH);
    HRESULT Create_DepthBuffer(_uint iW, _uint iH);
    HRESULT Create_QuadVB();
    HRESULT Create_NoiseTex();
    HRESULT Create_PingPongRT(_uint iW, _uint iH);
    HRESULT Load_Effect();

    void Set_EffectParams();
    void ApplyPass(const char* szTechnique, IDirect3DTexture9* pSrc, IDirect3DSurface9* pWriteSurf);

private:
    LPDIRECT3DDEVICE9 m_pGraphicDev = nullptr;

    IDirect3DTexture9* m_pRT = nullptr;
    IDirect3DSurface9* m_pRTSurface = nullptr;
    IDirect3DSurface9* m_pRTDepth = nullptr;

    IDirect3DTexture9* m_pRT_B = nullptr;
    IDirect3DSurface9* m_pRTSurface_B = nullptr;

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

    float m_fGrainAmt = 0.f;
    float m_fGrainDuration = 0.f;
    float m_fGrainPeak = 0.f;
    float m_fNoisePhase = 0.f;

    float m_fGlassAmt = 0.f;
    float m_fGlassDuration = 0.f;
    float m_fGlassPeak = 0.f;

    float m_fWaveAmt = 0.f;
    float m_fWaveDuration = 0.f;
    float m_fWavePeak = 0.f;

    bool  m_bBreathActive = false;

    bool  m_bFlameActive = false;
    float m_fFlameAmt = 0.f;

    float m_fBreathIntensity = 0.f;
    float m_fBreathCurrent = 0.f;

    _uint m_iWidth = 0;
    _uint m_iHeight = 0;

    float m_fScreenW = 0.f;
    float m_fScreenH = 0.f;

    bool m_bReady = false;

    // Dragon breath cone
    bool    m_bDragonBreathActive = false;
    float   m_fDBreathFadeIn     = 0.f;
    D3DXVECTOR2 m_vBreathOriginUV = { 0.f, 0.f };
    D3DXVECTOR2 m_vBreathDirUV    = { 0.f, 1.f };
    float   m_fBreathRadiusUV     = 0.05f;
    float   m_fBreathMaxLenUV     = 0.5f;

public:
    virtual void Free() override;
};
