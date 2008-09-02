#include "RendererGL.h"
#include "../GLAnimSkeletal.h"
#include "../../../Physics/Verlet/VConstraintLengthEql.h"
#include "../../../Physics/Verlet/VConstraintLengthMin.h"
#include "../../../Physics/Verlet/VConstraintLengthMax.h"
#include "../../../Physics/Verlet/VConstraintAngular.h"

// Bone / constraint view

void RenderBoneCore ( const xBone &bone )
{
    static xFLOAT4 W_bone = { 0.1f, 0.0f, 0.0f, 0.0f };

    xVector3 NW_fifth = (bone.P_end - bone.P_begin) * 0.2f;
    xVector3 P_fifth  = bone.P_begin + NW_fifth;
    xVector3 P_third  = P_fifth + xVector3::Orthogonal(NW_fifth).normalize() * 0.1f;

    xQuaternion QT_rot; QT_rot.init(NW_fifth.normalize() * sin(PI * 0.25f), cos(PI * 0.25f));

    W_bone[0] = bone.ID + 0.1f;
    g_AnimSkeletal.SetBoneIdxWghts(W_bone);

    for (int i=0; i<4; ++i) {
        g_AnimSkeletal.SetVertex(bone.P_end.xyz);
        g_AnimSkeletal.SetVertex(bone.P_begin.xyz);
        g_AnimSkeletal.SetVertex(P_third.xyz);
        P_third = QT_rot.rotate(P_third, P_fifth);
    }
}

void RenderBone ( const xBone *L_bones, xBYTE ID_bone, xWORD ID_selBone )
{
    const xBone &bone = L_bones[ID_bone];
    if (ID_bone)
    {
        if (ID_bone == ID_selBone)
        {
            glEnd();
            glColor4f(1.0f,0.0f,1.0f,1.0f);
            glBegin(GL_TRIANGLES);
        }

        RenderBoneCore(bone);

        if (ID_bone == ID_selBone) {
            glEnd();
            glColor4f(1.0f,1.0f,0.0f,1.0f);
            glBegin(GL_TRIANGLES);
        }
    }

    xBYTE *ID_iter = bone.ID_kids;
    for (int i = bone.I_kids; i; --i, ++ID_iter)
        RenderBone(L_bones, *ID_iter, ID_selBone);
}

xVector3 Ellipse(xFLOAT a, xFLOAT b, xFLOAT alpha)
{
    xFLOAT cosAlpha = cos(alpha);
    xFLOAT sinAlpha;
    xFLOAT gamma;

    if (cosAlpha > EPSILON)
    {
        xFLOAT tanAlpha = tan(alpha);
        sinAlpha = cosAlpha * tanAlpha;

        if (a > EPSILON && b > EPSILON)
        {
            xFLOAT beta = atan ( tanAlpha * a / b );
            xFLOAT cosBeta2 = cos(beta);
                   cosBeta2 *= cosBeta2;
            xFLOAT sinBeta2 = 1 - cosBeta2;
            gamma = sqrt( a*a*cosBeta2 + b*b*sinBeta2 );
        }
        else
            gamma = 0.f;
    }
    else
    {
        sinAlpha = 1.f;
        gamma    = a > EPSILON ? b : 0.f;
    }

    xFLOAT radius   = sin(gamma);
    return xVector3::Create(radius * 0.1f * cosAlpha, radius * 0.1f * sinAlpha, cos(gamma) * 0.1f);
}

void RenderConstraintAngular(const xSkeleton &spine, const xMatrix *MX_bones,
                             const xQuaternion *QT_bones, const VConstraintAngular &C_angular)
{
    const xBone
        &boneRB = spine.L_bones[C_angular.particleRootB],
        &boneRE = spine.L_bones[C_angular.particleRootE],
        &bone   = spine.L_bones[C_angular.particle];

    xVector3 P_rootB, P_rootE, P_current;
    P_rootB = MX_bones[boneRB.ID].postTransformP(boneRB.P_end);
    P_rootE = MX_bones[boneRE.ID].postTransformP(boneRE.P_end);

    xVector3 N_front;
    xVector3 N_up = (bone.P_end - bone.P_begin).normalize();
    if (C_angular.particleRootE)
    {
        N_up    = MX_bones[boneRE.ID].postTransformV(N_up);
        N_front = MX_bones[boneRE.ID].postTransformV(bone.getFront());
    }
    else
    {
        N_up    = QT_bones[boneRB.ID].rotate(N_up);
        N_front = QT_bones[boneRB.ID].rotate(bone.getFront());
    }

    xMatrix MX_BoneToWorld = xMatrixFromVectors(N_front, N_up).transpose();

    glBegin(GL_LINES);
    {
        // side axis - yellow
        glColor4f(1.0f,1.0f,0.0f,1.0f);
        xVector3 N_side = xVector3::CrossProduct(N_front, N_up).normalize();
        P_current = P_rootE + N_side * 0.1f;
        glVertex3fv(P_rootE.xyz);
        glVertex3fv(P_current.xyz);

        // up axis - green
        glColor4f(0.0f,0.6f,0.0f,1.0f);
        P_current = P_rootE + N_up * 0.1f;
        glVertex3fv(P_rootE.xyz);
        glVertex3fv(P_current.xyz);

        // front axis - blue
        glColor4f(0.0f,0.0f,1.0f,1.0f);
        P_current = P_rootE + xVector3::CrossProduct(N_side, N_up) * 0.1f;
        glVertex3fv(P_rootE.xyz);
        glVertex3fv(P_current.xyz);

        // current axis - aqua
        P_current = (bone.P_end - bone.P_begin).normalize() * 0.2f;
        P_current = P_rootE + MX_bones[bone.ID].postTransformV(P_current);
        if (C_angular.Test(P_rootB, P_rootE, P_current, N_up, N_front))
            glColor4f(1.0f,0.0f,0.0f,1.0f);
        else
            glColor4f(0.0f,1.0f,1.0f,1.0f);
        glVertex3fv(P_rootE.xyz);
        glVertex3fv(P_current.xyz);
    }
    glEnd();

    glColor4f(0.0f,1.0f,1.0f,1.0f);
    glBegin(GL_TRIANGLE_FAN);
    {
        glVertex3fv(P_rootE.xyz);

        xVector3 v;
        v.init(sin(C_angular.angleMaxX) * 0.1f, 0.f, cos(C_angular.angleMaxX) * 0.1);
        v = P_rootE + MX_BoneToWorld.postTransformV(v);
        glVertex3fv(v.xyz);

        int    stepsEnd = 10;
        xFLOAT stepsInv = 1.f / stepsEnd;
        for (int i = 1; i < stepsEnd; ++i)
        {
            xFLOAT alpha = i * stepsInv * 0.5f * PI;
            v = P_rootE + MX_BoneToWorld.postTransformV( Ellipse(C_angular.angleMaxX, C_angular.angleMaxY, alpha) );
            glVertex3fv(v.xyz);
        }

        v.init(0.f, sin(C_angular.angleMaxY) * 0.1f, cos(C_angular.angleMaxY) * 0.1);
        v = P_rootE + MX_BoneToWorld.postTransformV(v);
        glVertex3fv(v.xyz);

        for (int i = 1; i < stepsEnd; ++i)
        {
            xFLOAT alpha = i * stepsInv * 0.5f * PI;
            v = Ellipse(C_angular.angleMaxY, C_angular.angleMinX, alpha);
            xFLOAT swap = v.x; v.x = - v.y; v.y = swap;
            v = P_rootE + MX_BoneToWorld.postTransformV( v );
            glVertex3fv(v.xyz);
        }

        v.init(sin(C_angular.angleMinX) * -0.1f, 0.f, cos(C_angular.angleMinX) * 0.1);
        v = P_rootE + MX_BoneToWorld.postTransformV(v);
        glVertex3fv(v.xyz);

        for (int i = 1; i < stepsEnd; ++i)
        {
            xFLOAT alpha = i * stepsInv * 0.5f * PI;
            v = Ellipse(C_angular.angleMinX, C_angular.angleMinY, alpha);
            v.x = -v.x; v.y = -v.y;
            v = P_rootE + MX_BoneToWorld.postTransformV( v );
            glVertex3fv(v.xyz);
        }

        v.init(0.f, sin(C_angular.angleMinY) * -0.1f, cos(C_angular.angleMinY) * 0.1);
        v = P_rootE + MX_BoneToWorld.postTransformV(v);
        glVertex3fv(v.xyz);

        for (int i = 1; i < stepsEnd; ++i)
        {
            xFLOAT alpha = i * stepsInv * 0.5f * PI;
            v = Ellipse(C_angular.angleMinY, C_angular.angleMaxX, alpha);
            xFLOAT swap = v.x; v.x = v.y; v.y = -swap;
            v = P_rootE + MX_BoneToWorld.postTransformV( v );
            glVertex3fv(v.xyz);
        }

        glColor4f(0.0f,0.5f,0.5f,1.0f);
        v.init(sin(C_angular.angleMaxX) * 0.1f, 0.f, cos(C_angular.angleMaxX) * 0.1);
        v = P_rootE + MX_BoneToWorld.postTransformV(v);
        glVertex3fv(v.xyz);
    }
    glEnd();
}

void RenderConstraint ( const xSkeleton &spine, const xMatrix *MX_bones, const xQuaternion *QT_bones )
{
    const xBone *boneA, *boneB;
    xFLOAT minL, maxL;

    glPushAttrib(GL_LINE_BIT);
    glLineWidth(3.f);

    VConstraint **C_iter = spine.C_constraints;
    VConstraint **C_end  = C_iter + spine.I_constraints;
    for (; C_iter != C_end; ++C_iter)
    {
        if ((**C_iter).Type == VConstraint::Constraint_Angular)
        {
            RenderConstraintAngular(spine, MX_bones, QT_bones, *((VConstraintAngular *) *C_iter));
            continue;
        }
        if ((**C_iter).Type == VConstraint::Constraint_LengthEql)
        {
            VConstraintLengthEql *C_len = (VConstraintLengthEql *) *C_iter;
            boneA = spine.L_bones + C_len->particleA;
            boneB = spine.L_bones + C_len->particleB;
            minL = C_len->restLengthSqr - 0.001;
            maxL = C_len->restLengthSqr + 0.001;
            glColor4f(1.0f,0.0f,1.0f,1.0f);
        }
        else
        if ((**C_iter).Type == VConstraint::Constraint_LengthMin)
        {
            VConstraintLengthMin *C_min = (VConstraintLengthMin *) *C_iter;
            boneA = spine.L_bones + C_min->particleA;
            boneB = spine.L_bones + C_min->particleB;
            minL = C_min->minLengthSqr;
            maxL = 0.f;
            glColor4f(0.0f,1.0f,0.0f,1.0f);
        }
        else
        if ((**C_iter).Type == VConstraint::Constraint_LengthMax)
        {
            VConstraintLengthMax *C_max = (VConstraintLengthMax *) *C_iter;
            boneA = spine.L_bones + C_max->particleA;
            boneB = spine.L_bones + C_max->particleB;
            maxL = C_max->maxLengthSqr;
            minL = 0.f;
            glColor4f(0.0f,1.0f,1.0f,1.0f);
        }
        else
            continue;

        xVector3 P_boneA = MX_bones[boneA->ID].postTransformP(boneA->P_end);
        xVector3 P_boneB = MX_bones[boneB->ID].postTransformP(boneB->P_end);
        xFLOAT S_lenSqr = (P_boneA - P_boneB).lengthSqr();
        if (S_lenSqr < minL || (maxL != 0.f && S_lenSqr > maxL))
            glColor4f(1.0f,0.0f,0.0f,1.0f);
        glBegin(GL_LINES);
        {
            glVertex3fv(P_boneA.xyz);
            glVertex3fv(P_boneB.xyz);
        }
        glEnd();
    }
    glPopAttrib();
}


void RendererGL :: RenderSkeleton ( xModel &model, xModelInstance &instance, xWORD ID_selBone )
{
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
        glEnable(GL_COLOR_MATERIAL);

        glPushMatrix();
        glMultMatrixf(&instance.MX_LocalToWorld.x0);

        if (model.Spine.I_bones)
        {
            g_AnimSkeletal.BeginAnimation();
            g_AnimSkeletal.SetBones(instance.I_bones, instance.MX_bones, instance.QT_bones,
                                instance.P_bone_roots, instance.P_bone_trans, NULL, false);

            glColor4f(1.0f,1.0f,0.0f,1.0f);
            glBegin(GL_TRIANGLES);
            RenderBone(model.Spine.L_bones, 0, ID_selBone);
            glEnd();

            g_AnimSkeletal.EndAnimation();

            RenderConstraint(model.Spine, instance.MX_bones, instance.QT_bones);
        }
        else
        {
            /*
            glTranslatef(instance.P_center.x,instance.P_center.y,instance.P_center.z);
            glBegin(GL_LINES);
                glColor4f(1.f,0.f,0.f,1.f);
                glVertex3f(0,0,0);
                glVertex3f(1.f,0,0);

                glColor3f(0.f,1.f,0.f);
                glVertex3f(0,0,0);
                glVertex3f(0,1.f,0);

                glColor3f(0.f,0.f,1.f);
                glVertex3f(0,0,0);
                glVertex3f(0,0,1.f);
            glEnd();
            */
        }

        glPopMatrix();

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glDisable(GL_COLOR_MATERIAL);
}

// Bone / constraint selection

void RenderBoneSelection ( const xBone *L_bones, xBYTE ID_bone )
{
    const xBone &bone = L_bones[ID_bone];
    if (ID_bone)
    {
        glLoadName(ID_bone);
        glBegin(GL_TRIANGLES);
        RenderBoneCore(bone);
        glEnd();
    }

    xBYTE *ID_iter = bone.ID_kids;
    for (int i = bone.I_kids; i; --i, ++ID_iter)
        RenderBoneSelection(L_bones, *ID_iter);
}

void RenderConstraintSelection ( const xSkeleton &spine, const xMatrix *MX_bones, const xQuaternion *QT_bones )
{
    const xBone *boneA, *boneB;

    glPushAttrib(GL_LINE_BIT);
    glLineWidth(3.f);

    VConstraint **C_iter = spine.C_constraints;
    VConstraint **C_end  = C_iter + spine.I_constraints;
    xBYTE id = 0;
    for (; C_iter != C_end; ++C_iter, ++id)
    {
        glLoadName(id);

        if ((**C_iter).Type == VConstraint::Constraint_Angular)
        {
            RenderConstraintAngular(spine, MX_bones, QT_bones, *(VConstraintAngular *) *C_iter);
            continue;
        }
        if ((**C_iter).Type == VConstraint::Constraint_LengthEql)
        {
            VConstraintLengthEql *C_len = (VConstraintLengthEql *) *C_iter;
            boneA = spine.L_bones + C_len->particleA;
            boneB = spine.L_bones + C_len->particleB;
        }
        else
        if ((**C_iter).Type == VConstraint::Constraint_LengthMin)
        {
            VConstraintLengthMin *C_min = (VConstraintLengthMin *) *C_iter;
            boneA = spine.L_bones + C_min->particleA;
            boneB = spine.L_bones + C_min->particleB;
        }
        else
        if ((**C_iter).Type == VConstraint::Constraint_LengthMax)
        {
            VConstraintLengthMax *C_max = (VConstraintLengthMax *) *C_iter;
            boneA = spine.L_bones + C_max->particleA;
            boneB = spine.L_bones + C_max->particleB;
        }
        else
            continue;

        xVector3 P_boneA = MX_bones[boneA->ID].postTransformP(boneA->P_end);
        xVector3 P_boneB = MX_bones[boneB->ID].postTransformP(boneB->P_end);

        glBegin(GL_LINES);
        {
            glVertex3fv(P_boneA.xyz);
            glVertex3fv(P_boneB.xyz);
        }
        glEnd();
    }
    glPopAttrib();
}

void RendererGL :: RenderSkeletonSelection ( xModel &model, xModelInstance &instance, bool selectConstraint )
{
    if (model.Spine.I_bones)
    {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        glPushMatrix();
        glMultMatrixf(&instance.MX_LocalToWorld.x0);

        if (!selectConstraint)
        {
            g_AnimSkeletal.ForceSoftware(true);
            g_AnimSkeletal.BeginAnimation();
            g_AnimSkeletal.SetBones(instance.I_bones, instance.MX_bones, instance.QT_bones,
                                instance.P_bone_roots, instance.P_bone_trans, NULL, false);
            RenderBoneSelection(model.Spine.L_bones, 0);
            g_AnimSkeletal.EndAnimation();
            g_AnimSkeletal.ForceSoftware(false);
        }
        else
            RenderConstraintSelection(model.Spine, instance.MX_bones, instance.QT_bones);

        glPopMatrix();

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
    }
}
