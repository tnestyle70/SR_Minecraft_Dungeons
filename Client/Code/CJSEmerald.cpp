#include "pch.h"
#include "CJSEmerald.h"
#include "CRenderer.h"

CJSEmerald::CJSEmerald(LPDIRECT3DDEVICE9 pGraphicDev)
	: CGameObject(pGraphicDev)
	, m_pBufferCom(nullptr)
	, m_pTransformCom(nullptr)
	, m_pTextureCom(nullptr)
{
}

CJSEmerald::~CJSEmerald()
{
}

HRESULT CJSEmerald::Ready_GameObject(_vec3 vPos)
{
	if (FAILED(Add_Component()))
		return E_FAIL;

	m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
	m_pTransformCom->Update_Component(0.f);

	return S_OK;
}

_int CJSEmerald::Update_GameObject(const _float& fTimeDelta)
{
	_int iExit = CGameObject::Update_GameObject(fTimeDelta);

	return iExit;
}

void CJSEmerald::LateUpdate_GameObject(const _float& fTimeDelta)
{
	CGameObject::LateUpdate_GameObject(fTimeDelta);

	CRenderer::GetInstance()->Add_RenderGroup(RENDER_ALPHA, this);
}

void CJSEmerald::Render_GameObject()
{
	_matrix matWorld, matView;
	m_pGraphicDev->GetTransform(D3DTS_VIEW, &matView);
	D3DXMatrixInverse(&matWorld, 0, &matView);

	_vec3 vPos;
	m_pTransformCom->Get_Info(INFO_POS, &vPos);
	matWorld._41 = vPos.x;
	matWorld._42 = vPos.y;
	matWorld._43 = vPos.z;

	m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);

	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_pTextureCom->Set_Texture(0);
	m_pBufferCom->Render_Buffer();
	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}

_bool CJSEmerald::Check_Collect(_vec3 vPlayerPos)
{
	_vec3 vMyPos;
	m_pTransformCom->Get_Info(INFO_POS, &vMyPos);

	_vec3 vDiff = vMyPos - vPlayerPos;

	_float fDist = D3DXVec3Length(&vDiff);

	return fDist <= 2.0f;
}

HRESULT CJSEmerald::Add_Component()
{
	CComponent* pComponent = nullptr;

	// Buffer
	pComponent = m_pBufferCom = dynamic_cast<CRcTex*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RcTex"));

	if (!pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_Buffer", pComponent });

	// Transform
	pComponent = m_pTransformCom = dynamic_cast<CTransform*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));

	if (!pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_Transform", pComponent });

	// Texture
	pComponent = m_pTextureCom = dynamic_cast<CTexture*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_EmeraldTexture"));

	if (!pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_Texture", pComponent });

	return S_OK;
}

CJSEmerald* CJSEmerald::Create(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vPos)
{
	CJSEmerald* pEmerald = new CJSEmerald(pGraphicDev);

	if (FAILED(pEmerald->Ready_GameObject(vPos)))
	{
		Safe_Release(pEmerald);
		MSG_BOX("JSEmerald Create Failed");
		return nullptr;
	}

	return pEmerald;
}

void CJSEmerald::Free()
{
	CGameObject::Free();
}