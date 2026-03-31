#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"
#include "CJSTile.h"

namespace Engine
{
	class CLayer;
}

enum CHUNKTYPE
{
	CHUNK_FULL,
	CHUNK_LEFT,
	CHUNK_RIGHT,

	CHUNK_END
};

class CJSChunk : public CGameObject
{
private:
	explicit CJSChunk(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual ~CJSChunk();

public:
	HRESULT Ready_GameObject(_vec3 vPos, CLayer* pLayer, CHUNKTYPE eType);
	virtual _int Update_GameObject(const _float& fTimeDelta);
	virtual void LateUpdate_GameObject(const _float& fTimeDelta);
	virtual void Render_GameObject();

public:
	HRESULT Ready_Tile(_vec3 vChunkPos);

public:
	void Get_Position(_vec3& vPos) { m_pTransformCom->Get_Info(INFO_POS, &vPos); }

private:
	HRESULT Add_Component();

public:
	static CJSChunk* Create(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vPos, CLayer* pLayer, CHUNKTYPE eType);

private:
	CLayer* m_pLayer;

	CTransform* m_pTransformCom;

	static const _int   TILE_X = 3;
	static const _int   TILE_Z = 16;
	static const _float TILE_SIZE;

	vector<CJSTile*> m_vecTile;
	CHUNKTYPE m_eChunkType;

private:
	virtual void Free();
};