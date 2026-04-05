#include "pch.h"
#include "CJSMonster.h"
#include "CRenderer.h"
#include "CJSScoreMgr.h"

CJSMonster::CJSMonster(LPDIRECT3DDEVICE9 pGraphicDev)
    : CGameObject(pGraphicDev)
{
}

CJSMonster::~CJSMonster()
{
}

HRESULT CJSMonster::Ready_GameObject(_vec3 vStartPos)
{
    if (FAILED(Add_Component()))
        return E_FAIL;

    m_pTransformCom->Set_Pos(vStartPos.x, vStartPos.y, vStartPos.z);

    if (FAILED(Ready_BodyParts()))
        return E_FAIL;

    return S_OK;
}

HRESULT CJSMonster::Add_Component()
{
    CComponent* pComponent = nullptr;

    pComponent = m_pTransformCom = dynamic_cast<CTransform*>(
        CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));

    if (!pComponent) return E_FAIL;

    m_mapComponent[ID_DYNAMIC].insert({ L"Com_Transform", pComponent });

    return S_OK;
}

HRESULT CJSMonster::Ready_BodyParts()
{
    // ¸Ó¸®
    PartDesc headDesc;
    headDesc.pTexProto = L"Proto_JSMonster";
    headDesc.vOffset = { 0.f, 1.125f, 0.f };
    headDesc.fSizeX = 0.75f;
    headDesc.fSizeY = 0.75f;
    headDesc.fSizeZ = 0.75f;
    headDesc.front = { 0.12500f, 0.12500f, 0.25000f, 0.28125f };
    headDesc.back = { 0.37500f, 0.12500f, 0.50000f, 0.28125f };
    headDesc.left = { 0.00000f, 0.12500f, 0.12500f, 0.28125f };
    headDesc.right = { 0.25000f, 0.12500f, 0.37500f, 0.28125f };
    headDesc.top = { 0.12500f, 0.00000f, 0.25000f, 0.12500f };
    headDesc.bottom = { 0.25000f, 0.00000f, 0.37500f, 0.12500f };
    m_pHead = CJSBodyPart::Create(m_pGraphicDev, m_pTransformCom, headDesc);
    if (!m_pHead) return E_FAIL;

    // ¸öÅë
    PartDesc bodyDesc;
    bodyDesc.pTexProto = L"Proto_JSMonster";
    bodyDesc.vOffset = { 0.f, 0.f, 0.f };
    bodyDesc.fSizeX = 0.75f;
    bodyDesc.fSizeY = 1.125f;
    bodyDesc.fSizeZ = 0.45f;
    bodyDesc.front = { 0.34375f, 0.40625f, 0.46875f, 0.59375f };
    bodyDesc.back = { 0.56250f, 0.40625f, 0.68750f, 0.59375f };
    bodyDesc.left = { 0.25000f, 0.40625f, 0.34375f, 0.59375f };
    bodyDesc.right = { 0.46875f, 0.40625f, 0.56250f, 0.59375f };
    bodyDesc.top = { 0.34375f, 0.31250f, 0.46875f, 0.40625f };
    bodyDesc.bottom = { 0.46875f, 0.31250f, 0.59375f, 0.40625f };
    m_pBody = CJSBodyPart::Create(m_pGraphicDev, m_pTransformCom, bodyDesc);
    if (!m_pBody) return E_FAIL;

    // ¿ÞÆÈ
    PartDesc armLDesc;
    armLDesc.pTexProto = L"Proto_JSMonster";
    armLDesc.vOffset = { -0.6f, 0.f, 0.f };
    armLDesc.fSizeX = 0.45f;
    armLDesc.fSizeY = 1.125f;
    armLDesc.fSizeZ = 0.45f;
    armLDesc.front = { 0.06250f, 0.40625f, 0.12500f, 0.59375f };
    armLDesc.back = { 0.18750f, 0.40625f, 0.25000f, 0.59375f };
    armLDesc.left = { 0.00000f, 0.40625f, 0.06250f, 0.59375f };
    armLDesc.right = { 0.12500f, 0.40625f, 0.18750f, 0.59375f };
    armLDesc.top = { 0.06250f, 0.34375f, 0.12500f, 0.40625f };
    armLDesc.bottom = { 0.12500f, 0.34375f, 0.18750f, 0.40625f };
    m_pArmL = CJSBodyPart::Create(m_pGraphicDev, m_pTransformCom, armLDesc);
    if (!m_pArmL) return E_FAIL;

    // ¿À¸¥ÆÈ
    PartDesc armRDesc = armLDesc;
    armRDesc.vOffset = { 0.6f, 0.f, 0.f };
    m_pArmR = CJSBodyPart::Create(m_pGraphicDev, m_pTransformCom, armRDesc);
    if (!m_pArmR) return E_FAIL;

    // ¿Þ´Ù¸®
    PartDesc legLDesc;
    legLDesc.pTexProto = L"Proto_JSMonster";
    legLDesc.vOffset = { -0.225f, -1.125f, 0.f };
    legLDesc.fSizeX = 0.45f;
    legLDesc.fSizeY = 1.125f;
    legLDesc.fSizeZ = 0.45f;
    legLDesc.front = { 0.75000f, 0.65625f, 0.81250f, 0.84375f };
    legLDesc.back = { 0.87500f, 0.65625f, 0.93750f, 0.84375f };
    legLDesc.left = { 0.68750f, 0.65625f, 0.75000f, 0.84375f };
    legLDesc.right = { 0.81250f, 0.65625f, 0.87500f, 0.84375f };
    legLDesc.top = { 0.75000f, 0.59375f, 0.81250f, 0.65625f };
    legLDesc.bottom = { 0.81250f, 0.59375f, 0.87500f, 0.65625f };
    m_pLegL = CJSBodyPart::Create(m_pGraphicDev, m_pTransformCom, legLDesc);
    if (!m_pLegL) return E_FAIL;

    // ¿À¸¥´Ù¸®
    PartDesc legRDesc = legLDesc;
    legRDesc.vOffset = { 0.225f, -1.125f, 0.f };
    m_pLegR = CJSBodyPart::Create(m_pGraphicDev, m_pTransformCom, legRDesc);
    if (!m_pLegR) return E_FAIL;

    return S_OK;
}

_int CJSMonster::Update_GameObject(const _float& fTimeDelta)
{
    _int iExit = CGameObject::Update_GameObject(fTimeDelta);

    JSGAMESTAGE eStage = CJSScoreMgr::GetInstance()->Get_Stage();

    if (eStage == JSSTAGE_COUNTDOWN || eStage == JSSTAGE_PLAY)
    {
        Update_BodyParts(fTimeDelta);
        return iExit;
    }

    _vec3 vLook;
    m_pTransformCom->Get_Info(INFO_LOOK, &vLook);
    m_pTransformCom->Move_Pos(&vLook, m_fSpeed, fTimeDelta);

    Update_RunAnimation(fTimeDelta);
    Update_BodyParts(fTimeDelta);

    return iExit;
}

void CJSMonster::Update_RunAnimation(const _float& fTimeDelta)
{
    m_fAnimTime += fTimeDelta * m_fAnimSpeed;
    _float fSwing = sinf(m_fAnimTime) * 60.f;

    if (m_pArmL) m_pArmL->Get_Transform()->Set_Rotation(ROT_X, 90.f);
    if (m_pArmR) m_pArmR->Get_Transform()->Set_Rotation(ROT_X, 90.f);
    if (m_pLegL) m_pLegL->Get_Transform()->Set_Rotation(ROT_X, -fSwing);
    if (m_pLegR) m_pLegR->Get_Transform()->Set_Rotation(ROT_X, fSwing);
}

void CJSMonster::Update_BodyParts(const _float& fTimeDelta)
{
    if (m_pHead) m_pHead->Update_GameObject(fTimeDelta);
    if (m_pBody) m_pBody->Update_GameObject(fTimeDelta);
    if (m_pArmL) m_pArmL->Update_GameObject(fTimeDelta);
    if (m_pArmR) m_pArmR->Update_GameObject(fTimeDelta);
    if (m_pLegL) m_pLegL->Update_GameObject(fTimeDelta);
    if (m_pLegR) m_pLegR->Update_GameObject(fTimeDelta);
}

void CJSMonster::LateUpdate_BodyParts(const _float& fTimeDelta)
{
    if (m_pHead) m_pHead->LateUpdate_GameObject(fTimeDelta);
    if (m_pBody) m_pBody->LateUpdate_GameObject(fTimeDelta);
    if (m_pArmL) m_pArmL->LateUpdate_GameObject(fTimeDelta);
    if (m_pArmR) m_pArmR->LateUpdate_GameObject(fTimeDelta);
    if (m_pLegL) m_pLegL->LateUpdate_GameObject(fTimeDelta);
    if (m_pLegR) m_pLegR->LateUpdate_GameObject(fTimeDelta);
}

void CJSMonster::LateUpdate_GameObject(const _float& fTimeDelta)
{
    CGameObject::LateUpdate_GameObject(fTimeDelta);
    LateUpdate_BodyParts(fTimeDelta);
    CRenderer::GetInstance()->Add_RenderGroup(RENDER_NONALPHA, this);
}

void CJSMonster::Render_GameObject()
{
    
}

CJSMonster* CJSMonster::Create(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vStartPos)
{
    CJSMonster* pMonster = new CJSMonster(pGraphicDev);
    if (FAILED(pMonster->Ready_GameObject(vStartPos)))
    {
        Safe_Release(pMonster);
        MSG_BOX("JSMonster Create Failed");
        return nullptr;
    }
    return pMonster;
}

void CJSMonster::Free()
{
    Safe_Release(m_pHead);
    Safe_Release(m_pBody);
    Safe_Release(m_pArmL);
    Safe_Release(m_pArmR);
    Safe_Release(m_pLegL);
    Safe_Release(m_pLegR);

    CGameObject::Free();
}
