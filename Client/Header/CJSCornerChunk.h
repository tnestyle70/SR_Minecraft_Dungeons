#pragma once
#include "CJSBaseChunk.h"'
#include "CProtoMgr.h"
#include "CJSTile.h"
#include "CJSChunk.h"

namespace Engine
{
    class CLayer;
}

class CJSCornerChunk : public CJSBaseChunk
{
private:
    explicit CJSCornerChunk(LPDIRECT3DDEVICE9 pGraphicDev);
    virtual ~CJSCornerChunk();

public:
    HRESULT Ready_GameObject(_vec3 vPos, CLayer* pLayer, CHUNKTYPE eType, DIRECTION eDir);
    virtual _int Update_GameObject(const _float& fTimeDelta);
    virtual void LateUpdate_GameObject(const _float& fTimeDelta);
    virtual void Render_GameObject();

public:
    CHUNKTYPE Get_ChunkType() { return m_eChunkType; }
    void Get_Position(_vec3& vPos) { m_pTransformCom->Get_Info(INFO_POS, &vPos); }
    _vec3 Get_EndPos();
    virtual bool Is_Dead() override { return m_bDead; }
    void Set_Dead() { m_bDead = true; }

private:
    HRESULT Ready_Tile(_vec3 vChunkPos);
    HRESULT Add_Component();

private:
    static const _int   CORNER_SIZE = 5;        // 5x5
    static const _float TILE_SIZE;              // 2.f

private:
    CJSCubeTex* m_pBufferCom = nullptr;
    CTransform* m_pTransformCom = nullptr;

    CHUNKTYPE m_eChunkType = CHUNK_CORNER_RIGHT;

    vector<CJSTile*> m_vecTile;
    vector<CJSTile*> m_vecWall;

public:
    static CJSCornerChunk* Create(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vPos, CLayer* pLayer, CHUNKTYPE eType, DIRECTION eDir);

private:
    virtual void Free();
};

