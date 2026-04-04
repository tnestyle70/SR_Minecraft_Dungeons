#pragma once
#include "CGameObject.h"

class CJSWaterPlane : public CGameObject
{
private:
    explicit CJSWaterPlane(LPDIRECT3DDEVICE9 pGraphicDev);
    virtual ~CJSWaterPlane();

public:
    HRESULT Ready_GameObject();
    virtual _int    Update_GameObject(const _float& fTimeDelta) override;
    virtual void    LateUpdate_GameObject(const _float& fTimeDelta) override;
    virtual void    Render_GameObject() override;

private:
    HRESULT Create_Buffer();
    void    Update_Wave(const _float& fTimeDelta);

private:
    static const _int   GRID_X = 40;
    static const _int   GRID_Z = 40;
    static const _float GRID_SIZE;  // 격자 하나 크기
    static const _float PLANE_Y;    // 물 평면 Y 위치

    LPDIRECT3DVERTEXBUFFER9 m_pVB = nullptr;
    LPDIRECT3DINDEXBUFFER9  m_pIB = nullptr;

    _float  m_fWaveTime = 0.f;
    _float  m_fWaveSpeed = 0.1f;      // 파도 속도
    _float  m_fWaveHeight = 0.2f;     // 파도 높이
    _float  m_fWaveFreq = 0.1f;     // 파도 주파수

    _vec3   m_vPlayerPos = {};

public:
    void Set_PlayerPos(_vec3 vPos) { m_vPlayerPos = vPos; }

public:
    static CJSWaterPlane* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
    virtual void Free();
};

