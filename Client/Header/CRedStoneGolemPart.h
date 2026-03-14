#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"

enum REDSTONEGOLEM_PART
{
	GOLEM_HEAD,
	GOLEM_BODY,
	GOLEM_CORE,
	GOLEM_LSHOULDER,
	GOLEM_RSHOULDER,
	GOLEM_LARM,
	GOLEM_RARM,
	GOLEM_HIP,
	GOLEM_LLEG,
	GOLEM_RLEG,

	GOLEM_END
};

class CRedStoneGolemPart : public CGameObject
{
private:
	explicit CRedStoneGolemPart(LPDIRECT3DDEVICE9 pGrpahicDev, REDSTONEGOLEM_PART ePart);
	explicit CRedStoneGolemPart(const CRedStoneGolemPart& rhs);
	virtual ~CRedStoneGolemPart();

public:
	virtual HRESULT Ready_GameObject();
	virtual _int Update_GameObject(const _float& fTimeDelta);
	virtual	void LateUpdate_GameObject(const _float& fTimeDelta);
	virtual void Render_GameObject();

public:
	void Set_Buffer(CVIBuffer* pBuffer) { m_pBufferCom = pBuffer; }
	void Set_Parent(CTransform* pParent) { m_pParentTransformCom = pParent; }
	void Set_LocalOffset(_vec3 vOffset) { m_vLocalOffset = vOffset; }
	CTransform* Get_Transform() { return m_pTransformCom; }

private:
	HRESULT Add_Component();

private:
	REDSTONEGOLEM_PART m_ePart;

	CVIBuffer* m_pBufferCom;
	CTransform* m_pTransformCom;
	CTransform* m_pParentTransformCom;
	_vec3 m_vLocalOffset;

public:
	static CRedStoneGolemPart* Create(LPDIRECT3DDEVICE9 pGraphicDev, REDSTONEGOLEM_PART ePart);
	
private:
	virtual void Free();
};