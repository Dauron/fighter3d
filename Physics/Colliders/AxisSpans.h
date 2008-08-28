#ifndef __incl_Physics_Colliders_AxisSpans_h
#define __incl_Physics_Colliders_AxisSpans_h

#include "../../Math/Figures/xIFigure3d.h"

namespace Physics { namespace Colliders {
    using namespace ::Math::Figures;

    struct AxisSpans
    {
        // based on http://www.gamedev.net/community/forums/topic.asp?topic_id=193500

        xVector3 N_axis;
        xFLOAT   P_min1, P_max1;
        xFLOAT   P_min2, P_max2;
        bool     wrong;

        AxisSpans &init(const xVector3 &N_axis)
        {
            this->N_axis = N_axis;
            if (!(this->wrong = N_axis.lengthSqr() < EPSILON2))
                this->N_axis.normalize();
            return *this;
        }

        static xVector3 GetMinimalFix(const AxisSpans *spans, int I_spans, int &I_bestSpan)
        {
            xFLOAT   W_dist_min = xFLOAT_HUGE_POSITIVE;
            xFLOAT   S_dist_min = 0.f;

            for (int i=I_spans; i; --i, ++spans)
            {
                if (spans->wrong) continue;

                xFLOAT S_dist1 = spans->P_max2 - spans->P_min1;
                xFLOAT S_dist2 = spans->P_max1 - spans->P_min2;
                xFLOAT W_dist, W_scale = 1.f/spans->N_axis.lengthSqr();
                xFLOAT S_dist = (S_dist1 > S_dist2) ? -(W_dist = S_dist2) : (W_dist = S_dist1);
                W_dist = W_scale * W_dist;

                if (W_dist < W_dist_min)
                {
                    W_dist_min  = W_dist;
                    S_dist_min  = S_dist * W_scale;
                    I_bestSpan  = i;
                }
            }

            I_bestSpan = I_spans - I_bestSpan;
            return spans[-I_spans+I_bestSpan].N_axis * S_dist_min;
        }

        static bool AxisNotOverlap(const xIFigure3d &figure1, const xIFigure3d &figure2,
                                   AxisSpans &spans, int axis = -1)
        {
            if (spans.wrong) return false;
            figure1.ComputeSpan(spans.N_axis, spans.P_min1, spans.P_max1, axis);
            figure2.ComputeSpan(spans.N_axis, spans.P_min2, spans.P_max2);
            return spans.P_min1 + EPSILON2 > spans.P_max2 || spans.P_max1 < spans.P_min2 + EPSILON2;
        }

        static bool AxisNotOverlapClose(const xIFigure3d &figure1, const xIFigure3d &figure2,
                                        AxisSpans &spans, int axis = -1)
        {
            if (spans.wrong) return false;
            figure1.ComputeSpan(spans.N_axis, spans.P_min1, spans.P_max1, axis);
            figure2.ComputeSpan(spans.N_axis, spans.P_min2, spans.P_max2);
            return spans.P_min1 > spans.P_max2 + EPSILON2 || spans.P_max1 + EPSILON2 < spans.P_min2;
        }

        static bool AxisNotOverlap(const xIFigure3d &figure1, const xPoint3 &P_A,
                                   const xPoint3 &P_B, const xPoint3 &P_C,
                                   AxisSpans &spans, int axis = -1)
        {
            if (spans.wrong) return false;
            figure1.ComputeSpan(spans.N_axis, spans.P_min1, spans.P_max1, axis);
            xTriangle::ComputeSpan(P_A,P_B,P_C,spans.N_axis, spans.P_min2, spans.P_max2);
            return spans.P_min1 + EPSILON2 > spans.P_max2 || spans.P_max1 < spans.P_min2 + EPSILON2;
        }

        static bool AxisNotOverlap(const xPoint3 &P_A, const xPoint3 &P_B, const xPoint3 &P_C,
                                   const xIFigure3d &figure2,
                                   AxisSpans &spans, int axis = -1)
        {
            if (spans.wrong) return false;
            xTriangle::ComputeSpan(P_A,P_B,P_C,spans.N_axis, spans.P_min2, spans.P_max2, axis);
            figure2.ComputeSpan(spans.N_axis, spans.P_min1, spans.P_max1);
            return spans.P_min1 + EPSILON2 > spans.P_max2 || spans.P_max1 < spans.P_min2 + EPSILON2;
        }
    };

} } // namespace Physics.Colliders

#endif
