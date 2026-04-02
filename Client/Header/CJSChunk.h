#pragma once
#include "CJSBaseChunk.h"
#include "CProtoMgr.h"
#include "CJSTile.h"
#include "CJSEmerald.h"
#include "Engine_Enum.h"

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
	CHUNK_CORNER_LEFT,
	CHUNK_CORNER_RIGHT,

	CHUNK_END
};

class CJSChunk : public CJSBaseChunk
{
private:
	explicit CJSChunk(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual ~CJSChunk();

public:
	HRESULT Ready_GameObject(_vec3 vPos, CLayer* pLayer, CHUNKTYPE eType, DIRECTION eDir);
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
	_vec3 Get_EndPos();

private:
	HRESULT Add_Component();
	_vec3 Calc_TilePos(_vec3 vChunkPos, _int x, _int z);

public:
	static CJSChunk* Create(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vPos, CLayer* pLayer, CHUNKTYPE eType, DIRECTION eDir);

private:
	//CLayer* m_pLayer;

	CTransform* m_pTransformCom;

	static const _int   TILE_X = 5;
	static const _int   TILE_Z = 32;
	static const _float TILE_SIZE;

	vector<CJSTile*> m_vecTile;
	vector<CJSTile*> m_vecWall;
	vector<CJSEmerald*> m_vecEmerald;

	CHUNKTYPE m_eChunkType;
	//DIRECTION m_eDir = DIR_FORWARD;

	//_bool m_bDead = false;

private:
	virtual void Free();
};