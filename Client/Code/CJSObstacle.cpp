#include "pch.h"
#include "CJSObstacle.h"
#include "CRenderer.h"

CJSObstacle::CJSObstacle(LPDIRECT3DDEVICE9 pGraphicDev)
    : CGameObject(pGraphicDev)
    , m_pBufferCom(nullptr)
    , m_pTransformCom(nullptr)
    , m_pTextureCom(nullptr)
    , m_pColliderCom(nullptr)
{
}

CJSObstacle::~CJSObstacle()
{
}

HRESULT CJSObstacle::Ready_GameObject(_vec3 vPos)
{
    if (FAILED(Add_Component()))
        return E_FAIL;

    m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
    m_pTransformCom->Update_Component(0.f);

    m_pColliderCom = CJSCollider::Create(m_pGraphicDev, { 0.f, 0.f, 0.f }, { 1.5f, 2.f, 1.5f });

    if (!m_pColliderCom)
        return E_FAIL;

    return S_OK;
}

_int CJSObstacle::Update_GameObject(const _float& fTimeDelta)
{
    _int iExit = CGameObject::Update_GameObject(fTimeDelta);

    m_fFrameTime += fTimeDelta;

    if (m_fFrameTime >= m_fFrameSpeed)
    {
        m_fFrameTime = 0.f;
        ++m_iCurFrame;

        if (m_iCurFrame >= m_iTotalFrame)
            m_iCurFrame = 0;
    }

    _matrix matWorld = *m_pTransformCom->Get_World();
    m_pColliderCom->Update_Collider(&matWorld);

    return iExit;
}

void CJSObstacle::LateUpdate_GameObject(const _float& fTimeDelta)
{
    CGameObject::LateUpdate_GameObject(fTimeDelta);

    if (m_bDead)
        return;

    CRenderer::GetInstance()->Add_RenderGroup(RENDER_ALPHA, this);
}

void CJSObstacle::Render_GameObject()
{
    _matrix matWorld, matView;
    m_pGraphicDev->GetTransform(D3DTS_VIEW, &matView);
    D3DXMatrixInverse(&matWorld, 0, &matView);

    _vec3 vPos;
    m_pTransformCom->Get_Info(INFO_POS, &vPos);
    matWorld._41 = vPos.x;
    matWorld._42 = vPos.y;
    matWorld._43 = vPos.z;

    m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);

    // UV 변환 행렬로 프레임 처리
    _float fFrameSize = 1.f / (_float)m_iTotalFrame;
    _matrix matTex;
    D3DXMatrixIdentity(&matTex);
    matTex._22 = fFrameSize;
    matTex._32 = fFrameSize * (_float)m_iCurFrame;

    m_pGraphicDev->SetTransform(D3DTS_TEXTURE0, &matTex);
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);

    m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    m_pTextureCom->Set_Texture(0);
    m_pBufferCom->Render_Buffer();
    m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

    // 복구
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
}

HRESULT CJSObstacle::Add_Component()
{
    CComponent* pComponent = nullptr;

    // Buffer
    pComponent = m_pBufferCom = dynamic_cast<CJSSpriteBuffer*>(
        CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_JSSpriteBuffer"));

    if (!pComponent)
        return E_FAIL;

    m_mapComponent[ID_DYNAMIC].insert({ L"Com_Buffer", pComponent });

    // Transform
    pComponent = m_pTransformCom = dynamic_cast<CTransform*>(
        CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));

    if (!pComponent)
        return E_FAIL;

    m_mapComponent[ID_DYNAMIC].insert({ L"Com_Transform", pComponent });

    // Texture
    pComponent = m_pTextureCom = dynamic_cast<CTexture*>(
        CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_FireTexture"));

    if (!pComponent)
        return E_FAIL;

    m_mapComponent[ID_STATIC].insert({ L"Com_Texture", pComponent });

    return S_OK;
}

CJSObstacle* CJSObstacle::Create(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vPos)
{
    CJSObstacle* pObstacle = new CJSObstacle(pGraphicDev);

    if (FAILED(pObstacle->Ready_GameObject(vPos)))
    {
        Safe_Release(pObstacle);
        MSG_BOX("JSObstacle Create Failed");
        return nullptr;
    }
    return pObstacle;
}

void CJSObstacle::Free()
{
    Safe_Release(m_pColliderCom);
    CGameObject::Free();
}