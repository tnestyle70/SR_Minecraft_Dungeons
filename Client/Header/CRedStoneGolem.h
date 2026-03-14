#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"
#include "CRedStoneGolemPart.h"

class CRedStoneGolem : public CGameObject
{
private:
	explicit CRedStoneGolem(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CRedStoneGolem(const CRedStoneGolem& rhs);
	virtual ~CRedStoneGolem();

public:
	virtual			HRESULT		Ready_GameObject();
	virtual			_int		Update_GameObject(const _float& fTimeDelta);
	virtual			void		LateUpdate_GameObject(const _float& fTimeDelta);
	virtual			void		Render_GameObject();

private:
	HRESULT			Add_Component();
	void Set_PartsPos();
	void Set_DefaultScale();
	
private:
	CRedStoneGolemPart* m_pParts[GOLEM_END];

	Engine::CTexture* m_pTextureCom;

public:
	static CRedStoneGolem* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
	virtual void Free();
};

