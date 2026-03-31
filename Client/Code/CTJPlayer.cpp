#include "pch.h"
#include "CTJPlayer.h"
#include "CTNT.h"

CTJPlayer::CTJPlayer(LPDIRECT3DDEVICE9 pGraphicDev)
    : CPlayer(pGraphicDev)
{
}

CTJPlayer::~CTJPlayer()
{
}

HRESULT CTJPlayer::Ready_GameObject()
{
    if (FAILED(CPlayer::Ready_GameObject()))
        return E_FAIL;

    return S_OK;
}

_int CTJPlayer::Update_GameObject(const _float& fTimeDelta)
{
    _int iExit = CPlayer::Update_GameObject(fTimeDelta);

    // TNT 오라
    if (m_bTNTAura)
    {
        m_fTNTTimer += fTimeDelta;
        if (m_fTNTTimer >= m_fTNTInterval)
        {
            m_fTNTTimer = 0.f;

            _vec3 vPos;
            Get_Transform()->Get_Info(INFO_POS, &vPos);
            vPos.y += 1.5f;

            // 랜덤 방향
            float fAngle = ((float)(rand() % 360)) * D3DX_PI / 180.f;
            _vec3 vDir = { cosf(fAngle), 0.f, sinf(fAngle) };
            D3DXVec3Normalize(&vDir, &vDir);

            CTNT* pTNT = CTNT::Create(m_pGraphicDev, vPos);
            if (pTNT)
            {
                pTNT->Throw(vPos, vDir, 8.f);
                m_vecTNTAura.push_back(pTNT);
            }
        }
    }

    // TNT 업데이트 및 삭제
    for (auto& pTNT : m_vecTNTAura)
        pTNT->Update_GameObject(fTimeDelta);

    m_vecTNTAura.erase(
        remove_if(m_vecTNTAura.begin(), m_vecTNTAura.end(),
            [](CTNT* p) {
                if (p->Is_Dead()) { Safe_Release(p); return true; }
                return false;
            }),
        m_vecTNTAura.end());

    return iExit;
}

void CTJPlayer::LateUpdate_GameObject(const _float& fTimeDelta)
{
    CPlayer::LateUpdate_GameObject(fTimeDelta);

    for (auto& pTNT : m_vecTNTAura)
        pTNT->LateUpdate_GameObject(fTimeDelta);
}

void CTJPlayer::Render_GameObject()
{
    CPlayer::Render_GameObject();

    for (auto& pTNT : m_vecTNTAura)
        pTNT->Render_GameObject();
}

void CTJPlayer::Add_Exp(int iExp)
{
    m_iExp += iExp;
    if (m_iExp >= m_iMaxExp)
    {
        m_iExp -= m_iMaxExp;
        m_iLevel++;
        m_iMaxExp = (int)(m_iMaxExp * 1.5f);
        m_bLevelUp = true;
    }
}

void CTJPlayer::Apply_Ability(int iAbility)
{
    switch (iAbility)
    {
    case 0: m_bTNTAura = true;  break;
    case 1: m_iArrowCount++;    break;
    case 2: m_bPet = true;      break;
    }
}

CTJPlayer* CTJPlayer::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
    CTJPlayer* pPlayer = new CTJPlayer(pGraphicDev);

    if (FAILED(pPlayer->Ready_GameObject()))
    {
        Safe_Release(pPlayer);
        MSG_BOX("CTJPlayer Create Failed");
        return nullptr;
    }

    return pPlayer;
}

void CTJPlayer::Free()
{
    for (auto& pTNT : m_vecTNTAura)
        Safe_Release(pTNT);
    m_vecTNTAura.clear();

    CGameObject::Free();
}