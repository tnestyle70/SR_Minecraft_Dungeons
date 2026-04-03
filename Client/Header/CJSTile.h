#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"

class CJSTile : public CGameObject
{
private:
	explicit CJSTile(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual ~CJSTile();

public:
    HRESULT Ready_GameObject(_vec3 vPos, TILEID eTileID);
    virtual _int Update_GameObject(const _float& fTimeDelta);
    virtual void LateUpdate_GameObject(const _float& fTimeDelta);
    virtual void Render_GameObject();

public:
    virtual bool Is_Dead() override { return m_bDead; }
    void Set_Dead() { m_bDead = true; }
    void Get_Position(_vec3& vPos) { m_pTransformCom->Get_Info(INFO_POS, &vPos); }

private:
    HRESULT Add_Component();
    HRESULT Set_Material();

private:
    CJSCubeTex* m_pBufferCom;
    CTransform* m_pTransformCom;
    CTexture* m_pTextureCom;
    TILEID m_eTileID;
    _bool m_bDead = false;

public:
    TILEID  Get_TileID() { return m_eTileID; }

public:
    static CJSTile* Create(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vPos, TILEID eTileID);

private:
    virtual void Free();
};

