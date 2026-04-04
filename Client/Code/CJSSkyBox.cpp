#include "pch.h"
#include "CJSSkyBox.h"
#include "CRenderer.h"
#include "CManagement.h"

CJSSkyBox::CJSSkyBox(LPDIRECT3DDEVICE9 pGraphicDev)
	: CGameObject(pGraphicDev)
{
}

CJSSkyBox::CJSSkyBox(const CGameObject& rhs)
	: CGameObject(rhs)
{
}

CJSSkyBox::~CJSSkyBox()
{
}

HRESULT CJSSkyBox::Ready_GameObject()
{
	if (FAILED(Add_Component()))
		return E_FAIL;

	m_pTransformCom->m_vScale = { 200.f, 200.f, 200.f };

	return S_OK;
}

_int CJSSkyBox::Update_GameObject(const _float& fTimeDelta)
{
	_int iExit = CGameObject::Update_GameObject(fTimeDelta);

	CRenderer::GetInstance()->Add_RenderGroup(RENDER_SKYBOX, this);

	return iExit;
}

void CJSSkyBox::LateUpdate_GameObject(const _float& fTimeDelta)
{
	CGameObject::LateUpdate_GameObject(fTimeDelta);

	_matrix	matCamWorld;
	m_pGraphicDev->GetTransform(D3DTS_VIEW, &matCamWorld);
	D3DXMatrixInverse(&matCamWorld, 0, &matCamWorld);

	m_pTransformCom->Set_Pos(matCamWorld._41, matCamWorld._42 + 3.f, matCamWorld._43);
}

void CJSSkyBox::Render_GameObject()
{
	m_pGraphicDev->SetTransform(D3DTS_WORLD, m_pTransformCom->Get_World());

	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	m_pTextureCom->Set_Texture(0);

	m_pBufferCom->Render_Buffer();

	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
}

HRESULT CJSSkyBox::Add_Component()
{
	Engine::CComponent* pComponent = nullptr;

	// CubeTex
	pComponent = m_pBufferCom = dynamic_cast<CCubeTex*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_CubeTex"));

	if (nullptr == pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_Buffer", pComponent });

	// Transform
	pComponent = m_pTransformCom = dynamic_cast<CTransform*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));

	if (nullptr == pComponent)
		return E_FAIL;

	m_mapComponent[ID_DYNAMIC].insert({ L"Com_Transform", pComponent });


	// Texture
	pComponent = m_pTextureCom = dynamic_cast<CTexture*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_JungleSkyBoxTexture"));

	if (nullptr == pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_Texture", pComponent });

	return S_OK;
}

CJSSkyBox* CJSSkyBox::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
	CJSSkyBox* pSkyBox = new CJSSkyBox(pGraphicDev);

	if (FAILED(pSkyBox->Ready_GameObject()))
	{
		Safe_Release(pSkyBox);
		MSG_BOX("JSSkyBox Create Failed");
		return nullptr;
	}

	return pSkyBox;
}

void CJSSkyBox::Free()
{
	CGameObject::Free();
}
