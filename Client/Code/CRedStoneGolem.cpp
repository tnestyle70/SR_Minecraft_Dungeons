#include "pch.h"
#include "CRedStoneGolem.h"
#include "CRenderer.h"
#include "CDInputMgr.h"

CRedStoneGolem::CRedStoneGolem(LPDIRECT3DDEVICE9 pGraphicDev)
	: CGameObject(pGraphicDev)
	, m_pTextureCom(nullptr)
	, m_pTransformCom(nullptr)
	, m_eState(GOLEM_STATE_IDLE)
	, m_fWalkTime(0.f)
{
	ZeroMemory(m_pParts, sizeof(m_pParts));
}

CRedStoneGolem::CRedStoneGolem(const CRedStoneGolem& rhs)
	: CGameObject(rhs)
	, m_pTextureCom(nullptr)
	, m_pTransformCom(nullptr)
	, m_eState(rhs.m_eState)
	, m_fWalkTime(rhs.m_fWalkTime)
{
	ZeroMemory(m_pParts, sizeof(m_pParts));
}

CRedStoneGolem::~CRedStoneGolem()
{
}

HRESULT CRedStoneGolem::Ready_GameObject()
{
	if (FAILED(Add_Component()))
		return E_FAIL;

	m_pTransformCom->Set_Pos(0.f, 10.f, 0.f);

	Set_PartsOffset();
	Set_DefaultScale();
	Set_WorldScale();
	Set_PartsParent();

	return S_OK;
}

_int CRedStoneGolem::Update_GameObject(const _float& fTimeDelta)
{
	m_fWalkTime += fTimeDelta;

	Debug_Input();
	Golem_Animation(fTimeDelta);

	_int iExit = CGameObject::Update_GameObject(fTimeDelta);

	for (int i = 0; i < GOLEM_END; ++i)
	{
		m_pParts[i]->Update_GameObject(fTimeDelta);
	}

	CRenderer::GetInstance()->Add_RenderGroup(RENDER_NONALPHA, this);

	return iExit;
}

void CRedStoneGolem::LateUpdate_GameObject(const _float& fTimeDelta)
{
	for (int i = 0; i < GOLEM_END; ++i)
	{
		if (m_pParts[i])
			m_pParts[i]->LateUpdate_GameObject(fTimeDelta);
	}
}

void CRedStoneGolem::Render_GameObject()
{
	m_pTextureCom->Set_Texture(0);
	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	for (_int i = 0; i < GOLEM_END; ++i)
	{
		m_pParts[i]->Render_GameObject();
	}

	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}

HRESULT CRedStoneGolem::Add_Component()
{
	Engine::CComponent* pComponent = nullptr;

	// Transform
	pComponent = m_pTransformCom = dynamic_cast<CTransform*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));

	if (pComponent == nullptr)
		return E_FAIL;

	m_mapComponent[ID_DYNAMIC].insert({ L"Com_Transform", pComponent });

	// Texture
	pComponent = m_pTextureCom = dynamic_cast<CTexture*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RedStoneGolemTexture"));

	if (pComponent == nullptr)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_Texture", pComponent });

	for (int i = 0; i < GOLEM_END; i++)
	{
		m_pParts[i] = CRedStoneGolemPart::Create(m_pGraphicDev, (REDSTONEGOLEM_PART)i);
	}

	return S_OK;
}

void CRedStoneGolem::Set_DefaultScale()
{
	for (_int i = 0; i < GOLEM_END; ++i)
	{
		switch (i)
		{
		case GOLEM_HEAD:
			m_pParts[i]->Get_Transform()->Set_Scale(0.45f);
			break;

		case GOLEM_BODY:
			m_pParts[i]->Get_Transform()->Set_Scale(1.0f);
			break;

		case GOLEM_CORE:
			m_pParts[i]->Get_Transform()->Set_Scale(0.45f);
			break;

		case GOLEM_LSHOULDER:
		case GOLEM_RSHOULDER:
			m_pParts[i]->Get_Transform()->Set_Scale(0.6f);
			break;

		case GOLEM_LARM:
		case GOLEM_RARM:
			m_pParts[i]->Get_Transform()->Set_Scale(0.6f);
			break;

		case GOLEM_HIP:
			m_pParts[i]->Get_Transform()->Set_Scale(0.6f);
			break;

		case GOLEM_LLEG:
		case GOLEM_RLEG:
			m_pParts[i]->Get_Transform()->Set_Scale(0.55f);
			break;

		default:
			break;
		}
	}
}

void CRedStoneGolem::Set_WorldScale()
{
	for (_int i = 0; i < GOLEM_END; ++i)
	{
		m_pParts[i]->Get_Transform()->Set_Scale(m_fWorldScale);
	}
}

void CRedStoneGolem::Set_PartsOffset()
{
	m_pParts[GOLEM_HEAD]->Set_LocalOffset({ 0.f * m_fWorldScale,  0.15f * m_fWorldScale, -0.42f * m_fWorldScale });
	m_pParts[GOLEM_BODY]->Set_LocalOffset({ 0.f * m_fWorldScale,  0.f * m_fWorldScale,    0.f * m_fWorldScale });
	m_pParts[GOLEM_CORE]->Set_LocalOffset({ 0.f * m_fWorldScale, -0.1f * m_fWorldScale,   0.15f * m_fWorldScale });
	m_pParts[GOLEM_LSHOULDER]->Set_LocalOffset({ 0.7f * m_fWorldScale,  0.f * m_fWorldScale,  0.f * m_fWorldScale });
	m_pParts[GOLEM_RSHOULDER]->Set_LocalOffset({ -0.7f * m_fWorldScale,  0.f * m_fWorldScale,  0.f * m_fWorldScale });
	m_pParts[GOLEM_LARM]->Set_LocalOffset({ 0.1f * m_fWorldScale, -0.6f * m_fWorldScale,  0.f * m_fWorldScale });
	m_pParts[GOLEM_RARM]->Set_LocalOffset({ -0.1f * m_fWorldScale, -0.6f * m_fWorldScale,  0.f * m_fWorldScale });
	m_pParts[GOLEM_HIP]->Set_LocalOffset({ 0.f * m_fWorldScale, -0.5f * m_fWorldScale,   0.f * m_fWorldScale });
	m_pParts[GOLEM_LLEG]->Set_LocalOffset({ 0.35f * m_fWorldScale, -0.4f * m_fWorldScale, 0.f * m_fWorldScale });
	m_pParts[GOLEM_RLEG]->Set_LocalOffset({ -0.35f * m_fWorldScale, -0.4f * m_fWorldScale, 0.f * m_fWorldScale });
}

void CRedStoneGolem::Set_PartsParent()
{
	CTransform* pBody = m_pParts[GOLEM_BODY]->Get_Transform();
	CTransform* pHip = m_pParts[GOLEM_HIP]->Get_Transform();
	CTransform* pLShoulder = m_pParts[GOLEM_LSHOULDER]->Get_Transform();
	CTransform* pRShoulder = m_pParts[GOLEM_RSHOULDER]->Get_Transform();

	m_pParts[GOLEM_BODY]->Get_Transform()->Set_Parent(m_pTransformCom);

	m_pParts[GOLEM_HEAD]->Get_Transform()->Set_Parent(pBody);
	m_pParts[GOLEM_CORE]->Get_Transform()->Set_Parent(pBody);
	m_pParts[GOLEM_LSHOULDER]->Get_Transform()->Set_Parent(pBody);
	m_pParts[GOLEM_RSHOULDER]->Get_Transform()->Set_Parent(pBody);
	m_pParts[GOLEM_HIP]->Get_Transform()->Set_Parent(pBody);

	m_pParts[GOLEM_LARM]->Get_Transform()->Set_Parent(pLShoulder);
	m_pParts[GOLEM_RARM]->Get_Transform()->Set_Parent(pRShoulder);
	m_pParts[GOLEM_LLEG]->Get_Transform()->Set_Parent(pHip);
	m_pParts[GOLEM_RLEG]->Get_Transform()->Set_Parent(pHip);
}

void CRedStoneGolem::Debug_Input()
{
	if (CDInputMgr::GetInstance()->Get_DIKeyState(DIK_9))
	{
		m_eState = GOLEM_STATE_IDLE;
	}

	if (CDInputMgr::GetInstance()->Get_DIKeyState(DIK_0))
	{
		m_eState = GOLEM_STATE_WALK;
	}
}

void CRedStoneGolem::Golem_Animation(const _float& fTimeDelta)
{
	switch (m_eState)
	{
	case GOLEM_STATE_IDLE:
		Idle_Animation();
		break;

	case GOLEM_STATE_WALK:
		Walk_Animation();
		break;

	case GOLEM_STATE_ATTACK:
		break;

	case GOLEM_STATE_END:
		break;

	default:
		break;
	}
}

void CRedStoneGolem::Idle_Animation()
{
	const _float fCycleSpeed = 1.2f;  // 걷기보다 느리게
	const _float fAngle = m_fWalkTime * fCycleSpeed;

	//m_pParts[GOLEM_BODY]->Get_Transform()->Rotation(ROT_X, sinf(fAngle) * D3DXToRadian(2.f));
	//m_pParts[GOLEM_BODY]->Get_Transform()->Rotation(ROT_Z, sinf(fAngle * 0.5f) * D3DXToRadian(1.f));

	m_pParts[GOLEM_LSHOULDER]->Get_Transform()->Rotation(ROT_X, sinf(fAngle) * D3DXToRadian(5.f));
	m_pParts[GOLEM_RSHOULDER]->Get_Transform()->Rotation(ROT_X, sinf(fAngle + D3DX_PI * 0.1f) * D3DXToRadian(5.f));

	m_pParts[GOLEM_LARM]->Get_Transform()->Rotation(ROT_X, sinf(fAngle + D3DX_PI * 0.2f) * D3DXToRadian(3.f));
	m_pParts[GOLEM_RARM]->Get_Transform()->Rotation(ROT_X, sinf(fAngle + D3DX_PI * 0.3f) * D3DXToRadian(3.f));

	m_pParts[GOLEM_HIP]->Get_Transform()->Rotation(ROT_Z, sinf(fAngle * 0.7f) * D3DXToRadian(1.5f));
	m_pParts[GOLEM_HIP]->Get_Transform()->Rotation(ROT_X, 0.f);

	m_pParts[GOLEM_LLEG]->Get_Transform()->Rotation(ROT_X, 0.f);
	m_pParts[GOLEM_RLEG]->Get_Transform()->Rotation(ROT_X, 0.f);

	//m_pParts[GOLEM_HEAD]->Get_Transform()->Rotation(ROT_Y, sinf(fAngle * 0.6f) * D3DXToRadian(15.f));
	//m_pParts[GOLEM_HEAD]->Get_Transform()->Rotation(ROT_X, sinf(fAngle * 0.8f) * D3DXToRadian(3.f));

	const _float fBobHeight = 0.03f * m_fWorldScale;
	m_pTransformCom->Set_Pos(0.f, 10.f + sinf(fAngle) * fBobHeight, 0.f);
}

void CRedStoneGolem::Walk_Animation()
{
	const _float fCycleSpeed = 2.5f;
	const _float fAngle = m_fWalkTime * fCycleSpeed;

	const _float fLegSwingRad = D3DXToRadian(25.f);

	m_pParts[GOLEM_LLEG]->Get_Transform()->Rotation(ROT_X, sinf(fAngle) * fLegSwingRad);
	m_pParts[GOLEM_RLEG]->Get_Transform()->Rotation(ROT_X, sinf(fAngle + D3DX_PI) * fLegSwingRad);

	const _float fArmSwingRad = D3DXToRadian(20.f);

	m_pParts[GOLEM_LSHOULDER]->Get_Transform()->Rotation(ROT_X, sinf(fAngle + D3DX_PI) * fArmSwingRad);
	m_pParts[GOLEM_RSHOULDER]->Get_Transform()->Rotation(ROT_X, sinf(fAngle) * fArmSwingRad);

	const _float fForeArmBendRad = D3DXToRadian(15.f);
	const _float fPhaseDelay = D3DX_PI * 0.3f;

	m_pParts[GOLEM_LARM]->Get_Transform()->Rotation(ROT_X, sinf(fAngle + D3DX_PI + fPhaseDelay) * fForeArmBendRad);
	m_pParts[GOLEM_RARM]->Get_Transform()->Rotation(ROT_X, sinf(fAngle + fPhaseDelay) * fForeArmBendRad);

	m_pParts[GOLEM_HIP]->Get_Transform()->Rotation(ROT_Z, sinf(fAngle) * D3DXToRadian(6.f));
	m_pParts[GOLEM_HIP]->Get_Transform()->Rotation(ROT_X, sinf(fAngle * 2.f) * D3DXToRadian(4.f));

	m_pParts[GOLEM_BODY]->Get_Transform()->Rotation(ROT_Z, sinf(fAngle + D3DX_PI) * D3DXToRadian(3.f));
	m_pParts[GOLEM_BODY]->Get_Transform()->Rotation(ROT_X, sinf(fAngle * 2.f + D3DX_PI) * D3DXToRadian(2.f));

	m_pParts[GOLEM_HEAD]->Get_Transform()->Rotation(ROT_X, sinf(fAngle * 2.f) * D3DXToRadian(4.f));
}

CRedStoneGolem* CRedStoneGolem::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
	CRedStoneGolem* pRedStoneGolem = new CRedStoneGolem(pGraphicDev);

	if (FAILED(pRedStoneGolem->Ready_GameObject()))
	{
		Safe_Release(pRedStoneGolem);
		MSG_BOX("RedStoneGolem Create Failed");
		return nullptr;
	}

	return pRedStoneGolem;
}

void CRedStoneGolem::Free()
{
	for (int i = 0; i < GOLEM_END; ++i)
	{
		Safe_Release(m_pParts[i]);
	}

	CGameObject::Free();
}