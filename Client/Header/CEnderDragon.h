#pragma once
#include "CDragon.h"
#include "CCollider.h"
#include <vector>
#include <string>
#include "CNetworkPlayer.h"

// Bone count constants per dragon part
constexpr int ENDER_DRAGON_SPINE_COUNT = 9; // body spine
constexpr int ENDER_DRAGON_NECK_COUNT = 5;  // neck
constexpr int ENDER_DRAGON_TAIL_COUNT = 8;  // tail
constexpr int ENDER_DRAGON_WING_COUNT = 6;  // wing segments (per side)

enum class eEnderDragonState
{
	IDLE,        // curled idle, player detection
	ATTACK,      // player tracking
	BREATH,      // breath attack (head lock-on + CVoidFlame spawn)
	CIRCLE_DIVE, // circling + alternating VoidFlame/Beam attack
	TAIL_ATTACK  // rush + tail swing attack
};

// Flight Dynamics
// Accumulate_Forces accumulates forces per frame,
// Integrate_Physics applies F = ma to update velocity/position.
// Level flight: lift == gravity -> fwdSpeed^2 * liftCoeff = mass * gravity
struct DragonFlight
{
	_float fMass = 120.f;              // mass (kg)
	_float fGravity = 12.f;           // gravity acceleration
	_float fLiftCoeff = 2.8f;         // lift coefficient (upward)
	_float fDragCoeff = 0.55f;        // drag coefficient (opposing velocity)
	_float fThrustForce = 320.f;      // downstroke thrust per flap
	_float fMaxSpeed = 38.f;          // maximum velocity
	_float fBankFactor = 3.f;         // turn -> banking sensitivity
	_float fMaxBankAngle = D3DX_PI * 0.38f;
	_float fRestitution = 0.4f;       // collision restitution (0=perfectly inelastic, 1=perfectly elastic)
	_vec3 vAccumForce;                 // accumulated force per frame
};

// Dragon Update Flow:
// 1. Spine root moves toward moveTarget via velocity lerp
// 2. Spine chain: Follow the Leader
// 3. Neck/Head: CCD IK (faces movement direction)
// 4. Tail: Follow the Leader
// 5. Wings: sin-wave flapping
// 6. Post-chain deformation (IDLE curl / TAIL swing)
// 7. Compute BoneMatrix -> SetTransform -> Render

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
	// Dragon control API
	void Set_SpawnPos(const _vec3 vPos);

	void Set_MoveTarget(const _vec3& vTarget) { m_vMoveTarget = vTarget; }
	_vec3 Get_HeadPos() const { return m_Head.vPos; }
	_vec3 Get_SpineRoot() const { return m_Spine[0].vPos; }
	eEnderDragonState Get_State() const { return m_eState; }

	// Rider API
	_vec3 Get_RiderPos() const { return m_Spine[3].vPos + _vec3(0.f, 2.2f, 0.f); }
	_vec3 Get_RiderDir()  const { return m_Spine[0].vDir; }
	bool  Is_Ridden()     const { return m_bRidden; }
	void  Set_Ridden(bool bVal) { m_bRidden = bVal; }
	void  Force_Idle_State();
	void  Void_Breath(bool bActivate);
	// Network remote control API
	void  Set_NetworkControlled(bool b) { m_bNetworkControlled = b; }
	bool  Is_NetworkControlled()  const { return m_bNetworkControlled; }
	void  Force_RootPos(const _vec3& vPos);

	// Dragon-dragon collision accessors
	CCollider* Get_SpineCollider(_int i) const { return (i >= 0 && i < ENDER_DRAGON_SPINE_COUNT) ? m_pSpineCollider[i] : nullptr; }
	_vec3 Get_Velocity() const { return m_vVelocity; }
	void  Add_Impulse(const _vec3& vImpulse) { m_vVelocity += vImpulse; }
	_float Get_Mass() const { return m_Flight.fMass; }

	void Set_Player(CNetworkPlayer* pPlayer) { m_pPlayer = pPlayer; }
	
	int Get_HP() { return m_iHP; };
	int Get_MaxHP() { return m_iMaxHP; };
	
	void Take_Damage(int iDamage);
	
	bool Is_Dead() { return m_bDead; }

	CCollider** Get_SpineCollider() { return m_pSpineCollider; }

private:
	void Check_Hit();

private:
	CNetworkPlayer* m_pPlayer = nullptr;

	int m_iHP = 100;
	int m_iMaxHP = 100;
	bool m_bDead = false;
	
	bool m_bRidden = false;
	bool m_bNetworkControlled = false;

private:
	// Initialization helpers
	HRESULT Init_SpineChain();
	HRESULT Init_NeckAndHead();
	HRESULT Init_TailChain();
	HRESULT Init_WingChains();

	// fW / H / D : cube dimensions
	HRESULT Create_BoneBuffer(DRAGON_BONE& bone,
		_float fW, _float fH, _float fD, const FACE_UV& uv);
	HRESULT Create_FlexBoneBuffer(DRAGON_BONE& bone,
		const MESH& mesh);

	// Input: WASD movement / H,J,K,L state debug / 5-9 effect testing
	void Handle_Input(const _float& fTimeDelta);

	// Finite State Machine
	void Transition_State(eEnderDragonState eNext);
	void Evaluate_Transitions();
	void Update_IDLE(const _float& fTimeDelta);
	void Update_Attack(const _float& fTimeDelta);
	void Update_BREATH(const _float& fTimeDelta);
	void Update_CIRCLE_DIVE(const _float& fTimeDelta);
	void Update_TailAttack(const _float& fTimeDelta);

	// Post-chain deformation (applied after Solve_FollowLeader)
	void Apply_IdleCurlPostChain(const _float& fTimeDelta);
	void Apply_TailSwingPostChain(const _float& fTimeDelta);

	// Collision
	void Update_BoneColliders();
	void Check_BodyCollision();
	void Check_DragonCollision(CEnderDragon* pOther);

	// JSON loading
	HRESULT Load_DragonPatterns(const char* pPath);

	// ImGui debug panel
	void Render_DebugPanel();

	// Helpers
	eEnderDragonState StringToState(const std::string& s) const;
	const char* StateToString(eEnderDragonState e) const;
	_float Compute_HeadToPlayerAngleDeg() const;

private:
	// Inverse Kinematics
	// CCD IK - Cyclic Coordinate Descent
	void Solve_CCD(DRAGON_BONE* pChain, _int iCount,
		const _vec3& vTarget, _int iMaxIter = 10,
		_float fMaxAngle = D3DX_PI * 0.25f);
	// Follow the Leader chain solver
	void Solve_FollowLeader(DRAGON_BONE* pChain, _int iCount);
	// Wing flapping
	void Update_WingFlap(const _float& fTimeDelta);
	void Update_TailSwing(const _float& fTimeDelta);
	// Quaternion utilities
	D3DXQUATERNION DirToQuaternion(const _vec3& vDir);
	_vec3 QuaternionToDir(const D3DXQUATERNION& quaternion);
	// Quaternion interpolation
	void Slerp_NeckChain(DRAGON_BONE* pResult,
		DRAGON_BONE* pCurrent, _int iCount, _float fAlpha);
	// Bone world matrix (Look-At, DX9 Row-Major)
	// vZ = Normalize(bone.vDir), vX = normalize(worldUp x vZ), vY = vZ x vX
	void Compute_BoneMatrix(DRAGON_BONE& bone);
	void Update_ChainMatrices(DRAGON_BONE* pChain, _int iCount);
	void Render_Chain(DRAGON_BONE* pChain, _int iCount);

	// Physics Dynamics
	void Accumulate_Forces(const _float& fTimeDelta);
	// F = ma -> velocity -> position integration
	void Integrate_Physics(const _float& fTimeDelta);
	// Banking: adds roll to spine matrices during turns (visual only)
	void Update_Banking(const _float& fTimeDelta);

	_float DistToPlayer() const;

private:
	CTexture* m_pTextureCom = nullptr;

	// Bone chains
	DRAGON_BONE m_Spine[ENDER_DRAGON_SPINE_COUNT];
	DRAGON_BONE m_Neck[ENDER_DRAGON_NECK_COUNT];
	DRAGON_BONE m_Head;
	DRAGON_BONE m_Tail[ENDER_DRAGON_TAIL_COUNT];
	DRAGON_BONE m_WingL[ENDER_DRAGON_WING_COUNT];
	DRAGON_BONE m_WingR[ENDER_DRAGON_WING_COUNT];

	// Movement
	_vec3 m_vMoveTarget = { 0.f, 0.f, 0.f };   // target position
	_vec3 m_vVelocity = { 0.f, 0.f, 0.f };     // root velocity (lerp acceleration)
	_float m_fMoveSpeed = 28.f;                  // max move speed

	// Wing flapping
	_float m_fWingTimer = 0.f;                   // accumulated time
	_float m_fWingSpeed = 3.2f;                  // flap frequency (rad/sec)
	_float m_fWingAmp = D3DX_PI * 0.38f;        // max flap angle (rad)

	// Dynamics
	DragonFlight m_Flight; // flight dynamics parameters
	_float m_fPrevSinVal = 0.f; // previous sin value for downstroke detection
	_float m_fBankAngle = 0.f;  // banking angle
	_vec3 m_vLookPrev = { 0.f, 0.f, 1.f };

	// FSM
	eEnderDragonState m_eState = eEnderDragonState::IDLE;
	_float m_fStateTimer = 0.f; // time since entering current state
	_int m_iPatrolIndex = 0;

	// Tail attack
	_float m_fTailSwingTimer = 0.f;
	_float m_fTailSwingAmp = D3DX_PI * 0.8f;

	// IDLE: curled idle + player detection
	_float m_fDetectRange     = 100.f;   // player detection range
	//IDLE 복귀, 전투 해제 반경
	_float m_fDisEngageRange = 150.f;
	_float m_fDetectTimer     = 0.f;    // time spent within detection range
	_float m_fDetectThreshold = 0.0f;   // required dwell time for CIRCLE transition
	_float m_fIdleCurlBlend   = 0.f;    // curl blend (0=spread, 1=fully curled)
	
	// CIRCLE: orbit + VoidFlame/Beam alternating attack
	_float m_fCircleAttackTimer  = 0.f;   // current attack phase timer
	_float m_fVoidFlameDuration  = 3.0f;  // VoidFlame phase duration
	_float m_fBeamDuration       = 3.0f;  // Beam phase duration
	_bool  m_bCirclePhaseIsBeam  = false; // current phase (false=VoidFlame, true=Beam)

	// TAIL_ATTACK: rush + tail attack
	_float m_fTailRushSpeed      = 45.f;  // rush speed
	_bool  m_bTailHitApplied     = false; // prevent duplicate hits

	// Input
	_bool  m_bManualControl = false;
	_vec3  m_vInputForward = { 0.f, 0.f, 1.f };
	_vec3  m_vInputRight = { 1.f, 0.f, 0.f };
	_vec3  m_vPlayerPos = { 0.f, 0.f, 0.f };

	// HP
	_float m_fHP    = 100.f;
	_float m_fMaxHP = 100.f;

	//처음 스폰 위치
	_vec3 m_vSpawnPos = {};

	// BREATH state parameters (overwritten after JSON load)
	_float m_fBreathTimer       = 0.f;   // time until next CVoidFlame spawn
	_float m_fBreathDuration    = 3.5f;
	_float m_fBreathDmgPerSec   = 8.f;
	_float m_fBreathConeDeg     = 20.f;

	// CIRCLE_DIVE state parameters (overwritten after JSON load)
	_float m_fCircleAngle     = 0.f;   // orbit angle
	_float m_fCircleRadius    = 30.f;
	_float m_fDiveSpeed       = 42.f;
	_float m_fBankMultiplier  = 1.6f;

	// Spine/Tail/Wing AABB colliders
	CCollider* m_pSpineCollider[ENDER_DRAGON_SPINE_COUNT] = {};
	CCollider* m_pTailCollider[ENDER_DRAGON_TAIL_COUNT]   = {};
	CCollider* m_pWingLCollider[ENDER_DRAGON_WING_COUNT]  = {};

	// Breath flame management
	bool m_bBreathFiring = false;
	std::vector<class CVoidFlame*> m_vecBreathFlames;

	// JSON-based transition rules
	struct TransitionRule
	{
		eEnderDragonState eTo         = eEnderDragonState::IDLE;
		_float fMinStateTime          = -1.f; // -1 = unused
		_float fMaxPlayerDist         = -1.f;
		_float fMinPlayerDist         = -1.f;
		_float fMaxHpRatio            = -1.f;
		_float fMaxHeadAngleDeg       = -1.f;
	};
	// Transition rules per state (5 states)
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
