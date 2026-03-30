# SR_Minecraft_Dungeons — 프레임워크 구조 & 흐름 완전 설명서

> 이 문서는 코드를 직접 보지 않아도 프레임워크 전체를 이해할 수 있도록 작성된 기술 참조서입니다.

---

## 목차

1. [프로젝트 개요](#1-프로젝트-개요)
2. [디렉토리 구조](#2-디렉토리-구조)
3. [핵심 클래스 계층 구조](#3-핵심-클래스-계층-구조)
4. [게임 루프 & 메인 흐름](#4-게임-루프--메인-흐름)
5. [씬(Scene) & 스테이지 시스템](#5-씬scene--스테이지-시스템)
6. [게임 오브젝트 시스템](#6-게임-오브젝트-시스템)
7. [컴포넌트 시스템](#7-컴포넌트-시스템)
8. [렌더링 파이프라인](#8-렌더링-파이프라인)
9. [입력 시스템](#9-입력-시스템)
10. [물리 & 충돌 시스템](#10-물리--충돌-시스템)
11. [UI 시스템](#11-ui-시스템)
12. [매니저 & 싱글톤 목록](#12-매니저--싱글톤-목록)
13. [주요 게임 오브젝트 상세](#13-주요-게임-오브젝트-상세)
14. [이벤트 & 메시지 시스템](#14-이벤트--메시지-시스템)
15. [리소스 관리](#15-리소스-관리)
16. [네트워크 시스템](#16-네트워크-시스템)
17. [사운드 시스템](#17-사운드-시스템)
18. [파티클 시스템](#18-파티클-시스템)
19. [에디터 시스템](#19-에디터-시스템)
20. [디자인 패턴 정리](#20-디자인-패턴-정리)
21. [중요 설계 특성 & 주의사항](#21-중요-설계-특성--주의사항)

---

## 1. 프로젝트 개요

**장르**: 3D 액션 던전 크롤러 (마인크래프트 던전스 클론)
**빌드**: Visual Studio, `Frame161.sln`
**언어**: C++
**렌더러**: DirectX 9 (D3D9)
**입력**: DirectInput
**오디오**: FMOD
**해상도**: 1280×720 (Engine_Macro.h 정의)
**목표 FPS**: 60 (CFrameMgr 게이팅)

**구조 요약**:
- **Engine** 프로젝트: DLL — 씬, 레이어, 게임오브젝트, 컴포넌트, 매니저 등 코어 프레임워크
- **Client** 프로젝트: EXE — 플레이어, 몬스터, 블록, UI 등 게임 로직
- **Server** 프로젝트: 멀티플레이 서버
- **Shared**: 클라이언트·서버 공용 패킷 정의
- **UI_EDITOR**: 레벨 에디터 도구

---

## 2. 디렉토리 구조

```
SR_Minecraft_Dungeons/
├── Base/
│   ├── CBase.h          ← 레퍼런스 카운팅 최상위 베이스 클래스
│   └── CBase.inl
│
├── Engine/
│   ├── Header/
│   │   ├── Engine_Define.h     ← 기본 타입 정의 (_vec3, _mat, _tchar 등)
│   │   ├── Engine_Enum.h       ← 렌더 그룹, 컴포넌트 ID 등 열거형
│   │   ├── Engine_Macro.h      ← WINcx=1280, WINCY=720, Safe_Delete 매크로
│   │   ├── Engine_Struct.h     ← VERTEX 구조체, LIGHTINFO, MATERIALINFO 등
│   │   ├── CGameObject.h       ← 모든 엔티티의 추상 베이스
│   │   ├── CComponent.h        ← 모든 컴포넌트의 추상 베이스
│   │   ├── CScene.h            ← 씬 추상 베이스
│   │   ├── CLayer.h            ← 레이어 (씬 안의 오브젝트 컨테이너)
│   │   ├── CManagement.h       ← 씬 라우터 싱글톤
│   │   ├── CRenderer.h         ← 렌더 큐 싱글톤
│   │   ├── CGraphicDev.h       ← D3D9 디바이스 관리
│   │   ├── CTransform.h        ← 위치/회전/크기 컴포넌트
│   │   ├── CCollider.h         ← AABB/OBB 충돌 컴포넌트
│   │   ├── CTexture.h          ← 텍스처 로드/바인딩
│   │   ├── CVIBuffer.h         ← 버텍스/인덱스 버퍼 추상 베이스
│   │   ├── CRcTex.h            ← 사각형 텍스처 메시 (UI용)
│   │   ├── CCubeTex.h          ← 큐브 텍스처 메시
│   │   ├── CCamera.h           ← 뷰/프로젝션 행렬
│   │   ├── CLight.h            ← 조명 컴포넌트
│   │   ├── CParticleEmitter.h  ← 파티클 시스템
│   │   ├── CDInputMgr.h        ← DirectInput 키보드/마우스
│   │   ├── CTimerMgr.h         ← 델타타임 관리
│   │   ├── CFrameMgr.h         ← FPS 제한
│   │   ├── CSoundMgr.h         ← FMOD 오디오
│   │   ├── CProtoMgr.h         ← 프로토타입 오브젝트 풀
│   │   ├── CFontMgr.h          ← 폰트 렌더링
│   │   └── CLightMgr.h         ← 동적 조명 관리
│   └── Code/
│       └── (각 .cpp 구현 파일)
│
├── Client/
│   ├── Header/
│   │   ├── CMainApp.h          ← 앱 진입점 클래스
│   │   ├── CPlayer.h           ← 플레이어 캐릭터
│   │   ├── CMonster.h          ← 적 AI (좀비/해골/크리퍼/거미)
│   │   ├── CRedStoneGolem.h    ← 보스1: 레드스톤 골렘
│   │   ├── CDragon.h           ← 보스2: 드래곤 (IK 스켈레탈)
│   │   ├── CEnderDragon.h      ← 보스3: 엔더 드래곤
│   │   ├── CAncientGuardian.h  ← 보스4: 에인션트 가디언
│   │   ├── CNPC.h              ← 대화 NPC
│   │   ├── CBlock.h            ← 월드 블록 (복셀)
│   │   ├── CBlockMgr.h         ← 블록 저장소 + 쿼드트리 컬링
│   │   ├── CMonsterMgr.h       ← 몬스터 스폰/경로/트리거
│   │   ├── CTriggerBox.h       ← 활성화 존
│   │   ├── CTriggerBoxMgr.h    ← 트리거 박스 관리
│   │   ├── CIronBar.h          ← 잠긴 문
│   │   ├── CIronBarMgr.h       ← 철문 상태 동기화
│   │   ├── CJumpingTrap.h      ← 발사대 트랩
│   │   ├── CJumpingTrapMgr.h   ← 발사대 트랩 관리
│   │   ├── CPlayerArrow.h      ← 플레이어 화살 발사체
│   │   ├── CArrow.h            ← 몬스터 화살
│   │   ├── CTNT.h              ← 폭발물
│   │   ├── CUI.h               ← UI 위젯 베이스
│   │   ├── CInventoryMgr.h     ← 인벤토리 UI
│   │   ├── CInventorySlot.h    ← 인벤토리 슬롯
│   │   ├── CEquipSlot.h        ← 장비 슬롯
│   │   ├── CHUD.h              ← 체력 표시 HUD
│   │   ├── CDialogueBox.h      ← NPC 대화창
│   │   ├── CCMiniMap.h         ← 미니맵
│   │   ├── CNetworkMgr.h       ← TCP 네트워크 클라이언트
│   │   ├── CRemotePlayer.h     ← 원격 플레이어
│   │   ├── CScreenFX.h         ← 화면 후처리 효과
│   │   ├── CDamageMgr.h        ← 데미지 숫자 UI
│   │   ├── CParticleMgr.h      ← 파티클 이미터 생명주기
│   │   ├── CSceneChanger.h     ← 씬 전환 팩토리
│   │   ├── CStage.h            ← 기본 플레이어블 스테이지
│   │   ├── CJSStage.h / CCYStage.h / CTGStage.h ← 개별 레벨
│   │   ├── CNetworkStage.h     ← 멀티플레이 스테이지
│   │   ├── CEditor.h           ← 에디터 씬
│   │   └── CLoading.h          ← 로딩 화면
│   ├── Code/ (각 .cpp 구현)
│   └── Include/
│       └── Client.cpp          ← wWinMain() 진입점
│
├── Shared/
│   └── PacketDef.h             ← 클라이언트·서버 공용 패킷 구조체
│
└── Reference/                  ← DirectX, FMOD 등 외부 라이브러리 헤더
```

---

## 3. 핵심 클래스 계층 구조

```
CBase  (레퍼런스 카운팅)
  └── CGameObject  (추상 — 모든 엔티티)
        ├── [플레이어]
        │     └── CPlayer
        ├── [몬스터]
        │     ├── CMonster (좀비/해골/크리퍼/거미)
        │     ├── CRedStoneGolem
        │     ├── CDragon
        │     ├── CEnderDragon
        │     └── CAncientGuardian
        ├── [NPC & 원격]
        │     ├── CNPC
        │     └── CRemotePlayer
        ├── [발사체]
        │     ├── CPlayerArrow
        │     ├── CArrow
        │     └── CTNT
        ├── [월드]
        │     ├── CBlock
        │     ├── CTriggerBox
        │     ├── CIronBar
        │     ├── CJumpingTrap
        │     ├── CSkyBox
        │     └── CBackGround
        ├── [이펙트]
        │     ├── CParticleEmitter
        │     ├── CEffect
        │     └── CExplosionLight
        └── [UI]
              ├── CUI  (UI 위젯 베이스)
              │     ├── CInventorySlot
              │     ├── CEquipSlot
              │     └── 기타 UI 위젯
              ├── CHUD
              ├── CDialogueBox
              └── CCMiniMap

CComponent  (추상 — 모든 컴포넌트)
  ├── CTransform     (위치/회전/크기/월드행렬)
  ├── CCollider      (AABB/OBB 충돌)
  ├── CCamera        (뷰/프로젝션 행렬)
  ├── CLight         (D3D 조명 파라미터)
  ├── CTexture       (텍스처 바인딩)
  ├── CParticleEmitter (파티클 생성/풀)
  └── CVIBuffer  (추상 — 버텍스/인덱스 버퍼)
        ├── CRcTex       (사각형 메시, UI용)
        ├── CCubeTex     (큐브 메시)
        ├── CCubeBodyTex (관절 큐브)
        ├── CPlayerBody  (플레이어 파츠)
        ├── CMonsterBody (몬스터 파츠)
        └── ...
```

---

## 4. 게임 루프 & 메인 흐름

### 4-1. 진입점

`Client/Include/Client.cpp → wWinMain()`

```
wWinMain()
 ├─ 윈도우 클래스 등록 + 창 생성 (1280×720)
 ├─ CMainApp::Create()  ← CManagement, CRenderer, CDInputMgr, CSoundMgr 등 초기화
 ├─ CTimerMgr에 타이머 등록
 │    ├─ "Timer_Immediate" → 즉시 델타타임
 │    └─ "Timer_FPS60"     → FPS 측정용
 ├─ CFrameMgr에 "Frame60" 등록 (60FPS 제한)
 └─ 메시지 루프 (PeekMessage 방식)
      while (g_bRun):
          ├─ Windows 메시지 처리
          └─ CFrameMgr::IsPermit("Frame60") 이 true일 때:
               ├─ CMainApp::Update_MainApp(fDeltaTime)
               ├─ CMainApp::LateUpdate_MainApp(fDeltaTime)
               └─ CMainApp::Render_MainApp()
```

### 4-2. Update 흐름 (프레임당 순서)

```
Update_MainApp(fDelta)
 ├─ CDInputMgr::Update_InputDev()         ← 키/마우스 상태 폴링
 └─ CManagement::Update_Scene(fDelta)
      └─ 현재 CScene::Update_Scene(fDelta)
           └─ for each CLayer:
                └─ CLayer::Update_Layer(fDelta)
                     └─ for each CGameObject*:
                          ├─ obj->Update_GameObject(fDelta)
                          │    └─ 파생 클래스 로직 (이동, AI, 애니메이션)
                          │    └─ 컴포넌트 업데이트 (Transform IK, 물리)
                          ├─ 죽은 오브젝트 플래그 확인
                          └─ 죽은 오브젝트 삭제
```

### 4-3. LateUpdate 흐름

```
LateUpdate_MainApp(fDelta)
 └─ CManagement::LateUpdate_Scene(fDelta)
      └─ 현재 CScene::LateUpdate_Scene(fDelta)
           └─ for each CLayer:
                └─ CLayer::LateUpdate_Layer(fDelta)
                     └─ for each CGameObject*:
                          └─ obj->LateUpdate_GameObject(fDelta)
                               ├─ CRenderer에 렌더 그룹 등록
                               ├─ 이동 후 충돌 보정
                               └─ 카메라 행렬 갱신
```

### 4-4. Render 흐름

```
Render_MainApp()
 ├─ D3D9 디바이스 BeginScene / Clear
 ├─ CRenderer::Render_GameObject()
 │    ├─ Render_SkyBox()      ← 하늘 박스
 │    ├─ Render_Priority()    ← 보스, 특수 오브젝트
 │    ├─ Render_Block()       ← 블록 배치 (쿼드트리 컬링)
 │    ├─ Render_NonAlpha()    ← 불투명 오브젝트 (몬스터, 지형)
 │    ├─ Render_Alpha()       ← 반투명 오브젝트 (파티클, 이펙트)
 │    └─ Render_UI()          ← HUD, 인벤토리, 대화창
 ├─ CScene::Render_UI()       ← 씬별 추가 UI
 ├─ CRenderer::Clear_RenderGroup() ← 렌더 큐 초기화
 └─ D3D9 EndScene / Present
```

---

## 5. 씬(Scene) & 스테이지 시스템

### 5-1. CScene (추상 베이스)

```cpp
class CScene : public CBase {
    map<const _tchar*, CLayer*> m_mapLayer;   // 레이어 저장소

    virtual HRESULT Ready_Scene() = 0;           // 초기화 (순수 가상)
    virtual int     Update_Scene(float fDelta);  // 업데이트
    virtual int     LateUpdate_Scene(float fDelta);
    virtual void    Render_Scene();
    virtual void    Render_UI();
};
```

### 5-2. CLayer

```cpp
class CLayer : public CBase {
    list<CGameObject*> m_lstGameObject;  // 오브젝트 리스트

    void Add_GameObject(CGameObject* pObj);
    void Update_Layer(float fDelta);
    void LateUpdate_Layer(float fDelta);
};
```

### 5-3. CManagement (씬 라우터 싱글톤)

```cpp
class CManagement : public CBase {
    CScene* m_pCurrentScene;   // 현재 활성 씬

    HRESULT Set_Scene(CScene* pScene);   // 씬 전환
    int     Update_Scene(float fDelta);
    int     LateUpdate_Scene(float fDelta);
};
```

### 5-4. 씬 종류

| 클래스 | 용도 |
|--------|------|
| **CStage** | 기본 플레이어블 스테이지 |
| **CJSStage** | JS 레벨 (동적 카메라) |
| **CCYStage** | CY 레벨 (동적 카메라) |
| **CTGStage** | TG 레벨 |
| **CNetworkStage** | 멀티플레이 스테이지 |
| **CEditor** | 레벨 에디터 |
| **CLoading** | 비동기 로딩 화면 |

### 5-5. 씬 전환

`CSceneChanger::ChangeScene(eSceneID)` — enum 기반 정적 팩토리
내부적으로 `CManagement::Set_Scene(new CXXXStage())` 호출

### 5-6. 레이어 태그 (일반적)

| 태그 | 내용 |
|------|------|
| `"Layer_Camera"` | 카메라 오브젝트 |
| `"Layer_SkyBox"` | 스카이박스 |
| `"Layer_Block"` | 월드 블록 |
| `"Layer_Player"` | 플레이어 |
| `"Layer_Monster"` | 몬스터 |
| `"Layer_Effect"` | 이펙트/파티클 |
| `"Layer_UI"` | UI 요소 |
| `"Layer_NPC"` | NPC |

---

## 6. 게임 오브젝트 시스템

### 6-1. CGameObject (추상 베이스)

```cpp
class CGameObject : public CBase {
    // 컴포넌트 저장: ID_DYNAMIC(업데이트 필요), ID_STATIC(정적)
    multimap<const _tchar*, CComponent*> m_mapComponent[ID_END];

    bool m_bDead;              // 삭제 플래그
    float m_fViewZ;            // 렌더 정렬용 뷰Z

    // 순수 가상 함수
    virtual HRESULT Ready_GameObj() = 0;
    virtual int     Update_GameObject(float fDelta) = 0;
    virtual int     LateUpdate_GameObject(float fDelta) = 0;
    virtual void    Render_GameObject() = 0;

    // 컴포넌트 관리
    CComponent* Get_Component(const _tchar* pTag, COMPONENTID eID);
    void        Add_Component(const _tchar* pTag, CComponent* pCom, COMPONENTID eID);
};
```

### 6-2. 오브젝트 생명주기

```
1. Create():     정적 팩토리 메서드, new + Ready_GameObj()
2. Update():     매 프레임 로직 (이동, AI, 애니메이션)
3. LateUpdate(): 이동 후 보정, 렌더러 등록
4. Render():     GPU 렌더링 명령
5. 사망 시:      m_bDead = true → CLayer가 다음 프레임에 삭제
6. Free():       소멸자, 컴포넌트 Release()
```

---

## 7. 컴포넌트 시스템

### 7-1. CComponent (추상 베이스)

```cpp
class CComponent : public CBase {
    virtual int    Update_Component(float fDelta);
    virtual int    LateUpdate_Component(float fDelta);
    virtual CComponent* Clone() = 0;   // 풀링용 복사
};
```

### 7-2. 주요 컴포넌트 상세

#### CTransform

```cpp
// 공간 변환 컴포넌트
_vec3 m_vPosition;    // 월드 위치
_vec3 m_vRotation;    // 오일러 각도 (라디안)
_vec3 m_vScale;       // 크기
_mat  m_matWorld;     // 최종 월드 행렬
CTransform* m_pParent; // 부모 트랜스폼 (계층 구조)

// 이동/회전 메서드
void Move_Pos(const _vec3& vDir, float fSpeed, float fDelta);
void Rotation(ROT_DIR eDir, float fAngle);
void Chase_Target(_vec3 vTarget, float fSpeed); // IK 추적
```

#### CCollider

```cpp
// 충돌 컴포넌트
AABB m_aabb;   // Axis-Aligned Bounding Box
OBB  m_obb;    // Oriented Bounding Box

bool IsColliding(const AABB& other);
bool IsColliding_OBB(const OBB& A, const OBB& B);
bool IntersectRay(const _vec3& orig, const _vec3& dir);
_vec3 Resolve(const AABB& other);  // MTV (최소 이동 벡터)
```

#### CTexture

```cpp
// 텍스처 관리
vector<LPDIRECT3DTEXTURE9> m_vecTexture;  // 프레임 배열
UINT m_iCurrentFrame;

void Bind_OnShader(LPD3DXEFFECT pEffect, const char* pName);
void Next_Frame(float fSpeed);  // 애니메이션
```

#### CCamera

```cpp
// 카메라
_vec3 m_vEye, m_vAt, m_vUp;
float m_fFOV, m_fNear, m_fFar;
_mat  m_matView, m_matProj;
```

---

## 8. 렌더링 파이프라인

### 8-1. 렌더 그룹 (Engine_Enum.h)

```cpp
enum RENDERID {
    RENDER_SKYBOX,    // 스카이박스 (가장 먼저)
    RENDER_PRIORITY,  // 우선순위 오브젝트 (보스 등)
    RENDER_TEST,      // 디버그/에디터
    RENDER_BLOCK,     // 블록 배치 (콜백 방식)
    RENDER_NONALPHA,  // 불투명 (몬스터, 지형)
    RENDER_ALPHA,     // 반투명 (파티클, 이펙트)
    RENDER_UI,        // 2D UI (가장 나중)
    RENDER_END
};
```

### 8-2. CRenderer (싱글톤)

```cpp
class CRenderer : public CBase {
    list<CGameObject*> m_RenderGroup[RENDER_END];  // 그룹별 렌더 큐

    // LateUpdate에서 오브젝트가 자신을 등록
    void Add_RenderGroup(RENDERID eID, CGameObject* pObj);

    // Render 단계에서 순서대로 호출
    void Render_SkyBox();
    void Render_Priority();
    void Render_Block();       // 쿼드트리 컬링 후 렌더
    void Render_NonAlpha();
    void Render_Alpha();       // 뷰Z 기준 정렬 후 렌더
    void Render_UI();
    void Clear_RenderGroup();  // 매 프레임 초기화
};
```

### 8-3. 오브젝트가 렌더러에 등록하는 방법

```cpp
// CPlayer::LateUpdate_GameObject() 안에서:
CRenderer::GetInstance()->Add_RenderGroup(RENDER_NONALPHA, this);

// CParticleEmitter::LateUpdate_GameObject() 안에서:
CRenderer::GetInstance()->Add_RenderGroup(RENDER_ALPHA, this);
```

### 8-4. CGraphicDev

```cpp
// D3D9 디바이스 관리 싱글톤
LPDIRECT3DDEVICE9 m_pGraphicDev;

HRESULT Ready_GraphicDev(HWND hWnd, WINMODE eMode, UINT iWinCX, UINT iWinCY);
LPDIRECT3DDEVICE9 Get_GraphicDev();
```

### 8-5. 화면 후처리 (CScreenFX)

```cpp
// 싱글톤, 렌더 타겟 텍스처 기반
void Set_Shake(float fDuration, float fIntensity);  // 화면 흔들림
void Set_Vignette(float fIntensity);                // 비네트 효과
void Set_BreathEffect(float fAmount);               // 호흡 왜곡
void Render_ScreenFX();                             // 후처리 패스 렌더
```

---

## 9. 입력 시스템

### 9-1. CDInputMgr (싱글톤)

```cpp
class CDInputMgr : public CBase {
    BYTE m_byKeyState[256];        // 현재 키 상태
    BYTE m_byPrevKeyState[256];    // 이전 키 상태
    DIMOUSESTATE m_MouseState;     // 마우스 버튼 상태
    LONG m_lMouseX, m_lMouseY, m_lMouseZ;  // 마우스 이동량

    void Update_InputDev();  // 매 프레임 상태 갱신

    // 키보드
    bool Get_DIKeyState(_ubyte byKeyID);         // 키 눌림 중
    bool Get_DIKeyDown(_ubyte byKeyID);          // 키 방금 눌림
    bool Get_DIKeyUp(_ubyte byKeyID);            // 키 방금 뗌

    // 마우스
    bool Get_DIMouseState(MOUSEKEYSTATE eMouse); // 버튼 상태
    LONG Get_DIMouseMove(MOUSEMOVESTATE eMove);  // X/Y/Z 이동량
};
```

### 9-2. 주요 키 바인딩

| 키 | 동작 |
|----|------|
| W/A/S/D | 캐릭터 이동 |
| Space | 점프 |
| LShift | 구르기(롤 닷지) |
| LMB | 근접 공격 |
| RMB | 활 차지/발사 |
| I | 인벤토리 토글 |
| E | 상호작용 |
| F | TNT 던지기 |
| Tab | 미니맵 토글 |

---

## 10. 물리 & 충돌 시스템

### 10-1. 중력 & 속도

```
- 플레이어 중력: -50 (유닛/초²)
- 몬스터 중력: -20
- 점프 초기 속도: +8
- 최대 낙하 속도: -20
- 속도 누적: m_vVelocity (Vector3)
- 매 프레임: m_vVelocity.y += Gravity × fDelta
             Position += m_vVelocity × fDelta
```

### 10-2. 블록 충돌 해결

```
CBlockMgr 가 블록을 BlockPos(x,y,z) 맵으로 관리
→ 이동 후:
   1. 발 아래 레이캐스트 → 바닥 감지
   2. 측면 AABB 검사 → 벽 충돌
   3. MTV(최소 이동 벡터)로 위치 보정
   4. 바닥이 있으면 m_bGround = true, m_vVelocity.y = 0
```

### 10-3. 오브젝트 간 충돌

```
충돌 타입:
- 플레이어 공격 범위 vs 몬스터: m_pAtkColliderCom
- 몬스터 공격 범위 vs 플레이어: 직접 거리 체크
- 화살 vs 오브젝트: AABB 충돌
- 폭발 vs 범위 내 오브젝트: 원형 범위

충돌 결과:
- obj->Take_Damage(int iDamage)  직접 호출
- 이전 프레임 상태 플래그로 1프레임 충돌만 적용:
  m_bPrevMeleeColliding
```

### 10-4. CBlockMgr 쿼드트리

```cpp
// 블록 공간 인덱싱
map<BlockPos, CBlock*> m_mapBlock;  // 전체 블록 저장소
QuadTree m_QuadTree;                // 카메라 컬링용

BlockPos = { int x, int y, int z }  // 정수 좌표계

// 블록 추가/제거
void Add_Block(BlockPos pos, BLOCK_TYPE eType);
void Remove_Block(BlockPos pos);

// 레이캐스트 (블록 배치, 캐릭터 착지)
bool IntersectRay(const _vec3& orig, const _vec3& dir, BlockPos& outPos);
```

---

## 11. UI 시스템

### 11-1. CUI (베이스 위젯)

```cpp
class CUI : public CGameObject {
    CUI* m_pParent;                   // 부모 위젯
    vector<CUI*> m_vecChildren;       // 자식 위젯들

    float m_fX, m_fY;                 // 스크린 좌표
    float m_fUIScale;                  // 크기 스케일
    bool  m_bVisible;                  // 가시성

    // 스크린 스페이스 렌더 (원근 없음)
    void Render_UI(LPDIRECT3DDEVICE9 pDev);
};
```

### 11-2. 인벤토리 시스템

```cpp
// CInventoryMgr (싱글톤)
// 구조:
//   탭 3개: SWORD / ARMOR / BOW
//   탭당 슬롯 12개 (4×3 격자)
//   장비 슬롯 3개: MELEE / ARMOR / RANGED
//   아이템 정보 패널 (선택 아이템 표시)

enum eInventoryTab { TAB_SWORD, TAB_ARMOR, TAB_BOW };
enum eEquipType    { EQUIP_MELEE, EQUIP_ARMOR, EQUIP_RANGED };
enum eSlotState    { SLOT_DEFAULT, SLOT_HOVER, SLOT_CLICK };

// CInventorySlot
ItemData m_tItemData;         // 아이템 데이터 (이름, 데미지, 방어력, 텍스처)
eSlotState m_eState;          // 현재 슬롯 상태
float m_fDblClickTimer;       // 더블클릭 감지 (300ms 윈도우)

// CEquipSlot
eEquipType m_eEquipType;
bool m_bEquipped;

// 이벤트 처리 (CUIInterface)
virtual void Hover();         // 마우스 올렸을 때
virtual void Clicked();       // 클릭했을 때
virtual void Leave();         // 마우스 벗어났을 때
```

### 11-3. CHUD (체력 HUD)

```cpp
// 하트 아이콘 배열로 체력 표시
// CPlayer* 참조 → HP 동기화
// RENDER_UI 그룹에 등록
```

### 11-4. CDialogueBox

```cpp
// NPC와 상호작용 시 표시
// 텍스트 순차 출력 (타자기 효과)
// Space/E 키로 다음 대사 진행
```

---

## 12. 매니저 & 싱글톤 목록

| 클래스 | 패턴 | 역할 |
|--------|------|------|
| **CManagement** | Singleton | 현재 씬으로 Update/Render 라우팅 |
| **CRenderer** | Singleton | 렌더 큐 수집·정렬·렌더링 |
| **CDInputMgr** | Singleton | 키보드/마우스 상태 폴링 |
| **CBlockMgr** | Singleton | 블록 저장(맵), 쿼드트리 컬링, 레이캐스트 |
| **CMonsterMgr** | Singleton | 몬스터 스폰 그룹, 트리거 활성화, 경로 찾기 |
| **CInventoryMgr** | Singleton | 인벤토리 UI 상태, 아이템 장착 |
| **CTriggerBoxMgr** | Singleton | 트리거 존, 문/몬스터/씬전환 연동 |
| **CIronBarMgr** | Singleton | 철문 상태 동기화 |
| **CJumpingTrapMgr** | Singleton | 발사대 트랩 활성화 |
| **CDamageMgr** | Singleton | 데미지 숫자 UI 생성/관리 |
| **CCursorMgr** | Singleton | 커서 UI 렌더링/호버 감지 |
| **CNetworkMgr** | Singleton | TCP 소켓, 패킷 직렬화, 원격 플레이어 동기화 |
| **CScreenFX** | Singleton | 화면 후처리 (흔들림, 비네트, 호흡) |
| **CProtoMgr** | Singleton | 프로토타입 오브젝트 풀 |
| **CParticleMgr** | Singleton | 파티클 이미터 생명주기 |
| **CSoundMgr** | Singleton | FMOD 오디오 채널 관리 |
| **CTimerMgr** | Singleton | 이름별 델타타임 계산 |
| **CFrameMgr** | Singleton | FPS 제한 (프레임 게이팅) |
| **CGraphicDev** | Singleton | D3D9 디바이스 초기화/관리 |
| **CLightMgr** | Singleton | 동적 조명 관리 |
| **CFontMgr** | Singleton | 폰트 로드/텍스트 렌더링 |
| **CEnvironmentMgr** | Singleton | 환경 설정 (안개, 날씨 등) |

---

## 13. 주요 게임 오브젝트 상세

### 13-1. CPlayer (플레이어)

**스탯**
- HP: 100 (기본값, 장비로 증가 가능)
- 이동속도: 10 유닛/초
- 점프 힘: 8
- 근접 데미지: 10 (무기 보너스 +α)
- 활 데미지: 15 × 차지율(0.0~1.0)
- 구르기 속도: 22 유닛/초, 지속 0.5초, 쿨타임 3초

**컴포넌트**
```cpp
CPlayerBody*    m_pBufferCom[PART_END]   // 6개 파츠 (머리, 몸통, 팔×2, 다리×2)
CTexture*       m_pTextureCom            // 스킨 텍스처
CCollider*      m_pColliderCom           // 이동 충돌
CCollider*      m_pAtkColliderCom        // 근접 공격 충돌 (스윙 중만 활성)
CRcTex*         m_pBowBufferCom          // 활 렌더링
CParticleEmitter* m_pFootStepEmitter     // 발걸음 이펙트
```

**공격 시스템**

근접 콤보:
```
스윙1 (우→좌): 0.55초 → 스윙2 (좌→우): 0.55초 → 찌르기: 0.55초
입력 윈도우: 0.6초 이내 다음 공격 입력 시 콤보 연결
```

활 공격:
```
RMB 홀드: 0→2초 차지
RMB 릴리스: 화살 발사 (데미지 = 15 × 차지율)
불꽃화살 변형: 조건 달성 시 폭발 화살
```

**상태 목록**
```
IDLE, WALK, RUN, JUMP, FALL, LAND,
ATTACK1, ATTACK2, ATTACK3,         ← 근접 콤보
BOW_CHARGE, BOW_FIRE,              ← 원거리
ROLL,                               ← 구르기
HIT, DEAD
```

### 13-2. CMonster (적 AI)

**타입**: ZOMBIE, SKELETON, CREEPER, SPIDER

**AI 흐름**
```
1. 탐지: 플레이어와 거리 20 이내 → 추적 시작
2. 경로: A* (블록 그리드, 0.5초마다 재계산)
3. 추적: 2 유닛/초 이동
4. 근접 공격: 2 유닛 이내, 10 데미지
5. 타입별 특수:
   - 해골: 거리 10 이상이면 CArrow 발사
   - 크리퍼: 2 유닛 이내 접근 시 자폭 (12 데미지, AOE)
```

**컴포넌트**
```cpp
CMonsterBody*   m_pBodyCom
CCollider*      m_pColliderCom           // 이동 충돌
CCollider*      m_pAtkColliderCom        // 근접 공격
CCollider*      m_pExplosionColliderCom  // 크리퍼 폭발
CParticleEmitter* m_pDeathEmitter        // 사망 이펙트
vector<BlockPos>  m_vecPath              // A* 경로
```

### 13-3. CRedStoneGolem (보스 1)

**스탯**: 높은 HP 풀

**상태 기계**
```
IDLE → (거리 감지) → WALK → (사정거리) → ATTACK
ATTACK → (스킬 쿨다운) → SKILL
HIT → (무적 시간) → 이전 상태 복귀
DEAD → 사망 애니메이션 → m_bDead = true
```

**관절 시스템**
```
7 파츠: 머리, 몸통, 왼팔, 오른팔, 왼다리, 오른다리, 기타
부모-자식 계층구조로 행렬 계산
상태별 애니메이션 보간
```

### 13-4. CDragon (보스 2 — 복잡한 스켈레탈 IK)

**골격 구조**
```
척추 체인: 7개 뼈 (루트→꼬리)
목: 3개 뼈 (IK 타겟: 플레이어 머리)
꼬리: 6개 뼈 (Follow-Leader 물리)
날개: 좌우 각 4개 (사인파 날갯짓)
```

**역운동학**
```
CCD-IK (Cyclic Coordinate Descent):
- 목이 플레이어를 향해 자연스럽게 구부러짐
- Look-At 행렬 계산 (D3D 행 우선)

Follow-Leader:
- 각 꼬리 뼈가 앞 뼈의 위치를 지연 추적
- 리얼한 꼬리 흔들림

날개:
- sin(time × 속도) × 진폭 으로 Y축 회전
```

**이동**
```
목표 지점을 향한 속도 선형 보간 (Lerp)
순찰 루트: 재생성 포인트 배열
상태: IDLE(순찰), ATTACK(플레이어 추적), TAIL_ATTACK
```

### 13-5. CTriggerBox & 트리거 시스템

```cpp
// CTriggerBox: 플레이어가 진입 시 활성화되는 영역
// CTriggerBoxMgr이 관리

// 트리거 종류:
TRIGGER_MONSTER   → CMonsterMgr::ActivateGroup(int groupID)
TRIGGER_IRONBAR   → CIronBarMgr::Open() / Close()
TRIGGER_SCENE     → CSceneChanger::ChangeScene(eSceneID)
TRIGGER_DIALOGUE  → CDialogueBox::Open(string text)
TRIGGER_PARTICLE  → CParticleMgr::Spawn(eType, pos)
```

### 13-6. CJumpingTrap (발사대 트랩)

```cpp
// 플레이어가 밟으면 위로 튀어오름
// CJumpingTrapMgr이 활성/비활성 상태 관리
// 발사 힘: 상방 벡터 × fLaunchForce
```

---

## 14. 이벤트 & 메시지 시스템

**중앙 이벤트 버스 없음** — 직접 함수 호출 방식

### 14-1. 데미지 전달

```
충돌 감지 → 직접 호출:
monster->Take_Damage(int iDamage)
player->Hit(float fDamage)

Take_Damage 내부:
- m_iHP -= iDamage
- CDamageMgr::SpawnDamageNumber(pos, iDamage)  // 데미지 숫자 UI
- CParticleMgr::Spawn(PARTICLE_HIT, pos)        // 피격 이펙트
- if (m_iHP <= 0) m_bDead = true
```

### 14-2. 충돌 플래그

```cpp
// 동일 충돌이 여러 프레임에 걸쳐 적용되지 않도록:
bool m_bPrevMeleeColliding;  // 이전 프레임 충돌 여부
// 현재 충돌 && !이전 충돌 → 데미지 1번만 적용
```

### 14-3. 씬 전환 트리거

```
CTriggerBox 진입 → CTriggerBoxMgr → CSceneChanger::ChangeScene()
                                    → CManagement::Set_Scene(new Scene)
```

---

## 15. 리소스 관리

### 15-1. 레퍼런스 카운팅 (CBase)

```cpp
class CBase {
    UINT m_dwRefCnt;

    void AddRef();         // 카운트 증가
    UINT Release();        // 카운트 감소, 0이면 delete
    virtual void Free();   // 소멸자 로직 (순수 가상)
};
// 스마트 포인터 미사용, 수동 수명 관리
```

### 15-2. 프로토타입 풀 (CProtoMgr)

```cpp
// 자주 쓰는 오브젝트를 한 번만 생성 후 Clone()
class CProtoMgr {
    map<const _tchar*, CGameObject*> m_mapProto;

    HRESULT Add_ProtoType(const _tchar* pTag, CGameObject* pProto);
    CGameObject* Clone_Proto(const _tchar* pTag);  // 복사본 반환
};

// CComponent::Clone()도 동일 방식으로 동작
```

### 15-3. 파일 저장/로드

```
블록 데이터: CBlockMgr — FILE* 바이너리 직렬화
  형식: [BlockCount] [BlockPos×N] [BlockType×N]

스테이지 데이터: 바이너리 구조체 배열
  MonsterData   { eMonsterType, _vec3 spawnPos, int groupID }
  TriggerBoxData{ _vec3 min, max, eTriggerType, int targetID }
  IronBarData   { _vec3 pos, int linkedTriggerID }

파티클 프리셋: 하드코딩된 팩토리 (ParticleType.h 열거형 기반)
```

### 15-4. 텍스처/메시 캐싱

```
CTexture: 한 번 로드, 여러 오브젝트가 공유
CRcTex: UI 공용 사각형 버퍼
CVIBuffer 파생: 정적 지오메트리는 한 번만 생성
```

---

## 16. 네트워크 시스템

### 16-1. 구조

```
CNetworkMgr (싱글톤) — TCP 클라이언트
CNetworkStage — 멀티플레이 씬
CRemotePlayer — 원격 플레이어 엔티티
PacketDef.h — 패킷 구조체 정의
```

### 16-2. 패킷 종류

```cpp
// PacketDef.h
PKT_C2S_Connect      // 연결 요청
PKT_C2S_Input        // 플레이어 입력 전송
PKT_C2S_Attack       // 공격 이벤트

PKT_S2C_Spawn        // 원격 플레이어 생성
PKT_S2C_Despawn      // 원격 플레이어 삭제
PKT_S2C_StateSnapshot // 위치/회전/상태 동기화
PKT_S2C_Attack       // 원격 플레이어 화살 발사
PKT_S2C_Damage       // 로컬 플레이어 데미지 적용
```

### 16-3. 동기화 방식

```
- 입력 주도 (클라이언트): 입력 즉시 서버 전송
- 서버 권위적: StateSnapshot 수신 시 로컬 상태 덮어쓰기
- CRemotePlayer: 수신된 위치/회전으로 선형 보간(Lerp) 이동
```

---

## 17. 사운드 시스템

### 17-1. CSoundMgr (싱글톤, FMOD 기반)

```cpp
// 채널 분류
enum CHANNELID {
    BGM,     // 배경음악
    EFFECT,  // 효과음
    UI,      // UI 사운드
    WORLD,   // 환경음
    CHANNEL_END
};

void Play_Sound(const TCHAR* pSoundKey, CHANNELID eID);
void Play_BGM(const TCHAR* pSoundKey);
void Stop_Sound(CHANNELID eID);
void Stop_AllSound();
void Set_Volume(CHANNELID eID, float fVolume);
```

---

## 18. 파티클 시스템

### 18-1. CParticleEmitter (컴포넌트 겸 게임오브젝트)

```cpp
// 풀 기반 파티클 관리
struct Particle {
    _vec3 vPosition, vVelocity;
    float fLifeTime, fAge;
    _vec4 vColor;
    float fSize;
    bool  bAlive;
};

UINT m_iPoolSize;             // 파티클 풀 크기
vector<Particle> m_vecPool;   // 파티클 풀

// 프리셋 (ParticleType.h 열거형)
enum eParticlePreset {
    PARTICLE_FOOTSTEP,   // 발걸음 먼지
    PARTICLE_HIT,        // 피격 이펙트
    PARTICLE_DEATH,      // 사망 이펙트
    PARTICLE_EXPLOSION,  // 폭발
    PARTICLE_ARROW_TRAIL,// 화살 궤적
    ...
};
```

### 18-2. CParticleMgr (싱글톤)

```cpp
// 파티클 이미터 생명주기 관리
void Spawn(eParticlePreset eType, const _vec3& vPos);
void Update_Particles(float fDelta);  // 소멸된 이미터 제거
```

---

## 19. 에디터 시스템

### 19-1. CEditor (씬)

```
기능:
- 블록 배치/제거 (레이캐스트 + 우클릭/좌클릭)
- 몬스터 배치 (타입/그룹 ID 지정)
- 트리거 박스 배치 (크기/타입/연결 ID)
- 철문 배치
- 발사대 트랩 배치
- 저장/로드 (FILE* 바이너리)

ImGui 오버레이:
- 블록 타입 선택
- 몬스터 타입/그룹 선택
- 트리거 속성 편집
- 파일 저장/불러오기 버튼
```

---

## 20. 디자인 패턴 정리

| 패턴 | 위치 | 설명 |
|------|------|------|
| **Singleton** | 모든 Manager 클래스 | CRenderer, CDInputMgr, CBlockMgr 등 |
| **Reference Counting** | CBase | AddRef/Release/Free 수동 관리 |
| **Factory Method** | CGameObject::Create() | 정적 팩토리로 생성 |
| **Prototype + Clone** | CProtoMgr, CComponent | 프로토타입 복사로 인스턴스 생성 |
| **Component** | CGameObject ↔ CComponent | 컴포넌트 착탈로 기능 조합 |
| **Render Queue** | CRenderer | 오브젝트를 그룹별 큐에 모아 정렬 후 렌더 |
| **Observer (부분)** | 충돌 콜백, 트리거 | 피격/이벤트 시 직접 함수 호출 |
| **Quad-Tree** | CBlockMgr | 블록 공간 분할 컬링 |
| **A\* Pathfinding** | CMonster | 블록 그리드 기반 경로 탐색 |
| **Finite State Machine** | CMonster, 보스들 | 상태별 Update 분기, Change_State() |
| **Object Pool** | 파티클, 화살, TNT | 잦은 생성/삭제 방지 |
| **CCD-IK** | CDragon | 목/꼬리 역운동학 |
| **Follow-Leader** | CDragon 꼬리 | 체인 본 물리 |
| **Parent-Child Transform** | CTransform | 계층 행렬 계산 |
| **Render Target** | CScreenFX | 텍스처에 렌더 후 후처리 |

---

## 21. 중요 설계 특성 & 주의사항

1. **중앙 이벤트 버스 없음**
   모든 이벤트(데미지, 트리거 등)는 직접 함수 호출. 이벤트 큐나 옵저버 패턴 미사용.

2. **ECS 아님**
   전통적인 OOP 게임 오브젝트 구조. CGameObject가 로직과 데이터를 모두 담음.

3. **스마트 포인터 미사용**
   모든 메모리를 수동 관리. AddRef()/Release() 패턴 + 명시적 Free() 소멸.

4. **렌더 그룹 매 프레임 재등록**
   LateUpdate에서 매 프레임 CRenderer에 자신을 등록, Render 후 Clear.

5. **블록 기반 복셀 세계**
   1×1×1 정수 좌표 블록, AABB 충돌 해결.

6. **멀티플레이: 입력 주도 + 스냅샷**
   클라이언트가 입력 전송, 서버가 StateSnapshot으로 위치 수정.

7. **애니메이션: 시간 기반 보간**
   CPU 스켈레탈 변형 없음. 파츠별 행렬 직접 계산.

8. **파티클: 고정 크기 풀**
   이미터별 고정 크기 풀, 프리셋 팩토리로 생성.

9. **디버그 렌더링**
   충돌 박스 와이어프레임, ImGui 에디터 오버레이.

10. **레이어 태그 = 문자열**
    `map<const _tchar*, CLayer*>` — 레이어는 문자열 태그로 구분.

11. **컴포넌트 ID 구분**
    `ID_DYNAMIC`: 매 프레임 Update 필요한 컴포넌트
    `ID_STATIC`: 정적 버퍼 (메시, 텍스처 등)

12. **씬 전환 시 이전 씬 완전 삭제**
    CManagement::Set_Scene() → 이전 씬 Release() → 새 씬 Ready_Scene()

---

## 빠른 참조: 클래스 → 파일 매핑

| 클래스 | 헤더 파일 |
|--------|----------|
| CBase | Base/CBase.h |
| CGameObject | Engine/Header/CGameObject.h |
| CComponent | Engine/Header/CComponent.h |
| CScene | Engine/Header/CScene.h |
| CLayer | Engine/Header/CLayer.h |
| CManagement | Engine/Header/CManagement.h |
| CRenderer | Engine/Header/CRenderer.h |
| CGraphicDev | Engine/Header/CGraphicDev.h |
| CTransform | Engine/Header/CTransform.h |
| CCollider | Engine/Header/CCollider.h |
| CTexture | Engine/Header/CTexture.h |
| CVIBuffer | Engine/Header/CVIBuffer.h |
| CCamera | Engine/Header/CCamera.h |
| CDInputMgr | Engine/Header/CDInputMgr.h |
| CTimerMgr | Engine/Header/CTimerMgr.h |
| CFrameMgr | Engine/Header/CFrameMgr.h |
| CSoundMgr | Engine/Header/CSoundMgr.h |
| CProtoMgr | Engine/Header/CProtoMgr.h |
| CParticleEmitter | Engine/Header/CParticleEmitter.h |
| CPlayer | Client/Header/CPlayer.h |
| CMonster | Client/Header/CMonster.h |
| CRedStoneGolem | Client/Header/CRedStoneGolem.h |
| CDragon | Client/Header/CDragon.h |
| CBlockMgr | Client/Header/CBlockMgr.h |
| CMonsterMgr | Client/Header/CMonsterMgr.h |
| CTriggerBoxMgr | Client/Header/CTriggerBoxMgr.h |
| CInventoryMgr | Client/Header/CInventoryMgr.h |
| CNetworkMgr | Client/Header/CNetworkMgr.h |
| CScreenFX | Client/Header/CScreenFX.h |
| CSceneChanger | Client/Header/CSceneChanger.h |

---

*이 문서는 SR_Minecraft_Dungeons 프레임워크의 코드를 직접 보지 않고도 구조와 흐름을 이해할 수 있도록 작성되었습니다.*
