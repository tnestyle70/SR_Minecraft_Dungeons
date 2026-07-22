# SR Minecraft Dungeons

4인 팀으로 제작한 Minecraft Dungeons 스타일의 C++ 액션 게임 프로젝트입니다. 팀장으로 브랜치·PR 통합을 관리하면서 서버, Ender Dragon 전투, 맵·UI 편집 도구와 렌더링 최적화를 담당했습니다.

[범용 포트폴리오](https://shy-scorpio-3d0.notion.site/39cb8c3c75e280b597c0fd2f7aaacc43)

![Minecraft Dungeons Ender Dragon](https://raw.githubusercontent.com/tnestyle70/Winters_Engine/main/docs/media/dungeons_dragon_4split.gif)

## 핵심 구조

~~~text
Client Input
→ Packet Contract
→ non-blocking select Server / 20 TPS GameLoop
→ State Broadcast
→ Client gameplay, animation and rendering
~~~

- Client: 입력, 게임플레이, 카메라, UI와 DirectX 9 렌더링
- Engine DLL: 게임 오브젝트, 컴포넌트, 리소스와 렌더링 기반
- Server: non-blocking `select` 세션 처리와 20 TPS 게임 루프
- Shared: 클라이언트·서버가 함께 사용하는 패킷 계약
- UI_EDITOR: Dear ImGui 기반 계층·캔버스·저장 도구

## 담당한 범위

- 팀장으로 Git 브랜치·PR 통합과 충돌 조정
- 서버 루프와 패킷 기반 플레이어·보스 상태 동기화
- Ender Dragon의 블록형 날개, 움직임과 전용 동기화 패킷
- ImGui 기반 맵·UI 편집, 피킹과 저장·불러오기 흐름
- 배치 렌더링과 공간 분할 실험을 비교하고 프로젝트 규모에 맞는 경로 선택

## 저장소 근거

- 전체 342커밋 중 `winter77`·`winter` 명의 220커밋
- 서버 틱과 Dragon 권위 상태: `Server/CGameLoop.cpp`, `Server/CGameLoop.h`
- non-blocking 세션 처리: `Server/CServer.cpp`
- 패킷 계약: `Shared/PacketDef.h`
- Ender Dragon 표현: `Client/Code/CEnderDragon.cpp`
- ImGui 편집기: `UI_EDITOR/Editor.cpp`

## 빌드와 실행

1. Windows와 Visual Studio 2022에서 `Frame161.sln`을 엽니다.
2. NuGet 패키지를 복원합니다. DirectX 9의 D3DX 의존성은 `Microsoft.DXSDK.D3DX` 패키지로 연결됩니다.
3. Debug x64로 Client와 Server를 빌드합니다.
4. 저장소 루트의 `RunLocal.bat`으로 로컬 서버와 클라이언트를 실행합니다.

전체 게임 리소스는 저장소에 포함하지 않으므로 동일 화면 재현에는 별도 리소스 복원이 필요합니다.

## 현재 한계

- 교육 과정의 4인 팀 프로젝트이며 상용 서비스 운영 경험으로 표현하지 않습니다.
- `select` 기반 서버와 20 TPS 동기화는 로컬·소규모 협동 범위이며 대규모 서버 구조가 아닙니다.
- DirectX 9 기반의 레거시 렌더링 프로젝트로, 현대 API의 기능이나 성능을 주장하지 않습니다.

## 에셋 고지

원작 IP와 서드파티 에셋의 권리는 각 권리자에게 있습니다. 이 저장소는 학습·포트폴리오 목적의 비상업 코드 공개이며, 공개 GIF는 구현 결과 설명용입니다.
