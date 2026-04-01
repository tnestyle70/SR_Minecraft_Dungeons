#pragma once
#include "CScene.h"

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

private:
	void Clear_DeadObject(const _tchar* pLayerTag, const _float& fTimeDelta);

public:
	static CJSStage* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
	virtual void Free();
};
