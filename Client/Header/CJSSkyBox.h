#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"

class CJSSkyBox : public CGameObject
{
private:
	explicit CJSSkyBox(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CJSSkyBox(const CGameObject& rhs);
	virtual ~CJSSkyBox();

public:
	virtual			HRESULT		Ready_GameObject();
	virtual			_int		Update_GameObject(const _float& fTimeDelta);
	virtual			void		LateUpdate_GameObject(const _float& fTimeDelta);
	virtual			void		Render_GameObject();

private:
	HRESULT	Add_Component();

private:
	Engine::CCubeTex* m_pBufferCom = nullptr;
	Engine::CTransform* m_pTransformCom = nullptr;
	Engine::CTexture* m_pTextureCom = nullptr;

public:
	static CJSSkyBox* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
	virtual void Free();
};

