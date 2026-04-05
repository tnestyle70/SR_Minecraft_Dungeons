#include "pch.h"
#include "CDynamicCamera.h"
#include "CDInputMgr.h"

CDynamicCamera::CDynamicCamera(LPDIRECT3DDEVICE9 pGraphicDev)
    : CCamera(pGraphicDev), m_bFix(true), m_bCheck(true)
{
}

CDynamicCamera::CDynamicCamera(const CDynamicCamera& rhs)
    : CCamera(rhs), m_bFix(true), m_bCheck(true)
{
}

CDynamicCamera::~CDynamicCamera()
{
}

HRESULT CDynamicCamera::Ready_GameObject(const _vec3* pEye, 
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

    m_fSpeed = 50.f;

    return S_OK;
}

_int CDynamicCamera::Update_GameObject(const _float& fTimeDelta)
{
    _int iExit = CCamera::Update_GameObject(fTimeDelta);

    // 드래곤 카메라 전환은 CNetworkPlayer에서 Set_DragonCam() 호출로 제어
    // (G키 직접 감지 제거 — CNetworkPlayer의 G키 탑승/하차 로직과 충돌 방지)

    // F2 토글
    if (GetAsyncKeyState(VK_F2) & 0x8000)
    {
        if (!m_bF2Check)
        {
            m_bF2Check = true;
            m_bFollowMode = !m_bFollowMode;
            m_bFix = !m_bFollowMode;  // ← 추가
        }
    }
    else
    {
        m_bF2Check = false;
    }

    if (m_bActionCam)
    {
        Update_ActionCam(fTimeDelta);
    }
    else if (m_pTargetTransform && m_bFollowMode)
    {
        _vec3 vPlayerPos;
        m_pTargetTransform->Get_Info(INFO_POS, &vPlayerPos);

        if (m_bDragonCam)
            m_fCamBlend = min(1.f, m_fCamBlend + fTimeDelta * 2.f);
        else
            m_fCamBlend = max(0.f, m_fCamBlend - fTimeDelta * 2.f);
        _vec3 vOffset;
        if (m_fCamBlend > 0.0001f)
        {
            //드래곤 전방 벡터에서 yaw 계산
            D3DXVec3Lerp(&m_vSmoothDragonDir, &m_vSmoothDragonDir, &m_vDragonDir, fTimeDelta * 1.f);
            _vec3 vForward = m_vSmoothDragonDir;
            vForward.y = 0.f;
            if (D3DXVec3Length(&vForward) > 0.0001f)
                D3DXVec3Normalize(&vForward, &vForward);
            else
                vForward = { 0.f, 0.f, 1.f };
            
            float fYaw = atan2f(vForward.x, vForward.z);
            _matrix matYaw;
            D3DXMatrixRotationY(&matYaw, fYaw);
            //드래곤 오프셋을 yaw 만큼 회전
            _vec3 vRotatedDragonOffset;
            D3DXVec3TransformNormal(&vRotatedDragonOffset, &m_vDragonOffset, &matYaw);
            // 일반 오프셋 ↔ 회전된 드래곤 오프셋 블렌딩
            D3DXVec3Lerp(&vOffset, &m_vFollowOffset, &vRotatedDragonOffset, m_fCamBlend);

            //자유 시점 공전 적용
            if (m_bFreeLook || fabsf(m_fFreeCamYaw) > 0.001f || fabsf(m_fFreeCamPitch) > 0.001f)
            {
                // 좌클릭 해제 시 부드럽게 0으로 복귀
                if (!m_bFreeLook)
                {
                    float fDecay = 1.f - fTimeDelta * 5.f;   // 감쇠 팩터
                    m_fFreeCamYaw *= fDecay;
                    m_fFreeCamPitch *= fDecay;
                    if (fabsf(m_fFreeCamYaw) < 0.001f) m_fFreeCamYaw = 0.f;
                    if (fabsf(m_fFreeCamPitch) < 0.001f) m_fFreeCamPitch = 0.f;
                }

                // 피치 클램프 — 뒤집힘 방지 (-70도 ~ +70도)
                m_fFreeCamPitch = max(-1.22f, min(1.22f, m_fFreeCamPitch));

                // 기본 오프셋(vOffset) 위에 추가 회전 적용
                // 1) 수평 회전 (Y축, freeCamYaw)
                D3DXMATRIX matFreeYaw;
                D3DXMatrixRotationY(&matFreeYaw, m_fFreeCamYaw);

                // 2) 수직 회전 (Right축, freeCamPitch)
                //    Right = WorldUp x 현재오프셋 → 오프셋에 수직인 수평축
                _vec3 vUp = { 0.f, 1.f, 0.f };
                _vec3 vRight;
                D3DXVec3Cross(&vRight, &vUp, &vOffset);
                D3DXVec3Normalize(&vRight, &vRight);

                D3DXMATRIX matFreePitch;
                D3DXMatrixRotationAxis(&matFreePitch, &vRight, m_fFreeCamPitch);

                // 합성: Pitch 먼저, Yaw 나중 (순서 중요 — 수평 공전이 직관적)
                D3DXMATRIX matFree = matFreePitch * matFreeYaw;
                D3DXVec3TransformNormal(&vOffset, &vOffset, &matFree);
            }
        }
        else
        {
            vOffset = m_vFollowOffset;
        }
        //머지해서 각자 잘 되나 

        m_vEye = vPlayerPos + vOffset;

        //Look -at 탑승 중이면 드래곤 전방 약간 앞을 바라봄
        _vec3 vLookOffset = { 0.f, 1.5f, 0.f };
        if (m_fCamBlend > 0.001f)
        {
            _vec3 vForward = m_vDragonDir;
            vForward.y = 0.f;
            if (D3DXVec3Length(&vForward) > 0.001f)
                D3DXVec3Normalize(&vForward, &vForward);
            _vec3 vAHeadOffset = vForward * 5.f + _vec3(0.f, 1.5f, 0.f);
            D3DXVec3Lerp(&vLookOffset, &_vec3(0.f, 1.5f, 0.f), &vAHeadOffset, m_fCamBlend);
        }
        m_vAt = vPlayerPos + vLookOffset;

        // free-look: override At to player center (not dragon forward)
        if (m_fCamBlend > 0.001f &&
            (fabsf(m_fFreeCamYaw) > 0.001f || fabsf(m_fFreeCamPitch) > 0.001f))
        {
            _vec3 vPlayerAt = vPlayerPos + _vec3(0.f, 2.f, 0.f);
            // smooth blend: partial free-look → partial dragon forward
            float fFreeBlend = max(fabsf(m_fFreeCamYaw), fabsf(m_fFreeCamPitch));
            fFreeBlend = min(fFreeBlend * 2.f, 1.f);  // 0.5rad at full
            D3DXVec3Lerp(&m_vAt, &m_vAt, &vPlayerAt, fFreeBlend);
        }

        //D3DXVec3Lerp(&vOffset, &m_vFollowOffset, &m_vDragonOffset, m_fCamBlend);
        //m_vEye = vPlayerPos + vOffset;
        //m_vAt = vPlayerPos + _vec3(0.f, 1.5f, 0.f);

        m_vAt = vPlayerPos + _vec3(0.f, 1.5f, 0.f);

        // 카메라 쉐이킹
        if (m_fShakeTimer < m_fShakeDuration)
        {
            m_fShakeTimer += fTimeDelta;
            float fStrength = m_fShakeIntensity * (1.f - m_fShakeTimer / m_fShakeDuration);
            m_vEye.x += ((rand() % 100) / 100.f - 0.5f) * fStrength;
            m_vEye.y += ((rand() % 100) / 100.f - 0.5f) * fStrength;
            m_vEye.z += ((rand() % 100) / 100.f - 0.5f) * fStrength;
        }

    }
    else
    {
        Key_Input(fTimeDelta);
        if (m_bFix)
        {
            Mouse_Fix();
            Mouse_Move();
        }
    }

    return iExit;
}

void CDynamicCamera::LateUpdate_GameObject(const _float& fTimeDelta)
{
    CCamera::LateUpdate_GameObject(fTimeDelta);
}

_int CDynamicCamera::Update_ActionCam(const _float& fTimeDelta)
{
    if (!m_bActionCam)
        return 0;

    //WayPoint - vEye 로 방향 구해서, 거리 판단 이후 그 방향만큼 이동
    _vec3 vDir = m_wpTarget.vEye - m_vEye;
    _float fDist = D3DXVec3Length(&vDir);
   
    if (fDist < 1.f)
    {
        //목적지에 도달했는지 판단
        if (m_deqWayPoints.empty())
        {
            m_bActionCam = false;
            return 0;
        }
        //Move to Next Way Point
        m_wpTarget = m_deqWayPoints.front();
        m_deqWayPoints.pop_front();
        //new target의 방향으로 재계산
        vDir = m_wpTarget.vEye - m_vEye;
        fDist = D3DXVec3Length(&vDir);
    }
    if (fDist > 0.1f)
    {
        //Eye 업데이트
        D3DXVec3Normalize(&vDir, &vDir);
        m_vEye += vDir * fTimeDelta * m_fSpeed;
    }
    D3DXVec3Lerp(&m_vAt, &m_vAt, &m_wpTarget.vAt, fTimeDelta * 1.f);

    return 0;
}

void CDynamicCamera::SetActionCam(eActionCamType eType)
{
    switch (eType)
    {
    case eActionCamType::SQUID_COAST:
        Set_SquidCoastActionCam();
        break;
    case eActionCamType::GB_STAGE:
        Set_GBStageActionCam();
        break;
    default:
        break;
    }

    //액션캠 시작, 끝 위치 설정 - 스테이지 기본 세팅대로, 웨이 포인트들 지정해주고 웨이 포인트 끝나면 캠 종료
    //m_wpStart = { m_vEye, m_vUp, 0.f };
    //처음 Target 설정
    m_wpTarget = m_deqWayPoints.front();
    m_deqWayPoints.pop_front();

    m_bActionCam = true;

    return;
}

void CDynamicCamera::Set_SquidCoastActionCam()
{
    //eye 위치 처음 cam 위치로 설정, follow offset에서 -12의 값으로 오프셋을 설정해주고 있는 상태
    m_vEye = _vec3(-48.f, 1.f, -163.f) + m_vFollowOffset;
    m_vAt = { -25.f, 1.f, -114 };
    
    //Setting Way Points
    _vec3 vWaysPoints[] = {
        {-25, 20.f , -114}, //주민
        {-45, 20.f, -62}, //몬스터 웨이포인트1
        {40, 20.f, 44}, //몬스터 웨이포인트2
        {44, 20.f, 94}, //다리 위 
        {94, -10.f, 86}, //아래 마을 1
        {94, -10.f, 55}, //아래 마을 2
        {44, 20.f, 94}, //다리 위
        {-28, 20.f, 84}, //스켈레톤 1 
        {-30, 20.f, 107}, //스켈레톤 2 - 돌아가는 분기점
        {44, 20.f, 94}, //다리 위
        {44, 20.f, 200}, //가디언 
        {41, 40.f, 260}, //레드스톤 1 
        {18, 40.f, 293}, //레드스톤 2
        {60, 40.f, 302}, //레드스톤 3
        {41, 40.f, 260}, //레드스톤 4

        //복귀
        {44, 20.f, 94}, //다리 위
        {40, 20.f, 44}, //몬스터 웨이포인트2
        {-45, 20.f, -62}, //몬스터 웨이포인트1
    };
    
    for (int i = 0; i < 16; ++i)
    {
        m_deqWayPoints.push_back({ vWaysPoints[i], vWaysPoints[i + 1] ,{m_vUp.x, m_vUp.y + 10.f, m_vUp.z}, 5.f });
    }
    
    m_deqWayPoints.push_back({ { m_vEye.x, m_vEye.y , m_vEye.z }, {m_vAt.x, m_vAt.y, m_vAt.z} ,{m_vUp.x, m_vUp.y, m_vUp.z}, 5.f });
}

void CDynamicCamera::Set_GBStageActionCam()
{
    //eye 위치 처음 cam 위치로 설정, follow offset에서 -12의 값으로 오프셋을 설정해주고 있는 상태
    m_vEye = _vec3(0.f, 10.f, 0.f) + m_vFollowOffset;
    m_vAt = { 132.f, 30.f, 156.f }; //첫번째 웨이 포인트

    m_fSpeed = 100.f;
    
    //Setting Way Points
    _vec3 vWaysPoints[] = {
        {132, 30.f, 156}, //웨이 포인트 1
        {328, 40.f, 203}, //웨이포인트 2
        {366, 10.f, -19}, //웨이 포인트 3
        {260, 30.f, -195}, //웨이포인트 4
        {20.f, 30.f, -20.f}, //웨이포인트 5
    };
    
    for (int i = 0; i < 4; ++i)
    {
        m_deqWayPoints.push_back({ vWaysPoints[i], vWaysPoints[i + 1] ,{m_vUp.x, m_vUp.y + 10.f, m_vUp.z}, 5.f });
    }
    
    m_deqWayPoints.push_back({ { m_vEye.x, m_vEye.y , m_vEye.z }, {m_vAt.x, m_vAt.y, m_vAt.z} ,{m_vUp.x, m_vUp.y, m_vUp.z}, 5.f });
}

void CDynamicCamera::Key_Input(const _float& fTimeDelta)
{
    _matrix matCamWorld;
    D3DXMatrixInverse(&matCamWorld, 0, &m_matView);

    if (CDInputMgr::GetInstance()->Get_DIKeyState(DIK_RIGHT) & 0x80)
    {
        _vec3   vRight;
        memcpy(&vRight, &matCamWorld.m[0][0], sizeof(_vec3));

        _vec3   vLength = *D3DXVec3Normalize(&vRight, &vRight) * m_fSpeed * fTimeDelta;

        m_vEye += vLength;
        m_vAt += vLength;
    }

    if (CDInputMgr::GetInstance()->Get_DIKeyState(DIK_LEFT) & 0x80)
    {
        _vec3   vRight;
        memcpy(&vRight, &matCamWorld.m[0][0], sizeof(_vec3));

        _vec3   vLength = *D3DXVec3Normalize(&vRight, &vRight) * m_fSpeed * fTimeDelta;

        m_vEye -= vLength;
        m_vAt  -= vLength;
    }

    if (CDInputMgr::GetInstance()->Get_DIKeyState(DIK_UP) & 0x80)
    {
        _vec3   vLook;
        memcpy(&vLook, &matCamWorld.m[2][0], sizeof(_vec3));

        _vec3   vLength = *D3DXVec3Normalize(&vLook, &vLook) * m_fSpeed * fTimeDelta;

        m_vEye += vLength;
        m_vAt += vLength;
    }

    if (CDInputMgr::GetInstance()->Get_DIKeyState(DIK_DOWN) & 0x80)
    {
        _vec3   vLook;
        memcpy(&vLook, &matCamWorld.m[2][0], sizeof(_vec3));

        _vec3   vLength = *D3DXVec3Normalize(&vLook, &vLook) * m_fSpeed * fTimeDelta;

        m_vEye -= vLength;
        m_vAt  -= vLength;
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

void CDynamicCamera::Mouse_Move()
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

void CDynamicCamera::Mouse_Fix()
{
    POINT       ptMouse{ WINCX >> 1, WINCY >> 1 };

    ClientToScreen(g_hWnd, &ptMouse);
    SetCursorPos(ptMouse.x, ptMouse.y);

}

CDynamicCamera* CDynamicCamera::Create(LPDIRECT3DDEVICE9 pGraphicDev,
    const _vec3* pEye, const _vec3* pAt, const _vec3* pUp,
    const _float& fFov,    const _float& fAspect, const _float& fNear, const _float& fFar)
{
    CDynamicCamera* pCamera = new CDynamicCamera(pGraphicDev);

    if (FAILED(pCamera->Ready_GameObject(pEye, pAt, pUp, 
        fFov, fAspect, fNear, fFar)))
    {
        Safe_Release(pCamera);
        MSG_BOX("Camera Create Failed");
        return nullptr;
    }

    return pCamera;
}

// ─────────────────────────────────────────────────────────────────────────────
// ImGui debug panel
// ─────────────────────────────────────────────────────────────────────────────
void CDynamicCamera::Render_GameObject()
{
    /*
    ImGui::Begin("Camera Debug");

    ImGui::Text("-- Follow Offset --");
    ImGui::DragFloat3("FollowOffset", (float*)&m_vFollowOffset, 0.5f, -50.f, 50.f);

    ImGui::Separator();
    ImGui::Text("-- Dragon Camera --");
    ImGui::DragFloat3("DragonOffset", (float*)&m_vDragonOffset, 0.5f, -50.f, 50.f);
    ImGui::SliderFloat("CamBlend", &m_fCamBlend, 0.f, 1.f);
    ImGui::Text("DragonCam: %s", m_bDragonCam ? "ON" : "OFF");
    ImGui::Text("DragonDir: (%.2f, %.2f, %.2f)",
        m_vDragonDir.x, m_vDragonDir.y, m_vDragonDir.z);

    ImGui::Separator();
    ImGui::Text("-- Current State --");
    ImGui::Text("Eye: (%.1f, %.1f, %.1f)", m_vEye.x, m_vEye.y, m_vEye.z);
    ImGui::Text("At:  (%.1f, %.1f, %.1f)", m_vAt.x, m_vAt.y, m_vAt.z);
    ImGui::Text("Mode: %s", m_bFollowMode ? "Follow" : "Free");

    ImGui::End();
    */
}

void CDynamicCamera::Free()
{
    CCamera::Free();
}
