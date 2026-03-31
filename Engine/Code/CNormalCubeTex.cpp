#include "CNormalCubeTex.h"

//BEGIN(Engine)

CNormalCubeTex::CNormalCubeTex()
    : CVIBuffer()
{
}

CNormalCubeTex::CNormalCubeTex(LPDIRECT3DDEVICE9 pGraphicDev)
    : CVIBuffer(pGraphicDev)
{
}

CNormalCubeTex::CNormalCubeTex(const CNormalCubeTex& rhs)
    : CVIBuffer(rhs)
{
}

CNormalCubeTex::~CNormalCubeTex()
{
    Free();
}

HRESULT CNormalCubeTex::Ready_Buffer(float fWidth, float fHeight, float fDepth)
{
    m_dwVtxCnt = 24;
    m_dwVtxSize = sizeof(VTXCUBEBODY);
    m_dwTriCnt = 12;
    m_dwFVF = FVF_CUBEBODY;

    // 버텍스 버퍼 생성
    if (FAILED(m_pGraphicDev->CreateVertexBuffer(
        m_dwVtxCnt * m_dwVtxSize, 0, FVF_CUBEBODY,
        D3DPOOL_MANAGED, &m_pVB, nullptr)))
        return E_FAIL;

    // 인덱스 버퍼 생성
    if (FAILED(m_pGraphicDev->CreateIndexBuffer(
        m_dwTriCnt * sizeof(INDEX16), 0, D3DFMT_INDEX16,
        D3DPOOL_MANAGED, &m_pIB, nullptr)))
        return E_FAIL;

    VTXCUBEBODY* pV = nullptr;
    INDEX16* pI = nullptr;
    m_pVB->Lock(0, 0, (void**)&pV, 0);
    m_pIB->Lock(0, 0, (void**)&pI, 0);

    float hw = fWidth * 0.5f;
    float hh = fHeight * 0.5f;
    float hd = fDepth * 0.5f;

    // 앞면 (Normal: 0,0,-1)
    SetFace(pV, pI, 0, 0,
        { -hw,+hh,-hd }, { +hw,+hh,-hd }, { +hw,-hh,-hd }, { -hw,-hh,-hd },
        { 0.f, 0.f, -1.f });

    // 뒷면 (Normal: 0,0,+1)
    SetFace(pV, pI, 4, 2,
        { +hw,+hh,+hd }, { -hw,+hh,+hd }, { -hw,-hh,+hd }, { +hw,-hh,+hd },
        { 0.f, 0.f, +1.f });

    // 윗면 (Normal: 0,+1,0)
    SetFace(pV, pI, 8, 4,
        { -hw,+hh,+hd }, { +hw,+hh,+hd }, { +hw,+hh,-hd }, { -hw,+hh,-hd },
        { 0.f, +1.f, 0.f });

    // 아랫면 (Normal: 0,-1,0)
    SetFace(pV, pI, 12, 6,
        { -hw,-hh,-hd }, { +hw,-hh,-hd }, { +hw,-hh,+hd }, { -hw,-hh,+hd },
        { 0.f, -1.f, 0.f });

    // 오른면 (Normal: +1,0,0)
    SetFace(pV, pI, 16, 8,
        { +hw,+hh,-hd }, { +hw,+hh,+hd }, { +hw,-hh,+hd }, { +hw,-hh,-hd },
        { +1.f, 0.f, 0.f });

    // 왼면 (Normal: -1,0,0)
    SetFace(pV, pI, 20, 10,
        { -hw,+hh,+hd }, { -hw,+hh,-hd }, { -hw,-hh,-hd }, { -hw,-hh,+hd },
        { -1.f, 0.f, 0.f });

    m_pVB->Unlock();
    m_pIB->Unlock();
    return S_OK;
}

void CNormalCubeTex::SetFace(VTXCUBEBODY* pV, INDEX16* pI,
    int vb, int ib,
    _vec3 v0, _vec3 v1, _vec3 v2, _vec3 v3,
    _vec3 vN)
{
    pV[vb + 0] = { v0, vN, {0.f, 0.f} };
    pV[vb + 1] = { v1, vN, {1.f, 0.f} };
    pV[vb + 2] = { v2, vN, {1.f, 1.f} };
    pV[vb + 3] = { v3, vN, {0.f, 1.f} };

    pI[ib + 0] = { (WORD)(vb + 0), (WORD)(vb + 1), (WORD)(vb + 2) };
    pI[ib + 1] = { (WORD)(vb + 0), (WORD)(vb + 2), (WORD)(vb + 3) };
}

void CNormalCubeTex::Render_Buffer()
{
    m_pGraphicDev->SetFVF(FVF_CUBEBODY);
    m_pGraphicDev->SetStreamSource(0, m_pVB, 0, m_dwVtxSize);
    m_pGraphicDev->SetIndices(m_pIB);
    m_pGraphicDev->DrawIndexedPrimitive(
        D3DPT_TRIANGLELIST, 0, 0, m_dwVtxCnt, 0, m_dwTriCnt);
}

CNormalCubeTex* CNormalCubeTex::Create(LPDIRECT3DDEVICE9 pGraphicDev,
    float fWidth, float fHeight, float fDepth)
{
    CNormalCubeTex* pObj = new CNormalCubeTex(pGraphicDev);
    if (FAILED(pObj->Ready_Buffer(fWidth, fHeight, fDepth)))
    {
        Safe_Release(pObj);
        return nullptr;
    }
    return pObj;
}

CComponent* CNormalCubeTex::Clone()
{
    return new CNormalCubeTex(*this);
}

void CNormalCubeTex::Free()
{
    CVIBuffer::Free();
}

//END