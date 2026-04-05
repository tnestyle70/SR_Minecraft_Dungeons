#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"

enum class eMissionType
{
	MISSION_NPC1,
	MISSION_NPC2,
	MISSION_END
};

//HUD 플레이어의 생성과 동기화를 시켜서 체력 정보를 업데이트하는 것이 핵심
class CPlayer;
class CNetworkPlayer;

class CHUD : public CGameObject
{
private:
	explicit CHUD(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CHUD(const CGameObject& rhs);
	virtual ~CHUD();

public:
	virtual			HRESULT		Ready_GameObject();
	virtual			_int		Update_GameObject(const _float& fTimeDelta);
	virtual			void		LateUpdate_GameObject(const _float& fTimeDelta);
	virtual			void		Render_GameObject();

private:
	void Update_Dead(const _float fTimeDelta);
	void Update_Missison(const _float fTimeDelta);
	void Update_Death(const _float fTimeDelta);
	void Use_Posion(const _float fTimeDelta);
	
public:
	void Set_Player(CPlayer* pPlayer) { m_pPlayer = pPlayer; }
	void Set_NetworkPlayer(CNetworkPlayer* pPlayer) { m_pNetworkPlayer = pPlayer; }
	void Clear_Player() { m_pPlayer = nullptr; m_pNetworkPlayer = nullptr; }

	void ShowScore(bool bShow) { m_bShowScore = bShow; }

private:
	HRESULT			Add_Component();
	//UI 렌더링 설정
	void Render_BeginUI();
	void Render_EndUI();
	//포션 쿨타임
	void Render_PosionCoolTime();
	//에메랄드 개수
	void Render_CurrencyCount();
	//미션
	void Render_Mission();
	void Render_Mission1();
	void Render_Mission2();
	//미션 성공
	void Render_MissionComplete();
	//죽음
	void Render_Death();
	//킬 데스 스코어
	void Render_Score();

private:
	Engine::CRcTex* m_pBufferCom = nullptr;
	Engine::CTexture* m_pTextureCom = nullptr;
	//하트, 쿨타임, 미션 완료
	CTexture* m_pFilledHeart = nullptr;
	CTexture* m_pEmptyHeart = nullptr;
	CTexture* m_pPosionCoolTime = nullptr;
	CTexture* m_pMissionComplete = nullptr;
	//미션 텍스트
	CTexture* m_pZombieMission = nullptr;
	CTexture* m_pCripperMission = nullptr;
	CTexture* m_pSpiderMission = nullptr;
	CTexture* m_pSkeletonMission = nullptr;
	//아티펙트
	CTexture* m_pArtifact = nullptr;
	//Death 이미지
	CTexture* m_pDeath = nullptr;
	CTexture* m_pDeathBackground = nullptr;
	
	CPlayer* m_pPlayer = nullptr;
	CNetworkPlayer* m_pNetworkPlayer = nullptr;
	
	//Empty Heart 위치, 사이즈
	float m_fX, m_fY = 0.f;
	float m_fW, m_fH = 0.f;
	
	//Posion Cooltime 위치, 사이즈
	float m_fPosionX, m_fPosionY = 0.f;
	float m_fPosionW, m_fPosionH = 0.f;

	//MissionComplete 위치, 사이즈
	float m_fMissionComX, m_fMissionComY = 0.f;
	float m_fMissionComW, m_fMissionComH = 0.f;

	//MissionComplete Artifact 위치, 사이즈
	float m_fArtifactX, m_fArtifactY = 0.f;
	float m_fArtifactW, m_fArtifactH = 0.f;

	//Mission Text 위치, 사이즈
	float m_fMissionTextX, m_fMissionTextY = 0.f;
	float m_fMissionTextW, m_fMissionTextH = 0.f;

	//Dead Texture 위치, 사이즈
	float m_fDeathX, m_fDeathY = 0.f;
	float m_fDeathW, m_fDeathH = 0.f;

	//플레이어 체력
	int m_iHP = 0;
	int m_iMaxHP = 0;

	//포션 쿨타임
	_float m_fPosionCooltime = 0.f;
	_float m_fPosionDuration = 1.f;
	_bool m_bIsPosionCoolTime = false;

	//사망 쿨타임 
	_float m_fDeathCooltime = 0.f;
	_float m_fDeathDuration = 2.f;
	bool m_bDeath = false;
	
	//에메랄드, 유물 
	int m_iEmerald = 0;
	int m_iArtifact = 0;
	
	//미션 카운트
	int m_iZombieCount = 0;
	int m_iCreeperCount = 0; 
	int m_iSkeletonCount = 0;
	int m_iSpiderCount = 0;
	eMissionType m_eMissionType = eMissionType::MISSION_END;
	//미션 - 몬스터, 스켈레톤
	bool m_bMissionNPC1 = false;
	bool m_bMissionNPC2 = false;

	bool m_bMissionComplete = false;
	bool m_bFirstMissionComplete = true;
	float m_fMissionCoolTime = 0.f;
	float m_fMissionDuration = 1.f;

	//킬 데스 관리 
	int m_iKillCount = 0;
	int m_iDeathCount = 0;
	bool m_bShowScore = false;
	
	//원본 행렬 저장
	_matrix m_matOriginView;
	_matrix m_matOriginProj;

public:
	static CHUD* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
	virtual void Free();

};
