#pragma once

#include <winsock2.h>
#include <windows.h>

#define PROTOCOL_VERSION 2      // v2: dragon packet split, C2S_ARROW
#define MAX_PLAYERS 50
#define SESSION_TIMEOUT_MS 15000  // 15s timeout (after C2S_Input 20TPS verified)

enum PACKET_TYPE : WORD
{
    PKT_NONE = 0,

    C2S_LOGIN        = 1,
    C2S_INPUT        = 2,
    // C2S_ATTACK    = 4  (Deprecated — replaced by C2S_ARROW)
    C2S_DAMAGE       = 5,
    C2S_DRAGON_SYNC  = 6,   // dragon rider only, 5TPS
    C2S_ARROW        = 7,   // arrow fire event (replaces C2S_ATTACK)

    S2C_LOGIN_ACK    = 101,
    S2C_SPAWN        = 102,
    S2C_STATE_SNAPSHOT = 103,
    S2C_DESPAWN      = 104,
    // S2C_ATTACK    = 106  (Deprecated — replaced by S2C_ARROW)
    S2C_DAMAGE       = 107,
    S2C_ARROW        = 108,   // arrow broadcast
    S2C_DRAGON_SYNC  = 109,   // dragon position broadcast
};

#pragma pack(push, 1)

struct PKT_HEADER
{
    WORD wSize;
    WORD wType;
};
static_assert(sizeof(PKT_HEADER) == 4, "PKT_HEADER size mismatch");

struct PKT_C2S_Login
{
    PKT_HEADER header;
    char szNickname[32];
    int iVersion;
};

struct PKT_S2C_LoginAck
{
    PKT_HEADER header;
    int iPlayerId;
    int iStageId;
    bool bSuccess;
    char szMessage[64];
};

struct PlayerSpawnInfo
{
    int iPlayerId;
    float fX, fY, fZ;
    float fRotY;
    char szNickname[32];
};

struct PKT_S2C_Spawn
{
    PKT_HEADER header;
    int iMyPlayerId;
    int iPlayerCount;
    PlayerSpawnInfo players[MAX_PLAYERS];
};

// PKT_C2S_Input — dragon fields removed, position kept (option B)
// removed: bOnDragon(1), iDragonIdx(4), fDragonX/Y/Z(12) -> 17 bytes saved
struct PKT_C2S_Input
{
    PKT_HEADER header;  // 4
    int   iSequence;    // 4
    float fDirX;        // 4
    float fDirZ;        // 4
    float fRotY;        // 4
    bool  bMoving;      // 1
    BYTE  pad[3];       // 3  (explicit padding — alignment under #pragma pack(1))
    DWORD dwTimestamp;  // 4
    float fX;           // 4  client position (server correction)
    float fY;           // 4
    float fZ;           // 4
};                      // total 40 bytes
static_assert(sizeof(PKT_C2S_Input) == 40, "PKT_C2S_Input size mismatch");

// dragon sync — riders only, separate 5TPS
struct PKT_C2S_DragonSync
{
    PKT_HEADER header;        // 4
    int   iDragonIdx;         // 4
    float fRootX, fRootY, fRootZ; // 12
    float fRotY;              // 4
    BYTE  bOnDragon;          // 1
    BYTE  pad[3];             // 3
};  // total 28 bytes
static_assert(sizeof(PKT_C2S_DragonSync) == 28, "PKT_C2S_DragonSync size mismatch");

struct PKT_S2C_DragonSync
{
    PKT_HEADER header;        // 4
    int   iPlayerId;          // 4
    int   iDragonIdx;         // 4
    float fRootX, fRootY, fRootZ; // 12
    float fRotY;              // 4
    BYTE  bOnDragon;          // 1
    BYTE  pad[3];             // 3
};  // total 32 bytes
static_assert(sizeof(PKT_S2C_DragonSync) == 32, "PKT_S2C_DragonSync size mismatch");

struct PlayerState
{
    int iPlayerId;
    float fX, fY, fZ;
    float fRotY;
    int  iState;
    int  iLastSequence;
    bool  bOnDragon;    // on-dragon flag (kept in S2C StateSnapshot)
    int   iDragonIdx;
    float fDragonX, fDragonY, fDragonZ;
};

struct PKT_S2C_StateSnapshot
{
    PKT_HEADER header;
    DWORD dwServerTick;
    int iPlayerCount;
    PlayerState players[MAX_PLAYERS];
};

struct PKT_S2C_Despawn
{
    PKT_HEADER header;
    int iPlayerId;
};

// arrow shot — replaces C2S_ATTACK
struct PKT_C2S_Arrow
{
    PKT_HEADER header;          // 4
    float fPosX, fPosY, fPosZ; // 12  fire position
    float fDirX, fDirY, fDirZ; // 12  normalized direction
    float fCharge;              //  4  charge ratio (0.0~1.0)
    BYTE  bFirework;            //  1  firework arrow flag
    BYTE  pad[3];               //  3
};  // total 36 bytes
static_assert(sizeof(PKT_C2S_Arrow) == 36, "PKT_C2S_Arrow size mismatch");

// arrow broadcast
struct PKT_S2C_Arrow
{
    PKT_HEADER header;          // 4
    int   iPlayerId;            // 4  attacker id
    float fPosX, fPosY, fPosZ; // 12
    float fDirX, fDirY, fDirZ; // 12
    float fCharge;              //  4
    BYTE  bFirework;            //  1
    BYTE  pad[3];               //  3
};  // total 40 bytes
static_assert(sizeof(PKT_S2C_Arrow) == 40, "PKT_S2C_Arrow size mismatch");

// Deprecated — backward compat (server maps recv to C2S_ARROW)
struct PKT_C2S_Attack
{
    PKT_HEADER header;
    float fPosX, fPosY, fPosZ;
    float fDirX, fDirY, fDirZ;
    float fCharge;
    bool  bFirework;
};

struct PKT_S2C_Attack
{
    PKT_HEADER header;
    int   iPlayerId;
    float fPosX, fPosY, fPosZ;
    float fDirX, fDirY, fDirZ;
    float fCharge;
    bool  bFirework;
};

// PVP damage sync
struct PKT_C2S_Damage
{
    PKT_HEADER header;
    int   iTargetPlayerId;
    float fDamage;
};

struct PKT_S2C_Damage
{
    PKT_HEADER header;
    int   iTargetPlayerId;
    int   iAttackerPlayerId;
    float fDamage;
};

#pragma pack(pop)

template <typename T>
inline void FillHeader(T& pkt, PACKET_TYPE type)
{
    pkt.header.wSize = static_cast<WORD>(sizeof(T));
    pkt.header.wType = static_cast<WORD>(type);
}
