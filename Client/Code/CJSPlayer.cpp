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

	m_pTransformCom->Set_Pos(0.f, 3.f, 0.f);

	if (FAILED(Ready_BodyParts()))
		return E_FAIL;

	return S_OK;
}

_int CJSPlayer::Update_GameObject(const _float& fTimeDelta)
{
	_int iExit = CGameObject::Update_GameObject(fTimeDelta);

	if (CJSScoreMgr::GetInstance()->Is_GameOver())
	{
		if (CJSScoreMgr::GetInstance()->Get_DeathType() == DEATH_FALL)
			Jump(fTimeDelta);
		return iExit;
	}

	Falling();
	Jump(fTimeDelta);
	Key_Input(fTimeDelta);
	Advance(fTimeDelta);
	Check_WallCollision();
	Check_Collect();

	if (!CJSScoreMgr::GetInstance()->Is_GameOver())
	{
		if (m_bSlide)
			Update_SlideAnimation();
		else if (m_bJump && !m_bFalling)
			Update_JumpAnimation(fTimeDelta);
		else if (!m_bJump)
			Update_RunAnimation(fTimeDelta);
	}

	Update_BodyParts(fTimeDelta);

	return iExit;
}

void CJSPlayer::LateUpdate_GameObject(const _float& fTimeDelta)
{
	CGameObject::LateUpdate_GameObject(fTimeDelta);
	LateUpdate_BodyParts(fTimeDelta);

	m_pColliderCom->Update_Collider(m_pTransformCom->Get_World());
	CRenderer::GetInstance()->Add_RenderGroup(RENDER_NONALPHA, this);
}

void CJSPlayer::Render_GameObject()
{
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
	m_pColliderCom = CJSCollider::Create(m_pGraphicDev, { 0.f, 0.f, 0.f }, { 1.f, 3.f, 1.f });

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
		m_fVelocityY -= m_fGravity * fTimeDelta;

		_vec3 vPos;
		m_pTransformCom->Get_Info(INFO_POS, &vPos);
		vPos.y += m_fVelocityY;

		if (m_bFalling)
		{
			if (vPos.y <= -20.f)
				CJSScoreMgr::GetInstance()->Set_GameOver(DEATH_FALL);
			else
				m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
			return;
		}

		if (vPos.y <= m_fGroundY)
		{
			vPos.y = m_fGroundY;
			m_fVelocityY = 0.f;
			m_bJump = false;
		}

		m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
	}
}

void CJSPlayer::Falling()
{
	if (CJSScoreMgr::GetInstance()->Is_GameOver())
		return;

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

		CSoundMgr::GetInstance()->PlayEffect(L"JS/2-16.-Scream.wav", 1.f);
	}
}

void CJSPlayer::Check_Collect()
{
	_vec3 vPos;

	m_pTransformCom->Get_Info(INFO_POS, &vPos);
	
	CJSChunkMgr::GetInstance()->Check_Collect(vPos);
}

void CJSPlayer::Check_WallCollision()
{
	if (CJSScoreMgr::GetInstance()->Is_GameOver())
		return;

	_vec3 vPos, vLook;
	m_pTransformCom->Get_Info(INFO_POS, &vPos);
	m_pTransformCom->Get_Info(INFO_LOOK, &vLook);

	_vec3 vCheckPos = vPos + vLook * 0.5f;
	_matrix matCheck;
	D3DXMatrixTranslation(&matCheck, vCheckPos.x, vCheckPos.y, vCheckPos.z);
	m_pColliderCom->Update_Collider(&matCheck);

	if (CJSChunkMgr::GetInstance()->Check_WallCollision(m_pColliderCom))
		CJSScoreMgr::GetInstance()->Set_GameOver(DEATH_COLLISION);

	if (CJSChunkMgr::GetInstance()->Check_ObstacleCollision(m_pColliderCom))
		CJSScoreMgr::GetInstance()->Set_GameOver(DEATH_COLLISION);
}

HRESULT CJSPlayer::Ready_BodyParts()
{
	// ¸Ó¸®
	PartDesc headDesc;
	headDesc.vOffset = { 0.f, 1.125f, 0.f };   // 0.75 * 1.5
	headDesc.fSizeX = 0.75f;
	headDesc.fSizeY = 0.75f;
	headDesc.fSizeZ = 0.75f;
	headDesc.front = { 0.12500f, 0.12500f, 0.25000f, 0.25000f };
	headDesc.back = { 0.37500f, 0.12500f, 0.50000f, 0.25000f };
	headDesc.left = { 0.00000f, 0.12500f, 0.12500f, 0.25000f };
	headDesc.right = { 0.25000f, 0.12500f, 0.37500f, 0.25000f };
	headDesc.top = { 0.12500f, 0.00000f, 0.25000f, 0.12500f };
	headDesc.bottom = { 0.25000f, 0.00000f, 0.37500f, 0.12500f };
	m_pHead = CJSBodyPart::Create(m_pGraphicDev, m_pTransformCom, headDesc);
	if (!m_pHead) return E_FAIL;

	// ¸öÅë
	PartDesc bodyDesc;
	bodyDesc.vOffset = { 0.f, 0.f, 0.f };
	bodyDesc.fSizeX = 0.75f;
	bodyDesc.fSizeY = 1.125f;
	bodyDesc.fSizeZ = 0.45f;
	bodyDesc.front = { 0.31250f, 0.31250f, 0.43750f, 0.50000f };
	bodyDesc.back = { 0.50000f, 0.31250f, 0.62500f, 0.50000f };
	bodyDesc.left = { 0.25000f, 0.31250f, 0.31250f, 0.50000f };
	bodyDesc.right = { 0.43750f, 0.31250f, 0.50000f, 0.50000f };
	bodyDesc.top = { 0.31250f, 0.25000f, 0.43750f, 0.31250f };
	bodyDesc.bottom = { 0.43750f, 0.25000f, 0.56250f, 0.31250f };
	m_pBody = CJSBodyPart::Create(m_pGraphicDev, m_pTransformCom, bodyDesc);
	if (!m_pBody) return E_FAIL;

	// ¿ÞÆÈ
	PartDesc armLDesc;
	armLDesc.vOffset = { -0.6f, 0.f, 0.f };
	armLDesc.fSizeX = 0.45f;
	armLDesc.fSizeY = 1.125f;
	armLDesc.fSizeZ = 0.45f;
	armLDesc.front = { 0.68750f, 0.31250f, 0.75000f, 0.50000f };
	armLDesc.back = { 0.81250f, 0.31250f, 0.87500f, 0.50000f };
	armLDesc.left = { 0.62500f, 0.31250f, 0.68750f, 0.50000f };
	armLDesc.right = { 0.75000f, 0.31250f, 0.81250f, 0.50000f };
	armLDesc.top = { 0.68750f, 0.25000f, 0.75000f, 0.31250f };
	armLDesc.bottom = { 0.75000f, 0.25000f, 0.81250f, 0.31250f };
	m_pArmL = CJSBodyPart::Create(m_pGraphicDev, m_pTransformCom, armLDesc);
	if (!m_pArmL) return E_FAIL;

	// ¿À¸¥ÆÈ (¿ÞÆÈÀÌ¶û UV µ¿ÀÏ, ¿ÀÇÁ¼Â¸¸ ¹Ý´ë)
	PartDesc armRDesc = armLDesc;
	armRDesc.vOffset = { 0.6f, 0.f, 0.f };
	m_pArmR = CJSBodyPart::Create(m_pGraphicDev, m_pTransformCom, armRDesc);
	if (!m_pArmR) return E_FAIL;

	// ¿Þ´Ù¸®
	PartDesc legLDesc;
	legLDesc.vOffset = { -0.225f, -1.125f, 0.f };
	legLDesc.fSizeX = 0.45f;
	legLDesc.fSizeY = 1.125f;
	legLDesc.fSizeZ = 0.45f;
	legLDesc.front = { 0.06250f, 0.31250f, 0.12500f, 0.50000f };
	legLDesc.back = { 0.18750f, 0.31250f, 0.25000f, 0.50000f };
	legLDesc.left = { 0.00000f, 0.31250f, 0.06250f, 0.50000f };
	legLDesc.right = { 0.12500f, 0.31250f, 0.18750f, 0.50000f };
	legLDesc.top = { 0.06250f, 0.25000f, 0.12500f, 0.31250f };
	legLDesc.bottom = { 0.12500f, 0.25000f, 0.18750f, 0.31250f };
	m_pLegL = CJSBodyPart::Create(m_pGraphicDev, m_pTransformCom, legLDesc);
	if (!m_pLegL) return E_FAIL;

	// ¿À¸¥´Ù¸® (¿Þ´Ù¸®¶û UV µ¿ÀÏ, ¿ÀÇÁ¼Â¸¸ ¹Ý´ë)
	PartDesc legRDesc = legLDesc;
	legRDesc.vOffset = { 0.225f, -1.125f, 0.f };
	m_pLegR = CJSBodyPart::Create(m_pGraphicDev, m_pTransformCom, legRDesc);
	if (!m_pLegR) return E_FAIL;

	return S_OK;
}

void CJSPlayer::Update_BodyParts(const _float& fTimeDelta)
{
	if (m_pHead)  m_pHead->Update_GameObject(fTimeDelta);
	if (m_pBody)  m_pBody->Update_GameObject(fTimeDelta);
	if (m_pArmL)  m_pArmL->Update_GameObject(fTimeDelta);
	if (m_pArmR)  m_pArmR->Update_GameObject(fTimeDelta);
	if (m_pLegL)  m_pLegL->Update_GameObject(fTimeDelta);
	if (m_pLegR)  m_pLegR->Update_GameObject(fTimeDelta);
}

void CJSPlayer::LateUpdate_BodyParts(const _float& fTimeDelta)
{
	if (m_pHead)  m_pHead->LateUpdate_GameObject(fTimeDelta);
	if (m_pBody)  m_pBody->LateUpdate_GameObject(fTimeDelta);
	if (m_pArmL)  m_pArmL->LateUpdate_GameObject(fTimeDelta);
	if (m_pArmR)  m_pArmR->LateUpdate_GameObject(fTimeDelta);
	if (m_pLegL)  m_pLegL->LateUpdate_GameObject(fTimeDelta);
	if (m_pLegR)  m_pLegR->LateUpdate_GameObject(fTimeDelta);
}

void CJSPlayer::Update_RunAnimation(const _float& fTimeDelta)
{
	m_fAnimTime += fTimeDelta * m_fAnimSpeed;

	// »çÀÎÆÄ·Î ¾ÕµÚ Èçµé±â
	_float fSwing = sinf(m_fAnimTime) * 60.f;

	// ÆÈÀº ´Ù¸®¶û ¹Ý´ë·Î
	if (m_pArmL)  m_pArmL->Get_Transform()->Set_Rotation(ROT_X, fSwing);
	if (m_pArmR)  m_pArmR->Get_Transform()->Set_Rotation(ROT_X, -fSwing);
	if (m_pLegL)  m_pLegL->Get_Transform()->Set_Rotation(ROT_X, -fSwing);
	if (m_pLegR)  m_pLegR->Get_Transform()->Set_Rotation(ROT_X, fSwing);
}

void CJSPlayer::Update_JumpAnimation(const _float& fTimeDelta)
{
	m_fJumpAnimTime += fTimeDelta;

	if (m_fJumpAnimTime <= m_fJumpDuration)
	{
		// ¿Ã¶ó°¡´Â Áß
		if (m_pArmL) m_pArmL->Get_Transform()->Set_Rotation(ROT_X, -100.f);
		if (m_pArmR) m_pArmR->Get_Transform()->Set_Rotation(ROT_X, -100.f);
		if (m_pLegL) m_pLegL->Get_Transform()->Set_Rotation(ROT_X, 20.f);
		if (m_pLegR) m_pLegR->Get_Transform()->Set_Rotation(ROT_X, -20.f);
	}
	else
	{
		// ³»·Á¿À´Â Áß
		if (m_pArmL) m_pArmL->Get_Transform()->Set_Rotation(ROT_X, 40.f);
		if (m_pArmR) m_pArmR->Get_Transform()->Set_Rotation(ROT_X, 40.f);
		if (m_pLegL) m_pLegL->Get_Transform()->Set_Rotation(ROT_X, -10.f);
		if (m_pLegR) m_pLegR->Get_Transform()->Set_Rotation(ROT_X, 10.f);
	}
}

void CJSPlayer::Update_SlideAnimation()
{
	if (!m_bSlide) return;

	// ¸öÅë ¾ÕÀ¸·Î ´¯È÷±â
	if (m_pBody) m_pBody->Get_Transform()->Set_Rotation(ROT_X, 80.f);
	if (m_pHead) m_pHead->Get_Transform()->Set_Rotation(ROT_X, 80.f);

	// ÆÈ À§·Î »¸±â
	if (m_pArmL) m_pArmL->Get_Transform()->Set_Rotation(ROT_X, -150.f);
	if (m_pArmR) m_pArmR->Get_Transform()->Set_Rotation(ROT_X, -150.f);

	// ´Ù¸® »¸±â
	if (m_pLegL) m_pLegL->Get_Transform()->Set_Rotation(ROT_X, 80.f);
	if (m_pLegR) m_pLegR->Get_Transform()->Set_Rotation(ROT_X, 80.f);
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

	if (CDInputMgr::GetInstance()->Get_DIKeyState(DIK_UP) && !m_bJump && !m_bFalling && !m_bSlide)
	{
		m_fVelocityY = m_fJumpPower;
		m_bJump = true;
		m_fJumpAnimTime = 0.f;

		CSoundMgr::GetInstance()->PlayEffect(L"JS/2-04.-Grunt-Jump.wav", 0.8f);
	}

	if (CDInputMgr::GetInstance()->Get_DIKeyState(DIK_DOWN) && !m_bJump && !m_bFalling)
	{
		if (!m_bSlide)
		{
			m_bSlide = true;
			Safe_Release(m_pColliderCom);
			m_pColliderCom = CJSCollider::Create(m_pGraphicDev, { 0.f, -1.f, 0.f }, m_vSlideColSize);
			m_mapComponent[ID_DYNAMIC].erase(L"Com_Collider");
			m_mapComponent[ID_DYNAMIC].insert({ L"Com_Collider", m_pColliderCom });
		}
	}
	else
	{
		if (m_bSlide)
		{
			m_bSlide = false;
			Safe_Release(m_pColliderCom);
			m_pColliderCom = CJSCollider::Create(m_pGraphicDev, { 0.f, 0.0f, 0.f }, m_vNormalColSize);
			m_mapComponent[ID_DYNAMIC].erase(L"Com_Collider");
			m_mapComponent[ID_DYNAMIC].insert({ L"Com_Collider", m_pColliderCom });
		}
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
	Safe_Release(m_pHead);
	Safe_Release(m_pBody);
	Safe_Release(m_pArmL);
	Safe_Release(m_pArmR);
	Safe_Release(m_pLegL);
	Safe_Release(m_pLegR);
	CGameObject::Free();
}