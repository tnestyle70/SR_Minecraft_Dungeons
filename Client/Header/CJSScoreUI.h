#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"

class CJSScoreUI : public CGameObject
{
private:
	explicit CJSScoreUI(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual ~CJSScoreUI();

public:
	virtual HRESULT Ready_GameObject();
	virtual _int Update_GameObject(const _float& fTimeDelta);
	virtual void LateUpdate_GameObject(const _float& fTimeDelta);
	virtual void Render_GameObject();

public:
	void Add_Score(_int iScore) { m_iScore += iScore; }
	void Set_Distance(_float fDistance) { m_fDistance = fDistance; }

private:
	HRESULT Add_Component();
	//void Render_Overlay();
	void Render_Score();
	void Render_GameOver();

public:
	static CJSScoreUI* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
	CRcTex* m_pEmeraldBuf = nullptr;
	CTexture* m_pEmeraldTex = nullptr;
	
	CRcTex* m_pGameOverBuf = nullptr;
	CTexture* m_pGameOverTex = nullptr;

	_int m_iScore = 0;
	_float m_fDistance = 0.f;
	_float m_fSpeed = 20.f;

private:
	virtual void Free();
};