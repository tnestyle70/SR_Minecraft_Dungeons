#pragma once

#include <thread>
#include <atomic>
#include "../Shared/PacketDef.h"

// =====================================================================
//  CGameLoop  —  서버 게임 틱 루프
//  - 20 TPS (50ms / tick)
//  - 매 틱: 입력 수집 → 위치 계산 → S2C_StateSnapshot 브로드캐스트
//  - CServer::Run() 에서 별도 스레드로 실행
// =====================================================================

struct ServerEnderDragon
{
    float fX = 366.f, fY = 10.f, fZ = -19.f;  // 스폰 위치
    float fDirX = 0.f, fDirZ = 1.f;
    int   iState = 0;  // eEnderDragonState::IDLE
    int   iHP = 100;
    int   iMaxHP = 100;
    int   iTargetPlayerId = -1;
    float fTargetX = 0.f, fTargetY = 0.f, fTargetZ = 0.f;
    float fStateTimer = 0.f;
    bool  bDead = false;
};

class CGameLoop
{
public:
    CGameLoop()  = default;
    ~CGameLoop() { Stop(); }

    void Start();   // 틱 스레드 시작
    void Stop();    // 틱 스레드 종료 (Run() 반환 대기)

    bool IsRunning() const { return m_bRunning; }

    ServerEnderDragon& GetEnderDragon() { return m_EnderDragon; }

private:
    void Run();             // 틱 루프 본체 (스레드)
    void Tick(float fDt);   // 단일 틱 처리

    void UpdateWorld  (float fDt);      // 입력 → 위치 계산
    void BroadcastSnapshot(DWORD dwTick); // S2C_StateSnapshot 전송

    void UpdateEnderDragon(float fDt);
    void BroadcastEnderDragon();
    int FindNearestPlayer(float fX, float fY, float fZ);

    ServerEnderDragon m_EnderDragon;

    // 플레이어 이동 속도 — CPlayer::m_fMoveSpeed 와 동일
    static constexpr float PLAYER_SPEED = 10.f;
    static constexpr int   TICK_RATE    = 20;       // TPS
    static constexpr int   TICK_MS      = 1000 / TICK_RATE; // 50ms

    std::thread       m_thread;
    std::atomic<bool> m_bRunning = false;
};
