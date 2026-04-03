#include "pch.h"
#include "CJSPlayer.h"
#include "CRenderer.h"
#include "CDInputMgr.h"
#include "CJSChunkMgr.h"
#include "CJSScoreMgr.h"
#include "CSoundMgr.h"

CJSPlayer::CJSPlayer(LPDIRECT3DDEVICE9 pGraphicDev)
	: CGameObject(pGraphicDev)
	, m_pBufferCom(nullptr)
	, m_pTransformCom(nullptr)
	, m_pTextureCom(nullptr)
	, m_pColliderCom(nullptr)
{
}

CJSPlayer::~CJSPlayer()
{
}

HRESULT CJSPlayer::Ready_GameObject()
{
	if (FAILED(Add_Component()))
		return E_FAIL;

	m_pTransformCom->Set_Pos(0.f, 2.f, 0.f);

	return S_OK;
}

_int CJSPlayer::Update_GameObject(const _float& fTimeDelta)
{
	_int iExit = CGameObject::Update_GameObject(fTimeDelta);

	Falling();
	Jump(fTimeDelta);
	Key_Input(fTimeDelta);
	Advance(fTimeDelta);
	Check_Collect();

	return iExit;
}

void CJSPlayer::LateUpdate_GameObject(const _float& fTimeDelta)
{
	CGameObject::LateUpdate_GameObject(fTimeDelta);

	m_pColliderCom->Update_Collider(m_pTransformCom->Get_World());
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

	m_pColliderCom->Render_Collider();
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

	// Collider
	m_pColliderCom = CJSCollider::Create(m_pGraphicDev, { 0.f, 0.5f, 0.f }, { 1.f, 2.f, 1.f });

	if (!m_pColliderCom)
		return E_FAIL;

	m_mapComponent[ID_DYNAMIC].insert({ L"Com_Collider", m_pColliderCom });
	
	return S_OK;
}

void CJSPlayer::Advance(const _float& fTimeDelta)
{
	_vec3 vLook;
	m_pTransformCom->Get_Info(INFO_LOOK, &vLook);
	m_pTransformCom->Move_Pos(&vLook, m_fSpeed, fTimeDelta);

	m_fTotalDistance += m_fSpeed * fTimeDelta;
	CJSScoreMgr::GetInstance()->Set_Distance(m_fTotalDistance);

	if (m_fTotalDistance >= m_fNextSpeedUpZ && m_fSpeed < m_fMaxSpeed)
	{
		m_fSpeed += m_fSpeedUpAmount;
		m_fNextSpeedUpZ += m_fSpeedUpInterval;

		CJSScoreMgr::GetInstance()->Set_Speed(m_fSpeed);
	}
}

void CJSPlayer::Jump(const _float& fTimeDelta)
{
	if (m_bJump)
	{
		_vec3 vMyPos;
		m_pTransformCom->Get_Info(INFO_POS, &vMyPos);

		m_fVelocityY -= m_fGravity * fTimeDelta;

		vMyPos.y += m_fVelocityY;

		_vec3 vUp = { 0.f, 1.f, 0.f };
		m_pTransformCom->Set_Pos(vMyPos.x, vMyPos.y, vMyPos.z);

		if (m_bFalling)
			return;

		if (vMyPos.y <= m_fGroundY)
		{
			vMyPos.y = m_fGroundY;
			m_fVelocityY = 0.f;
			m_bJump = false;
			m_pTransformCom->Set_Pos(vMyPos.x, vMyPos.y, vMyPos.z);
		}
	}
}

void CJSPlayer::Falling()
{
	if (m_bFalling)
		return;

	_vec3 vPos;
	m_pTransformCom->Get_Info(INFO_POS, &vPos);

	CHUNKTYPE eChunkType = CJSChunkMgr::GetInstance()->Get_CurrentChunkType(vPos);

	if (eChunkType == CHUNK_CORNER_LEFT || eChunkType == CHUNK_CORNER_RIGHT)
		return;

	TILEID eTileID = CJSChunkMgr::GetInstance()->Get_TileID(vPos);

	if (eTileID == TILE_EMPTY && !m_bJump)
	{
		m_bFalling = true;
		m_bJump = true;
		m_fVelocityY = 0.f;

		CSoundMgr::GetInstance()->PlayEffect(L"Player/Scream.wav", 1.f);
	}
}

void CJSPlayer::Check_Collect()
{
	_vec3 vPos;

	m_pTransformCom->Get_Info(INFO_POS, &vPos);
	
	CJSChunkMgr::GetInstance()->Check_Collect(vPos);
}

void CJSPlayer::Key_Input(const _float& fTimeDelta)
{
	_vec3 vPos;
	m_pTransformCom->Get_Info(INFO_POS, &vPos);
	CHUNKTYPE eType = CJSChunkMgr::GetInstance()->Get_CurrentChunkType(vPos);

	if (eType == CHUNK_CORNER_LEFT || eType == CHUNK_CORNER_RIGHT)
	{
		if (CDInputMgr::GetInstance()->Get_DIKeyState(DIK_A) && !m_bRotated)
		{
			m_pTransformCom->Rotation(ROT_Y, -90.f);
			m_bRotated = true;
		}
		if (CDInputMgr::GetInstance()->Get_DIKeyState(DIK_D) && !m_bRotated)
		{
			m_pTransformCom->Rotation(ROT_Y, 90.f);
			m_bRotated = true;
		}
	}
	else
	{
		m_bRotated = false;
	}

	if (CDInputMgr::GetInstance()->Get_DIKeyState(DIK_LEFT))
	{
		_vec3 vRight;
		m_pTransformCom->Get_Info(INFO_RIGHT, &vRight);

		_vec3 vPos;
		m_pTransformCom->Get_Info(INFO_POS, &vPos);
		vPos += -vRight * m_fSideSpeed * fTimeDelta;

		_matrix matTemp;
		D3DXMatrixTranslation(&matTemp, vPos.x, vPos.y, vPos.z);
		m_pColliderCom->Update_Collider(&matTemp);

		if (!CJSChunkMgr::GetInstance()->Check_WallCollision(m_pColliderCom))
			m_pTransformCom->Move_Pos(&vRight, -m_fSideSpeed, fTimeDelta);
		else
		{
			m_pTransformCom->Get_Info(INFO_POS, &vPos);
			D3DXMatrixTranslation(&matTemp, vPos.x, vPos.y, vPos.z);
			m_pColliderCom->Update_Collider(&matTemp);
		}
	}
	else if (CDInputMgr::GetInstance()->Get_DIKeyState(DIK_RIGHT))
	{
		_vec3 vRight;
		m_pTransformCom->Get_Info(INFO_RIGHT, &vRight);

		_vec3 vPos;
		m_pTransformCom->Get_Info(INFO_POS, &vPos);
		vPos += vRight * m_fSideSpeed * fTimeDelta;

		_matrix matTemp;
		D3DXMatrixTranslation(&matTemp, vPos.x, vPos.y, vPos.z);
		m_pColliderCom->Update_Collider(&matTemp);

		if (!CJSChunkMgr::GetInstance()->Check_WallCollision(m_pColliderCom))
			m_pTransformCom->Move_Pos(&vRight, m_fSideSpeed, fTimeDelta);
		else
		{
			m_pTransformCom->Get_Info(INFO_POS, &vPos);
			D3DXMatrixTranslation(&matTemp, vPos.x, vPos.y, vPos.z);
			m_pColliderCom->Update_Collider(&matTemp);
		}
	}

	if (CDInputMgr::GetInstance()->Get_DIKeyState(DIK_UP) && !m_bJump && !m_bFalling)
	{
		m_fVelocityY = m_fJumpPower;
		m_bJump = true;

		CSoundMgr::GetInstance()->PlayEffect(L"Player/Grunt-Jump.wav", 0.8f);
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