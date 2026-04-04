#pragma once
#include "CCamera.h"
#include <deque>
#include "CTransform.h"

enum class eActionCamType
{
	SQUID_COAST,
	GB_STAGE,
	CAMTYPE_END
};

//Idea - deque에 waypoint를 저장하고 스테이지에서 위치값 뽑아내면서
//쭉 훑고 deque가 비면 캠 전환
struct CamWayPoint
{
	_vec3 vEye;
	_vec3 vAt;
	_vec3 vUp;
	_float fDuration;
};

class CDynamicCamera :  public CCamera
{
private:
	explicit CDynamicCamera(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CDynamicCamera(const CDynamicCamera& rhs);
	virtual ~CDynamicCamera();

public:
	HRESULT		Ready_GameObject(const _vec3* pEye,
								 const _vec3* pAt,
								 const _vec3* pUp,
								 const _float& fFov,
								 const _float & fAspect,
								 const _float & fNear,
								 const _float & fFar);

	virtual _int	Update_GameObject(const _float& fTimeDelta);
	virtual void	LateUpdate_GameObject(const _float& fTimeDelta);
	virtual void	Render_GameObject() {}

private:
	_int Update_ActionCam(const _float& fTimeDelta);

private:
	void		Key_Input(const _float& fTimeDelta);
	void		Mouse_Move();
	void		Mouse_Fix();
public:
	void SetActionCam(eActionCamType eType);
	bool IsActionCamFinished() { return m_bActionCam; }

	void SetFollowOffset(_vec3 vOffset) { m_vFollowOffset = vOffset; }

private:
	void Set_SquidCoastActionCam();
	void Set_GBStageActionCam();
	
private:
	_float		m_fSpeed;
	_bool		m_bFix;
	_bool		m_bCheck;
private: //Cam Way Point
	deque<CamWayPoint> m_deqWayPoints;
	CamWayPoint m_wpStart;
	CamWayPoint m_wpTarget;
	CamWayPoint m_wpEnd;
	bool m_bActionCam = false;

	eActionCamType m_eType = eActionCamType::CAMTYPE_END;
	
public:
	static CDynamicCamera* Create(LPDIRECT3DDEVICE9 pGraphicDev,
									const _vec3* pEye,
									const _vec3* pAt,
									const _vec3* pUp,
									const _float& fFov = D3DXToRadian(60.f),
									const _float& fAspect = (_float)WINCX / WINCY,
									const _float& fNear = 0.1f,
									const _float& fFar = 1000.f);

private:
	virtual void		Free();



//플레이어에게 카메라 고정
public:
	void SetFollowTarget(Engine::CTransform* pTransform) { m_pTargetTransform = pTransform; }

	void Set_DragonCam(bool bDragonCam) { m_bDragonCam = bDragonCam; }

private:
	Engine::CTransform* m_pTargetTransform = nullptr;

	_vec3 m_vFollowOffset = { -12.f, 20.f, -12.f };

	//드래곤 카메라 오프셋
	_vec3  m_vDragonOffset = { 0.f, 6.f, -15.f };
	bool m_bDragonCam = false;
	float m_fCamBlend = 0.f;

	//자유카메라 <-> 고정카메라
	bool m_bFollowMode = true;
	bool m_bF2Check = false;

public:
	//TJ스테이지 전용
	void Start_Shake(float fDuration, float fIntensity)
	{
		m_fShakeDuration = fDuration;
		m_fShakeTimer = 0.f;
		m_fShakeIntensity = fIntensity;
	}

private:
	_float m_fShakeDuration = 0.f;
	_float m_fShakeTimer = 0.f;
	_float m_fShakeIntensity = 0.f;
};

