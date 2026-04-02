#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"

class CJSEmerald : public CGameObject
{
private:
	explicit CJSEmerald(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual ~CJSEmerald();

public:
	virtual HRESULT Ready_GameObject(_vec3 vPos);
	virtual _int Update_GameObject(const _float& fTimeDelta);
	virtual void LateUpdate_GameObject(const _float& fTimeDelta);
	virtual void Render_GameObject();

public:
	virtual bool Is_Dead() override { return m_bDead; }
	void Set_Dead() { m_bDead = true; }
	_bool Check_Collect(_vec3 vPlayerPos);

private:
	HRESULT Add_Component();

public:
	static CJSEmerald* Create(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vPos);

private:
	CRcTex* m_pBufferCom;
	CTransform* m_pTransformCom;
	CTexture* m_pTextureCom;

	_bool m_bDead = false;

private:
	virtual void Free();
};

