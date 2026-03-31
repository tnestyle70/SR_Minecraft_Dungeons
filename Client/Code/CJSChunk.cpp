#include "pch.h"
#include "CJSChunk.h"
#include "CRenderer.h"
#include "CLayer.h"

const _float CJSChunk::TILE_SIZE = 1.0f;

CJSChunk::CJSChunk(LPDIRECT3DDEVICE9 pGraphicDev)
	: CGameObject(pGraphicDev)
	, m_pLayer(nullptr)
	, m_pTransformCom(nullptr)
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
            TILEID eTileID = TILE_NORMAL;

            switch (m_eChunkType)
            {
            case CHUNK_LEFT:   // 왼쪽 1열만 (x==0만 살아있음)
                if (x != 0)
                    eTileID = TILE_EMPTY;
                break;
            case CHUNK_RIGHT:  // 오른쪽 1열만 (x==2만 살아있음)
                if (x != 2)
                    eTileID = TILE_EMPTY;
                break;
            default:           // CHUNK_FULL
                break;
            }

            _vec3 vTilePos =
            {
                vChunkPos.x + (x - 1) * TILE_SIZE,
                vChunkPos.y,
                vChunkPos.z + z * TILE_SIZE
            };

            CJSTile* pTile = CJSTile::Create(m_pGraphicDev, vTilePos, eTileID);
            if (pTile == nullptr)
                return E_FAIL;

            m_pLayer->Add_GameObject(L"Environment_Layer", pTile);
            m_vecTile.push_back(pTile);
        }
    }

    return S_OK;
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
	CGameObject::Free();
}