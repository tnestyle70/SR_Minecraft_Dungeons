#pragma once
#include "CUIInterface.h"

enum class eCurrencyType
{
    EMERALD,
    ARTIFACT,
    CURRENCY_END
};

class CCurrencyHUD : public CGameObject
{
private:
    explicit CCurrencyHUD(LPDIRECT3DDEVICE9 pGraphicDev);
    virtual ~CCurrencyHUD();

public:
    virtual HRESULT Ready_GameObject();
    virtual _int    Update_GameObject(const _float& fTimeDelta);
    virtual void    LateUpdate_GameObject(const _float& fTimeDelta);
    virtual void    Render_GameObject();
    
public:
    void Set_Info(eCurrencyType eType, float fX, float fY, float fW, float fH)
    {
        m_eType = eType;
        m_fX = fX; m_fY = fY; m_fW = fW; m_fH = fH;
    }

private:
    HRESULT Add_Component();

    void Begin_UIRender(); 
    void End_UIRender();


public:
    static CCurrencyHUD* Create(LPDIRECT3DDEVICE9 pGraphicDev,
        eCurrencyType eType,
        float fX, float fY, float fW, float fH);
private:
    CRcTex* m_pBufferCom = nullptr;
    CTexture* m_pCurrencyTexture = nullptr;

    //스크린 좌표, 크기
    float m_fX = 0.f, m_fY = 0.f;
    float m_fW = 0.f, m_fH = 0.f;

    //UI 복원용 matrix
    _matrix m_matOriginView, m_matOriginProj;

    eCurrencyType m_eType = eCurrencyType::CURRENCY_END;

private:
    virtual void Free();
};