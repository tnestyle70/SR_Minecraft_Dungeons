#pragma once
#include "CVIBuffer.h"

BEGIN(Engine)

class ENGINE_DLL CJSCubeTex : public CVIBuffer
{
protected:
	explicit CJSCubeTex();
	explicit CJSCubeTex(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CJSCubeTex(const CJSCubeTex& rhs);
	virtual ~CJSCubeTex();

public:
	HRESULT Ready_Buffer();
	virtual void Render_Buffer();

public:
	static CJSCubeTex* Create(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual CComponent* Clone();

private:
	virtual void Free();
};

END