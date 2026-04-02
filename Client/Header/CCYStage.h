#pragma once
#include "CScene.h"
#include "CCYCamera.h"
#include "CCYPlayer.h"
#include "CTorch.h"
#include <vector>
#include <algorithm>

class CCYStage : public CScene
{
protected:
    explicit CCYStage(LPDIRECT3DDEVICE9 pGraphicDev);
    virtual ~CCYStage();

public:
    virtual HRESULT Ready_Scene();
    virtual _int    Update_Scene(const _float& fTimeDelta);
    virtual void    LateUpdate_Scene(const _float& fTimeDelta);
    virtual void    Render_Scene();
    virtual void    Render_UI() override;

private:
    HRESULT Ready_Environment_Layer(const _tchar* pLayerTag);
    HRESULT Ready_GameLogic_Layer(const _tchar* pLayerTag);
    HRESULT Ready_UI_Layer(const _tchar* pLayerTag);
    HRESULT Ready_Light();
    HRESULT Ready_StageData(const _tchar* szPath);
    void    Update_TorchLights();
    _vec3   Get_PlayerPos();

private:
    CCYCamera* m_pCYCamera = nullptr;
    CCYPlayer* m_pCYPlayer = nullptr;
    vector<CTorch*> m_vecTorches;

public:
    static CCYStage* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
    virtual void Free();
};