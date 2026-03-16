#pragma once
#include "CIronBar.h"

class CIronBarMgr : public CBase
{
	DECLARE_SINGLETON(CIronBarMgr)
private:
	explicit CIronBarMgr();
	virtual ~CIronBarMgr();
public:
	HRESULT Ready_IronBarMgr();
	_int Update(const _float& fTimeDelta);
	void LateUpdate(const _float& fTimeDelta);
	void Render();
public:
	void Open(int iTriggerID);
	void Close(int iTriggerID);
public:
	void AddIronBar(CGameObject* pGameObject, int iTriggerID);
	void Clear();
public:
	bool IsClosed(int iTriggerID);
private:
	void UpdateIronBarAnim();
private://IronBar을 컨테이너로 관리
	map<int, vector<CIronBar*>> m_mapIronBarGroups;
	bool m_bClosed = false;
	//닫혔는지 열렸는지 map으로 관리
	map<int, bool> m_mapClosed;
private:
	virtual void Free();
};