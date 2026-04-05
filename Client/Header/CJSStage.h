#pragma once
#include "CScene.h"
#include "CJSTile.h"
#include "CJSMonster.h"

class CJSWaterPlane;

class CJSStage : public CScene
{
protected:
	explicit CJSStage(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual ~CJSStage();

public:
	virtual HRESULT Ready_Scene();
	virtual _int Update_Scene(const _float& fTimeDelta);
	virtual void LateUpdate_Scene(const _float& fTimeDelta);
	virtual void Render_Scene();
	virtual void Render_UI() override {};

private:
	HRESULT Ready_Environment_Layer(const _tchar* pLayerTag);
	HRESULT Ready_GameLogic_Layer(const _tchar* pLayerTag);
	HRESULT Ready_UI_Layer(const _tchar* pLayerTag);
	HRESULT Ready_Light();
	HRESULT Ready_Fog();
	HRESULT Ready_IntroCave(CLayer* pLayer);

private:
	void Clear_DeadObject(const _tchar* pLayerTag, const _float& fTimeDelta);
	void Remove_IntroCave();
	void Remove_Monsters();

public:
	static CJSStage* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
	vector<CJSTile*>    m_vecCaveTiles;
	vector<CJSMonster*> m_vecMonsters;
	_bool               m_bCaveRemoved = false;
	_bool				m_bMonsterRemoved = false;

	CJSWaterPlane* m_pWaterPlane = nullptr;
	_float m_fGameOverTimer = 0.f;
	_float m_fGameOverDelay = 2.f;  // 2초 후 씬 전환

	_float  m_fIntroTimer = 0.f;
	_float  m_fIntroDuration = 3.f;   // 인트로 지속 시간
	_float  m_fCountdownTimer = 0.f;
	_int    m_iCountdown = 3;     // 3, 2, 1

private:
	virtual void Free();
};
