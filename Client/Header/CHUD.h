#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"

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
	
public:
	void Set_Player(CPlayer* pPlayer) { m_pPlayer = pPlayer; }
	void Set_NetworkPlayer(CNetworkPlayer* pPlayer) { m_pNetworkPlayer = pPlayer; }
	void Clear_Player() { m_pPlayer = nullptr; m_pNetworkPlayer = nullptr; }

private:
	HRESULT			Add_Component();
	//UI 렌더링 설정
	void Render_BeginUI();
	void Render_EndUI();
	//포션 쿨타임
	void Render_PosionCoolTime();
	//에메랄드 개수
	void Render_EmeraldCount();
	//미션
	void Render_Mission();

	void Use_Posion(const _float fTimeDelta);

private:
	Engine::CRcTex* m_pBufferCom = nullptr;
	Engine::CTexture* m_pTextureCom = nullptr;
	CTexture* m_pFilledHeart = nullptr;
	CTexture* m_pEmptyHeart = nullptr;
	CTexture* m_pPosionCoolTime = nullptr;

	CPlayer* m_pPlayer = nullptr;
	CNetworkPlayer* m_pNetworkPlayer = nullptr;
	
	//Empty Heart 위치, 사이즈
	float m_fX, m_fY = 0.f;
	float m_fW, m_fH = 0.f;
	
	//Posion Cooltime 위치, 사이즈
	float m_fPosionX, m_fPosionY = 0.f;
	float m_fPosionW, m_fPosionH = 0.f;

	//플레이어 체력
	int m_iHP = 0;
	int m_iMaxHP = 0;

	//포션 쿨타임
	_float m_fPosionCooltime = 0.f;
	_float m_fPosionDuration = 1.f;
	_bool m_bIsPosionCoolTime = false;

	//에메랄드 
	int m_iEmerald = 0;
	
	//미션 카운트
	int m_iZombieCount = 0;
	int m_iCreeperCount = 0; 
	int m_iSkeletonCount = 0;
	int m_iSpiderCount = 0;

	//원본 행렬 저장
	_matrix m_matOriginView;
	_matrix m_matOriginProj;

public:
	static CHUD* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
	virtual void Free();

};

