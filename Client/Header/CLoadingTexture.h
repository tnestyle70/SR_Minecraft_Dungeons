#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"

class CLoadingTexture : public CGameObject
{
private:
	explicit CLoadingTexture(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CLoadingTexture(const CGameObject& rhs);
	virtual ~CLoadingTexture();

public:
	virtual			HRESULT		Ready_GameObject(const _tchar* pProtoPath);
	virtual			_int		Update_GameObject(const _float& fTimeDelta);
	virtual			void		LateUpdate_GameObject(const _float& fTimeDelta);
	virtual			void		Render_GameObject();
	
private:
	HRESULT			Add_Component(const _tchar* pProtoPath);
	
	void BeginUIRender();
	void EndUIRender();

private:
	CRcTex* m_pBufferCom = nullptr;
	CTexture* m_pTextTexture = nullptr;

	float m_fX, m_fY, m_fW, m_fH = 0.f;

public:
	static CLoadingTexture* Create(LPDIRECT3DDEVICE9 pGraphicDev,
		const _tchar* pProtoPath);

private:
	virtual void Free();
};
