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

private:
	void		Key_Input(const _float& fTimeDelta);
	void		Mouse_Move();
	void		Mouse_Fix();
	void		Get_PlayerPos();

private:
	_float		m_fSpeed = 10.f;
	_bool		m_bFix;
	_bool		m_bCheck;

	_vec3		m_vPlayerPos = { 0.f, 0.f, 0.f };

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

