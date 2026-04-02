#include "pch.h"
#include "CJSBaseChunk.h"

CJSBaseChunk::CJSBaseChunk(LPDIRECT3DDEVICE9 pGraphicDev)
	: CGameObject(pGraphicDev)
{
}

CJSBaseChunk::~CJSBaseChunk()
{
}

void CJSBaseChunk::Free()
{
	CGameObject::Free();
}