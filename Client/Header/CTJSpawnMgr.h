#pragma once
#include "CBase.h"
#include "CMonster.h"
#include "CMonsterMgr.h"
#include "CTJExpOrb.h"
#include "CTJMagnet.h"

class CTJPlayer;
class CPlayer;

class CTJSpawnMgr : public CBase
{
    DECLARE_SINGLETON(CTJSpawnMgr)

private:
    explicit CTJSpawnMgr();
    virtual ~CTJSpawnMgr();

public:
    void Update(const _float& fTimeDelta);
    void Set_Player(CPlayer* pPlayer) { m_pPlayer = pPlayer; }
    void Set_GraphicDev(LPDIRECT3DDEVICE9 pDev) { m_pGraphicDev = pDev; }
    void Clear();

private:
    void Spawn_Monster();

public:
    void Set_TJPlayer(CTJPlayer* pPlayer) { m_pTJPlayer = pPlayer; }
    void Set_SpawnInterval(float f) { m_fSpawnInterval = f; }
    void Stop_Spawn() { m_bSpawnStop = true; }
    vector<CTJExpOrb*>& Get_ExpOrbs() { return m_vecExpOrbs; }
    void Render_Magnet();
private:
    LPDIRECT3DDEVICE9 m_pGraphicDev = nullptr;
    CPlayer* m_pPlayer = nullptr;

    _float m_fSpawnTimer = 0.f;
    _float m_fSpawnInterval = 2.f;
    _float m_fSpawnRadius = 15.f;
    _int m_iMaxMonsters = 80;

    vector<CTJExpOrb*> m_vecExpOrbs;
    CTJPlayer* m_pTJPlayer = nullptr;
    vector<CMonster*> m_vecDeadMonsters;
    bool m_bSpawnStop = false;
    vector<CTJMagnet*> m_vecMagnets;

private:
    virtual void Free();
};