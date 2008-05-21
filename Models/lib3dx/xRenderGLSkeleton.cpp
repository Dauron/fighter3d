#include "xRenderGL.h"
#include "../../OpenGL/GLAnimSkeletal.h"

// Bone / constraint view

void RenderBoneCore ( const xIKNode &bone )
{
    static xFLOAT4 boneWght = { 0.1f, 0.0f, 0.0f, 0.0f };

    xVector3 halfVector = (bone.pointE - bone.pointB) / 5.0f;
    xVector3 pointHalf  = bone.pointB + halfVector;
    xVector3 point3 = pointHalf + xVector3::Orthogonal(halfVector).normalize() / 10.0f;

    halfVector.normalize();
    float s = sin(PI/4.0f);
    xVector4 q; q.init(halfVector.x*s, halfVector.y*s, halfVector.z*s, cos(PI/4.0f));

    boneWght[0] = bone.id + 0.1f;
    g_AnimSkeletal.SetBoneIdxWghts(boneWght);

    for (int i=0; i<4; ++i) {
        g_AnimSkeletal.SetVertex(bone.pointE.xyz);
        g_AnimSkeletal.SetVertex(bone.pointB.xyz);
        g_AnimSkeletal.SetVertex(point3.xyz);
        point3 = xQuaternion::rotate(q, point3-pointHalf)+pointHalf;
    }
}

void RenderBone ( const xIKNode * boneP, xBYTE boneId, xWORD selBoneId )
{
    static xFLOAT4 boneWght = { 0.1f, 0.0f, 0.0f, 0.0f };

    const xIKNode &bone = boneP[boneId];
    if (boneId)
    {
        if (boneId == selBoneId)
        {
            glEnd();
            glColor4f(1.0f,0.0f,1.0f,1.0f);
            glBegin(GL_TRIANGLES);
        }

        RenderBoneCore(bone);

        if (boneId == selBoneId) {
            glEnd();
            glColor4f(1.0f,1.0f,0.0f,1.0f);
            glBegin(GL_TRIANGLES);
        }
    }

    xBYTE *cbone = bone.joinsEP;
    for (int i = bone.joinsEC; i; --i, ++cbone)
        RenderBone(boneP, *cbone, selBoneId);
}


void RenderConstraintAngular(const xSkeleton &spine, const xMatrix *bonesM,
                             const xVector4 *bonesQ,
                             const xVConstraintAngular *src)
{
    const xIKNode *boneRB, *boneRE, *boneB;
    boneRB = spine.boneP + src->particleRootB;
    boneRE = spine.boneP + src->particleRootE;
    boneB = spine.boneP + src->particle;

    xVector3 rpointB, rpointE, cpoint;

    rpointB = bonesM[boneRB->id].postTransformP(boneRB->pointE);
    rpointE = bonesM[boneRE->id].postTransformP(boneRE->pointE);

    xVector3 front;
    xVector3 up = (boneB->pointE - boneB->pointB).normalize();
    xVector3 up2 = (boneRE->pointE - boneRB->pointE).normalize();
    up2 = xQuaternion::rotate(src->upQuat, up2);
    if (src->particleRootE)
    {
        up    = bonesM[boneRE->id].postTransformV(up);
        front = bonesM[boneRE->id].postTransformV(xVector3::Create(0,1,0));
    }
    else
    {
        up    = xQuaternion::rotate(bonesQ[boneRB->id*2], up);
        front = xQuaternion::rotate(bonesQ[boneRB->id*2], xVector3::Create(0,1,0));
    }

    xMatrix transf = xMatrixFromVectors(front, up);//.invert();

    glBegin(GL_LINES);
    {
        // side - yellow
        glColor4f(1.0f,1.0f,0.0f,1.0f);
        xVector3 side = xVector3::CrossProduct(front, up).normalize();
        cpoint = rpointE + side * 0.1f;
        glVertex3fv(rpointE.xyz);
        glVertex3fv(cpoint.xyz);

        // up - green
        glColor4f(0.0f,0.6f,0.0f,1.0f);
        cpoint = rpointE + up * 0.1f;
        glVertex3fv(rpointE.xyz);
        glVertex3fv(cpoint.xyz);

        // front - blue
        glColor4f(0.0f,0.0f,1.0f,1.0f);
        cpoint = rpointE + xVector3::CrossProduct(side, up) * 0.1f;
        glVertex3fv(rpointE.xyz);
        glVertex3fv(cpoint.xyz);

        // current - aqua
        cpoint = (boneB->pointE - boneB->pointB).normalize() * 0.2f;
        cpoint = rpointE + bonesM[boneB->id].postTransformV(cpoint);
        if (src->Test(rpointB, rpointE, cpoint, up, front))
            glColor4f(1.0f,0.0f,0.0f,1.0f);
        else
            glColor4f(0.0f,1.0f,1.0f,1.0f);
        glVertex3fv(rpointE.xyz);
        glVertex3fv(cpoint.xyz);
    }
    glEnd();

    xFLOAT maxX  = src->elipseMaxX * 0.1f;
    xFLOAT maxXH = maxX * 0.707f; // cos45
    xFLOAT maxY = src->elipseMaxY * 0.1f;
    xFLOAT maxYH = maxY * 0.707f; // sin45
    xFLOAT minX = -src->elipseMinX * 0.1f;
    xFLOAT minXH = minX * 0.707f; // cos45
    xFLOAT minY = -src->elipseMinY * 0.1f;
    xFLOAT minYH = minY * 0.707f; // sin45
    xFLOAT maxZ;

    glColor4f(0.0f,1.0f,1.0f,1.0f);
    glBegin(GL_TRIANGLE_FAN);
    {
        glVertex3fv(rpointE.xyz);

        maxZ = src->zSignMaxX * sqrt(max(0.01f - maxX*maxX, 0.f));
        cpoint = rpointE + transf.postTransformV(xVector3::Create(maxX, 0.f, maxZ));
        glVertex3fv(cpoint.xyz);

        if (src->zSignMaxX*src->zSignMaxY> 0)
            maxZ = sqrt(max(0.01f - maxXH*maxXH - maxYH*maxYH, 0.f));
        else
        {
            xFLOAT ZmX = src->zSignMaxX * sqrt(max(0.01f - maxX*maxX, 0.f));
            xFLOAT ZmY = src->zSignMaxY * sqrt(max(0.01f - maxY*maxY, 0.f));
            maxZ = 0.707f*ZmX + (1-0.707f)*ZmY;
        }
        cpoint = rpointE + transf.postTransformV(xVector3::Create(maxXH, maxYH, maxZ));
        glVertex3fv(cpoint.xyz);

        maxZ = src->zSignMaxY * sqrt(max(0.01f - maxY*maxY, 0.f));
        cpoint = rpointE + transf.postTransformV(xVector3::Create(0.f, maxY, maxZ));
        glVertex3fv(cpoint.xyz);

        if (src->zSignMinX*src->zSignMaxY> 0)
            maxZ = sqrt(max(0.01f - minXH*minXH - maxYH*maxYH, 0.f));
        else
        {
            xFLOAT ZmX = src->zSignMinX * sqrt(max(0.01f - minX*minX, 0.f));
            xFLOAT ZmY = src->zSignMaxY * sqrt(max(0.01f - maxY*maxY, 0.f));
            maxZ = 0.707f*ZmX + (1-0.707f)*ZmY;
        }
        cpoint = rpointE + transf.postTransformV(xVector3::Create(minXH, maxYH, maxZ));
        glVertex3fv(cpoint.xyz);

        maxZ = src->zSignMinX * sqrt(max(0.01f - minX*minX, 0.f));
        cpoint = rpointE + transf.postTransformV(xVector3::Create(minX, 0.f, maxZ));
        glVertex3fv(cpoint.xyz);

        if (src->zSignMinX*src->zSignMinY> 0)
            maxZ = sqrt(max(0.01f - minXH*minXH - minYH*minYH, 0.f));
        else
        {
            xFLOAT ZmX = src->zSignMinX * sqrt(max(0.01f - minX*minX, 0.f));
            xFLOAT ZmY = src->zSignMinY * sqrt(max(0.01f - minY*minY, 0.f));
            maxZ = 0.707f*ZmX + (1-0.707f)*ZmY;
        }
        cpoint = rpointE + transf.postTransformV(xVector3::Create(minXH, minYH, maxZ));
        glVertex3fv(cpoint.xyz);

        maxZ = src->zSignMinY * sqrt(max(0.01f - minY*minY, 0.f));
        cpoint = rpointE + transf.postTransformV(xVector3::Create(0.f, minY, maxZ));
        glVertex3fv(cpoint.xyz);

        if (src->zSignMaxX*src->zSignMinY> 0)
            maxZ = sqrt(max(0.01f - maxXH*maxXH - minYH*minYH, 0.f));
        else
        {
            xFLOAT ZmX = src->zSignMaxX * sqrt(max(0.01f - maxX*maxX, 0.f));
            xFLOAT ZmY = src->zSignMinY * sqrt(max(0.01f - minY*minY, 0.f));
            maxZ = 0.707f*ZmX + (1-0.707f)*ZmY;
        }
        cpoint = rpointE + transf.postTransformV(xVector3::Create(maxXH, minYH, maxZ));
        glVertex3fv(cpoint.xyz);

        maxZ = src->zSignMaxX * sqrt(max(0.01f - maxX*maxX, 0.f));
        cpoint = rpointE + transf.postTransformV(xVector3::Create(maxX, 0.f, maxZ));
        glVertex3fv(cpoint.xyz);
    }
    glEnd();
}

void RenderConstraint ( const xSkeleton &spine, const xMatrix *bonesM, const xVector4 *bonesQ )
{
    static xFLOAT4 boneWght = { 0.1f, 0.0f, 0.0f, 0.0f };
    const xIKNode *boneA, *boneB;
    xFLOAT minL, maxL;

    glPushAttrib(GL_LINE_BIT);
    glLineWidth(3.f);

    xIVConstraint **iter = spine.constraintsP;
    xIVConstraint **end  = iter + spine.constraintsC;
    for (; iter != end; ++iter)
    {
        if ((**iter).Type == xIVConstraint::Constraint_Angular)
        {
            RenderConstraintAngular(spine, bonesM, bonesQ, (xVConstraintAngular *) *iter);
            continue;
        }
        if ((**iter).Type == xIVConstraint::Constraint_LengthEql)
        {
            xVConstraintLengthEql *src = (xVConstraintLengthEql *) *iter;
            boneA = spine.boneP + src->particleA;
            boneB = spine.boneP + src->particleB;
            minL = src->restLengthSqr - 0.001;
            maxL = src->restLengthSqr + 0.001;
            glColor4f(1.0f,0.0f,1.0f,1.0f);
        }
        else
        if ((**iter).Type == xIVConstraint::Constraint_LengthMin)
        {
            xVConstraintLengthMin *src = (xVConstraintLengthMin *) *iter;
            boneA = spine.boneP + src->particleA;
            boneB = spine.boneP + src->particleB;
            minL = src->minLengthSqr;
            maxL = 0.f;
            glColor4f(0.0f,1.0f,0.0f,1.0f);
        }
        else
        if ((**iter).Type == xIVConstraint::Constraint_LengthMax)
        {
            xVConstraintLengthMax *src = (xVConstraintLengthMax *) *iter;
            boneA = spine.boneP + src->particleA;
            boneB = spine.boneP + src->particleB;
            maxL = src->maxLengthSqr;
            minL = 0.f;
            glColor4f(0.0f,1.0f,1.0f,1.0f);
        }
        else
            continue;

        xVector3 posA = bonesM[boneA->id].postTransformP(boneA->pointE);
        xVector3 posB = bonesM[boneB->id].postTransformP(boneB->pointE);
        xFLOAT len = (posA - posB).lengthSqr();
        if (len < minL || (maxL != 0.f && len > maxL))
            glColor4f(1.0f,0.0f,0.0f,1.0f);

        glBegin(GL_LINES);
        {

            glVertex3fv(posA.xyz);
            glVertex3fv(posB.xyz);
        }
        glEnd();
    }
    glPopAttrib();
}


void xRenderGL :: RenderSkeleton ( xModel &model, xModelInstance &instance, xWORD selBoneId )
{
    if (model.spine.boneC)
    {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
        glEnable(GL_COLOR_MATERIAL);

        glPushMatrix();
        glMultMatrixf(&instance.location.x0);

        g_AnimSkeletal.BeginAnimation();
        g_AnimSkeletal.SetBones(instance.bonesC, instance.bonesM, instance.bonesQ, NULL, false);

        glColor4f(1.0f,1.0f,0.0f,1.0f);
        glBegin(GL_TRIANGLES);
        RenderBone(model.spine.boneP, 0, selBoneId);
        glEnd();

        g_AnimSkeletal.EndAnimation();

        RenderConstraint(model.spine, instance.bonesM, instance.bonesQ);

        glPopMatrix();

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glDisable(GL_COLOR_MATERIAL);
    }
}

// Bone / constraint selection

void RenderBoneSelection ( const xIKNode * boneP, xBYTE boneId )
{
    const xIKNode &bone = boneP[boneId];
    if (boneId)
    {
        glLoadName(boneId);
        glBegin(GL_TRIANGLES);
        RenderBoneCore(bone);
        glEnd();
    }

    xBYTE *cbone = bone.joinsEP;
    for (int i = bone.joinsEC; i; --i, ++cbone)
        RenderBoneSelection(boneP, *cbone);
}

void RenderConstraintSelection ( const xSkeleton &spine, const xMatrix *bonesM, const xVector4 *bonesQ )
{
    static xFLOAT4 boneWght = { 0.1f, 0.0f, 0.0f, 0.0f };
    const xIKNode *boneA, *boneB;

    glPushAttrib(GL_LINE_BIT);
    glLineWidth(3.f);

    xIVConstraint **iter = spine.constraintsP;
    xIVConstraint **end  = iter + spine.constraintsC;
    xBYTE id = 0;
    for (; iter != end; ++iter, ++id)
    {
        glLoadName(id);

        if ((**iter).Type == xIVConstraint::Constraint_Angular)
        {
            RenderConstraintAngular(spine, bonesM, bonesQ, (xVConstraintAngular *) *iter);
            continue;
        }
        if ((**iter).Type == xIVConstraint::Constraint_LengthEql)
        {
            xVConstraintLengthEql *src = (xVConstraintLengthEql *) *iter;
            boneA = spine.boneP + src->particleA;
            boneB = spine.boneP + src->particleB;
        }
        else
        if ((**iter).Type == xIVConstraint::Constraint_LengthMin)
        {
            xVConstraintLengthMin *src = (xVConstraintLengthMin *) *iter;
            boneA = spine.boneP + src->particleA;
            boneB = spine.boneP + src->particleB;
        }
        else
        if ((**iter).Type == xIVConstraint::Constraint_LengthMax)
        {
            xVConstraintLengthMax *src = (xVConstraintLengthMax *) *iter;
            boneA = spine.boneP + src->particleA;
            boneB = spine.boneP + src->particleB;
        }
        else
            continue;

        xVector3 posA = bonesM[boneA->id].postTransformP(boneA->pointE);
        xVector3 posB = bonesM[boneB->id].postTransformP(boneB->pointE);

        glBegin(GL_LINES);
        {
            glVertex3fv(posA.xyz);
            glVertex3fv(posB.xyz);
        }
        glEnd();
    }
    glPopAttrib();
}

void xRenderGL :: RenderSkeletonSelection ( xModel &model, xModelInstance &instance, bool selectConstraint )
{
    if (model.spine.boneC)
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
            RenderBoneSelection(model.spine.boneP, 0);
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
