#pragma once
#include "CCubeBodyTex.h"

BEGIN(Engine)

class ENGINE_DLL CFlexibleCubeTex : public CVIBuffer
{
protected:
	explicit CFlexibleCubeTex();
	explicit CFlexibleCubeTex(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CFlexibleCubeTex(const CFlexibleCubeTex& rhs);
	virtual ~CFlexibleCubeTex();
public:
	HRESULT Ready_Buffer(const MESH& mesh);
	virtual void Render_Buffer();
public:
	static CFlexibleCubeTex* Create(LPDIRECT3DDEVICE9 pGraphicDev,
		const MESH& mesh);
	virtual CComponent* Clone();
private://각 면에 버텍스 4개 + 인덱스 2삼각형 세팅 한 다음에
	//UV 좌표 설정해주기
	void SetFace(VTXCUBEBODY* pVtx, INDEX16* pIdx,
		int vtxBase, int idxBase,
		_vec3 vP0, _vec3 vP1, _vec3 vP2, _vec3 vP3,
		const FACE_UV& uv);
private:
	virtual void Free();
};

END