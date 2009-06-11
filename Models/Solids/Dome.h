#ifndef __incl_Models_Solids_Dome_h
#define __incl_Models_Solids_Dome_h

#include "../../Math/Cameras/Camera.h"
#include "../../Graphics/Textures/TextureMgr.h"

namespace Models { namespace Solids {

    class CDome {
        bool        m_bIsValid;

        xFLOAT      m_fRadius;

        xFLOAT      m_fViewerElevation;
        xFLOAT      m_fSunElevation;
        xFLOAT      m_fSunRadius;
        xFLOAT      m_fSunGlow;
        xFLOAT      m_fSunGlowMult;
        xPoint3     m_pntSunLastPaint;
        xMatrix     m_mtxSunOrbit;
        xQuaternion m_qtrSunOrbit;

        struct xStar
        {
            xColor4b m_clrCurColor;
            xPoint3  m_pntPosition;

            xColor3b m_clrMinColor;
            xColor3b m_clrMaxColor;
            xFLOAT   m_fPhase;
            xFLOAT   m_fPhaseMult;

            void Random(xFLOAT fMaxZ, xFLOAT fMaxZ_Sqr);
        };

        xStar     * m_lstStars;
        xWORD       m_nStarsCountSmall;
        xWORD       m_nStarsCountMedium;
        xWORD       m_nStarsCountBig;

        xPoint3   * m_lstVertices;
        xColor3   * m_lstVerticesColors;
        xWORD       m_nVerticesCount;

        xWORD     * m_lstNorthernFan;
        xWORD       m_nNorthernFanSize;

        xWORD     * m_lstSouthernFan;
        xWORD       m_nSouthernFanSize;

        xWORD    ** m_lstParallelStrips;
        xWORD       m_nParallelStripsCount;
        xWORD       m_nParallelStripsSize;

        HTexture    m_hSun;

    public:

        enum xFaceCulling {
            FC_CULL_NOTHING ,
            FC_CULL_INSIDE  ,
            FC_CULL_OUTSIDE ,
        };

        xFaceCulling m_nFaceCulling;

        xPoint3      m_pntSun;
        xColor3      m_clrSun;

        xFLOAT       GetSunElevation() { return m_fSunElevation; }

        bool IsValid() { return m_bIsValid; }

        CDome() { Zero(); }

        void Zero() { memset(this, 0, sizeof(CDome)); }
        void Free();

        bool Create(xBYTE        nAngle         /*0-180*/,
                    xFLOAT       fRadius                 ,
                    xWORD        nLatitudeAngleStep  = 10,
                    xWORD        nLongitudeAngleStep = 10
                   );

        void Paint (xFLOAT fTimeDelta);

        void Update(xFLOAT fTimeDelta);

        void Render(const Math::Cameras::Camera &camera);
    };

} }

#endif