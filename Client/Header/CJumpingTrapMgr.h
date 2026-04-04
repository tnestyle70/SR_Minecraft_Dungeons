#pragma once
#include "CJumpingTrap.h"

class CPlayer;

class CJumpingTrapMgr : public CBase
{
	DECLARE_SINGLETON(CJumpingTrapMgr)
private:
	explicit CJumpingTrapMgr();
	virtual ~CJumpingTrapMgr();
public:
	HRESULT Ready_JumpingTrapMgr();
	_int Update(const _float& fTimeDelta);
	void LateUpdate(const _float& fTimeDelta);
	void Render();
public:
	void Activate(int iTriggerID);
public:
	void Add_JumpingTrap(CGameObject* pGameObject, int iTriggerID);
	void Clear();

	void Set_Player(CPlayer* pPlayer);

	void Set_GroupVisible(int iTriggerID, bool bVisible);
public:
	bool Is_Active(int iTriggerID);
private://Jumping Trap을 컨테이너로 관리
	map<int, vector<CJumpingTrap*>> m_mapJumpingTrapGroups;
	map<int, bool> m_mapVisible; //아이디별 가시성 관리
	bool m_bClosed = false;
	//닫혔는지 열렸는지 map으로 관리
	map<int, bool> m_mapClosed;
	CPlayer* m_pPlayer = nullptr;
private:
	virtual void Free();
};
