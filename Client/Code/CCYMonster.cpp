#include "pch.h"
#include "CCYMonster.h"

CCYMonster::CCYMonster(LPDIRECT3DDEVICE9 pGraphicDev)
    : CMonster(pGraphicDev)
{
}

CCYMonster::~CCYMonster()
{
}

CCYMonster* CCYMonster::Create(LPDIRECT3DDEVICE9 pGraphicDev,
    EMonsterType eType, _vec3 vPos)
{
    CCYMonster* pMonster = new CCYMonster(pGraphicDev);
    pMonster->m_eType = eType;  // Set_Type 대신 직접 접근

    if (FAILED(pMonster->Ready_GameObject(vPos)))
    {
        Safe_Release(pMonster);
        MSG_BOX("CCYMonster Create Failed");
        return nullptr;
    }
    return pMonster;
}

void CCYMonster::Free()
{
    CMonster::Free();
}