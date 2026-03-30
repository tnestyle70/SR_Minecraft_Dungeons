#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"

class CTGSkyBox : public CGameObject
{
private:
	explicit CTGSkyBox(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CTGSkyBox(const CGameObject& rhs);
	virtual ~CTGSkyBox();

public:
	virtual			HRESULT		Ready_GameObject();
	virtual			_int		Update_GameObject(const _float& fTimeDelta);
	virtual			void		LateUpdate_GameObject(const _float& fTimeDelta);
	virtual			void		Render_GameObject();

private:
	HRESULT			Add_Component();

private:
	Engine::CCubeTex* m_pBufferCom;
	Engine::CTransform* m_pTransformCom;
	Engine::CTexture* m_pTextureCom;

public:
	static CTGSkyBox* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
	virtual void Free();

};