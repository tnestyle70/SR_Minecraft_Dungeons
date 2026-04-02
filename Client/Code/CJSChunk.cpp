#include "pch.h"
#include "CJSChunk.h"
#include "CRenderer.h"
#include "CLayer.h"
#include "CJSScoreMgr.h"

const _float CJSChunk::TILE_SIZE = 2.0f;

CJSChunk::CJSChunk(LPDIRECT3DDEVICE9 pGraphicDev)
	: CGameObject(pGraphicDev)
	, m_pLayer(nullptr)
	, m_pTransformCom(nullptr)
    , m_eChunkType(CHUNK_FULL)
{
}

CJSChunk::~CJSChunk()
{
}

HRESULT CJSChunk::Ready_GameObject(_vec3 vPos, CLayer* pLayer, CHUNKTYPE eType)
{
	m_pLayer = pLayer;
    m_eChunkType = eType;

	if (FAILED(Add_Component()))
		return E_FAIL;

	m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
	m_pTransformCom->Update_Component(0.f);

	if (FAILED(Ready_Tile(vPos)))
		return E_FAIL;

    if (FAILED(Ready_Emerald(vPos)))
        return E_FAIL;

	return S_OK;
}

_int CJSChunk::Update_GameObject(const _float& fTimeDelta)
{
	_int iExit = CGameObject::Update_GameObject(fTimeDelta);

	return iExit;
}

void CJSChunk::LateUpdate_GameObject(const _float& fTimeDelta)
{
	CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CJSChunk::Render_GameObject()
{
}

HRESULT CJSChunk::Ready_Tile(_vec3 vChunkPos)
{
    for (_int z = 0; z < TILE_Z; ++z)
    {
        for (_int x = 0; x < TILE_X; ++x)
        {
            _bool bWall = (x == 0 || x == 4);

            if (bWall)
            {
                for (_int y = 0; y <= 2; ++y)
                {
                    if (m_eChunkType == CHUNK_GAP)
                    {
                        _int iMidZ = TILE_Z / 2;
                        if (z >= iMidZ - 2 && z <= iMidZ + 2)
                            continue;
                    }

                    _vec3 vTilePos =
                    {
                        vChunkPos.x + (x - 2) * TILE_SIZE,
                        vChunkPos.y + y * TILE_SIZE,
                        vChunkPos.z + z * TILE_SIZE
                    };

                    CJSTile* pTile = CJSTile::Create(m_pGraphicDev, vTilePos, TILE_NORMAL);

                    if (pTile == nullptr)
                        return E_FAIL;

                    m_pLayer->Add_GameObject(L"Tile", pTile);
                    m_vecWall.push_back(pTile);
                }
            }
            else
            {
                TILEID eTileID = TILE_NORMAL;

                switch (m_eChunkType)
                {
                case CHUNK_LEFT:
                    if (x != 1)
                        eTileID = TILE_EMPTY;
                    break;

                case CHUNK_RIGHT:
                    if (x != 3)
                        eTileID = TILE_EMPTY;
                    break;

                case CHUNK_GAP:
                {
                    _int iMidZ = TILE_Z / 2;  // 8
                    if (z >= iMidZ - 2 && z <= iMidZ + 2)
                        eTileID = TILE_EMPTY;
                }
                break;

                default:
                    break;
                }

                _vec3 vTilePos =
                {
                    vChunkPos.x + (x - 2) * TILE_SIZE,
                    vChunkPos.y,
                    vChunkPos.z + z * TILE_SIZE
                };

                CJSTile* pTile = CJSTile::Create(m_pGraphicDev, vTilePos, eTileID);
                if (pTile == nullptr)
                    return E_FAIL;

                m_pLayer->Add_GameObject(L"Tile", pTile);
                m_vecTile.push_back(pTile);
            }
        }
    }
    return S_OK;
}

HRESULT CJSChunk::Ready_Emerald(_vec3 vChunkPos)
{
    switch (m_eChunkType)
    {
    case CHUNK_FULL:
        Ready_Emerald_Line(vChunkPos, 2);
        break;

    case CHUNK_LEFT:
        //Ready_Emerald_Line(vChunkPos, 1);
        break;

    case CHUNK_RIGHT:
        //Ready_Emerald_Line(vChunkPos, 3);
        break;

    case CHUNK_GAP:
        Ready_Emerald_Parabola(vChunkPos);
        break;

    case CHUNK_END:
        break;

    default:
        break;
    }

    return S_OK;
}

HRESULT CJSChunk::Ready_Emerald_Line(_vec3 vChunkPos, _int iX)
{
    for (_int z = 0; z < TILE_Z; z += 3)
    {
        _vec3 vPos =
        {
            vChunkPos.x + (iX - 2) * TILE_SIZE,
            vChunkPos.y + 3.0f,
            vChunkPos.z + z * TILE_SIZE
        };

        CJSEmerald* pEmerald = CJSEmerald::Create(m_pGraphicDev, vPos);

        if (pEmerald == nullptr)
            return E_FAIL;

        m_pLayer->Add_GameObject(L"JSEmerald", pEmerald);
        m_vecEmerald.push_back(pEmerald);
    }

    return S_OK;
}

HRESULT CJSChunk::Ready_Emerald_Parabola(_vec3 vChunkPos)
{
    _int iMidZ = TILE_Z / 2;

    for (_int z = 0; z < TILE_Z; z += 3)
    {
        _float fT = (_float)(z - iMidZ) / (_float)iMidZ;  // -1 ~ +1
        _float fParabolaY = 4.5f * (1.f - fT * fT);        // 최고점 4.5f

        _vec3 vPos =
        {
            vChunkPos.x,           // 가운데 X
            vChunkPos.y + 3.0f + fParabolaY,
            vChunkPos.z + z * TILE_SIZE
        };

        CJSEmerald* pEmerald = CJSEmerald::Create(m_pGraphicDev, vPos);

        if (pEmerald == nullptr)
            return E_FAIL;

        m_pLayer->Add_GameObject(L"JSEmerald", pEmerald);
        m_vecEmerald.push_back(pEmerald);
    }

    return S_OK;
}

TILEID CJSChunk::Get_TileID(_vec3 vPlayerPos)
{
    _vec3 vChunkPos;
    m_pTransformCom->Get_Info(INFO_POS, &vChunkPos);

    _float fSnappedX = roundf(vPlayerPos.x / TILE_SIZE) * TILE_SIZE;

    _int iX = (_int)((fSnappedX - (vChunkPos.x - 1.f * TILE_SIZE)) / TILE_SIZE);
    _int iZ = (_int)((vPlayerPos.z - vChunkPos.z) / TILE_SIZE);

    TCHAR szBuf[128];
    wsprintf(szBuf, L"iX: %d, iZ: %d, ChunkZ: %d, PlayerZ: %d",
        iX, iZ, (_int)vChunkPos.z, (_int)vPlayerPos.z);
    OutputDebugString(szBuf);
        
    // 범위 체크
    if (iX < 0 || iX >= 3 || iZ < 0 || iZ >= TILE_Z)
        return TILE_EMPTY;

    _int iIndex = iZ * 3 + iX;

    if (iIndex < 0 || iIndex >= (_int)m_vecTile.size())
        return TILE_EMPTY;

    return m_vecTile[iIndex]->Get_TileID();
}

void CJSChunk::Check_Collect(_vec3 vPlayerPos)
{
    for (auto it = m_vecEmerald.begin(); it != m_vecEmerald.end();)
    {
        if ((*it)->Check_Collect(vPlayerPos))
        {
            (*it)->Set_Dead();
            CJSScoreMgr::GetInstance()->Add_Score(1);
            it = m_vecEmerald.erase(it);
        }
        else
            ++it;
    }
}

HRESULT CJSChunk::Add_Component()
{
	CComponent* pComponent = nullptr;

	// Transform
	pComponent = m_pTransformCom = dynamic_cast<CTransform*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));

	if (pComponent == nullptr)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_Transform", pComponent });

	return S_OK;
}

CJSChunk* CJSChunk::Create(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vPos, CLayer* pLayer, CHUNKTYPE eType)
{
	CJSChunk* pJSChunk = new CJSChunk(pGraphicDev);

	if (FAILED(pJSChunk->Ready_GameObject(vPos, pLayer, eType)))
	{
		Safe_Release(pJSChunk);
		MSG_BOX("JSChunk Create Failed");
		return nullptr;
	}

	return pJSChunk;
}

void CJSChunk::Free()
{
    for (auto& pTile : m_vecTile)
        pTile->Set_Dead();
    m_vecTile.clear();

    for (auto& pWall : m_vecWall)
        pWall->Set_Dead();
    m_vecWall.clear();

    for (auto& pEmerald : m_vecEmerald)
        pEmerald->Set_Dead();
    m_vecEmerald.clear();

    CGameObject::Free();
}