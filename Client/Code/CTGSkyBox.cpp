#include "pch.h"
#include "CTGSkyBox.h"
#include "CRenderer.h"
#include "CManagement.h"

CTGSkyBox::CTGSkyBox(LPDIRECT3DDEVICE9 pGraphicDev)
	: CGameObject(pGraphicDev)
{
}

CTGSkyBox::CTGSkyBox(const CGameObject& rhs)
	: CGameObject(rhs)
{
}

CTGSkyBox::~CTGSkyBox()
{
}

HRESULT CTGSkyBox::Ready_GameObject()
{
	if (FAILED(Add_Component()))
		return E_FAIL;

	m_pTransformCom->m_vScale = { 300.f, 300.f, 300.f };

	return S_OK;
}

_int CTGSkyBox::Update_GameObject(const _float& fTimeDelta)
{
	_int iExit = CGameObject::Update_GameObject(fTimeDelta);

	CRenderer::GetInstance()->Add_RenderGroup(RENDER_SKYBOX, this);

	return iExit;
}

void CTGSkyBox::LateUpdate_GameObject(const _float& fTimeDelta)
{
	CGameObject::LateUpdate_GameObject(fTimeDelta);

	_matrix	matCamWorld;
	m_pGraphicDev->GetTransform(D3DTS_VIEW, &matCamWorld);
	D3DXMatrixInverse(&matCamWorld, 0, &matCamWorld);

	m_pTransformCom->Set_Pos(matCamWorld._41, matCamWorld._42 + 3.f, matCamWorld._43);
}

void CTGSkyBox::Render_GameObject()
{
	m_pGraphicDev->SetTransform(D3DTS_WORLD, m_pTransformCom->Get_World());

	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	m_pTextureCom->Set_Texture(0);

	m_pBufferCom->Render_Buffer();

	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
}

HRESULT CTGSkyBox::Add_Component()
{
	Engine::CComponent* pComponent = nullptr;

	// RcTex
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
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_ColosseumSkyBoxTexture"));

	if (nullptr == pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_Texture", pComponent });

	return S_OK;
}

CTGSkyBox* CTGSkyBox::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
	CTGSkyBox* pSkyBox = new CTGSkyBox(pGraphicDev);

	if (FAILED(pSkyBox->Ready_GameObject()))
	{
		Safe_Release(pSkyBox);
		MSG_BOX("pSkyBox Create Failed");
		return nullptr;
	}

	return pSkyBox;
}

void CTGSkyBox::Free()
{
	CGameObject::Free();
}
