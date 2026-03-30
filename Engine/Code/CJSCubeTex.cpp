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

	VTXCUBEBODY* pVertex = NULL;
	m_pVB->Lock(0, 0, (void**)&pVertex, 0);

	

	m_pVB->Unlock();

	INDEX32* pIndex = NULL;
	m_pIB->Lock(0, 0, (void**)&pIndex, 0);

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
}

CComponent* CJSCubeTex::Clone()
{
	return new CJSCubeTex(*this);
}

void CJSCubeTex::Free()
{
	CVIBuffer::Free();
}