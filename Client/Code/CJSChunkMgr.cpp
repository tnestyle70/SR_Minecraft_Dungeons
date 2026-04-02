#include "pch.h"
#include "CJSChunkMgr.h"
#include "CLayer.h"
#include "CJSChunk.h"
#include "CJSCornerChunk.h"
#include "CJSBaseChunk.h"

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
            CJSChunk* pChunk = CJSChunk::Create(m_pGraphicDev, vPos, m_pLayer, CHUNK_FULL, m_eCurrentDir);
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
        _vec3 vEndPos = m_ChunkList.back()->Get_EndPos();

        _float fDist = 0.f;
        switch (m_eCurrentDir)
        {
        case DIR_FORWARD:
            fDist = vEndPos.z - vPlayerPos.z;
            break;
        case DIR_RIGHT:
            fDist = vEndPos.x - vPlayerPos.x;
            break;
        case DIR_LEFT:
            fDist = vPlayerPos.x - vEndPos.x;
            break;
        }

        if (fDist < m_fChunkSize * (m_iRenderCount / 2))
            Spawn_Chunk(vEndPos);
    }
}

TILEID CJSChunkMgr::Get_TileID(_vec3 vPlayerPos)
{
    for (auto& pChunk : m_ChunkList)
    {
        CJSChunk* pJSChunk = dynamic_cast<CJSChunk*>(pChunk);
        if (pJSChunk == nullptr)
            continue;  // 코너 청크면 스킵

        _vec3 vChunkPos;
        pChunk->Get_Position(vChunkPos);

        if (vPlayerPos.z >= vChunkPos.z &&
            vPlayerPos.z < vChunkPos.z + m_fChunkSize)
        {
            return pJSChunk->Get_TileID(vPlayerPos);
        }
    }
    return TILE_EMPTY;
}

void CJSChunkMgr::Check_Collect(_vec3 vPlayerPos)
{
    for (auto& pChunk : m_ChunkList)
    {
        CJSChunk* pJSChunk = dynamic_cast<CJSChunk*>(pChunk);
        if (pJSChunk == nullptr)
            continue;  // 코너 청크면 스킵

        pJSChunk->Check_Collect(vPlayerPos);
    }
}

void CJSChunkMgr::Spawn_Chunk(_vec3 vPos)
{
    TCHAR szBuf[128];
    wsprintf(szBuf, L"Spawn Pos X: %d, Z: %d, Dir: %d", (_int)vPos.x, (_int)vPos.z, m_eCurrentDir);
    OutputDebugString(szBuf);

    CHUNKTYPE eType = CHUNK_FULL;
    CJSBaseChunk* pChunk = nullptr;

    if (m_iStraightCount >= m_iStraightMax)
    {
        if (rand() % 2 == 0)
            eType = CHUNK_CORNER_LEFT;
        else
            eType = CHUNK_CORNER_RIGHT;

        m_iStraightCount = 0;
        m_iStraightMax = 2 + rand() % 2;

        pChunk = CJSCornerChunk::Create(m_pGraphicDev, vPos, m_pLayer, eType, m_eCurrentDir);

        // 생성 후 방향 전환
        if (eType == CHUNK_CORNER_LEFT)
            m_eCurrentDir = Turn_Left(m_eCurrentDir);
        else
            m_eCurrentDir = Turn_Right(m_eCurrentDir);
    }
    else
    {
        _int iRand = rand() % 10;
        if (iRand == 7)
            eType = CHUNK_LEFT;
        else if (iRand == 8)
            eType = CHUNK_RIGHT;
        else if (iRand == 9)
            eType = CHUNK_GAP;

        ++m_iStraightCount;

        pChunk = CJSChunk::Create(m_pGraphicDev, vPos, m_pLayer, eType, m_eCurrentDir);
    }

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

    _float fDist = 0.f;
    switch (m_eCurrentDir)
    {
    case DIR_FORWARD:
        fDist = vPlayerPos.z - vBackPos.z;
        break;
    case DIR_RIGHT:
        fDist = vPlayerPos.x - vBackPos.x;
        break;
    case DIR_LEFT:
        fDist = vBackPos.x - vPlayerPos.x;
        break;
    }

    if (fDist > m_fChunkSize + 10.f)
    {
        m_ChunkList.front()->Set_Dead();
        m_RemoveList.push_back(m_ChunkList.front());
        m_ChunkList.pop_front();
    }
}

DIRECTION CJSChunkMgr::Turn_Left(DIRECTION eDir)
{
    switch (eDir)
    {
    case DIR_FORWARD:   return DIR_LEFT;
    case DIR_LEFT:      return DIR_FORWARD;
    case DIR_RIGHT:     return DIR_FORWARD;
    }
    return DIR_FORWARD;
}

DIRECTION CJSChunkMgr::Turn_Right(DIRECTION eDir)
{
    switch (eDir)
    {
    case DIR_FORWARD:   return DIR_RIGHT;
    case DIR_RIGHT:     return DIR_FORWARD;
    case DIR_LEFT:      return DIR_FORWARD;
    }
    return DIR_FORWARD;
}

void CJSChunkMgr::Free()
{
	for (auto& pChunk : m_ChunkList)
		Safe_Release(pChunk);

	m_ChunkList.clear();
}
