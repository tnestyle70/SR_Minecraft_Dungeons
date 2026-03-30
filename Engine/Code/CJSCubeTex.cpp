#include "CJSCubeTex.h"

CJSCubeTex::CJSCubeTex()
{
}

CJSCubeTex::CJSCubeTex(LPDIRECT3DDEVICE9 pGraphicDev)
	: CVIBuffer(pGraphicDev)
{
}

CJSCubeTex::CJSCubeTex(const CJSCubeTex& rhs)
	: CVIBuffer(rhs)
{
}

CJSCubeTex::~CJSCubeTex()
{
}

HRESULT CJSCubeTex::Ready_Buffer()
{
    m_dwVtxSize = sizeof(VTXCUBEBODY);
    m_dwVtxCnt = 24;
    m_dwTriCnt = 12;
    m_dwFVF = FVF_CUBEBODY;
    m_dwIdxSize = sizeof(INDEX32);
    m_IdxFmt = D3DFMT_INDEX32;

    if (FAILED(CVIBuffer::Ready_Buffer()))
        return E_FAIL;

    VTXCUBEBODY* pVertex = nullptr;
    m_pVB->Lock(0, 0, (void**)&pVertex, 0);

    // +Z
    pVertex[0] = { {-0.5f,  0.5f,  0.5f}, {0.f, 0.f, 1.f}, {0.f, 0.f} };
    pVertex[1] = { { 0.5f,  0.5f,  0.5f}, {0.f, 0.f, 1.f}, {1.f, 0.f} };
    pVertex[2] = { { 0.5f, -0.5f,  0.5f}, {0.f, 0.f, 1.f}, {1.f, 1.f} };
    pVertex[3] = { {-0.5f, -0.5f,  0.5f}, {0.f, 0.f, 1.f}, {0.f, 1.f} };

    // -Z
    pVertex[4] = { { 0.5f,  0.5f, -0.5f}, {0.f, 0.f, -1.f}, {0.f, 0.f} };
    pVertex[5] = { {-0.5f,  0.5f, -0.5f}, {0.f, 0.f, -1.f}, {1.f, 0.f} };
    pVertex[6] = { {-0.5f, -0.5f, -0.5f}, {0.f, 0.f, -1.f}, {1.f, 1.f} };
    pVertex[7] = { { 0.5f, -0.5f, -0.5f}, {0.f, 0.f, -1.f}, {0.f, 1.f} };

    // +Y
    pVertex[8] = { {-0.5f,  0.5f, -0.5f}, {0.f, 1.f, 0.f}, {0.f, 0.f} };
    pVertex[9] = { { 0.5f,  0.5f, -0.5f}, {0.f, 1.f, 0.f}, {1.f, 0.f} };
    pVertex[10] = { { 0.5f,  0.5f,  0.5f}, {0.f, 1.f, 0.f}, {1.f, 1.f} };
    pVertex[11] = { {-0.5f,  0.5f,  0.5f}, {0.f, 1.f, 0.f}, {0.f, 1.f} };

    // -Y
    pVertex[12] = { {-0.5f, -0.5f,  0.5f}, {0.f, -1.f, 0.f}, {0.f, 0.f} };
    pVertex[13] = { { 0.5f, -0.5f,  0.5f}, {0.f, -1.f, 0.f}, {1.f, 0.f} };
    pVertex[14] = { { 0.5f, -0.5f, -0.5f}, {0.f, -1.f, 0.f}, {1.f, 1.f} };
    pVertex[15] = { {-0.5f, -0.5f, -0.5f}, {0.f, -1.f, 0.f}, {0.f, 1.f} };

    // +X
    pVertex[16] = { { 0.5f,  0.5f,  0.5f}, {1.f, 0.f, 0.f}, {0.f, 0.f} };
    pVertex[17] = { { 0.5f,  0.5f, -0.5f}, {1.f, 0.f, 0.f}, {1.f, 0.f} };
    pVertex[18] = { { 0.5f, -0.5f, -0.5f}, {1.f, 0.f, 0.f}, {1.f, 1.f} };
    pVertex[19] = { { 0.5f, -0.5f,  0.5f}, {1.f, 0.f, 0.f}, {0.f, 1.f} };

    // -X
    pVertex[20] = { {-0.5f,  0.5f, -0.5f}, {-1.f, 0.f, 0.f}, {0.f, 0.f} };
    pVertex[21] = { {-0.5f,  0.5f,  0.5f}, {-1.f, 0.f, 0.f}, {1.f, 0.f} };
    pVertex[22] = { {-0.5f, -0.5f,  0.5f}, {-1.f, 0.f, 0.f}, {1.f, 1.f} };
    pVertex[23] = { {-0.5f, -0.5f, -0.5f}, {-1.f, 0.f, 0.f}, {0.f, 1.f} };

    m_pVB->Unlock();

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

void CJSCubeTex::Render_Buffer()
{
	CVIBuffer::Render_Buffer();
}

CJSCubeTex* CJSCubeTex::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
	CJSCubeTex* pJSCubeTex = new CJSCubeTex(pGraphicDev);

	if (FAILED(pJSCubeTex->Ready_Buffer()))
	{
		Safe_Release(pJSCubeTex);
		MSG_BOX("JSCubeTex Create Failed");
		return nullptr;
	}

	return pJSCubeTex;
}

CComponent* CJSCubeTex::Clone()
{
	return new CJSCubeTex(*this);
}

void CJSCubeTex::Free()
{
	CVIBuffer::Free();
}