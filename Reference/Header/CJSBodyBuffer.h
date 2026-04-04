#pragma once
#include "CVIBuffer.h"

BEGIN(Engine)

class ENGINE_DLL CJSBodyBuffer : public CVIBuffer
{
public:
    struct FaceUV
    {
        _float u0, v0, u1, v1;
    };

protected:
    explicit CJSBodyBuffer();
    explicit CJSBodyBuffer(LPDIRECT3DDEVICE9 pGraphicDev);
    explicit CJSBodyBuffer(const CJSBodyBuffer& rhs);
    virtual ~CJSBodyBuffer();

public:
    virtual HRESULT Ready_Buffer() override;
    virtual void    Render_Buffer() override;
    void            Set_SizeAndUVs(_float fX, _float fY, _float fZ,
        FaceUV front, FaceUV back, FaceUV left, FaceUV right, FaceUV top, FaceUV bottom);

public:
    static CJSBodyBuffer* Create(LPDIRECT3DDEVICE9 pGraphicDev);
    virtual CComponent* Clone() override;

private:
    virtual void Free() override;
};

END