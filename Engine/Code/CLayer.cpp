#include "CLayer.h"

CLayer::CLayer()
{
}

CLayer::~CLayer()
{
}

CComponent* CLayer::Get_Component(COMPONENTID eID, const _tchar* pObjTag, const _tchar* pComponentTag)
{
	auto	iter = find_if(m_mapObject.begin(), m_mapObject.end(), 
		CTag_Finder(pObjTag));

	if (iter == m_mapObject.end())
		return nullptr;

	return iter->second->Get_Component(eID, pComponentTag);
}

HRESULT CLayer::Add_GameObject(const _tchar* pObjTag, CGameObject* pGameObject)
{
	if (nullptr == pGameObject)
		return E_FAIL;

	m_mapObject.insert({ pObjTag, pGameObject });

	return S_OK;
}

_int CLayer::Delete_GameObject(const _float& fTimeDelta) // 몬스터 삭제 
{
	for (auto iter = m_mapObject.begin(); iter != m_mapObject.end();)
	{
		if (iter->second->Is_Dead())
		{

			Safe_Release(iter->second);
			iter = m_mapObject.erase(iter);
		}
		else
			++iter;
	}
	return 0;
}

//void CLayer::Delete_GameObjectByTag(const _tchar* pObjTag)
//{
//	auto range = m_mapObject.equal_range(pObjTag);
//	for (auto it = range.first; it != range.second; ++it)
//		Safe_Release(it->second);
//	m_mapObject.erase(range.first, range.second);
//}

void CLayer::Delete_GameObjectByPtr(CGameObject* pTarget)
{
	for (auto it = m_mapObject.begin(); it != m_mapObject.end(); ++it)
	{
		if (it->second == pTarget)
		{
			m_mapObject.erase(it);
			return;
		}
	}
}

HRESULT CLayer::Ready_Layer()
{
	return S_OK;
}

_int CLayer::Update_Layer(const _float& fTimeDelta)
{
	_int	iResult(0);

	for (auto& pObj : m_mapObject)
	{
		iResult = pObj.second->Update_GameObject(fTimeDelta);

		if (iResult & 0x80000000)
			return iResult;
	}

	return iResult;
}

void CLayer::LateUpdate_Layer(const _float& fTimeDelta)
{
	for (auto& pObj : m_mapObject)
		pObj.second->LateUpdate_GameObject(fTimeDelta);
}




CLayer* CLayer::Create()
{
	CLayer* pLayer = new CLayer;

	if (FAILED(pLayer->Ready_Layer()))
	{
		MSG_BOX("Layer Create Failed");
		Safe_Release(pLayer);
		return nullptr;
	}

	return pLayer;
}

void CLayer::Free()
{
	for_each(m_mapObject.begin(), m_mapObject.end(), CDeleteMap());
	m_mapObject.clear();
}
