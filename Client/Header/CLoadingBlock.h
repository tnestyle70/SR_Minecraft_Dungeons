#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"

enum eRotAxis {AXIS_X, AXIS_Y, AXIS_Z, AXIS_END};

class CLoadingBlock : public CGameObject
{
private:
	explicit CLoadingBlock(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CLoadingBlock(const CGameObject& rhs);
	virtual ~CLoadingBlock();

public:
	virtual			HRESULT		Ready_GameObject();
	virtual			_int		Update_GameObject(const _float& fTimeDelta);
	virtual			void		LateUpdate_GameObject(const _float& fTimeDelta);
	virtual			void		Render_GameObject();

private:
	HRESULT			Add_Component();

	void Rotate_Block(const _float& fTimeDelta);
	void Start_NextRotation();
	void Apply_Rotation();

private:
	Engine::CCubeTex* m_pBufferCom = nullptr;
	Engine::CTransform* m_pTransformCom = nullptr;
	Engine::CTexture* m_pTextureCom = nullptr;
	
	//현재 누적 회전
	D3DXQUATERNION m_qCurrent; //현재 실제 회전
	D3DXQUATERNION m_qFrom; //보간 시작점
	D3DXQUATERNION m_qTo; //보간 목표점

	float m_fBlendTime = 0.5f; //한 번 회전에 걸리는 시간
	float m_fBlendAcc = 0.f; //경과 시간 누적
	bool m_bRotating = false; //현재 회전 중인지 아닌지 판단
	float m_fIdleAcc = 0.15f; //멈춤 대기 시간 누적
	float m_fIdleDelay = 0.f;
	
	float m_fRotX, m_fRotY = 0.f;
	//회전 순서 기억(0 : 위 1 : 왼쪽, 2 : 위, 3: 오른쪽)
	int m_iRotStep = 0;

public:
	static CLoadingBlock* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
	virtual void Free();
};