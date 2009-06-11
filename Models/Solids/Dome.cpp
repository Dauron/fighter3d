#include "Dome.h"
#include "../../Graphics/OGL/ogl.h"
#include "../../Utils/Stat.h"

using namespace Models::Solids;

void CDome::xStar::Random(xFLOAT fMaxZ, xFLOAT fMaxZ_Sqr)
{
    xFLOAT fLongitudeAngle = randf() * 2.f * PI;
    xFLOAT fLatitudeAngle  = randf() * PI - PI * 0.5f;

    xFLOAT fZ = fMaxZ * sin(fLatitudeAngle);
    xFLOAT fX = sqrt(fMaxZ_Sqr - fZ * fZ);

    m_pntPosition.init(fX*cos(fLongitudeAngle), fX*sin(fLongitudeAngle), fZ);
    m_clrMinColor.init(100 + rand() % 106, 100 + rand() % 106, 50 + rand() % 106);
    m_clrMaxColor.init(150 + rand() % 106, 150 + rand() % 106, 50 + rand() % 206);
    m_clrCurColor.init(m_clrMinColor, 128);

    m_fPhase     = randf();
    m_fPhaseMult = randf() * 10.f - 5.f;
}


void CDome::Free()
{
    m_bIsValid = false;

    SAFE_DELETE_ARRAY_IF(m_lstStars);

    SAFE_DELETE_ARRAY_IF(m_lstVertices);
    SAFE_DELETE_ARRAY_IF(m_lstVerticesColors);
    SAFE_DELETE_ARRAY_IF(m_lstNorthernFan);
    SAFE_DELETE_ARRAY_IF(m_lstSouthernFan);

    if (m_lstParallelStrips)
    {
        for (int i = 0; i < m_nParallelStripsCount; ++i)
        {
            if (m_lstParallelStrips[i])
                delete[] m_lstParallelStrips[i];
            else
                break;
        }
        delete[] m_lstParallelStrips;
        m_lstParallelStrips = 0;
    }

    g_TextureMgr.Release(m_hSun);
}

bool CDome::Create(xBYTE        nAngle              /*0-180*/,
                   xFLOAT       fRadius                      ,
                   xWORD        nLatitudeAngleStep  /*= 10*/ ,
                   xWORD        nLongitudeAngleStep /*= 10*/
                  )
{
    Free();

    m_fRadius = fRadius;
    const xFLOAT fMaxZ     = fRadius;
    const xFLOAT fMaxZ_Sqr = fMaxZ * fMaxZ;

    m_fViewerElevation     = m_fRadius * 0.666f;

    //........................... Init sun, orbit and stars
    xVector3 vecOrbitAxis; vecOrbitAxis.init(3, 0, 1).normalize();

    m_qtrSunOrbit.init(vecOrbitAxis * sin(0.01), cos(0.01));
    m_mtxSunOrbit.identity();
    m_mtxSunOrbit *= xMatrixFromQuaternionT( xQuaternion::Create(vecOrbitAxis * sin(-0.51), cos(-0.51)) );

    m_pntSun.init(0.f, m_fRadius, 0.f);
    m_pntSun = m_mtxSunOrbit.postTransformP(m_pntSun);

    m_fSunRadius   = m_fRadius * 0.05f;
    m_fSunGlow     = 0.5f;
    m_fSunGlowMult = 1.f;

    m_nStarsCountSmall  = static_cast<xWORD>( m_fRadius ) << 1;
    m_nStarsCountMedium = static_cast<xSHORT>( sqrtf(static_cast<xFLOAT>( m_nStarsCountSmall )) );
    m_nStarsCountBig    = 5 + static_cast<xSHORT>( sqrtf(static_cast<xFLOAT>( m_nStarsCountMedium )) );
    m_lstStars          = new xStar [m_nStarsCountSmall + m_nStarsCountMedium + m_nStarsCountBig];

    if (!m_lstStars) { Free(); return false; }

    xStar *STR_iter = m_lstStars,
          *STR_last = m_lstStars + m_nStarsCountSmall + m_nStarsCountMedium + m_nStarsCountBig;
    for (; STR_iter != STR_last; ++STR_iter)
        STR_iter->Random(fMaxZ, fMaxZ_Sqr);

    //........................... Calculate vertex density
    if (nAngle < nLatitudeAngleStep)
        nAngle = nLatitudeAngleStep;
    if (nAngle > 180) nAngle = 180;

    const xWORD  nParallels          = nAngle / nLatitudeAngleStep;
    const xFLOAT fLatitudeAngleStep  = DegToRad(nLatitudeAngleStep);

    const xWORD  nMeridians          = (360 / nLongitudeAngleStep);
    const xFLOAT fLongitudeAngleStep = DegToRad(nLongitudeAngleStep);

    //........................... Make place for vertices and their colors
    m_nVerticesCount    = nMeridians * nParallels + 2;
    m_lstVertices       = new xPoint3 [m_nVerticesCount];
    m_lstVerticesColors = new xColor3 [m_nVerticesCount];

    if (!m_lstVertices || !m_lstVerticesColors) { Free(); return false; }

    xPoint3 *GPS_iter = m_lstVertices;

    //........................... Fill the northern pole with vertices
    GPS_iter->init(0,0, fMaxZ);
    ++GPS_iter;

    //........................... Fill the parallels with vertices
    xFLOAT fLatitudeAngle = PI * 0.5f - fLatitudeAngleStep;
    for (int nParallel = 1; nParallel <= nParallels;
         ++nParallel, fLatitudeAngle -= fLatitudeAngleStep)
    {
        xFLOAT fSinLatitude = sin(fLatitudeAngle);
        xFLOAT fZ = fMaxZ * fSinLatitude;
        xFLOAT fX = sqrt(fMaxZ_Sqr - fZ * fZ);

        xFLOAT fLongitudeAngle = 0.f;
        for (int nMeridian = 1; nMeridian <= nMeridians;
             ++nMeridian, fLongitudeAngle += fLongitudeAngleStep,
             ++GPS_iter)
        {
            GPS_iter->z = fZ;
            GPS_iter->x = fX*cos(fLongitudeAngle);
            GPS_iter->y = fX*sin(fLongitudeAngle);
        }
    }

    //........................... Fill northern fan
    m_nNorthernFanSize = nMeridians + 2;
    m_lstNorthernFan   = new xWORD[m_nNorthernFanSize];

    if (!m_lstNorthernFan) { Free(); return false; }

    m_lstNorthernFan[0]              = 0;
    m_lstNorthernFan[nMeridians + 1] = 1;

    xWORD *IDX_iter = m_lstNorthernFan + 1;
    for (int nMeridian = 1; nMeridian <= nMeridians; ++nMeridian, ++IDX_iter)
        *IDX_iter = nMeridian;

    //........................... Fill parrallel strips
    m_nParallelStripsCount = nParallels - 1;
    m_nParallelStripsSize  = nMeridians * 2 + 2;
    m_lstParallelStrips    = new xWORD*[m_nParallelStripsCount];

    if (!m_lstParallelStrips) { Free(); return false; }
    // Zero mem, in case we would have to Free noninitialized subarrays
    memset(m_lstParallelStrips, 0, sizeof(xWORD**)*m_nParallelStripsCount);

    xWORD nFirstVertex = 1;
    for (int nParallel = 0; nParallel < m_nParallelStripsCount; ++nParallel)
    {
        xWORD *lstStrip = m_lstParallelStrips[nParallel] = new xWORD[m_nParallelStripsSize];

        if (!lstStrip) { Free(); return false; }

        lstStrip[m_nParallelStripsSize-2] = nFirstVertex;
        lstStrip[m_nParallelStripsSize-1] = nFirstVertex + nMeridians;

        IDX_iter = lstStrip-1;
        for (int nMeridian = 1; nMeridian <= nMeridians; ++nMeridian, ++nFirstVertex)
        {
            *(++IDX_iter) = nFirstVertex;
            *(++IDX_iter) = nFirstVertex + nMeridians;
        }
    }

    if (nAngle > 170)
    {
        //........................... Fill the southern pole with vertices
        GPS_iter->init(0,0, -fMaxZ);

        //........................... Fill southern fan
        m_nSouthernFanSize = nMeridians + 2;
        m_lstSouthernFan   = new xWORD[m_nSouthernFanSize];

        if (!m_lstSouthernFan) { Free(); return false; }

        m_lstSouthernFan[0]              = m_nVerticesCount - 1;
        m_lstSouthernFan[nMeridians + 1] = m_nVerticesCount - 2;

        IDX_iter = m_lstSouthernFan + 1;
        for (int nMeridian = 1; nMeridian <= nMeridians; ++nMeridian, ++IDX_iter)
            *IDX_iter = m_nVerticesCount - nMeridian - 1;
    }
    else
    {
        m_nSouthernFanSize = 0;
        m_lstSouthernFan   = NULL;
    }

    m_bIsValid = true;
    return true;

    Paint(0.f);
}

void CDome::Paint(xFLOAT fTimeDelta)
{
    if (!m_bIsValid) { LOG(0, "CDome::Paint() # The dome is not Created"); return; }

    m_pntSunLastPaint = m_pntSun;

    xPoint3 pntSunOnDome = m_pntSun * m_fRadius / m_pntSun.length();

    //.......................... Select colors for vertices near and far from the sun
    xColor3 clrClose;
    xColor3 clrFar;

    xFLOAT fAtmosphereHeight   = (m_fRadius - m_fViewerElevation);
    xFLOAT fElevation          = (pntSunOnDome.z - m_fViewerElevation) / fAtmosphereHeight;
    if (fElevation < -1.f) fElevation = -1.f;
    else
    if (fElevation >  1.f) fElevation =  1.f;

    m_fSunElevation     = fElevation;

    xFLOAT fSunRadiusElevation = m_fSunElevation + m_fSunRadius  / fAtmosphereHeight;

    if (fSunRadiusElevation > 0.6f)
    {
        m_clrSun.r = 245 / 255.f;
        m_clrSun.g = 205 / 255.f;
        m_clrSun.b = 80  / 255.f;
    }
    else
    if (fSunRadiusElevation > 0.f)
    {
        m_clrSun.r = 120 / 255.f + (fSunRadiusElevation) / 0.6f * 125 / 255.f;
        m_clrSun.g = 80  / 255.f + (fSunRadiusElevation) / 0.6f * 125 / 255.f;
        m_clrSun.b = 0   / 255.f + (fSunRadiusElevation) / 0.6f * 80  / 255.f;
    }
    else
    {
        m_clrSun.g = 80  / 255.f + (fSunRadiusElevation) * 80  / 255.f;
        m_clrSun.b =             - (fSunRadiusElevation) * 80  / 255.f;
        m_clrSun.r = 120 / 255.f + (fSunRadiusElevation) * 120 / 255.f;
    }

    if (fElevation >= -0.1)
        clrClose.r = 235 / 255.f - (fElevation+0.1) * 85  / 255.f;
    else
        clrClose.r = 235 / 255.f + (fElevation+0.1) * 170 / 255.f;
    clrClose.g = 150 / 255.f + fElevation * 85 / 255.f;
    clrClose.b = 180 / 255.f + fElevation * 85 / 255.f;

    if (fElevation < 0.f) fElevation = 0.f;
    clrFar.r = 20 / 255.f + fElevation * 215 / 255.f;
    clrFar.g = 20 / 255.f + fElevation * 215 / 255.f;
    clrFar.b = 50 / 255.f + fElevation * 215 / 255.f;

    //.......................... Paint vertices according to their distance from the sun
    xFLOAT fChord     = m_fRadius * 2;
    xFLOAT fChord_Sqr = fChord * fChord;

    xPoint3  *GPS_iter = m_lstVertices,
             *GPS_last = m_lstVertices + m_nVerticesCount;
    xColor3  *CLR_iter = m_lstVerticesColors;

    xFLOAT fDistanceToSun;
    for (; GPS_iter != GPS_last; ++GPS_iter, ++CLR_iter)
    {
        fDistanceToSun  = (pntSunOnDome - *GPS_iter).lengthSqr() / fChord_Sqr;
        fDistanceToSun  = 1.f - fDistanceToSun;
        fDistanceToSun  = fDistanceToSun * fDistanceToSun;

        *CLR_iter = (1.f-fDistanceToSun) * clrFar + fDistanceToSun * clrClose;
    }
}

void CDome::Update(xFLOAT fTimeDelta)
{
    if (!m_bIsValid) { LOG(0, "CDome::Update() # The dome is not Created"); return; }

    Profile("Update sky box");

    xQuaternion qtrOrbitRot = xQuaternion::Interpolate(m_qtrSunOrbit, fTimeDelta);
    m_pntSun = qtrOrbitRot.rotate(m_pntSun);

    m_mtxSunOrbit *= xMatrixFromQuaternionT(qtrOrbitRot);

    m_fSunGlow += m_fSunGlowMult * fTimeDelta * 0.5f;
    if (m_fSunGlow > 0.8f) { m_fSunGlow = 0.8f; m_fSunGlowMult = -fabs(m_fSunGlowMult); }
    else
    if (m_fSunGlow < 0.6f) { m_fSunGlow = 0.6f; m_fSunGlowMult =  fabs(m_fSunGlowMult); }

    if ((m_pntSunLastPaint - m_pntSun).lengthSqr() > m_fRadius * 0.002f)
        Paint(fTimeDelta);

    //========================== Color stars
    xFLOAT fElevation = m_fSunElevation + 0.3f;
    if (fElevation > 1.f) fElevation = 1.f;
    else
    if (fElevation < 0.f) fElevation = 0.f;

    xBYTE bElevationAlpha = 255 - static_cast<xBYTE>( fElevation * 255 );

    xStar *STR_iter = m_lstStars,
          *STR_last = m_lstStars + m_nStarsCountSmall + m_nStarsCountMedium + m_nStarsCountBig;
    for (; STR_iter != STR_last; ++STR_iter)
    {
        STR_iter->m_fPhase += STR_iter->m_fPhaseMult * fTimeDelta;
        if (STR_iter->m_fPhase > 1.f) { STR_iter->m_fPhase = 1.f; STR_iter->m_fPhaseMult = - fabs(STR_iter->m_fPhaseMult); }
        if (STR_iter->m_fPhase < 0.f) { STR_iter->m_fPhase = 0.f; STR_iter->m_fPhaseMult =   fabs(STR_iter->m_fPhaseMult); }

        STR_iter->m_clrCurColor.color3b =
            STR_iter->m_clrMinColor * STR_iter->m_fPhase +
            STR_iter->m_clrMaxColor * (1.f - STR_iter->m_fPhase);
        STR_iter->m_clrCurColor.a = bElevationAlpha;
    }
}

void CDome::Render(const Math::Cameras::Camera &camera)
{
    if (!m_bIsValid) { LOG(0, "CDome::Render() # The dome is not Created"); return; }

    Profile("Render sky box");

    glPushAttrib(GL_POLYGON_BIT | GL_POINT_BIT);
    glPushMatrix();
    {
        glTranslatef(0.f, 0.f, -m_fViewerElevation);

        if (m_nFaceCulling == FC_CULL_NOTHING)
            glDisable(GL_CULL_FACE);
        else
        {
            glEnable   ( GL_CULL_FACE );
            glFrontFace( m_nFaceCulling != FC_CULL_OUTSIDE ? GL_CCW : GL_CW );
            glCullFace ( GL_BACK );
        }
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);

        //========================== Draw the sky dome
        glVertexPointer(3, GL_FLOAT, sizeof(xPoint3), m_lstVertices);
        glColorPointer (3, GL_FLOAT, sizeof(xColor3), m_lstVerticesColors);

        glDrawElements(GL_TRIANGLE_FAN, m_nNorthernFanSize, GL_UNSIGNED_SHORT, m_lstNorthernFan);
        if (m_lstSouthernFan)
            glDrawElements(GL_TRIANGLE_FAN, m_nSouthernFanSize, GL_UNSIGNED_SHORT, m_lstSouthernFan);

        for (int nParallel = 0; nParallel < m_nParallelStripsCount; ++nParallel)
            glDrawElements(GL_TRIANGLE_STRIP, m_nParallelStripsSize, GL_UNSIGNED_SHORT, m_lstParallelStrips[nParallel]);

        //========================== Draw the stars
        glEnable    (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable    (GL_POINT_SMOOTH);

        glPointSize (1.f);
        glInterleavedArrays(GL_C4UB_V3F, sizeof(xStar), m_lstStars);
        glDrawArrays(GL_POINTS, 0, m_nStarsCountSmall);
        glPointSize (2.f);
        glDrawArrays(GL_POINTS, m_nStarsCountSmall, m_nStarsCountMedium);
        glPointSize (3.f);
        glDrawArrays(GL_POINTS, m_nStarsCountSmall + m_nStarsCountMedium, m_nStarsCountBig);

        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);

        //========================== Draw the sun
        if (m_hSun.IsNull())
            m_hSun = g_TextureMgr.GetTexture("Data/textures/sun2.tga");

        g_TextureMgr.BindTexture(m_hSun);
        glEnable(GL_TEXTURE_2D);

        //_____________ Calculate perspective correct sun quad coordinates
        xVector3 vecFront = (camera.P_center - camera.P_eye).normalize();
        xVector3 vecUp    = m_mtxSunOrbit.preTransformP(xVector3::Create(0,0,1));
        xVector3 vecSide  = xVector3::CrossProduct(vecFront, vecUp);
        if (vecSide.isZero())
        {
            vecUp   = xVector3::Normalize(camera.NW_up);
            vecSide = xVector3::CrossProduct(vecFront, vecUp);
            vecSide.z = 0.f;
            vecSide.normalize();
        }
        vecUp    = xVector3::CrossProduct(vecSide, vecFront) * m_fSunRadius;
        vecSide *= m_fSunRadius;

        //_____________ Render the sun
        glBegin(GL_QUADS);
        {
            // Render sun
            glColor3ub(255,255,255);
            glTexCoord2f(1,0); glVertex3fv( (m_pntSun + vecSide - vecUp).xyz );
            glTexCoord2f(0,0); glVertex3fv( (m_pntSun - vecSide - vecUp).xyz );
            glTexCoord2f(0,1); glVertex3fv( (m_pntSun - vecSide + vecUp).xyz );
            glTexCoord2f(1,1); glVertex3fv( (m_pntSun + vecSide + vecUp).xyz );

            // Render glow
            vecUp   *= 2;
            vecSide *= 2;
            glColor4f(1.f,1.f,1.f, m_fSunGlow);
            glTexCoord2f(1,0); glVertex3fv( (m_pntSun + vecSide - vecUp).xyz );
            glTexCoord2f(0,0); glVertex3fv( (m_pntSun - vecSide - vecUp).xyz );
            glTexCoord2f(0,1); glVertex3fv( (m_pntSun - vecSide + vecUp).xyz );
            glTexCoord2f(1,1); glVertex3fv( (m_pntSun + vecSide + vecUp).xyz );
        }
        glEnd();

        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
    }
    glPopMatrix();
    glPopAttrib();
}