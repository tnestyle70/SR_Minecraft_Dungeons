#include "CFlexibleCubeTex.h"

CFlexibleCubeTex::CFlexibleCubeTex()
{}

CFlexibleCubeTex::CFlexibleCubeTex(LPDIRECT3DDEVICE9 pGraphicDev)
	: CVIBuffer(pGraphicDev)
{}

CFlexibleCubeTex::CFlexibleCubeTex(const CFlexibleCubeTex& rhs)
	: CVIBuffer(rhs)
{}

CFlexibleCubeTex::~CFlexibleCubeTex()
{}

HRESULT CFlexibleCubeTex::Ready_Buffer(const MESH& mesh)
{
	m_dwVtxSize = sizeof(VTXCUBEBODY);
	m_dwVtxCnt = 24;
	m_dwTriCnt = 12;
	m_dwFVF = FVF_CUBEBODY;
	m_dwIdxSize = sizeof(INDEX16);
	m_IdxFmt = D3DFMT_INDEX16;

	if (FAILED(CVIBuffer::Ready_Buffer()))
		return E_FAIL;

	VTXCUBEBODY* pVtx = NULL;
	m_pVB->Lock(0, 0, (void**)&pVtx, 0);

	INDEX16* pIdx = NULL;
	m_pIB->Lock(0, 0, (void**)&pIdx, 0);

	const _vec3* c = mesh.corners;

	// 면-코너 매핑 (CCubeBodyTex와 동일한 와인딩)
	// front  : c[0] c[1] c[2] c[3]
	// back   : c[4] c[5] c[6] c[7]
	// top    : c[5] c[4] c[1] c[0]
	// bottom : c[3] c[2] c[7] c[6]
	// right  : c[4] c[1] c[2] c[7]
	// left   : c[0] c[5] c[6] c[3]

	// ---- 정면 (+Z) ----
	SetFace(pVtx, pIdx, 0, 0, c[0], c[1], c[2], c[3], mesh.front);
	// ---- 후면 (-Z) ----
	SetFace(pVtx, pIdx, 4, 2, c[4], c[5], c[6], c[7], mesh.back);
	// ---- 윗면 (+Y) ----
	SetFace(pVtx, pIdx, 8, 4, c[5], c[4], c[1], c[0], mesh.top);
	// ---- 아랫면 (-Y) ----
	SetFace(pVtx, pIdx, 12, 6, c[3], c[2], c[7], c[6], mesh.bottom);
	// ---- 오른쪽 (+X) ----
	SetFace(pVtx, pIdx, 16, 8, c[4], c[1], c[2], c[7], mesh.right);
	// ---- 왼쪽 (-X) ----
	SetFace(pVtx, pIdx, 20, 10, c[0], c[5], c[6], c[3], mesh.left);

	m_pVB->Unlock();
	m_pIB->Unlock();

	return S_OK;
}

void CFlexibleCubeTex::Render_Buffer()
{
	CVIBuffer::Render_Buffer();
}

void CFlexibleCubeTex::SetFace(VTXCUBEBODY* pVtx, INDEX16* pIdx,
	int vtxBase, int idxBase,
	_vec3 vP0, _vec3 vP1, _vec3 vP2, _vec3 vP3,
	const FACE_UV& uv)
{
	// 아웃워드 노말: cross(p2-p0, p1-p0) — CCubeBodyTex 와인딩 기준
	_vec3 vEdge1 = vP1 - vP0;
	_vec3 vEdge2 = vP2 - vP0;
	_vec3 vNormal;
	D3DXVec3Cross(&vNormal, &vEdge2, &vEdge1);
	D3DXVec3Normalize(&vNormal, &vNormal);

	// p0 - 좌상
	pVtx[vtxBase + 0].vPosition = vP0;
	pVtx[vtxBase + 0].vNormal = vNormal;
	pVtx[vtxBase + 0].vTexUV = { uv.u0, uv.v0 };
	// p1 - 우상
	pVtx[vtxBase + 1].vPosition = vP1;
	pVtx[vtxBase + 1].vNormal = vNormal;
	pVtx[vtxBase + 1].vTexUV = { uv.u1, uv.v0 };
	// p2 - 우하
	pVtx[vtxBase + 2].vPosition = vP2;
	pVtx[vtxBase + 2].vNormal = vNormal;
	pVtx[vtxBase + 2].vTexUV = { uv.u1, uv.v1 };
	// p3 - 좌하
	pVtx[vtxBase + 3].vPosition = vP3;
	pVtx[vtxBase + 3].vNormal = vNormal;
	pVtx[vtxBase + 3].vTexUV = { uv.u0, uv.v1 };

	// 삼각형 0 1 2
	pIdx[idxBase]._0 = (_ushort)(vtxBase + 0);
	pIdx[idxBase]._1 = (_ushort)(vtxBase + 1);
	pIdx[idxBase]._2 = (_ushort)(vtxBase + 2);
	// 삼각형 0 2 3
	pIdx[idxBase + 1]._0 = (_ushort)(vtxBase + 0);
	pIdx[idxBase + 1]._1 = (_ushort)(vtxBase + 2);
	pIdx[idxBase + 1]._2 = (_ushort)(vtxBase + 3);
}

CFlexibleCubeTex* CFlexibleCubeTex::Create(LPDIRECT3DDEVICE9 pGraphicDev, const MESH& mesh)
{
	CFlexibleCubeTex* pBuffer = new CFlexibleCubeTex(pGraphicDev);

	if (FAILED(pBuffer->Ready_Buffer(mesh)))
	{
		Safe_Release(pBuffer);
		MSG_BOX("CFlexibleCubeTex Create Failed");
		return nullptr;
	}

	return pBuffer;
}

CComponent* CFlexibleCubeTex::Clone()
{
	return new CFlexibleCubeTex(*this);
}

void CFlexibleCubeTex::Free()
{
	CVIBuffer::Free();
}
