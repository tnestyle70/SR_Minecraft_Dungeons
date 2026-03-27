#include "pch.h"
//#include "pch.h"
//#include "CEnderDragon.h"
//#include "CRenderer.h"
//#include "CManagement.h"
//#include "CDInputMgr.h"
//#include <algorithm>
//
//// 순찰 지점 - 엔더드래곤 크기에 맞게 확장
//static const _vec3 s_PatrolPoints[] =
//{
//	_vec3(30.f, 22.f,  30.f),
//	_vec3(-30.f, 24.f,  30.f),
//	_vec3(-30.f, 22.f, -30.f),
//	_vec3(30.f, 24.f, -30.f)
//};
//
//// ─────────────────────────────────────────────────────────────────────────────
//// 생성 / 소멸
//// ─────────────────────────────────────────────────────────────────────────────
//CEnderDragon::CEnderDragon(LPDIRECT3DDEVICE9 pGraphicDev)
//	: CGameObject(pGraphicDev)
//	, m_vMoveTarget(0.f, 18.f, 20.f)
//	, m_vVelocity(0.f, 0.f, 0.f)
//	, m_fMoveSpeed(28.f)
//	, m_fWingTimer(0.f)
//	, m_fWingSpeed(3.2f)
//	, m_fWingAmp(D3DX_PI * 0.38f)
//	, m_eState(eDragonState::IDLE)
//	, m_fStateTimer(0.f)
//	, m_iPatrolIndex(0)
//	, m_fTailSwingTimer(0.f)
//	, m_fTailSwingAmp(D3DX_PI * 0.8f)
//	, m_vInputForward(0.f, 0.f, 1.f)
//	, m_vInputRight(1.f, 0.f, 0.f)
//	, m_vPlayerPos(0.f, 0.f, 0.f)
//{
//	ZeroMemory(m_Spine, sizeof(m_Spine));
//	ZeroMemory(m_Neck, sizeof(m_Neck));
//	ZeroMemory(&m_Head, sizeof(m_Head));
//	ZeroMemory(m_Tail, sizeof(m_Tail));
//	ZeroMemory(m_WingL, sizeof(m_WingL));
//	ZeroMemory(m_WingR, sizeof(m_WingR));
//
//	// ZeroMemory 후 쿼터니언 명시 초기화 (0,0,0,0은 유효하지 않은 쿼터니언)
//	for (int i = 0; i < DRAGON_SPINE_COUNT; ++i) D3DXQuaternionIdentity(&m_Spine[i].qRot);
//	for (int i = 0; i < DRAGON_NECK_COUNT; ++i) D3DXQuaternionIdentity(&m_Neck[i].qRot);
//	for (int i = 0; i < DRAGON_TAIL_COUNT; ++i) D3DXQuaternionIdentity(&m_Tail[i].qRot);
//	D3DXQuaternionIdentity(&m_Head.qRot);
//}
//
//CEnderDragon::CEnderDragon(const CEnderDragon& rhs)
//	: CGameObject(rhs)
//	, m_vMoveTarget(rhs.m_vMoveTarget)
//	, m_vVelocity(rhs.m_vVelocity)
//	, m_fMoveSpeed(rhs.m_fMoveSpeed)
//	, m_fWingTimer(rhs.m_fWingTimer)
//	, m_fWingSpeed(rhs.m_fWingSpeed)
//	, m_fWingAmp(rhs.m_fWingAmp)
//	, m_eState(rhs.m_eState)
//	, m_fStateTimer(rhs.m_fStateTimer)
//	, m_iPatrolIndex(rhs.m_iPatrolIndex)
//	, m_fTailSwingTimer(rhs.m_fTailSwingTimer)
//	, m_fTailSwingAmp(rhs.m_fTailSwingAmp)
//	, m_vInputForward(rhs.m_vInputForward)
//	, m_vInputRight(rhs.m_vInputRight)
//	, m_vPlayerPos(rhs.m_vPlayerPos)
//{
//	ZeroMemory(m_Spine, sizeof(m_Spine));
//	ZeroMemory(m_Neck, sizeof(m_Neck));
//	ZeroMemory(&m_Head, sizeof(m_Head));
//	ZeroMemory(m_Tail, sizeof(m_Tail));
//	ZeroMemory(m_WingL, sizeof(m_WingL));
//	ZeroMemory(m_WingR, sizeof(m_WingR));
//
//	for (int i = 0; i < DRAGON_SPINE_COUNT; ++i) D3DXQuaternionIdentity(&m_Spine[i].qRot);
//	for (int i = 0; i < DRAGON_NECK_COUNT; ++i) D3DXQuaternionIdentity(&m_Neck[i].qRot);
//	for (int i = 0; i < DRAGON_TAIL_COUNT; ++i) D3DXQuaternionIdentity(&m_Tail[i].qRot);
//	D3DXQuaternionIdentity(&m_Head.qRot);
//}
//
//CEnderDragon::~CEnderDragon() {}
//
//// ─────────────────────────────────────────────────────────────────────────────
//// 초기화
//// ─────────────────────────────────────────────────────────────────────────────
//HRESULT CEnderDragon::Ready_GameObject()
//{
//	if (FAILED(Init_SpineChain()))   return E_FAIL;
//	if (FAILED(Init_NeckAndHead()))  return E_FAIL;
//	if (FAILED(Init_TailChain()))    return E_FAIL;
//	if (FAILED(Init_WingChains()))   return E_FAIL;
//
//	for (_int i = 0; i < DRAGON_NECK_COUNT; ++i)
//		m_Neck[i].qRot = DirToQuaternion(m_Neck[i].vDir);
//	m_Head.qRot = DirToQuaternion(m_Head.vDir);
//
//	m_pTextureCom = dynamic_cast<CTexture*>(
//		CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_ObsidianPngTexture"));
//	if (!m_pTextureCom)
//	{
//		MSG_BOX("Dragon Texture Clone Failed");
//		return E_FAIL;
//	}
//	return S_OK;
//}
//
//HRESULT CEnderDragon::Create_BoneBuffer(DRAGON_BONE& bone,
//	_float fW, _float fH, _float fD, const FACE_UV& uv)
//{
//	CUBE cube{};
//	cube.fWidth = fW;
//	cube.fHeight = fH;
//	cube.fDepth = fD;
//	cube.front = cube.back = cube.top = cube.bottom = cube.left = cube.right = uv;
//
//	bone.pBuffer = CCubeBodyTex::Create(m_pGraphicDev, cube);
//	if (!bone.pBuffer) { MSG_BOX("Dragon BoneBuffer Create Failed"); return E_FAIL; }
//	D3DXMatrixIdentity(&bone.matWorld);
//	return S_OK;
//}
//
//HRESULT CEnderDragon::Create_FlexBoneBuffer(DRAGON_BONE& bone, const MESH& mesh)
//{
//	bone.pBuffer = CFlexibleCubeTex::Create(m_pGraphicDev, mesh);
//	if (!bone.pBuffer) { MSG_BOX("Dragon Flexible BoneBuffer Create Failed"); return E_FAIL; }
//	D3DXMatrixIdentity(&bone.matWorld);
//	return S_OK;
//}
//
//// ─────────────────────────────────────────────────────────────────────────────
//// Init: 척추 (9개, 기존 7개보다 큼)
//// ─────────────────────────────────────────────────────────────────────────────
//HRESULT CEnderDragon::Init_SpineChain()
//{
//	// Index 0 = 목 쪽(앞), 8 = 꼬리 쪽(뒤)
//	const _float fW[DRAGON_SPINE_COUNT] = { 3.2f, 3.0f, 2.8f, 2.6f, 2.4f, 2.1f, 1.8f, 1.5f, 1.2f };
//	const _float fH[DRAGON_SPINE_COUNT] = { 2.4f, 2.2f, 2.0f, 1.9f, 1.8f, 1.6f, 1.4f, 1.2f, 0.9f };
//	const _float fD[DRAGON_SPINE_COUNT] = { 2.4f, 2.2f, 2.0f, 1.9f, 1.8f, 1.6f, 1.4f, 1.2f, 0.9f };
//	const _float fBoneLen = 2.0f;
//
//	FACE_UV uv = { 0.f, 0.f, 1.f, 1.f };
//
//	for (int i = 0; i < DRAGON_SPINE_COUNT; ++i)
//	{
//		m_Spine[i].vPos = _vec3(0.f, 0.f, -(_float)i * fBoneLen);
//		m_Spine[i].vDir = _vec3(0.f, 0.f, 1.f);
//		m_Spine[i].fBoneLen = fBoneLen;
//		if (FAILED(Create_BoneBuffer(m_Spine[i], fW[i], fH[i], fD[i], uv))) return E_FAIL;
//	}
//	return S_OK;
//}
//
//// ─────────────────────────────────────────────────────────────────────────────
//// Init: 목(5개) + 머리
//// ─────────────────────────────────────────────────────────────────────────────
//HRESULT CEnderDragon::Init_NeckAndHead()
//{
//	const _float fNeckLen = 1.6f;
//	FACE_UV uv = { 0.f, 0.f, 1.f, 1.f };
//
//	// 목 두께: 루트에서 머리 쪽으로 좁아짐
//	const _float fNW[DRAGON_NECK_COUNT] = { 1.4f, 1.2f, 1.0f, 0.85f, 0.7f };
//	const _float fNH[DRAGON_NECK_COUNT] = { 1.3f, 1.1f, 0.95f, 0.8f, 0.65f };
//
//	for (int i = 0; i < DRAGON_NECK_COUNT; ++i)
//	{
//		_vec3 vBase = m_Spine[0].vPos;
//		m_Neck[i].vPos = _vec3(
//			vBase.x,
//			vBase.y + (_float)(i + 1) * 0.5f,
//			vBase.z + (_float)(i + 1) * fNeckLen);
//		m_Neck[i].vDir = _vec3(0.f, 0.f, 1.f);
//		m_Neck[i].fBoneLen = fNeckLen;
//		if (FAILED(Create_BoneBuffer(m_Neck[i], fNW[i], fNH[i], 1.4f, uv))) return E_FAIL;
//	}
//
//	// 머리 - 목 끝에 붙음
//	_vec3 vNeckEnd = m_Neck[DRAGON_NECK_COUNT - 1].vPos;
//	m_Head.vPos = _vec3(vNeckEnd.x, vNeckEnd.y + 0.5f, vNeckEnd.z + fNeckLen);
//	m_Head.vDir = _vec3(0.f, 0.f, 1.f);
//	m_Head.fBoneLen = 2.4f;
//	if (FAILED(Create_BoneBuffer(m_Head, 2.4f, 1.8f, 2.6f, uv))) return E_FAIL;
//
//	return S_OK;
//}
//
//// ─────────────────────────────────────────────────────────────────────────────
//// Init: 꼬리 (8개)
//// ─────────────────────────────────────────────────────────────────────────────
//HRESULT CEnderDragon::Init_TailChain()
//{
//	_vec3 vBase = m_Spine[DRAGON_SPINE_COUNT - 1].vPos;
//	const _float fTailLen = 1.4f;
//	FACE_UV uv = { 0.f, 0.f, 1.f, 1.f };
//
//	for (int i = 0; i < DRAGON_TAIL_COUNT; ++i)
//	{
//		_float fScale = 1.f - (_float)i * 0.10f;
//		m_Tail[i].vPos = _vec3(vBase.x, vBase.y, vBase.z - (_float)(i + 1) * fTailLen);
//		m_Tail[i].vDir = _vec3(0.f, 0.f, -1.f);
//		m_Tail[i].fBoneLen = fTailLen;
//		if (FAILED(Create_BoneBuffer(m_Tail[i],
//			fScale * 1.4f, fScale * 1.4f, fScale * 1.4f, uv))) return E_FAIL;
//	}
//	return S_OK;
//}
//
//// ─────────────────────────────────────────────────────────────────────────────
//// Init: 날개 (6세그먼트, 칼날 테이퍼)
//// ─────────────────────────────────────────────────────────────────────────────
//HRESULT CEnderDragon::Init_WingChains()
//{
//	_vec3 vWingRoot = m_Spine[2].vPos;
//	const _float fWingLen = 3.0f;
//	const _float hd = fWingLen * 0.5f; // 로컬 Z 절반
//	FACE_UV uv = { 0.f, 0.f, 1.f, 1.f };
//
//	// fwd < 0 : +X 방향 날 끝 (상향 칼날)
//	// bkd     : 등날 (완만한 테이퍼)
//	// hh      : 두께 절반
//	struct BladeFace { float fwd, bkd, hh; };
//
//	// outer[i] = inner[i+1] → 세그먼트 간 이음새 연속성 보장
//	const BladeFace inner[DRAGON_WING_COUNT] = {
//		{ -5.6f, 0.80f, 0.18f },  // [0] 루트 (가장 넓음)
//		{ -4.8f, 0.68f, 0.15f },  // [1]
//		{ -4.0f, 0.56f, 0.12f },  // [2]
//		{ -3.0f, 0.42f, 0.09f },  // [3]
//		{ -2.0f, 0.28f, 0.06f },  // [4]
//		{ -1.0f, 0.16f, 0.03f },  // [5]
//	};
//	const BladeFace outer[DRAGON_WING_COUNT] = {
//		{ -4.8f, 0.68f, 0.15f },  // outer[0] = inner[1] ✓
//		{ -4.0f, 0.56f, 0.12f },  // outer[1] = inner[2] ✓
//		{ -3.0f, 0.42f, 0.09f },  // outer[2] = inner[3] ✓
//		{ -2.0f, 0.28f, 0.06f },  // outer[3] = inner[4] ✓
//		{ -1.0f, 0.16f, 0.03f },  // outer[4] = inner[5] ✓
//		{ -0.05f, 0.06f, 0.01f }, // outer[5] = 칼끝
//	};
//
//	for (_int i = 0; i < DRAGON_WING_COUNT; ++i)
//	{
//		_float fOfs = (_float)(i + 1) * fWingLen;
//		_float fZFwdR = (_float)i * (-0.5f); // 오른쪽: 끝으로 갈수록 -Z
//		_float fZFwdL = (_float)i * (+0.5f); // 왼쪽: 반대 방향
//
//		m_WingL[i].vPos = _vec3(vWingRoot.x - fOfs, vWingRoot.y, vWingRoot.z + fZFwdL);
//		m_WingL[i].vDir = _vec3(-1.f, 0.f, 0.f);
//		m_WingL[i].fBoneLen = fWingLen;
//
//		m_WingR[i].vPos = _vec3(vWingRoot.x + fOfs, vWingRoot.y, vWingRoot.z + fZFwdR);
//		m_WingR[i].vDir = _vec3(1.f, 0.f, 0.f);
//		m_WingR[i].fBoneLen = fWingLen;
//
//		// 코너 인덱스:
//		// [0..3] tip쪽 (Z=+hd, 날개끝)   [4..7] body쪽 (Z=-hd, 몸통)
//		MESH mesh{};
//		mesh.front = mesh.back = mesh.top = mesh.bottom = mesh.right = mesh.left = uv;
//
//		const BladeFace& o = outer[i]; // tip쪽 (좁음)
//		const BladeFace& n = inner[i]; // body쪽 (넓음)
//
//		mesh.corners[0] = { -o.fwd, +o.hh, +hd };
//		mesh.corners[1] = { +o.bkd, +o.hh, +hd };
//		mesh.corners[2] = { +o.bkd, -o.hh, +hd };
//		mesh.corners[3] = { -o.fwd, -o.hh, +hd };
//		mesh.corners[4] = { +n.bkd, +n.hh, -hd };
//		mesh.corners[5] = { -n.fwd, +n.hh, -hd };
//		mesh.corners[6] = { -n.fwd, -n.hh, -hd };
//		mesh.corners[7] = { +n.bkd, -n.hh, -hd };
//
//		// 왼쪽 날개: X 미러 + CCW 와인딩 복원
//		MESH meshL = mesh;
//		for (auto& c : meshL.corners) c.x = -c.x;
//		std::swap(meshL.corners[0], meshL.corners[1]);
//		std::swap(meshL.corners[2], meshL.corners[3]);
//		std::swap(meshL.corners[4], meshL.corners[5]);
//		std::swap(meshL.corners[6], meshL.corners[7]);
//
//		if (FAILED(Create_FlexBoneBuffer(m_WingL[i], meshL))) return E_FAIL;
//		if (FAILED(Create_FlexBoneBuffer(m_WingR[i], mesh)))  return E_FAIL;
//	}
//	return S_OK;
//}
//
//// ─────────────────────────────────────────────────────────────────────────────
//// Update
//// ─────────────────────────────────────────────────────────────────────────────
//_int CEnderDragon::Update_GameObject(const _float& fTimeDelta)
//{
//	// fTimeDelta 클램프: 씬 로드 등 첫 프레임 폭발 방지
//	const _float dt = min(fTimeDelta, 0.05f);
//
//	Handle_Input(dt);
//	m_fStateTimer += dt;
//
//	// ── 매 프레임 힘 초기화 ──────────────────────────────────────────
//	m_Flight.vAccumForce = _vec3(0.f, 0.f, 0.f);
//
//	// ── FSM: 스티어링 힘 누적 + Spine[0].vDir 갱신 ───────────────────
//	switch (m_eState)
//	{
//	case eDragonState::IDLE:        Update_IDLE(dt);        break;
//	case eDragonState::ATTACK:      Update_Attack(dt);      break;
//	case eDragonState::TAIL_ATTACK: Update_TailAttack(dt);  break;
//	default: break;
//	}
//
//	// ── 물리 힘 누적: 중력 + 항력 + 양력 ────────────────────────────
//	Accumulate_Forces(dt);
//
//	// ── 날개짓: sin 파형 + 다운스트로크 추력 ─────────────────────────
//	Update_WingFlap(dt);
//
//	// ── 물리 적분: F=ma → 속도 → 위치 ───────────────────────────────
//	Integrate_Physics(dt);
//
//	// ── 체인 해석 (Spine[0] 위치 확정 후) ────────────────────────────
//	Solve_FollowLeader(m_Spine, DRAGON_SPINE_COUNT);
//
//	// 목 루트 = Spine[0] 약간 위
//	m_Neck[0].vPos = m_Spine[0].vPos + _vec3(0.f, 0.5f, 0.f);
//
//	// 목/머리 CCD + Slerp
//	{
//		DRAGON_BONE combined[DRAGON_NECK_COUNT + 1];
//		for (int i = 0; i < DRAGON_NECK_COUNT; ++i) combined[i] = m_Neck[i];
//		combined[DRAGON_NECK_COUNT] = m_Head;
//
//		_vec3 vNeckTarget = m_vMoveTarget + _vec3(0.f, 1.5f, 0.f);
//		_float fCCDAngle = (m_eState == eDragonState::ATTACK)
//			? D3DX_PI * 0.45f : D3DX_PI * 0.2f;
//		_int   iCCDIter = (m_eState == eDragonState::ATTACK) ? 10 : 6;
//		Solve_CCD(combined, DRAGON_NECK_COUNT + 1, vNeckTarget, iCCDIter, fCCDAngle);
//
//		DRAGON_BONE current[DRAGON_NECK_COUNT + 1];
//		for (int i = 0; i < DRAGON_NECK_COUNT; ++i) current[i] = m_Neck[i];
//		current[DRAGON_NECK_COUNT] = m_Head;
//
//		_float fSlerpAlpha = (m_eState == eDragonState::ATTACK)
//			? dt * 8.f : dt * 3.f;
//		Slerp_NeckChain(combined, current, DRAGON_NECK_COUNT + 1, fSlerpAlpha);
//
//		for (int i = 0; i < DRAGON_NECK_COUNT; ++i) m_Neck[i] = current[i];
//		m_Head = current[DRAGON_NECK_COUNT];
//	}
//
//	// 꼬리 추종
//	m_Tail[0].vPos = m_Spine[DRAGON_SPINE_COUNT - 1].vPos;
//	Solve_FollowLeader(m_Tail, DRAGON_TAIL_COUNT);
//
//	// ── 행렬 갱신 ────────────────────────────────────────────────────
//	Update_ChainMatrices(m_Spine, DRAGON_SPINE_COUNT);
//	Update_ChainMatrices(m_Neck, DRAGON_NECK_COUNT);
//	Compute_BoneMatrix(m_Head);
//	Update_ChainMatrices(m_Tail, DRAGON_TAIL_COUNT);
//	Update_ChainMatrices(m_WingL, DRAGON_WING_COUNT);
//	Update_ChainMatrices(m_WingR, DRAGON_WING_COUNT);
//
//	// ── 뱅킹: 선회 시 Spine Roll 시각 효과 ───────────────────────────
//	Update_Banking(dt);
//
//	CRenderer::GetInstance()->Add_RenderGroup(RENDER_NONALPHA, this);
//	return CGameObject::Update_GameObject(fTimeDelta);
//}
//
//void CEnderDragon::LateUpdate_GameObject(const _float& fTimeDelta)
//{
//	CGameObject::LateUpdate_GameObject(fTimeDelta);
//}
//
//// ─────────────────────────────────────────────────────────────────────────────
//// 렌더
//// ─────────────────────────────────────────────────────────────────────────────
//void CEnderDragon::Render_GameObject()
//{
//	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
//	m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
//
//	if (m_pTextureCom) m_pTextureCom->Set_Texture(0);
//
//	Render_Chain(m_Spine, DRAGON_SPINE_COUNT);
//	Render_Chain(m_Neck, DRAGON_NECK_COUNT);
//
//	if (m_Head.pBuffer)
//	{
//		m_pGraphicDev->SetTransform(D3DTS_WORLD, &m_Head.matWorld);
//		m_Head.pBuffer->Render_Buffer();
//	}
//
//	Render_Chain(m_Tail, DRAGON_TAIL_COUNT);
//
//	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
//	Render_Chain(m_WingL, DRAGON_WING_COUNT);
//	Render_Chain(m_WingR, DRAGON_WING_COUNT);
//	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
//}
//
//void CEnderDragon::Set_RootPos(const _vec3 vPos)
//{
//
//}
//
//// ─────────────────────────────────────────────────────────────────────────────
//// 동역학 ① — 힘 누적 (중력 / 항력 / 양력)
//// ─────────────────────────────────────────────────────────────────────────────
//void CEnderDragon::Accumulate_Forces(const _float& fTimeDelta)
//{
//	// ① 중력 : F = -Y * m * g
//	m_Flight.vAccumForce += _vec3(0.f, -(m_Flight.fMass * m_Flight.fGravity), 0.f);
//
//	// ② 항력 : F = -normalize(v) * |v|² * Cd
//	//    빠를수록 저항이 급증 → 최대 속도를 자연스럽게 제한
//	_float fSpeed = D3DXVec3Length(&m_vVelocity);
//	if (fSpeed > 0.01f)
//	{
//		_vec3 vDragDir = -m_vVelocity;
//		D3DXVec3Normalize(&vDragDir, &vDragDir);
//		m_Flight.vAccumForce += vDragDir * (fSpeed * fSpeed * m_Flight.fDragCoeff);
//	}
//
//	// ③ 양력 : F = +Y * fwdSpeed² * Cl
//	//    전진 속도가 빠를수록 위로 뜨는 힘 발생
//	//    수평 순항 조건: fwdSpeed² * Cl = m * g
//	//    → Cl=2.8, m=120, g=12 → 순항 속도 ≈ sqrt(1440/2.8) ≈ 22.7 m/s
//	_vec3 vLook = m_Spine[0].vDir;
//	_float fFwdSpeed = D3DXVec3Dot(&m_vVelocity, &vLook);
//	if (fFwdSpeed > 0.f)
//	{
//		_float fLift = fFwdSpeed * fFwdSpeed * m_Flight.fLiftCoeff;
//		m_Flight.vAccumForce += _vec3(0.f, fLift, 0.f);
//	}
//}
//
//// ─────────────────────────────────────────────────────────────────────────────
//// 동역학 ② — 물리 적분 (F = ma → 속도 → 위치)
//// ─────────────────────────────────────────────────────────────────────────────
//void CEnderDragon::Integrate_Physics(const _float& fTimeDelta)
//{
//
//	// a = F / m
//	_vec3 vAccel = m_Flight.vAccumForce * (1.f / m_Flight.fMass);
//	m_vVelocity += vAccel * fTimeDelta;
//
//	// 최대 속도 클램프
//	_float fSpeed = D3DXVec3Length(&m_vVelocity);
//	if (fSpeed > m_Flight.fMaxSpeed)
//	{
//		D3DXVec3Normalize(&m_vVelocity, &m_vVelocity);
//		m_vVelocity *= m_Flight.fMaxSpeed;
//	}
//
//	// 위치 적분
//	m_Spine[0].vPos += m_vVelocity * fTimeDelta;
//
//	// 다음 프레임을 위해 누적 힘 초기화 (Accumulate_Forces가 매 프레임 시작에도 초기화함)
//	m_Flight.vAccumForce = _vec3(0.f, 0.f, 0.f);
//}
//
//// ─────────────────────────────────────────────────────────────────────────────
//// 동역학 ③ — 뱅킹 (선회 시 Spine 행렬에 Roll 추가, 순수 시각 효과)
//// ─────────────────────────────────────────────────────────────────────────────
//void CEnderDragon::Update_Banking(const _float& fTimeDelta)
//{
//	_vec3 vLookCur = m_Spine[0].vDir;
//
//	_float fSpeed = D3DXVec3Length(&m_vVelocity);
//
//	// 이전 Look 과의 외적 Y성분 = 수평 선회율 (양수: 오른쪽)
//	_vec3 vCross;
//	D3DXVec3Cross(&vCross, &m_vLookPrev, &vLookCur);
//	_float fTurnRate = vCross.y / max(fTimeDelta, 0.001f);
//
//	// 목표 뱅킹 각도 계산 + 클램프
//	_float fTargetBank = -fTurnRate * m_Flight.fBankFactor;
//	fTargetBank = max(-m_Flight.fMaxBankAngle, min(m_Flight.fMaxBankAngle, fTargetBank));
//
//	// 뱅킹 각도 부드럽게 보간
//	m_fBankAngle += (fTargetBank - m_fBankAngle) * min(fTimeDelta * 4.f, 1.f);
//	m_vLookPrev = vLookCur;
//
//	if (fabsf(m_fBankAngle) < 0.001f) return;
//
//	// Spine 행렬에 Roll 적용 (Look 축 기준 회전)
//	for (int i = 0; i < DRAGON_SPINE_COUNT; ++i)
//	{
//		_vec3 vLook(m_Spine[i].matWorld._31,
//			m_Spine[i].matWorld._32,
//			m_Spine[i].matWorld._33);
//		_matrix matRoll;
//		D3DXMatrixRotationAxis(&matRoll, &vLook, m_fBankAngle);
//
//		// 이동 성분 분리 후 회전 적용, 이동 성분 복원
//		_vec3 vPos(m_Spine[i].matWorld._41,
//			m_Spine[i].matWorld._42,
//			m_Spine[i].matWorld._43);
//		m_Spine[i].matWorld._41 = m_Spine[i].matWorld._42 = m_Spine[i].matWorld._43 = 0.f;
//		D3DXMatrixMultiply(&m_Spine[i].matWorld, &m_Spine[i].matWorld, &matRoll);
//		m_Spine[i].matWorld._41 = vPos.x;
//		m_Spine[i].matWorld._42 = vPos.y;
//		m_Spine[i].matWorld._43 = vPos.z;
//	}
//}
//
//// ─────────────────────────────────────────────────────────────────────────────
//// FSM — IDLE: 순찰 + 스티어링 힘 누적
//// ─────────────────────────────────────────────────────────────────────────────
//void CEnderDragon::Update_IDLE(const _float& fTimeDelta)
//{
//	if (!m_bManualControl && !m_bNetworkControlled)
//	{
//		_float fDist = D3DXVec3Length(&(m_vMoveTarget - m_Spine[0].vPos));
//		if (fDist < 2.5f)
//			m_iPatrolIndex = (m_iPatrolIndex + 1) % m_iPatrolCount;
//		m_vMoveTarget = s_PatrolPoints[m_iPatrolIndex];
//	}
//
//	_vec3 vToTarget = m_vMoveTarget - m_Spine[0].vPos;
//	_float fDist = D3DXVec3Length(&vToTarget);
//
//	if (fDist > 1.f)
//	{
//		_vec3 vDir;
//		D3DXVec3Normalize(&vDir, &vToTarget);
//		D3DXVec3Normalize(&m_Spine[0].vDir, &vDir);
//
//		// 스티어링 힘: 거리에 비례하되 최대치 클램프
//		// m * targetSpeed = 관성 기준 → 적절한 응답성
//		_float fSteerMag = min(fDist * m_Flight.fMass * 3.f,
//			m_Flight.fMass * m_fMoveSpeed);
//		m_Flight.vAccumForce += vDir * fSteerMag;
//	}
//}
//
//// ─────────────────────────────────────────────────────────────────────────────
//// FSM — ATTACK: 플레이어 추적 + 스티어링 힘 누적
//// ─────────────────────────────────────────────────────────────────────────────
//void CEnderDragon::Update_Attack(const _float& fTimeDelta)
//{
//	CComponent* pCom = CManagement::GetInstance()->Get_Component(
//		ID_DYNAMIC, L"GameLogic_Layer", L"Player", L"Com_Transform");
//	if (!pCom) return;
//
//	CTransform* pTransform = dynamic_cast<CTransform*>(pCom);
//	if (pTransform) pTransform->Get_Info(INFO_POS, &m_vPlayerPos);
//
//	m_vMoveTarget = m_vPlayerPos + _vec3(0.f, 4.f, 0.f);
//
//	_vec3  vToTarget = m_vMoveTarget - m_Spine[0].vPos;
//	_float fDist = D3DXVec3Length(&vToTarget);
//
//	if (fDist > 2.f)
//	{
//		_vec3 vDir;
//		D3DXVec3Normalize(&vDir, &vToTarget);
//		D3DXVec3Normalize(&m_Spine[0].vDir, &vDir);
//
//		// 공격 상태: 더 강한 스티어링 (4배)
//		_float fSteerMag = min(fDist * m_Flight.fMass * 5.f,
//			m_Flight.fMass * m_fMoveSpeed * 1.5f);
//		m_Flight.vAccumForce += vDir * fSteerMag;
//	}
//}
//
//void CEnderDragon::Update_TailAttack(const _float& fTimeDelta)
//{
//	// TODO: 꼬리 휘두르기 물리 구현
//}
//
//// ─────────────────────────────────────────────────────────────────────────────
//// 날개짓 — sin 파형 + 다운스트로크 추력
//// ─────────────────────────────────────────────────────────────────────────────
//void CEnderDragon::Update_WingFlap(const _float& fTimeDelta)
//{
//	m_fWingTimer += fTimeDelta * m_fWingSpeed;
//
//	// ── 다운스트로크 감지 ────────────────────────────────────────────
//	// sin 값이 양→음으로 영점 교차 = 날개가 최고점에서 아래로 내려치는 시작
//	_float fCurSin = sinf(m_fWingTimer);
//	if (m_fPrevSinVal > 0.05f && fCurSin <= 0.05f)
//	{
//		// 추력: 전진 방향 + 위 방향 성분
//		_vec3 vThrust = m_Spine[0].vDir * m_Flight.fThrustForce;
//		vThrust.y += m_Flight.fThrustForce * 0.55f; // 상승 성분
//		m_Flight.vAccumForce += vThrust;
//	}
//	m_fPrevSinVal = fCurSin;
//
//	// ── 날개 뼈 위치 계산 ────────────────────────────────────────────
//	_matrix matBodyRot = m_Spine[2].matWorld;
//	matBodyRot._41 = 0.f; matBodyRot._42 = 0.f; matBodyRot._43 = 0.f;
//
//	_vec3 vWingRoot = m_Spine[2].vPos;
//
//	for (int i = 0; i < DRAGON_WING_COUNT; ++i)
//	{
//		_float fPhase = (_float)i * 0.3f;
//		_float fAmplitudeMul = 1.f + (_float)i * 0.15f;
//		_float fAngle = m_fWingAmp * fAmplitudeMul * sinf(m_fWingTimer + fPhase);
//
//		_matrix matFlapL, matFlapR;
//		D3DXMatrixRotationZ(&matFlapL, fAngle);
//		D3DXMatrixRotationZ(&matFlapR, -fAngle);
//
//		_vec3 vPrevL = (i == 0) ? vWingRoot : m_WingL[i - 1].vPos;
//		_vec3 vPrevR = (i == 0) ? vWingRoot : m_WingR[i - 1].vPos;
//
//		_vec3 vBaseL = _vec3(-m_WingL[i].fBoneLen, 0.f, 0.f);
//		_vec3 vBaseR = _vec3(m_WingR[i].fBoneLen, 0.f, 0.f);
//
//		// 1단계: 로컬 플랩 회전
//		D3DXVec3TransformNormal(&vBaseL, &vBaseL, &matFlapL);
//		D3DXVec3TransformNormal(&vBaseR, &vBaseR, &matFlapR);
//
//		// 2단계: 몸통 회전 적용
//		_vec3 vWorldOffsetL, vWorldOffsetR;
//		D3DXVec3TransformNormal(&vWorldOffsetL, &vBaseL, &matBodyRot);
//		D3DXVec3TransformNormal(&vWorldOffsetR, &vBaseR, &matBodyRot);
//
//		m_WingL[i].vPos = vPrevL + vWorldOffsetL;
//		m_WingR[i].vPos = vPrevR + vWorldOffsetR;
//
//		D3DXVec3Normalize(&m_WingL[i].vDir, &vWorldOffsetL);
//		D3DXVec3Normalize(&m_WingR[i].vDir, &vWorldOffsetR);
//	}
//}
//
//void CEnderDragon::Update_TailSwing(const _float& fTimeDelta)
//{
//	// TODO
//}
//
//// ─────────────────────────────────────────────────────────────────────────────
//// IK — CCD
//// ─────────────────────────────────────────────────────────────────────────────
//void CEnderDragon::Solve_CCD(DRAGON_BONE* pChain, _int iCount,
//	const _vec3& vTarget, _int iMaxIter, _float fMaxAngle)
//{
//	for (_int iter = 0; iter < iMaxIter; ++iter)
//	{
//		for (_int i = iCount - 2; i >= 0; --i)
//		{
//			_vec3 vEnd = pChain[iCount - 1].vPos
//				+ pChain[iCount - 1].vDir * pChain[iCount - 1].fBoneLen;
//			_vec3 vToEnd = vEnd - pChain[i].vPos;
//			_vec3 vToTarget = vTarget - pChain[i].vPos;
//
//			if (D3DXVec3Length(&vToEnd) < 0.0001f) continue;
//			if (D3DXVec3Length(&vToTarget) < 0.0001f) continue;
//
//			D3DXVec3Normalize(&vToEnd, &vToEnd);
//			D3DXVec3Normalize(&vToTarget, &vToTarget);
//
//			_float fDot = max(-1.f, min(1.f, D3DXVec3Dot(&vToEnd, &vToTarget)));
//			_float fAngle = min(acosf(fDot), fMaxAngle);
//
//			_vec3 vAxis;
//			D3DXVec3Cross(&vAxis, &vToEnd, &vToTarget);
//			if (D3DXVec3Length(&vAxis) < 0.001f) continue;
//			D3DXVec3Normalize(&vAxis, &vAxis);
//
//			_matrix matRot;
//			D3DXMatrixRotationAxis(&matRot, &vAxis, fAngle);
//
//			for (_int j = i + 1; j < iCount; ++j)
//			{
//				_vec3 vRel = pChain[j].vPos - pChain[i].vPos;
//				D3DXVec3TransformCoord(&vRel, &vRel, &matRot);
//				pChain[j].vPos = pChain[i].vPos + vRel;
//				D3DXVec3TransformNormal(&pChain[j].vDir, &pChain[j].vDir, &matRot);
//				D3DXVec3Normalize(&pChain[j].vDir, &pChain[j].vDir);
//			}
//		}
//	}
//}
//
//// ─────────────────────────────────────────────────────────────────────────────
//// IK — Follow the Leader
//// ─────────────────────────────────────────────────────────────────────────────
//void CEnderDragon::Solve_FollowLeader(DRAGON_BONE* pChain, _int iCount)
//{
//	for (int i = 1; i < iCount; ++i)
//	{
//		_vec3 vDir = pChain[i].vPos - pChain[i - 1].vPos;
//		_float fLen = D3DXVec3Length(&vDir);
//
//		if (fLen < 0.001f)
//			vDir = pChain[i - 1].vDir * -1.f;
//		else
//			D3DXVec3Normalize(&vDir, &vDir);
//
//		pChain[i].vPos = pChain[i - 1].vPos + vDir * pChain[i].fBoneLen;
//		pChain[i].vDir = vDir * -1.f;
//	}
//}
//
//// ─────────────────────────────────────────────────────────────────────────────
//// 쿼터니언 유틸
//// ─────────────────────────────────────────────────────────────────────────────
//D3DXQUATERNION CEnderDragon::DirToQuaternion(const _vec3& vDir)
//{
//	_vec3 vZ = vDir;
//	if (D3DXVec3Length(&vZ) < 0.0001f)
//	{
//		D3DXQUATERNION q; D3DXQuaternionIdentity(&q); return q;
//	}
//	D3DXVec3Normalize(&vZ, &vZ);
//
//	_vec3 vWorldUp(0.f, 1.f, 0.f);
//	if (fabsf(D3DXVec3Dot(&vZ, &vWorldUp)) > 0.99f)
//		vWorldUp = _vec3(1.f, 0.f, 0.f);
//
//	_vec3 vX, vY;
//	D3DXVec3Cross(&vX, &vWorldUp, &vZ); D3DXVec3Normalize(&vX, &vX);
//	D3DXVec3Cross(&vY, &vZ, &vX);       D3DXVec3Normalize(&vY, &vY);
//
//	_matrix mat; D3DXMatrixIdentity(&mat);
//	mat._11 = vX.x; mat._12 = vX.y; mat._13 = vX.z;
//	mat._21 = vY.x; mat._22 = vY.y; mat._23 = vY.z;
//	mat._31 = vZ.x; mat._32 = vZ.y; mat._33 = vZ.z;
//
//	D3DXQUATERNION q;
//	D3DXQuaternionRotationMatrix(&q, &mat);
//	D3DXQuaternionNormalize(&q, &q);
//	return q;
//}
//
//_vec3 CEnderDragon::QuaternionToDir(const D3DXQUATERNION& quat)
//{
//	_matrix mat;
//	D3DXMatrixRotationQuaternion(&mat, &quat);
//	return _vec3(mat._31, mat._32, mat._33);
//}
//
//void CEnderDragon::Slerp_NeckChain(DRAGON_BONE* pResult,
//	DRAGON_BONE* pCurrent, _int iCount, _float fAlpha)
//{
//	for (int i = 0; i < iCount; ++i)
//	{
//		D3DXQUATERNION qTarget = DirToQuaternion(pResult[i].vDir);
//		if (D3DXQuaternionDot(&pCurrent[i].qRot, &qTarget) < 0.f)
//			qTarget = -qTarget;
//
//		D3DXQUATERNION qResult;
//		D3DXQuaternionSlerp(&qResult, &pCurrent[i].qRot, &qTarget, fAlpha);
//		D3DXQuaternionNormalize(&qResult, &qResult);
//
//		pCurrent[i].qRot = qResult;
//		pCurrent[i].vDir = QuaternionToDir(qResult);
//
//		if (i > 0)
//			pCurrent[i].vPos = pCurrent[i - 1].vPos
//			+ pCurrent[i].vDir * pCurrent[i].fBoneLen;
//	}
//}
//
//// ─────────────────────────────────────────────────────────────────────────────
//// 뼈 행렬 계산 (Look-At 방식, DX9 Row-Major)
//// ─────────────────────────────────────────────────────────────────────────────
//void CEnderDragon::Compute_BoneMatrix(DRAGON_BONE& bone)
//{
//	_vec3 vZ = bone.vDir;
//	if (D3DXVec3Length(&vZ) < 0.001f)
//	{
//		D3DXMatrixTranslation(&bone.matWorld,
//			bone.vPos.x, bone.vPos.y, bone.vPos.z);
//		return;
//	}
//	D3DXVec3Normalize(&vZ, &vZ);
//
//	_vec3 vWorldUp(0.f, 1.f, 0.f);
//	if (fabsf(D3DXVec3Dot(&vZ, &vWorldUp)) > 0.99f)
//		vWorldUp = _vec3(1.f, 0.f, 0.f);
//
//	_vec3 vX, vY;
//	D3DXVec3Cross(&vX, &vWorldUp, &vZ); D3DXVec3Normalize(&vX, &vX);
//	D3DXVec3Cross(&vY, &vZ, &vX);       D3DXVec3Normalize(&vY, &vY);
//
//	bone.matWorld._11 = vX.x; bone.matWorld._12 = vX.y; bone.matWorld._13 = vX.z; bone.matWorld._14 = 0.f;
//	bone.matWorld._21 = vY.x; bone.matWorld._22 = vY.y; bone.matWorld._23 = vY.z; bone.matWorld._24 = 0.f;
//	bone.matWorld._31 = vZ.x; bone.matWorld._32 = vZ.y; bone.matWorld._33 = vZ.z; bone.matWorld._34 = 0.f;
//	bone.matWorld._41 = bone.vPos.x;
//	bone.matWorld._42 = bone.vPos.y;
//	bone.matWorld._43 = bone.vPos.z;
//	bone.matWorld._44 = 1.f;
//}
//
//void CEnderDragon::Update_ChainMatrices(DRAGON_BONE* pChain, _int iCount)
//{
//	for (int i = 0; i < iCount; ++i) Compute_BoneMatrix(pChain[i]);
//}
//
//void CEnderDragon::Render_Chain(DRAGON_BONE* pChain, _int iCount)
//{
//	for (int i = 0; i < iCount; ++i)
//	{
//		if (!pChain[i].pBuffer) continue;
//		m_pGraphicDev->SetTransform(D3DTS_WORLD, &pChain[i].matWorld);
//		pChain[i].pBuffer->Render_Buffer();
//	}
//}
//
//// ─────────────────────────────────────────────────────────────────────────────
//// FSM 전환
//// ─────────────────────────────────────────────────────────────────────────────
//void CEnderDragon::Transition_State(eDragonState eNext)
//{
//	if (m_eState == eNext) return;
//	m_eState = eNext;
//	m_fStateTimer = 0.f;
//
//	switch (eNext)
//	{
//	case eDragonState::IDLE:
//		m_fWingSpeed = 2.8f;
//		m_fWingAmp = D3DX_PI * 0.32f;
//		m_fMoveSpeed = 22.f;
//		m_vMoveTarget = s_PatrolPoints[m_iPatrolIndex];
//		break;
//	case eDragonState::ATTACK:
//		m_fWingSpeed = 5.5f;
//		m_fWingAmp = D3DX_PI * 0.48f;
//		m_fMoveSpeed = 32.f;
//		break;
//	case eDragonState::TAIL_ATTACK:
//		m_fTailSwingTimer = 0.f;
//		m_fTailSwingAmp = D3DX_PI * 0.85f;
//		m_fMoveSpeed = 12.f;
//		break;
//	default: break;
//	}
//}
//
//void CEnderDragon::Force_Idle_State()
//{
//	Transition_State(eDragonState::IDLE);
//}
//
//void CEnderDragon::Force_RootPos(const _vec3& vPos)
//{
//	m_Spine[0].vPos = vPos;
//	m_vMoveTarget = vPos;
//	m_vVelocity = _vec3(0.f, 0.f, 0.f);
//}
//
//void CEnderDragon::Handle_Input(const _float& fTimeDelta)
//{
//	// 필요 시 주석 해제
//}
//
//_float CEnderDragon::DistToPlayer() const
//{
//	CComponent* pComp = CManagement::GetInstance()->Get_Component(
//		ID_DYNAMIC, L"GameLogic_Layer", L"Player", L"Com_Transform");
//	if (!pComp) return FLT_MAX;
//
//	CTransform* pTrans = dynamic_cast<CTransform*>(pComp);
//	if (!pTrans) return FLT_MAX;
//
//	_vec3 vPlayerPos;
//	pTrans->Get_Info(INFO_POS, &vPlayerPos);
//	return D3DXVec3Length(&(vPlayerPos - m_Spine[0].vPos));
//}
//
//// ─────────────────────────────────────────────────────────────────────────────
//// Create / Free
//// ─────────────────────────────────────────────────────────────────────────────
//CEnderDragon* CEnderDragon::Create(LPDIRECT3DDEVICE9 pGraphicDev)
//{
//	CEnderDragon* pDragon = new CEnderDragon(pGraphicDev);
//	if (FAILED(pDragon->Ready_GameObject()))
//	{
//		Safe_Release(pDragon);
//		MSG_BOX("CEnderDragon Create Failed");
//		return nullptr;
//	}
//	return pDragon;
//}
//
//void CEnderDragon::Free()
//{
//	Safe_Release(m_pTextureCom);
//
//	auto ReleaseChain = [](DRAGON_BONE* pChain, _int iCount)
//		{
//			for (int i = 0; i < iCount; ++i) Safe_Release(pChain[i].pBuffer);
//		};
//
//	ReleaseChain(m_Spine, DRAGON_SPINE_COUNT);
//	ReleaseChain(m_Neck, DRAGON_NECK_COUNT);
//	Safe_Release(m_Head.pBuffer);
//	ReleaseChain(m_Tail, DRAGON_TAIL_COUNT);
//	ReleaseChain(m_WingL, DRAGON_WING_COUNT);
//	ReleaseChain(m_WingR, DRAGON_WING_COUNT);
//
//	CGameObject::Free();
//}
