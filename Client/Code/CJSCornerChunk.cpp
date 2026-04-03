#include "pch.h"
#include "CJSCornerChunk.h"
#include "CRenderer.h"
#include "CLayer.h"
#include "CJSChunkMgr.h"

const _float CJSCornerChunk::TILE_SIZE = 2.f;

CJSCornerChunk::CJSCornerChunk(LPDIRECT3DDEVICE9 pGraphicDev)
    : CJSBaseChunk(pGraphicDev)
    , m_pBufferCom(nullptr)
    , m_pTransformCom(nullptr)
{
}

CJSCornerChunk::~CJSCornerChunk()
{
}

HRESULT CJSCornerChunk::Ready_GameObject(_vec3 vPos, CLayer* pLayer, CHUNKTYPE eType, DIRECTION eDir)
{
    m_pLayer = pLayer;
    m_eChunkType = eType;
    m_eDir = eDir;

    if (FAILED(Add_Component()))
        return E_FAIL;

    m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
    m_pTransformCom->Update_Component(0.f);

    if (FAILED(Ready_Tile(vPos)))
        return E_FAIL;

    return S_OK;
}

HRESULT CJSCornerChunk::Ready_Tile(_vec3 vChunkPos)
{
    for (_int z = 0; z < CORNER_SIZE; ++z)
    {
        for (_int x = 0; x < CORNER_SIZE; ++x)
        {
            _bool bWall = false;

            if (m_eChunkType == CHUNK_CORNER_RIGHT)
            {
                // 위쪽 행 전체 벽
                if (z == CORNER_SIZE - 1) bWall = true;
                // 왼쪽 열 전체 벽
                if (x == 0) bWall = true;
                // 오른쪽 아래 코너 벽
                if (z == 0 && x == CORNER_SIZE - 1) bWall = true;
            }
            else if (m_eChunkType == CHUNK_CORNER_LEFT)
            {
                // 위쪽 행 전체 벽
                if (z == CORNER_SIZE - 1) bWall = true;
                // 오른쪽 열 전체 벽
                if (x == CORNER_SIZE - 1) bWall = true;
                // 왼쪽 아래 코너 벽
                if (z == 0 && x == 0) bWall = true;
            }

            _vec3 vTilePos = {};
            vTilePos.y = vChunkPos.y;

            // 방향에 따라 좌표 계산
            switch (m_eDir)
            {
            case DIR_FORWARD:
                vTilePos.x = vChunkPos.x + (x - 2) * TILE_SIZE;
                vTilePos.z = vChunkPos.z + z * TILE_SIZE;
                break;
            case DIR_RIGHT:
                vTilePos.x = vChunkPos.x + z * TILE_SIZE;
                vTilePos.z = vChunkPos.z - (x - 2) * TILE_SIZE;
                break;
            case DIR_LEFT:
                vTilePos.x = vChunkPos.x - z * TILE_SIZE;
                vTilePos.z = vChunkPos.z + (x - 2) * TILE_SIZE;
                break;
            case DIR_BACKWARD:
                vTilePos.x = vChunkPos.x - (x - 2) * TILE_SIZE;
                vTilePos.z = vChunkPos.z - z * TILE_SIZE;
            }

            if (bWall)
            {
                for (_int y = 0; y <= 2; ++y)
                {
                    _vec3 vWallPos = vTilePos;
                    vWallPos.y = vChunkPos.y + y * TILE_SIZE;

                    CJSTile* pTile = CJSTile::Create(m_pGraphicDev, vWallPos, TILE_NORMAL);
                    if (!pTile) return E_FAIL;
                    m_pLayer->Add_GameObject(L"Tile", pTile);
                    m_vecWall.push_back(pTile);
                }
            }
            else
            {
                CJSTile* pTile = CJSTile::Create(m_pGraphicDev, vTilePos, TILE_NORMAL);
                if (!pTile) return E_FAIL;
                m_pLayer->Add_GameObject(L"Tile", pTile);
                m_vecTile.push_back(pTile);
            }
        }
    }
    return S_OK;
}

_vec3 CJSCornerChunk::Get_EndPos()
{
    _vec3 vPos;
    m_pTransformCom->Get_Info(INFO_POS, &vPos);

    _float fLong = TILE_SIZE * CORNER_SIZE - (2 * TILE_SIZE);  // 6.f
    _float fShort = 2 * TILE_SIZE;  // 4.f

    switch (m_eChunkType)
    {
    case CHUNK_CORNER_RIGHT:
        switch (m_eDir)
        {
        case DIR_FORWARD:   vPos.x += fLong;  vPos.z += fShort; break;
        case DIR_RIGHT:     vPos.z -= fLong;  vPos.x += fShort; break;
        case DIR_BACKWARD:  vPos.x -= fLong;  vPos.z -= fShort; break;
        case DIR_LEFT:      vPos.z += fLong;  vPos.x -= fShort; break;
        }
        break;

    case CHUNK_CORNER_LEFT:
        switch (m_eDir)
        {
        case DIR_FORWARD:   vPos.x -= fLong;  vPos.z += fShort; break;
        case DIR_LEFT:      vPos.z -= fLong;  vPos.x -= fShort; break;
        case DIR_BACKWARD:  vPos.x += fLong;  vPos.z -= fShort; break;
        case DIR_RIGHT:     vPos.z += fLong;  vPos.x += fShort; break;
        }
        break;
    }
    return vPos;
}

_int CJSCornerChunk::Update_GameObject(const _float& fTimeDelta)
{
    return CGameObject::Update_GameObject(fTimeDelta);
}

void CJSCornerChunk::LateUpdate_GameObject(const _float& fTimeDelta)
{
    CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CJSCornerChunk::Render_GameObject()
{
}

HRESULT CJSCornerChunk::Add_Component()
{
    CComponent* pComponent = nullptr;

    pComponent = m_pTransformCom = dynamic_cast<CTransform*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Transform", pComponent });

    return S_OK;
}

CJSCornerChunk* CJSCornerChunk::Create(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vPos, CLayer* pLayer, CHUNKTYPE eType, DIRECTION eDir)
{
    CJSCornerChunk* pCorner = new CJSCornerChunk(pGraphicDev);
    if (FAILED(pCorner->Ready_GameObject(vPos, pLayer, eType, eDir)))
    {
        Safe_Release(pCorner);
        MSG_BOX("CornerChunk Create Failed");
        return nullptr;
    }
    return pCorner;
}

void CJSCornerChunk::Free()
{
    for (auto& pTile : m_vecTile)
        pTile->Set_Dead();
    m_vecTile.clear();

    for (auto& pWall : m_vecWall)
        pWall->Set_Dead();
    m_vecWall.clear();

    CGameObject::Free();
}