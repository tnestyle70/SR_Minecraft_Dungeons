#include "pch.h"
#include "CLoadingTexture.h"
#include "CRenderer.h"

CLoadingTexture::CLoadingTexture(LPDIRECT3DDEVICE9 pGraphicDev)
	: CGameObject(pGraphicDev)
{}

CLoadingTexture::CLoadingTexture(const CGameObject& rhs)
	: CGameObject(rhs)
{}

CLoadingTexture::~CLoadingTexture()
{}

HRESULT CLoadingTexture::Ready_GameObject(const _tchar* pProtoPath)
{
	if (FAILED(Add_Component(pProtoPath)))
		return E_FAIL;

	m_fX = 100.f;
	m_fY = 500.f;
	m_fW = 400.f;
	m_fH = 150.f;
	
	return S_OK;
}

_int CLoadingTexture::Update_GameObject(const _float& fTimeDelta)
{
	_int iExit = CGameObject::Update_GameObject(fTimeDelta);

	CRenderer::GetInstance()->Add_RenderGroup(RENDER_PRIORITY, this);

	return iExit;
}

void CLoadingTexture::LateUpdate_GameObject(const _float& fTimeDelta)
{
	CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CLoadingTexture::Render_GameObject()
{
	BeginUIRender();

	_matrix matWorld;
	float fNDCX = (m_fX + m_fW * 0.5f) / (WINCX * 0.5f) - 1.f;
	float fNDCY = 1.f - (m_fY + m_fH * 0.5f) / (WINCY * 0.5f);
	float fScaleX = m_fW / WINCX;
	float fScaleY = m_fH / WINCY;

	D3DXMatrixTransformation2D(&matWorld,
		nullptr, 0.f,
		&_vec2(fScaleX, fScaleY),
		nullptr, 0.f,
		&_vec2(fNDCX, fNDCY));

	m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);

	//Set_Texture
	//GPU의 텍스쳐 스테이지에 해당 이미지를 올려둠, 모든 도형에는 해당 텍스쳐로 그려짐
	//Vertex Texture에서 설정한 texture UV 값을 통해서 
	//GPU는 설정된 텍스쳐의 어디서부터 어디까지를 그릴지를 결정해서 렌더링한다

	m_pTextTexture->Set_Texture(0);

	m_pBufferCom->Render_Buffer();

	EndUIRender();
}

void CLoadingTexture::BeginUIRender()
{
	//월드 행렬 항등 행렬로 설정
	//_matrix matWorld;
	//D3DXMatrixIdentity(&matWorld);
	//m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);
	////원본 뷰, 투영 저장
	//m_pGraphicDev->GetTransform(D3DTS_VIEW, &m_matOriginView);
	//m_pGraphicDev->GetTransform(D3DTS_PROJECTION, &m_matOriginProj);
	////뷰, 투영 풀어주기
	//_matrix matView, matProj;
	//D3DXMatrixIdentity(&matView);
	//D3DXMatrixIdentity(&matProj);
	//m_pGraphicDev->SetTransform(D3DTS_VIEW, &matView);
	//m_pGraphicDev->SetTransform(D3DTS_PROJECTION, &matProj);
	//CullMode 설정
	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	//알파 블렌딩 활성화
	m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

	m_pGraphicDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_pGraphicDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	m_pGraphicDev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	m_pGraphicDev->SetRenderState(D3DRS_ALPHAREF, 0xc0);
}

void CLoadingTexture::EndUIRender()
{
	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	//투영 다시 적용시켜주기
	//m_pGraphicDev->SetTransform(D3DTS_VIEW, &m_matOriginView);
	//m_pGraphicDev->SetTransform(D3DTS_PROJECTION, &m_matOriginProj);

	//알파블렌딩 - 옵션 다시 꺼주기!!
	m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
}

HRESULT CLoadingTexture::Add_Component(const _tchar* pProtoPath)
{
	CComponent* pComponent = nullptr;

	pComponent = m_pBufferCom = dynamic_cast<CRcTex*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RcTex"));

	if (!pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_Buffer", pComponent });

	// Texture
	pComponent = m_pTextTexture = dynamic_cast<CTexture*>
		(CProtoMgr::GetInstance()->Clone_Prototype(pProtoPath));

	if (nullptr == pComponent)
		return E_FAIL;
	
	m_mapComponent[ID_STATIC].insert({ L"Com_Texture", pComponent });

	return S_OK;
}

CLoadingTexture* CLoadingTexture::Create(LPDIRECT3DDEVICE9 pGraphicDev, 
	const _tchar* pProtoPath)
{
	CLoadingTexture* pBlock = new CLoadingTexture(pGraphicDev);

	if (FAILED(pBlock->Ready_GameObject(pProtoPath)))
	{
		Safe_Release(pBlock);
		MSG_BOX("Block Create Failed");
		return nullptr;
	}

	return pBlock;
}

void CLoadingTexture::Free()
{
	CGameObject::Free();
}
