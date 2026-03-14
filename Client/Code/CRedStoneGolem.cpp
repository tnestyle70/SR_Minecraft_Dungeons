#include "pch.h"
#include "CRedStoneGolem.h"
#include "CRenderer.h"

CRedStoneGolem::CRedStoneGolem(LPDIRECT3DDEVICE9 pGraphicDev)
	: CGameObject(pGraphicDev)
	, m_pTextureCom(nullptr)
	, m_pTransformCom(nullptr)
	, m_fWalkTime(0.f)
{
	ZeroMemory(m_pParts, sizeof(m_pParts));
}

CRedStoneGolem::CRedStoneGolem(const CRedStoneGolem& rhs)
	: CGameObject(rhs)
	, m_pTextureCom(nullptr)
	, m_pTransformCom(nullptr)
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

	Walk_Animation();

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

	m_pParts[GOLEM_BODY]->Set_Parent(m_pTransformCom);

	m_pParts[GOLEM_HEAD]->Set_Parent(pBody);
	m_pParts[GOLEM_CORE]->Set_Parent(pBody);
	m_pParts[GOLEM_LSHOULDER]->Set_Parent(pBody);
	m_pParts[GOLEM_RSHOULDER]->Set_Parent(pBody);
	m_pParts[GOLEM_HIP]->Set_Parent(pBody);

	m_pParts[GOLEM_LARM]->Set_Parent(pLShoulder);
	m_pParts[GOLEM_RARM]->Set_Parent(pRShoulder);
	m_pParts[GOLEM_LLEG]->Set_Parent(pHip);
	m_pParts[GOLEM_RLEG]->Set_Parent(pHip);
}

void CRedStoneGolem::Walk_Animation()
{
	_float fAngle = sinf(m_fWalkTime * 4.f) * 0.8f;

	m_pParts[GOLEM_BODY]->Get_Transform()->Rotation(ROT_X, fAngle * 0.2f);

	m_pParts[GOLEM_HEAD]->Get_Transform()->Rotation(ROT_X, fAngle * 0.2f);

	m_pParts[GOLEM_CORE]->Get_Transform()->Rotation(ROT_X, fAngle * 0.2f);

	m_pParts[GOLEM_LLEG]->Get_Transform()->Rotation(ROT_X, fAngle * 1.5f);
	m_pParts[GOLEM_RLEG]->Get_Transform()->Rotation(ROT_X, -fAngle * 1.5f);

	m_pParts[GOLEM_LSHOULDER]->Get_Transform()->Rotation(ROT_X, fAngle);
	m_pParts[GOLEM_RSHOULDER]->Get_Transform()->Rotation(ROT_X, -fAngle);

	m_pParts[GOLEM_LARM]->Get_Transform()->Rotation(ROT_X, fAngle);
	m_pParts[GOLEM_RARM]->Get_Transform()->Rotation(ROT_X, fAngle);
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