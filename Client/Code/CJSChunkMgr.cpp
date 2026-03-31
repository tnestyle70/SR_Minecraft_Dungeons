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
        Spawn_Chunk(vPos);
    }

    return S_OK;
}

void CJSChunkMgr::Update_Manager(const _float& fTimeDelta, _vec3 vPlayerPos)
{
    Remove_OldChunk(vPlayerPos);

    if (!m_ChunkList.empty())
    {
        _vec3 vFrontPos;
        m_ChunkList.back()->Get_Position(vFrontPos);

        if (vFrontPos.z - vPlayerPos.z < m_fChunkSize * (m_iRenderCount / 2))
            Spawn_Chunk({ 0.f, 0.f, vFrontPos.z + m_fChunkSize });
    }
}

void CJSChunkMgr::Spawn_Chunk(_vec3 vPos)
{
    CHUNKTYPE eType = (CHUNKTYPE)(rand() % CHUNK_END);

    CJSChunk* pChunk = CJSChunk::Create(m_pGraphicDev, vPos, m_pLayer, eType);
    if (pChunk == nullptr)
        return;

    m_pLayer->Add_GameObject(L"Environment_Layer", pChunk);
    m_ChunkList.push_back(pChunk);
}

void CJSChunkMgr::Remove_OldChunk(_vec3 vPlayerPos)
{
    if (m_ChunkList.empty())
        return;

    _vec3 vBackPos;
    m_ChunkList.front()->Get_Position(vBackPos);

    if (vPlayerPos.z - vBackPos.z > m_fChunkSize)
    {
        // 레이어 삭제 나중에 처리
        // 지금은 리스트에서만

        m_ChunkList.pop_front();
    }
}

void CJSChunkMgr::Free()
{
	for (auto& pChunk : m_ChunkList)
		Safe_Release(pChunk);

	m_ChunkList.clear();
}
