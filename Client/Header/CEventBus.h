#pragma once
#include "CBase.h"
#include "CProtoMgr.h"
#include <functional>
#include <unordered_map>
#include <vector>

enum class eEventType
{
	CURRENCY_COLLECTED,  //재화 획득
	MONSTER_DEAD, //몬스터 처치
	MISSION_ACCEPT, //미션 수락
	MISSION_COMPLETE, //미션 완료
	BOSS_DEAD,
	PLAYER_DEAD,
	PLAYER_KILL,
	GAME_EVENT_END
};

struct FGameEvent
{
	eEventType eType;
	int iValue = 0;
	int iSubType = 0;
	void* pSender = nullptr;
};

//콜백 함수 등록
using EventCallback = std::function<void(const FGameEvent&)>;

class CEventBus : public CBase
{
	DECLARE_SINGLETON(CEventBus)
private:
	explicit CEventBus();
	virtual ~CEventBus();
public:
	//구독 등록
	void Subscribe(eEventType eType, void* pOwner, EventCallback callback);
	//구독 해제
	void Unsubscribe(eEventType eType, void* pOwner);
	//구독 발행
	void Publish(const FGameEvent& event);
private:
	struct FListener
	{
		void* pOwner;
		EventCallback callback;
	};

	std::unordered_map<eEventType, std::vector<FListener>> m_mapCallbacks;
	
private:
	virtual void Free();
};