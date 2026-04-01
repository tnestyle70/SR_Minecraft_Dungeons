#pragma once

#include <winsock2.h>
#include <windows.h>
#include <mswsock.h>
#pragma comment(lib, "Mswsock.lib")

// =====================================================================
//  IO_TYPE  —  IOCP 완료 키 / OverlappedEx 타입 구분
// =====================================================================
enum class IO_TYPE : DWORD
{
    ACCEPT = 0,
    RECV   = 1,
    SEND   = 2,   // Phase 2에서 비동기 Send 전환 시 사용
};

// completionKey 구분값
static constexpr ULONG_PTR KEY_ACCEPT = 0;
static constexpr ULONG_PTR KEY_IO     = 1;

// =====================================================================
//  OverlappedEx  —  RECV / SEND 전용
//  규칙: OVERLAPPED 반드시 오프셋 0
//  GQCS 완료 후 reinterpret_cast<OverlappedEx*>(lpOverlapped) 캐스팅
// =====================================================================
struct OverlappedEx
{
    OVERLAPPED  overlapped;     // 오프셋 0 필수 — OS가 이 주소를 돌려줌
    IO_TYPE     ioType;
    WSABUF      wsaBuf;
    int         iSessionId;
    char        dataBuf[8192];  // 세션당 8KB (MAX_PLAYERS=50 → 총 400KB)
};

// =====================================================================
//  AcceptOverlappedEx  —  ACCEPT 전용 (메모리 낭비 방지를 위해 분리)
//  규칙: OVERLAPPED 반드시 오프셋 0
// =====================================================================
struct AcceptOverlappedEx
{
    OVERLAPPED  overlapped;     // 오프셋 0 필수
    IO_TYPE     ioType;         // = IO_TYPE::ACCEPT
    SOCKET      hAcceptSocket;  // AcceptEx용 미리 생성한 소켓
    // AcceptEx 주소 버퍼: 로컬+원격 각 sizeof(sockaddr_in)+16
    char        addrBuf[(sizeof(sockaddr_in) + 16) * 2];
};
