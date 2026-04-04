#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"

class CJSPlayer : public CGameObject
{
private:
	explicit CJSPlayer(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual ~CJSPlayer();

public:
	virtual HRESULT Ready_GameObject();
	virtual _int Update_GameObject(const _float& fTimeDelta);
	virtual void LateUpdate_GameObject(const _float& fTimeDelta);
	virtual void Render_GameObject();

private:
	HRESULT Add_Component();
	void Key_Input(const _float& fTimeDelta);

private:
	void Advance(const _float& fTimeDelta);
	void Jump(const _float& fTimeDelta);
	void Falling();
	void Check_Collect();
	void Check_WallCollision();

public:
	static CJSPlayer* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
	CJSCubeTex* m_pBufferCom;
	CTransform* m_pTransformCom;
	CTexture* m_pTextureCom;
	CJSCollider* m_pColliderCom;

	_float  m_fNextSpeedUpZ = 25.f;   // 처음 속도 증가 지점
	_float  m_fSpeedUpInterval = 25.f; // 속도 증가 간격
	_float  m_fSpeedUpAmount = 1.f;    // 한 번에 증가하는 속도
	_float  m_fMaxSpeed = 50.f;        // 최대 속도

	_float  m_fSpeed = 20.f;     // 전진 속도
	_float  m_fSideSpeed = 8.f;      // 좌우 속도
	_float  m_fJumpPower = 0.6f;     // 점프 초기 속도
	_float  m_fGravity = 2.f;     // 중력
	_float  m_fVelocityY = 0.f;      // 현재 Y 속도
	_bool   m_bJump = false;    // 점프 중 여부s
	_float  m_fGroundY = 2.f;      // 바닥 Y 위치
	_bool	m_bFalling = false;

	_bool	m_bRotated = false;
	_float  m_fTotalDistance = 0.f;  // 총 이동 거리 누적

private:
	virtual void Free();
};