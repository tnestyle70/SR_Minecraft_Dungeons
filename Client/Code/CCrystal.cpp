#include "pch.h"
#include "CCrystal.h"
#include "CRenderer.h"

CCrystal::CCrystal(LPDIRECT3DDEVICE9 pGraphicDev)
	: CGameObject(pGraphicDev)
	, m_pTransformCom(nullptr)
	, m_pTextureCom(nullptr)
	, m_pColliderCom(nullptr)
{
	ZeroMemory(m_pParts, sizeof(m_pParts));
}

CCrystal::~CCrystal()
{
}

HRESULT CCrystal::Ready_GameObject()
{
	if (FAILED(Add_Component()))
		return E_FAIL;

	m_pTransformCom->Set_Pos(0.f, 10.f, 0.f);

	Set_PartsOffset();
	Set_WorldScale();
	Set_PartsParent();

	return S_OK;
}

_int CCrystal::Update_GameObject(const _float& fTimeDelta)
{
	_int iExit = CGameObject::Update_GameObject(fTimeDelta);

	for (_int i = 0; i < CRYSTAL_END; ++i)
	{
		m_pParts[i]->Update_GameObject(fTimeDelta);
	}

	_vec3 vPos;
	m_pTransformCom->Get_Info(INFO_POS, &vPos);

	m_pColliderCom->Update_AABB(vPos);

	CRenderer::GetInstance()->Add_RenderGroup(RENDER_NONALPHA, this);

	return iExit;
}

void CCrystal::LateUpdate_GameObject(const _float& fTimeDelta)
{
	for (_int i = 0; i < CRYSTAL_END; ++i)
	{
		m_pParts[i]->Update_GameObject(fTimeDelta);
	}

	CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CCrystal::Render_GameObject()
{
	m_pTextureCom->Set_Texture(0);
	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	for (_int i = 0; i < CRYSTAL_END; ++i)
	{
		m_pParts[i]->Render_GameObject();
	}

	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}

HRESULT CCrystal::Add_Component()
{
	CComponent* pComponent = nullptr;

	// Transform
	pComponent = m_pTransformCom = dynamic_cast<CTransform*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));

	if (!pComponent)
		return E_FAIL;

	m_mapComponent[ID_DYNAMIC].insert({ { L"Com_Transform", pComponent } });

	// Texture
	pComponent = m_pTextureCom = dynamic_cast<CTexture*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_CrystalTexture"));

	if (!pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ { L"Com_Texture", pComponent } });

	// Collider
	m_pColliderCom = CCollider::Create(m_pGraphicDev, _vec3(1.f, 1.f, 1.f), _vec3(0.f, 0.f, 0.f));

	m_mapComponent[ID_STATIC].insert({ L"Com_Collider", m_pColliderCom });

	// Create Child
	for (int i = 0; i < CRYSTAL_END; i++)
	{
		m_pParts[i] = CCrystalPart::Create(m_pGraphicDev, (CRYSTAL_PART)i);
	}

	return S_OK;
}

void CCrystal::Set_PartsOffset()
{
	_vec3 vRootPos;

	m_pTransformCom->Get_Info(INFO_POS, &vRootPos);

	m_pParts[CRYSTAL_8x20]->Get_Transform()->Set_Pos(vRootPos.x, vRootPos.y, vRootPos.z);
	m_pParts[CRYSTAL_5x14]->Get_Transform()->Set_Pos(vRootPos.x + 0.325f, vRootPos.y - 0.185f, vRootPos.z);
	m_pParts[CRYSTAL_4x11]->Get_Transform()->Set_Pos(vRootPos.x - 0.3f, vRootPos.y - 0.2f, vRootPos.z);
	m_pParts[CRYSTAL_3x15]->Get_Transform()->Set_Pos(vRootPos.x, vRootPos.y - 0.16f, vRootPos.z + 0.28f);
	m_pParts[CRYSTAL_2x13]->Get_Transform()->Set_Pos(vRootPos.x, vRootPos.y - 0.2f, vRootPos.z - 0.2f);
	m_pParts[CRYSTAL_3x5]->Get_Transform()->Set_Pos(vRootPos.x - 0.2f, vRootPos.y - 0.3f, vRootPos.z - 0.28f);
}

void CCrystal::Set_WorldScale()
{
	m_pParts[CRYSTAL_8x20]->Get_Transform()->Set_Scale(20.f / 20.f);
	m_pParts[CRYSTAL_5x14]->Get_Transform()->Set_Scale(14.f / 20.f);
	m_pParts[CRYSTAL_4x11]->Get_Transform()->Set_Scale(11.f / 20.f);
	m_pParts[CRYSTAL_3x15]->Get_Transform()->Set_Scale(15.f / 20.f);
	m_pParts[CRYSTAL_2x13]->Get_Transform()->Set_Scale(13.f / 20.f);
	m_pParts[CRYSTAL_3x5]->Get_Transform()->Set_Scale(5.f / 20.f);
}

void CCrystal::Set_PartsParent()
{
	for (_int i = 0; i < CRYSTAL_END; ++i)
	{
		m_pParts[i]->Get_Transform()->Set_Parent(m_pTransformCom);
	}
}

CCrystal* CCrystal::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
	CCrystal* pCrystal = new CCrystal(pGraphicDev);

	if (FAILED(pCrystal->Ready_GameObject()))
	{
		Safe_Release(pCrystal);
		MSG_BOX("Crystal Create Failed");
		return nullptr;
	}

	return pCrystal;
}

void CCrystal::Free()
{
	for (_int i = 0; i < CRYSTAL_END; ++i)
	{
		Safe_Release(m_pParts[i]);
	}

	CGameObject::Free();
}