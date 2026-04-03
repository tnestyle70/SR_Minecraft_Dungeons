#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"
#include "CPlayer.h"

class CEnderEye : public CGameObject
{
private:
	explicit CEnderEye(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual ~CEnderEye();

public:
	virtual HRESULT Ready_GameObject();
	virtual _int Update_GameObject(const _float& fTimeDelta);
	virtual void LateUpdate_GameObject(const _float& fTimeDelta);
	virtual void Render_GameObject();

private:
	HRESULT Add_Component();
	void    Check_ArrowCollision();

public:
	void Set_Player(CPlayer* pPlayer) { m_pPlayer = pPlayer; }
	void Start_Flicker() { m_bFlickering = true; }
	bool Is_Flickering() const { return m_bFlickering; }

public:
	static CEnderEye* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
	CRcTex* m_pBufferCom = nullptr;
	CTransform* m_pTransformCom = nullptr;
	CTexture* m_pTextureCom = nullptr;
	CCollider* m_pColliderCom = nullptr;

	CPlayer* m_pPlayer = nullptr;
	bool        m_bFlickering = false;
	float       m_fFlickerTime = 0.f;
	float       m_fAlpha = 1.f;

private:
	virtual void Free();
};

