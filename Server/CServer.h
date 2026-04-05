#pragma once
#include <winsock2.h>
#include <windows.h>
#include <thread>
#include <atomic>
#include "../Shared/PacketDef.h"
#include "CGameLoop.h"

class CSession;

class CServer
{
public:
    CServer();
    ~CServer();

    bool Init(int iPort);
    void Run();
    void Stop() { m_bRunning = false; }
    bool IsRunning() const { return m_bRunning; }

private:
    void AcceptThread();
    void RecvThread();
    void HandleLogin      (CSession* pSession, const PKT_C2S_Login*       pPkt);
    void HandleInput      (CSession* pSession, const PKT_C2S_Input*       pPkt);
    void HandleArrow      (CSession* pSession, const PKT_C2S_Arrow*       pPkt); // C2S_ARROW
    void HandleFlame(CSession* pSession, const PKT_C2S_Flame* pPkt);
    void HandleDragonSync (CSession* pSession, const PKT_C2S_DragonSync*  pPkt); // C2S_DRAGON_SYNC
    void HandleDamage     (CSession* pSession, const PKT_C2S_Damage*      pPkt);
    //EnderDragon
    void HandleEnderDragonDamage(CSession* pSession, 
        const PKT_C2S_EnderDragonDamage* pPkt);

    // 세션 종료 공통 처리: 스폰 해제 + Despawn 브로드캐스트 + OnDisconnect
    void HandleDisconnect(int iSessId, int iPlayerId, const char* szNickname);

    //NickName Allocation
    static constexpr const char* NICK_POOL[] = { "GB", "JS", "TJ", "CY" };
    std::atomic<int>             m_iNickIndex{ 0 };

    SOCKET m_hListenSocket = INVALID_SOCKET;
    std::atomic<bool> m_bRunning = false;
    std::thread m_acceptThread;
    std::thread m_recvThread;
    DWORD m_dwLastStatusPrint = 0;
    DWORD m_dwLastTimeoutCheck = 0;

    CGameLoop m_gameLoop;
};
