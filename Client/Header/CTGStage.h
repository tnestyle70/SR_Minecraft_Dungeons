#pragma once
#include "CScene.h"
#include "CDynamicCamera.h"
#include "CTJLevelUpUI.h"
#include "CTJPlayer.h"
#include "CTJBoss.h"

class CTGStage : public CScene
{
protected:
	explicit CTGStage(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual ~CTGStage();
public:
	virtual HRESULT Ready_Scene();
	virtual _int Update_Scene(const _float& fTimeDelta);
	virtual void LateUpdate_Scene(const _float& fTimeDelta);
	virtual void Render_Scene();
	virtual void Render_UI() override;
private:
	HRESULT Ready_Environment_Layer(const _tchar* pLayerTag);
	HRESULT Ready_GameLogic_Layer(const _tchar* pLayerTag);
	HRESULT Ready_UI_Layer(const _tchar* pLayerTag);
	HRESULT Ready_Light();
	HRESULT Ready_StageData(const _tchar* szPath);
public:
	static CTGStage* Create(LPDIRECT3DDEVICE9 pGraphicDev);
private:
	virtual void Free();

	CDynamicCamera* m_pDynamicCamera = nullptr;
	CTJLevelUpUI* m_pLevelUpUI = nullptr;
	CTJPlayer* m_pTJPlayer = nullptr;
	CTJBoss* m_pTJBoss = nullptr;
	bool m_bBossSpawned = false;
	ID3DXLine* m_pLine = nullptr;
	ID3DXFont* m_pFont = nullptr;

	//문
	Engine::CRcTex* m_pDoorBufferCom = nullptr;
	Engine::CTexture* m_pDoorTextureCom = nullptr;
	bool m_bDoorSpawned = false;
	_vec3 m_vDoorPos = {};
	_float m_fDoorTimer = 0.f;
};
