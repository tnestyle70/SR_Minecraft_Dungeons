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

public:
	static CJSPlayer* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
	CJSCubeTex* m_pBufferCom;
	CTransform* m_pTransformCom;
	CTexture* m_pTextureCom;

	_float  m_fSpeed = 10.f;     // 전진 속도
	_float  m_fSideSpeed = 8.f;      // 좌우 속도
	_float  m_fJumpPower = 10.f;     // 점프 초기 속도
	_float  m_fGravity = 20.f;     // 중력
	_float  m_fVelocityY = 0.f;      // 현재 Y 속도
	_bool   m_bJump = false;    // 점프 중 여부
	_float  m_fGroundY = 1.f;      // 바닥 Y 위치

private:
	virtual void Free();
};