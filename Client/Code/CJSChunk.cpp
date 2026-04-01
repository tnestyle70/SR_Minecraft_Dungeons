#include "pch.h"
#include "CJSChunk.h"
#include "CRenderer.h"
#include "CLayer.h"

const _float CJSChunk::TILE_SIZE = 1.0f;

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
                // y축 3칸, 아래, 중간, 위
                for (_int y = 0; y <= 2; ++y)
                {
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

TILEID CJSChunk::Get_TileID(_vec3 vPlayerPos)
{
    _vec3 vChunkPos;
    m_pTransformCom->Get_Info(INFO_POS, &vChunkPos);

    // X 인덱스 (바닥 타일은 x=1,2,3 → 인덱스 0,1,2)
    _int iX = (_int)((vPlayerPos.x - (vChunkPos.x - 1.f * TILE_SIZE)) / TILE_SIZE);
    // Z 인덱스
    _int iZ = (_int)((vPlayerPos.z - vChunkPos.z) / TILE_SIZE);

    // 범위 체크
    if (iX < 0 || iX >= 3 || iZ < 0 || iZ >= TILE_Z)
        return TILE_EMPTY;

    _int iIndex = iZ * 3 + iX;

    if (iIndex < 0 || iIndex >= (_int)m_vecTile.size())
        return TILE_EMPTY;

    return m_vecTile[iIndex]->Get_TileID();
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

    CGameObject::Free();
}