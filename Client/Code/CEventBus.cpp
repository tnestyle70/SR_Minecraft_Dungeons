#include "pch.h"
#include "CEventBus.h"

IMPLEMENT_SINGLETON(CEventBus)

CEventBus::CEventBus()
{}

CEventBus::~CEventBus()
{}

//구독 등록
void CEventBus::Subscribe(eEventType eType, void* pOwner, EventCallback callback)
{
	m_mapCallbacks[eType].push_back({pOwner, callback});
}
//구독 해제
void CEventBus::Unsubscribe(eEventType eType, void* pOwner)
{
    auto& vec = m_mapCallbacks[eType];
    vec.erase(
        std::remove_if(vec.begin(), vec.end(),
            [pOwner](const FListener& l) { return l.pOwner == pOwner; }),
        vec.end());
}
//이벤트 발행
void CEventBus::Publish(const FGameEvent & event)
{
    auto it = m_mapCallbacks.find(event.eType);
    //해당 이벤트를 구독하고 있는 구독자들에게 이벤트 전송
    if (it == m_mapCallbacks.end())
        return;

    for (auto& listener : it->second)
    {
        listener.callback(event);
    }
}

void CEventBus::Free()
{
    m_mapCallbacks.clear();
}
