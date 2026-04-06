#pragma once

// winsock2.h는 windows.h보다 먼저 — framework.h에 WIN32_LEAN_AND_MEAN이 있어서 충돌 없음
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include <map>
#include <vector>
#include "../../Shared/PacketDef.h"

class CRemotePlayer;
class CPlayerArrow;   // Day 9: 네트워크 화살 시각 객체
class CVoidFlame;
class CNetworkPlayer; // Day 9: 로컬 플레이어 HP 갱신용

// =====================================================================
//  CNetworkMgr  —  네트워크 싱글턴 매니저
//  - 서버 Connect / Disconnect
//  - 매 프레임 Update() 에서 패킷 수신 + 파싱
//  - CRemotePlayer 생성 / 제거 / 갱신
//  - CNetworkStage 에서만 Update / Render 호출
// =====================================================================

// 엔더드래곤 동기화 데이터 (클라이언트 보관)
struct EnderDragonSyncData
{
    float fRootX, fRootY, fRootZ;
    float fDirX, fDirZ;
    int   iState;
    int   iHP, iMaxHP;
    int   iTargetPlayerId;
    float fTargetX, fTargetY, fTargetZ;
    float fStateTimer;
    bool  bDead;
    bool  bUpdated = false;  // 이번 프레임에 수신됐는지
};

class CNetworkMgr
{
private:
    CNetworkMgr() = default;
    ~CNetworkMgr();
    CNetworkMgr(const CNetworkMgr&) = delete;
    CNetworkMgr& operator=(const CNetworkMgr&) = delete;

    static CNetworkMgr* m_pInstance;

public:
    static CNetworkMgr* GetInstance();
    static void         DestroyInstance();

    // ── 초기화 / 해제 ──────────────────────────────────────────────────
    // pGraphicDev : CRemotePlayer 생성 시 필요
    bool Connect(LPDIRECT3DDEVICE9 pGraphicDev,
        const char* szIP, int iPort,
        const char* szNickname);
    void Disconnect();
    bool IsConnected() const { return m_bConnected; }

    // ── 프레임 루프 ────────────────────────────────────────────────────
    void Update(float fTimeDelta);  // 패킷 수신 + RemotePlayer Update
    void LateUpdate(float fTimeDelta);  // RemotePlayer LateUpdate
    void Render();                  // RemotePlayer Render

    // ── 송신 ──────────────────────────────────────────────────────────
    // 이동 입력 (드래곤 필드 제거 — DragonSync로 분리)
    void SendInput(float fDirX, float fDirZ, float fRotY, bool bMoving,
        float fX, float fY, float fZ);

    // 드래곤 탑승 동기화 — 탑승 중일 때만 5TPS 호출
    void SendDragonSync(int iDragonIdx,
        float fRootX, float fRootY, float fRootZ,
        float fRotY, bool bOnDragon);

    // 화살 발사 이벤트
    void SendArrow(float fPosX, float fPosY, float fPosZ,
        float fDirX, float fDirY, float fDirZ,
        float fCharge, bool bFirework);

    //Send Void Flame Event
    void SendFlame(float fPosX, float fPosY, float fPosZ,
        float fDirX, float fDirY, float fDirZ,
        float fDamage);

    void SendDamage(int iTargetPlayerId, float fDamage);

    //송신
    void SendEnderDragonDamage(int iDamage);
    //수신
    void On_EnderDragonSync(const PKT_S2C_EnderDragonSync* pPkt);
    
    // ── 상태 조회 ─────────────────────────────────────────────────────
    int  GetMyPlayerId()  const { return m_iMyPlayerId; }
    const char* GetMyNick()      const { return m_szMyNick; }   
    bool IsLoggedIn()     const { return m_iMyPlayerId != -1; }

    // ImGui 디버그 패널용
    int    GetRemoteCount()   const { return static_cast<int>(m_mapRemote.size()); }
    const std::map<int, CRemotePlayer*>& GetRemotePlayers() const { return m_mapRemote; }

    // Day 8: 드래곤 위치 동기화용 — CNetworkStage에서 dragon 제어에 사용
    const std::map<int, CRemotePlayer*>& GetRemoteMap() const { return m_mapRemote; }

    // Day 9: 로컬 플레이어 등록 (피격 HP 갱신용)
    void SetLocalPlayer(CNetworkPlayer* pPlayer) { m_pLocalPlayer = pPlayer; }

    const EnderDragonSyncData& GetEnderDragonSync() const { return m_EnderDragonSync; }
    void ConsumeEnderDragonSync() { m_EnderDragonSync.bUpdated = false; }

private:
    // ── 수신 / 파싱 ───────────────────────────────────────────────────
    void RecvAndProcess();
    void ProcessPacket(const PKT_HEADER* pHdr);

    void On_LoginAck   (const PKT_S2C_LoginAck*       pPkt);
    void On_Spawn      (const PKT_S2C_Spawn*           pPkt);
    void On_Despawn    (const PKT_S2C_Despawn*         pPkt);
    void On_Snapshot   (const PKT_S2C_StateSnapshot*   pPkt);
    void On_Arrow      (const PKT_S2C_Arrow*           pPkt); // S2C_ARROW
    void On_Flame(const PKT_S2C_Flame* pPkt); //S2C_Flame
    void On_DragonSync (const PKT_S2C_DragonSync*      pPkt); // S2C_DRAGON_SYNC
    void On_Damage     (const PKT_S2C_Damage*          pPkt);

    void SendPlayerDead(int iKillerPlayerId);
    void OnPlayerDead(const PKT_S2C_PlayerDead* pPkt);

    // ── 소켓 / 버퍼 ───────────────────────────────────────────────────
    bool Send(const void* pData, int iSize);

    SOCKET  m_hSocket = INVALID_SOCKET;
    bool    m_bConnected = false;

    static constexpr int RECV_BUF_SIZE = 8192;
    char    m_recvBuf[RECV_BUF_SIZE] = {};
    int     m_iRecvLen = 0;

    // ── 게임 상태 ─────────────────────────────────────────────────────
    int     m_iMyPlayerId = -1;
    char    m_szMyNick[32] = {};
    int     m_iSequence = 0;

    //엔더드래곤 상태
    EnderDragonSyncData m_EnderDragonSync;

    LPDIRECT3DDEVICE9             m_pGraphicDev = nullptr;
    std::map<int, CRemotePlayer*> m_mapRemote;  // playerId → CRemotePlayer*

    // Day 9: 네트워크 화살 (원격 플레이어가 발사한 시각 전용 화살)
    std::vector<CPlayerArrow*>    m_vecNetArrows;
    vector<CVoidFlame*> m_vecNetFlames;
    CNetworkPlayer* m_pLocalPlayer = nullptr;  // 피격 HP 갱신용
};

