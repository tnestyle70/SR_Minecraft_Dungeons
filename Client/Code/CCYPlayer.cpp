#include "pch.h"
#include "CCYPlayer.h"
#include "CCYCamera.h"
#include "CRenderer.h"

CCYPlayer::CCYPlayer(LPDIRECT3DDEVICE9 pGraphicDev)
    : CGameObject(pGraphicDev)
{
}

CCYPlayer::~CCYPlayer()
{
}

HRESULT CCYPlayer::Ready_GameObject()
{
    if (FAILED(Add_Component()))
        return E_FAIL;

    m_pTransformCom->Set_Pos(-3.f, 19.f, 19.f);

    return S_OK;
}

HRESULT CCYPlayer::Add_Component()
{
    CComponent* pComponent = nullptr;

    // Transform
    pComponent = m_pTransformCom = dynamic_cast<Engine::CTransform*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_DYNAMIC].insert({ L"Com_Transform", pComponent });

    // Collider
    m_pColliderCom = Engine::CCollider::Create(m_pGraphicDev,
        _vec3(0.8f, 1.8f, 0.8f), _vec3(0.f, 0.9f, 0.f));
    if (!m_pColliderCom) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Collider", m_pColliderCom });

    // 오른팔 UV
    FACE_UV uvRArm[6] = {
        {0.6875f, 0.3125f, 0.75f,   0.5f   },
        {0.8125f, 0.3125f, 0.875f,  0.5f   },
        {0.6875f, 0.25f,   0.75f,   0.3125f},
        {0.75f,   0.25f,   0.8125f, 0.3125f},
        {0.625f,  0.3125f, 0.6875f, 0.5f   },
        {0.75f,   0.3125f, 0.8125f, 0.5f   },
    };
    m_pHandBufferCom = CPlayerBody::Create(m_pGraphicDev, uvRArm);
    if (!m_pHandBufferCom) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_HandBuffer", m_pHandBufferCom });

    // 왼팔 UV
    FACE_UV uvLArm[6] = {
        {0.5625f, 0.8125f, 0.625f,  1.0f   },
        {0.6875f, 0.8125f, 0.75f,   1.0f   },
        {0.5625f, 0.75f,   0.625f,  0.8125f},
        {0.625f,  0.75f,   0.6875f, 0.8125f},
        {0.5f,    0.8125f, 0.5625f, 1.0f   },
        {0.625f,  0.8125f, 0.6875f, 1.0f   },
    };
    m_pLeftHandBufferCom = CPlayerBody::Create(m_pGraphicDev, uvLArm);
    if (!m_pLeftHandBufferCom) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_LeftHandBuffer", m_pLeftHandBufferCom });

    // 플레이어 텍스처
    m_pHandTextureCom = dynamic_cast<Engine::CTexture*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_PlayerTexture"));
    if (!m_pHandTextureCom) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_HandTexture", m_pHandTextureCom });

    // 칼 버퍼
    pComponent = m_pSwordBufferCom = dynamic_cast<Engine::CRcTex*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RcTex"));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_SwordBuffer", pComponent });

    // 칼 텍스처
    m_pSwordTextureCom = dynamic_cast<Engine::CTexture*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_SwordTexture"));
    if (!m_pSwordTextureCom) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_SwordTexture", m_pSwordTextureCom });

    // 활 버퍼
    pComponent = m_pBowBufferCom = dynamic_cast<Engine::CRcTex*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RcTex"));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_BowBuffer", pComponent });

    // 활 텍스처
    m_pBowTextureCom = dynamic_cast<Engine::CTexture*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_BowStandbyTexture"));
    if (!m_pBowTextureCom) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_BowTexture", m_pBowTextureCom });

    return S_OK;
}

_int CCYPlayer::Update_GameObject(const _float& fTimeDelta)
{
    _int iExit = CGameObject::Update_GameObject(fTimeDelta);

    // 공격 콤보 타이머
    if (m_iComboStep > 0)
    {
        m_fAtkTime += fTimeDelta;
        if (m_fAtkTime >= m_fAtkDuration)
        {
            if (m_fComboTimer <= 0.f)
                m_fComboTimer = m_fComboWindow;
        }
    }
    if (m_fComboTimer > 0.f)
    {
        m_fComboTimer -= fTimeDelta;
        if (m_fComboTimer <= 0.f)
        {
            m_iComboStep = 0;
            m_fAtkTime = 0.f;
        }
    }

    // 좌클릭 - 칼 공격
    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
    {
        if (!m_bAtkInput)
        {
            m_bAtkInput = true;
            if (m_iComboStep == 0 ||
                (m_fAtkTime >= m_fAtkDuration && m_fComboTimer > 0.f))
            {
                m_iComboStep = (m_iComboStep % 3) + 1;
                m_fAtkTime = 0.f;
                m_fComboTimer = m_fComboWindow;
            }
        }
    }
    else m_bAtkInput = false;

    // 우클릭 - 활 발사
    bool bRClick = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
    if (bRClick)
    {
        m_bCharging = true;
        m_fCharge += fTimeDelta;
        if (m_fCharge > m_fMaxCharge)
            m_fCharge = m_fMaxCharge;
    }
    else if (m_bCharging)
    {
        // 카메라 Look 방향으로 발사
        _matrix matView = m_pCamera->Get_ViewMatrix();
        _matrix matCamWorld;
        D3DXMatrixInverse(&matCamWorld, 0, &matView);

        _vec3 vLook, vEye;
        memcpy(&vLook, &matCamWorld.m[2][0], sizeof(_vec3));
        memcpy(&vEye, &matCamWorld.m[3][0], sizeof(_vec3));
        D3DXVec3Normalize(&vLook, &vLook);

        _vec3 vFirePos = vEye + vLook * 0.5f;
        CPlayerArrow* pArrow = CPlayerArrow::Create(m_pGraphicDev, vFirePos, vLook, 10.f);
        if (pArrow)
            m_vecArrows.push_back(pArrow);

        m_fCharge = 0.f;
        m_bCharging = false;
    }

    // 화살 업데이트
    for (auto& pArrow : m_vecArrows)
        pArrow->Update_GameObject(fTimeDelta);

    // 죽은 화살 제거
    m_vecArrows.erase(
        remove_if(m_vecArrows.begin(), m_vecArrows.end(),
            [](CPlayerArrow* p) {
                if (p->Is_Dead() && !p->Is_Exploding()) { Safe_Release(p); return true; }
                return false;
            }),
        m_vecArrows.end());

    FPS_Gravity(fTimeDelta);
    FPS_BlockCollision();

    CRenderer::GetInstance()->Add_RenderGroup(RENDER_ALPHA, this);

    return iExit;
}

void CCYPlayer::LateUpdate_GameObject(const _float& fTimeDelta)
{
    Sync_WithCamera();
    CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CCYPlayer::Sync_WithCamera()
{
    if (!m_pCamera || !m_pTransformCom) return;

    _vec3 vCamEye = m_pCamera->Get_Eye();
    _vec3 vPos;
    m_pTransformCom->Get_Info(INFO_POS, &vPos);
    m_pTransformCom->Set_Pos(vCamEye.x, vPos.y, vCamEye.z);
}

void CCYPlayer::FPS_Gravity(const _float& fTimeDelta)
{
    if (m_bOnGround)
    {
        _vec3 vPos;
        m_pTransformCom->Get_Info(INFO_POS, &vPos);
        vPos.y -= 0.05f;
        m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
        m_fVelocityY = 0.f;
        return;
    }

    m_fVelocityY += m_fGravity * fTimeDelta;
    if (m_fVelocityY < m_fMaxFall)
        m_fVelocityY = m_fMaxFall;

    _vec3 vPos;
    m_pTransformCom->Get_Info(INFO_POS, &vPos);
    vPos.y += m_fVelocityY * fTimeDelta;
    m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
}

void CCYPlayer::FPS_BlockCollision()
{
    if (!m_pTransformCom || !m_pColliderCom) return;

    _vec3 vPos;
    m_pTransformCom->Get_Info(INFO_POS, &vPos);
    m_pColliderCom->Update_AABB(vPos);
    AABB tPlayerAABB = m_pColliderCom->Get_AABB();

    m_bOnGround = false;

    int iMinX = (int)floorf(tPlayerAABB.vMin.x);
    int iMaxX = (int)ceilf(tPlayerAABB.vMax.x);
    int iMinY = (int)floorf(tPlayerAABB.vMin.y) - 3;
    int iMaxY = (int)ceilf(tPlayerAABB.vMax.y);
    int iMinZ = (int)floorf(tPlayerAABB.vMin.z);
    int iMaxZ = (int)ceilf(tPlayerAABB.vMax.z);

    for (int y = iMinY; y <= iMaxY; ++y)
        for (int x = iMinX; x <= iMaxX; ++x)
            for (int z = iMinZ; z <= iMaxZ; ++z)
            {
                BlockPos tBlockPos = { x, y, z };
                if (!CBlockMgr::GetInstance()->HasBlock(tBlockPos)) continue;

                AABB tBlockAABB = CBlockMgr::GetInstance()->Get_BlockAABB(tBlockPos);
                if (!m_pColliderCom->IsColliding(tBlockAABB)) continue;

                _vec3 vResolve = m_pColliderCom->Resolve(tBlockAABB);
                vPos += vResolve;
                m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
                m_pColliderCom->Update_AABB(vPos);
                tPlayerAABB = m_pColliderCom->Get_AABB();

                if (vResolve.y > 0.01f)
                {
                    m_bOnGround = true;
                    m_fVelocityY = 0.f;
                }
                else if (vResolve.y < -0.01f)
                {
                    m_fVelocityY = 0.f;
                }

                if (fabsf(vResolve.y) < 0.01f &&
                    (fabsf(vResolve.x) > 0.f || fabsf(vResolve.z) > 0.f))
                {
                    BlockPos tAbove = { tBlockPos.x, tBlockPos.y + 1, tBlockPos.z };
                    BlockPos tAbove2 = { tBlockPos.x, tBlockPos.y + 2, tBlockPos.z };
                    if (!CBlockMgr::GetInstance()->HasBlock(tAbove) &&
                        !CBlockMgr::GetInstance()->HasBlock(tAbove2))
                    {
                        vPos.y += 1.f;
                        m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
                        m_pColliderCom->Update_AABB(vPos);
                        tPlayerAABB = m_pColliderCom->Get_AABB();
                    }
                }
            }
}

void CCYPlayer::Render_GameObject()
{
    if (!m_pCamera || !m_pHandBufferCom || !m_pHandTextureCom) return;

    _matrix matView = m_pCamera->Get_ViewMatrix();
    _matrix matCamWorld;
    D3DXMatrixInverse(&matCamWorld, 0, &matView);

    _vec3 vRight, vUp, vLook, vEye;
    memcpy(&vRight, &matCamWorld.m[0][0], sizeof(_vec3));
    memcpy(&vUp, &matCamWorld.m[1][0], sizeof(_vec3));
    memcpy(&vLook, &matCamWorld.m[2][0], sizeof(_vec3));
    memcpy(&vEye, &matCamWorld.m[3][0], sizeof(_vec3));
    D3DXVec3Normalize(&vRight, &vRight);
    D3DXVec3Normalize(&vUp, &vUp);
    D3DXVec3Normalize(&vLook, &vLook);

    m_pGraphicDev->Clear(0, nullptr, D3DCLEAR_ZBUFFER, 0, 1.f, 0);

    _matrix matCamRot = matCamWorld;
    matCamRot._41 = 0.f; matCamRot._42 = 0.f;
    matCamRot._43 = 0.f; matCamRot._44 = 1.f;

    _matrix matArmRot;
    D3DXMatrixRotationAxis(&matArmRot, &vRight, D3DXToRadian(-90.f));

    float fRatio = (m_iComboStep > 0) ? min(m_fAtkTime / m_fAtkDuration, 1.f) : 0.f;

    float fHandR = 0.28f + (-0.10f - 0.28f) * fRatio;
    float fHandU = -0.15f + (-0.25f - (-0.15f)) * fRatio;
    _vec3 vRHandPos = vEye + vRight * fHandR + vUp * fHandU + vLook * 0.4f;

    _matrix matScale, matTrans, matWorld;

    // ===== 오른손 =====
    D3DXMatrixScaling(&matScale, 0.05f, 0.15f, 0.05f);
    D3DXMatrixTranslation(&matTrans, vRHandPos.x, vRHandPos.y, vRHandPos.z);
    matWorld = matScale * matArmRot * matCamRot * matTrans;
    m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);
    m_pHandTextureCom->Set_Texture(0);
    m_pHandBufferCom->Render_Buffer();

    // ===== 칼 =====
    if (m_pSwordBufferCom && m_pSwordTextureCom)
    {
        _vec3 vLocalRight = { 1.f, 0.f, 0.f };
        _vec3 vLocalUp = { 0.f, 1.f, 0.f };

        float fSwingX = D3DXToRadian(-30.f + (-90.f - (-30.f)) * fRatio);
        float fSwingZ = D3DXToRadian(20.f + (-50.f - 20.f) * fRatio);

        _matrix matSwingX, matSwingZ, matSwing;
        D3DXMatrixRotationAxis(&matSwingX, &vLocalRight, fSwingX);
        D3DXMatrixRotationAxis(&matSwingZ, &vLocalUp, fSwingZ);
        matSwing = matSwingX * matSwingZ;

        _matrix matSwordScale, matSwordRotX, matPivot;
        D3DXMatrixScaling(&matSwordScale, 0.08f, 0.4f, 0.08f);
        D3DXMatrixRotationAxis(&matSwordRotX, &vLocalRight, D3DXToRadian(90.f));
        D3DXMatrixTranslation(&matPivot, 0.f, 0.f, 0.3f);

        m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
        m_pGraphicDev->SetRenderState(D3DRS_ALPHAREF, 128);
        m_pGraphicDev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
        m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
        m_pSwordTextureCom->Set_Texture(0);

        for (int i = 0; i < 8; ++i)
        {
            float fOffset = (i - 4.f) * 0.004f;
            _vec3 vSwordPos = vRHandPos + vRight * fOffset;
            D3DXMatrixTranslation(&matTrans, vSwordPos.x, vSwordPos.y, vSwordPos.z);
            matWorld = matSwordScale * matSwordRotX * matPivot * matSwing * matCamRot * matTrans;
            m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);
            m_pSwordBufferCom->Render_Buffer();
        }

        m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
        m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
    }

    // ===== 왼손 + 활 =====
    // ===== 왼손 + 활 =====
    if (m_pLeftHandBufferCom)
    {
        _vec3 vLHandPos = vEye - vRight * 0.28f - vUp * 0.18f + vLook * 0.4f;
        D3DXMatrixScaling(&matScale, 0.05f, 0.15f, 0.05f);
        D3DXMatrixTranslation(&matTrans, vLHandPos.x, vLHandPos.y, vLHandPos.z);
        matWorld = matScale * matArmRot * matCamRot * matTrans;
        m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);
        m_pHandTextureCom->Set_Texture(0);
        m_pLeftHandBufferCom->Render_Buffer();

        // 활 렌더링 - 원래 방식
        if (m_pBowBufferCom && m_pBowTextureCom)
        {
            _matrix matBowScale, matBowRotY, matBowRotX;
            D3DXMatrixScaling(&matBowScale, 0.18f, 0.18f, 0.18f);
            D3DXMatrixRotationAxis(&matBowRotY, &vUp, D3DXToRadian(90.f));
            D3DXMatrixRotationAxis(&matBowRotX, &vRight, D3DXToRadian(20.f));

            m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
            m_pGraphicDev->SetRenderState(D3DRS_ALPHAREF, 128);
            m_pGraphicDev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
            m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
            m_pBowTextureCom->Set_Texture(0);

            for (int i = 0; i < 8; ++i)
            {
                float fOffset = (i - 4.f) * 0.004f;
                _vec3 vBowPos = vLHandPos + vRight * fOffset + vLook * 0.2f;
                D3DXMatrixTranslation(&matTrans, vBowPos.x, vBowPos.y, vBowPos.z);
                matWorld = matBowScale * matBowRotX * matBowRotY * matCamRot * matTrans;
                m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);
                m_pBowBufferCom->Render_Buffer();
            }

            m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
            m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
        }
    }

    // 화살 렌더링
    m_pGraphicDev->SetRenderState(D3DRS_LIGHTING, FALSE);
    for (auto& pArrow : m_vecArrows)
        pArrow->Render_GameObject();
    m_pGraphicDev->SetRenderState(D3DRS_LIGHTING, TRUE);

    m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}

CCYPlayer* CCYPlayer::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
    CCYPlayer* pPlayer = new CCYPlayer(pGraphicDev);
    if (FAILED(pPlayer->Ready_GameObject()))
    {
        Safe_Release(pPlayer);
        MSG_BOX("CCYPlayer Create Failed");
        return nullptr;
    }
    return pPlayer;
}

void CCYPlayer::Free()
{
    for (auto& pArrow : m_vecArrows)
        Safe_Release(pArrow);
    m_vecArrows.clear();
    CGameObject::Free();
}