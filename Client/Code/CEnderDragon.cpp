#include "pch.h"
#include "CEnderDragon.h"
#include "CRenderer.h"
#include "CManagement.h"
#include "CDInputMgr.h"
#include "CMonsterMgr.h"
#include "CPlayer.h"
#include "CVoidFlame.h"
#include "CCollider.h"
#include "CScreenFX.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <algorithm>

using json = nlohmann::json;

// 순찰 지점 - 엔더드래곤 크기에 맞게 확장
static const _vec3 s_PatrolPoints[] =
{
	_vec3(30.f, 22.f,  30.f),
	_vec3(-30.f, 24.f,  30.f),
	_vec3(-30.f, 22.f, -30.f),
	_vec3(30.f, 24.f, -30.f)
};

// ─────────────────────────────────────────────────────────────────────────────
// 생성 / 소멸
// ─────────────────────────────────────────────────────────────────────────────
CEnderDragon::CEnderDragon(LPDIRECT3DDEVICE9 pGraphicDev)
	: CGameObject(pGraphicDev)
	, m_vMoveTarget(0.f, 18.f, 20.f)
	, m_vVelocity(0.f, 0.f, 0.f)
	, m_fMoveSpeed(28.f)
	, m_fWingTimer(0.f)
	, m_fWingSpeed(3.2f)
	, m_fWingAmp(D3DX_PI * 0.38f)
	, m_eState(eEnderDragonState::IDLE)
	, m_fStateTimer(0.f)
	, m_iPatrolIndex(0)
	, m_fTailSwingTimer(0.f)
	, m_fTailSwingAmp(D3DX_PI * 0.8f)
	, m_vInputForward(0.f, 0.f, 1.f)
	, m_vInputRight(1.f, 0.f, 0.f)
	, m_vPlayerPos(0.f, 0.f, 0.f)
{
	ZeroMemory(m_Spine, sizeof(m_Spine));
	ZeroMemory(m_Neck, sizeof(m_Neck));
	ZeroMemory(&m_Head, sizeof(m_Head));
	ZeroMemory(m_Tail, sizeof(m_Tail));
	ZeroMemory(m_WingL, sizeof(m_WingL));
	ZeroMemory(m_WingR, sizeof(m_WingR));

	// ZeroMemory 후 쿼터니언 명시 초기화 (0,0,0,0은 유효하지 않은 쿼터니언)
	for (int i = 0; i < DRAGON_SPINE_COUNT; ++i) D3DXQuaternionIdentity(&m_Spine[i].qRot);
	for (int i = 0; i < DRAGON_NECK_COUNT; ++i) D3DXQuaternionIdentity(&m_Neck[i].qRot);
	for (int i = 0; i < DRAGON_TAIL_COUNT; ++i) D3DXQuaternionIdentity(&m_Tail[i].qRot);
	D3DXQuaternionIdentity(&m_Head.qRot);
}

CEnderDragon::CEnderDragon(const CEnderDragon& rhs)
	: CGameObject(rhs)
	, m_vMoveTarget(rhs.m_vMoveTarget)
	, m_vVelocity(rhs.m_vVelocity)
	, m_fMoveSpeed(rhs.m_fMoveSpeed)
	, m_fWingTimer(rhs.m_fWingTimer)
	, m_fWingSpeed(rhs.m_fWingSpeed)
	, m_fWingAmp(rhs.m_fWingAmp)
	, m_eState(rhs.m_eState)
	, m_fStateTimer(rhs.m_fStateTimer)
	, m_iPatrolIndex(rhs.m_iPatrolIndex)
	, m_fTailSwingTimer(rhs.m_fTailSwingTimer)
	, m_fTailSwingAmp(rhs.m_fTailSwingAmp)
	, m_vInputForward(rhs.m_vInputForward)
	, m_vInputRight(rhs.m_vInputRight)
	, m_vPlayerPos(rhs.m_vPlayerPos)
{
	ZeroMemory(m_Spine, sizeof(m_Spine));
	ZeroMemory(m_Neck, sizeof(m_Neck));
	ZeroMemory(&m_Head, sizeof(m_Head));
	ZeroMemory(m_Tail, sizeof(m_Tail));
	ZeroMemory(m_WingL, sizeof(m_WingL));
	ZeroMemory(m_WingR, sizeof(m_WingR));

	for (int i = 0; i < DRAGON_SPINE_COUNT; ++i) D3DXQuaternionIdentity(&m_Spine[i].qRot);
	for (int i = 0; i < DRAGON_NECK_COUNT; ++i) D3DXQuaternionIdentity(&m_Neck[i].qRot);
	for (int i = 0; i < DRAGON_TAIL_COUNT; ++i) D3DXQuaternionIdentity(&m_Tail[i].qRot);
	D3DXQuaternionIdentity(&m_Head.qRot);
}

CEnderDragon::~CEnderDragon() {}

// ─────────────────────────────────────────────────────────────────────────────
// 초기화
// ─────────────────────────────────────────────────────────────────────────────
HRESULT CEnderDragon::Ready_GameObject()
{
	if (FAILED(Init_SpineChain()))   return E_FAIL;
	if (FAILED(Init_NeckAndHead()))  return E_FAIL;
	if (FAILED(Init_TailChain()))    return E_FAIL;
	if (FAILED(Init_WingChains()))   return E_FAIL;

	for (_int i = 0; i < DRAGON_NECK_COUNT; ++i)
		m_Neck[i].qRot = DirToQuaternion(m_Neck[i].vDir);
	m_Head.qRot = DirToQuaternion(m_Head.vDir);

	m_pTextureCom = dynamic_cast<CTexture*>(
		CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_ObsidianPngTexture"));
	if (!m_pTextureCom)
	{
		MSG_BOX("Dragon Texture Clone Failed");
		return E_FAIL;
	}

	// JSON 파라미터 로드 (실패해도 하드코딩 기본값으로 동작)
	Load_DragonPatterns("Data/dragon_patterns.json");

	// 척추 AABB 콜라이더 생성 (Gotcha #3: Create() 직접 사용)
	for (int i = 0; i < ENDER_DRAGON_SPINE_COUNT; ++i)
	{
		m_pSpineCollider[i] = CCollider::Create(m_pGraphicDev,
			_vec3(2.5f, 2.0f, 2.5f), _vec3(0.f, 0.f, 0.f));
		if (!m_pSpineCollider[i]) return E_FAIL;
		m_mapComponent[ID_STATIC].insert({ L"Com_SpineCol", m_pSpineCollider[i] });
	}

	// 꼬리 콜라이더
	for (int i = 0; i < ENDER_DRAGON_TAIL_COUNT; ++i)
	{
		m_pTailCollider[i] = CCollider::Create(m_pGraphicDev,
			_vec3(1.8f, 1.5f, 1.8f), _vec3(0.f, 0.f, 0.f));
		if (!m_pTailCollider[i]) return E_FAIL;
		m_mapComponent[ID_STATIC].insert({ L"Com_TailCol", m_pTailCollider[i] });
	}

	// 날개(좌) 콜라이더
	for (int i = 0; i < ENDER_DRAGON_WING_COUNT; ++i)
	{
		m_pWingLCollider[i] = CCollider::Create(m_pGraphicDev,
			_vec3(2.0f, 0.5f, 1.5f), _vec3(0.f, 0.f, 0.f));
		if (!m_pWingLCollider[i]) return E_FAIL;
		m_mapComponent[ID_STATIC].insert({ L"Com_WingCol", m_pWingLCollider[i] });
	}

	return S_OK;
}

HRESULT CEnderDragon::Create_BoneBuffer(DRAGON_BONE& bone,
	_float fW, _float fH, _float fD, const FACE_UV& uv)
{
	CUBE cube{};
	cube.fWidth = fW;
	cube.fHeight = fH;
	cube.fDepth = fD;
	cube.front = cube.back = cube.top = cube.bottom = cube.left = cube.right = uv;

	bone.pBuffer = CCubeBodyTex::Create(m_pGraphicDev, cube);
	if (!bone.pBuffer) { MSG_BOX("Dragon BoneBuffer Create Failed"); return E_FAIL; }
	D3DXMatrixIdentity(&bone.matWorld);
	return S_OK;
}

HRESULT CEnderDragon::Create_FlexBoneBuffer(DRAGON_BONE& bone, const MESH& mesh)
{
	bone.pBuffer = CFlexibleCubeTex::Create(m_pGraphicDev, mesh);
	if (!bone.pBuffer) { MSG_BOX("Dragon Flexible BoneBuffer Create Failed"); return E_FAIL; }
	D3DXMatrixIdentity(&bone.matWorld);
	return S_OK;
}

// ─────────────────────────────────────────────────────────────────────────────
// Init: 척추 (9개, 기존 7개보다 큼)
// ─────────────────────────────────────────────────────────────────────────────
HRESULT CEnderDragon::Init_SpineChain()
{
	// Index 0 = 목 쪽(앞), 8 = 꼬리 쪽(뒤)
	const _float fW[ENDER_DRAGON_SPINE_COUNT] = { 3.2f, 3.0f, 2.8f, 2.6f, 2.4f, 2.1f, 1.8f, 1.5f, 1.2f };
	const _float fH[ENDER_DRAGON_SPINE_COUNT] = { 2.4f, 2.2f, 2.0f, 1.9f, 1.8f, 1.6f, 1.4f, 1.2f, 0.9f };
	const _float fD[ENDER_DRAGON_SPINE_COUNT] = { 2.4f, 2.2f, 2.0f, 1.9f, 1.8f, 1.6f, 1.4f, 1.2f, 0.9f };
	const _float fBoneLen = 2.0f;

	FACE_UV uv = { 0.f, 0.f, 1.f, 1.f };

	for (int i = 0; i < ENDER_DRAGON_SPINE_COUNT; ++i)
	{
		m_Spine[i].vPos = _vec3(0.f, 0.f, -(_float)i * fBoneLen);
		m_Spine[i].vDir = _vec3(0.f, 0.f, 1.f);
		m_Spine[i].fBoneLen = fBoneLen;
		if (FAILED(Create_BoneBuffer(m_Spine[i], fW[i], fH[i], fD[i], uv))) return E_FAIL;
	}
	return S_OK;
}

// ─────────────────────────────────────────────────────────────────────────────
// Init: 목(5개) + 머리
// ─────────────────────────────────────────────────────────────────────────────
HRESULT CEnderDragon::Init_NeckAndHead()
{
	const _float fNeckLen = 1.6f;
	FACE_UV uv = { 0.f, 0.f, 1.f, 1.f };

	// 목 두께: 루트에서 머리 쪽으로 좁아짐
	const _float fNW[ENDER_DRAGON_NECK_COUNT] = { 1.4f, 1.2f, 1.0f, 0.85f, 0.7f };
	const _float fNH[ENDER_DRAGON_NECK_COUNT] = { 1.3f, 1.1f, 0.95f, 0.8f, 0.65f };

	for (int i = 0; i < ENDER_DRAGON_NECK_COUNT; ++i)
	{
		_vec3 vBase = m_Spine[0].vPos;
		m_Neck[i].vPos = _vec3(
			vBase.x,
			vBase.y + (_float)(i + 1) * 0.5f,
			vBase.z + (_float)(i + 1) * fNeckLen);
		m_Neck[i].vDir = _vec3(0.f, 0.f, 1.f);
		m_Neck[i].fBoneLen = fNeckLen;
		if (FAILED(Create_BoneBuffer(m_Neck[i], fNW[i], fNH[i], 1.4f, uv))) return E_FAIL;
	}

	// 머리 - 목 끝에 붙음
	_vec3 vNeckEnd = m_Neck[ENDER_DRAGON_NECK_COUNT - 1].vPos;
	m_Head.vPos = _vec3(vNeckEnd.x, vNeckEnd.y + 0.5f, vNeckEnd.z + fNeckLen);
	m_Head.vDir = _vec3(0.f, 0.f, 1.f);
	m_Head.fBoneLen = 2.4f;
	if (FAILED(Create_BoneBuffer(m_Head, 2.4f, 1.8f, 2.6f, uv))) return E_FAIL;

	return S_OK;
}

// ─────────────────────────────────────────────────────────────────────────────
// Init: 꼬리 (8개)
// ─────────────────────────────────────────────────────────────────────────────
HRESULT CEnderDragon::Init_TailChain()
{
	_vec3 vBase = m_Spine[ENDER_DRAGON_SPINE_COUNT - 1].vPos;
	const _float fTailLen = 1.4f;
	FACE_UV uv = { 0.f, 0.f, 1.f, 1.f };

	for (int i = 0; i < ENDER_DRAGON_TAIL_COUNT; ++i)
	{
		_float fScale = 1.f - (_float)i * 0.10f;
		m_Tail[i].vPos = _vec3(vBase.x, vBase.y, vBase.z - (_float)(i + 1) * fTailLen);
		m_Tail[i].vDir = _vec3(0.f, 0.f, -1.f);
		m_Tail[i].fBoneLen = fTailLen;
		if (FAILED(Create_BoneBuffer(m_Tail[i],
			fScale * 1.4f, fScale * 1.4f, fScale * 1.4f, uv))) return E_FAIL;
	}
	return S_OK;
}

// ─────────────────────────────────────────────────────────────────────────────
// Init: 날개 (6세그먼트, 칼날 테이퍼)
// ─────────────────────────────────────────────────────────────────────────────
HRESULT CEnderDragon::Init_WingChains()
{
	_vec3 vWingRoot = m_Spine[2].vPos;
	const _float fWingLen = 3.0f;
	const _float hd = fWingLen * 0.5f; // 로컬 Z 절반
	FACE_UV uv = { 0.f, 0.f, 1.f, 1.f };

	// fwd < 0 : +X 방향 날 끝 (상향 칼날)
	// bkd     : 등날 (완만한 테이퍼)
	// hh      : 두께 절반
	struct BladeFace { float fwd, bkd, hh; };

	// outer[i] = inner[i+1] → 세그먼트 간 이음새 연속성 보장
	const BladeFace inner[ENDER_DRAGON_WING_COUNT] = {
		{ -5.6f, 0.80f, 0.18f },  // [0] 루트 (가장 넓음)
		{ -4.8f, 0.68f, 0.15f },  // [1]
		{ -4.0f, 0.56f, 0.12f },  // [2]
		{ -3.0f, 0.42f, 0.09f },  // [3]
		{ -2.0f, 0.28f, 0.06f },  // [4]
		{ -1.0f, 0.16f, 0.03f },  // [5]
	};
	const BladeFace outer[ENDER_DRAGON_WING_COUNT] = {
		{ -4.8f, 0.68f, 0.15f },  // outer[0] = inner[1] ✓
		{ -4.0f, 0.56f, 0.12f },  // outer[1] = inner[2] ✓
		{ -3.0f, 0.42f, 0.09f },  // outer[2] = inner[3] ✓
		{ -2.0f, 0.28f, 0.06f },  // outer[3] = inner[4] ✓
		{ -1.0f, 0.16f, 0.03f },  // outer[4] = inner[5] ✓
		{ -0.05f, 0.06f, 0.01f }, // outer[5] = 칼끝
	};

	for (_int i = 0; i < ENDER_DRAGON_WING_COUNT; ++i)
	{
		_float fOfs = (_float)(i + 1) * fWingLen;
		_float fZFwdR = (_float)i * (-0.5f); // 오른쪽: 끝으로 갈수록 -Z
		_float fZFwdL = (_float)i * (+0.5f); // 왼쪽: 반대 방향

		m_WingL[i].vPos = _vec3(vWingRoot.x - fOfs, vWingRoot.y, vWingRoot.z + fZFwdL);
		m_WingL[i].vDir = _vec3(-1.f, 0.f, 0.f);
		m_WingL[i].fBoneLen = fWingLen;

		m_WingR[i].vPos = _vec3(vWingRoot.x + fOfs, vWingRoot.y, vWingRoot.z + fZFwdR);
		m_WingR[i].vDir = _vec3(1.f, 0.f, 0.f);
		m_WingR[i].fBoneLen = fWingLen;

		// 코너 인덱스:
		// [0..3] tip쪽 (Z=+hd, 날개끝)   [4..7] body쪽 (Z=-hd, 몸통)
		MESH mesh{};
		mesh.front = mesh.back = mesh.top = mesh.bottom = mesh.right = mesh.left = uv;

		const BladeFace& o = outer[i]; // tip쪽 (좁음)
		const BladeFace& n = inner[i]; // body쪽 (넓음)

		mesh.corners[0] = { -o.fwd, +o.hh, +hd };
		mesh.corners[1] = { +o.bkd, +o.hh, +hd };
		mesh.corners[2] = { +o.bkd, -o.hh, +hd };
		mesh.corners[3] = { -o.fwd, -o.hh, +hd };
		mesh.corners[4] = { +n.bkd, +n.hh, -hd };
		mesh.corners[5] = { -n.fwd, +n.hh, -hd };
		mesh.corners[6] = { -n.fwd, -n.hh, -hd };
		mesh.corners[7] = { +n.bkd, -n.hh, -hd };

		// 왼쪽 날개: X 미러 + CCW 와인딩 복원
		MESH meshL = mesh;
		for (auto& c : meshL.corners) c.x = -c.x;
		std::swap(meshL.corners[0], meshL.corners[1]);
		std::swap(meshL.corners[2], meshL.corners[3]);
		std::swap(meshL.corners[4], meshL.corners[5]);
		std::swap(meshL.corners[6], meshL.corners[7]);

		if (FAILED(Create_FlexBoneBuffer(m_WingL[i], meshL))) return E_FAIL;
		if (FAILED(Create_FlexBoneBuffer(m_WingR[i], mesh)))  return E_FAIL;
	}
	return S_OK;
}

// ─────────────────────────────────────────────────────────────────────────────
// Update
// ─────────────────────────────────────────────────────────────────────────────
_int CEnderDragon::Update_GameObject(const _float& fTimeDelta)
{
	// fTimeDelta 클램프: 씬 로드 등 첫 프레임 폭발 방지
	const _float dt = min(fTimeDelta, 0.05f);

	Handle_Input(dt);
	m_fStateTimer += dt;

	// ── 매 프레임 힘 초기화 ──────────────────────────────────────────
	m_Flight.vAccumForce = _vec3(0.f, 0.f, 0.f);

	// ── FSM: 스티어링 힘 누적 + Spine[0].vDir 갱신 ───────────────────
	switch (m_eState)
	{
	case eEnderDragonState::IDLE:        Update_IDLE(dt);        break;
	case eEnderDragonState::ATTACK:      Update_Attack(dt);      break;
	case eEnderDragonState::BREATH:      Update_BREATH(dt);      break;
	case eEnderDragonState::CIRCLE_DIVE: Update_CIRCLE_DIVE(dt); break;
	case eEnderDragonState::TAIL_ATTACK: Update_TailAttack(dt);  break;
	default: break;
	}

	// ── JSON 조건 기반 상태 전환 평가 ─────────────────────────────────
	Evaluate_Transitions();

	// ── 물리 힘 누적: 중력 + 항력 + 양력 ────────────────────────────
	Accumulate_Forces(dt);

	// ── 날개짓: sin 파형 + 다운스트로크 추력 ─────────────────────────
	Update_WingFlap(dt);

	// ── 물리 적분: F=ma → 속도 → 위치 ───────────────────────────────
	Integrate_Physics(dt);

	// ── 뼈 콜라이더 갱신 + 플레이어 충돌 ───────────────────────────────
	Update_BoneColliders();
	Check_BodyCollision();

	// ── 체인 해석 (Spine[0] 위치 확정 후) ────────────────────────────
	Solve_FollowLeader(m_Spine, ENDER_DRAGON_SPINE_COUNT);

	// 목 루트 = Spine[0] 약간 위
	m_Neck[0].vPos = m_Spine[0].vPos + _vec3(0.f, 0.5f, 0.f);

	// 목/머리 CCD + Slerp
	{
		DRAGON_BONE combined[ENDER_DRAGON_NECK_COUNT + 1];
		for (int i = 0; i < ENDER_DRAGON_NECK_COUNT; ++i) combined[i] = m_Neck[i];
		combined[ENDER_DRAGON_NECK_COUNT] = m_Head;

		// BREATH: 머리가 플레이어를 직접 락온
		_vec3 vNeckTarget = (m_eState == eEnderDragonState::BREATH)
			? m_vPlayerPos
			: m_vMoveTarget + _vec3(0.f, 1.5f, 0.f);

		_float fCCDAngle = (m_eState == eEnderDragonState::ATTACK || m_eState == eEnderDragonState::BREATH)
			? D3DX_PI * 0.5f : D3DX_PI * 0.2f;
		_int   iCCDIter = (m_eState == eEnderDragonState::ATTACK || m_eState == eEnderDragonState::BREATH) ? 10 : 6;
		Solve_CCD(combined, ENDER_DRAGON_NECK_COUNT + 1, vNeckTarget, iCCDIter, fCCDAngle);

		DRAGON_BONE current[ENDER_DRAGON_NECK_COUNT + 1];
		for (int i = 0; i < ENDER_DRAGON_NECK_COUNT; ++i) current[i] = m_Neck[i];
		current[ENDER_DRAGON_NECK_COUNT] = m_Head;

		_float fSlerpAlpha = (m_eState == eEnderDragonState::BREATH)
			? dt * 12.f
			: (m_eState == eEnderDragonState::ATTACK) ? dt * 8.f : dt * 3.f;
		Slerp_NeckChain(combined, current, ENDER_DRAGON_NECK_COUNT + 1, fSlerpAlpha);

		for (int i = 0; i < ENDER_DRAGON_NECK_COUNT; ++i) m_Neck[i] = current[i];
		m_Head = current[ENDER_DRAGON_NECK_COUNT];
	}

	// 꼬리 추종
	m_Tail[0].vPos = m_Spine[ENDER_DRAGON_SPINE_COUNT - 1].vPos;
	Solve_FollowLeader(m_Tail, ENDER_DRAGON_TAIL_COUNT);

	// ── 행렬 갱신 ────────────────────────────────────────────────────
	Update_ChainMatrices(m_Spine, ENDER_DRAGON_SPINE_COUNT);
	Update_ChainMatrices(m_Neck, ENDER_DRAGON_NECK_COUNT);
	Compute_BoneMatrix(m_Head);
	Update_ChainMatrices(m_Tail, ENDER_DRAGON_TAIL_COUNT);
	Update_ChainMatrices(m_WingL, ENDER_DRAGON_WING_COUNT);
	Update_ChainMatrices(m_WingR, ENDER_DRAGON_WING_COUNT);

	// ── 뱅킹: 선회 시 Spine Roll 시각 효과 ───────────────────────────
	Update_Banking(dt);

	// ── 브래스 화염 업데이트 / 정리 ──────────────────────────────────
	for (auto iter = m_vecBreathFlames.begin(); iter != m_vecBreathFlames.end();)
	{
		(*iter)->Update_GameObject(dt);
		(*iter)->LateUpdate_GameObject(dt);
		if ((*iter)->Is_Dead())
		{
			Safe_Release(*iter);
			iter = m_vecBreathFlames.erase(iter);
		}
		else
			++iter;
	}

	CRenderer::GetInstance()->Add_RenderGroup(RENDER_NONALPHA, this);
	return CGameObject::Update_GameObject(fTimeDelta);
}

void CEnderDragon::LateUpdate_GameObject(const _float& fTimeDelta)
{
	CGameObject::LateUpdate_GameObject(fTimeDelta);
}

// ─────────────────────────────────────────────────────────────────────────────
// 렌더
// ─────────────────────────────────────────────────────────────────────────────
void CEnderDragon::Render_GameObject()
{
	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

	if (m_pTextureCom) m_pTextureCom->Set_Texture(0);

	Render_Chain(m_Spine, ENDER_DRAGON_SPINE_COUNT);
	Render_Chain(m_Neck, ENDER_DRAGON_NECK_COUNT);

	if (m_Head.pBuffer)
	{
		m_pGraphicDev->SetTransform(D3DTS_WORLD, &m_Head.matWorld);
		m_Head.pBuffer->Render_Buffer();
	}

	Render_Chain(m_Tail, ENDER_DRAGON_TAIL_COUNT);

	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	Render_Chain(m_WingL, ENDER_DRAGON_WING_COUNT);
	Render_Chain(m_WingR, ENDER_DRAGON_WING_COUNT);
	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	// 브래스 화염 렌더링
	for (auto& pFlame : m_vecBreathFlames)
	{
		if (pFlame && !pFlame->Is_Dead())
			pFlame->Render_GameObject();
	}

	// ImGui 디버그 패널
	Render_DebugPanel();
}

void CEnderDragon::Set_RootPos(const _vec3 vPos)
{

}

// ─────────────────────────────────────────────────────────────────────────────
// 동역학 ① — 힘 누적 (중력 / 항력 / 양력)
// ─────────────────────────────────────────────────────────────────────────────
void CEnderDragon::Accumulate_Forces(const _float& fTimeDelta)
{
	// ① 중력 : F = -Y * m * g
	m_Flight.vAccumForce += _vec3(0.f, -(m_Flight.fMass * m_Flight.fGravity), 0.f);

	// ② 항력 : F = -normalize(v) * |v|² * Cd
	//    빠를수록 저항이 급증 → 최대 속도를 자연스럽게 제한
	_float fSpeed = D3DXVec3Length(&m_vVelocity);
	if (fSpeed > 0.01f)
	{
		_vec3 vDragDir = -m_vVelocity;
		D3DXVec3Normalize(&vDragDir, &vDragDir);
		m_Flight.vAccumForce += vDragDir * (fSpeed * fSpeed * m_Flight.fDragCoeff);
	}

	// ③ 양력 : F = +Y * fwdSpeed² * Cl
	//    전진 속도가 빠를수록 위로 뜨는 힘 발생
	//    수평 순항 조건: fwdSpeed² * Cl = m * g
	//    → Cl=2.8, m=120, g=12 → 순항 속도 ≈ sqrt(1440/2.8) ≈ 22.7 m/s
	_vec3 vLook = m_Spine[0].vDir;
	_float fFwdSpeed = D3DXVec3Dot(&m_vVelocity, &vLook);
	if (fFwdSpeed > 0.f)
	{
		_float fLift = fFwdSpeed * fFwdSpeed * m_Flight.fLiftCoeff;
		m_Flight.vAccumForce += _vec3(0.f, fLift, 0.f);
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// 동역학 ② — 물리 적분 (F = ma → 속도 → 위치)
// ─────────────────────────────────────────────────────────────────────────────
void CEnderDragon::Integrate_Physics(const _float& fTimeDelta)
{
	// a = F / m
	_vec3 vAccel = m_Flight.vAccumForce * (1.f / m_Flight.fMass);
	m_vVelocity += vAccel * fTimeDelta;

	// 최대 속도 클램프
	_float fSpeed = D3DXVec3Length(&m_vVelocity);
	if (fSpeed > m_Flight.fMaxSpeed)
	{
		D3DXVec3Normalize(&m_vVelocity, &m_vVelocity);
		m_vVelocity *= m_Flight.fMaxSpeed;
	}

	// 위치 적분
	m_Spine[0].vPos += m_vVelocity * fTimeDelta;

	// 다음 프레임을 위해 누적 힘 초기화 (Accumulate_Forces가 매 프레임 시작에도 초기화함)
	m_Flight.vAccumForce = _vec3(0.f, 0.f, 0.f);
}

// ─────────────────────────────────────────────────────────────────────────────
// 동역학 ③ — 뱅킹 (선회 시 Spine 행렬에 Roll 추가, 순수 시각 효과)
// ─────────────────────────────────────────────────────────────────────────────
void CEnderDragon::Update_Banking(const _float& fTimeDelta)
{
	_vec3 vLookCur = m_Spine[0].vDir;

	_float fSpeed = D3DXVec3Length(&m_vVelocity);

	// 이전 Look 과의 외적 Y성분 = 수평 선회율 (양수: 오른쪽)
	_vec3 vCross;
	D3DXVec3Cross(&vCross, &m_vLookPrev, &vLookCur);
	_float fTurnRate = vCross.y / max(fTimeDelta, 0.001f);

	// 목표 뱅킹 각도 계산 + 클램프
	_float fBankFact = m_Flight.fBankFactor;
	_float fBankMax  = m_Flight.fMaxBankAngle;
	if (m_eState == eEnderDragonState::CIRCLE_DIVE)
	{
		fBankFact *= m_fBankMultiplier;
		fBankMax  *= m_fBankMultiplier;
	}
	_float fTargetBank = -fTurnRate * fBankFact;
	fTargetBank = max(-fBankMax, min(fBankMax, fTargetBank));

	// 뱅킹 각도 부드럽게 보간
	m_fBankAngle += (fTargetBank - m_fBankAngle) * min(fTimeDelta * 4.f, 1.f);
	m_vLookPrev = vLookCur;

	if (fabsf(m_fBankAngle) < 0.001f) return;

	// Spine 행렬에 Roll 적용 (Look 축 기준 회전)
	for (int i = 0; i < ENDER_DRAGON_SPINE_COUNT; ++i)
	{
		_vec3 vLook(m_Spine[i].matWorld._31,
			m_Spine[i].matWorld._32,
			m_Spine[i].matWorld._33);
		_matrix matRoll;
		D3DXMatrixRotationAxis(&matRoll, &vLook, m_fBankAngle);

		// 이동 성분 분리 후 회전 적용, 이동 성분 복원
		_vec3 vPos(m_Spine[i].matWorld._41,
			m_Spine[i].matWorld._42,
			m_Spine[i].matWorld._43);
		m_Spine[i].matWorld._41 = m_Spine[i].matWorld._42 = m_Spine[i].matWorld._43 = 0.f;
		D3DXMatrixMultiply(&m_Spine[i].matWorld, &m_Spine[i].matWorld, &matRoll);
		m_Spine[i].matWorld._41 = vPos.x;
		m_Spine[i].matWorld._42 = vPos.y;
		m_Spine[i].matWorld._43 = vPos.z;
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// FSM — IDLE: 순찰 + 스티어링 힘 누적
// ─────────────────────────────────────────────────────────────────────────────
void CEnderDragon::Update_IDLE(const _float& fTimeDelta)
{
	if (!m_bManualControl && !m_bNetworkControlled)
	{
		_float fDist = D3DXVec3Length(&(m_vMoveTarget - m_Spine[0].vPos));
		if (fDist < 2.5f)
			m_iPatrolIndex = (m_iPatrolIndex + 1) % m_iPatrolCount;
		m_vMoveTarget = s_PatrolPoints[m_iPatrolIndex];
	}

	_vec3 vToTarget = m_vMoveTarget - m_Spine[0].vPos;
	_float fDist = D3DXVec3Length(&vToTarget);

	if (fDist > 1.f)
	{
		_vec3 vDir;
		D3DXVec3Normalize(&vDir, &vToTarget);
		D3DXVec3Normalize(&m_Spine[0].vDir, &vDir);

		// 스티어링 힘: 거리에 비례하되 최대치 클램프
		// m * targetSpeed = 관성 기준 → 적절한 응답성
		_float fSteerMag = min(fDist * m_Flight.fMass * 3.f,
			m_Flight.fMass * m_fMoveSpeed);
		m_Flight.vAccumForce += vDir * fSteerMag;
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// FSM — ATTACK: 플레이어 추적 + 스티어링 힘 누적
// ─────────────────────────────────────────────────────────────────────────────
void CEnderDragon::Update_Attack(const _float& fTimeDelta)
{
	CComponent* pCom = CManagement::GetInstance()->Get_Component(
		ID_DYNAMIC, L"GameLogic_Layer", L"Player", L"Com_Transform");
	if (!pCom) return;

	CTransform* pTransform = dynamic_cast<CTransform*>(pCom);
	if (pTransform) pTransform->Get_Info(INFO_POS, &m_vPlayerPos);

	m_vMoveTarget = m_vPlayerPos + _vec3(0.f, 4.f, 0.f);

	_vec3  vToTarget = m_vMoveTarget - m_Spine[0].vPos;
	_float fDist = D3DXVec3Length(&vToTarget);

	if (fDist > 2.f)
	{
		_vec3 vDir;
		D3DXVec3Normalize(&vDir, &vToTarget);
		D3DXVec3Normalize(&m_Spine[0].vDir, &vDir);

		// 공격 상태: 더 강한 스티어링 (4배)
		_float fSteerMag = min(fDist * m_Flight.fMass * 5.f,
			m_Flight.fMass * m_fMoveSpeed * 1.5f);
		m_Flight.vAccumForce += vDir * fSteerMag;
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// FSM — BREATH: 머리 락온 + CVoidFlame 0.15초 간격 스폰
// ─────────────────────────────────────────────────────────────────────────────
void CEnderDragon::Update_BREATH(const _float& fTimeDelta)
{
	// 플레이어 위치 갱신
	CComponent* pCom = CManagement::GetInstance()->Get_Component(
		ID_DYNAMIC, L"GameLogic_Layer", L"Player", L"Com_Transform");
	if (pCom)
	{
		CTransform* pTrans = dynamic_cast<CTransform*>(pCom);
		if (pTrans) pTrans->Get_Info(INFO_POS, &m_vPlayerPos);
	}

	// 드래곤 본체 호버링 (중력의 일부를 상쇄)
	m_Flight.vAccumForce += _vec3(0.f, m_Flight.fMass * m_Flight.fGravity * 0.5f, 0.f);

	// CVoidFlame 스폰 (0.15초 간격)
	m_fBreathTimer += fTimeDelta;
	if (m_fBreathTimer >= 0.15f)
	{
		m_fBreathTimer = 0.f;
		_vec3 vDir = m_vPlayerPos - m_Head.vPos;
		_float fLen = D3DXVec3Length(&vDir);
		if (fLen > 0.01f)
		{
			D3DXVec3Normalize(&vDir, &vDir);
			float fDamage = m_fBreathDmgPerSec * 0.15f;
			CVoidFlame* pFlame = CVoidFlame::Create(m_pGraphicDev, m_Head.vPos, vDir, fDamage);
			if (pFlame)
				m_vecBreathFlames.push_back(pFlame);
		}
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// FSM — CIRCLE_DIVE: 플레이어 중심 원 궤도 선회 활강
// ─────────────────────────────────────────────────────────────────────────────
void CEnderDragon::Update_CIRCLE_DIVE(const _float& fTimeDelta)
{
	// 플레이어 위치 갱신
	CComponent* pCom = CManagement::GetInstance()->Get_Component(
		ID_DYNAMIC, L"GameLogic_Layer", L"Player", L"Com_Transform");
	if (pCom)
	{
		CTransform* pTrans = dynamic_cast<CTransform*>(pCom);
		if (pTrans) pTrans->Get_Info(INFO_POS, &m_vPlayerPos);
	}

	// 원 궤도 각도 증가 (각속도 = 선속도 / 반경)
	_float fAngSpeed = m_fDiveSpeed / m_fCircleRadius;
	m_fCircleAngle += fAngSpeed * fTimeDelta;

	// 목표 위치: 플레이어 중심 원 궤도 + 고도 유지
	_vec3 vOrbitTarget;
	vOrbitTarget.x = m_vPlayerPos.x + cosf(m_fCircleAngle) * m_fCircleRadius;
	vOrbitTarget.z = m_vPlayerPos.z + sinf(m_fCircleAngle) * m_fCircleRadius;
	vOrbitTarget.y = m_vPlayerPos.y + 8.f;

	// 플레이어보다 낮아지면 상승
	if (m_Spine[0].vPos.y < m_vPlayerPos.y)
		vOrbitTarget.y = m_vPlayerPos.y + 12.f;

	m_vMoveTarget = vOrbitTarget;

	_vec3 vToTarget = m_vMoveTarget - m_Spine[0].vPos;
	_float fDist = D3DXVec3Length(&vToTarget);
	if (fDist > 1.f)
	{
		_vec3 vDir;
		D3DXVec3Normalize(&vDir, &vToTarget);
		D3DXVec3Normalize(&m_Spine[0].vDir, &vDir);

		_float fSteer = min(fDist * m_Flight.fMass * 4.f,
			m_Flight.fMass * m_fDiveSpeed * 1.2f);
		m_Flight.vAccumForce += vDir * fSteer;
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// FSM — TAIL_ATTACK: 꼬리 좌우 스윙 + 콜라이더 활성화
// ─────────────────────────────────────────────────────────────────────────────
void CEnderDragon::Update_TailAttack(const _float& fTimeDelta)
{
	m_fTailSwingTimer += fTimeDelta;

	// 꼬리 스윙 각도 (sin 파형)
	_float fSwingAng = m_fTailSwingAmp * sinf(m_fTailSwingTimer * 2.8f);
	(void)fSwingAng; // 실제 뼈 회전은 Update_TailSwing()이 담당, 여기서는 타이머만 구동

	// 꼬리 AABB 위치 갱신 (충돌 감지용)
	for (int i = 0; i < ENDER_DRAGON_TAIL_COUNT; ++i)
	{
		if (m_pTailCollider[i])
			m_pTailCollider[i]->Update_AABB(m_Tail[i].vPos);
	}

	// 몸체를 제자리에 유지: 약한 제동
	_float fSpeed = D3DXVec3Length(&m_vVelocity);
	if (fSpeed > 0.01f)
	{
		_vec3 vBrake = -m_vVelocity;
		D3DXVec3Normalize(&vBrake, &vBrake);
		m_Flight.vAccumForce += vBrake * (fSpeed * m_Flight.fMass * 0.8f);
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// 날개짓 — sin 파형 + 다운스트로크 추력
// ─────────────────────────────────────────────────────────────────────────────
void CEnderDragon::Update_WingFlap(const _float& fTimeDelta)
{
	m_fWingTimer += fTimeDelta * m_fWingSpeed;

	// ── 다운스트로크 감지 ────────────────────────────────────────────
	// sin 값이 양→음으로 영점 교차 = 날개가 최고점에서 아래로 내려치는 시작
	_float fCurSin = sinf(m_fWingTimer);
	if (m_fPrevSinVal > 0.05f && fCurSin <= 0.05f)
	{
		// 추력: 전진 방향 + 위 방향 성분
		_vec3 vThrust = m_Spine[0].vDir * m_Flight.fThrustForce;
		vThrust.y += m_Flight.fThrustForce * 0.55f; // 상승 성분
		m_Flight.vAccumForce += vThrust;
	}

	m_fPrevSinVal = fCurSin;

	// ── 날개 뼈 위치 계산 ────────────────────────────────────────────
	_matrix matBodyRot = m_Spine[2].matWorld;
	matBodyRot._41 = 0.f; matBodyRot._42 = 0.f; matBodyRot._43 = 0.f;

	_vec3 vWingRoot = m_Spine[2].vPos;

	for (int i = 0; i < ENDER_DRAGON_WING_COUNT; ++i)
	{
		_float fPhase = (_float)i * 0.3f;
		_float fAmplitudeMul = 1.f + (_float)i * 0.15f;
		_float fAngle = m_fWingAmp * fAmplitudeMul * sinf(m_fWingTimer + fPhase);

		_matrix matFlapL, matFlapR;
		D3DXMatrixRotationZ(&matFlapL, fAngle);
		D3DXMatrixRotationZ(&matFlapR, -fAngle);

		_vec3 vPrevL = (i == 0) ? vWingRoot : m_WingL[i - 1].vPos;
		_vec3 vPrevR = (i == 0) ? vWingRoot : m_WingR[i - 1].vPos;

		_vec3 vBaseL = _vec3(-m_WingL[i].fBoneLen, 0.f, 0.f);
		_vec3 vBaseR = _vec3(m_WingR[i].fBoneLen, 0.f, 0.f);

		// 1단계: 로컬 플랩 회전
		D3DXVec3TransformNormal(&vBaseL, &vBaseL, &matFlapL);
		D3DXVec3TransformNormal(&vBaseR, &vBaseR, &matFlapR);

		// 2단계: 몸통 회전 적용
		_vec3 vWorldOffsetL, vWorldOffsetR;
		D3DXVec3TransformNormal(&vWorldOffsetL, &vBaseL, &matBodyRot);
		D3DXVec3TransformNormal(&vWorldOffsetR, &vBaseR, &matBodyRot);

		m_WingL[i].vPos = vPrevL + vWorldOffsetL;
		m_WingR[i].vPos = vPrevR + vWorldOffsetR;

		D3DXVec3Normalize(&m_WingL[i].vDir, &vWorldOffsetL);
		D3DXVec3Normalize(&m_WingR[i].vDir, &vWorldOffsetR);
	}
}

void CEnderDragon::Update_TailSwing(const _float& fTimeDelta)
{
	// TODO
}

// ─────────────────────────────────────────────────────────────────────────────
// IK — CCD
// ─────────────────────────────────────────────────────────────────────────────
void CEnderDragon::Solve_CCD(DRAGON_BONE* pChain, _int iCount,
	const _vec3& vTarget, _int iMaxIter, _float fMaxAngle)
{
	for (_int iter = 0; iter < iMaxIter; ++iter)
	{
		for (_int i = iCount - 2; i >= 0; --i)
		{
			_vec3 vEnd = pChain[iCount - 1].vPos
				+ pChain[iCount - 1].vDir * pChain[iCount - 1].fBoneLen;
			_vec3 vToEnd = vEnd - pChain[i].vPos;
			_vec3 vToTarget = vTarget - pChain[i].vPos;

			if (D3DXVec3Length(&vToEnd) < 0.0001f) continue;
			if (D3DXVec3Length(&vToTarget) < 0.0001f) continue;

			D3DXVec3Normalize(&vToEnd, &vToEnd);
			D3DXVec3Normalize(&vToTarget, &vToTarget);

			_float fDot = max(-1.f, min(1.f, D3DXVec3Dot(&vToEnd, &vToTarget)));
			_float fAngle = min(acosf(fDot), fMaxAngle);

			_vec3 vAxis;
			D3DXVec3Cross(&vAxis, &vToEnd, &vToTarget);
			if (D3DXVec3Length(&vAxis) < 0.001f) continue;
			D3DXVec3Normalize(&vAxis, &vAxis);

			_matrix matRot;
			D3DXMatrixRotationAxis(&matRot, &vAxis, fAngle);

			for (_int j = i + 1; j < iCount; ++j)
			{
				_vec3 vRel = pChain[j].vPos - pChain[i].vPos;
				D3DXVec3TransformCoord(&vRel, &vRel, &matRot);
				pChain[j].vPos = pChain[i].vPos + vRel;
				D3DXVec3TransformNormal(&pChain[j].vDir, &pChain[j].vDir, &matRot);
				D3DXVec3Normalize(&pChain[j].vDir, &pChain[j].vDir);
			}
		}
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// IK — Follow the Leader
// ─────────────────────────────────────────────────────────────────────────────
void CEnderDragon::Solve_FollowLeader(DRAGON_BONE* pChain, _int iCount)
{
	for (int i = 1; i < iCount; ++i)
	{
		_vec3 vDir = pChain[i].vPos - pChain[i - 1].vPos;
		_float fLen = D3DXVec3Length(&vDir);

		if (fLen < 0.001f)
			vDir = pChain[i - 1].vDir * -1.f;
		else
			D3DXVec3Normalize(&vDir, &vDir);

		pChain[i].vPos = pChain[i - 1].vPos + vDir * pChain[i].fBoneLen;
		pChain[i].vDir = vDir * -1.f;
	}
}
// ─────────────────────────────────────────────────────────────────────────────
// 쿼터니언 유틸
// ─────────────────────────────────────────────────────────────────────────────
D3DXQUATERNION CEnderDragon::DirToQuaternion(const _vec3& vDir)
{
	_vec3 vZ = vDir;
	if (D3DXVec3Length(&vZ) < 0.0001f)
	{
		D3DXQUATERNION q; D3DXQuaternionIdentity(&q); return q;
	}
	D3DXVec3Normalize(&vZ, &vZ);

	_vec3 vWorldUp(0.f, 1.f, 0.f);
	if (fabsf(D3DXVec3Dot(&vZ, &vWorldUp)) > 0.99f)
		vWorldUp = _vec3(1.f, 0.f, 0.f);

	_vec3 vX, vY;
	D3DXVec3Cross(&vX, &vWorldUp, &vZ); D3DXVec3Normalize(&vX, &vX);
	D3DXVec3Cross(&vY, &vZ, &vX);       D3DXVec3Normalize(&vY, &vY);

	_matrix mat; D3DXMatrixIdentity(&mat);
	mat._11 = vX.x; mat._12 = vX.y; mat._13 = vX.z;
	mat._21 = vY.x; mat._22 = vY.y; mat._23 = vY.z;
	mat._31 = vZ.x; mat._32 = vZ.y; mat._33 = vZ.z;

	D3DXQUATERNION q;
	D3DXQuaternionRotationMatrix(&q, &mat);
	D3DXQuaternionNormalize(&q, &q);
	return q;
}

_vec3 CEnderDragon::QuaternionToDir(const D3DXQUATERNION& quat)
{
	_matrix mat;
	D3DXMatrixRotationQuaternion(&mat, &quat);
	return _vec3(mat._31, mat._32, mat._33);
}

void CEnderDragon::Slerp_NeckChain(DRAGON_BONE* pResult,
	DRAGON_BONE* pCurrent, _int iCount, _float fAlpha)
{
	for (int i = 0; i < iCount; ++i)
	{
		D3DXQUATERNION qTarget = DirToQuaternion(pResult[i].vDir);
		if (D3DXQuaternionDot(&pCurrent[i].qRot, &qTarget) < 0.f)
			qTarget = -qTarget;

		D3DXQUATERNION qResult;
		D3DXQuaternionSlerp(&qResult, &pCurrent[i].qRot, &qTarget, fAlpha);
		D3DXQuaternionNormalize(&qResult, &qResult);

		pCurrent[i].qRot = qResult;
		pCurrent[i].vDir = QuaternionToDir(qResult);

		if (i > 0)
			pCurrent[i].vPos = pCurrent[i - 1].vPos
			+ pCurrent[i].vDir * pCurrent[i].fBoneLen;
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// 뼈 행렬 계산 (Look-At 방식, DX9 Row-Major)
// ─────────────────────────────────────────────────────────────────────────────
void CEnderDragon::Compute_BoneMatrix(DRAGON_BONE& bone)
{
	_vec3 vZ = bone.vDir;
	if (D3DXVec3Length(&vZ) < 0.001f)
	{
		D3DXMatrixTranslation(&bone.matWorld,
			bone.vPos.x, bone.vPos.y, bone.vPos.z);
		return;
	}
	D3DXVec3Normalize(&vZ, &vZ);

	_vec3 vWorldUp(0.f, 1.f, 0.f);
	if (fabsf(D3DXVec3Dot(&vZ, &vWorldUp)) > 0.99f)
		vWorldUp = _vec3(1.f, 0.f, 0.f);

	_vec3 vX, vY;
	D3DXVec3Cross(&vX, &vWorldUp, &vZ); D3DXVec3Normalize(&vX, &vX);
	D3DXVec3Cross(&vY, &vZ, &vX);       D3DXVec3Normalize(&vY, &vY);

	bone.matWorld._11 = vX.x; bone.matWorld._12 = vX.y; bone.matWorld._13 = vX.z; bone.matWorld._14 = 0.f;
	bone.matWorld._21 = vY.x; bone.matWorld._22 = vY.y; bone.matWorld._23 = vY.z; bone.matWorld._24 = 0.f;
	bone.matWorld._31 = vZ.x; bone.matWorld._32 = vZ.y; bone.matWorld._33 = vZ.z; bone.matWorld._34 = 0.f;
	bone.matWorld._41 = bone.vPos.x;
	bone.matWorld._42 = bone.vPos.y;
	bone.matWorld._43 = bone.vPos.z;
	bone.matWorld._44 = 1.f;
}

void CEnderDragon::Update_ChainMatrices(DRAGON_BONE* pChain, _int iCount)
{
	for (int i = 0; i < iCount; ++i) Compute_BoneMatrix(pChain[i]);
}

void CEnderDragon::Render_Chain(DRAGON_BONE* pChain, _int iCount)
{
	for (int i = 0; i < iCount; ++i)
	{
		if (!pChain[i].pBuffer) continue;
		m_pGraphicDev->SetTransform(D3DTS_WORLD, &pChain[i].matWorld);
		pChain[i].pBuffer->Render_Buffer();
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// FSM 전환
// ─────────────────────────────────────────────────────────────────────────────
void CEnderDragon::Transition_State(eEnderDragonState eNext)
{
	if (m_eState == eNext) return;

	// BREATH 상태 이탈 시 VoidFlame 화면 효과 끄기
	if (m_eState == eEnderDragonState::BREATH)
	{
		CScreenFX* pFX = CScreenFX::GetInstance();
		if (pFX) pFX->Trigger_VoidFlame(false);
	}

	m_eState = eNext;
	m_fStateTimer = 0.f;

	switch (eNext)
	{
	case eEnderDragonState::IDLE:
		m_fWingSpeed = 2.8f;
		m_fWingAmp = D3DX_PI * 0.32f;
		m_fMoveSpeed = 22.f;
		m_vMoveTarget = s_PatrolPoints[m_iPatrolIndex];
		break;
	case eEnderDragonState::ATTACK:
		m_fWingSpeed = 5.5f;
		m_fWingAmp = D3DX_PI * 0.48f;
		m_fMoveSpeed = 32.f;
		break;
	case eEnderDragonState::BREATH:
	{
		m_fBreathTimer = 0.f;
		m_fWingSpeed = 3.5f;
		m_fWingAmp = D3DX_PI * 0.35f;
		m_fMoveSpeed = 8.f;
		// 화면 VoidFlame 효과 시작
		CScreenFX* pFX = CScreenFX::GetInstance();
		if (pFX) pFX->Trigger_VoidFlame(true);
		break;
	}
	case eEnderDragonState::CIRCLE_DIVE:
		// 선회 시작 각도를 현재 드래곤 위치 기준으로 초기화
		m_fCircleAngle = atan2f(
			m_Spine[0].vPos.z - m_vPlayerPos.z,
			m_Spine[0].vPos.x - m_vPlayerPos.x);
		m_fWingSpeed = 6.0f;
		m_fWingAmp = D3DX_PI * 0.50f;
		m_fMoveSpeed = m_fDiveSpeed;
		break;
	case eEnderDragonState::TAIL_ATTACK:
		m_fTailSwingTimer = 0.f;
		m_fTailSwingAmp = D3DX_PI * 0.85f;
		m_fMoveSpeed = 12.f;
		break;
	default: break;
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// FSM — JSON 조건 기반 전환 평가
// ─────────────────────────────────────────────────────────────────────────────
void CEnderDragon::Evaluate_Transitions()
{
	// 전환 규칙이 없으면 스킵
	int iSrc = (int)m_eState;
	if (m_TransRules[iSrc].empty()) return;

	_float fDist      = DistToPlayer();
	_float fHpRatio   = (m_fMaxHP > 0.f) ? (m_fHP / m_fMaxHP) : 1.f;
	_float fHeadAngle = Compute_HeadToPlayerAngleDeg();

	for (const auto& rule : m_TransRules[iSrc])
	{
		bool bPass = true;
		if (rule.fMinStateTime    >= 0.f && m_fStateTimer  < rule.fMinStateTime)    bPass = false;
		if (rule.fMaxPlayerDist   >= 0.f && fDist          > rule.fMaxPlayerDist)   bPass = false;
		if (rule.fMinPlayerDist   >= 0.f && fDist          < rule.fMinPlayerDist)   bPass = false;
		if (rule.fMaxHpRatio      >= 0.f && fHpRatio       > rule.fMaxHpRatio)      bPass = false;
		if (rule.fMaxHeadAngleDeg >= 0.f && fHeadAngle     > rule.fMaxHeadAngleDeg) bPass = false;
		if (bPass)
		{
			Transition_State(rule.eTo);
			return;
		}
	}
}

void CEnderDragon::Force_Idle_State()
{
	Transition_State(eEnderDragonState::IDLE);
}

void CEnderDragon::Force_RootPos(const _vec3& vPos)
{
	m_Spine[0].vPos = vPos;
	m_vMoveTarget = vPos;
	m_vVelocity = _vec3(0.f, 0.f, 0.f);
}

void CEnderDragon::Handle_Input(const _float& fTimeDelta)
{
	// F5 → JSON 런타임 리로드
	if (GetAsyncKeyState(VK_F5) & 0x0001) // 0x0001: just pressed
		Load_DragonPatterns("Data/dragon_patterns.json");

	// ── Debug keys: ScreenFX effect testing ──
	CScreenFX* pFX = CScreenFX::GetInstance();
	if (!pFX) return;

	// 5 → Breath start (Wave + Noise + VoidTint)
	if (GetAsyncKeyState('5') & 0x0001)
	{
		pFX->Trigger_Wave(0.6f, 3.5f);
		pFX->Trigger_Noise(0.4f, 3.5f);
		pFX->Set_BreathActive(true, 1.f);
		OutputDebugStringA("[ScreenFX-DBG] Key5: Breath ON  (Wave 0.6/3.5s + Noise 0.4/3.5s + BreathTint)\n");
	}

	// 6 → Breath end (all effects fade out)
	if (GetAsyncKeyState('6') & 0x0001)
	{
		pFX->Set_BreathActive(false);
		OutputDebugStringA("[ScreenFX-DBG] Key6: Breath OFF (fade out)\n");
	}

	// 7 → Tail hit (GlassBreak + Noise)
	if (GetAsyncKeyState('7') & 0x0001)
	{
		pFX->Trigger_GlassBreak(1.0f, 0.4f);
		pFX->Trigger_Noise(0.8f, 0.5f);
		pFX->Trigger_Hit(1.0f);
		OutputDebugStringA("[ScreenFX-DBG] Key7: TailHit (GlassBreak 1.0/0.4s + Noise 0.8/0.5s + Shake)\n");
	}

	// 8 → Body collision (Wave only)
	if (GetAsyncKeyState('8') & 0x0001)
	{
		pFX->Trigger_Wave(0.5f, 0.8f);
		OutputDebugStringA("[ScreenFX-DBG] Key8: BodyHit (Wave 0.5/0.8s)\n");
	}
}

_float CEnderDragon::DistToPlayer() const
{
	CComponent* pComp = CManagement::GetInstance()->Get_Component(
		ID_DYNAMIC, L"GameLogic_Layer", L"Player", L"Com_Transform");
	if (!pComp) return FLT_MAX;

	CTransform* pTrans = dynamic_cast<CTransform*>(pComp);
	if (!pTrans) return FLT_MAX;

	_vec3 vPlayerPos;
	pTrans->Get_Info(INFO_POS, &vPlayerPos);
	return D3DXVec3Length(&(vPlayerPos - m_Spine[0].vPos));
}

// ─────────────────────────────────────────────────────────────────────────────
// 헬퍼 — 상태 ↔ 문자열 변환
// ─────────────────────────────────────────────────────────────────────────────
eEnderDragonState CEnderDragon::StringToState(const string& s) const
{
	if (s == "IDLE")        return eEnderDragonState::IDLE;
	if (s == "ATTACK")      return eEnderDragonState::ATTACK;
	if (s == "BREATH")      return eEnderDragonState::BREATH;
	if (s == "CIRCLE_DIVE") return eEnderDragonState::CIRCLE_DIVE;
	if (s == "TAIL_ATTACK") return eEnderDragonState::TAIL_ATTACK;
	return eEnderDragonState::IDLE;
}

const char* CEnderDragon::StateToString(eEnderDragonState e) const
{
	switch (e)
	{
	case eEnderDragonState::IDLE:        return "IDLE";
	case eEnderDragonState::ATTACK:      return "ATTACK";
	case eEnderDragonState::BREATH:      return "BREATH";
	case eEnderDragonState::CIRCLE_DIVE: return "CIRCLE_DIVE";
	case eEnderDragonState::TAIL_ATTACK: return "TAIL_ATTACK";
	default: return "UNKNOWN";
	}
}

_float CEnderDragon::Compute_HeadToPlayerAngleDeg() const
{
	_vec3 vHeadDir = m_Head.vDir;
	_vec3 vToPlayer = m_vPlayerPos - m_Head.vPos;
	_float fLen = D3DXVec3Length(&vToPlayer);
	if (fLen < 0.01f) return 0.f;

	D3DXVec3Normalize(&vToPlayer, &vToPlayer);
	D3DXVec3Normalize(&vHeadDir, &vHeadDir);

	_float fDot = D3DXVec3Dot(&vHeadDir, &vToPlayer);
	fDot = max(-1.f, min(1.f, fDot));
	return D3DXToDegree(acosf(fDot));
}

// ─────────────────────────────────────────────────────────────────────────────
// JSON 로드 — dragon_patterns.json
// ─────────────────────────────────────────────────────────────────────────────
HRESULT CEnderDragon::Load_DragonPatterns(const char* pPath)
{
	ifstream file(pPath);
	if (!file.is_open()) return E_FAIL;

	json j;
	try { file >> j; }
	catch (...) { return E_FAIL; }

	// physics 블록 → DragonFlight 필드 오버라이드
	if (j.contains("physics"))
	{
		auto& phy = j["physics"];
		m_Flight.fMass        = phy.value("mass",            m_Flight.fMass);
		m_Flight.fGravity     = phy.value("gravity",         m_Flight.fGravity);
		m_Flight.fLiftCoeff   = phy.value("lift_coeff",      m_Flight.fLiftCoeff);
		m_Flight.fDragCoeff   = phy.value("drag_coeff",      m_Flight.fDragCoeff);
		m_Flight.fThrustForce = phy.value("thrust_force",    m_Flight.fThrustForce);
		m_Flight.fMaxSpeed    = phy.value("max_speed",       m_Flight.fMaxSpeed);
		m_Flight.fBankFactor  = phy.value("bank_factor",     m_Flight.fBankFactor);
		m_Flight.fMaxBankAngle = phy.value("max_bank_angle_rad", m_Flight.fMaxBankAngle);
		m_Flight.fRestitution = phy.value("restitution",     m_Flight.fRestitution);
	}

	// patterns 배열 → 상태별 파라미터 + 전환 규칙
	if (j.contains("patterns"))
	{
		for (auto& pat : j["patterns"])
		{
			string id = pat.value("id", string(""));
			eEnderDragonState eSrc = StringToState(id);
			int iSrc = (int)eSrc;

			// 상태별 파라미터 오버라이드
			if (id == "IDLE")
			{
				m_fMoveSpeed = pat.value("patrol_speed", 18.f);
			}
			else if (id == "ATTACK")
			{
				// chase_speed → Transition_State 에서 m_fMoveSpeed에 반영됨
				// 여기서는 읽어서 별도 저장하지 않음 (Transition_State 에 이미 하드코딩)
			}
			else if (id == "BREATH")
			{
				m_fBreathDuration  = pat.value("breath_duration_sec",  m_fBreathDuration);
				m_fBreathConeDeg   = pat.value("breath_cone_deg",      m_fBreathConeDeg);
				m_fBreathDmgPerSec = pat.value("breath_damage_per_sec", m_fBreathDmgPerSec);
			}
			else if (id == "CIRCLE_DIVE")
			{
				m_fCircleRadius   = pat.value("circle_radius",    m_fCircleRadius);
				m_fDiveSpeed      = pat.value("dive_speed",       m_fDiveSpeed);
				m_fBankMultiplier = pat.value("bank_multiplier",  m_fBankMultiplier);
			}
			else if (id == "TAIL_ATTACK")
			{
				_float fAmpDeg = pat.value("swing_amp_deg", 55.f);
				m_fTailSwingAmp = D3DXToRadian(fAmpDeg);
			}

			// 전환 규칙 파싱
			if (pat.contains("transition"))
			{
				m_TransRules[iSrc].clear();
				for (auto& rule : pat["transition"])
				{
					TransitionRule tr;
					tr.eTo = StringToState(rule.value("to", string("IDLE")));
					string cond = rule.value("condition", string(""));

					// "&&"로 분리해서 각 조건 토큰 파싱
					istringstream iss(cond);
					string segment;
					while (getline(iss, segment, '&'))
					{
						if (segment.empty() || segment == "&") continue;
						istringstream tok(segment);
						string var, op;
						float val = 0.f;
						tok >> var >> op >> val;

						if (var == "player_dist")
						{
							if (op == "<" || op == "<=") tr.fMaxPlayerDist = val;
							else if (op == ">" || op == ">=") tr.fMinPlayerDist = val;
						}
						else if (var == "state_time")
						{
							if (op == ">" || op == ">=") tr.fMinStateTime = val;
						}
						else if (var == "hp_ratio")
						{
							if (op == "<" || op == "<=") tr.fMaxHpRatio = val;
						}
						else if (var == "head_to_player_angle")
						{
							if (op == "<" || op == "<=") tr.fMaxHeadAngleDeg = val;
						}
					}

					m_TransRules[iSrc].push_back(tr);
				}
			}
		}
	}

	return S_OK;
}

// ─────────────────────────────────────────────────────────────────────────────
// 충돌 — 뼈 AABB 갱신 + 플레이어 충돌 감지
// ─────────────────────────────────────────────────────────────────────────────
void CEnderDragon::Update_BoneColliders()
{
	// 척추 AABB (항상 갱신)
	for (int i = 0; i < ENDER_DRAGON_SPINE_COUNT; ++i)
	{
		if (m_pSpineCollider[i])
			m_pSpineCollider[i]->Update_AABB(m_Spine[i].vPos);
	}

	// 꼬리 AABB (TAIL_ATTACK 상태에서만 활성)
	if (m_eState == eEnderDragonState::TAIL_ATTACK)
	{
		for (int i = 0; i < ENDER_DRAGON_TAIL_COUNT; ++i)
		{
			if (m_pTailCollider[i])
				m_pTailCollider[i]->Update_AABB(m_Tail[i].vPos);
		}
	}

	// 날개 AABB (좌측만, 다운스트로크 시)
	for (int i = 0; i < ENDER_DRAGON_WING_COUNT; ++i)
	{
		if (m_pWingLCollider[i])
			m_pWingLCollider[i]->Update_AABB(m_WingL[i].vPos);
	}
}

void CEnderDragon::Check_BodyCollision()
{
	//CPlayer* pPlayer = CMonsterMgr::GetInstance()->Get_Player();
	//if (!pPlayer) return;

	//CCollider* pPlayerCol = dynamic_cast<CCollider*>(
	//	pPlayer->Get_Component(ID_STATIC, L"Com_Collider"));
	//if (!pPlayerCol) return;

	//AABB tPlayerAABB = pPlayerCol->Get_AABB();

	//// ── 척추 AABB 충돌 → 반발 임펄스 ──────────────────────────────────
	//for (int i = 0; i < ENDER_DRAGON_SPINE_COUNT; ++i)
	//{
	//	if (!m_pSpineCollider[i]) continue;
	//	if (!m_pSpineCollider[i]->IsColliding(tPlayerAABB)) continue;

	//	// 충돌 법선: 플레이어 ← 뼈 위치
	//	_vec3 vPlayerPos;
	//	CTransform* pTrans = dynamic_cast<CTransform*>(
	//		pPlayer->Get_Component(ID_DYNAMIC, L"Com_Transform"));
	//	if (!pTrans) break;
	//	pTrans->Get_Info(INFO_POS, &vPlayerPos);

	//	_vec3 vNormal = vPlayerPos - m_Spine[i].vPos;
	//	_float fLen = D3DXVec3Length(&vNormal);
	//	if (fLen < 0.01f) break;
	//	D3DXVec3Normalize(&vNormal, &vNormal);

	//	// 반발 임펄스 계산
	//	const _float fMassP = 80.f;
	//	const _float fMassD = m_Flight.fMass;
	//	_float fRelVel = D3DXVec3Dot(&m_vVelocity, &vNormal);
	//	if (fRelVel >= 0.f) break; // 이미 분리 중

	//	_float fImpulse = -(1.f + m_Flight.fRestitution) * fRelVel
	//		/ (1.f / fMassP + 1.f / fMassD);

	//	// 드래곤 속도 변화 (질량이 크므로 거의 안 밀림)
	//	m_vVelocity += vNormal * (fImpulse / fMassD);

	//	// 플레이어 → Hit() 호출 (5 데미지, 넉백 임계 이하)
	//	pPlayer->Hit(5.f);
	//	break; // 첫 충돌만 처리
	//}

	//// ── 꼬리 충돌 (TAIL_ATTACK: 피해 12 + 넉백) ────────────────────────
	//if (m_eState == eEnderDragonState::TAIL_ATTACK)
	//{
	//	for (int i = 0; i < ENDER_DRAGON_TAIL_COUNT; ++i)
	//	{
	//		if (!m_pTailCollider[i]) continue;
	//		if (!m_pTailCollider[i]->IsColliding(tPlayerAABB)) continue;

	//		pPlayer->Hit(12.f); // >= threshold(12) → 넉백 발동
	//		break;
	//	}
	//}

	//// ── 날개 충돌 (다운스트로크 시 넉백만, 경미한 피해) ──────────────────
	//_float fCurSin = sinf(m_fWingTimer);
	//if (m_fPrevSinVal > 0.f && fCurSin <= 0.f) // 다운스트로크 구간
	//{
	//	for (int i = 0; i < ENDER_DRAGON_WING_COUNT; ++i)
	//	{
	//		if (!m_pWingLCollider[i]) continue;
	//		if (!m_pWingLCollider[i]->IsColliding(tPlayerAABB)) continue;

	//		pPlayer->Hit(12.f); // 넉백 트리거, 날개에 맞으면 날아감
	//		break;
	//	}
	//}
}

// ─────────────────────────────────────────────────────────────────────────────
// ImGui 디버그 패널
// ─────────────────────────────────────────────────────────────────────────────
void CEnderDragon::Render_DebugPanel()
{
	ImGui::Begin("EnderDragon Debug");

	ImGui::Text("State: %s", StateToString(m_eState));
	ImGui::Text("state_time: %.2f s", m_fStateTimer);
	ImGui::Text("Speed: %.2f m/s", D3DXVec3Length(&m_vVelocity));
	ImGui::Text("HP: %.0f / %.0f", m_fHP, m_fMaxHP);
	ImGui::Separator();

	ImGui::Text("Pos: (%.1f, %.1f, %.1f)",
		m_Spine[0].vPos.x, m_Spine[0].vPos.y, m_Spine[0].vPos.z);
	ImGui::Text("AccumForce: (%.1f, %.1f, %.1f)",
		m_Flight.vAccumForce.x, m_Flight.vAccumForce.y, m_Flight.vAccumForce.z);
	ImGui::Text("BankAngle: %.1f deg", D3DXToDegree(m_fBankAngle));
	ImGui::Text("DistToPlayer: %.1f", DistToPlayer());
	ImGui::Text("HeadAngle: %.1f deg", Compute_HeadToPlayerAngleDeg());
	ImGui::Separator();

	// 물리 파라미터
	ImGui::Text("-- Physics --");
	ImGui::Text("Mass: %.1f  Gravity: %.1f", m_Flight.fMass, m_Flight.fGravity);
	ImGui::Text("Lift: %.2f  Drag: %.2f", m_Flight.fLiftCoeff, m_Flight.fDragCoeff);
	ImGui::Text("Thrust: %.1f  MaxSpeed: %.1f", m_Flight.fThrustForce, m_Flight.fMaxSpeed);
	ImGui::Text("Restitution: %.2f", m_Flight.fRestitution);

	if (ImGui::Button("Reload JSON (F5)"))
		Load_DragonPatterns("Data/dragon_patterns.json");

	ImGui::End();
}

// ─────────────────────────────────────────────────────────────────────────────
// Create / Free
// ─────────────────────────────────────────────────────────────────────────────
CEnderDragon* CEnderDragon::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
	CEnderDragon* pDragon = new CEnderDragon(pGraphicDev);
	if (FAILED(pDragon->Ready_GameObject()))
	{
		Safe_Release(pDragon);
		MSG_BOX("CEnderDragon Create Failed");
		return nullptr;
	}
	return pDragon;
}

void CEnderDragon::Free()
{
	Safe_Release(m_pTextureCom);

	auto ReleaseChain = [](DRAGON_BONE* pChain, _int iCount)
		{
			for (int i = 0; i < iCount; ++i) Safe_Release(pChain[i].pBuffer);
		};

	ReleaseChain(m_Spine, ENDER_DRAGON_SPINE_COUNT);
	ReleaseChain(m_Neck, ENDER_DRAGON_NECK_COUNT);
	Safe_Release(m_Head.pBuffer);
	ReleaseChain(m_Tail, ENDER_DRAGON_TAIL_COUNT);
	ReleaseChain(m_WingL, ENDER_DRAGON_WING_COUNT);
	ReleaseChain(m_WingR, ENDER_DRAGON_WING_COUNT);

	// 브래스 화염 해제
	for (auto& pFlame : m_vecBreathFlames)
		Safe_Release(pFlame);
	m_vecBreathFlames.clear();

	// 콜라이더 해제 (Gotcha #6: Safe_Release)
	for (int i = 0; i < ENDER_DRAGON_SPINE_COUNT; ++i)
		Safe_Release(m_pSpineCollider[i]);
	for (int i = 0; i < ENDER_DRAGON_TAIL_COUNT; ++i)
		Safe_Release(m_pTailCollider[i]);
	for (int i = 0; i < ENDER_DRAGON_WING_COUNT; ++i)
		Safe_Release(m_pWingLCollider[i]);

	// Gotcha #7: 베이스 호출은 반드시 마지막
	CGameObject::Free();
}
