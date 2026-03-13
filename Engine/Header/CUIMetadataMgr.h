#ifndef CUIMetadataMgr_h__
#define CUIMetadataMgr_h__

#include "Engine_Define.h"
#include "CBase.h"

BEGIN(Engine)

struct ENGINE_DLL CUIMetadata
{
	float ImportedSizeX;
	float ImportedSizeY;
	float SizeX;
	float SizeY;
};

class ENGINE_DLL CUIMetadataMgr : public CBase
{
	DECLARE_SINGLETON(CUIMetadataMgr)

private:
	explicit CUIMetadataMgr();
	virtual ~CUIMetadataMgr();

public:
	HRESULT Ready_MetaData(const std::wstring& strFilePath, const std::wstring& strTextureKey);
	const CUIMetadata* Get_MetaData(const std::wstring& strTextureKey);

private:
	unordered_map<std::wstring, CUIMetadata> m_mapMetaData;

public:
	virtual void Free() override;
};

END

#endif // CUIMetadataMgr_h__
