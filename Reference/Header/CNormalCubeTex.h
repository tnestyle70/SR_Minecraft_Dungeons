#pragma once
#include "CVIBuffer.h"
#include "Engine_Define.h"

BEGIN(Engine)

class ENGINE_DLL CNormalCubeTex : public CVIBuffer
{
protected:
	explicit CNormalCubeTex();
	explicit CNormalCubeTex(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CNormalCubeTex(const CNormalCubeTex& rhs);
	virtual ~CNormalCubeTex();

public:
	HRESULT		Ready_Buffer(float fwidth, float fHeight, float fDepth);
	virtual  void Render_Buffer();


public:
	static CNormalCubeTex* Create(LPDIRECT3DDEVICE9,float fWidth, float fHeight, float fDepth);
	virtual CComponent* Clone();

private:
	void SetFace(VTXCUBEBODY* pVtx, INDEX16* pIndex, int vtxBase, int idxBase, _vec3 v0, _vec3 v1, _vec3 v2, _vec3 v3, _vec3 vNormal);

private:
	virtual void Free();

};

END