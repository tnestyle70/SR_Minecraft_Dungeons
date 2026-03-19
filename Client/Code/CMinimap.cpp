#include "pch.h"
#include "CMonsterMgr.h"
#include "CPlayer.h"
#include "CMinimap.h"

CMinimap::CMinimap(LPDIRECT3DDEVICE9 pGraphicDev)
	:CGameObject(pGraphicDev)
{}

CMinimap::CMinimap(const CGameObject& rhs)
	:CGameObject(rhs)
{}

CMinimap::~CMinimap()
{}

HRESULT CMinimap::Ready_GameObject()
{
	return S_OK;
}

_int CMinimap::Update_GameObject(const _float& fTimeDelta)
{
	return _int();
}

void CMinimap::LateUpdate_GameObject(const _float& fTimeDelta)
{}

void CMinimap::Render_GameObject()
{}

HRESULT CMinimap::Add_Component()
{
	return S_OK;
}

void CMinimap::Free()
{}
