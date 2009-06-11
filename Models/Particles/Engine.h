#ifndef __incl_Models_Particles_Engine_h
#define __incl_Models_Particles_Engine_h

#include "../../Utils/Utils.h"
#include "Particle.h"
#include "../../Math/Cameras/Camera.h"
#include "../../Graphics/Textures/TextureMgr.h"

namespace Models { namespace Particles {

    class CEngine {

        bool       m_bTextured;

        SParticle *m_lstParticles;
        xWORD      m_nParticlesCount;
        xWORD      m_nParticlesValid;

        HTexture       m_hParticle;
        SParticleQuad *m_lstParticleQuads;
        xWORD         *m_lstParticleQuadsIndices;

        xWORD     *m_lstExistingParticleIndices;
        xWORD      m_nExistingParticleIndicesInUse;
        xWORD     *m_lstFreeParticleIndices;
        xWORD      m_nFreeParticleIndicesInUse;

        xVector3  *m_lstForces;
        xWORD      m_nForcesInUse;
        xWORD      m_nForcesCount;

        xFLOAT     m_fTimeDeltaFromLastParticle;

    public:

        SParticle  m_BaseParticle;
        xFLOAT     m_fGunSpreadAngle;
        xFLOAT     m_fTimeDeltaPerParticle;

        void ClearForces() { m_nForcesInUse = 0; }
        bool AddForce(const xVector3 &vecForce);

        CEngine()  { Zero(); }
        ~CEngine() { Free(); }

        void Zero() { memset(this, 0, sizeof(CEngine)); }
        void Free();

        bool Create(bool bTextured, xWORD nParticles);

        void Update(xFLOAT fTimeDelta);

        void Render(const Math::Cameras::Camera &camera);
    };

} }

#endif // __incl_Models_Particles_Engine_h