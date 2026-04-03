#include "pch.h"
#include "CJSCamera.h"
#include "CDInputMgr.h"
#include "CManagement.h"
#include "CTransform.h"

CJSCamera::CJSCamera(LPDIRECT3DDEVICE9 pGraphicDev)
    : CCamera(pGraphicDev)
{
}

CJSCamera::CJSCamera(const CJSCamera& rhs)
    : CCamera(rhs)
{
}

CJSCamera::~CJSCamera()
{
}

HRESULT CJSCamera::Ready_GameObject(const _vec3* pEye,
    const _vec3* pAt,
    const _vec3* pUp,
    const _float& fFov,
    const _float& fAspect,
    const _float& fNear,
    const _float& fFar)
{
    m_vEye = *pEye;
    m_vAt = *pAt;
    m_vUp = *pUp;

    m_fFov = fFov;
    m_fAspect = fAspect;
    m_fNear = fNear;
    m_fFar = fFar;

    if (FAILED(CCamera::Ready_GameObject()))
        return E_FAIL;

    return S_OK;
}

_int CJSCamera::Update_GameObject(const _float& fTimeDelta)
{
    _int iExit = CCamera::Update_GameObject(fTimeDelta);

    return iExit;
}

void CJSCamera::LateUpdate_GameObject(const _float& fTimeDelta)
{
    CCamera::LateUpdate_GameObject(fTimeDelta);
    
    Get_PlayerPos();
    Get_PlayerLook();

    _vec3 vTargetLook = -m_vPlayerLook;
    m_vCamLook.x += (vTargetLook.x - m_vCamLook.x) * 5.f * fTimeDelta;
    m_vCamLook.z += (vTargetLook.z - m_vCamLook.z) * 5.f * fTimeDelta;

    m_vEye.x = m_vPlayerPos.x + m_vCamLook.x * 15.f;
    m_vEye.z = m_vPlayerPos.z + m_vCamLook.z * 13.f;

    m_vAt.x = m_vPlayerPos.x;
    m_vAt.z = m_vPlayerPos.z;
}

void CJSCamera::Get_PlayerPos()
{
    CTransform* pPlayerTrans = dynamic_cast<CTransform*>(CManagement::GetInstance()->Get_Component(ID_DYNAMIC, L"GameLogic_Layer", L"JSPlayer", L"Com_Transform"));

    if (!pPlayerTrans)
        return;

    pPlayerTrans->Get_Info(INFO_POS, &m_vPlayerPos);
}

void CJSCamera::Get_PlayerLook()
{
    CTransform* pPlayerTrans = dynamic_cast<CTransform*>(CManagement::GetInstance()->Get_Component(ID_DYNAMIC, L"GameLogic_Layer", L"JSPlayer", L"Com_Transform"));

    if (!pPlayerTrans)
        return;

    pPlayerTrans->Get_Info(INFO_LOOK, &m_vPlayerLook);
}

CJSCamera* CJSCamera::Create(LPDIRECT3DDEVICE9 pGraphicDev,
    const _vec3* pEye, const _vec3* pAt, const _vec3* pUp,
    const _float& fFov, const _float& fAspect, const _float& fNear, const _float& fFar)
{
    CJSCamera* pCamera = new CJSCamera(pGraphicDev);

    if (FAILED(pCamera->Ready_GameObject(pEye, pAt, pUp,
        fFov, fAspect, fNear, fFar)))
    {
        Safe_Release(pCamera);
        MSG_BOX("Camera Create Failed");
        return nullptr;
    }

    return pCamera;
}

void CJSCamera::Free()
{
    CCamera::Free();
}