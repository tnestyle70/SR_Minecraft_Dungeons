#include "CUITexture.h"
#include "CUIMetadataMgr.h"

BEGIN(Engine)

CUITexture::CUITexture(LPDIRECT3DDEVICE9 pGraphicDev)
	: CComponent(pGraphicDev)
{
}

CUITexture::CUITexture(const CUITexture& rhs)
	: CComponent(rhs)
	, m_pTexture(rhs.m_pTexture)
	, m_strTextureKey(rhs.m_strTextureKey)
{
	if (m_pTexture)
		m_pTexture->AddRef();
}

CUITexture::~CUITexture()
{
	Free();
}

HRESULT CUITexture::Ready_Texture(const std::wstring& strFilePath, const std::wstring& strTextureKey)
{
	if (FAILED(D3DXCreateTextureFromFile(m_pGraphicDev, strFilePath.c_str(), &m_pTexture)))
		return E_FAIL;

	m_strTextureKey = strTextureKey;
	return S_OK;
}

void CUITexture::GetUVCoordinates(float& outMaxU, float& outMaxV) const
{
	const CUIMetadata* pMeta = CUIMetadataMgr::GetInstance()->Get_MetaData(m_strTextureKey);

	if (pMeta && pMeta->SizeX > 0.f && pMeta->SizeY > 0.f)
	{
		outMaxU = pMeta->ImportedSizeX / pMeta->SizeX;
		outMaxV = pMeta->ImportedSizeY / pMeta->SizeY;
	}
	else
	{
		outMaxU = 1.0f;
		outMaxV = 1.0f;
	}
}

void CUITexture::Render_Texture()
{
	if (m_pGraphicDev && m_pTexture)
		m_pGraphicDev->SetTexture(0, m_pTexture);
}

CUITexture* CUITexture::Create(LPDIRECT3DDEVICE9 pGraphicDev, const std::wstring& strFilePath, const std::wstring& strTextureKey)
{
	CUITexture* pInstance = new CUITexture(pGraphicDev);
	if (FAILED(pInstance->Ready_Texture(strFilePath, strTextureKey)))
	{
		Safe_Release(pInstance);
		return nullptr;
	}
	return pInstance;
}

CComponent* CUITexture::Clone()
{
	return new CUITexture(*this);
}

void CUITexture::Free()
{
	Safe_Release(m_pTexture);
	CComponent::Free();
}

END
