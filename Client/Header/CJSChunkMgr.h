#pragma once
#include "CBase.h"
#include "Engine_Define.h"

class CJSChunk;

namespace Engine
{
	class CLayer;
}

class CJSChunkMgr : public CBase
{
	DECLARE_SINGLETON(CJSChunkMgr)

public:
	explicit CJSChunkMgr();
	virtual ~CJSChunkMgr();

public:
	HRESULT Ready_Manager(LPDIRECT3DDEVICE9 pGraphicDev, CLayer* pLayer);
	void Update_Manager(const _float& fTimeDelta, _vec3 vPlayerPos);

public:
	TILEID Get_TileID(_vec3 vPlayerPos);

private:
	void Spawn_Chunk(_vec3 vPos);
	void Remove_OldChunk(_vec3 vPlayerPos);

private:
	LPDIRECT3DDEVICE9 m_pGraphicDev = nullptr;
	CLayer* m_pLayer = nullptr;
	list<CJSChunk*> m_ChunkList;
	list<CJSChunk*> m_RemoveList;
	_float m_fChunkSize = 16.f;
	_int m_iRenderCount = 5;

private:
	virtual void Free();
};

