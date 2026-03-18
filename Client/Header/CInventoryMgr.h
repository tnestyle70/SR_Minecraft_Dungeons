#pragma once
#include "CBase.h"
#include "CProtoMgr.h"

class CInventoryMgr : public CBase
{
	DECLARE_SINGLETON(CInventoryMgr)
private:
	explicit CInventoryMgr();
	virtual ~CInventoryMgr();
public:
	HRESULT Ready_InventoryMgr(LPDIRECT3DDEVICE9 pGraphicDev);
private:
	LPDIRECT3DDEVICE9 m_pGraphicDev;
private:
	virtual void Free();
};