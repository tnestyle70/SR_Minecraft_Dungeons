#pragma once
#include "CVIBuffer.h"
#include "Engine_Define.h"

BEGIN(Engine)

class ENGINE_DLL CJSSpriteBuffer : public CVIBuffer
{
protected:
    explicit CJSSpriteBuffer();
    explicit CJSSpriteBuffer(LPDIRECT3DDEVICE9 pGraphicDev);
    explicit CJSSpriteBuffer(const CJSSpriteBuffer& rhs);
    virtual ~CJSSpriteBuffer();

public:
    virtual HRESULT     Ready_Buffer();
    virtual void        Render_Buffer();

public:
    static CJSSpriteBuffer* Create(LPDIRECT3DDEVICE9 pGraphicDev);
    virtual CComponent* Clone();

private:
    virtual void Free();
};

END