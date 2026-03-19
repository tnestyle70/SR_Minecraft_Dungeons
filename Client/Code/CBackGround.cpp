#include "pch.h"
#include "CBackGround.h"
#include "CRenderer.h"

CBackGround::CBackGround(LPDIRECT3DDEVICE9 pGraphicDev)
	: CGameObject(pGraphicDev)
{
}

CBackGround::CBackGround(const CGameObject& rhs)
	: CGameObject(rhs)
{
}

CBackGround::~CBackGround()
{
}

HRESULT CBackGround::Ready_GameObject(const _tchar* pProtoPath)
{
	if (FAILED(Add_Component(pProtoPath)))
		return E_FAIL;

	//m_pTransformCom->m_vScale = { 2.f, 1.f, 1.f };

	return S_OK;
}

_int CBackGround::Update_GameObject(const _float& fTimeDelta)
{
	_int iExit = CGameObject::Update_GameObject(fTimeDelta);

	CRenderer::GetInstance()->Add_RenderGroup(RENDER_UI, this);

	return iExit;
}

void CBackGround::LateUpdate_GameObject(const _float& fTimeDelta)
{
	CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CBackGround::Render_GameObject()
{
	//RcTex가 -1 ~ 1로 로컬 좌표가 설정되어있기 때문에 
	//World Identity -> View Identity -> Projection Identity -> NDC(-1 ~ 1) -> 픽셀
	//로컬 좌표가 그대로 NDC로 적용되어있기 때문에 화면을 꽉 채우는 화면이 그려지게 되는 것이다.

	//월드 행렬 Identity로 설정해주기!!
	_matrix matWorld;
	D3DXMatrixIdentity(&matWorld);
	m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);

	//뷰, 투영 풀어주기
	_matrix matOriginView, matOriginProj;
	m_pGraphicDev->GetTransform(D3DTS_VIEW, &matOriginView);
	m_pGraphicDev->GetTransform(D3DTS_PROJECTION, &matOriginProj);

	//뷰 풀어주기
	_matrix matView;
	D3DXMatrixIdentity(&matView);
	m_pGraphicDev->SetTransform(D3DTS_VIEW, &matView);

	//투영 풀어주기 설정
	_matrix matProj;
	D3DXMatrixIdentity(&matProj);
	m_pGraphicDev->SetTransform(D3DTS_PROJECTION, &matProj);


	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	//알파블렌딩

	m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

	m_pGraphicDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_pGraphicDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	m_pGraphicDev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	m_pGraphicDev->SetRenderState(D3DRS_ALPHAREF, 0xc0);



	m_pTextureCom->Set_Texture(0);

	m_pBufferCom->Render_Buffer();

	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	//투영 다시 적용시켜주기
	m_pGraphicDev->SetTransform(D3DTS_VIEW, &matOriginView);
	m_pGraphicDev->SetTransform(D3DTS_PROJECTION, &matOriginProj);

	//알파블렌딩 - 옵션 다시 꺼주기!!
	m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

	m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
}

HRESULT CBackGround::Add_Component(const _tchar* pPath)
{
	Engine::CComponent* pComponent = nullptr;

	// RcTex
	pComponent = m_pBufferCom = dynamic_cast<CRcTex*>(
		CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RcTex"));

	if (nullptr == pComponent)
		return E_FAIL;
	
	m_mapComponent[ID_STATIC].insert({ L"Com_Buffer", pComponent });

	// Texture
	pComponent = m_pTextureCom = dynamic_cast<CTexture*>
		(CProtoMgr::GetInstance()->Clone_Prototype(pPath));

	if (nullptr == pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_Texture", pComponent });

	return S_OK;
}

CBackGround* CBackGround::Create(LPDIRECT3DDEVICE9 pGraphicDev,
	const _tchar* pProtoPath)
{
	CBackGround* pBackGround = new CBackGround(pGraphicDev);

	if (FAILED(pBackGround->Ready_GameObject(pProtoPath)))
	{
		Safe_Release(pBackGround);
		MSG_BOX("pBackGround Create Failed");
		return nullptr;
	}

	return pBackGround;
}

void CBackGround::Free()
{
	CGameObject::Free();
}
