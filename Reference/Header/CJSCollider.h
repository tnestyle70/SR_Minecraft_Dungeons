#pragma once
#include "CComponent.h"

BEGIN(Engine)

class ENGINE_DLL CJSCollider : public CComponent
{
private:
    explicit CJSCollider(LPDIRECT3DDEVICE9 pGraphicDev);
    explicit CJSCollider(const CJSCollider& rhs);
    virtual ~CJSCollider();

public:
    HRESULT Ready_Collider(_vec3 vCenter, _vec3 vSize);
    void    Update_Collider(_matrix* pWorldMatrix);
    _bool   Check_Collision(CJSCollider* pOther);
    void    Render_Collider();

public:
    _vec3   Get_Min() { return m_vMin; }
    _vec3   Get_Max() { return m_vMax; }

private:
    _vec3   m_vLocalMin;   // 로컬 공간 min
    _vec3   m_vLocalMax;   // 로컬 공간 max
    _vec3   m_vMin;        // 월드 공간 min
    _vec3   m_vMax;        // 월드 공간 max

public:
    static CJSCollider* Create(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vCenter, _vec3 vSize);
    virtual CComponent* Clone();

private:
    virtual void Free();
};

END