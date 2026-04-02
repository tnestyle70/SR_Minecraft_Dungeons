#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"
#include "CJSTile.h"
#include "CJSEmerald.h"

namespace Engine
{
	class CLayer;
}

enum CHUNKTYPE
{
	CHUNK_FULL,
	CHUNK_LEFT,
	CHUNK_RIGHT,
	CHUNK_GAP,

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
	HRESULT Ready_Emerald(_vec3 vChunkPos);
	HRESULT Ready_Emerald_Line(_vec3 vChunkPos, _int iX);
	HRESULT Ready_Emerald_Parabola(_vec3 vChunkPos);

public:
	void Get_Position(_vec3& vPos) { m_pTransformCom->Get_Info(INFO_POS, &vPos); }
	virtual bool Is_Dead() override { return m_bDead; }
	void Set_Dead() { m_bDead = true; }
	TILEID Get_TileID(_vec3 vPlayerPos);
	void Check_Collect(_vec3 vPlayerPos);

private:
	HRESULT Add_Component();

public:
	static CJSChunk* Create(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vPos, CLayer* pLayer, CHUNKTYPE eType);

private:
	CLayer* m_pLayer;

	CTransform* m_pTransformCom;

	static const _int   TILE_X = 5;
	static const _int   TILE_Z = 32;
	static const _float TILE_SIZE;

	vector<CJSTile*> m_vecTile;
	vector<CJSTile*> m_vecWall;
	vector<CJSEmerald*> m_vecEmerald;

	CHUNKTYPE m_eChunkType;

	_bool m_bDead = false;

private:
	virtual void Free();
};