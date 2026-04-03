#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"
#include "CBlockMgr.h"
#include "CPlayerBody.h"
#include "CPlayerArrow.h"

class CCYCamera;

class CCYPlayer : public Engine::CGameObject
{
public:
    explicit CCYPlayer(LPDIRECT3DDEVICE9 pGraphicDev);
    virtual ~CCYPlayer();

public:
    virtual HRESULT Ready_GameObject() override;
    virtual _int    Update_GameObject(const _float& fTimeDelta) override;
    virtual void    LateUpdate_GameObject(const _float& fTimeDelta) override;
    virtual void    Render_GameObject() override;

    void Set_Camera(CCYCamera* pCamera) { m_pCamera = pCamera; }

    Engine::CTransform* Get_Transform() { return m_pTransformCom; }
    Engine::CCollider* Get_Collider() { return m_pColliderCom; }

    _float Get_Hp()    const { return m_fHp; }
    _float Get_MaxHp() const { return m_fMaxHp; }
    void   Hit(float fDmg) { m_fHp -= fDmg; if (m_fHp < 0.f) m_fHp = 0.f; }

    const vector<CPlayerArrow*>& Get_Arrows() const { return m_vecArrows; }

private:
    HRESULT Add_Component();
    void    Sync_WithCamera();
    void    FPS_Gravity(const _float& fTimeDelta);
    void    FPS_BlockCollision();

private:
    Engine::CTransform* m_pTransformCom = nullptr;
    Engine::CCollider* m_pColliderCom = nullptr;
    CCYCamera* m_pCamera = nullptr;

    // ¿À¸¥ÆÈ
    CPlayerBody* m_pHandBufferCom = nullptr;
    Engine::CTexture* m_pHandTextureCom = nullptr;

    // ¿ÞÆÈ
    CPlayerBody* m_pLeftHandBufferCom = nullptr;

    // Ä®
    Engine::CRcTex* m_pSwordBufferCom = nullptr;
    Engine::CTexture* m_pSwordTextureCom = nullptr;

    // È°
    Engine::CRcTex* m_pBowBufferCom = nullptr;
    Engine::CTexture* m_pBowTextureCom = nullptr;

    float m_fVelocityY = 0.f;
    float m_fGravity = -35.f;
    float m_fMaxFall = -20.f;
    bool  m_bOnGround = false;

    float m_fHp = 100.f;
    float m_fMaxHp = 100.f;

    // °ø°Ý
    int   m_iComboStep = 0;
    float m_fAtkTime = 0.f;
    float m_fAtkDuration = 0.4f;
    float m_fComboTimer = 0.f;
    float m_fComboWindow = 0.3f;
    bool  m_bAtkInput = false;

    // È°
    bool  m_bCharging = false;
    float m_fCharge = 0.f;
    float m_fMaxCharge = 2.f;
    vector<CPlayerArrow*> m_vecArrows;

public:
    static CCYPlayer* Create(LPDIRECT3DDEVICE9 pGraphicDev);

protected:
    virtual void Free();
};