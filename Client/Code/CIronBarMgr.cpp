#include "pch.h"
#include "CIronBarMgr.h"

IMPLEMENT_SINGLETON(CIronBarMgr)

CIronBarMgr::CIronBarMgr()
{
}

CIronBarMgr::~CIronBarMgr()
{
	Free();
}

HRESULT CIronBarMgr::Ready_IronBarMgr()
{
	return S_OK;
}

_int CIronBarMgr::Update(const _float& fTimeDelta)
{
	for (auto& pair : m_mapIronBarGroups)
	{
		for (auto& pIronBar : pair.second)
		{
			pIronBar->Update_GameObject(fTimeDelta);
		}
	}
	return 0;
}

void CIronBarMgr::LateUpdate(const _float & fTimeDelta)
{
	for (auto& pair : m_mapIronBarGroups)
	{
		for (auto& pIronBar : pair.second)
		{
			pIronBar->LateUpdate_GameObject(fTimeDelta);
		}
	}
}

void CIronBarMgr::Render()
{
	for (auto& pair : m_mapIronBarGroups)
	{
		for (auto& pIronBar : pair.second)
		{
			pIronBar->Render_GameObject();
		}
	}
}

void CIronBarMgr::Open(int iTriggerID)
{
	for (auto& pair : m_mapIronBarGroups)
	{
		if (pair.first == iTriggerID)
		{
			m_mapClosed[iTriggerID] = false;

			for (auto& pIronBar : pair.second)
			{
				pIronBar->SetIronBarState(eIronBarState::MOVE_UP);
			}
		}
	}

	return;
}

void CIronBarMgr::Close(int iTriggerID)
{
	for (auto& pair : m_mapIronBarGroups)
	{
		if(pair.first == iTriggerID)
		{
			m_mapClosed[iTriggerID] = true;

			for (auto& pIronBar : pair.second)
			{
				pIronBar->SetIronBarState(eIronBarState::MOVE_DOWN);
			}
		}
	}

	return;
}

void CIronBarMgr::AddIronBar(CGameObject* pGameObject, int iTriggerID)
{
	CIronBar* pIronBar = dynamic_cast<CIronBar*>(pGameObject);

	if (pIronBar)
	{
		//iTriggerID의 값이 추가되지 않은 상태일 closed false 추가
		if (m_mapClosed.find(iTriggerID) == m_mapClosed.end())
		{
			m_mapClosed[iTriggerID] = false;
		}

		m_mapIronBarGroups[iTriggerID].push_back(pIronBar);
	}
}

void CIronBarMgr::Clear()
{
	for (auto& pair : m_mapIronBarGroups)
	{
		for (auto& pIronBar : pair.second)
		{
			Safe_Release(pIronBar);
		}
	}
	m_mapIronBarGroups.clear();
}

bool CIronBarMgr::IsClosed(int iTriggerID)
{
	//없을 경우 기본값 false
	if (m_mapClosed.find(iTriggerID) == m_mapClosed.end())
	{
		return false;
	}

	return m_mapClosed[iTriggerID];
}

void CIronBarMgr::Free()
{
	for (auto& pair : m_mapIronBarGroups)
	{
		for (auto& pIronBar : pair.second)
		{
			Safe_Release(pIronBar);
		}
	}
	m_mapIronBarGroups.clear();
}
