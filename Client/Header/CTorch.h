#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"

class CTorch : public CGameObject
{
private:
    explicit CTorch(LPDIRECT3DDEVICE9 pGraphicDev);
    virtual ~CTorch();

public:
    virtual HRESULT Ready_GameObject();
    virtual _int    Update_GameObject(const _float& fTimeDelta);
    virtual void    LateUpdate_GameObject(const _float& fTimeDelta);
    virtual void    Render_GameObject();

private:
    HRESULT Add_Component();

private:
    CTexture* m_pTextureCom = nullptr;
    CTransform* m_pTransformCom = nullptr;
    CCollider* m_pColliderCom = nullptr;
    Engine::CRcTex* m_pBufferCom = nullptr;
    _float m_fFlicker = 0.f;
    int m_iLightIdx = -1;

public:
    // 외부(CCYStage)에서 조명 인덱스 할당
    void Set_LightIndex(int idx) { m_iLightIdx = idx; }
    int  Get_LightIndex() const { return m_iLightIdx; }

    // 조명 등록 (CCYStage에서 호출)
    void Apply_Light();
    void Disable_Light();

    // 위치 반환 (거리 계산용)
    _vec3 Get_Pos()
    {
        _vec3 vPos;
        m_pTransformCom->Get_Info(INFO_POS, &vPos);
        return vPos;
    }

public:
    static CTorch* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
    virtual void Free();
};