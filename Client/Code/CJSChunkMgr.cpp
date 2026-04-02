#include "pch.h"
#include "CJSChunkMgr.h"
#include "CLayer.h"
#include "CJSChunk.h"

IMPLEMENT_SINGLETON(CJSChunkMgr)

CJSChunkMgr::CJSChunkMgr()
{
}

CJSChunkMgr::~CJSChunkMgr()
{
}

HRESULT CJSChunkMgr::Ready_Manager(LPDIRECT3DDEVICE9 pGraphicDev, CLayer* pLayer)
{
    m_pGraphicDev = pGraphicDev;
    m_pLayer = pLayer;

    for (_int i = 0; i < m_iRenderCount; ++i)
    {
        _vec3 vPos = { 0.f, 0.f, m_fChunkSize * i };

        // 첫 번째 청크는 무조건 FULL
        if (i == 0)
        {
            CJSChunk* pChunk = CJSChunk::Create(m_pGraphicDev, vPos, m_pLayer, CHUNK_FULL);
            if (pChunk)
            {
                m_pLayer->Add_GameObject(L"Chunk", pChunk);
                m_ChunkList.push_back(pChunk);
            }
        }
        else
            Spawn_Chunk(vPos);
    }
    return S_OK;
}

void CJSChunkMgr::Update_Manager(const _float& fTimeDelta, _vec3 vPlayerPos)
{
    for (auto& pChunk : m_RemoveList)
        Safe_Release(pChunk);

    m_RemoveList.clear();

    Remove_OldChunk(vPlayerPos);

    if (!m_ChunkList.empty())
    {
        _vec3 vFrontPos;
        m_ChunkList.back()->Get_Position(vFrontPos);

        if (vFrontPos.z - vPlayerPos.z < m_fChunkSize * (m_iRenderCount / 2))
            Spawn_Chunk({ 0.f, 0.f, vFrontPos.z + m_fChunkSize });
    }
}

TILEID CJSChunkMgr::Get_TileID(_vec3 vPlayerPos)
{
    for (auto& pChunk : m_ChunkList)
    {
        _vec3 vChunkPos;
        pChunk->Get_Position(vChunkPos);

        TCHAR szBuf[128];
        wsprintf(szBuf, L"ChunkZ: %d ~ %d, PlayerZ: %d",
            (_int)vChunkPos.z, (_int)(vChunkPos.z + m_fChunkSize), (_int)vPlayerPos.z);
        OutputDebugString(szBuf);

        if (vPlayerPos.z >= vChunkPos.z &&
            vPlayerPos.z < vChunkPos.z + m_fChunkSize)
        {
            return pChunk->Get_TileID(vPlayerPos);
        }
    }
    return TILE_EMPTY;
}

void CJSChunkMgr::Check_Collect(_vec3 vPlayerPos)
{
    for (auto& pChunk : m_ChunkList)
    {
        pChunk->Check_Collect(vPlayerPos);
    }
}

void CJSChunkMgr::Spawn_Chunk(_vec3 vPos)
{
    _int iRand = rand() % 10;

    CHUNKTYPE eType = CHUNK_FULL;

    if (iRand == 7)
        eType = CHUNK_GAP;
    else if (iRand == 8)
        eType = CHUNK_LEFT;
    else if (iRand == 9)
        eType = CHUNK_RIGHT;

    CJSChunk* pChunk = CJSChunk::Create(m_pGraphicDev, vPos, m_pLayer, eType);
    if (pChunk == nullptr)
        return;

    m_pLayer->Add_GameObject(L"Chunk", pChunk);
    m_ChunkList.push_back(pChunk);
}

void CJSChunkMgr::Remove_OldChunk(_vec3 vPlayerPos)
{
    if (m_ChunkList.empty())
        return;

    _vec3 vBackPos;
    m_ChunkList.front()->Get_Position(vBackPos);

    if (vPlayerPos.z - vBackPos.z > m_fChunkSize + 10.f)
    {
        m_ChunkList.front()->Set_Dead();
        m_RemoveList.push_back(m_ChunkList.front());
        m_ChunkList.pop_front();
    }
}

void CJSChunkMgr::Free()
{
	for (auto& pChunk : m_ChunkList)
		Safe_Release(pChunk);

	m_ChunkList.clear();
}
