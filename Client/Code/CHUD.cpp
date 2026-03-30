#include "pch.h"
#include "CHUD.h"
#include "CRenderer.h"
#include "CPlayer.h"
#include "CNetworkPlayer.h"
#include "CInventoryMgr.h"
#include "CFontMgr.h"
#include "CEventBus.h"
#include "CMonster.h"

CHUD::CHUD(LPDIRECT3DDEVICE9 pGraphicDev)
	:CGameObject(pGraphicDev)
{
	D3DXMatrixIdentity(&m_matOriginView);
	D3DXMatrixIdentity(&m_matOriginProj);
}

CHUD::CHUD(const CGameObject& rhs)
	:CGameObject(rhs)
{
}

CHUD::~CHUD()
{
}

HRESULT CHUD::Ready_GameObject()
{
	if (FAILED(Add_Component()))
		return E_FAIL;

	//하트 위치 사이즈 지정
	m_fX = 595.f; m_fY = 622.f;
	m_fW = 90.f; m_fH = 80.f;

	//포션 쿨타임 위치 사이즈 지정
	m_fPosionX = 690.f; m_fPosionY = 694.f;
	m_fPosionW = 53.f; m_fPosionH = 65.f;

	//이벤트 버스 연결, 처치된 몬스터 수 받기
	CEventBus::GetInstance()->Subscribe(eEventType::MONSTER_DEAD, this,
		[this](const FGameEvent& event)
		{
			switch (static_cast<EMonsterType>(event.iSubType))
			{
			case EMonsterType::ZOMBIE:    m_iZombieCount += event.iValue; break;
			case EMonsterType::CREEPER:   m_iCreeperCount += event.iValue; break;
			case EMonsterType::SKELETON:  m_iSkeletonCount += event.iValue; break;
			case EMonsterType::SPIDER:    m_iSpiderCount += event.iValue; break;
			}

		});

	return S_OK;
}

_int CHUD::Update_GameObject(const _float& fTimeDelta)
{
	_int iExit = CGameObject::Update_GameObject(fTimeDelta);

	Use_Posion(fTimeDelta);

	CRenderer::GetInstance()->Add_RenderGroup(RENDER_UI, this);

	return iExit;
}

void CHUD::LateUpdate_GameObject(const _float& fTimeDelta)
{
	CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CHUD::Render_GameObject()
{
	if (!m_pPlayer && !m_pNetworkPlayer)
		return;
	//플레이어 체력 연동
	if (m_pPlayer)
	{
		m_iHP = m_pPlayer->Get_Hp();
		m_iMaxHP = m_pPlayer->Get_MaxHp();
	}
	else if (m_pNetworkPlayer)
	{
		m_iHP = m_pNetworkPlayer->Get_Hp();
		m_iMaxHP = m_pNetworkPlayer->Get_MaxHp();
	}
	else
	{
		return;
	}
	
	//체력 비율에 따른 데미지 비율, 즉 Empty Heart 렌더 비율 설정
	float fRatio = (m_iMaxHP > 0) ? (float)m_iHP / (float)m_iMaxHP : 0.f;
	float fDamageRatio = 1.f - fRatio;

	Render_BeginUI();

	//기본 HUD Render
	m_pTextureCom->Set_Texture(0);
	m_pBufferCom->Render_Buffer();

	Render_PosionCoolTime();
	Render_EmeraldCount(); 
	Render_Mission();

	//Empty Heart 비율에 따른 렌더
	if (fDamageRatio > 0.f)
	{
		//쿼드의 높이는 fEmptyH로 줄이되, UV도 fDamageRatio만큼만 줄이기
		float fEmptyH = m_fH * fDamageRatio;
		//상단을 고정하기 위해서 Y를 fEmptyH * 0.5를 기준으로 잡는다?
		float fNDCX = (m_fX + m_fW * 0.5f) / (WINCX * 0.5f) - 1.f;
		float fNDCY = 1.f - (m_fY + fEmptyH * 0.5f) / (WINCY * 0.5f);
		
		_matrix matWorld;
		D3DXMatrixTransformation2D(&matWorld,
			nullptr, 0.f,
			&_vec2(m_fW / WINCX, fEmptyH / WINCY), //높이 비율 조정
			nullptr, 0.f,
			&_vec2(fNDCX, fNDCY));
		m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);
		//UV도 DamageRatio만큼만 샘플링
		_matrix matTexture;
		D3DXMatrixScaling(&matTexture, 1.f, fDamageRatio, 1.f);
		m_pGraphicDev->SetTransform(D3DTS_TEXTURE0, &matTexture);
		m_pGraphicDev->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
		//텍스쳐만 교체하고, Buffer는 그대로 써도 무관
		m_pEmptyHeart->Set_Texture(0);
		m_pBufferCom->Render_Buffer();
	}

	//UV Transform 복구
	m_pGraphicDev->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
	
	Render_EndUI();
}

void CHUD::Render_BeginUI()
{
	//원본 행렬 저장
	m_pGraphicDev->GetTransform(D3DTS_VIEW, &m_matOriginView);
	m_pGraphicDev->GetTransform(D3DTS_PROJECTION, &m_matOriginProj);

	//월드 행렬 Identity로 설정
	_matrix matWorld;
	D3DXMatrixIdentity(&matWorld);
	m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);
	
	//View / Proj 행렬 풀어주기
	_matrix matIdentity;
	D3DXMatrixIdentity(&matIdentity);
	m_pGraphicDev->SetTransform(D3DTS_VIEW, &matIdentity);
	m_pGraphicDev->SetTransform(D3DTS_PROJECTION, &matIdentity);

	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_pGraphicDev->SetRenderState(D3DRS_ZENABLE, FALSE);
	m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_pGraphicDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_pGraphicDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	m_pGraphicDev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	m_pGraphicDev->SetRenderState(D3DRS_ALPHAREF, 0x10);
}

void CHUD::Render_EndUI()
{
	//렌더링 상태 복구 - 월드 행렬은 개별 클래스 Render시에 World 행렬 세팅
	m_pGraphicDev->SetTransform(D3DTS_VIEW, &m_matOriginView);
	m_pGraphicDev->SetTransform(D3DTS_PROJECTION, &m_matOriginProj);

	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	m_pGraphicDev->SetRenderState(D3DRS_ZENABLE, TRUE);
	m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
}

void CHUD::Render_PosionCoolTime()
{
	//시간에 따른 포션 쿨타임 렌더링 정도 설정
	float fRatio = m_fPosionCooltime / m_fPosionDuration;
	//float fCooltimeRatio = 1.f - fRatio;

	//포션 쿨타임 렌더링
	if (m_bIsPosionCoolTime)
	{
		//쿼드의 높이는 fEmptyH로 줄이되, UV도 fDamageRatio만큼만 줄이기
		float fCoolTimeH = 1.f - m_fPosionH * fRatio;
		//상단을 고정하기 위해서 Y를 fEmptyH * 0.5를 기준으로 잡는다?
		float fNDCX = (m_fPosionX + m_fPosionW * 0.5f) / (WINCX * 0.5f) - 1.f;
		float fNDCY = 1.f - (m_fPosionY + fCoolTimeH * 0.5f) / (WINCY * 0.5f);

		_matrix matWorld;
		D3DXMatrixTransformation2D(&matWorld,
			nullptr, 0.f,
			&_vec2(m_fPosionW / WINCX, fCoolTimeH / WINCY), //높이 비율 조정
			nullptr, 0.f,
			&_vec2(fNDCX, fNDCY));
		m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);
		//UV도 DamageRatio만큼만 샘플링
		_matrix matTexture;
		D3DXMatrixScaling(&matTexture, 1.f, fCoolTimeH, 1.f);
		m_pGraphicDev->SetTransform(D3DTS_TEXTURE0, &matTexture);
		m_pGraphicDev->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
		//텍스쳐만 교체하고, Buffer는 그대로 써도 무관
		m_pPosionCoolTime->Set_Texture(0);
		m_pBufferCom->Render_Buffer();
	}
	else
		return;
}

void CHUD::Render_EmeraldCount()
{
	_vec2 vPos{ 1000.f, 680.f };
	m_iEmerald = CInventoryMgr::GetInstance()->Get_EmeraldCount();
	_tchar buf[32];
	swprintf_s(buf, L"%d", m_iEmerald);

	CFontMgr::GetInstance()->Render_Font(
		L"Font_Minecraft", buf, &vPos, D3DXCOLOR(1.f, 1.f, 1.f, 1.f));
}

void CHUD::Render_Mission()
{
	_vec2 vPos{ 800.f, 50.f };
	//좀비 
	_tchar missionBuf[32];
	swprintf_s(missionBuf, L"좀비를 처치하세요 %d / 10", m_iZombieCount);
	
	CFontMgr::GetInstance()->Render_Font(
		L"Font_Minecraft", missionBuf, &vPos, D3DXCOLOR(1.f, 1.f, 1.f, 1.f));
	
	//크리퍼
	vPos.y += 20.f;

	swprintf_s(missionBuf, L"크리퍼를 처치하세요 %d / 10", m_iCreeperCount);

	CFontMgr::GetInstance()->Render_Font(
		L"Font_Minecraft", missionBuf, &vPos, D3DXCOLOR(1.f, 1.f, 1.f, 1.f));
	
	//스켈레톤
	vPos.y += 20.f;

	swprintf_s(missionBuf, L"스켈레톤을 처치하세요 %d / 10", m_iSkeletonCount);

	CFontMgr::GetInstance()->Render_Font(
		L"Font_Minecraft", missionBuf, &vPos, D3DXCOLOR(1.f, 1.f, 1.f, 1.f));

	//거미
	vPos.y += 20.f;

	swprintf_s(missionBuf, L"거미를 처치하세요 %d / 10", m_iSpiderCount);

	CFontMgr::GetInstance()->Render_Font(
		L"Font_Minecraft", missionBuf, &vPos, D3DXCOLOR(1.f, 1.f, 1.f, 1.f));
}

void CHUD::Use_Posion(const _float fTimeDelta)
{
	//포션 사용시 쿨타임 돌리기
	if (GetAsyncKeyState('T') & 0x8000)
	{
		m_bIsPosionCoolTime = true;
		m_fPosionCooltime = m_fPosionDuration;
	}
	//포션 쿨타임 돌리기
	if (m_bIsPosionCoolTime)
	{
		m_fPosionCooltime -= fTimeDelta;
		if (m_fPosionCooltime <= 0.f)
		{
			m_fPosionCooltime = 0.f;
			m_bIsPosionCoolTime = false;
		}
	}
}

HRESULT CHUD::Add_Component()
{
	Engine::CComponent* pComponent = nullptr;

	// RcTex
	pComponent = m_pBufferCom = dynamic_cast<CRcTex*>(
		CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RcTex"));

	if (nullptr == pComponent)
		return E_FAIL;
	
	m_mapComponent[ID_STATIC].insert({ L"Com_Buffer", pComponent });

	// Texture
	pComponent = m_pTextureCom = dynamic_cast<CTexture*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_HUDTexture"));

	if (nullptr == pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_HUDTexture", pComponent });

	//Filled Heart Texture
	pComponent = m_pFilledHeart = dynamic_cast<CTexture*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_FilledHeart"));

	if (nullptr == pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_FillHeartTexture", pComponent });

	//Empty Heart Texture
	pComponent = m_pEmptyHeart = dynamic_cast<CTexture*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_EmptyHeart"));

	if (nullptr == pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_EmptyHeartTexture", pComponent });

	//Posion CoolTime
	pComponent = m_pPosionCoolTime = dynamic_cast<CTexture*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_PosionCoolDown"));

	if (nullptr == pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_PosionCoolTimeTexture", pComponent });

	return S_OK;
}

CHUD* CHUD::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
	CHUD* pHUD = new CHUD(pGraphicDev);

	if (FAILED(pHUD->Ready_GameObject()))
	{
		Safe_Release(pHUD);
		MSG_BOX("pHUD Create Failed");
		return nullptr;
	}

	return pHUD;
}

void CHUD::Free()
{
	//이벤트 버스 소멸전 반드시 해제!
	CEventBus::GetInstance()->Unsubscribe(eEventType::MONSTER_DEAD, this);
	CGameObject::Free();
}
