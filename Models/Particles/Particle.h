#ifndef __incl_Models_Particles_Particle_h
#define __incl_Models_Particles_Particle_h

#include "../../Math/xMath.h"

namespace Models { namespace Particles {

    struct SParticle {
        xColor4b    m_clrColor;    // must be 1st for open gl

        xPoint3     m_pntPosition; // must be 2nd for open gl
        xVector3    m_vecVelocity;

        xFLOAT      m_fRadius;

        xFLOAT      m_fEnergy;
        xFLOAT      m_fMass;

        bool        m_bValid;

        void Zero() { memset(this, 0, sizeof(SParticle)); }
    };

    struct SVertexTex
    {
        xTextUV    m_texCoords;
        xColor4b   m_colColor;
        xPoint3    m_pntPosition;
    };

    struct SParticleQuad
    {
        SVertexTex lstCorners[4];
    };
} }

#endif // __incl_Models_Particles_Particle_h