#include "CJSCollider.h"

CJSCollider::CJSCollider(LPDIRECT3DDEVICE9 pGraphicDev)
    : CComponent(pGraphicDev)
{
}

CJSCollider::CJSCollider(const CJSCollider& rhs)
    : CComponent(rhs)
    , m_vLocalMin(rhs.m_vLocalMin)
    , m_vLocalMax(rhs.m_vLocalMax)
    , m_vMin(rhs.m_vMin)
    , m_vMax(rhs.m_vMax)
{
}

CJSCollider::~CJSCollider()
{
}

HRESULT CJSCollider::Ready_Collider(_vec3 vCenter, _vec3 vSize)
{
    // 로컬 공간 min, max 계산
    m_vLocalMin = { vCenter.x - vSize.x * 0.5f,
                    vCenter.y - vSize.y * 0.5f,
                    vCenter.z - vSize.z * 0.5f };

    m_vLocalMax = { vCenter.x + vSize.x * 0.5f,
                    vCenter.y + vSize.y * 0.5f,
                    vCenter.z + vSize.z * 0.5f };

    m_vMin = m_vLocalMin;
    m_vMax = m_vLocalMax;

    return S_OK;
}

void CJSCollider::Update_Collider(_matrix* pWorldMatrix)
{
    // 8개 꼭짓점 변환 후 min, max 갱신
    _vec3 vCorners[8] =
    {
        { m_vLocalMin.x, m_vLocalMin.y, m_vLocalMin.z },
        { m_vLocalMax.x, m_vLocalMin.y, m_vLocalMin.z },
        { m_vLocalMin.x, m_vLocalMax.y, m_vLocalMin.z },
        { m_vLocalMax.x, m_vLocalMax.y, m_vLocalMin.z },
        { m_vLocalMin.x, m_vLocalMin.y, m_vLocalMax.z },
        { m_vLocalMax.x, m_vLocalMin.y, m_vLocalMax.z },
        { m_vLocalMin.x, m_vLocalMax.y, m_vLocalMax.z },
        { m_vLocalMax.x, m_vLocalMax.y, m_vLocalMax.z },
    };

    m_vMin = { FLT_MAX,  FLT_MAX,  FLT_MAX };
    m_vMax = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

    for (_int i = 0; i < 8; ++i)
    {
        _vec3 vWorld;
        D3DXVec3TransformCoord(&vWorld, &vCorners[i], pWorldMatrix);

        m_vMin.x = min(m_vMin.x, vWorld.x);
        m_vMin.y = min(m_vMin.y, vWorld.y);
        m_vMin.z = min(m_vMin.z, vWorld.z);

        m_vMax.x = max(m_vMax.x, vWorld.x);
        m_vMax.y = max(m_vMax.y, vWorld.y);
        m_vMax.z = max(m_vMax.z, vWorld.z);
    }
}

_bool CJSCollider::Check_Collision(CJSCollider* pOther)
{
    if (m_vMax.x < pOther->m_vMin.x || m_vMin.x > pOther->m_vMax.x) return false;
    if (m_vMax.y < pOther->m_vMin.y || m_vMin.y > pOther->m_vMax.y) return false;
    if (m_vMax.z < pOther->m_vMin.z || m_vMin.z > pOther->m_vMax.z) return false;
    return true;
}

void CJSCollider::Render_Collider()
{
    // 디버그용 와이어프레임 박스
    _vec3 vLines[24] =
    {
        // 아래 사각형
        { m_vMin.x, m_vMin.y, m_vMin.z }, { m_vMax.x, m_vMin.y, m_vMin.z },
        { m_vMax.x, m_vMin.y, m_vMin.z }, { m_vMax.x, m_vMin.y, m_vMax.z },
        { m_vMax.x, m_vMin.y, m_vMax.z }, { m_vMin.x, m_vMin.y, m_vMax.z },
        { m_vMin.x, m_vMin.y, m_vMax.z }, { m_vMin.x, m_vMin.y, m_vMin.z },
        // 위 사각형
        { m_vMin.x, m_vMax.y, m_vMin.z }, { m_vMax.x, m_vMax.y, m_vMin.z },
        { m_vMax.x, m_vMax.y, m_vMin.z }, { m_vMax.x, m_vMax.y, m_vMax.z },
        { m_vMax.x, m_vMax.y, m_vMax.z }, { m_vMin.x, m_vMax.y, m_vMax.z },
        { m_vMin.x, m_vMax.y, m_vMax.z }, { m_vMin.x, m_vMax.y, m_vMin.z },
        // 기둥 4개
        { m_vMin.x, m_vMin.y, m_vMin.z }, { m_vMin.x, m_vMax.y, m_vMin.z },
        { m_vMax.x, m_vMin.y, m_vMin.z }, { m_vMax.x, m_vMax.y, m_vMin.z },
        { m_vMax.x, m_vMin.y, m_vMax.z }, { m_vMax.x, m_vMax.y, m_vMax.z },
        { m_vMin.x, m_vMin.y, m_vMax.z }, { m_vMin.x, m_vMax.y, m_vMax.z },
    };

    m_pGraphicDev->SetRenderState(D3DRS_LIGHTING, FALSE);
    m_pGraphicDev->SetFVF(D3DFVF_XYZ);

    _matrix matIdentity;
    D3DXMatrixIdentity(&matIdentity);
    m_pGraphicDev->SetTransform(D3DTS_WORLD, &matIdentity);

    m_pGraphicDev->DrawPrimitiveUP(D3DPT_LINELIST, 12, vLines, sizeof(_vec3));

    m_pGraphicDev->SetRenderState(D3DRS_LIGHTING, TRUE);
}

CJSCollider* CJSCollider::Create(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vCenter, _vec3 vSize)
{
    CJSCollider* pCollider = new CJSCollider(pGraphicDev);
    if (FAILED(pCollider->Ready_Collider(vCenter, vSize)))
    {
        Safe_Release(pCollider);
        return nullptr;
    }
    return pCollider;
}

CComponent* CJSCollider::Clone()
{
    return new CJSCollider(*this);
}

void CJSCollider::Free()
{
    CComponent::Free();
}