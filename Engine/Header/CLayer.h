#pragma once
#include "CBase.h"
#include "CGameObject.h"

BEGIN(Engine)

class ENGINE_DLL CLayer : public CBase
{
private:
	explicit CLayer();
	virtual ~CLayer();

public:
	CComponent*		Get_Component(COMPONENTID eID, const _tchar* pObjTag, const _tchar* pComponentTag);
	HRESULT			Add_GameObject(const _tchar* pObjTag, CGameObject* pGameObject);
	_int			Delete_GameObject(const _float& fTimeDelta); // 몬스터 삭제 

	// 주승 오브젝트 삭제
	//void Delete_GameObjectByTag(const _tchar* pObjTag);
	void Delete_GameObjectByPtr(CGameObject* pTarget);
	// 디버깅용
	_int Get_MapSize() { return m_mapObject.size(); }

public:
	HRESULT			Ready_Layer();
	_int			Update_Layer(const _float& fTimeDelta);
	void			LateUpdate_Layer(const _float& fTimeDelta); 


private:
	multimap<const _tchar*, CGameObject*>			m_mapObject;

public:
	static CLayer* Create();

private:
	virtual void	Free();
};

END