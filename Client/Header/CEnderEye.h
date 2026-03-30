#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"

class CEnderEye : public CGameObject
{
private:
	explicit CEnderEye(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual ~CEnderEye();

public:
	virtual HRESULT Ready_GameObject();
	virtual _int Update_GameObject(const _float& fTimeDelta);
	virtual void LateUpdate_GameObject(const _float& fTimeDelta);
	virtual void Render_GameObject();

private:
	HRESULT Add_Component();

public:
	static CEnderEye* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
	CRcTex* m_pBufferCom;
	CTransform* m_pTransformCom;
	CTexture* m_pTextureCom;
	CCollider* m_pColliderCom;

private:
	virtual void Free();
};

