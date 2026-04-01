#include "pch.h"
#include "CJSCamera.h"
#include "CDInputMgr.h"
#include "CManagement.h"
#include "CTransform.h"

CJSCamera::CJSCamera(LPDIRECT3DDEVICE9 pGraphicDev)
    : CCamera(pGraphicDev)
    , m_bFix(true)
    , m_bCheck(true)
{
}

CJSCamera::CJSCamera(const CJSCamera& rhs)
    : CCamera(rhs)
    , m_bFix(true)
    , m_bCheck(true)
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

    Key_Input(fTimeDelta);

    if (m_bFix)
    {
        Mouse_Fix();
        Mouse_Move();
    }
    
    Get_PlayerPos();

    m_vEye = m_vPlayerPos + _vec3(0.f, 5.f, -7.f);
    m_vAt = m_vPlayerPos;
}

void CJSCamera::Key_Input(const _float& fTimeDelta)
{
    _matrix matCamWorld;
    D3DXMatrixInverse(&matCamWorld, 0, &m_matView);

    if (CDInputMgr::GetInstance()->Get_DIKeyState(DIK_D) & 0x80)
    {
        _vec3   vRight;
        memcpy(&vRight, &matCamWorld.m[0][0], sizeof(_vec3));

        _vec3   vLength = *D3DXVec3Normalize(&vRight, &vRight) * m_fSpeed * fTimeDelta;

        m_vEye += vLength;
        m_vAt += vLength;
    }

    if (CDInputMgr::GetInstance()->Get_DIKeyState(DIK_A) & 0x80)
    {
        _vec3   vRight;
        memcpy(&vRight, &matCamWorld.m[0][0], sizeof(_vec3));

        _vec3   vLength = *D3DXVec3Normalize(&vRight, &vRight) * m_fSpeed * fTimeDelta;

        m_vEye -= vLength;
        m_vAt -= vLength;
    }

    if (CDInputMgr::GetInstance()->Get_DIKeyState(DIK_W) & 0x80)
    {
        _vec3   vLook;
        memcpy(&vLook, &matCamWorld.m[2][0], sizeof(_vec3));

        _vec3   vLength = *D3DXVec3Normalize(&vLook, &vLook) * m_fSpeed * fTimeDelta;

        m_vEye += vLength;
        m_vAt += vLength;
    }

    if (CDInputMgr::GetInstance()->Get_DIKeyState(DIK_S) & 0x80)
    {
        _vec3   vLook;
        memcpy(&vLook, &matCamWorld.m[2][0], sizeof(_vec3));

        _vec3   vLength = *D3DXVec3Normalize(&vLook, &vLook) * m_fSpeed * fTimeDelta;

        m_vEye -= vLength;
        m_vAt -= vLength;
    }


    if (CDInputMgr::GetInstance()->Get_DIKeyState(DIK_TAB) & 0x80)
    {
        if (m_bCheck)
            return;

        m_bCheck = true;

        if (m_bFix)
            m_bFix = false;

        else
            m_bFix = true;

    }

    else
    {
        m_bCheck = false;
    }

    if (false == m_bFix)
        return;
}

void CJSCamera::Mouse_Move()
{
    _matrix matCamWorld;
    D3DXMatrixInverse(&matCamWorld, 0, &m_matView);

    _long   dwMouseMove(0);

    if (dwMouseMove = CDInputMgr::GetInstance()->Get_DIMouseMove(DIMS_Y))
    {
        _vec3   vRight;
        memcpy(&vRight, &matCamWorld.m[0][0], sizeof(_vec3));

        _vec3   vLook = m_vAt - m_vEye;

        _matrix matRot;

        D3DXMatrixRotationAxis(&matRot, &vRight, D3DXToRadian(dwMouseMove / 10.f));

        D3DXVec3TransformNormal(&vLook, &vLook, &matRot);

        m_vAt = m_vEye + vLook;
    }

    if (dwMouseMove = CDInputMgr::GetInstance()->Get_DIMouseMove(DIMS_X))
    {
        _vec3   vUp{ 0.f, 1.f, 0.f };

        _vec3   vLook = m_vAt - m_vEye;

        _matrix matRot;

        D3DXMatrixRotationAxis(&matRot, &vUp, D3DXToRadian(dwMouseMove / 10.f));

        D3DXVec3TransformNormal(&vLook, &vLook, &matRot);

        m_vAt = m_vEye + vLook;
    }

}

void CJSCamera::Mouse_Fix()
{
    POINT       ptMouse{ WINCX >> 1, WINCY >> 1 };

    ClientToScreen(g_hWnd, &ptMouse);
    SetCursorPos(ptMouse.x, ptMouse.y);

}

void CJSCamera::Get_PlayerPos()
{
    CTransform* pPlayerTrans = dynamic_cast<CTransform*>(CManagement::GetInstance()->Get_Component(ID_DYNAMIC, L"GameLogic_Layer", L"JSPlayer", L"Com_Transform"));

    if (!pPlayerTrans)
        return;

    pPlayerTrans->Get_Info(INFO_POS, &m_vPlayerPos);
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