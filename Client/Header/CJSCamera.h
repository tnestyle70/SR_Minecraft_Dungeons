#pragma once
#include "CCamera.h"

class CJSCamera : public CCamera
{
private:
	explicit CJSCamera(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CJSCamera(const CJSCamera& rhs);
	virtual ~CJSCamera();

public:
	HRESULT		Ready_GameObject(const _vec3* pEye,
		const _vec3* pAt,
		const _vec3* pUp,
		const _float& fFov,
		const _float& fAspect,
		const _float& fNear,
		const _float& fFar);

	virtual _int	Update_GameObject(const _float& fTimeDelta);
	virtual void	LateUpdate_GameObject(const _float& fTimeDelta);
	virtual void	Render_GameObject() {}

public:
	void		Start_Shake() { m_bShaking = true; m_fShakeTime = 0.f; }

private:
	void		Get_PlayerPos();
	void		Get_PlayerLook();

private:
	_vec3		m_vPlayerPos = { 0.f, 0.f, 0.f };
	_vec3		m_vPlayerLook =  { 0.f, 0.f, 1.f };
	_vec3		m_vCamLook = { 0.f, 0.f, -1.f };

	_bool   m_bShaking = false;
	_float  m_fShakeTime = 0.f;
	_float  m_fShakeMax = 0.5f;   // 쉐이킹 지속 시간
	_float  m_fShakeStrength = 3.f;  // 쉐이킹 강도

	_float m_fTargetEyeY = 8.f;   // 목표 Y
	_float m_fNormalEyeY = 8.f;   // 기본 Y
	_float m_fSlideEyeY = 3.f;   // 슬라이드 Y
	_bool m_bWasSliding = false;

public:
	static CJSCamera* Create(LPDIRECT3DDEVICE9 pGraphicDev,
		const _vec3* pEye,
		const _vec3* pAt,
		const _vec3* pUp,
		const _float& fFov = D3DXToRadian(60.f),
		const _float& fAspect = (_float)WINCX / WINCY,
		const _float& fNear = 0.1f,
		const _float& fFar = 1000.f);

private:
	virtual void		Free();
};

