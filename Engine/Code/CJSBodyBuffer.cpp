#include "CJSBodyBuffer.h"

CJSBodyBuffer::CJSBodyBuffer()
{
}

CJSBodyBuffer::CJSBodyBuffer(LPDIRECT3DDEVICE9 pGraphicDev)
    : CVIBuffer(pGraphicDev)
{
}

CJSBodyBuffer::CJSBodyBuffer(const CJSBodyBuffer& rhs)
    : CVIBuffer(rhs)
{
}

CJSBodyBuffer::~CJSBodyBuffer()
{
}

HRESULT CJSBodyBuffer::Ready_Buffer()
{
    m_dwVtxSize = sizeof(VTXCUBEBODY);
    m_dwVtxCnt = 24;
    m_dwTriCnt = 12;
    m_dwFVF = FVF_CUBEBODY;
    m_dwIdxSize = sizeof(INDEX32);
    m_IdxFmt = D3DFMT_INDEX32;

    if (FAILED(CVIBuffer::Ready_Buffer()))
        return E_FAIL;

    INDEX32* pIndex = nullptr;
    m_pIB->Lock(0, 0, (void**)&pIndex, 0);

    for (_ulong i = 0; i < 6; ++i)
    {
        _ulong base = i * 4;
        _ulong tri = i * 2;

        pIndex[tri + 0]._0 = base + 0;
        pIndex[tri + 0]._1 = base + 1;
        pIndex[tri + 0]._2 = base + 2;
        pIndex[tri + 1]._0 = base + 0;
        pIndex[tri + 1]._1 = base + 2;
        pIndex[tri + 1]._2 = base + 3;
    }

    m_pIB->Unlock();
    return S_OK;
}

void CJSBodyBuffer::Render_Buffer()
{
    CVIBuffer::Render_Buffer();
}

void CJSBodyBuffer::Set_SizeAndUVs(_float fX, _float fY, _float fZ, FaceUV front, FaceUV back, FaceUV left, FaceUV right, FaceUV top, FaceUV bottom)
{
    _float hX = fX * 0.5f;
    _float hY = fY * 0.5f;
    _float hZ = fZ * 0.5f;

    VTXCUBEBODY* pVertex = nullptr;
    m_pVB->Lock(0, 0, (void**)&pVertex, 0);

    // FRONT
    pVertex[0].vPosition = { -hX,  hY,  hZ }; pVertex[0].vTexUV = { front.u0, front.v0 };
    pVertex[1].vPosition = { hX,  hY,  hZ }; pVertex[1].vTexUV = { front.u1, front.v0 };
    pVertex[2].vPosition = { hX, -hY,  hZ }; pVertex[2].vTexUV = { front.u1, front.v1 };
    pVertex[3].vPosition = { -hX, -hY,  hZ }; pVertex[3].vTexUV = { front.u0, front.v1 };

    // BACK
    pVertex[4].vPosition = { hX,  hY, -hZ }; pVertex[4].vTexUV = { back.u0, back.v0 };
    pVertex[5].vPosition = { -hX,  hY, -hZ }; pVertex[5].vTexUV = { back.u1, back.v0 };
    pVertex[6].vPosition = { -hX, -hY, -hZ }; pVertex[6].vTexUV = { back.u1, back.v1 };
    pVertex[7].vPosition = { hX, -hY, -hZ }; pVertex[7].vTexUV = { back.u0, back.v1 };

    // LEFT
    pVertex[8].vPosition = { -hX,  hY, -hZ }; pVertex[8].vTexUV = { left.u0, left.v0 };
    pVertex[9].vPosition = { -hX,  hY,  hZ }; pVertex[9].vTexUV = { left.u1, left.v0 };
    pVertex[10].vPosition = { -hX, -hY,  hZ }; pVertex[10].vTexUV = { left.u1, left.v1 };
    pVertex[11].vPosition = { -hX, -hY, -hZ }; pVertex[11].vTexUV = { left.u0, left.v1 };

    // RIGHT
    pVertex[12].vPosition = { hX,  hY,  hZ }; pVertex[12].vTexUV = { right.u0, right.v0 };
    pVertex[13].vPosition = { hX,  hY, -hZ }; pVertex[13].vTexUV = { right.u1, right.v0 };
    pVertex[14].vPosition = { hX, -hY, -hZ }; pVertex[14].vTexUV = { right.u1, right.v1 };
    pVertex[15].vPosition = { hX, -hY,  hZ }; pVertex[15].vTexUV = { right.u0, right.v1 };

    // TOP
    pVertex[16].vPosition = { -hX,  hY, -hZ }; pVertex[16].vTexUV = { top.u0, top.v0 };
    pVertex[17].vPosition = { hX,  hY, -hZ }; pVertex[17].vTexUV = { top.u1, top.v0 };
    pVertex[18].vPosition = { hX,  hY,  hZ }; pVertex[18].vTexUV = { top.u1, top.v1 };
    pVertex[19].vPosition = { -hX,  hY,  hZ }; pVertex[19].vTexUV = { top.u0, top.v1 };

    // BOTTOM
    pVertex[20].vPosition = { -hX, -hY,  hZ }; pVertex[20].vTexUV = { bottom.u0, bottom.v0 };
    pVertex[21].vPosition = { hX, -hY,  hZ }; pVertex[21].vTexUV = { bottom.u1, bottom.v0 };
    pVertex[22].vPosition = { hX, -hY, -hZ }; pVertex[22].vTexUV = { bottom.u1, bottom.v1 };
    pVertex[23].vPosition = { -hX, -hY, -hZ }; pVertex[23].vTexUV = { bottom.u0, bottom.v1 };

    m_pVB->Unlock();
}

CJSBodyBuffer* CJSBodyBuffer::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
    CJSBodyBuffer* pBuffer = new CJSBodyBuffer(pGraphicDev);

    if (FAILED(pBuffer->Ready_Buffer()))
    {
        Safe_Release(pBuffer);
        MSG_BOX("JSBodyBuffer Create Failed");
        return nullptr;
    }

    return pBuffer;
}

CComponent* CJSBodyBuffer::Clone()
{
    return new CJSBodyBuffer(*this);
}

void CJSBodyBuffer::Free()
{
    CVIBuffer::Free();
}