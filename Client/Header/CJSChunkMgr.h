#pragma once
#include "CBase.h"
#include "Engine_Define.h"

class CJSChunk;
class CJSCornerChunk;
class CJSBaseChunk;

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
	void Check_Collect(_vec3 vPlayerPos);

private:
	void Spawn_Chunk(_vec3 vPos);
	void Remove_OldChunk(_vec3 vPlayerPos);
	DIRECTION Turn_Left(DIRECTION eDir);
	DIRECTION Turn_Right(DIRECTION eDir);

private:
	LPDIRECT3DDEVICE9 m_pGraphicDev = nullptr;
	CLayer* m_pLayer = nullptr;

	list<CJSBaseChunk*> m_ChunkList;
	list<CJSBaseChunk*> m_RemoveList;
	list<CJSBaseChunk*> m_CornerList;

	_float m_fChunkSize = 64.f;
	_int m_iRenderCount = 3;

	_int        m_iStraightCount = 0;       // 현재 직선 청크 수
	_int        m_iStraightMax = 3;         // 코너 전 직선 최대 개수
	DIRECTION   m_eCurrentDir = DIR_FORWARD; // 현재 진행 방향

private:
	virtual void Free();
};

