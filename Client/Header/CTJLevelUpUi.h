#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"
#include "CTJPlayer.h"

class CTJLevelUpUI : public CGameObject
{
private:
    explicit CTJLevelUpUI(LPDIRECT3DDEVICE9 pGraphicDev);
    virtual ~CTJLevelUpUI();

public:
    virtual HRESULT Ready_GameObject();
    virtual _int    Update_GameObject(const _float& fTimeDelta);
    virtual void    LateUpdate_GameObject(const _float& fTimeDelta);
    virtual void    Render_GameObject();

public:
    void    Show(CTJPlayer* pPlayer);  // 플레이어 상태 보고 랜덤 카드 선택
    void    Hide();
    bool    Is_Visible() const { return m_bVisible; }
    bool    Is_Selected() const { return m_iSelected != -1; }
    ETJAbility Get_Selected() const { return m_eSelectedAbility; }
    void    Reset_Selected() { m_iSelected = -1; }

private:
    void    Pick_RandomAbilities(CTJPlayer* pPlayer);
    void    Render_BeginUI();
    void    Render_EndUI();
    void    Render_Card(int iIndex, float fX, float fY, float fW, float fH);
    bool    IsMouseInCard(float fX, float fY, float fW, float fH);

public:
    static CTJLevelUpUI* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
    virtual void Free();

private:
    Engine::CRcTex* m_pBufferCom = nullptr;
    Engine::CTexture* m_pCardTexture[(int)ETJAbility::ABILITY_END][3] = {};

    _matrix m_matOriginView;
    _matrix m_matOriginProj;

    bool        m_bVisible = false;
    int         m_iSelected = -1;
    ETJAbility  m_eSelectedAbility = ETJAbility::ABILITY_END;
    bool        m_bPrevLBtn = false;

    // 레벨업에 표시할 카드 3장
    ETJAbility  m_eCardAbility[3] = {};

    float   m_fCardW = 200.f;
    float   m_fCardH = 300.f;
    float   m_fCardY = 210.f;

    CTJPlayer* m_pCurPlayer = nullptr;
};