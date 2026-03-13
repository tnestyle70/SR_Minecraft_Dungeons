#include "CUIMetadataMgr.h"
#include <fstream>

// ��ũ�� �浹 ����
#pragma warning(push)
#pragma warning(disable: 4005)

#ifdef new
#undef new
#endif

#include "../Include/json.hpp" 

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#pragma warning(pop)

using json = nlohmann::json;

BEGIN(Engine)
IMPLEMENT_SINGLETON(CUIMetadataMgr)

CUIMetadataMgr::CUIMetadataMgr()
{
}

CUIMetadataMgr::~CUIMetadataMgr()
{
	Free();
}

HRESULT CUIMetadataMgr::Ready_MetaData(const std::wstring& strFilePath, const std::wstring& strTextureKey)
{
	string strPath(strFilePath.begin(), strFilePath.end());
	ifstream file(strPath);

	if (!file.is_open())
		return E_FAIL;

	try
	{
		json jArray;
		file >> jArray;

		// The JSON provided is an array
		if (!jArray.is_array())
			return E_FAIL;

		for (auto& item : jArray)
		{
			// Search for the Texture2D entry
			if (item.value("Type", "") == "Texture2D")
			{
				CUIMetadata metadata;

				// 1. Get ImportedSize (X, Y) from inside "Properties"
				if (item.contains("Properties") && item["Properties"].contains("ImportedSize"))
				{
					auto& importedSize = item["Properties"]["ImportedSize"];
					metadata.ImportedSizeX = importedSize.value("X", 0.0f);
					metadata.ImportedSizeY = importedSize.value("Y", 0.0f);
				}
				else
				{
					// Fallback to top-level size if ImportedSize is missing
					metadata.ImportedSizeX = (float)item.value("SizeX", 0);
					metadata.ImportedSizeY = (float)item.value("SizeY", 0);
				}

				// 2. Get the actual Texture size (SizeX, SizeY)
				metadata.SizeX = (float)item.value("SizeX", 0.0f);
				metadata.SizeY = (float)item.value("SizeY", 0.0f);

				m_mapMetaData[strTextureKey] = metadata;
				return S_OK;
			}
		}
	}
	catch (const json::exception& e)
	{
		return E_FAIL;
	}

	return E_FAIL; // No "Texture2D" type found in JSON array
}

const CUIMetadata* CUIMetadataMgr::Get_MetaData(const std::wstring& strTextureKey)
{
	auto iter = m_mapMetaData.find(strTextureKey);

	if (iter == m_mapMetaData.end())
		return nullptr;

	return &iter->second;
}

void CUIMetadataMgr::Free()
{
	m_mapMetaData.clear();
}
END
