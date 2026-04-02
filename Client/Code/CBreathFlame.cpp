#include "pch.h"
#include "CBreathFlame.h"
#include "CScreenFX.h"

IMPLEMENT_SINGLETON(CBreathFlame)

CBreathFlame::CBreathFlame()
{
	D3DXMatrixIdentity(&m_matBeamWorld);
}
CBreathFlame::~CBreathFlame() { Free(); }

void CBreathFlame::Activate(LPDIRECT3DDEVICE9 pDev, float fBeamRadius, float fBeamLength)
{
	if (m_bActive) return;

	if (m_pGraphicDev != pDev)
	{
		if (m_pGraphicDev) m_pGraphicDev->Release();
		m_pGraphicDev = pDev;
		if (m_pGraphicDev) m_pGraphicDev->AddRef();
	}

	m_bActive     = true;
	m_fBeamRadius = fBeamRadius;
	m_fBeamLength = fBeamLength;
	m_fGrowthTimer = 0.f;

	if (!m_pBeamBuffer)
		Create_BeamMesh();
}

void CBreathFlame::Deactivate()
{
	m_bActive = false;
	m_fGrowthTimer = 0.f;
}

HRESULT CBreathFlame::Create_BeamMesh()
{
	if (!m_pGraphicDev) return E_FAIL;

	// Beam cube: diameter = fBeamRadius*2, length along Z
	CUBE cube{};
	cube.fWidth  = m_fBeamRadius * 2.f;
	cube.fHeight = m_fBeamRadius * 2.f;
	cube.fDepth  = m_fBeamLength;

	FACE_UV uv = { 0.f, 0.f, 1.f, 1.f };
	cube.front = cube.back = cube.top = cube.bottom = cube.left = cube.right = uv;

	m_pBeamBuffer = Engine::CCubeBodyTex::Create(m_pGraphicDev, cube);
	if (!m_pBeamBuffer)
	{
		MSG_BOX("CBreathFlame BeamMesh Create Failed");
		return E_FAIL;
	}
	return S_OK;
}

void CBreathFlame::Update(const _float& fTimeDelta,
	const _vec3& vHeadPos,
	const _vec3& vHeadDir)
{
	if (!m_bActive) return;

	m_fGrowthTimer += fTimeDelta;
	m_fTime += fTimeDelta;

	float fGrowth = min(1.f, m_fGrowthTimer / 1.0f);

	// Build beam world matrix from head direction
	_vec3 vZ = vHeadDir;
	if (D3DXVec3Length(&vZ) < 0.001f)
		vZ = _vec3(0.f, 0.f, 1.f);
	D3DXVec3Normalize(&vZ, &vZ);

	_vec3 vWorldUp(0.f, 1.f, 0.f);
	if (fabsf(D3DXVec3Dot(&vZ, &vWorldUp)) > 0.99f)
		vWorldUp = _vec3(1.f, 0.f, 0.f);

	_vec3 vX, vY;
	D3DXVec3Cross(&vX, &vWorldUp, &vZ);
	D3DXVec3Normalize(&vX, &vX);
	D3DXVec3Cross(&vY, &vZ, &vX);
	D3DXVec3Normalize(&vY, &vY);

	// Position: head + half beam length forward (cube is centered at origin)
	float fCurrentLen = m_fBeamLength * fGrowth;
	_vec3 vCenter = vHeadPos + vZ * (fCurrentLen * 0.5f);

	// Scale Z by growth ratio
	_matrix matScale;
	D3DXMatrixScaling(&matScale, 1.f, 1.f, fGrowth);

	// Rotation matrix (look-at from head direction)
	_matrix matRot;
	D3DXMatrixIdentity(&matRot);
	matRot._11 = vX.x; matRot._12 = vX.y; matRot._13 = vX.z; matRot._14 = 0.f;
	matRot._21 = vY.x; matRot._22 = vY.y; matRot._23 = vY.z; matRot._24 = 0.f;
	matRot._31 = vZ.x; matRot._32 = vZ.y; matRot._33 = vZ.z; matRot._34 = 0.f;
	matRot._41 = 0.f;  matRot._42 = 0.f;  matRot._43 = 0.f;  matRot._44 = 1.f;

	_matrix matTrans;
	D3DXMatrixTranslation(&matTrans, vCenter.x, vCenter.y, vCenter.z);

	m_matBeamWorld = matScale * matRot * matTrans;
}

void CBreathFlame::Render()
{
	if (!m_bActive || !m_pBeamBuffer || !m_pGraphicDev) return;

	CScreenFX* pFX = CScreenFX::GetInstance();
	if (!pFX) return;

	ID3DXEffect* pEffect = pFX->Get_Effect();
	if (!pEffect) return;

	_matrix matView, matProj;
	m_pGraphicDev->GetTransform(D3DTS_VIEW, &matView);
	m_pGraphicDev->GetTransform(D3DTS_PROJECTION, &matProj);

	const _matrix matWVP = m_matBeamWorld * matView * matProj;
	const _matrix matWV  = m_matBeamWorld * matView;

	// Save world + FVF/stream (Effect does not restore these; same idea as CScreenFX::Apply_VoidFlame)
	_matrix matWorldPrev;
	m_pGraphicDev->GetTransform(D3DTS_WORLD, &matWorldPrev);

	DWORD dwPrevFVF = 0;
	IDirect3DVertexBuffer9* pPrevVB = nullptr;
	UINT uPrevOffset = 0, uPrevStride = 0;
	m_pGraphicDev->GetFVF(&dwPrevFVF);
	m_pGraphicDev->GetStreamSource(0, &pPrevVB, &uPrevOffset, &uPrevStride);

	// Additive beam: no world SetTransform — VS uses matWVP only (avoids double transform / dirty world).
	// Z off: depth test was clipping the beam against terrain/dragon; glow reads as one clean ray.
	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_pGraphicDev->SetRenderState(D3DRS_ZENABLE, FALSE);
	m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_pGraphicDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	m_pGraphicDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
	m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	m_pGraphicDev->SetRenderState(D3DRS_LIGHTING, FALSE);

	pEffect->SetTechnique("TechBreathBeam");
	pEffect->SetMatrix("matWVP", &matWVP);
	pEffect->SetMatrix("matWV", &matWV);
	pEffect->SetFloat("fTime", m_fTime);
	pEffect->SetFloat("fFlameAmt", min(1.f, m_fGrowthTimer / 0.3f));
	pEffect->SetTexture("NoiseMap", pFX->Get_NoiseTex());

	UINT passes = 0;
	pEffect->Begin(&passes, 0);
	for (UINT p = 0; p < passes; ++p)
	{
		pEffect->BeginPass(p);
		m_pBeamBuffer->Render_Buffer();
		pEffect->EndPass();
	}
	pEffect->End();

	m_pGraphicDev->SetStreamSource(0, pPrevVB, uPrevOffset, uPrevStride);
	m_pGraphicDev->SetFVF(dwPrevFVF);
	if (pPrevVB) pPrevVB->Release();

	m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorldPrev);

	pEffect->SetTechnique("PostProcess");
	m_pGraphicDev->SetTexture(0, nullptr);
	m_pGraphicDev->SetTexture(1, nullptr);

	// Match CEnderDragon::Render_GameObject baseline after opaque meshes (Z on, Z write on, no blend)
	m_pGraphicDev->SetRenderState(D3DRS_ZENABLE, TRUE);
	m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
}

void CBreathFlame::Free()
{
	if (m_pBeamBuffer) { m_pBeamBuffer->Release(); m_pBeamBuffer = nullptr; }
	if (m_pGraphicDev) { m_pGraphicDev->Release(); m_pGraphicDev = nullptr; }
}
