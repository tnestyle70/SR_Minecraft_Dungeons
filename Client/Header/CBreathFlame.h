#pragma once
#include "CBase.h"
#include "Engine_Define.h"
#include "CCubeBodyTex.h"

class CBreathFlame : public CBase
{
	DECLARE_SINGLETON(CBreathFlame)

private:
	explicit CBreathFlame();
	virtual ~CBreathFlame();

public:
	void Activate(LPDIRECT3DDEVICE9 pDev, float fBeamRadius, float fBeamLength);
	void Deactivate();

	// Called every frame during BREATH: updates beam world matrix
	void Update(const _float& fTimeDelta,
		const _vec3& vHeadPos,
		const _vec3& vHeadDir);

	// Called during Render pass
	void Render();

	bool Is_Active() const { return m_bActive; }

	void Set_BeamLength(float fLength) { m_fBeamLength = fLength; }

private:
	HRESULT Create_BeamMesh();

private:
	LPDIRECT3DDEVICE9 m_pGraphicDev = nullptr;
	Engine::CCubeBodyTex* m_pBeamBuffer = nullptr;

	bool    m_bActive      = false;
	float   m_fBeamRadius  = 1.f;
	float   m_fBeamLength  = 50.f;
	//원본 크기 기록
	float m_fMeshDepth = 50.f;
	float   m_fGrowthTimer = 0.f;
	float   m_fTime        = 0.f;
	
	_matrix m_matBeamWorld;

public:
	virtual void Free() override;
};
