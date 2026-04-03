#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"

namespace Engine
{
    class CLayer;
}

class CJSBaseChunk : public CGameObject
{
protected:
    explicit CJSBaseChunk(LPDIRECT3DDEVICE9 pGraphicDev);
    virtual ~CJSBaseChunk();

public:
    virtual void    Get_Position(_vec3& vPos) { m_pTransformCom->Get_Info(INFO_POS, &vPos); }
    virtual _vec3   Get_EndPos() PURE;
    virtual bool    Is_Dead() override { return m_bDead; }
    void            Set_Dead() { m_bDead = true; }
    DIRECTION       Get_Dir() { return m_eDir; }

protected:
    CTransform* m_pTransformCom = nullptr;
    CLayer* m_pLayer = nullptr;
    DIRECTION       m_eDir = DIR_FORWARD;
    _bool           m_bDead = false;

protected:
    virtual void Free();
};

