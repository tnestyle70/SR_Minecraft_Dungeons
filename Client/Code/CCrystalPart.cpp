#include "pch.h"
#include "CCrystalPart.h"

CCrystalPart::CCrystalPart(LPDIRECT3DDEVICE9 pGraphicDev, CRYSTAL_PART ePart)
	: CGameObject(pGraphicDev)
	, m_ePart(ePart)
	, m_pBufferCom(nullptr)
	, m_pTransformCom(nullptr)
{
}

CCrystalPart::~CCrystalPart()
{
}

HRESULT CCrystalPart::Ready_GameObject()
{
	if (FAILED(Add_Component()))
		return E_FAIL;

	return S_OK;
}

_int CCrystalPart::Update_GameObject(const _float& fTimeDelta)
{
	_int iExit = CGameObject::Update_GameObject(fTimeDelta);

	return iExit;
}

void CCrystalPart::LateUpdate_GameObject(const _float& fTimeDelta)
{
	CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CCrystalPart::Render_GameObject()
{
	m_pGraphicDev->SetTransform(D3DTS_WORLD, m_pTransformCom->Get_World());

	m_pBufferCom->Render_Buffer();
}

HRESULT CCrystalPart::Add_Component()
{
	CComponent* pComponent = nullptr;
	
	CUBE c{};

	switch (m_ePart)
	{
	case CRYSTAL_8x20:
		c.fWidth = 0.40000f; c.fHeight = 1.00000f; c.fDepth = 0.40000f;
		c.front = { 0.06250f, 0.06250f, 0.12500f, 0.21875f };
		c.back = { 0.18750f, 0.06250f, 0.25000f, 0.21875f };
		c.left = { 0.00000f, 0.06250f, 0.06250f, 0.21875f };
		c.right = { 0.12500f, 0.06250f, 0.18750f, 0.21875f };
		c.top = { 0.06250f, 0.00000f, 0.12500f, 0.06250f };
		c.bottom = { 0.12500f, 0.00000f, 0.18750f, 0.06250f };
		break;

	case CRYSTAL_5x14:
		c.fWidth = 0.35714f; c.fHeight = 1.00000f; c.fDepth = 0.35714f;
		c.front = { 0.28906f, 0.10938f, 0.32813f, 0.21875f };
		c.back = { 0.36719f, 0.10938f, 0.40625f, 0.21875f };
		c.left = { 0.25000f, 0.10938f, 0.28906f, 0.21875f };
		c.right = { 0.32813f, 0.10938f, 0.36719f, 0.21875f };
		c.top = { 0.28906f, 0.07031f, 0.32813f, 0.10938f };
		c.bottom = { 0.32813f, 0.07031f, 0.36719f, 0.10938f };
		break;

	case CRYSTAL_4x11:
		c.fWidth = 0.36364f; c.fHeight = 1.00000f; c.fDepth = 0.36364f;
		c.front = { 0.43750f, 0.13281f, 0.46875f, 0.21875f };
		c.back = { 0.50000f, 0.13281f, 0.53125f, 0.21875f };
		c.left = { 0.40625f, 0.13281f, 0.43750f, 0.21875f };
		c.right = { 0.46875f, 0.13281f, 0.50000f, 0.21875f };
		c.top = { 0.43750f, 0.10156f, 0.46875f, 0.13281f };
		c.bottom = { 0.46875f, 0.10156f, 0.50000f, 0.13281f };
		break;

	case CRYSTAL_3x15:
		c.fWidth = 0.20000f; c.fHeight = 1.00000f; c.fDepth = 0.20000f;
		c.front = { 0.55469f, 0.10156f, 0.57813f, 0.21875f };
		c.back = { 0.60156f, 0.10156f, 0.62500f, 0.21875f };
		c.left = { 0.53125f, 0.10156f, 0.55469f, 0.21875f };
		c.right = { 0.57813f, 0.10156f, 0.60156f, 0.21875f };
		c.top = { 0.55469f, 0.07813f, 0.57813f, 0.10156f };
		c.bottom = { 0.57813f, 0.07813f, 0.60156f, 0.10156f };
		break;

	case CRYSTAL_2x13:
		c.fWidth = 0.15385f; c.fHeight = 1.00000f; c.fDepth = 0.15385f;
		c.front = { 0.14063f, 0.23438f, 0.15625f, 0.33594f };
		c.back = { 0.17188f, 0.23438f, 0.18750f, 0.33594f };
		c.left = { 0.12500f, 0.23438f, 0.14063f, 0.33594f };
		c.right = { 0.15625f, 0.23438f, 0.17188f, 0.33594f };
		c.top = { 0.14063f, 0.21875f, 0.15625f, 0.23438f };
		c.bottom = { 0.15625f, 0.21875f, 0.17188f, 0.23438f };
		break;

	case CRYSTAL_3x5:
		c.fWidth = 0.60000f; c.fHeight = 1.00000f; c.fDepth = 0.60000f;
		c.front = { 0.21094f, 0.29688f, 0.23438f, 0.33594f };
		c.back = { 0.25781f, 0.29688f, 0.28125f, 0.33594f };
		c.left = { 0.18750f, 0.29688f, 0.21094f, 0.33594f };
		c.right = { 0.23438f, 0.29688f, 0.25781f, 0.33594f };
		c.top = { 0.21094f, 0.27344f, 0.23438f, 0.29688f };
		c.bottom = { 0.23438f, 0.27344f, 0.25781f, 0.29688f };
		break;

	case CRYSTAL_END:
		break;

	default:
		break;
	}

	// Buffer

	pComponent = m_pBufferCom = CCubeBodyTex::Create(m_pGraphicDev, c);

	if (!pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_Buffer", pComponent });

	// Transform

	pComponent = m_pTransformCom = dynamic_cast<CTransform*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));

	if (!pComponent)
		return E_FAIL;

	m_mapComponent[ID_DYNAMIC].insert({ L"Com_Transform", pComponent });

	return S_OK;
}

CCrystalPart* CCrystalPart::Create(LPDIRECT3DDEVICE9 pGraphicDev, CRYSTAL_PART ePart)
{
	CCrystalPart* pCrystalPart = new CCrystalPart(pGraphicDev, ePart);

	if (FAILED(pCrystalPart->Ready_GameObject()))
	{
		Safe_Release(pCrystalPart);
		MSG_BOX("CrystalPart Create Failed");
		return nullptr;
	}

	return pCrystalPart;
}

void CCrystalPart::Free()
{
	CGameObject::Free();
}