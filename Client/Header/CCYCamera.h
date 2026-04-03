#pragma once
#include "CCamera.h"
#include "CDInputMgr.h"
#include "CTransform.h"

class CCYPlayer;

class CCYCamera : public Engine::CCamera
{
public:
    explicit CCYCamera(LPDIRECT3DDEVICE9 pGraphicDev);
    virtual ~CCYCamera();

public:
    HRESULT Ready_GameObject(const _vec3* pEye, const _vec3* pAt, const _vec3* pUp,
                              float fFov = D3DXToRadian(60.f),
                              float fAspect = (_float)WINCX / WINCY,
                              float fNear = 0.1f,
                              float fFar = 1000.f);

    virtual _int Update_GameObject(const _float& fTimeDelta) override;
    virtual void Render_GameObject() override {}
    virtual void LateUpdate_GameObject(const _float& fTimeDelta) override;

    _matrix Get_ViewMatrix() { return m_matView; }
    _vec3   Get_Eye() { return m_vEye; }

    void Set_Player(CCYPlayer* pPlayer) { m_pCYPlayer = pPlayer; }

private:
    void FPS_MouseRotate();
    void Free_Move(const _float& fTimeDelta);

private:
    CCYPlayer* m_pCYPlayer = nullptr;
    float       m_fPitch = 0.f;
    float       m_fFreeSpeed = 50.f;
    bool        m_bF2Check = false;
    bool        m_bFreeMode = false;
    bool        m_bMouseFix = true;
    bool        m_bTabCheck = false;

public:
    static CCYCamera* Create(LPDIRECT3DDEVICE9 pGraphicDev,
        const _vec3* pEye, const _vec3* pAt, const _vec3* pUp);

protected:
    virtual void Free();
};