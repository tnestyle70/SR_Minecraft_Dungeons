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

	//빈하트 위치 사이즈 지정
	m_fX = 590.f; m_fY = 616.f;
	m_fW = 104.f; m_fH = 92.f;
	
	//포션 쿨타임 위치 사이즈 지정
	m_fPosionX = 696.f; m_fPosionY = 694.f;
	m_fPosionW = 48.f; m_fPosionH = 62.f;

	//화살
	m_fArrowsX = 884.f; m_fArrowsY = 636.f;
	m_fArrowsW = 60.f; m_fArrowsH = 56.f;
	
	//미션 완료 텍스트
	m_fMissionComX = 470.f; m_fMissionComY = 80.f;
	m_fMissionComW = 340.f; m_fMissionComH = 150.f;
	
	m_fMissionCoolTime = m_fMissionDuration;
	//m_eMissionType = eMissionType::MISSION_NPC1;

	//아티펙트
	m_fArtifactX = 560.f; m_fArtifactY = 160.f;
	m_fArtifactW = 150.f; m_fArtifactH = 150.f;

	//미션 텍스트
	m_fMissionTextX = 835.f; m_fMissionTextY = 15.f;
	m_fMissionTextW = 300.f; m_fMissionTextH = 40.f;

	//Death Image
	m_fDeathX = 470.f; m_fDeathY = 50.f;
	m_fDeathW = 300.f; m_fDeathH = 300.f;
	
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

	//이벤트 버스 연결, 미션 성공
	CEventBus::GetInstance()->Subscribe(eEventType::MISSION_COMPLETE, this,
		[this](const FGameEvent& event)
		{
			switch (static_cast<eMissionType>(event.iSubType))
			{
			case eMissionType::MISSION_NPC1:    m_bMissionNPC1 = false; break;
			case eMissionType::MISSION_NPC2:   m_bMissionNPC2 = false; break;
			case eMissionType::MISSION_ENDERDRAGON:   
				m_bMissionEnderDragon = false;
				m_iEnderDragonCount++;
				break;
			}
			m_bMissionComplete = static_cast<bool>(event.iValue);
		});

	//이벤트 미션 수락
	CEventBus::GetInstance()->Subscribe(eEventType::MISSION_ACCEPT, this,
		[this](const FGameEvent& event)
		{
			switch (static_cast<eMissionType>(event.iValue))
			{
			case eMissionType::MISSION_NPC1:    m_bMissionNPC1 = true; break;
			case eMissionType::MISSION_NPC2:   m_bMissionNPC2 = true; break;
			}
		});
	//킬 데스 이벤트 버스 등록
	CEventBus::GetInstance()->Subscribe(eEventType::PLAYER_KILL, this,
		[this](const FGameEvent& event)
		{
			m_iKillCount += 1;
		});
	CEventBus::GetInstance()->Subscribe(eEventType::PLAYER_DEAD, this,
		[this](const FGameEvent& event)
		{
			m_iDeathCount += 1;
		});

	return S_OK;
}

_int CHUD::Update_GameObject(const _float& fTimeDelta)
{
	_int iExit = CGameObject::Update_GameObject(fTimeDelta);

	Update_Dead(fTimeDelta);

	Use_Posion(fTimeDelta);

	Update_Missison(fTimeDelta);

	CRenderer::GetInstance()->Add_RenderGroup(RENDER_UI, this);

	return iExit;
}

void CHUD::LateUpdate_GameObject(const _float& fTimeDelta)
{
	CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CHUD::Update_Dead(const _float fTimeDelta)
{
	if (!m_pNetworkPlayer)
		return;

	bool bPlayerDead = m_pNetworkPlayer->Is_Dead();
	//살아있다가 죽음 감지
	if (bPlayerDead && !m_bDeath)
	{
		m_bDeath = true;
		m_fDeathCooltime = 0.f;
	}

	if (m_bDeath)
	{
		m_fDeathCooltime += fTimeDelta;
		if (m_fDeathCooltime >= m_fDeathDuration)
		{
			m_bDeath = false;
			m_fDeathCooltime = 0.f;
			m_pNetworkPlayer->Set_Respawned();
		}
	}
}

void CHUD::Update_Missison(const _float fTimeDelta)
{
	//미션 성공 체크
	if (m_bMissionComplete)
	{
		m_fMissionCoolTime -= fTimeDelta;
		if (m_fMissionCoolTime <= 0.f)
		{
			m_fMissionCoolTime = m_fMissionDuration;
			m_bMissionComplete = false;
			m_bFirstMissionComplete = true;
			//미션 enum 증가
			m_eMissionType = eMissionType((int)m_eMissionType + 1);
		}
	}
	
	///미션 성공 Event Bus 
	switch (m_eMissionType)
	{
	case eMissionType::MISSION_NPC1:
		if (m_iZombieCount >= 10 && m_iCreeperCount >= 10 && m_iSpiderCount >= 10
			 && m_bFirstMissionComplete)
		{
			FGameEvent event;
			event.eType = eEventType::MISSION_COMPLETE;
			event.iValue = 1; //Gain Artifact
			event.iSubType = 0;
			CEventBus::GetInstance()->Publish(event);

			m_bFirstMissionComplete = false;
		}
		break;
	case eMissionType::MISSION_NPC2:
		if (m_iSkeletonCount >= 10 && m_bFirstMissionComplete)
		{
			FGameEvent event;
			event.eType = eEventType::MISSION_COMPLETE;
			event.iValue = 1; //Gain Artifact
			event.iSubType = 1;
			CEventBus::GetInstance()->Publish(event);

			m_bFirstMissionComplete = false;
		}
		break;
	case eMissionType::MISSION_ENDERDRAGON:
		if (m_iEnderDragonCount >= 1)
		{
			FGameEvent event;
			event.eType = eEventType::MISSION_COMPLETE;
			event.iValue = 1; //Gain Artifact
			event.iSubType = 1;
			CEventBus::GetInstance()->Publish(event);

			m_bFirstMissionComplete = false;
		}
		break;
	case eMissionType::MISSION_END:
		break;
	default:
		break;
	}
}

void CHUD::Update_Death(const _float fTimeDelta)
{
	//플레이어가 죽었을 경우 타이머 돌리기
	m_bDeath = m_pNetworkPlayer->Is_Dead();
	if (m_bDeath)
	{
		m_fDeathCooltime += fTimeDelta;
		if (m_fDeathCooltime >= m_fDeathDuration)
		{
			m_bDeath = false;
			//m_pNetworkPlayer->Set_Revive();
		}
	}
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

	Render_Arrows();
	Render_PosionCoolTime();
	Render_CurrencyCount(); 
	Render_Mission();
	Render_MissionComplete();
	Render_Death();
	Render_Score();
	
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

void CHUD::Render_Arrows()
{
	_matrix matWorld;
	float fNDCX = (m_fArrowsX + m_fArrowsW * 0.5f) / (WINCX * 0.5f) - 1.f;
	float fNDCY = 1.f - (m_fArrowsY + m_fArrowsH * 0.5f) / (WINCY * 0.5f);
	float fScaleX = m_fArrowsW / WINCX;
	float fScaleY = m_fArrowsH / WINCY;

	D3DXMatrixTransformation2D(&matWorld,
		nullptr, 0.f,
		&_vec2(fScaleX, fScaleY),
		nullptr, 0.f,
		&_vec2(fNDCX, fNDCY));

	m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);

	//Set_Texture
	//GPU의 텍스쳐 스테이지에 해당 이미지를 올려둠, 모든 도형에는 해당 텍스쳐로 그려짐
	//Vertex Texture에서 설정한 texture UV 값을 통해서 
	//GPU는 설정된 텍스쳐의 어디서부터 어디까지를 그릴지를 결정해서 렌더링한다

	m_pArrows->Set_Texture(0);
	m_pBufferCom->Render_Buffer();
}

void CHUD::Render_CurrencyCount()
{
	//에메랄드
	_vec2 vPos{ 1000.f, 670.f };
	m_iEmerald = CInventoryMgr::GetInstance()->Get_EmeraldCount();
	_tchar buf[32];
	swprintf_s(buf, L"%d", m_iEmerald);

	CFontMgr::GetInstance()->Render_Font(
		L"Font_Minecraft", buf, &vPos, D3DXCOLOR(1.f, 1.f, 1.f, 1.f));

	//유물
	_vec2 vArtifact{ 314.f, 670.f };

	m_iArtifact = CInventoryMgr::GetInstance()->Get_ArtifactCount();
	swprintf_s(buf, L"%d", m_iArtifact);
	
	CFontMgr::GetInstance()->Render_Font(
		L"Font_Minecraft", buf, &vArtifact, D3DXCOLOR(1.f, 1.f, 1.f, 1.f));
}

void CHUD::Render_Mission()
{
	switch (m_eMissionType)
	{
	case eMissionType::MISSION_NPC1:
		Render_Mission1();
		break;
	case eMissionType::MISSION_NPC2:
		Render_Mission2();
		break;
	case eMissionType::MISSION_ENDERDRAGON:
		Render_MissionEnderDragon();
		break;
	default:
		break;
	}
}

void CHUD::Render_Mission1()
{
	//NPC1에게 미션을 받았을 경우 렌더링
	if (!m_bMissionNPC1)
		return;

	//좀비 미션
	float fNDCX = (m_fMissionTextX + m_fMissionTextW * 0.5f) / (WINCX * 0.5f) - 1.f;
	float fNDCY = 1.f - (m_fMissionTextY + m_fMissionTextH * 0.5f) / (WINCY * 0.5f);
	
	_matrix matWorld;
	D3DXMatrixTransformation2D(&matWorld,
		nullptr, 0.f,
		&_vec2(m_fMissionTextW / WINCX, m_fMissionTextH / WINCY), //높이 비율 조정
		nullptr, 0.f,
		&_vec2(fNDCX, fNDCY));
	m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);

	_matrix matTexture;
	D3DXMatrixScaling(&matTexture, 1.f, 1.f, 1.f);
	m_pGraphicDev->SetTransform(D3DTS_TEXTURE0, &matTexture);
	m_pGraphicDev->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
	//텍스쳐만 교체하고, Buffer는 그대로 써도 무관
	m_pZombieMission->Set_Texture(0);
	m_pBufferCom->Render_Buffer();

	//크리퍼 미션
	fNDCX = (m_fMissionTextX + m_fMissionTextW * 0.5f) / (WINCX * 0.5f) - 1.f;
	fNDCY = 1.f - (m_fMissionTextY + 40.f +m_fMissionTextH * 0.5f) / (WINCY * 0.5f);

	D3DXMatrixTransformation2D(&matWorld,
		nullptr, 0.f,
		&_vec2(m_fMissionTextW / WINCX, m_fMissionTextH / WINCY), //높이 비율 조정
		nullptr, 0.f,
		&_vec2(fNDCX, fNDCY));
	m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);
	//텍스쳐 렌더링
	m_pCripperMission->Set_Texture(0);
	m_pBufferCom->Render_Buffer();

	//거미 미션
	fNDCX = (m_fMissionTextX + m_fMissionTextW * 0.5f) / (WINCX * 0.5f) - 1.f;
	fNDCY = 1.f - (m_fMissionTextY + 80.f + m_fMissionTextH * 0.5f) / (WINCY * 0.5f);

	D3DXMatrixTransformation2D(&matWorld,
		nullptr, 0.f,
		&_vec2(m_fMissionTextW / WINCX, m_fMissionTextH / WINCY), //높이 비율 조정
		nullptr, 0.f,
		&_vec2(fNDCX, fNDCY));
	m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);

	//텍스쳐 렌더링
	m_pSpiderMission->Set_Texture(0);
	m_pBufferCom->Render_Buffer();

	//숫자 렌더링
	_vec2 vPos{ 1150.f, 20.f };
	//좀비
	_tchar missionBuf[32];
	swprintf_s(missionBuf, L"%d / 10", m_iZombieCount);

	CFontMgr::GetInstance()->Render_Font(
		L"Font_Minecraft", missionBuf, &vPos, D3DXCOLOR(1.f, 1.f, 1.f, 1.f));

	//크리퍼
	vPos.y += 40.f;

	swprintf_s(missionBuf, L"%d / 10", m_iCreeperCount);

	CFontMgr::GetInstance()->Render_Font(
		L"Font_Minecraft", missionBuf, &vPos, D3DXCOLOR(1.f, 1.f, 1.f, 1.f));

	//거미
	vPos.y += 40.f;

	swprintf_s(missionBuf, L"%d / 10", m_iSpiderCount);

	CFontMgr::GetInstance()->Render_Font(
		L"Font_Minecraft", missionBuf, &vPos, D3DXCOLOR(1.f, 1.f, 1.f, 1.f));
}

void CHUD::Render_Mission2()
{
	//NPC2에게 미션을 받았을 경우 렌더링
	if (!m_bMissionNPC2)
		return;
	
	//상단을 고정하기 위해서 Y를 fEmptyH * 0.5를 기준으로 잡는다?
	float fNDCX = (m_fMissionTextX + m_fMissionTextW * 0.5f) / (WINCX * 0.5f) - 1.f;
	float fNDCY = 1.f - (m_fMissionTextY + m_fMissionTextH * 0.5f) / (WINCY * 0.5f);

	_matrix matWorld;
	D3DXMatrixTransformation2D(&matWorld,
		nullptr, 0.f,
		&_vec2(m_fMissionTextW / WINCX, m_fMissionTextH / WINCY), //높이 비율 조정
		nullptr, 0.f,
		&_vec2(fNDCX, fNDCY));
	m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);

	_matrix matTexture;
	D3DXMatrixScaling(&matTexture, 1.f, 1.f, 1.f);
	m_pGraphicDev->SetTransform(D3DTS_TEXTURE0, &matTexture);
	m_pGraphicDev->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
	//텍스쳐만 교체하고, Buffer는 그대로 써도 무관
	m_pSkeletonMission->Set_Texture(0);
	m_pBufferCom->Render_Buffer();

	_vec2 vPos{ 1150.f, 20.f };

	//스켈레톤
	_tchar missionBuf[32];
	swprintf_s(missionBuf, L"%d / 10", m_iSkeletonCount);

	CFontMgr::GetInstance()->Render_Font(
		L"Font_Minecraft", missionBuf, &vPos, D3DXCOLOR(1.f, 1.f, 1.f, 1.f));
}

void CHUD::Render_MissionEnderDragon()
{
	//EnderDragon 엔더 드래곤
	if (!m_bMissionEnderDragon)
		return;

	//상단을 고정하기 위해서 Y를 fEmptyH * 0.5를 기준으로 잡는다?
	float fNDCX = (m_fMissionTextX + m_fMissionTextW * 0.5f) / (WINCX * 0.5f) - 1.f;
	float fNDCY = 1.f - (m_fMissionTextY + m_fMissionTextH * 0.5f) / (WINCY * 0.5f);

	_matrix matWorld;
	D3DXMatrixTransformation2D(&matWorld,
		nullptr, 0.f,
		&_vec2(m_fMissionTextW / WINCX, m_fMissionTextH / WINCY), //높이 비율 조정
		nullptr, 0.f,
		&_vec2(fNDCX, fNDCY));
	m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);

	_matrix matTexture;
	D3DXMatrixScaling(&matTexture, 1.f, 1.f, 1.f);
	m_pGraphicDev->SetTransform(D3DTS_TEXTURE0, &matTexture);
	m_pGraphicDev->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
	//텍스쳐만 교체하고, Buffer는 그대로 써도 무관
	m_pEnderDragonMission->Set_Texture(0);
	m_pBufferCom->Render_Buffer();

	_vec2 vPos{ 1150.f, 20.f };
	//엔더드래곤
	_tchar missionBuf[32];
	swprintf_s(missionBuf, L"%d / 1", m_iEnderDragonCount);

	CFontMgr::GetInstance()->Render_Font(
		L"Font_Minecraft", missionBuf, &vPos, D3DXCOLOR(1.f, 1.f, 1.f, 1.f));
}

void CHUD::Render_MissionComplete()
{
	if (!m_bMissionComplete)
		return;
	//상단을 고정하기 위해서 Y를 fEmptyH * 0.5를 기준으로 잡는다?
	float fNDCX = (m_fMissionComX + m_fMissionComW * 0.5f) / (WINCX * 0.5f) - 1.f;
	float fNDCY = 1.f - (m_fMissionComY + m_fMissionComH * 0.5f) / (WINCY * 0.5f);

	_matrix matWorld;
	D3DXMatrixTransformation2D(&matWorld,
		nullptr, 0.f,
		&_vec2(m_fMissionComW / WINCX, m_fMissionComH / WINCY), //높이 비율 조정
		nullptr, 0.f,
		&_vec2(fNDCX, fNDCY));
	m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);

	_matrix matTexture;
	D3DXMatrixScaling(&matTexture, 1.f, 1.f, 1.f);
	m_pGraphicDev->SetTransform(D3DTS_TEXTURE0, &matTexture);
	m_pGraphicDev->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
	//텍스쳐만 교체하고, Buffer는 그대로 써도 무관
	m_pMissionComplete->Set_Texture(0);
	m_pBufferCom->Render_Buffer();

	//아티펙트 렌더링
	fNDCX = (m_fArtifactX + m_fArtifactW * 0.5f) / (WINCX * 0.5f) - 1.f;
	fNDCY = 1.f - (m_fArtifactY + m_fArtifactH * 0.5f) / (WINCY * 0.5f);

	D3DXMatrixTransformation2D(&matWorld,
		nullptr, 0.f,
		&_vec2(m_fArtifactW / WINCX, m_fArtifactH / WINCY), //높이 비율 조정
		nullptr, 0.f,
		&_vec2(fNDCX, fNDCY));
	m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);
	
	m_pArtifact->Set_Texture(0);
	m_pBufferCom->Render_Buffer();
}

void CHUD::Render_Death()
{
	if (!m_bDeath)
		return;
	//검정 반투명 배경 깔기
	_matrix matBackground;
	D3DXMatrixTransformation2D(&matBackground,
		nullptr, 0.f, //클라이언트 사이즈에 대한 death의 비율
		&_vec2(1.f, 1.f),
		nullptr, 0.f,
		&_vec2(0.f, 0.f));
	m_pGraphicDev->SetTransform(D3DTS_WORLD, &matBackground);

	_matrix matTexBG;
	D3DXMatrixScaling(&matTexBG, 1.f, 1.f, 1.f);
	m_pGraphicDev->SetTransform(D3DTS_TEXTURE0, &matTexBG);
	m_pGraphicDev->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);

	m_pDeathBackground->Set_Texture(0);
	m_pBufferCom->Render_Buffer();

	//사망 텍스쳐 띄우기
	float fNDCX = (m_fDeathX + m_fDeathW * 0.5f) / (WINCX * 0.5f) - 1.f;
	float fNDCY = 1.f - (m_fDeathY + m_fDeathH * 0.5f) / (WINCY * 0.5f);

	_matrix matWorld;
	D3DXMatrixTransformation2D(&matWorld,
		nullptr, 0.f, //클라이언트 사이즈에 대한 death의 비율
		&_vec2(m_fDeathW / WINCX, m_fDeathH / WINCY),
		nullptr, 0.f,
		&_vec2(fNDCX, fNDCY));
	m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);

	_matrix matTexture;
	D3DXMatrixScaling(&matTexture, 1.f, 1.f, 1.f);
	m_pGraphicDev->SetTransform(D3DTS_TEXTURE0, &matTexture);
	m_pGraphicDev->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);

	m_pDeath->Set_Texture(0);
	m_pBufferCom->Render_Buffer();

	//Respawn 카운트 다운 텍스트
	float fRemaining = m_fDeathDuration - m_fDeathCooltime;
	int iCountDown = (int)ceilf(fRemaining);
	if (iCountDown < 1)
		iCountDown = 1;
	if (iCountDown > 5)
		iCountDown = 5;


	//리스폰 카운트 다운
	_tchar countdownBuf[32];
	swprintf_s(countdownBuf, L"Respawn %d", iCountDown);

	_vec2 vPos = { (float)(WINCX / 2 - 50), (float)(WINCY / 2 + 80) };

	CFontMgr::GetInstance()->Render_Font(
		L"Font_Minecraft", countdownBuf, &vPos, D3DXCOLOR(1.f, 1.f, 1.f, 1.f));
}

void CHUD::Render_Score()
{
	if (!m_bShowScore)
		return;
	//킬
	_tchar scoreBuf[32];
	_vec2 vPos = { 20.f, 20.f };
	swprintf_s(scoreBuf, L"Kill : %d", m_iKillCount);
	
	CFontMgr::GetInstance()->Render_Font(
		L"Font_Minecraft", scoreBuf, &vPos, D3DXCOLOR(1.f, 1.f, 1.f, 1.f));

	//데스 
	vPos.y += 30.f;
	swprintf_s(scoreBuf, L"Death : %d", m_iDeathCount);

	CFontMgr::GetInstance()->Render_Font(
		L"Font_Minecraft", scoreBuf, &vPos, D3DXCOLOR(1.f, 1.f, 1.f, 1.f));
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

	//Arrows
	pComponent = m_pArrows= dynamic_cast<CTexture*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_ArrowsTexture"));

	if (nullptr == pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_ArrowsTexture", pComponent });

	//Mission Complete
	pComponent = m_pMissionComplete = dynamic_cast<CTexture*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_MissionCompleteText"));

	if (nullptr == pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_MissionCompleteTexture", pComponent });

	//Zombie Mission
	pComponent = m_pZombieMission = dynamic_cast<CTexture*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_ZombieText"));

	if (nullptr == pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_ZombieTextTexture", pComponent });

	//Cripper Mission
	pComponent = m_pCripperMission = dynamic_cast<CTexture*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_CripperText"));

	if (nullptr == pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_CripperTextTexture", pComponent });

	//Spider Mission
	pComponent = m_pSpiderMission = dynamic_cast<CTexture*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_SpiderText"));

	if (nullptr == pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_SpiderTextTexture", pComponent });

	//Skeleton Mission
	pComponent = m_pSkeletonMission = dynamic_cast<CTexture*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_SkeletonText"));

	if (nullptr == pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_SkeletonTextTexture", pComponent });

	//EnderDragon Mission
	pComponent = m_pEnderDragonMission = dynamic_cast<CTexture*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_EnderDragonText"));

	if (nullptr == pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_EnderDragonTextTexture", pComponent });

	//Artifact
	pComponent = m_pArtifact = dynamic_cast<CTexture*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_ArtifactTexture"));

	if (nullptr == pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_ArtifactTexture", pComponent });

	//Death
	pComponent = m_pDeath = dynamic_cast<CTexture*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_DeathTexture"));

	if (nullptr == pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_DeathTexture", pComponent });

	//Death Background
	pComponent = m_pDeathBackground = dynamic_cast<CTexture*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_PosionCoolDown"));

	if (nullptr == pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_DeathBackgroundTexture", pComponent });
	
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
	CEventBus::GetInstance()->Unsubscribe(eEventType::MISSION_COMPLETE, this);
	CEventBus::GetInstance()->Unsubscribe(eEventType::MISSION_ACCEPT, this);
	CEventBus::GetInstance()->Unsubscribe(eEventType::PLAYER_KILL, this);
	CEventBus::GetInstance()->Unsubscribe(eEventType::PLAYER_DEAD, this);

	CGameObject::Free();
}
