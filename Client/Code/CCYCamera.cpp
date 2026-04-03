#include "pch.h"
#include "CCYCamera.h"
#include "CCYPlayer.h"

CCYCamera::CCYCamera(LPDIRECT3DDEVICE9 pGraphicDev)
    : CCamera(pGraphicDev)
{
}

CCYCamera::~CCYCamera()
{
}

HRESULT CCYCamera::Ready_GameObject(const _vec3* pEye, const _vec3* pAt, const _vec3* pUp,
    float fFov, float fAspect, float fNear, float fFar)
{
    m_vEye = *pEye;
    m_vAt = *pAt;
    m_vUp = *pUp;
    m_fFov = fFov;
    m_fAspect = fAspect;
    m_fNear = fNear;
    m_fFar = fFar;

    return CCamera::Ready_GameObject();
}

_int CCYCamera::Update_GameObject(const _float& fTimeDelta)
{
    // F2 토글 - 자유시점
    if (GetAsyncKeyState(VK_F2) & 0x8000)
    {
        if (!m_bF2Check)
        {
            m_bF2Check = true;
            m_bFreeMode = !m_bFreeMode;
        }
    }
    else m_bF2Check = false;

    // TAB 토글 - 마우스 고정
    if (GetAsyncKeyState(VK_TAB) & 0x8000)
    {
        if (!m_bTabCheck)
        {
            m_bTabCheck = true;
            m_bMouseFix = !m_bMouseFix;
        }
    }
    else m_bTabCheck = false;

    // 카메라 이동
    Free_Move(fTimeDelta);
    FPS_MouseRotate();

    // 마우스 중앙 고정
    if (m_bMouseFix)
    {
        POINT ptMouse{ WINCX >> 1, WINCY >> 1 };
        ClientToScreen(g_hWnd, &ptMouse);
        SetCursorPos(ptMouse.x, ptMouse.y);
    }

    return CCamera::Update_GameObject(fTimeDelta);
}

void CCYCamera::LateUpdate_GameObject(const _float& fTimeDelta)
{ 
    if (m_pCYPlayer && !m_bFreeMode)
    {

        Engine::CTransform* pTrans = m_pCYPlayer->Get_Transform();
        if (pTrans)
        {
            _vec3 vPlayerPos;
            pTrans->Get_Info(INFO_POS, &vPlayerPos);
            _vec3 vLook = m_vAt - m_vEye;
            m_vEye.y = vPlayerPos.y + 1.7f;
            m_vAt = m_vEye + vLook;
            D3DXMatrixLookAtLH(&m_matView, &m_vEye, &m_vAt, &m_vUp);
            m_pGraphicDev->SetTransform(D3DTS_VIEW, &m_matView);
        }
    }
    CCamera::LateUpdate_GameObject(fTimeDelta);
}

void CCYCamera::FPS_MouseRotate()
{
    D3DXMatrixLookAtLH(&m_matView, &m_vEye, &m_vAt, &m_vUp);

    // 마우스 X → 좌우 회전
    _long dwMouseX = CDInputMgr::GetInstance()->Get_DIMouseMove(DIMS_X);
    if (dwMouseX)
    {
        _vec3 vUp{ 0.f, 1.f, 0.f };
        _vec3 vLook = m_vAt - m_vEye;
        _matrix matRot;
        D3DXMatrixRotationAxis(&matRot, &vUp, D3DXToRadian(dwMouseX / 10.f));
        D3DXVec3TransformNormal(&vLook, &vLook, &matRot);
        m_vAt = m_vEye + vLook;
    }

    // 마우스 Y → 상하 회전 (±80도 클램프)
    _long dwMouseY = CDInputMgr::GetInstance()->Get_DIMouseMove(DIMS_Y);
    if (dwMouseY)
    {
        float fDelta = dwMouseY / 10.f;
        float fNewPitch = m_fPitch + fDelta;

        if (fNewPitch > 80.f) fDelta = 80.f - m_fPitch;
        if (fNewPitch < -80.f) fDelta = -80.f - m_fPitch;
        m_fPitch += fDelta;

        D3DXMatrixLookAtLH(&m_matView, &m_vEye, &m_vAt, &m_vUp);
        _matrix matCamWorld;
        D3DXMatrixInverse(&matCamWorld, 0, &m_matView);
        _vec3 vRight;
        memcpy(&vRight, &matCamWorld.m[0][0], sizeof(_vec3));

        _vec3 vLook = m_vAt - m_vEye;
        _matrix matRot;
        D3DXMatrixRotationAxis(&matRot, &vRight, D3DXToRadian(fDelta));
        D3DXVec3TransformNormal(&vLook, &vLook, &matRot);
        m_vAt = m_vEye + vLook;
    }
}

void CCYCamera::Free_Move(const _float& fTimeDelta)
{
    D3DXMatrixLookAtLH(&m_matView, &m_vEye, &m_vAt, &m_vUp);
    _matrix matCamWorld;
    D3DXMatrixInverse(&matCamWorld, 0, &m_matView);

    // Shift 누르면 5배 빠르게
    float fSpeed = m_fFreeSpeed;
    if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
        fSpeed *= 5.f;

    _vec3 vLook, vRight;
    memcpy(&vLook, &matCamWorld.m[2][0], sizeof(_vec3));
    memcpy(&vRight, &matCamWorld.m[0][0], sizeof(_vec3));
    D3DXVec3Normalize(&vLook, &vLook);
    D3DXVec3Normalize(&vRight, &vRight);

    // 1인칭 모드에서는 Y축 제거
    if (!m_bFreeMode)
    {
        vLook.y = 0.f;
        float fLen = D3DXVec3Length(&vLook);
        if (fLen > 0.001f) D3DXVec3Normalize(&vLook, &vLook);
        vRight.y = 0.f;
        D3DXVec3Normalize(&vRight, &vRight);
    }

    if (CDInputMgr::GetInstance()->Get_DIKeyState(DIK_W) & 0x80)
    {
        m_vEye += vLook * fSpeed * fTimeDelta; m_vAt += vLook * fSpeed * fTimeDelta;
    }
    if (CDInputMgr::GetInstance()->Get_DIKeyState(DIK_S) & 0x80)
    {
        m_vEye -= vLook * fSpeed * fTimeDelta; m_vAt -= vLook * fSpeed * fTimeDelta;
    }
    if (CDInputMgr::GetInstance()->Get_DIKeyState(DIK_A) & 0x80)
    {
        m_vEye -= vRight * fSpeed * fTimeDelta; m_vAt -= vRight * fSpeed * fTimeDelta;
    }
    if (CDInputMgr::GetInstance()->Get_DIKeyState(DIK_D) & 0x80)
    {
        m_vEye += vRight * fSpeed * fTimeDelta; m_vAt += vRight * fSpeed * fTimeDelta;
    }
}

CCYCamera* CCYCamera::Create(LPDIRECT3DDEVICE9 pGraphicDev,
    const _vec3* pEye, const _vec3* pAt, const _vec3* pUp)
{
    CCYCamera* pCamera = new CCYCamera(pGraphicDev);
    if (FAILED(pCamera->Ready_GameObject(pEye, pAt, pUp)))
    {
        Safe_Release(pCamera);
        MSG_BOX("CCYCamera Create Failed");
        return nullptr;
    }
    return pCamera;
}

void CCYCamera::Free()
{
    CCamera::Free();
}