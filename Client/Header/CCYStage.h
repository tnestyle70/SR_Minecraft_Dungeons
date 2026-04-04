#pragma once
#include "CScene.h"
#include "CCYCamera.h"
#include "CCYPlayer.h"
#include "CTorch.h"
#include "CEventBus.h"
#include <vector>
#include <algorithm>
#include <set>

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

    void Add_Time(float fTime)
    {
        m_fTimer += fTime;
        m_bShowAddTime = true;
        m_fAddTimeShow = 1.5f;
    }

private:
    HRESULT Ready_Environment_Layer(const _tchar* pLayerTag);
    HRESULT Ready_GameLogic_Layer(const _tchar* pLayerTag);
    HRESULT Ready_UI_Layer(const _tchar* pLayerTag);
    HRESULT Ready_Light();
    HRESULT Ready_StageData(const _tchar* szPath);
    void    Update_TorchLights();
    _vec3   Get_PlayerPos(); 
    set<int> m_setIronBarTriggered;

private:
    CCYCamera* m_pCYCamera = nullptr;
    CCYPlayer* m_pCYPlayer = nullptr;
    vector<CTorch*> m_vecTorches;

    // 타이머
    float      m_fTimer = 10.f;
    bool       m_bGameOver = false;
    LPD3DXFONT m_pFont = nullptr;

    // +5초 표시
    float m_fAddTimeShow = 0.f;
    bool  m_bShowAddTime = false;

    // 낙하 트리거
    bool m_bFallTriggered = false;

    // 아이언바 오픈 추적
    set<int> m_setOpenedGroups; 

    bool m_bPrevAtkColliding = false; 
    bool m_bPrevPlayerHit = false;

public:
    static CCYStage* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
    virtual void Free();
};