#include "pch.h"
#include "CJSTile.h"
#include "CRenderer.h"

CJSTile::CJSTile(LPDIRECT3DDEVICE9 pGraphicDev)
	: CGameObject(pGraphicDev)
	, m_eTileID(TILE_NORMAL)
	, m_pBufferCom(nullptr)
	, m_pTransformCom(nullptr)
	, m_pTextureCom(nullptr)
{
}

CJSTile::~CJSTile()
{
}

HRESULT CJSTile::Ready_GameObject(_vec3 vPos, TILEID eTileID)
{
	m_eTileID = eTileID;

	if (FAILED(Add_Component()))
		return E_FAIL;

	m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
	m_pTransformCom->Set_Scale(2.f);
	m_pTransformCom->Update_Component(0.f);

	return S_OK;
}

_int CJSTile::Update_GameObject(const _float& fTimeDelta)
{
	_int iExit = CGameObject::Update_GameObject(fTimeDelta);

	return iExit;
}

void CJSTile::LateUpdate_GameObject(const _float& fTimeDelta)
{
	CGameObject::LateUpdate_GameObject(fTimeDelta);

	if (m_eTileID == TILE_EMPTY)
		return;

	CRenderer::GetInstance()->Add_RenderGroup(RENDER_NONALPHA, this);
}

void CJSTile::Render_GameObject()
{
	m_pGraphicDev->SetRenderState(D3DRS_LIGHTING, TRUE);

	m_pGraphicDev->SetTransform(D3DTS_WORLD, m_pTransformCom->Get_World());
	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	m_pTextureCom->Set_Texture(0);

	if (FAILED(Set_Material()))
		return;

	m_pBufferCom->Render_Buffer();

	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	m_pGraphicDev->SetRenderState(D3DRS_LIGHTING, FALSE);
}

HRESULT CJSTile::Add_Component()
{
	CComponent* pComponent = nullptr;

	// Buffer
	pComponent = m_pBufferCom = dynamic_cast<CJSCubeTex*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_JSCubeTex"));
	if (pComponent == nullptr)
		return E_FAIL;
	m_mapComponent[ID_STATIC].insert({ L"Com_Buffer", pComponent });

	// Transform
	pComponent = m_pTransformCom = dynamic_cast<CTransform*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));
	if (pComponent == nullptr)
		return E_FAIL;
	m_mapComponent[ID_STATIC].insert({ L"Com_Transform", pComponent });

	// Texture
	pComponent = m_pTextureCom = dynamic_cast<CTexture*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_JSBlockTexture"));
	if (pComponent == nullptr)
		return E_FAIL;
	m_mapComponent[ID_STATIC].insert({ L"Com_Texture", pComponent });

	return S_OK;
}

HRESULT CJSTile::Set_Material()
{
	D3DMATERIAL9			tMtrl;
	ZeroMemory(&tMtrl, sizeof(D3DMATERIAL9));

	tMtrl.Diffuse = D3DXCOLOR(1.f, 1.f, 1.f, 1.f);
	tMtrl.Specular = D3DXCOLOR(1.f, 1.f, 1.f, 1.f);
	tMtrl.Ambient = D3DXCOLOR(0.2f, 0.2f, 0.2f, 1.f);

	tMtrl.Emissive = D3DXCOLOR(0.f, 0.f, 0.f, 0.f);
	tMtrl.Power = 0.f;

	m_pGraphicDev->SetMaterial(&tMtrl);

	return S_OK;
}

CJSTile* CJSTile::Create(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vPos, TILEID eTileID)
{
	CJSTile* pTile = new CJSTile(pGraphicDev);
	if (FAILED(pTile->Ready_GameObject(vPos, eTileID)))
	{
		Safe_Release(pTile);
		MSG_BOX("JSTile Create Failed");
		return nullptr;
	}
	return pTile;
}

void CJSTile::Free()
{
	CGameObject::Free();
}