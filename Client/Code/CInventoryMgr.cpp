#include "pch.h"
#include "CInventoryMgr.h"

IMPLEMENT_SINGLETON(CInventoryMgr)

CInventoryMgr::CInventoryMgr()
{
}

CInventoryMgr::~CInventoryMgr()
{
	Free();
}

HRESULT CInventoryMgr::Ready_InventoryMgr(LPDIRECT3DDEVICE9 pGraphicDev)
{
	m_pGraphicDev = pGraphicDev;

	return S_OK;
}

void CInventoryMgr::Free()
{
}
