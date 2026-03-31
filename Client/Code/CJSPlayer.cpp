#include "pch.h"
#include "CJSPlayer.h"
#include "CRenderer.h"
#include "CDInputMgr.h"

CJSPlayer::CJSPlayer(LPDIRECT3DDEVICE9 pGraphicDev)
	: CGameObject(pGraphicDev)
	, m_pBufferCom(nullptr)
	, m_pTransformCom(nullptr)
	, m_pTextureCom(nullptr)
{
}

CJSPlayer::~CJSPlayer()
{
}

HRESULT CJSPlayer::Ready_GameObject()
{
	if (FAILED(Add_Component()))
		return E_FAIL;

	m_pTransformCom->Set_Pos(0.f, 1.f, 0.f);

	return S_OK;
}

_int CJSPlayer::Update_GameObject(const _float& fTimeDelta)
{
	_int iExit = CGameObject::Update_GameObject(fTimeDelta);

	Key_Input(fTimeDelta);
	Jump(fTimeDelta);
	Advance(fTimeDelta);

	return iExit;
}

void CJSPlayer::LateUpdate_GameObject(const _float& fTimeDelta)
{
	CGameObject::LateUpdate_GameObject(fTimeDelta);

	CRenderer::GetInstance()->Add_RenderGroup(RENDER_NONALPHA, this);
}

void CJSPlayer::Render_GameObject()
{
	m_pGraphicDev->SetTransform(D3DTS_WORLD, m_pTransformCom->Get_World());

	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_pGraphicDev->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);  // 와이어프레임 ON

	m_pBufferCom->Render_Buffer();

	m_pGraphicDev->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);      // 솔리드로 복구
	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}

HRESULT CJSPlayer::Add_Component()
{
	CComponent* pComponent = nullptr;

	// Buffer
	pComponent = m_pBufferCom = dynamic_cast<CJSCubeTex*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_JSCubeTex"));

	if (pComponent == nullptr)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_Buffer", pComponent });

	// Transform
	pComponent = m_pTransformCom = dynamic_cast<CTransform*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));

	if (pComponent == nullptr)
		return E_FAIL;

	m_mapComponent[ID_DYNAMIC].insert({ L"Com_Transform", pComponent });

	// Texture
	

	return S_OK;
}

void CJSPlayer::Advance(const _float& fTimeDelta)
{
	_vec3 vLook;
	m_pTransformCom->Get_Info(INFO_LOOK, &vLook);

	m_pTransformCom->Move_Pos(&vLook, m_fSpeed, fTimeDelta);

	m_fSpeed += fTimeDelta;
}

void CJSPlayer::Jump(const _float& fTimeDelta)
{
	if (m_bJump)
	{
		m_fVelocityY -= m_fGravity * fTimeDelta;

		_vec3 vPos;
		m_pTransformCom->Get_Info(INFO_POS, &vPos);
		vPos.y += m_fVelocityY * fTimeDelta;

		if (vPos.y <= m_fGroundY)
		{
			vPos.y = m_fGroundY;
			m_fVelocityY = 0.f;
			m_bJump = false;
		}

		_vec3 vUp = { 0.f, 1.f, 0.f };
		m_pTransformCom->Move_Pos(&vUp, m_fVelocityY, fTimeDelta);
	}
}

void CJSPlayer::Key_Input(const _float& fTimeDelta)
{
	if (CDInputMgr::GetInstance()->Get_DIKeyState(DIK_LEFT))
	{
		_vec3 vRight;
		m_pTransformCom->Get_Info(INFO_RIGHT, &vRight);

		m_pTransformCom->Move_Pos(&vRight, -m_fSideSpeed, fTimeDelta);
	}
	else if (CDInputMgr::GetInstance()->Get_DIKeyState(DIK_RIGHT))
	{
		_vec3 vRight;
		m_pTransformCom->Get_Info(INFO_RIGHT, &vRight);

		m_pTransformCom->Move_Pos(&vRight, m_fSideSpeed, fTimeDelta);
	}

	if (CDInputMgr::GetInstance()->Get_DIKeyState(DIK_UP) && !m_bJump)
	{
		m_fVelocityY = m_fJumpPower;
		m_bJump = true;
	}
}

CJSPlayer* CJSPlayer::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
	CJSPlayer* pJSPlayer = new CJSPlayer(pGraphicDev);

	if (FAILED(pJSPlayer->Ready_GameObject()))
	{
		Safe_Release(pJSPlayer);
		MSG_BOX("JSPlayer Create Failed");
		return nullptr;
	}

	return pJSPlayer;
}

void CJSPlayer::Free()
{
	CGameObject::Free();
}