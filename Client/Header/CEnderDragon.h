#pragma once
#include "CDragon.h"
#include "CCollider.h"
#include <vector>
#include <string>

//드래곤 파트별 뼈 개수 상수
constexpr int ENDER_DRAGON_SPINE_COUNT = 9; //몸통 척추
constexpr int ENDER_DRAGON_NECK_COUNT = 5; //목
constexpr int ENDER_DRAGON_TAIL_COUNT = 8; //꼬리
constexpr int ENDER_DRAGON_WING_COUNT = 6; //날개 한 쪽 세그먼트

enum class eEnderDragonState
{
	IDLE,        //순찰, 대기
	ATTACK,      //플레이어 추적
	BREATH,      //브래스 (머리 락온 + CVoidFlame 스폰)
	CIRCLE_DIVE, //선회 활강
	TAIL_ATTACK  //꼬리 공격
};

//Fight Dynamics
//Accumulate Force가 힘을 누적하고, 
//Intergrate Physics가 F = ma로 속도/위치를 갱신
//level flight 양력 = 중력 -> fFwdSpeed^2 * fLiftCoeff = fMass * fGravity
struct DragonFlight
{
	_float fMass = 120.f;              //질량
	_float fGravity = 12.f;           //중력 가속도
	_float fLiftCoeff = 2.8f;         //양력 계수 -> 위 방향
	_float fDragCoeff = 0.55f;        //항력 계수 -> 역방향
	_float fThrustForce = 320.f;      //다운스트로크 1회 추력
	_float fMaxSpeed = 38.f;          //최대 속도
	_float fBankFactor = 3.f;         //선회 -> 뱅킹 민감도
	_float fMaxBankAngle = D3DX_PI * 0.38f;
	_float fRestitution = 0.4f;       //충돌 반발 계수 (0=완전비탄성, 1=완전탄성)
	_vec3 vAccumForce;                 //매 프레임 누적되는 힘
};

//Dragon - Update Flow
//Spine -> moveTarget 방향으로 속도 lerp 만큼 이동
//Spine -> Follow the Leader(척추 추종)
//Neck Head -> CCD IK(목/머리가 이동 방향을 바라봄)
//Tail -> Follow the Leader(꼬리 추종)
//WingL/R sin 파형 날개짓
//Compute BoneMatrix -> SetTransform -> Render

class CEnderDragon : public CGameObject
{
public:
	CEnderDragon(LPDIRECT3DDEVICE9 pGraphicDev);
	CEnderDragon(const CEnderDragon& rhs);
	virtual ~CEnderDragon();
public:
	virtual HRESULT Ready_GameObject() override;
	virtual _int Update_GameObject(const _float& fTimeDelta)override;
	virtual void LateUpdate_GameObject(const _float& fTimeDelta)override;
	virtual void Render_GameObject();
public:
	//드래곤 제어 Application Programming Interface
	void Set_RootPos(const _vec3 vPos);
	void Set_MoveTarget(const _vec3& vTarget) { m_vMoveTarget = vTarget; }
	_vec3 Get_HeadPos() const { return m_Head.vPos; }
	_vec3 Get_SpineRoot() const { return m_Spine[0].vPos; }
	eEnderDragonState Get_State() const { return m_eState; }

	//탑승자용 추가 API
	_vec3 Get_RiderPos() const { return m_Spine[3].vPos + _vec3(0.f, 2.2f, 0.f); }
	_vec3 Get_RiderDir()  const { return m_Spine[0].vDir; }
	bool  Is_Ridden()     const { return m_bRidden; }
	void  Set_Ridden(bool bVal) { m_bRidden = bVal; }
	void  Force_Idle_State();   // CDragon.cpp에 구현
	//네트워크 원격 제어 API
	void  Set_NetworkControlled(bool b) { m_bNetworkControlled = b; }
	bool  Is_NetworkControlled()  const { return m_bNetworkControlled; }
	void  Force_RootPos(const _vec3& vPos);

private:
	bool m_bRidden = false;
	bool m_bNetworkControlled = false;

private:
	//초기화 헬퍼
	HRESULT Init_SpineChain();
	HRESULT Init_NeckAndHead();
	HRESULT Init_TailChain();
	HRESULT Init_WingChains();

	//fW / H / D : 큐브 크기
	HRESULT Create_BoneBuffer(DRAGON_BONE& bone,
		_float fW, _float fH, _float fD, const FACE_UV& uv);
	HRESULT Create_FlexBoneBuffer(DRAGON_BONE& bone,
		const MESH& mesh);

	//입력 - WASD : 이동 / RTY : 상태 강제 전환
	void Handle_Input(const _float& fTimeDelta);

	//Finite State Machine
	void Transition_State(eEnderDragonState eNext);
	void Evaluate_Transitions();
	void Update_IDLE(const _float& fTimeDelta);
	void Update_Attack(const _float& fTimeDelta);
	void Update_BREATH(const _float& fTimeDelta);
	void Update_CIRCLE_DIVE(const _float& fTimeDelta);
	void Update_TailAttack(const _float& fTimeDelta);

	//충돌
	void Update_BoneColliders();
	void Check_BodyCollision();

	//JSON 로드
	HRESULT Load_DragonPatterns(const char* pPath);

	//ImGui 디버그
	void Render_DebugPanel();

	//헬퍼
	eEnderDragonState StringToState(const std::string& s) const;
	const char* StateToString(eEnderDragonState e) const;
	_float Compute_HeadToPlayerAngleDeg() const;

private:
	//Inverse Kinetics - 추종 알고리즘
	//CCD IK - Cyclic Coordinate Descent
	void Solve_CCD(DRAGON_BONE* pChain, _int iCount,
		const _vec3& vTarget, _int iMaxIter = 10,
		_float fMaxAngle = D3DX_PI * 0.25f);
	//Follow the Leader
	void Solve_FollowLeader(DRAGON_BONE* pChain, _int iCount);
	//날개짓
	void Update_WingFlap(const _float& fTimeDelta);
	void Update_TailSwing(const _float& fTimeDelta);
	//쿼터니언
	D3DXQUATERNION DirToQuaternion(const _vec3& vDir);
	_vec3 QuaternionToDir(const D3DXQUATERNION& quaternion);
	//쿼터니언 보간
	void Slerp_NeckChain(DRAGON_BONE* pResult,
		DRAGON_BONE* pCurrent, _int iCount, _float fAlpha);
	//뼈 월드 행렬 - Look-At 방식, DX9 Row-Major
	//vZ = Normalize(bone.vDir)
	//vX = normalize(worldUp * z)
	//vY = vZ * vX
	void Compute_BoneMatrix(DRAGON_BONE& bone);
	void Update_ChainMatrices(DRAGON_BONE* pChain, _int iCount);
	void Render_Chain(DRAGON_BONE* pChain, _int iCount);

	//Physics Dynamics
	void Accumulate_Forces(const _float& fTimeDelta);
	//F = ma -> 속도 -> 위치 적분
	void Integrate_Physics(const _float& fTimeDelta);
	//선회시 Spine 행렬에 Roll 추가
	void Update_Banking(const _float& fTimeDelta);

	_float DistToPlayer() const;
private:
	CTexture* m_pTextureCom = nullptr;

	//뼈 체인
	DRAGON_BONE m_Spine[ENDER_DRAGON_SPINE_COUNT];
	DRAGON_BONE m_Neck[ENDER_DRAGON_NECK_COUNT];
	DRAGON_BONE m_Head;
	DRAGON_BONE m_Tail[ENDER_DRAGON_TAIL_COUNT];
	DRAGON_BONE m_WingL[ENDER_DRAGON_WING_COUNT];
	DRAGON_BONE m_WingR[ENDER_DRAGON_WING_COUNT];

	//이동
	_vec3 m_vMoveTarget; //목표 위치 
	_vec3 m_vVelocity; //루트 속도(Lerp 가속)
	_float m_fMoveSpeed; //최대 이동 속도

	//날개짓
	_float m_fWingTimer; //누적 시간
	_float m_fWingSpeed; //날개짓 주기(rad/sec)
	_float m_fWingAmp; //날개짓 최대 각도(rad)

	//동역학 Dynamics
	DragonFlight m_Flight; //다이나믹스 움직임을 구현하기 위해서 필요한 변수들 모음
	_float m_fPrevSinVal = 0.f; //다운스트로크 감지용 이전 sin 
	_float m_fBankAngle = 0.f; //뱅킹 각도
	_vec3 m_vLookPrev = { 0.f, 0.f, 1.f };

	//FSM
	eEnderDragonState m_eState = eEnderDragonState::IDLE;
	_float m_fStateTimer = 0.f; //현재 상태 진입 후 누적 시간
	_int m_iPatrolIndex = 0;

	//꼬리 공격
	_float m_fTailSwingTimer = 0.f;
	_float m_fTailSwingAmp = 0.f;

	// 입력
	_bool  m_bManualControl = false;
	_vec3  m_vInputForward;
	_vec3  m_vInputRight;
	_vec3  m_vPlayerPos;

	// HP
	_float m_fHP    = 100.f;
	_float m_fMaxHP = 100.f;

	// BREATH 상태 파라미터 (JSON 로드 후 덮어씀)
	_float m_fBreathTimer       = 0.f;   //다음 CVoidFlame 스폰까지 누적 시간
	_float m_fBreathDuration    = 3.5f;
	_float m_fBreathDmgPerSec   = 8.f;
	_float m_fBreathConeDeg     = 20.f;

	// CIRCLE_DIVE 상태 파라미터 (JSON 로드 후 덮어씀)
	_float m_fCircleAngle     = 0.f;   //선회 궤도 각도
	_float m_fCircleRadius    = 30.f;
	_float m_fDiveSpeed       = 42.f;
	_float m_fBankMultiplier  = 1.6f;

	// 척추/꼬리/날개 충돌 AABB
	CCollider* m_pSpineCollider[ENDER_DRAGON_SPINE_COUNT] = {};
	CCollider* m_pTailCollider[ENDER_DRAGON_TAIL_COUNT]   = {};
	CCollider* m_pWingLCollider[ENDER_DRAGON_WING_COUNT]  = {};

	// 브래스 화염 관리
	std::vector<class CVoidFlame*> m_vecBreathFlames;

	// JSON 기반 전환 규칙
	struct TransitionRule
	{
		eEnderDragonState eTo         = eEnderDragonState::IDLE;
		_float fMinStateTime          = -1.f; //-1 = 미사용
		_float fMaxPlayerDist         = -1.f;
		_float fMinPlayerDist         = -1.f;
		_float fMaxHpRatio            = -1.f;
		_float fMaxHeadAngleDeg       = -1.f;
	};
	// 상태 5개 각각에 대한 전환 규칙 목록
	std::vector<TransitionRule> m_TransRules[5];

private:
	static constexpr _float m_fAttackRange = 20.f;
	static constexpr _float m_fAttackDuration = 6.f;
	static constexpr _float m_fTailAttackDuration = 3.f;
	static constexpr _float m_fTailHPRatio = 0.5f;
	static constexpr _int m_iPatrolCount = 4;
public:
	static CEnderDragon* Create(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual void Free();
};