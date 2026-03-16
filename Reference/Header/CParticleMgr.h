#pragma once
#include "Engine_Define.h"
#include "CParticleEmitter.h"

BEGIN(Engine)

class ENGINE_DLL CParticleMgr : public CBase
{
    DECLARE_SINGLETON(CParticleMgr)

private:
    explicit CParticleMgr();
    ~CParticleMgr();

public:
    void    Add_Emitter(CParticleEmitter* pEmitter);

    void    Update(const _float& fTimeDelta);
    void    Render();

    _uint Get_EmitterCount() const { return (_uint)m_EmitterList.size(); }

    void    Clear_Emitters();

private:
    list<CParticleEmitter*>    m_EmitterList;

public:
    virtual void Free();
};

END
