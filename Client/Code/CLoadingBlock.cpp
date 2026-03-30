#include "pch.h"
#include "CLoadingBlock.h"
#include "CRenderer.h"

CLoadingBlock::CLoadingBlock(LPDIRECT3DDEVICE9 pGraphicDev)
	: CGameObject(pGraphicDev)
{
	D3DXQuaternionIdentity(&m_qCurrent);
	D3DXQuaternionIdentity(&m_qFrom);
	D3DXQuaternionIdentity(&m_qTo);
}

CLoadingBlock::CLoadingBlock(const CGameObject& rhs)
	: CGameObject(rhs)
{}

CLoadingBlock::~CLoadingBlock()
{}

HRESULT CLoadingBlock::Ready_GameObject()
{
	if (FAILED(Add_Component()))
		return E_FAIL;

	// 블럭이 화면에 보이도록 위치, 스케일 설정
	m_pTransformCom->m_vScale = { 1.f, 1.f, 1.f };
	//m_pTransformCom->Set_Pos(1.f, 1.f, 1.f);
	
	return S_OK;
}

_int CLoadingBlock::Update_GameObject(const _float& fTimeDelta)
{
	_int iExit = CGameObject::Update_GameObject(fTimeDelta);

	CRenderer::GetInstance()->Add_RenderGroup(RENDER_PRIORITY, this);

	Rotate_Block(fTimeDelta);
	
	return iExit;
}

void CLoadingBlock::LateUpdate_GameObject(const _float& fTimeDelta)
{
	CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CLoadingBlock::Render_GameObject()
{
	m_pGraphicDev->SetTransform(D3DTS_WORLD, m_pTransformCom->Get_World());

	//Z Write 끄기!
	//m_pGraphicDev->SetRenderState(D3DRS_ZENABLE, FALSE);
	//m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

	m_pTextureCom->Set_Texture(0);

	m_pBufferCom->Render_Buffer();

	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	//Z Write 설정 복구
	//m_pGraphicDev->SetRenderState(D3DRS_ZENABLE, TRUE);
	//m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

	m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
}

void CLoadingBlock::Rotate_Block(const _float& fTimeDelta)
{
	if (m_bRotating)
	{
		m_fBlendAcc += fTimeDelta;
		float t = m_fBlendAcc / m_fBlendTime;

		if (t >= 1.f)
		{
			t = 1.f;
			m_bRotating = false;
			m_fIdleAcc = 0.f;

			// 목표값을 현재값으로 확정 (오차 누적 방지)
			m_qCurrent = m_qTo;
		}
		else
		{
			// EaseInOut 커브로 딱딱한 느낌 연출
			t = t * t * (3.f - 2.f * t);  // smoothstep
		}

		D3DXQuaternionSlerp(&m_qCurrent, &m_qFrom, &m_qTo, t);
	}
	else
	{
		// 짧게 멈췄다가 다음 회전 시작
		m_fIdleAcc += fTimeDelta;
		if (m_fIdleAcc >= m_fIdleDelay)
			Start_NextRotation();
	}

	Apply_Rotation();
}

void CLoadingBlock::Start_NextRotation()
{
	// 현재 상태를 시작점으로
	m_qFrom = m_qCurrent;
	m_fBlendAcc = 0.f;
	m_bRotating = true;

	D3DXQUATERNION qStep;
	float fAngle = D3DXToRadian(90.f);

	//위, 왼쪽, 위, 오른쪽
	switch (m_iRotStep % 4)
	{
		//위로 굴리기 
	case 0:
	case 2:
		D3DXQuaternionRotationAxis(&qStep, &D3DXVECTOR3(1, 0, 0), fAngle); 
		break;
		//왼쪽으로 돌리기
	case 1: D3DXQuaternionRotationAxis(&qStep, &D3DXVECTOR3(0, 1, 0), fAngle); 
		break;
		//오른쪽으로 돌리기
	case 3: D3DXQuaternionRotationAxis(&qStep, &D3DXVECTOR3(0, 1, 0), -fAngle);
		break;
	}

	// 목표 = 현재 누적 회전에 이번 스텝 추가
	D3DXQuaternionMultiply(&m_qTo, &m_qFrom, &qStep);
	D3DXQuaternionNormalize(&m_qTo, &m_qTo);

	//다음 스텝
	m_iRotStep++;
}

void CLoadingBlock::Apply_Rotation()
{
	D3DXMATRIX matRot, matScale, matTrans, matWorld;

	// 쿼터니언 → 회전 행렬
	D3DXMatrixRotationQuaternion(&matRot, &m_qCurrent);

	D3DXMatrixScaling(&matScale, 0.25f, 0.25f, 0.25f);
	D3DXMatrixTranslation(&matTrans, -1.5f, -0.8f, 0.5f);
	
	matWorld = matScale * matRot * matTrans;
	m_pTransformCom->Set_World(&matWorld);
}

HRESULT CLoadingBlock::Add_Component()
{
	CComponent* pComponent = nullptr;

	pComponent = m_pBufferCom = dynamic_cast<CCubeTex*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_CubeTex"));

	if (!pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_Buffer", pComponent });

	// Transform
	pComponent = m_pTransformCom = dynamic_cast<CTransform*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));

	if (nullptr == pComponent)
		return E_FAIL;

	m_mapComponent[ID_DYNAMIC].insert({ L"Com_Transform", pComponent });

	// Texture
	pComponent = m_pTextureCom = dynamic_cast<CTexture*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_LoadingBlockTexture"));

	if (nullptr == pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_Texture", pComponent });

	return S_OK;
}

CLoadingBlock* CLoadingBlock::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
	CLoadingBlock* pBlock = new CLoadingBlock(pGraphicDev);

	if (FAILED(pBlock->Ready_GameObject()))
	{
		Safe_Release(pBlock);
		MSG_BOX("Block Create Failed");
		return nullptr;
	}

	return pBlock;
}

void CLoadingBlock::Free()
{
	CGameObject::Free();
}
