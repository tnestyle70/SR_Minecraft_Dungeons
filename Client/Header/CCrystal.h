#pragma once
#include "CGameObject.h"
#include "CCrystalPart.h"

struct CRYSTAL_PART_DESC
{
	_float fScaleRatio;   // scale / 20.f
	_vec3  vNormOffset;   // 스케일 1.0 기준 오프셋
};

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
	void Set_PartsParent();
	void Set_CrystalScale(_float fScale);

private:
	static const CRYSTAL_PART_DESC s_Desc[CRYSTAL_END];

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