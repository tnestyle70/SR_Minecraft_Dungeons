#pragma once
#include "Engine_Define.h"
#include "CCubeTex.h"

class CSpawnEffect
{
public:
    explicit    CSpawnEffect(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vPos);
    ~CSpawnEffect();

public:
    void        Update(const _float& fTimeDelta);
    void        Render();
    bool        Is_Done() const { return m_bDone; }

private:
    LPDIRECT3DDEVICE9       m_pGraphicDev = nullptr;
    Engine::CCubeTex* m_pSpawnCube = nullptr;
    _vec3                   m_vPos = {};
    _float                  m_fTimer = 0.f;
    bool                    m_bDone = false;

    static constexpr int    m_iLightIndex = 6;
    static constexpr _float m_fExpandEnd = 0.15f;
    static constexpr _float m_fGlowEnd = 0.50f;
    static constexpr _float m_fDecayEnd = 1.20f;
    static constexpr _float m_fCubeEnd = 0.60f;
    static constexpr _float m_fMaxRange = 35.f;
    static constexpr _float m_fMaxScale = 6.f;
    static constexpr _float m_fMaxAlpha = 0.70f;
};
