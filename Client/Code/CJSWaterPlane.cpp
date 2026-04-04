#include "pch.h"
#include "CJSWaterPlane.h"
#include "CRenderer.h"

const _float CJSWaterPlane::GRID_SIZE = 6.f;
const _float CJSWaterPlane::PLANE_Y = -1.f;

struct VTXWATER
{
    _vec3   vPosition;
    DWORD   dwColor;
};
#define FVF_WATER (D3DFVF_XYZ | D3DFVF_DIFFUSE)

CJSWaterPlane::CJSWaterPlane(LPDIRECT3DDEVICE9 pGraphicDev)
    : CGameObject(pGraphicDev)
{
}

CJSWaterPlane::~CJSWaterPlane()
{
}

HRESULT CJSWaterPlane::Ready_GameObject()
{
    if (FAILED(Create_Buffer()))
        return E_FAIL;

    return S_OK;
}

HRESULT CJSWaterPlane::Create_Buffer()
{
    _int iVtxCnt = (GRID_X + 1) * (GRID_Z + 1);
    _int iTriCnt = GRID_X * GRID_Z * 2;

    if (FAILED(m_pGraphicDev->CreateVertexBuffer(
        iVtxCnt * sizeof(VTXWATER),
        D3DUSAGE_WRITEONLY,  // DYNAMIC 제거
        FVF_WATER,
        D3DPOOL_MANAGED,     // DEFAULT → MANAGED
        &m_pVB, nullptr)))
        return E_FAIL;

    if (FAILED(m_pGraphicDev->CreateIndexBuffer(
        iTriCnt * sizeof(INDEX32),
        0,
        D3DFMT_INDEX32,
        D3DPOOL_MANAGED,
        &m_pIB, nullptr)))
        return E_FAIL;

    INDEX32* pIndex = nullptr;
    m_pIB->Lock(0, 0, (void**)&pIndex, 0);

    _int iIdx = 0;
    for (_int z = 0; z < GRID_Z; ++z)
    {
        for (_int x = 0; x < GRID_X; ++x)
        {
            _int iBase = z * (GRID_X + 1) + x;

            pIndex[iIdx]._0 = iBase;
            pIndex[iIdx]._1 = iBase + 1;
            pIndex[iIdx]._2 = iBase + (GRID_X + 1) + 1;
            ++iIdx;

            pIndex[iIdx]._0 = iBase;
            pIndex[iIdx]._1 = iBase + (GRID_X + 1) + 1;
            pIndex[iIdx]._2 = iBase + (GRID_X + 1);
            ++iIdx;
        }
    }

    m_pIB->Unlock();
    return S_OK;
}

void CJSWaterPlane::Update_Wave(const _float& fTimeDelta)
{
    m_fWaveTime += fTimeDelta * m_fWaveSpeed;
    _float fHalfX = GRID_X * GRID_SIZE * 0.5f;
    _float fHalfZ = GRID_Z * GRID_SIZE * 0.5f;

    VTXWATER* pVertex = nullptr;
    m_pVB->Lock(0, 0, (void**)&pVertex, 0);

    for (_int z = 0; z <= GRID_Z; ++z)
    {
        for (_int x = 0; x <= GRID_X; ++x)
        {
            _float fX = m_vPlayerPos.x - fHalfX + x * GRID_SIZE;
            _float fZ = m_vPlayerPos.z - fHalfZ + z * GRID_SIZE;

            _float fY = PLANE_Y +
                sinf(fX * m_fWaveFreq + m_fWaveTime) * m_fWaveHeight +
                sinf(fZ * m_fWaveFreq * 0.7f + m_fWaveTime * 0.8f) * m_fWaveHeight;

            _float fDistFromPlayer = sqrtf(
                powf(fX - m_vPlayerPos.x, 2.f) +
                powf(fZ - m_vPlayerPos.z, 2.f));
            _float fFade = 1.f - min(fDistFromPlayer / (fHalfX), 1.f);

            BYTE r = (_byte)(50);
            BYTE g = (_byte)(87 * fFade);
            BYTE b = (_byte)(90 * fFade);

            pVertex[z * (GRID_X + 1) + x].vPosition = { fX, fY, fZ };
            pVertex[z * (GRID_X + 1) + x].dwColor = D3DCOLOR_XRGB(r, g, b);
        }
    }
    m_pVB->Unlock();
}

_int CJSWaterPlane::Update_GameObject(const _float& fTimeDelta)
{
    _int iExit = CGameObject::Update_GameObject(fTimeDelta);

    Update_Wave(fTimeDelta);

    return iExit;
}

void CJSWaterPlane::LateUpdate_GameObject(const _float& fTimeDelta)
{
    CGameObject::LateUpdate_GameObject(fTimeDelta);

    CRenderer::GetInstance()->Add_RenderGroup(RENDER_NONALPHA, this);
}

void CJSWaterPlane::Render_GameObject()
{
    m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);  // 컬링 OFF
    m_pGraphicDev->SetTexture(0, nullptr);
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

    _matrix matWorld;
    D3DXMatrixIdentity(&matWorld);
    m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);

    m_pGraphicDev->SetStreamSource(0, m_pVB, 0, sizeof(VTXWATER));
    m_pGraphicDev->SetFVF(FVF_WATER);
    m_pGraphicDev->SetIndices(m_pIB);
    m_pGraphicDev->DrawIndexedPrimitive(
        D3DPT_TRIANGLELIST, 0, 0,
        (GRID_X + 1) * (GRID_Z + 1),
        0,
        GRID_X * GRID_Z * 2);

    m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);  // 복구
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
}

CJSWaterPlane* CJSWaterPlane::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
    CJSWaterPlane* pWater = new CJSWaterPlane(pGraphicDev);

    if (FAILED(pWater->Ready_GameObject()))
    {
        Safe_Release(pWater);
        MSG_BOX("WaterPlane Create Failed");
        return nullptr;
    }
    return pWater;
}

void CJSWaterPlane::Free()
{
    Safe_Release(m_pVB);
    Safe_Release(m_pIB);
    CGameObject::Free();
}

