#pragma once
#include "CGameObject.h"

class CPlayer;

class CMinimap : public CGameObject
{
private:
	explicit CMinimap(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CMinimap(const CGameObject& rhs);
	virtual ~CMinimap();
public:
	virtual HRESULT Ready_GameObject();
	virtual _int Update_GameObject(const _float& fTimeDelta);
	virtual void LateUpdate_GameObject(const _float& fTimeDelta);
	virtual void Render_GameObject();
private:
	HRESULT Add_Component();
private:
	virtual void Free();
};