#include "pch.h"
#include "CJSChunkMgr.h"
#include "CLayer.h"
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

        CJSChunk* pChunk = CJSChunk::Create(m_pGraphicDev, vPos, m_pLayer, CHUNK_FULL, m_eCurrentDir);
        if (pChunk)
        {
            m_pLayer->Add_GameObject(L"Chunk", pChunk);
            m_ChunkList.push_back(pChunk);
        }
    }

    m_iStraightCount = 0;
    m_iStraightMax = 2 + rand() % 2;

    return S_OK;
}

void CJSChunkMgr::Update_Manager(const _float& fTimeDelta, _vec3 vPlayerPos)
{
    //for (auto& pChunk : m_RemoveList)
    //    Safe_Release(pChunk);

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
        case DIR_BACKWARD:
            fDist = vPlayerPos.z - vEndPos.z;
            break;
        }

        if (fDist < m_fChunkSize * (m_iRenderCount / 2))
        {
            Spawn_Chunk(vEndPos);
        }
    }
}

TILEID CJSChunkMgr::Get_TileID(_vec3 vPlayerPos)
{
    for (auto& pChunk : m_ChunkList)
    {
        CJSChunk* pJSChunk = dynamic_cast<CJSChunk*>(pChunk);
        if (pJSChunk == nullptr)
            continue;

        TILEID eID = pJSChunk->Get_TileID(vPlayerPos);
        if (eID == TILE_NORMAL)
            return TILE_NORMAL;
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

CHUNKTYPE CJSChunkMgr::Get_CurrentChunkType(_vec3 vPlayerPos)
{
    for (auto& pChunk : m_ChunkList)
    {
        CJSCornerChunk* pCorner = dynamic_cast<CJSCornerChunk*>(pChunk);
        if (pCorner == nullptr)
            continue;

        _vec3 vChunkPos;
        pCorner->Get_Position(vChunkPos);

        _float fSize = 10.f;
        _bool bInRange = false;

        switch (pCorner->Get_Dir())
        {
        case DIR_FORWARD:
            bInRange = (vPlayerPos.x >= vChunkPos.x - fSize &&
                vPlayerPos.x <= vChunkPos.x + fSize &&
                vPlayerPos.z >= vChunkPos.z - fSize * 0.5f &&
                vPlayerPos.z <= vChunkPos.z + fSize);
            break;
        case DIR_RIGHT:
            bInRange = (vPlayerPos.z >= vChunkPos.z - fSize &&
                vPlayerPos.z <= vChunkPos.z + fSize &&
                vPlayerPos.x >= vChunkPos.x - fSize * 0.5f &&
                vPlayerPos.x <= vChunkPos.x + fSize);
            break;
        case DIR_LEFT:
            bInRange = (vPlayerPos.z >= vChunkPos.z - fSize &&
                vPlayerPos.z <= vChunkPos.z + fSize &&
                vPlayerPos.x <= vChunkPos.x + fSize * 0.5f &&
                vPlayerPos.x >= vChunkPos.x - fSize);
            break;
        case DIR_BACKWARD:
            bInRange = (vPlayerPos.x >= vChunkPos.x - fSize &&
                vPlayerPos.x <= vChunkPos.x + fSize &&
                vPlayerPos.z <= vChunkPos.z + fSize * 0.5f &&
                vPlayerPos.z >= vChunkPos.z - fSize);
            break;
        }

        if (bInRange)
            return pCorner->Get_ChunkType();
    }
    return CHUNK_FULL;
}

void CJSChunkMgr::Spawn_Chunk(_vec3 vPos)
{
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

    // 방향 상관없이 3D 직선 거리로 체크
    _vec3 vDiff = vPlayerPos - vBackPos;
    vDiff.y = 0.f;  // Y 무시
    _float fDist = D3DXVec3Length(&vDiff);

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
    case DIR_LEFT:      return DIR_BACKWARD;
    case DIR_RIGHT:     return DIR_FORWARD;
    case DIR_BACKWARD:  return DIR_RIGHT;
    }
    return DIR_FORWARD;
}

DIRECTION CJSChunkMgr::Turn_Right(DIRECTION eDir)
{
    switch (eDir)
    {
    case DIR_FORWARD:   return DIR_RIGHT;
    case DIR_RIGHT:     return DIR_BACKWARD;
    case DIR_BACKWARD:  return DIR_LEFT;
    case DIR_LEFT:      return DIR_FORWARD;
    }
    return DIR_FORWARD;
}

_bool CJSChunkMgr::Check_WallCollision(CJSCollider* pPlayerCol)
{
    for (auto& pChunk : m_ChunkList)
    {
        CJSChunk* pJSChunk = dynamic_cast<CJSChunk*>(pChunk);
        if (pJSChunk)
        {
            if (pJSChunk->Get_LeftWallCol() &&
                pPlayerCol->Check_Collision(pJSChunk->Get_LeftWallCol()))
                return true;
            if (pJSChunk->Get_RightWallCol() &&
                pPlayerCol->Check_Collision(pJSChunk->Get_RightWallCol()))
                return true;
            continue;
        }

        CJSCornerChunk* pCorner = dynamic_cast<CJSCornerChunk*>(pChunk);
        if (pCorner)
        {
            for (auto& pCol : pCorner->Get_WallCols())
            {
                if (pPlayerCol->Check_Collision(pCol))
                    return true;
            }
        }
    }
    return false;
}

_bool CJSChunkMgr::Check_ObstacleCollision(CJSCollider* pPlayerCol)
{
    for (auto& pChunk : m_ChunkList)
    {
        CJSChunk* pJSChunk = dynamic_cast<CJSChunk*>(pChunk);
        if (pJSChunk == nullptr)
            continue;

        if (pJSChunk->Check_ObstacleCollision(pPlayerCol))
            return true;
    }
    return false;
}

void CJSChunkMgr::Free()
{
	for (auto& pChunk : m_ChunkList)
		Safe_Release(pChunk);

	m_ChunkList.clear();
}
