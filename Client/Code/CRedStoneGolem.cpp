#include "pch.h"
#include "CRedStoneGolem.h"
#include "CRenderer.h"

CRedStoneGolem::CRedStoneGolem(LPDIRECT3DDEVICE9 pGraphicDev)
	: CGameObject(pGraphicDev)
	, m_pTextureCom(nullptr)
{
	ZeroMemory(m_pParts, sizeof(m_pParts));
}

CRedStoneGolem::CRedStoneGolem(const CRedStoneGolem& rhs)
	: CGameObject(rhs)
	, m_pTextureCom(nullptr)
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

	Set_DefaultScale();

	return S_OK;
}

_int CRedStoneGolem::Update_GameObject(const _float& fTimeDelta)
{
	Set_PartsPos();

	for (int i = 0; i < GOLEM_END; ++i)
	{
		if (m_pParts[i])
			m_pParts[i]->Update_GameObject(fTimeDelta);
	}

	CRenderer::GetInstance()->Add_RenderGroup(RENDER_NONALPHA, this);

	return 0;
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

void CRedStoneGolem::Set_PartsPos()
{
	// Root
	_vec3 vBodyPos(0.f, 10.f, 0.f);

	for (int i = 0; i < GOLEM_END; ++i)
	{
		switch (i)
		{
		case GOLEM_HEAD:
			m_pParts[i]->Get_Transform()->Set_Pos(vBodyPos.x, vBodyPos.y + 0.15f, vBodyPos.z - 0.42f);
			break;

		case GOLEM_BODY:
			m_pParts[i]->Get_Transform()->Set_Pos(vBodyPos.x, vBodyPos.y, vBodyPos.z);
			break;

		case GOLEM_LSHOULDER:
			m_pParts[i]->Get_Transform()->Set_Pos(vBodyPos.x + 0.7f, vBodyPos.y, vBodyPos.z);
			break;

		case GOLEM_RSHOULDER:
			m_pParts[i]->Get_Transform()->Set_Pos(vBodyPos.x - 0.7f, vBodyPos.y, vBodyPos.z);
			break;

		case GOLEM_HIP:
			m_pParts[i]->Get_Transform()->Set_Pos(vBodyPos.x, vBodyPos.y - 0.5f, vBodyPos.z);
			break;

		case GOLEM_CORE:
			m_pParts[i]->Get_Transform()->Set_Pos(vBodyPos.x, vBodyPos.y - 0.1f, vBodyPos.z + 0.15f);
			break;

		case GOLEM_LARM:
			m_pParts[i]->Get_Transform()->Set_Pos(vBodyPos.x + 0.8f, vBodyPos.y - 0.6f, vBodyPos.z);
			break;

		case GOLEM_RARM:
			m_pParts[i]->Get_Transform()->Set_Pos(vBodyPos.x - 0.8f, vBodyPos.y - 0.6f, vBodyPos.z);
			break;

		case GOLEM_LLEG:
			m_pParts[i]->Get_Transform()->Set_Pos(vBodyPos.x + 0.35f, vBodyPos.y - 0.9f, vBodyPos.z);
			break;

		case GOLEM_RLEG:
			m_pParts[i]->Get_Transform()->Set_Pos(vBodyPos.x - 0.35f, vBodyPos.y - 0.9f, vBodyPos.z);
			break;

		default:
			break;
		}
	}
}

void CRedStoneGolem::Set_DefaultScale()
{
	for (int i = 0; i < GOLEM_END; ++i)
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