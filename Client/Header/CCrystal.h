#pragma once
#include "CGameObject.h"
#include "CCrystalPart.h"

class CCrystal : public CGameObject
{
private:
	explicit CCrystal(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual ~CCrystal();

public:
	virtual HRESULT Ready_GameObject();
	virtual _int Update_GameObject(const _float& fTimeDelta);
	virtual void LateUpdate_GameObject(const _float& fTimeDelta);
	virtual void Render_GameObject();

private:
	HRESULT Add_Component();

	void Set_PartsOffset();
	void Set_WorldScale();
	void Set_PartsParent();

private:
	CCrystalPart* m_pParts[CRYSTAL_END];

	CTexture* m_pTextureCom;
	CTransform* m_pTransformCom;
	CCollider* m_pColliderCom;

public:
	static CCrystal* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
	virtual void Free();
};