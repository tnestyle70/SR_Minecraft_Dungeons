// #pragma once
// #include "CGameObject.h"
// 
// namespace Engine {
// 	class CUITexture;
// 	class CRenderer;
// }
// 
// class CUIObject : public Engine::CGameObject
// {
// protected:
// 	explicit CUIObject(LPDIRECT3DDEVICE9 pGraphicDev);
// 	virtual ~CUIObject();
// 
// public:
// 	virtual HRESULT Ready_GameObject() override;
// 	virtual _int Update_GameObject(const _float& fTimeDelta) override;
// 	virtual void LateUpdate_GameObject(const _float& fTimeDelta) override;
// 	virtual void Render_GameObject() override;
// 
// protected:
// 	Engine::CUITexture* m_pTextureCom = nullptr;
// 	Engine::CRenderer* m_pRendererCom = nullptr;
// 	float m_fX, m_fY, m_fWidth, m_fHeight;
// 
// public:
// 	static CUIObject* Create(LPDIRECT3DDEVICE9 pGraphicDev, float fX, float fY, float fWidth, float fHeight, const std::wstring& strProtoTag);
// 
// private:
// 	HRESULT Add_Component(const std::wstring& strProtoTag);
// 	virtual void Free() override;
// };
