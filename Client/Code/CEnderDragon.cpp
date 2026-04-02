#include "pch.h"
#include "CEnderDragon.h"
#include "CRenderer.h"
#include "CManagement.h"
#include "CDInputMgr.h"
#include "CMonsterMgr.h"
#include "CPlayer.h"
#include "CVoidFlame.h"
#include "CBreathFlame.h"
#include "CCollider.h"
#include "CScreenFX.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <algorithm>

using json = nlohmann::json;

// Patrol waypoints - scaled for Ender Dragon size
static const _vec3 s_PatrolPoints[] =
{
	_vec3(30.f, 22.f,  30.f),
	_vec3(-30.f, 24.f,  30.f),
	_vec3(-30.f, 22.f, -30.f),
	_vec3(30.f, 24.f, -30.f)
};

// ─────────────────────────────────────────────────────────────────────────────
// Construction / Destruction
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

	// Explicit quaternion init after ZeroMemory (0,0,0,0 is invalid)
	// Use ENDER_DRAGON_* constants (larger arrays than CDragon's DRAGON_*)
	for (int i = 0; i < ENDER_DRAGON_SPINE_COUNT; ++i) D3DXQuaternionIdentity(&m_Spine[i].qRot);
	for (int i = 0; i < ENDER_DRAGON_NECK_COUNT; ++i) D3DXQuaternionIdentity(&m_Neck[i].qRot);
	for (int i = 0; i < ENDER_DRAGON_TAIL_COUNT; ++i) D3DXQuaternionIdentity(&m_Tail[i].qRot);
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

	for (int i = 0; i < ENDER_DRAGON_SPINE_COUNT; ++i) D3DXQuaternionIdentity(&m_Spine[i].qRot);
	for (int i = 0; i < ENDER_DRAGON_NECK_COUNT; ++i) D3DXQuaternionIdentity(&m_Neck[i].qRot);
	for (int i = 0; i < ENDER_DRAGON_TAIL_COUNT; ++i) D3DXQuaternionIdentity(&m_Tail[i].qRot);
	D3DXQuaternionIdentity(&m_Head.qRot);
}

CEnderDragon::~CEnderDragon() {}

// ─────────────────────────────────────────────────────────────────────────────
// Initialization
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

	// Load JSON parameters (falls back to hardcoded defaults on failure)
	Load_DragonPatterns("Data/dragon_patterns.json");

	// Spine AABB collider creation (Gotcha #3: use Create() directly)
	for (int i = 0; i < ENDER_DRAGON_SPINE_COUNT; ++i)
	{
		m_pSpineCollider[i] = CCollider::Create(m_pGraphicDev,
			_vec3(2.5f, 2.0f, 2.5f), _vec3(0.f, 0.f, 0.f));
		if (!m_pSpineCollider[i]) return E_FAIL;
		m_mapComponent[ID_STATIC].insert({ L"Com_SpineCol", m_pSpineCollider[i] });
	}

	// Tail colliders
	for (int i = 0; i < ENDER_DRAGON_TAIL_COUNT; ++i)
	{
		m_pTailCollider[i] = CCollider::Create(m_pGraphicDev,
			_vec3(1.8f, 1.5f, 1.8f), _vec3(0.f, 0.f, 0.f));
		if (!m_pTailCollider[i]) return E_FAIL;
		m_mapComponent[ID_STATIC].insert({ L"Com_TailCol", m_pTailCollider[i] });
	}

	// Left wing colliders
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
// Init: Spine chain (9 bones, larger than original 7)
// ─────────────────────────────────────────────────────────────────────────────
HRESULT CEnderDragon::Init_SpineChain()
{
	// Index 0 = neck end (front), 8 = tail end (back)
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
// Init: Neck (5 bones) + Head
// ─────────────────────────────────────────────────────────────────────────────
HRESULT CEnderDragon::Init_NeckAndHead()
{
	const _float fNeckLen = 1.6f;
	FACE_UV uv = { 0.f, 0.f, 1.f, 1.f };

	// Neck thickness: tapers from root toward head
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

	// Head - attached at neck end
	_vec3 vNeckEnd = m_Neck[ENDER_DRAGON_NECK_COUNT - 1].vPos;
	m_Head.vPos = _vec3(vNeckEnd.x, vNeckEnd.y + 0.5f, vNeckEnd.z + fNeckLen);
	m_Head.vDir = _vec3(0.f, 0.f, 1.f);
	m_Head.fBoneLen = 2.4f;
	if (FAILED(Create_BoneBuffer(m_Head, 2.4f, 1.8f, 2.6f, uv))) return E_FAIL;

	return S_OK;
}

// ─────────────────────────────────────────────────────────────────────────────
// Init: Tail chain (8 bones)
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
// Init: Wings (6 segments, blade taper)
// ─────────────────────────────────────────────────────────────────────────────
HRESULT CEnderDragon::Init_WingChains()
{
	_vec3 vWingRoot = m_Spine[2].vPos;
	const _float fWingLen = 3.0f;
	const _float hd = fWingLen * 0.5f; // half local Z depth
	FACE_UV uv = { 0.f, 0.f, 1.f, 1.f };

	// fwd < 0 : +X blade tip (upward edge)
	// bkd     : back edge (gentle taper)
	// hh      : half thickness
	struct BladeFace { float fwd, bkd, hh; };

	// outer[i] = inner[i+1] -> ensures seam continuity between segments
	const BladeFace inner[ENDER_DRAGON_WING_COUNT] = {
		{ -5.6f, 0.80f, 0.18f },  // [0] root (widest)
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
		{ -0.05f, 0.06f, 0.01f }, // outer[5] = blade tip
	};

	for (_int i = 0; i < ENDER_DRAGON_WING_COUNT; ++i)
	{
		_float fOfs = (_float)(i + 1) * fWingLen;
		_float fZFwdR = (_float)i * (-0.5f); // right wing: -Z toward tip
		_float fZFwdL = (_float)i * (+0.5f); // left wing: opposite direction

		m_WingL[i].vPos = _vec3(vWingRoot.x - fOfs, vWingRoot.y, vWingRoot.z + fZFwdL);
		m_WingL[i].vDir = _vec3(-1.f, 0.f, 0.f);
		m_WingL[i].fBoneLen = fWingLen;

		m_WingR[i].vPos = _vec3(vWingRoot.x + fOfs, vWingRoot.y, vWingRoot.z + fZFwdR);
		m_WingR[i].vDir = _vec3(1.f, 0.f, 0.f);
		m_WingR[i].fBoneLen = fWingLen;

		// Corner indices:
		// [0..3] tip side (Z=+hd)   [4..7] body side (Z=-hd)
		MESH mesh{};
		mesh.front = mesh.back = mesh.top = mesh.bottom = mesh.right = mesh.left = uv;

		const BladeFace& o = outer[i]; // tip side (narrow)
		const BladeFace& n = inner[i]; // body side (wide)

		mesh.corners[0] = { -o.fwd, +o.hh, +hd };
		mesh.corners[1] = { +o.bkd, +o.hh, +hd };
		mesh.corners[2] = { +o.bkd, -o.hh, +hd };
		mesh.corners[3] = { -o.fwd, -o.hh, +hd };
		mesh.corners[4] = { +n.bkd, +n.hh, -hd };
		mesh.corners[5] = { -n.fwd, +n.hh, -hd };
		mesh.corners[6] = { -n.fwd, -n.hh, -hd };
		mesh.corners[7] = { +n.bkd, -n.hh, -hd };

		// Left wing: X mirror + restore CCW winding
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
	// Clamp fTimeDelta: prevent first-frame spike during scene load
	const _float dt = min(fTimeDelta, 0.05f);

	Handle_Input(dt);
	m_fStateTimer += dt;

	// ── Reset accumulated force per frame ──────────────────────────────────────────
	m_Flight.vAccumForce = _vec3(0.f, 0.f, 0.f);

	// ── FSM: accumulate steering force + update Spine[0].vDir ───────────────────
	switch (m_eState)
	{
	case eEnderDragonState::IDLE:        Update_IDLE(dt);        break;
	case eEnderDragonState::ATTACK:      Update_Attack(dt);      break;
	case eEnderDragonState::BREATH:      Update_BREATH(dt);      break;
	case eEnderDragonState::CIRCLE_DIVE: Update_CIRCLE_DIVE(dt); break;
	case eEnderDragonState::TAIL_ATTACK: Update_TailAttack(dt);  break;
	default: break;
	}

	// ── Evaluate JSON-based state transitions ─────────────────────────────────
	Evaluate_Transitions();

	// ── Accumulate physics forces: gravity + drag + lift ────────────────────────────
	Accumulate_Forces(dt);

	// ── Wing flap: sin wave + downstroke thrust ─────────────────────────
	Update_WingFlap(dt);

	// ── Physics integration: F=ma -> velocity -> position ───────────────────────────────
	Integrate_Physics(dt);

	// ── Update bone colliders + player collision ───────────────────────────────
	Update_BoneColliders();
	Check_BodyCollision();

	// ── Chain solving (after Spine[0] position finalized) ────────────────────────────
	Solve_FollowLeader(m_Spine, ENDER_DRAGON_SPINE_COUNT);

	// Neck root = slightly above Spine[0]
	m_Neck[0].vPos = m_Spine[0].vPos + _vec3(0.f, 0.5f, 0.f);

	// Neck/Head CCD + Slerp
	{
		DRAGON_BONE combined[ENDER_DRAGON_NECK_COUNT + 1];
		for (int i = 0; i < ENDER_DRAGON_NECK_COUNT; ++i) combined[i] = m_Neck[i];
		combined[ENDER_DRAGON_NECK_COUNT] = m_Head;

		// BREATH/CIRCLE_DIVE/ATTACK: head locks on to player directly
		bool bLockOnPlayer = (m_eState == eEnderDragonState::BREATH
			|| m_eState == eEnderDragonState::CIRCLE_DIVE
			|| m_eState == eEnderDragonState::ATTACK);
		_vec3 vNeckTarget = bLockOnPlayer
			? m_vPlayerPos
			: m_vMoveTarget + _vec3(0.f, 1.5f, 0.f);

		_float fCCDAngle = bLockOnPlayer ? D3DX_PI * 0.5f : D3DX_PI * 0.2f;
		_int   iCCDIter = bLockOnPlayer ? 10 : 6;
		Solve_CCD(combined, ENDER_DRAGON_NECK_COUNT + 1, vNeckTarget, iCCDIter, fCCDAngle);

		DRAGON_BONE current[ENDER_DRAGON_NECK_COUNT + 1];
		for (int i = 0; i < ENDER_DRAGON_NECK_COUNT; ++i) current[i] = m_Neck[i];
		current[ENDER_DRAGON_NECK_COUNT] = m_Head;

		_float fSlerpAlpha = (m_eState == eEnderDragonState::BREATH) ? dt * 12.f
			: bLockOnPlayer ? dt * 8.f : dt * 3.f;
		Slerp_NeckChain(combined, current, ENDER_DRAGON_NECK_COUNT + 1, fSlerpAlpha);

		for (int i = 0; i < ENDER_DRAGON_NECK_COUNT; ++i) m_Neck[i] = current[i];
		m_Head = current[ENDER_DRAGON_NECK_COUNT];
	}

	// Tail follow-the-leader
	m_Tail[0].vPos = m_Spine[ENDER_DRAGON_SPINE_COUNT - 1].vPos;
	Solve_FollowLeader(m_Tail, ENDER_DRAGON_TAIL_COUNT);

	// ── Post-chain deformation: state-specific placement after FollowLeader ──────
	Apply_IdleCurlPostChain(dt);
	Apply_TailSwingPostChain(dt);

	// Re-update colliders after post-deformation (latest positions after swing/curl)
	Update_BoneColliders();

	// ── Matrix update ────────────────────────────────────────────────────
	Update_ChainMatrices(m_Spine, ENDER_DRAGON_SPINE_COUNT);
	Update_ChainMatrices(m_Neck, ENDER_DRAGON_NECK_COUNT);
	Compute_BoneMatrix(m_Head);
	Update_ChainMatrices(m_Tail, ENDER_DRAGON_TAIL_COUNT);
	Update_ChainMatrices(m_WingL, ENDER_DRAGON_WING_COUNT);
	Update_ChainMatrices(m_WingR, ENDER_DRAGON_WING_COUNT);

	// ── Banking: visual roll effect on spine during turns ───────────────────────────
	Update_Banking(dt);

	// ── Breath flame update / cleanup ──────────────────────────────────
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

	// CBreathFlame 3D beam update — runs in any state if active (debug key 9)
	if (CBreathFlame::GetInstance()->Is_Active())
	{
		_vec3 vBeamDir = m_vPlayerPos - m_Head.vPos;
		D3DXVec3Normalize(&vBeamDir, &vBeamDir);
		CBreathFlame::GetInstance()->Update(dt, m_Head.vPos, vBeamDir);
	}

	CRenderer::GetInstance()->Add_RenderGroup(RENDER_NONALPHA, this);
	return CGameObject::Update_GameObject(fTimeDelta);
}

void CEnderDragon::LateUpdate_GameObject(const _float& fTimeDelta)
{
	CGameObject::LateUpdate_GameObject(fTimeDelta);
}

// ─────────────────────────────────────────────────────────────────────────────
// Render
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

	// Breath flame rendering
	for (auto& pFlame : m_vecBreathFlames)
	{
		if (pFlame && !pFlame->Is_Dead())
			pFlame->Render_GameObject();
	}

	// 3D beam breath
	if (CBreathFlame::GetInstance()->Is_Active())
		CBreathFlame::GetInstance()->Render();

	// ImGui debug panel
	Render_DebugPanel();
}

void CEnderDragon::Set_RootPos(const _vec3 vPos)
{

}

// ─────────────────────────────────────────────────────────────────────────────
// Dynamics (1) - Force accumulation (gravity / drag / lift)
// ─────────────────────────────────────────────────────────────────────────────
void CEnderDragon::Accumulate_Forces(const _float& fTimeDelta)
{
	// (1) Gravity : F = -Y * m * g
	m_Flight.vAccumForce += _vec3(0.f, -(m_Flight.fMass * m_Flight.fGravity), 0.f);

	// (2) Drag : F = -normalize(v) * |v|² * Cd
	//    Faster speed -> quadratic resistance -> natural speed limit
	_float fSpeed = D3DXVec3Length(&m_vVelocity);
	if (fSpeed > 0.01f)
	{
		_vec3 vDragDir = -m_vVelocity;
		D3DXVec3Normalize(&vDragDir, &vDragDir);
		m_Flight.vAccumForce += vDragDir * (fSpeed * fSpeed * m_Flight.fDragCoeff);
	}

	// (3) Lift : F = +Y * fwdSpeed² * Cl
	//    Higher forward speed generates upward lift
	//    Level flight condition: fwdSpeed² * Cl = m * g
	//    → Cl=2.8, m=120, g=12 → cruise speed ≈ sqrt(1440/2.8) ≈ 22.7 m/s
	_vec3 vLook = m_Spine[0].vDir;
	_float fFwdSpeed = D3DXVec3Dot(&m_vVelocity, &vLook);
	if (fFwdSpeed > 0.f)
	{
		_float fLift = fFwdSpeed * fFwdSpeed * m_Flight.fLiftCoeff;
		m_Flight.vAccumForce += _vec3(0.f, fLift, 0.f);
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// Dynamics (2) - Physics integration (F = ma -> velocity -> position)
// ─────────────────────────────────────────────────────────────────────────────
void CEnderDragon::Integrate_Physics(const _float& fTimeDelta)
{
	// a = F / m
	_vec3 vAccel = m_Flight.vAccumForce * (1.f / m_Flight.fMass);
	m_vVelocity += vAccel * fTimeDelta;

	// Clamp to max speed
	_float fSpeed = D3DXVec3Length(&m_vVelocity);
	if (fSpeed > m_Flight.fMaxSpeed)
	{
		D3DXVec3Normalize(&m_vVelocity, &m_vVelocity);
		m_vVelocity *= m_Flight.fMaxSpeed;
	}

	// Position integration
	m_Spine[0].vPos += m_vVelocity * fTimeDelta;

	// Reset accumulated force for next frame (also reset at start of Accumulate_Forces)
	m_Flight.vAccumForce = _vec3(0.f, 0.f, 0.f);
}

// ─────────────────────────────────────────────────────────────────────────────
// Dynamics (3) - Banking (adds Roll to spine matrices during turns, visual only)
// ─────────────────────────────────────────────────────────────────────────────
void CEnderDragon::Update_Banking(const _float& fTimeDelta)
{
	_vec3 vLookCur = m_Spine[0].vDir;

	_float fSpeed = D3DXVec3Length(&m_vVelocity);

	// Cross product Y component = horizontal turn rate (positive: right)
	_vec3 vCross;
	D3DXVec3Cross(&vCross, &m_vLookPrev, &vLookCur);
	_float fTurnRate = vCross.y / max(fTimeDelta, 0.001f);

	// Compute target banking angle + clamp
	_float fBankFact = m_Flight.fBankFactor;
	_float fBankMax  = m_Flight.fMaxBankAngle;
	if (m_eState == eEnderDragonState::CIRCLE_DIVE)
	{
		fBankFact *= m_fBankMultiplier;
		fBankMax  *= m_fBankMultiplier;
	}
	_float fTargetBank = -fTurnRate * fBankFact;
	fTargetBank = max(-fBankMax, min(fBankMax, fTargetBank));

	// Smooth interpolation of banking angle
	m_fBankAngle += (fTargetBank - m_fBankAngle) * min(fTimeDelta * 4.f, 1.f);
	m_vLookPrev = vLookCur;

	if (fabsf(m_fBankAngle) < 0.001f) return;

	// Apply Roll to spine matrices (rotation around Look axis)
	for (int i = 0; i < ENDER_DRAGON_SPINE_COUNT; ++i)
	{
		_vec3 vLook(m_Spine[i].matWorld._31,
			m_Spine[i].matWorld._32,
			m_Spine[i].matWorld._33);
		_matrix matRoll;
		D3DXMatrixRotationAxis(&matRoll, &vLook, m_fBankAngle);

		// Separate translation, apply rotation, restore translation
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
// FSM - IDLE: curled idle + player distance monitoring
// ─────────────────────────────────────────────────────────────────────────────
void CEnderDragon::Update_IDLE(const _float& fTimeDelta)
{
	// Only update curl blend value (actual positioning in Apply_IdleCurlPostChain)
	m_fIdleCurlBlend = min(1.f, m_fIdleCurlBlend + fTimeDelta * 0.8f);

	// ── Hover: counteract gravity (stationary flight) ──
	m_Flight.vAccumForce += _vec3(0.f, m_Flight.fMass * m_Flight.fGravity * 0.95f, 0.f);

	// Velocity damping (bring to halt)
	_float fSpeed = D3DXVec3Length(&m_vVelocity);
	if (fSpeed > 0.01f)
	{
		_vec3 vBrake = -m_vVelocity;
		D3DXVec3Normalize(&vBrake, &vBrake);
		m_Flight.vAccumForce += vBrake * (fSpeed * m_Flight.fMass * 1.5f);
	}

	// ── Player distance monitoring ──
	_float fPlayerDist = DistToPlayer();
	if (fPlayerDist < m_fDetectRange)
	{
		m_fDetectTimer += fTimeDelta;
		if (m_fDetectTimer >= m_fDetectThreshold)
		{
			Transition_State(eEnderDragonState::CIRCLE_DIVE);
			return;
		}
	}
	else
	{
		m_fDetectTimer = max(0.f, m_fDetectTimer - fTimeDelta * 0.5f);
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// FSM - ATTACK: player tracking + steering force accumulation
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

		// Attack state: stronger steering (4x)
		_float fSteerMag = min(fDist * m_Flight.fMass * 5.f,
			m_Flight.fMass * m_fMoveSpeed * 1.5f);
		m_Flight.vAccumForce += vDir * fSteerMag;
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// FSM - BREATH: head lock-on + CVoidFlame spawn at 0.15s intervals
// ─────────────────────────────────────────────────────────────────────────────
void CEnderDragon::Update_BREATH(const _float& fTimeDelta)
{
	// Update player position
	CComponent* pCom = CManagement::GetInstance()->Get_Component(
		ID_DYNAMIC, L"GameLogic_Layer", L"Player", L"Com_Transform");
	if (pCom)
	{
		CTransform* pTrans = dynamic_cast<CTransform*>(pCom);
		if (pTrans) pTrans->Get_Info(INFO_POS, &m_vPlayerPos);
	}

	// Dragon body hover (partially counteract gravity)
	m_Flight.vAccumForce += _vec3(0.f, m_Flight.fMass * m_Flight.fGravity * 0.5f, 0.f);

	// CVoidFlame spawn (0.15s interval)
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

	// CBreathFlame update is now outside FSM switch (Update_GameObject)
}

// ─────────────────────────────────────────────────────────────────────────────
// FSM - CIRCLE_DIVE: orbit around player + alternating VoidFlame/Beam attack
// ─────────────────────────────────────────────────────────────────────────────
void CEnderDragon::Update_CIRCLE_DIVE(const _float& fTimeDelta)
{
	// Update player position
	CComponent* pCom = CManagement::GetInstance()->Get_Component(
		ID_DYNAMIC, L"GameLogic_Layer", L"Player", L"Com_Transform");
	if (pCom)
	{
		CTransform* pTrans = dynamic_cast<CTransform*>(pCom);
		if (pTrans) pTrans->Get_Info(INFO_POS, &m_vPlayerPos);
	}

	// ── Circular orbit ──
	_float fAngSpeed = m_fDiveSpeed / m_fCircleRadius;
	m_fCircleAngle += fAngSpeed * fTimeDelta;

	_vec3 vOrbitTarget;
	vOrbitTarget.x = m_vPlayerPos.x + cosf(m_fCircleAngle) * m_fCircleRadius;
	vOrbitTarget.z = m_vPlayerPos.z + sinf(m_fCircleAngle) * m_fCircleRadius;
	vOrbitTarget.y = m_vPlayerPos.y + 8.f;

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

	// ── VoidFlame / Beam alternating attack ──
	m_fCircleAttackTimer += fTimeDelta;

	_float fPhaseDuration = m_bCirclePhaseIsBeam ? m_fBeamDuration : m_fVoidFlameDuration;
	if (m_fCircleAttackTimer >= fPhaseDuration)
	{
		// Phase transition
		m_fCircleAttackTimer = 0.f;
		m_bCirclePhaseIsBeam = !m_bCirclePhaseIsBeam;

		if (m_bCirclePhaseIsBeam)
		{
			// Begin Beam phase
			Void_Breath(true);
			CScreenFX* pFX = CScreenFX::GetInstance();
			if (pFX) pFX->Trigger_VoidFlame(true);
		}
		else
		{
			// Begin VoidFlame phase (end Beam)
			Void_Breath(false);
			CScreenFX* pFX = CScreenFX::GetInstance();
			if (pFX) pFX->Trigger_VoidFlame(false);
			m_fBreathTimer = 0.f;
		}
	}

	// Execute attack based on current phase
	if (m_bCirclePhaseIsBeam)
	{
		// Beam: CBreathFlame auto-updates in Update_GameObject
		// Head direction faces player (handled by CCD IK)
	}
	else
	{
		// VoidFlame: projectile spawn at 0.15s intervals
		m_fBreathTimer += fTimeDelta;
		if (m_fBreathTimer >= 0.15f)
		{
			m_fBreathTimer = 0.f;
			_vec3 vFireDir = m_vPlayerPos - m_Head.vPos;
			_float fLen = D3DXVec3Length(&vFireDir);
			if (fLen > 0.01f)
			{
				D3DXVec3Normalize(&vFireDir, &vFireDir);
				float fDamage = m_fBreathDmgPerSec * 0.15f;
				CVoidFlame* pFlame = CVoidFlame::Create(m_pGraphicDev, m_Head.vPos, vFireDir, fDamage);
				if (pFlame)
					m_vecBreathFlames.push_back(pFlame);
			}
		}
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// FSM - TAIL_ATTACK: rush toward player + tail swing + hit detection
// ─────────────────────────────────────────────────────────────────────────────
void CEnderDragon::Update_TailAttack(const _float& fTimeDelta)
{
	// Update player position
	CComponent* pCom = CManagement::GetInstance()->Get_Component(
		ID_DYNAMIC, L"GameLogic_Layer", L"Player", L"Com_Transform");
	if (pCom)
	{
		CTransform* pTrans = dynamic_cast<CTransform*>(pCom);
		if (pTrans) pTrans->Get_Info(INFO_POS, &m_vPlayerPos);
	}

	m_fTailSwingTimer += fTimeDelta;

	// ── High-speed rush toward player ──
	_vec3 vToPlayer = m_vPlayerPos - m_Spine[0].vPos;
	vToPlayer.y += 2.f;
	_float fDist = D3DXVec3Length(&vToPlayer);

	if (fDist > 3.f)
	{
		_vec3 vDir;
		D3DXVec3Normalize(&vDir, &vToPlayer);
		D3DXVec3Normalize(&m_Spine[0].vDir, &vDir);

		_float fSteerMag = min(fDist * m_Flight.fMass * 6.f,
			m_Flight.fMass * m_fTailRushSpeed * 1.5f);
		m_Flight.vAccumForce += vDir * fSteerMag;
	}

	// Tail swing positioning done in Apply_TailSwingPostChain (after Solve_FollowLeader)

	// ── Tail hit detection (colliders updated after Apply_TailSwingPostChain) ──
	//if (!m_bTailHitApplied)
	//{
	//	CPlayer* pPlayer = CMonsterMgr::GetInstance()->Get_Player();
	//	if (pPlayer)
	//	{
	//		CCollider* pPlayerCol = dynamic_cast<CCollider*>(
	//			pPlayer->Get_Component(ID_STATIC, L"Com_Collider"));
	//		if (pPlayerCol)
	//		{
	//			AABB tPlayerAABB = pPlayerCol->Get_AABB();
	//			for (int i = 0; i < ENDER_DRAGON_TAIL_COUNT; ++i)
	//			{
	//				if (!m_pTailCollider[i]) continue;
	//				if (m_pTailCollider[i]->IsColliding(tPlayerAABB))
	//				{
	//					pPlayer->Hit(12.f); // triggers knockback
	//					m_bTailHitApplied = true;

	//					// Screen effects
	//					CScreenFX* pFX = CScreenFX::GetInstance();
	//					if (pFX)
	//					{
	//						pFX->Trigger_GlassBreak(1.0f, 0.4f);
	//						pFX->Trigger_Noise(0.8f, 0.5f);
	//						pFX->Trigger_Hit(1.0f);
	//					}
	//					break;
	//				}
	//			}
	//		}
	//	}
	//}

	// ── Time expired -> return to CIRCLE ──
	if (m_fStateTimer >= m_fTailAttackDuration)
	{
		Transition_State(eEnderDragonState::CIRCLE_DIVE);
		return;
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// Wing flap - sin wave + downstroke thrust
// ─────────────────────────────────────────────────────────────────────────────
void CEnderDragon::Update_WingFlap(const _float& fTimeDelta)
{
	m_fWingTimer += fTimeDelta * m_fWingSpeed;

	// ── Downstroke detection ────────────────────────────────────────────
	// sin crossing positive to negative = wing begins downstroke from peak
	_float fCurSin = sinf(m_fWingTimer);
	if (m_fPrevSinVal > 0.05f && fCurSin <= 0.05f)
	{
		// Thrust: forward direction + upward component
		_vec3 vThrust = m_Spine[0].vDir * m_Flight.fThrustForce;
		vThrust.y += m_Flight.fThrustForce * 0.55f; // upward component
		m_Flight.vAccumForce += vThrust;
	}

	m_fPrevSinVal = fCurSin;

	// ── Wing bone position calculation ────────────────────────────────────────────
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

		// Step 1: local flap rotation
		D3DXVec3TransformNormal(&vBaseL, &vBaseL, &matFlapL);
		D3DXVec3TransformNormal(&vBaseR, &vBaseR, &matFlapR);

		// Step 2: apply body rotation
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
	// Unused - replaced by Apply_TailSwingPostChain
}

// ─────────────────────────────────────────────────────────────────────────────
// Post-chain deformation: IDLE curl (applied after Solve_FollowLeader)
// ─────────────────────────────────────────────────────────────────────────────
void CEnderDragon::Apply_IdleCurlPostChain(const _float& fTimeDelta)
{
	if (m_eState != eEnderDragonState::IDLE) return;

	_vec3 vCenter = m_Spine[0].vPos;
	const _float fCurlRadius = 6.f;

	// Spine curl: blend from FollowLeader position toward circular layout
	for (int i = 1; i < ENDER_DRAGON_SPINE_COUNT; ++i)
	{
		_float fAngle = D3DX_PI * (_float)i / (ENDER_DRAGON_SPINE_COUNT - 1);
		_vec3 vCurled;
		vCurled.x = vCenter.x + fCurlRadius * sinf(fAngle);
		vCurled.y = vCenter.y - 1.5f * (_float)i / ENDER_DRAGON_SPINE_COUNT;
		vCurled.z = vCenter.z - fCurlRadius * cosf(fAngle);

		// Blend strength = curl blend * dt interpolation
		_float fBlend = m_fIdleCurlBlend * min(fTimeDelta * 3.f, 1.f);
		m_Spine[i].vPos += (vCurled - m_Spine[i].vPos) * fBlend;

		// Also update direction (toward next bone)
		if (i < ENDER_DRAGON_SPINE_COUNT - 1)
		{
			_vec3 vNextCurled;
			_float fNextAngle = D3DX_PI * (_float)(i + 1) / (ENDER_DRAGON_SPINE_COUNT - 1);
			vNextCurled.x = vCenter.x + fCurlRadius * sinf(fNextAngle);
			vNextCurled.y = vCenter.y - 1.5f * (_float)(i + 1) / ENDER_DRAGON_SPINE_COUNT;
			vNextCurled.z = vCenter.z - fCurlRadius * cosf(fNextAngle);

			_vec3 vDir = vNextCurled - m_Spine[i].vPos;
			if (D3DXVec3Length(&vDir) > 0.01f)
				D3DXVec3Normalize(&m_Spine[i].vDir, &vDir);
		}
	}

	// Tail curl: wrap toward body
	for (int i = 0; i < ENDER_DRAGON_TAIL_COUNT; ++i)
	{
		_float fAngle = D3DX_PI + D3DX_PI * 0.5f * (_float)i / (ENDER_DRAGON_TAIL_COUNT - 1);
		_float fTailRadius = fCurlRadius * 0.6f;
		_vec3 vCurled;
		vCurled.x = vCenter.x + fTailRadius * sinf(fAngle);
		vCurled.y = vCenter.y - 2.f;
		vCurled.z = vCenter.z - fTailRadius * cosf(fAngle);

		_float fBlend = m_fIdleCurlBlend * min(fTimeDelta * 3.f, 1.f);
		m_Tail[i].vPos += (vCurled - m_Tail[i].vPos) * fBlend;
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// Post-chain deformation: TAIL_ATTACK tail swing (applied after Solve_FollowLeader)
// ─────────────────────────────────────────────────────────────────────────────
void CEnderDragon::Apply_TailSwingPostChain(const _float& fTimeDelta)
{
	if (m_eState != eEnderDragonState::TAIL_ATTACK) return;

	_float fSwingAng = m_fTailSwingAmp * sinf(m_fTailSwingTimer * 2.8f);

	// Swing perpendicular to spine direction (rightward)
	_vec3 vSpineRight;
	_vec3 vUp(0.f, 1.f, 0.f);
	D3DXVec3Cross(&vSpineRight, &m_Spine[0].vDir, &vUp);
	if (D3DXVec3Length(&vSpineRight) > 0.001f)
		D3DXVec3Normalize(&vSpineRight, &vSpineRight);
	else
		vSpineRight = _vec3(1.f, 0.f, 0.f);

	for (int i = 0; i < ENDER_DRAGON_TAIL_COUNT; ++i)
	{
		_float fSwingScale = (_float)(i + 1) / ENDER_DRAGON_TAIL_COUNT;
		_vec3 vSwingOffset = vSpineRight * (fSwingAng * fSwingScale * 3.f);
		m_Tail[i].vPos += vSwingOffset;

		// Update colliders (latest position after swing)
		if (m_pTailCollider[i])
			m_pTailCollider[i]->Update_AABB(m_Tail[i].vPos);
	}
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
// Quaternion utilities
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
// Bone matrix computation (Look-At, DX9 Row-Major)
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
// FSM state transition
// ─────────────────────────────────────────────────────────────────────────────
void CEnderDragon::Transition_State(eEnderDragonState eNext)
{
	if (m_eState == eNext) return;

	// Disable VoidFlame + BreathFlame when leaving BREATH state
	if (m_eState == eEnderDragonState::BREATH)
	{
		CScreenFX* pFX = CScreenFX::GetInstance();
		if (pFX) pFX->Trigger_VoidFlame(false);
		Void_Breath(false);
	}

	// Clean up in-progress attacks when leaving CIRCLE_DIVE state
	if (m_eState == eEnderDragonState::CIRCLE_DIVE)
	{
		if (m_bCirclePhaseIsBeam)
			Void_Breath(false);
		CScreenFX* pFX = CScreenFX::GetInstance();
		if (pFX) pFX->Trigger_VoidFlame(false);
	}

	m_eState = eNext;
	m_fStateTimer = 0.f;

	switch (eNext)
	{
	case eEnderDragonState::IDLE:
		m_fWingSpeed = 1.2f;          // wings folded: slow subtle movement
		m_fWingAmp = D3DX_PI * 0.05f; // nearly folded
		m_fMoveSpeed = 0.f;           // stationary
		m_fDetectTimer = 0.f;
		m_fIdleCurlBlend = 0.f;       // reset curl blend
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
		// Start screen VoidFlame effect
		CScreenFX* pFX = CScreenFX::GetInstance();
		if (pFX) pFX->Trigger_VoidFlame(true);
		// Start heat-haze breath cone
		Void_Breath(true);
		break;
	}
	case eEnderDragonState::CIRCLE_DIVE:
		// Initialize orbit angle based on current dragon position
		m_fCircleAngle = atan2f(
			m_Spine[0].vPos.z - m_vPlayerPos.z,
			m_Spine[0].vPos.x - m_vPlayerPos.x);
		m_fWingSpeed = 6.0f;
		m_fWingAmp = D3DX_PI * 0.50f;
		m_fMoveSpeed = m_fDiveSpeed;
		// Initialize attack alternation
		m_fCircleAttackTimer = 0.f;
		m_bCirclePhaseIsBeam = false; // VoidFlame first
		m_fBreathTimer = 0.f;
		break;
	case eEnderDragonState::TAIL_ATTACK:
		m_fTailSwingTimer = 0.f;
		m_fTailSwingAmp = D3DX_PI * 0.85f;
		m_fMoveSpeed = m_fTailRushSpeed;
		m_fWingSpeed = 7.0f;
		m_fWingAmp = D3DX_PI * 0.55f;
		m_bTailHitApplied = false;
		break;
	default: break;
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// FSM - JSON condition-based transition evaluation
// ─────────────────────────────────────────────────────────────────────────────
void CEnderDragon::Evaluate_Transitions()
{
	// Skip if no transition rules exist
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

void CEnderDragon::Void_Breath(bool bActivate)
{
	if (bActivate && !m_bBreathFiring)
	{
		m_bBreathFiring = true;
		CBreathFlame::GetInstance()->Activate(m_pGraphicDev, 1.f, 15.f);
		// SetBreathActive is separate — global purple tint, not the 3D beam
		// Transition_State already calls Trigger_VoidFlame for the tint
	}
	else if (!bActivate && m_bBreathFiring)
	{
		m_bBreathFiring = false;
		CBreathFlame::GetInstance()->Deactivate();
	}
}

void CEnderDragon::Force_RootPos(const _vec3& vPos)
{
	m_Spine[0].vPos = vPos;
	m_vMoveTarget = vPos;
	m_vVelocity = _vec3(0.f, 0.f, 0.f);
}

void CEnderDragon::Handle_Input(const _float& fTimeDelta)
{
	// F5 -> JSON runtime reload
	if (GetAsyncKeyState(VK_F5) & 0x0001)
		Load_DragonPatterns("Data/dragon_patterns.json");

	// ── H/J/K/L: force state transition debug keys (always works, independent of CScreenFX) ──
	if (GetAsyncKeyState('H') & 0x0001)
	{
		Transition_State(eEnderDragonState::IDLE);
		OutputDebugStringA("[Dragon-FSM] H: -> IDLE\n");
	}
	if (GetAsyncKeyState('J') & 0x0001)
	{
		Transition_State(eEnderDragonState::CIRCLE_DIVE);
		OutputDebugStringA("[Dragon-FSM] J: -> CIRCLE_DIVE\n");
	}
	if (GetAsyncKeyState('K') & 0x0001)
	{
		Transition_State(eEnderDragonState::TAIL_ATTACK);
		OutputDebugStringA("[Dragon-FSM] K: -> TAIL_ATTACK\n");
	}
	if (GetAsyncKeyState('L') & 0x0001)
	{
		Transition_State(eEnderDragonState::BREATH);
		OutputDebugStringA("[Dragon-FSM] L: -> BREATH\n");
	}

	// ── Debug keys: ScreenFX effect testing (skip this block if pFX is null) ──
	CScreenFX* pFX = CScreenFX::GetInstance();
	if (pFX)
	{
		// 5 → Breath start (Wave + Noise + VoidTint)
		if (GetAsyncKeyState('5') & 0x0001)
		{
			pFX->Trigger_Wave(0.6f, 3.5f);
			pFX->Trigger_Noise(0.4f, 3.5f);
			pFX->Set_BreathActive(true, 1.f);
			OutputDebugStringA("[ScreenFX-DBG] Key5: Breath ON\n");
		}
		// 6 → Breath end
		if (GetAsyncKeyState('6') & 0x0001)
		{
			pFX->Set_BreathActive(false);
			OutputDebugStringA("[ScreenFX-DBG] Key6: Breath OFF\n");
		}
		// 7 → Tail hit (GlassBreak + Noise)
		if (GetAsyncKeyState('7') & 0x0001)
		{
			pFX->Trigger_GlassBreak(1.0f, 0.4f);
			pFX->Trigger_Noise(0.8f, 0.5f);
			pFX->Trigger_Hit(1.0f);
			OutputDebugStringA("[ScreenFX-DBG] Key7: TailHit\n");
		}
		// 8 → Body collision (Wave only)
		if (GetAsyncKeyState('8') & 0x0001)
		{
			pFX->Trigger_Wave(0.5f, 0.8f);
			OutputDebugStringA("[ScreenFX-DBG] Key8: BodyHit\n");
		}
	}

	// 9 → BreathFlame heat-haze toggle (independent of CScreenFX)
	if (GetAsyncKeyState('9') & 0x8000)
	{
		if (CBreathFlame::GetInstance()->Is_Active())
		{
			Void_Breath(false);
			OutputDebugStringA("[ScreenFX-DBG] Key9: BreathFlame OFF\n");
		}
		else
		{
			Void_Breath(true);
			OutputDebugStringA("[ScreenFX-DBG] Key9: BreathFlame ON\n");
		}
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
// Helper - state <-> string conversion
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
// JSON loading - dragon_patterns.json
// ─────────────────────────────────────────────────────────────────────────────
HRESULT CEnderDragon::Load_DragonPatterns(const char* pPath)
{
	ifstream file(pPath);
	if (!file.is_open()) return E_FAIL;

	json j;
	try { file >> j; }
	catch (...) { return E_FAIL; }

	// physics block -> DragonFlight field overrides
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

	// patterns array -> per-state parameters + transition rules
	if (j.contains("patterns"))
	{
		for (auto& pat : j["patterns"])
		{
			string id = pat.value("id", string(""));
			eEnderDragonState eSrc = StringToState(id);
			int iSrc = (int)eSrc;

			// Per-state parameter overrides
			if (id == "IDLE")
			{
				m_fMoveSpeed = pat.value("patrol_speed", 18.f);
			}
			else if (id == "ATTACK")
			{
				// chase_speed -> reflected in m_fMoveSpeed via Transition_State
				// Not stored separately here (already hardcoded in Transition_State)
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

			// Parse transition rules
			if (pat.contains("transition"))
			{
				m_TransRules[iSrc].clear();
				for (auto& rule : pat["transition"])
				{
					TransitionRule tr;
					tr.eTo = StringToState(rule.value("to", string("IDLE")));
					string cond = rule.value("condition", string(""));

					// Split by "&&" and parse each condition token
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
// Collision - bone AABB update + player collision detection
// ─────────────────────────────────────────────────────────────────────────────
void CEnderDragon::Update_BoneColliders()
{
	// Spine AABB (always updated)
	for (int i = 0; i < ENDER_DRAGON_SPINE_COUNT; ++i)
	{
		if (m_pSpineCollider[i])
			m_pSpineCollider[i]->Update_AABB(m_Spine[i].vPos);
	}

	// Tail AABB (active only in TAIL_ATTACK state)
	if (m_eState == eEnderDragonState::TAIL_ATTACK)
	{
		for (int i = 0; i < ENDER_DRAGON_TAIL_COUNT; ++i)
		{
			if (m_pTailCollider[i])
				m_pTailCollider[i]->Update_AABB(m_Tail[i].vPos);
		}
	}

	// Wing AABB (left side only, during downstroke)
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

	//// ── Spine AABB collision -> restitution impulse ──
	//for (int i = 0; i < ENDER_DRAGON_SPINE_COUNT; ++i)
	//{
	//	if (!m_pSpineCollider[i]) continue;
	//	if (!m_pSpineCollider[i]->IsColliding(tPlayerAABB)) continue;

	//	_vec3 vPlayerPos;
	//	CTransform* pTrans = dynamic_cast<CTransform*>(
	//		pPlayer->Get_Component(ID_DYNAMIC, L"Com_Transform"));
	//	if (!pTrans) break;
	//	pTrans->Get_Info(INFO_POS, &vPlayerPos);

	//	_vec3 vNormal = vPlayerPos - m_Spine[i].vPos;
	//	_float fLen = D3DXVec3Length(&vNormal);
	//	if (fLen < 0.01f) break;
	//	D3DXVec3Normalize(&vNormal, &vNormal);

	//	const _float fMassP = 80.f;
	//	const _float fMassD = m_Flight.fMass;
	//	_float fRelVel = D3DXVec3Dot(&m_vVelocity, &vNormal);
	//	if (fRelVel >= 0.f) break; // already separating

	//	_float fImpulse = -(1.f + m_Flight.fRestitution) * fRelVel
	//		/ (1.f / fMassP + 1.f / fMassD);

	//	// Dragon velocity change
	//	m_vVelocity += vNormal * (fImpulse / fMassD);

	//	pPlayer->Hit(5.f);
	//	break; // process only first collision
	//}

	//// ── Tail collision handled in Update_TailAttack (TAIL_ATTACK only) ──

	//// ── Wing collision (knockback during downstroke) ──
	//_float fCurSin = sinf(m_fWingTimer);
	//if (m_fPrevSinVal > 0.f && fCurSin <= 0.f) // downstroke phase
	//{
	//	for (int i = 0; i < ENDER_DRAGON_WING_COUNT; ++i)
	//	{
	//		if (!m_pWingLCollider[i]) continue;
	//		if (!m_pWingLCollider[i]->IsColliding(tPlayerAABB)) continue;

	//		pPlayer->Hit(12.f);
	//		break;
	//	}
	//}
}

// ─────────────────────────────────────────────────────────────────────────────
// Collision - dragon vs dragon restitution
// Usage (when multiple dragons exist, call pairwise once per frame in scene/manager):
//   for (int a = 0; a < vecDragons.size(); ++a)
//       for (int b = a+1; b < vecDragons.size(); ++b)
//           vecDragons[a]->Check_DragonCollision(vecDragons[b]);
// Currently only 1 dragon in scene, so no call site - API only.
// ─────────────────────────────────────────────────────────────────────────────
void CEnderDragon::Check_DragonCollision(CEnderDragon* pOther)
{
	if (!pOther || pOther == this) return;

	// Only check front body (Spine 0~3) for performance
	const int iCheckCount = 4;
	for (int i = 0; i < iCheckCount; ++i)
	{
		if (!m_pSpineCollider[i]) continue;
		AABB tMyAABB = m_pSpineCollider[i]->Get_AABB();

		for (int j = 0; j < iCheckCount; ++j)
		{
			CCollider* pOtherCol = pOther->Get_SpineCollider(j);
			if (!pOtherCol) continue;
			if (!pOtherCol->IsColliding(tMyAABB)) continue;

			// Collision detected! Compute restitution force
			_vec3 vNormal = m_Spine[0].vPos - pOther->Get_SpineRoot();
			_float fLen = D3DXVec3Length(&vNormal);
			if (fLen < 0.01f)
				vNormal = _vec3(1.f, 0.f, 0.f); // fallback
			else
				D3DXVec3Normalize(&vNormal, &vNormal);

			// Relative velocity
			_vec3 vRelVel = m_vVelocity - pOther->Get_Velocity();
			_float fRelVelN = D3DXVec3Dot(&vRelVel, &vNormal);

			if (fRelVelN >= 0.f) return; // already separating

			// Impulse calculation
			_float fMassA = m_Flight.fMass;
			_float fMassB = pOther->Get_Mass();
			_float fRestitution = m_Flight.fRestitution;

			_float fImpulse = -(1.f + fRestitution) * fRelVelN
				/ (1.f / fMassA + 1.f / fMassB);

			// Apply impulse to both sides
			m_vVelocity += vNormal * (fImpulse / fMassA);
			pOther->Add_Impulse(vNormal * (-fImpulse / fMassB));

			// Screen effect (light collision)
			CScreenFX* pFX = CScreenFX::GetInstance();
			if (pFX) pFX->Trigger_Wave(0.3f, 0.5f);

			return; // once per frame only
		}
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// ImGui debug panel
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

	// Physics parameters
	ImGui::Text("-- Physics --");
	ImGui::Text("Mass: %.1f  Gravity: %.1f", m_Flight.fMass, m_Flight.fGravity);
	ImGui::Text("Lift: %.2f  Drag: %.2f", m_Flight.fLiftCoeff, m_Flight.fDragCoeff);
	ImGui::Text("Thrust: %.1f  MaxSpeed: %.1f", m_Flight.fThrustForce, m_Flight.fMaxSpeed);
	ImGui::Text("Restitution: %.2f", m_Flight.fRestitution);

	ImGui::Separator();
	ImGui::Text("-- State Debug Keys --");
	ImGui::Text("H: IDLE  J: CIRCLE  K: TAIL  L: BREATH");

	// IDLE detection info
	if (m_eState == eEnderDragonState::IDLE)
	{
		ImGui::Text("DetectTimer: %.2f / %.2f", m_fDetectTimer, m_fDetectThreshold);
		ImGui::Text("DetectRange: %.1f", m_fDetectRange);
	}
	// CIRCLE attack alternation info
	if (m_eState == eEnderDragonState::CIRCLE_DIVE)
	{
		ImGui::Text("AttackPhase: %s", m_bCirclePhaseIsBeam ? "BEAM" : "VOIDFLAME");
		ImGui::Text("AttackTimer: %.2f", m_fCircleAttackTimer);
	}

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

	// Release breath flames
	for (auto& pFlame : m_vecBreathFlames)
		Safe_Release(pFlame);
	m_vecBreathFlames.clear();

	// Release colliders (Gotcha #6: Safe_Release)
	for (int i = 0; i < ENDER_DRAGON_SPINE_COUNT; ++i)
		Safe_Release(m_pSpineCollider[i]);
	for (int i = 0; i < ENDER_DRAGON_TAIL_COUNT; ++i)
		Safe_Release(m_pTailCollider[i]);
	for (int i = 0; i < ENDER_DRAGON_WING_COUNT; ++i)
		Safe_Release(m_pWingLCollider[i]);

	// Gotcha #7: base class call must be last
	CGameObject::Free();
}
