#pragma once
#include "CMonster.h"
#include "CPlayer.h"
//스테이지별로 트리거 박스 밟으면 생성될 몬스터 목록들 저장해두고,
//트리거 박스 밟을 경우 몬스터들 지정된 위치로 쭉 스폰되도록 설정
//MonsterMgr에서 IronBarMgr의 false 상태로 전환 시키기

class CMonster;
class CRedStoneGolem;
class CAncientGuradian;
class CEnderDragon;
class CSpawnEffect;

struct SpawnGroup
{
	vector<CMonster*> vecMonsters = {};
	float fSpawnTimer = 0.f;
	float fSpawnDelay = 1.f;
	int iNextSpawnIndex = 0;
	bool bTriggered = false;
	bool bAllSpawned = false;
};

class CMonsterMgr : public CBase
{
	DECLARE_SINGLETON(CMonsterMgr)
private:
	explicit CMonsterMgr();
	virtual ~CMonsterMgr();
public:
	HRESULT Ready_MonsterMgr(LPDIRECT3DDEVICE9 pGraphicDev);
	_int Update(const _float& fTimeDelta);
	void LateUpdate(const _float& fTimeDelta);
	void Render();
private:
	void SpawnMonsters(const _float& fTimeDelta);
	void CheckCursorHover();
public:
	bool IsGroupAllDead(int iTriggerID);
	void SetActiveMonsterGroup(int iTriggerID);
	void SetPlayer(CPlayer* pPlayer) { m_pPlayer = pPlayer; }
	CPlayer* Get_Player() const { return m_pPlayer; }

	//외부에서 몬스터 목록 받아가기
	const map<int, SpawnGroup>& Get_MonsterGroups() const { return m_mapMonsterGroups; }

private:
	CPlayer* m_pPlayer = nullptr;

public:
	void AddMonster(CGameObject* pGameObject, int iTriggerID, _vec3 vPos);
	void AddGuardian(CAncientGuardian* pBoss) { m_pGuardian = pBoss; }
	void AddGolem(CRedStoneGolem* pBoss) { m_pGolem = pBoss; }
	void Add_EnderDragon(CEnderDragon* pEnderDragon) { m_pEnderDragon = pEnderDragon; }
	void Clear();
; private: 
	//트리거 박스 밟았을 경우 생성할 ID별 몬스터 목록
	CAncientGuardian* m_pGuardian = nullptr;
	CRedStoneGolem* m_pGolem = nullptr;
	CEnderDragon* m_pEnderDragon = nullptr;
	map<int, SpawnGroup> m_mapMonsterGroups = {};
	map<int, vector<_vec3>> m_mapMonsterPos = {};
	int m_iTriggerID = -1;

	//스폰 이펙트
	vector<CSpawnEffect*> m_vecSpawnEffects = {};

	LPDIRECT3DDEVICE9 m_pGraphicDev = nullptr;

private:
	virtual void Free();
};
