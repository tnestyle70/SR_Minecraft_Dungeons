#pragma once
#include "CMonster.h"

class CCYMonster : public CMonster
{
public:
    explicit CCYMonster(LPDIRECT3DDEVICE9 pGraphicDev);
    virtual ~CCYMonster();

public:
    virtual HRESULT Ready_GameObject(_vec3& vPos) override
    {
        return CMonster::Ready_GameObject(vPos);
    }

public:
    static CCYMonster* Create(LPDIRECT3DDEVICE9 pGraphicDev,
        EMonsterType eType, _vec3 vPos);

protected:
    virtual void Free();
};