#pragma once
#include "CVIBuffer.h"
#include "Engine_Define.h"

//한 면 UV 영역
struct FACE_UV
{
	float u0, v0;
	float u1, v1;
};

struct CUBE
{
	float fWidth;
	float fHeight;
	float fDepth;
	//6면 UV 
	FACE_UV front;
	FACE_UV back;
	FACE_UV top;
	FACE_UV bottom;
	FACE_UV right;
	FACE_UV left;
};

struct MESH
{
	_vec3 corners[8];
	FACE_UV front, back, top, bottom, left, right;
};

// CUBE -> MESH 변환 헬퍼 (기존 정육면체 코드 마이그레이션용)
inline MESH CUBE_To_MESH(const CUBE& c)
{
	float hw = c.fWidth * 0.5f, hh = c.fHeight * 0.5f, hd = c.fDepth * 0.5f;
	MESH m{};
	m.corners[0] = { -hw,+hh,+hd }; m.corners[1] = { +hw,+hh,+hd };
	m.corners[2] = { +hw,-hh,+hd }; m.corners[3] = { -hw,-hh,+hd };
	m.corners[4] = { +hw,+hh,-hd }; m.corners[5] = { -hw,+hh,-hd };
	m.corners[6] = { -hw,-hh,-hd }; m.corners[7] = { +hw,-hh,-hd };
	m.front = c.front; m.back = c.back; m.top = c.top;
	m.bottom = c.bottom; m.right = c.right; m.left = c.left;
	return m;
}

BEGIN(Engine)

class ENGINE_DLL CCubeBodyTex : public CVIBuffer
{
protected:
	explicit CCubeBodyTex();
	explicit CCubeBodyTex(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CCubeBodyTex(const CCubeBodyTex& rhs);
	virtual ~CCubeBodyTex();
public:
	HRESULT Ready_Buffer(const CUBE& cube);
	virtual void Render_Buffer();
public:
	static CCubeBodyTex* Create(LPDIRECT3DDEVICE9 pGraphicDev,
		const CUBE& cube);
	virtual CComponent* Clone();
private://각 면에 버텍스 4개 + 인덱스 2삼각형 세팅 한 다음에
	//UV 좌표 설정해주기
	void SetFace(VTXCUBEBODY* pVtx, INDEX16* pIndex,
		int vtxBase, int idxBase, _vec3 vPos0,
		_vec3 vPos1, _vec3 vPos2, _vec3 vPos3,
		_vec3 vNormal, const FACE_UV& uv); 
private:
	virtual void Free();
};

END