#include "xRenderGL.h"
#include "../GLAnimSkeletal.h"
#include "../../Physics/Verlet/xVConstraintLengthEql.h"
#include "../../Physics/Verlet/xVConstraintLengthMin.h"
#include "../../Physics/Verlet/xVConstraintLengthMax.h"
#include "../../Physics/Verlet/xVConstraintAngular.h"

// Bone / constraint view

void RenderBoneCore ( const xBone &bone )
{
    static xFLOAT4 W_bone = { 0.1f, 0.0f, 0.0f, 0.0f };

    xVector3 NW_fifth = (bone.P_end - bone.P_begin) * 0.2f;
    xVector3 P_fifth  = bone.P_begin + NW_fifth;
    xVector3 P_third  = P_fifth + xVector3::Orthogonal(NW_fifth).normalize() * 0.1f;

    float W_sin = sin(PI * 0.25f);
    xVector4 QT_rot; QT_rot.init(NW_fifth.normalize() * sin(PI * 0.25f), cos(PI * 0.25f));

    W_bone[0] = bone.ID + 0.1f;
    g_AnimSkeletal.SetBoneIdxWghts(W_bone);

    for (int i=0; i<4; ++i) {
        g_AnimSkeletal.SetVertex(bone.P_end.xyz);
        g_AnimSkeletal.SetVertex(bone.P_begin.xyz);
        g_AnimSkeletal.SetVertex(P_third.xyz);
        P_third = xQuaternion::rotate(QT_rot, P_third-P_fifth)+P_fifth;
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


void RenderConstraintAngular(const xSkeleton &spine, const xMatrix *MX_bones,
                             const xVector4 *QT_bones, const xVConstraintAngular &C_angular)
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
        N_front = MX_bones[boneRE.ID].postTransformV(xVector3::Create(0,1,0));
    }
    else
    {
        N_up    = xQuaternion::rotate(QT_bones[boneRB.ID*2], N_up);
        N_front = xQuaternion::rotate(QT_bones[boneRB.ID*2], xVector3::Create(0,1,0));
    }

    xMatrix MX_BoneToWorld = xMatrixFromVectors(N_front, N_up);

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

    xVector3 v[8];
    v[0].init(sin(C_angular.angleMaxX) * 0.1f, 0.f, cos(C_angular.angleMaxX) * 0.1);

    xFLOAT angleH = (C_angular.angleMaxX + C_angular.angleMaxY) * 0.5f;
    xFLOAT radius = sin(angleH);
    v[1].init(radius * 0.0707f, radius * 0.0707f, cos(angleH) * 0.1f);

    v[2].init(0.f, sin(C_angular.angleMaxY) * 0.1f, cos(C_angular.angleMaxY) * 0.1);

    angleH = (C_angular.angleMinX + C_angular.angleMaxY) * 0.5f;
    radius = sin(angleH);
    v[3].init(-radius * 0.0707f, radius * 0.0707f, cos(angleH) * 0.1f);

    v[4].init(sin(C_angular.angleMinX) * -0.1f, 0.f, cos(C_angular.angleMinX) * 0.1);

    angleH = (C_angular.angleMinX + C_angular.angleMinY) * 0.5f;
    radius = sin(angleH);
    v[5].init(-radius * 0.0707f, -radius * 0.0707f, cos(angleH) * 0.1f);

    v[6].init(0.f, sin(C_angular.angleMinY) * -0.1f, cos(C_angular.angleMinY) * 0.1);

    angleH = (C_angular.angleMaxX + C_angular.angleMinY) * 0.5f;
    radius = sin(angleH);
    v[7].init(radius * 0.0707f, -radius * 0.0707f, cos(angleH) * 0.1f);

    glColor4f(0.0f,1.0f,1.0f,1.0f);
    glBegin(GL_TRIANGLE_FAN);
    {
        glVertex3fv(P_rootE.xyz);
        for (int i=0; i<8; ++i)
        {
            v[i] = P_rootE + MX_BoneToWorld.postTransformV(v[i]);
            glVertex3fv(v[i].xyz);
        }
        glColor4f(0.0f,0.5f,0.5f,1.0f);
        glVertex3fv(v[0].xyz);
    }
    glEnd();
}

void RenderConstraint ( const xSkeleton &spine, const xMatrix *MX_bones, const xVector4 *QT_bones )
{
    const xBone *boneA, *boneB;
    xFLOAT minL, maxL;

    glPushAttrib(GL_LINE_BIT);
    glLineWidth(3.f);

    xVConstraint **C_iter = spine.C_constraints;
    xVConstraint **C_end  = C_iter + spine.I_constraints;
    for (; C_iter != C_end; ++C_iter)
    {
        if ((**C_iter).Type == xVConstraint::Constraint_Angular)
        {
            RenderConstraintAngular(spine, MX_bones, QT_bones, *((xVConstraintAngular *) *C_iter));
            continue;
        }
        if ((**C_iter).Type == xVConstraint::Constraint_LengthEql)
        {
            xVConstraintLengthEql *C_len = (xVConstraintLengthEql *) *C_iter;
            boneA = spine.L_bones + C_len->particleA;
            boneB = spine.L_bones + C_len->particleB;
            minL = C_len->restLengthSqr - 0.001;
            maxL = C_len->restLengthSqr + 0.001;
            glColor4f(1.0f,0.0f,1.0f,1.0f);
        }
        else
        if ((**C_iter).Type == xVConstraint::Constraint_LengthMin)
        {
            xVConstraintLengthMin *C_min = (xVConstraintLengthMin *) *C_iter;
            boneA = spine.L_bones + C_min->particleA;
            boneB = spine.L_bones + C_min->particleB;
            minL = C_min->minLengthSqr;
            maxL = 0.f;
            glColor4f(0.0f,1.0f,0.0f,1.0f);
        }
        else
        if ((**C_iter).Type == xVConstraint::Constraint_LengthMax)
        {
            xVConstraintLengthMax *C_max = (xVConstraintLengthMax *) *C_iter;
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


void xRenderGL :: RenderSkeleton ( xModel &model, xModelInstance &instance, xWORD ID_selBone )
{
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
        glEnable(GL_COLOR_MATERIAL);

        glPushMatrix();
        glMultMatrixf(&instance.location.x0);

        if (model.spine.I_bones)
        {
            g_AnimSkeletal.BeginAnimation();
            g_AnimSkeletal.SetBones(instance.bonesC, instance.bonesM, instance.bonesQ, NULL, false);

            glColor4f(1.0f,1.0f,0.0f,1.0f);
            glBegin(GL_TRIANGLES);
            RenderBone(model.spine.L_bones, 0, ID_selBone);
            glEnd();

            g_AnimSkeletal.EndAnimation();

            RenderConstraint(model.spine, instance.bonesM, instance.bonesQ);
        }
        else
        {
            glTranslatef(instance.center.x,instance.center.y,instance.center.z);
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

void RenderConstraintSelection ( const xSkeleton &spine, const xMatrix *MX_bones, const xVector4 *QT_bones )
{
    const xBone *boneA, *boneB;

    glPushAttrib(GL_LINE_BIT);
    glLineWidth(3.f);

    xVConstraint **C_iter = spine.C_constraints;
    xVConstraint **C_end  = C_iter + spine.I_constraints;
    xBYTE id = 0;
    for (; C_iter != C_end; ++C_iter, ++id)
    {
        glLoadName(id);

        if ((**C_iter).Type == xVConstraint::Constraint_Angular)
        {
            RenderConstraintAngular(spine, MX_bones, QT_bones, *(xVConstraintAngular *) *C_iter);
            continue;
        }
        if ((**C_iter).Type == xVConstraint::Constraint_LengthEql)
        {
            xVConstraintLengthEql *C_len = (xVConstraintLengthEql *) *C_iter;
            boneA = spine.L_bones + C_len->particleA;
            boneB = spine.L_bones + C_len->particleB;
        }
        else
        if ((**C_iter).Type == xVConstraint::Constraint_LengthMin)
        {
            xVConstraintLengthMin *C_min = (xVConstraintLengthMin *) *C_iter;
            boneA = spine.L_bones + C_min->particleA;
            boneB = spine.L_bones + C_min->particleB;
        }
        else
        if ((**C_iter).Type == xVConstraint::Constraint_LengthMax)
        {
            xVConstraintLengthMax *C_max = (xVConstraintLengthMax *) *C_iter;
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

void xRenderGL :: RenderSkeletonSelection ( xModel &model, xModelInstance &instance, bool selectConstraint )
{
    if (model.spine.I_bones)
    {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        glPushMatrix();
        glMultMatrixf(&instance.location.x0);

        if (!selectConstraint)
        {
            g_AnimSkeletal.ForceSoftware(true);
            g_AnimSkeletal.BeginAnimation();
            g_AnimSkeletal.SetBones(instance.bonesC, instance.bonesM, instance.bonesQ, NULL, false);
            RenderBoneSelection(model.spine.L_bones, 0);
            g_AnimSkeletal.EndAnimation();
            g_AnimSkeletal.ForceSoftware(false);
        }
        else
            RenderConstraintSelection(model.spine, instance.bonesM, instance.bonesQ);

        glPopMatrix();

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
    }
}
