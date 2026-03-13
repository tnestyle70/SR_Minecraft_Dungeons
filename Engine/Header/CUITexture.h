#ifndef CUITexture_h__
#define CUITexture_h__

#include "CComponent.h"

BEGIN(Engine)

class ENGINE_DLL CUITexture final : public CComponent
{
private:
	explicit CUITexture(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CUITexture(const CUITexture& rhs);
	virtual ~CUITexture();

public:
	HRESULT Ready_Texture(const std::wstring& strFilePath, const std::wstring& strTextureKey);
	void Render_Texture();

	// Returns the texture pointer for ImGui integration
	LPDIRECT3DTEXTURE9 Get_Texture() const { return m_pTexture; }

	// Core UV calculation
	void GetUVCoordinates(float& outMaxU, float& outMaxV) const;

public:
	static CUITexture* Create(LPDIRECT3DDEVICE9 pGraphicDev, const std::wstring& strFilePath, const std::wstring& strTextureKey);
	virtual CComponent* Clone() override;

private:
	LPDIRECT3DTEXTURE9  m_pTexture = nullptr;
	std::wstring        m_strTextureKey = L""; // Used to link with CUIMetadataMgr

public:
	virtual void Free() override;
};

END

#endif // CUITexture_h__
