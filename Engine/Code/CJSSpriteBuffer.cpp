#include "CJSSpriteBuffer.h"

CJSSpriteBuffer::CJSSpriteBuffer()
{
}

CJSSpriteBuffer::CJSSpriteBuffer(LPDIRECT3DDEVICE9 pGraphicDev)
    : CVIBuffer(pGraphicDev)
{
}

CJSSpriteBuffer::CJSSpriteBuffer(const CJSSpriteBuffer& rhs)
    : CVIBuffer(rhs)
{
}

CJSSpriteBuffer::~CJSSpriteBuffer()
{
}

HRESULT CJSSpriteBuffer::Ready_Buffer()
{
    m_dwVtxSize = sizeof(VTXTEX);
    m_dwVtxCnt = 4;
    m_dwTriCnt = 2;
    m_dwFVF = FVF_TEX;
    m_dwIdxSize = sizeof(INDEX32);
    m_IdxFmt = D3DFMT_INDEX32;

    //if (FAILED(m_pGraphicDev->CreateVertexBuffer(
    //    m_dwVtxCnt * m_dwVtxSize,
    //    D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
    //    m_dwFVF,
    //    D3DPOOL_DEFAULT,
    //    &m_pVB, nullptr)))
    //    return E_FAIL;

    if (FAILED(m_pGraphicDev->CreateVertexBuffer(
        m_dwVtxCnt * m_dwVtxSize,
        D3DUSAGE_WRITEONLY,  // DYNAMIC Á¦°Å
        m_dwFVF,
        D3DPOOL_MANAGED,     // DEFAULT ¡æ MANAGED
        &m_pVB, nullptr)))
        return E_FAIL;

    if (FAILED(m_pGraphicDev->CreateIndexBuffer(
        m_dwTriCnt * m_dwIdxSize,
        0,
        m_IdxFmt,
        D3DPOOL_MANAGED,
        &m_pIB, nullptr)))
        return E_FAIL;

    VTXTEX* pVertex = nullptr;
    m_pVB->Lock(0, 0, (void**)&pVertex, D3DLOCK_DISCARD);

    pVertex[0].vPosition = { -1.f,  1.f, 0.f }; pVertex[0].vTexUV = { 0.f, 0.f };
    pVertex[1].vPosition = { 1.f,  1.f, 0.f }; pVertex[1].vTexUV = { 1.f, 0.f };
    pVertex[2].vPosition = { 1.f, -1.f, 0.f }; pVertex[2].vTexUV = { 1.f, 1.f };
    pVertex[3].vPosition = { -1.f, -1.f, 0.f }; pVertex[3].vTexUV = { 0.f, 1.f };

    m_pVB->Unlock();

    INDEX32* pIndex = nullptr;
    m_pIB->Lock(0, 0, (void**)&pIndex, 0);

    pIndex[0]._0 = 0; pIndex[0]._1 = 1; pIndex[0]._2 = 2;
    pIndex[1]._0 = 0; pIndex[1]._1 = 2; pIndex[1]._2 = 3;

    m_pIB->Unlock();

    return S_OK;
}

void CJSSpriteBuffer::Render_Buffer()
{
    CVIBuffer::Render_Buffer();
}

CJSSpriteBuffer* CJSSpriteBuffer::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
    CJSSpriteBuffer* pBuffer = new CJSSpriteBuffer(pGraphicDev);

    if (FAILED(pBuffer->Ready_Buffer()))
    {
        Safe_Release(pBuffer);
        MSG_BOX("JSSpriteBuffer Create Failed");
        return nullptr;
    }

    return pBuffer;
}

CComponent* CJSSpriteBuffer::Clone()
{
    return new CJSSpriteBuffer(*this);
}

void CJSSpriteBuffer::Free()
{
    CVIBuffer::Free();
}