#include "Engine.h"
#include "../../Utils/Stat.h"
#include "../../Graphics/OGL/ogl.h"
#include "../../Utils/Debug.h"

using namespace Models::Particles;

void CEngine::Free()
{
    SAFE_DELETE_ARRAY_IF(m_lstForces);
    SAFE_DELETE_ARRAY_IF(m_lstParticles);
    SAFE_DELETE_ARRAY_IF(m_lstParticleQuads);
    SAFE_DELETE_ARRAY_IF(m_lstParticleQuadsIndices);
    SAFE_DELETE_ARRAY_IF(m_lstExistingParticleIndices);
    SAFE_DELETE_ARRAY_IF(m_lstFreeParticleIndices);

    g_TextureMgr.Release(m_hParticle);
}

bool CEngine::Create(bool bTextured, xWORD nParticles)
{
    Free();

    m_bTextured = bTextured;

    m_nForcesCount = 10;
    m_nForcesInUse = 1;
    m_lstForces    = new xVector3[m_nForcesCount];
    if (!m_lstForces)    { Free(); return false; }
    m_lstForces[0].init(0.f,0.f,-10.f);

    m_nParticlesCount  = nParticles;
    m_nParticlesValid  = 0;
    m_lstParticles     = new SParticle[m_nParticlesCount];
    if (!m_lstParticles)
    { Free(); return false; }

    if (m_bTextured)
    {
        m_lstParticleQuads        = new SParticleQuad[m_nParticlesCount];
        m_lstParticleQuadsIndices = new xWORD        [m_nParticlesCount*4];
        if (!m_lstParticleQuads || !m_lstParticleQuadsIndices)
        { Free(); return false; }
    }

    m_nExistingParticleIndicesInUse = 0;
    m_lstExistingParticleIndices = new xWORD[m_nParticlesCount];
    if (!m_lstExistingParticleIndices)
    { Free(); return false; }

    m_nFreeParticleIndicesInUse = 0;
    m_lstFreeParticleIndices = new xWORD[m_nParticlesCount];
    if (!m_lstFreeParticleIndices)
    { Free(); return false; }

    m_fTimeDeltaPerParticle      = 0.005f;
    m_fTimeDeltaFromLastParticle = 0.f;

    m_BaseParticle.m_bValid  = true;
    m_BaseParticle.m_fRadius = m_bTextured ? 0.05f : 5.f;
    m_BaseParticle.m_clrColor.init(200, 200, 200, 200);
    m_BaseParticle.m_fEnergy = 50.f;
    m_BaseParticle.m_fMass   = 0.5f;
    m_BaseParticle.m_pntPosition.init(0.f,0.f,0.f);
    m_BaseParticle.m_vecVelocity.init(0.f,0.f,8.f);

    m_fGunSpreadAngle = PI * 0.1f;

    return true;
}

bool CEngine::AddForce(const xVector3 &vecForce)
{
    if (m_nForcesInUse < m_nForcesCount)
    {
        m_lstForces[m_nForcesInUse] = vecForce;
        ++m_nForcesInUse;
        return true;
    }
    LOG(1, "CEngine::AddForce() # Max number of forces has been exceeded");
    return false;
}

void CEngine::Update(xFLOAT fTimeDelta)
{
    Profile("Update particle engine");

    m_fTimeDeltaFromLastParticle += fTimeDelta;

    xVector3 vecForce; vecForce.zero();

    xVector3 *FRC_curr = m_lstForces,
             *FRC_last = m_lstForces + m_nForcesInUse;
    for (; FRC_curr != FRC_last; ++FRC_curr)
        vecForce += *FRC_curr;

    xFLOAT fMaxEnergyInv   = 1.f / m_BaseParticle.m_fEnergy;
    xFLOAT fAlphaEnergyDep = m_BaseParticle.m_clrColor.a * fMaxEnergyInv;

    SParticle *PRT_curr = m_lstParticles,
              *PRT_last = m_lstParticles + m_nParticlesValid;
    xWORD      idx = 0;
    for (; PRT_curr != PRT_last; ++PRT_curr, ++idx)
    {
        if (PRT_curr->m_bValid)
        {
            PRT_curr->m_fEnergy -= fTimeDelta;

            if (PRT_curr->m_fEnergy > 0.f)
            {
                PRT_curr->m_clrColor.a   = static_cast<xBYTE>( fAlphaEnergyDep * PRT_curr->m_fEnergy );
                PRT_curr->m_vecVelocity += fTimeDelta * vecForce;
                PRT_curr->m_pntPosition += PRT_curr->m_vecVelocity * fTimeDelta;
                PRT_curr->m_fRadius = m_BaseParticle.m_fRadius + 0.5f * (1.f - fMaxEnergyInv * PRT_curr->m_fEnergy);
            }
            else
            {
                PRT_curr->m_bValid = false;

                // Remove from valid particles list
                for (int i = 0; i < m_nExistingParticleIndicesInUse; ++i)
                    if (m_lstExistingParticleIndices[i] == idx)
                    {
                        m_lstExistingParticleIndices[i] =
                            m_lstExistingParticleIndices[m_nExistingParticleIndicesInUse - 1];
                        --m_nExistingParticleIndicesInUse;
                        break;
                    }
                // Add to invalid particles list
                m_lstFreeParticleIndices[m_nFreeParticleIndicesInUse] = idx;
                ++m_nFreeParticleIndicesInUse;
            }
        }
    }

    if (m_fTimeDeltaFromLastParticle > m_fTimeDeltaPerParticle)
    {
        xWORD nParticlesToAdd        =
            static_cast<xWORD>( floorf( m_fTimeDeltaFromLastParticle / m_fTimeDeltaPerParticle ) );
        m_fTimeDeltaFromLastParticle = 0.f;

        if (m_nParticlesValid - m_nFreeParticleIndicesInUse + nParticlesToAdd > m_nParticlesCount)
            nParticlesToAdd = m_nParticlesCount - m_nParticlesValid + m_nFreeParticleIndicesInUse;

        xVector3 vecParal = xVector3::Normalize(m_BaseParticle.m_vecVelocity);
        xVector3 vecOrtho = xVector3::Orthogonal(m_BaseParticle.m_vecVelocity);

        while (nParticlesToAdd)
        {
            if (m_nFreeParticleIndicesInUse)
            {
                --m_nFreeParticleIndicesInUse;
                m_lstExistingParticleIndices[m_nExistingParticleIndicesInUse]
                    = m_lstFreeParticleIndices[m_nFreeParticleIndicesInUse];
                ++m_nExistingParticleIndicesInUse;
                PRT_curr = m_lstParticles + m_lstFreeParticleIndices[m_nFreeParticleIndicesInUse];
            }
            else
            {
                m_lstExistingParticleIndices[m_nExistingParticleIndicesInUse]
                    = m_nParticlesValid;
                ++m_nExistingParticleIndicesInUse;
                PRT_curr = m_lstParticles + m_nParticlesValid;
                ++m_nParticlesValid;
            }

            *PRT_curr = m_BaseParticle;

            xFLOAT fAngle = 0.5f * m_fGunSpreadAngle * randf();
            if (fAngle > 0.001f)
            {
                xQuaternion quatRotate; quatRotate.init(vecOrtho * sin(fAngle), cos(fAngle));
                PRT_curr->m_vecVelocity = quatRotate.rotate(PRT_curr->m_vecVelocity);

                fAngle = 180 * randf();
                if (fAngle > 0.001f)
                {
                    quatRotate.init(vecParal * sin(fAngle), cos(fAngle));
                    PRT_curr->m_vecVelocity = quatRotate.rotate(PRT_curr->m_vecVelocity);
                }
            }

            --nParticlesToAdd;
        }
    }
}

void CEngine::Render(const Math::Cameras::Camera &camera)
{
    if (!m_lstParticles) { LOG(0, "CEngine::Render() # The particle engine is not Created"); return; }

    Profile("Render particle engine");

    if (!m_nExistingParticleIndicesInUse) return;

    glPushAttrib(GL_POLYGON_BIT | GL_POINT_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix();
    {
        glDisable (GL_CULL_FACE);
        glFrontFace  ( GL_CCW  );
        glCullFace   ( GL_BACK );
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        glDepthMask  ( GL_FALSE );

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);

        //========================== Draw the particles
        glEnable    (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        if (m_bTextured)
        {
            //_____________ Calculate perspective correct quad coordinates
            xVector3 vecFront = (camera.P_center - camera.P_eye).normalize();
            xVector3 vecUp    = xVector3::Create(0,0,1);
            xVector3 vecSide  = xVector3::CrossProduct(vecFront, vecUp);
            if (vecSide.isZero())
            {
                vecUp   = xVector3::Normalize(camera.NW_up);
                vecSide = xVector3::CrossProduct(vecFront, vecUp);
                vecSide.z = 0.f;
                vecSide.normalize();
            }
            vecUp    = xVector3::CrossProduct(vecSide, vecFront);

            SParticle     *PRT_curr = m_lstParticles,
                          *PRT_last = m_lstParticles + m_nParticlesValid;
            SParticleQuad *PRQ_curr = m_lstParticleQuads;
            xWORD         *IDX_curr = m_lstParticleQuadsIndices;
            xWORD          idx = 0;
            for (; PRT_curr != PRT_last; ++PRT_curr, ++PRQ_curr, idx += 4)
            {
                SParticle &particle = *PRT_curr;
                if (PRT_curr->m_bValid)
                {
                    PRQ_curr->lstCorners[0].m_texCoords.init(0,0);
                    PRQ_curr->lstCorners[0].m_colColor    = particle.m_clrColor;
                    PRQ_curr->lstCorners[0].m_pntPosition = particle.m_pntPosition - (vecUp + vecSide) * particle.m_fRadius;
                    PRQ_curr->lstCorners[1].m_texCoords.init(1,0);
                    PRQ_curr->lstCorners[1].m_colColor    = particle.m_clrColor;
                    PRQ_curr->lstCorners[1].m_pntPosition = particle.m_pntPosition - (vecUp - vecSide) * particle.m_fRadius;
                    PRQ_curr->lstCorners[2].m_texCoords.init(1,1);
                    PRQ_curr->lstCorners[2].m_colColor    = particle.m_clrColor;
                    PRQ_curr->lstCorners[2].m_pntPosition = particle.m_pntPosition + (vecUp + vecSide) * particle.m_fRadius;
                    PRQ_curr->lstCorners[3].m_texCoords.init(0,1);
                    PRQ_curr->lstCorners[3].m_colColor    = particle.m_clrColor;
                    PRQ_curr->lstCorners[3].m_pntPosition = particle.m_pntPosition + (vecUp - vecSide) * particle.m_fRadius;

                    IDX_curr[0] = idx;
                    IDX_curr[1] = idx+1;
                    IDX_curr[2] = idx+2;
                    IDX_curr[3] = idx+3;
                    IDX_curr += 4;
                }
            }

            if (m_hParticle.IsNull())
                m_hParticle = g_TextureMgr.GetTexture("Data/textures/particle.tga");

            g_TextureMgr.BindTexture(m_hParticle);
            glEnable(GL_TEXTURE_2D);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);

            glInterleavedArrays(GL_T2F_C4UB_V3F, sizeof(SVertexTex), m_lstParticleQuads);
            glColor3ub(255,255,255);
            glDrawElements(GL_QUADS, m_nExistingParticleIndicesInUse*4, GL_UNSIGNED_SHORT, m_lstParticleQuadsIndices);

            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            glDisable(GL_TEXTURE_2D);
        }
        else
        {
            glEnable    (GL_POINT_SMOOTH);
            glPointSize (50.f);
            glInterleavedArrays(GL_C4UB_V3F, sizeof(SParticle), m_lstParticles);
            //glDrawArrays(GL_POINTS, 0, m_nParticlesValid);
            glDrawElements(GL_POINTS, m_nExistingParticleIndicesInUse, GL_UNSIGNED_SHORT, m_lstExistingParticleIndices);
        }

        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);

        glDisable   (GL_BLEND);
    }
    glPopMatrix();
    glPopAttrib();
}