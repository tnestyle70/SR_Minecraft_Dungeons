#include "pch.h"
// #include "CUIObject.h"
// #include "CUITexture.h"
// #include "CRenderer.h"
// #include "CProtoMgr.h"
// 
// CUIObject::CUIObject(LPDIRECT3DDEVICE9 pGraphicDev)
// 	: Engine::CGameObject(pGraphicDev)
// 	, m_fX(0.f), m_fY(0.f), m_fWidth(100.f), m_fHeight(100.f)
// {
// }
// 
// CUIObject::~CUIObject()
// {
// 	Free();
// }
// 
// HRESULT CUIObject::Ready_GameObject()
// {
// 	return S_OK;
// }
// 
// _int CUIObject::Update_GameObject(const _float& fTimeDelta)
// {
// 	return Engine::CGameObject::Update_GameObject(fTimeDelta);
// }
// 
// void CUIObject::LateUpdate_GameObject(const _float& fTimeDelta)
// {
// 	Engine::CGameObject::LateUpdate_GameObject(fTimeDelta);
// 
// 	if (nullptr != m_pRendererCom)
// 		m_pRendererCom->Add_RenderGroup(Engine::RENDER_UI, this);
// }
// 
// void CUIObject::Render_GameObject()
// {
// 	if (!m_pTextureCom)
// 		return;
// 
// 	float maxU = 1.0f, maxV = 1.0f;
// 	m_pTextureCom->GetUVCoordinates(maxU, maxV);
// 
// 	struct UI_Vertex {
// 		float x, y, z, rhw;
// 		float u, v;
// 	};
// 
// 	float x = m_fX, y = m_fY, w = m_fWidth, h = m_fHeight;
// 
// 	UI_Vertex vertices[4] = {
// 		{ x,     y,     0.0f, 1.0f, 0.0f, 0.0f }, // Top-Left
// 		{ x + w, y,     0.0f, 1.0f, maxU, 0.0f }, // Top-Right
// 		{ x,     y + h, 0.0f, 1.0f, 0.0f, maxV }, // Bottom-Left
// 		{ x + w, y + h, 0.0f, 1.0f, maxU, maxV }  // Bottom-Right
// 	};
// 
// 	m_pTextureCom->Render_Texture();
// 	m_pGraphicDev->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
// 	m_pGraphicDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(UI_Vertex));
// }
// 
// CUIObject* CUIObject::Create(LPDIRECT3DDEVICE9 pGraphicDev, float fX, float fY, float fWidth, float fHeight, const std::wstring& strProtoTag)
// {
// 	CUIObject* pInstance = new CUIObject(pGraphicDev);
// 
// 	pInstance->m_fX = fX;
// 	pInstance->m_fY = fY;
// 	pInstance->m_fWidth = fWidth;
// 	pInstance->m_fHeight = fHeight;
// 
// 	if (FAILED(pInstance->Add_Component(strProtoTag)))
// 	{
// 		Engine::Safe_Release(pInstance);
// 		return nullptr;
// 	}
// 
// 	if (FAILED(pInstance->Ready_GameObject()))
// 	{
// 		Engine::Safe_Release(pInstance);
// 		return nullptr;
// 	}
// 
// 	return pInstance;
// }
// 
// HRESULT CUIObject::Add_Component(const std::wstring& strProtoTag)
// {
// 
// 	// Engine::CComponent* pComponent = nullptr;
// 	// 
// 	// // Texture
// 	// pComponent = m_pTextureCom = dynamic_cast<Engine::CUITexture*>(Engine::CProtoMgr::GetInstance()->Clone_Prototype(strProtoTag.c_str()));
// 	// if (nullptr == pComponent)
// 	// 	return E_FAIL;
// 	// m_mapComponent[Engine::ID_STATIC].insert({ L"Com_Texture", pComponent });
// 
// 	return S_OK;
// }
// 
// void CUIObject::Free()
// {
// 	Engine::CGameObject::Free();
// }
// 