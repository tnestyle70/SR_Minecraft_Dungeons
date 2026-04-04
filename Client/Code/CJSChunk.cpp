#include "pch.h"
#include "CJSChunk.h"
#include "CRenderer.h"
#include "CLayer.h"
#include "CJSScoreMgr.h"
#include "CSoundMgr.h"
#include "CManagement.h"

const _float CJSChunk::TILE_SIZE = 2.0f;

CJSChunk::CJSChunk(LPDIRECT3DDEVICE9 pGraphicDev)
	: CJSBaseChunk(pGraphicDev)
	, m_pTransformCom(nullptr)
    , m_eChunkType(CHUNK_FULL)
{
}

CJSChunk::~CJSChunk()
{
}

HRESULT CJSChunk::Ready_GameObject(_vec3 vPos, CLayer* pLayer, CHUNKTYPE eType, DIRECTION eDir)
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
    if (FAILED(Ready_Collider(vPos)))
        return E_FAIL;
    if (FAILED(Ready_Obstacle(vPos)))
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
            _vec3 vTilePos = Calc_TilePos(vChunkPos, x, z);

            if (bWall)
            {
                if (m_eChunkType == CHUNK_LEFT && x == 4)
                    continue;

                if (m_eChunkType == CHUNK_RIGHT && x == 0)
                    continue;

                for (_int y = 0; y <= 2; ++y)
                {
                    if (m_eChunkType == CHUNK_GAP)
                    {
                        _int iMidZ = TILE_Z / 2;
                        if (z >= iMidZ - 1 && z <= iMidZ + 1)
                            continue;
                    }

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
                TILEID eTileID = TILE_NORMAL;
                switch (m_eChunkType)
                {
                case CHUNK_LEFT:
                    if (x != 1) eTileID = TILE_EMPTY;
                    break;
                case CHUNK_RIGHT:
                    if (x != 3) eTileID = TILE_EMPTY;
                    break;
                case CHUNK_GAP:
                {
                    _int iMidZ = TILE_Z / 2;
                    if (z >= iMidZ - 1 && z <= iMidZ + 1)
                        eTileID = TILE_EMPTY;
                    break;
                }
                default:
                    break;
                }

                CJSTile* pTile = CJSTile::Create(m_pGraphicDev, vTilePos, eTileID);
                if (!pTile) return E_FAIL;
                m_pLayer->Add_GameObject(L"Tile", pTile);
                m_vecTile.push_back(pTile);
            }
        }
    }
    return S_OK;
}

HRESULT CJSChunk::Ready_Emerald(_vec3 vChunkPos)
{
    if (m_bHasObstacle)
        return S_OK;

    switch (m_eChunkType)
    {
    case CHUNK_FULL:
        Ready_Emerald_Line(vChunkPos);
        break;
    case CHUNK_LEFT:
        break;
    case CHUNK_RIGHT:
        break;
    case CHUNK_GAP:
        Ready_Emerald_Parabola(vChunkPos);
        break;
    }
    return S_OK;
}

HRESULT CJSChunk::Ready_Emerald_Line(_vec3 vChunkPos)
{
    for (_int z = 0; z < TILE_Z; z += 2)
    {
        _vec3 vPos = Calc_TilePos(vChunkPos, 2, z);

        vPos.y = vChunkPos.y + 2.5f;

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

    for (_int z = 0; z < TILE_Z; z += 2)
    {
        _float fT = (_float)(z - iMidZ) / (_float)iMidZ;

        _float fParabolaY = 4.f * (1.f - fT * fT);

        _vec3 vPos = Calc_TilePos(vChunkPos, 2, z);

        vPos.y = vChunkPos.y + 2.5f + fParabolaY;

        CJSEmerald* pEmerald = CJSEmerald::Create(m_pGraphicDev, vPos);

        if (pEmerald == nullptr)
            return E_FAIL;

        m_pLayer->Add_GameObject(L"JSEmerald", pEmerald);
        m_vecEmerald.push_back(pEmerald);
    }
    return S_OK;
}

HRESULT CJSChunk::Ready_Collider(_vec3 vChunkPos)
{
    _float fWallOffset = 2.f * TILE_SIZE;   // 4.f  (벽 위치)
    _float fHeight = 3.f * TILE_SIZE;   // 6.f  (벽 높이)
    _float fLength = TILE_Z * TILE_SIZE; // 청크 전체 길이 32.f
    _float fThickness = TILE_SIZE;          // 2.f  (벽 두께)

    _vec3 vLeftCenter = {};
    _vec3 vRightCenter = {};
    _vec3 vSizeAlongZ = { fThickness, fHeight, fLength };  // Z방향 청크용
    _vec3 vSizeAlongX = { fLength, fHeight, fThickness };  // X방향 청크용

    _float fHalfLen = fLength * 0.5f;
    _float fHalfY = fHeight * 0.5f;

    switch (m_eDir)
    {
    case DIR_FORWARD:
        vLeftCenter = { vChunkPos.x - fWallOffset, vChunkPos.y + fHalfY, vChunkPos.z + fHalfLen };
        vRightCenter = { vChunkPos.x + fWallOffset, vChunkPos.y + fHalfY, vChunkPos.z + fHalfLen };
        m_pLeftWallCol = CJSCollider::Create(m_pGraphicDev, vLeftCenter, vSizeAlongZ);
        m_pRightWallCol = CJSCollider::Create(m_pGraphicDev, vRightCenter, vSizeAlongZ);
        break;

    case DIR_BACKWARD:
        vLeftCenter = { vChunkPos.x + fWallOffset, vChunkPos.y + fHalfY, vChunkPos.z - fHalfLen };
        vRightCenter = { vChunkPos.x - fWallOffset, vChunkPos.y + fHalfY, vChunkPos.z - fHalfLen };
        m_pLeftWallCol = CJSCollider::Create(m_pGraphicDev, vLeftCenter, vSizeAlongZ);
        m_pRightWallCol = CJSCollider::Create(m_pGraphicDev, vRightCenter, vSizeAlongZ);
        break;

    case DIR_RIGHT:
        vLeftCenter = { vChunkPos.x + fHalfLen, vChunkPos.y + fHalfY, vChunkPos.z + fWallOffset };
        vRightCenter = { vChunkPos.x + fHalfLen, vChunkPos.y + fHalfY, vChunkPos.z - fWallOffset };
        m_pLeftWallCol = CJSCollider::Create(m_pGraphicDev, vLeftCenter, vSizeAlongX);
        m_pRightWallCol = CJSCollider::Create(m_pGraphicDev, vRightCenter, vSizeAlongX);
        break;

    case DIR_LEFT:
        vLeftCenter = { vChunkPos.x - fHalfLen, vChunkPos.y + fHalfY, vChunkPos.z - fWallOffset };
        vRightCenter = { vChunkPos.x - fHalfLen, vChunkPos.y + fHalfY, vChunkPos.z + fWallOffset };
        m_pLeftWallCol = CJSCollider::Create(m_pGraphicDev, vLeftCenter, vSizeAlongX);
        m_pRightWallCol = CJSCollider::Create(m_pGraphicDev, vRightCenter, vSizeAlongX);
        break;
    }

    return S_OK;
}

HRESULT CJSChunk::Ready_Obstacle(_vec3 vChunkPos)
{
    if (m_eChunkType != CHUNK_FULL)
        return S_OK;

    if (rand() % 10 >= 1)
        return S_OK;

    m_bHasObstacle = true;

    _int iObstacleZ = TILE_Z / 2;

    for (_int x = 1; x <= 3; ++x)
    {
        _vec3 vPos = Calc_TilePos(vChunkPos, x, iObstacleZ);
        vPos.y = vChunkPos.y + 2.5f;

        CJSObstacle* pObstacle = CJSObstacle::Create(m_pGraphicDev, vPos);
        if (pObstacle == nullptr)
            return E_FAIL;

        m_pLayer->Add_GameObject(L"Obstacle", pObstacle);
        m_vecObstacle.push_back(pObstacle);
    }

    return S_OK;
}

TILEID CJSChunk::Get_TileID(_vec3 vPlayerPos)
{
    for (auto& pTile : m_vecTile)
    {
        if (pTile->Is_Dead())
            continue;
        if (pTile->Get_TileID() == TILE_EMPTY)
            continue;

        _vec3 vTilePos;
        pTile->Get_Position(vTilePos);

        _float fDist = sqrtf(
            powf(vTilePos.x - vPlayerPos.x, 2.f) +
            powf(vTilePos.z - vPlayerPos.z, 2.f)
        );

        if (fDist <= TILE_SIZE * 1.f)
            return TILE_NORMAL;
    }
    return TILE_EMPTY;
}

void CJSChunk::Check_Collect(_vec3 vPlayerPos)
{
    for (auto it = m_vecEmerald.begin(); it != m_vecEmerald.end();)
    {
        if ((*it)->Check_Collect(vPlayerPos))
        {
            (*it)->Set_Dead();

            CJSScoreMgr::GetInstance()->Add_Score(1);

            CSoundMgr::GetInstance()->PlayEffect(L"Emerald/sfx_item_emerald-001_soundWave.wav", 0.8f);

            it = m_vecEmerald.erase(it);
        }
        else
            ++it;
    }
}

_vec3 CJSChunk::Get_EndPos()
{
    _vec3 vPos;
    m_pTransformCom->Get_Info(INFO_POS, &vPos);

    _float fChunkLength = TILE_SIZE * TILE_Z;

    switch (m_eDir)
    {
    case DIR_FORWARD:
        vPos.z += fChunkLength;
        break;
    case DIR_RIGHT:
        vPos.x += fChunkLength;
        break;
    case DIR_LEFT:
        vPos.x -= fChunkLength;
        break;
    case DIR_BACKWARD:
        vPos.z -= fChunkLength;
    }
    return vPos;
}

_bool CJSChunk::Check_WallCollision(_vec3 vPlayerPos)
{
    for (auto& pWall : m_vecWall)
    {
        if (pWall->Is_Dead())
            continue;

        _vec3 vWallPos;
        pWall->Get_Position(vWallPos);

        _float fDist = sqrtf(
            powf(vWallPos.x - vPlayerPos.x, 2.f) +
            powf(vWallPos.z - vPlayerPos.z, 2.f)
        );

        if (fDist <= TILE_SIZE * 1.f)
            return true;
    }
    return false;
}

_bool CJSChunk::Check_ObstacleCollision(CJSCollider* pPlayerCol)
{
    for (auto& pObstacle : m_vecObstacle)
    {
        if (pObstacle->Is_Dead())
            continue;

        if (pPlayerCol->Check_Collision(pObstacle->Get_Collider()))
            return true;
    }

    return false;
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

_vec3 CJSChunk::Calc_TilePos(_vec3 vChunkPos, _int x, _int z)
{
    _vec3 vPos = {};
    vPos.y = vChunkPos.y;

    switch (m_eDir)
    {
    case DIR_FORWARD:
        vPos.x = vChunkPos.x + (x - 2) * TILE_SIZE;
        vPos.z = vChunkPos.z + z * TILE_SIZE;
        break;

    case DIR_RIGHT:
        vPos.x = vChunkPos.x + z * TILE_SIZE;
        vPos.z = vChunkPos.z - (x - 2) * TILE_SIZE;
        break;

    case DIR_LEFT:
        vPos.x = vChunkPos.x - z * TILE_SIZE;
        vPos.z = vChunkPos.z + (x - 2) * TILE_SIZE;
        break;

    case DIR_BACKWARD:
        vPos.x = vChunkPos.x - (x - 2) * TILE_SIZE;
        vPos.z = vChunkPos.z - z * TILE_SIZE;
    }

    return vPos;
}

CJSChunk* CJSChunk::Create(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vPos, CLayer* pLayer, CHUNKTYPE eType, DIRECTION eDir)
{
	CJSChunk* pJSChunk = new CJSChunk(pGraphicDev);

	if (FAILED(pJSChunk->Ready_GameObject(vPos, pLayer, eType, eDir)))
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

    for (auto& pObstacle : m_vecObstacle)
        pObstacle->Set_Dead();
    m_vecObstacle.clear();

    if (m_pLeftWallCol)
        Safe_Release(m_pLeftWallCol);
    if (m_pRightWallCol)
        Safe_Release(m_pRightWallCol);

    CGameObject::Free();
}