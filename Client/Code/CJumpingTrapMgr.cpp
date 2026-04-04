#include "pch.h"
#include "CJumpingTrapMgr.h"
#include "CPlayer.h"
#include "CSoundMgr.h"
#include "CEventBus.h"

IMPLEMENT_SINGLETON(CJumpingTrapMgr)

CJumpingTrapMgr::CJumpingTrapMgr()
{
}

CJumpingTrapMgr::~CJumpingTrapMgr()
{
	Free();
}

HRESULT CJumpingTrapMgr::Ready_JumpingTrapMgr()
{
	//Event Bus 구독, RedStone 죽었는지 아닌지 판단
	CEventBus::GetInstance()->Subscribe(eEventType::BOSS_DEAD, this,
		[this](const FGameEvent& event)
		{
			//TriggerId 9의 Box 생성
			m_mapVisible[9] = true;
		});
	
	return S_OK;
}

_int CJumpingTrapMgr::Update(const _float& fTimeDelta)
{
	for (auto& pair : m_mapJumpingTrapGroups)
	{
		//해당 TriggerID의 visible이 false인 경우 return
		if (!m_mapVisible[pair.first])
			return 0;
		for (auto& pJumpingTrap : pair.second)
		{
			pJumpingTrap->Update_GameObject(fTimeDelta);
		}
	}
	return 0;
}

void CJumpingTrapMgr::LateUpdate(const _float& fTimeDelta)
{
	for (auto& pair : m_mapJumpingTrapGroups)
	{
		for (auto& pJumpingTrap : pair.second)
		{
			pJumpingTrap->LateUpdate_GameObject(fTimeDelta);
		}
	}
}

void CJumpingTrapMgr::Render()
{
	for (auto& pair : m_mapJumpingTrapGroups)
	{
		for (auto& pJumpingTrap : pair.second)
		{
			pJumpingTrap->Render_GameObject();
		}
	}
}

void CJumpingTrapMgr::Activate(int iTriggerID)
{
	auto it = m_mapJumpingTrapGroups.find(iTriggerID);

	if (it == m_mapJumpingTrapGroups.end())
		return;

	//애니메이션 재생
	for (auto* pTrap : it->second)
		pTrap->Set_Active(true);
	//사운드 재생
	CSoundMgr::GetInstance()->PlayEffect(L"Effect/Effect_Jump.wav", 1.f);
	
	//플레이어 점프
	if (iTriggerID == 0)
	{
		m_pPlayer->LaunchByTrap(30.f, eJumpingTrapDir::RIGHT);
	}
	if (iTriggerID == 1)
	{
		m_pPlayer->LaunchByTrap(46.f, eJumpingTrapDir::LEFT);
	}
	if (iTriggerID == 2)
	{
		m_pPlayer->LaunchByTrap(20.f, eJumpingTrapDir::LEFT);
	}
	if (iTriggerID == 3)
	{
		m_pPlayer->LaunchByTrap(20.f, eJumpingTrapDir::LEFT);
	}
	if (iTriggerID == 4)
	{
		m_pPlayer->LaunchByTrap(20.f, eJumpingTrapDir::LEFT);
	}
	if (iTriggerID == 5)
	{
		m_pPlayer->LaunchByTrap(20.f, eJumpingTrapDir::RIGHT);
	}
	if (iTriggerID == 6)
	{
		m_pPlayer->LaunchByTrap(20.f, eJumpingTrapDir::RIGHT);
	}
	if (iTriggerID == 7)
	{
		m_pPlayer->LaunchByTrap(20.f, eJumpingTrapDir::RIGHT);
	}
	if (iTriggerID == 8)
	{
		m_pPlayer->LaunchByTrap(48.f, eJumpingTrapDir::FORWARD);
	}
	if (iTriggerID == 9)
	{
		m_pPlayer->LaunchByTrap(45.f, eJumpingTrapDir::FORWARD);
	}
}

void CJumpingTrapMgr::Add_JumpingTrap(CGameObject * pGameObject, int iTriggerID)
{
	CJumpingTrap* pJumpingTrap = dynamic_cast<CJumpingTrap*>(pGameObject);

	if (pJumpingTrap)
	{
		//iTriggerID의 값이 추가되지 않은 상태일 closed false 추가
		if (m_mapClosed.find(iTriggerID) == m_mapClosed.end())
		{
			m_mapClosed[iTriggerID] = false;
		}

		m_mapJumpingTrapGroups[iTriggerID].push_back(pJumpingTrap);
	}
}

void CJumpingTrapMgr::Clear()
{
	for (auto& [id, vecTraps] : m_mapJumpingTrapGroups)
		for (auto* pTrap : vecTraps)
			Safe_Release(pTrap);

	m_mapJumpingTrapGroups.clear();

	m_pPlayer = nullptr;
}

void CJumpingTrapMgr::Set_Player(CPlayer * pPlayer)
{
	m_pPlayer = pPlayer;
}

void CJumpingTrapMgr::Set_GroupVisible(int iTriggerID, bool bVisible)
{
	m_mapVisible[iTriggerID] = bVisible;
}

bool CJumpingTrapMgr::Is_Active(int iTriggerID)
{
	return false;
}

void CJumpingTrapMgr::Free()
{
	//이벤트 버스 소멸전 해제
	CEventBus::GetInstance()->Unsubscribe(eEventType::BOSS_DEAD, this);
}
