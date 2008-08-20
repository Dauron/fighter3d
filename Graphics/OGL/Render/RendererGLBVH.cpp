#include "RendererGL.h"
#include "../../../Math/Figures/xSphere.h"
#include "../../../Math/Figures/xCapsule.h"
#include "../../../Math/Figures/xBoxO.h"
#include "../../../Math/Figures/xCylinder.h"
#include "../../../Math/Figures/xMesh.h"

using namespace Math::Figures;

////////////////////////////// Sphere

void RenderSphere(const xSphere &object)
{
    glPushMatrix();
    glTranslatef(object.P_center.x,object.P_center.y,object.P_center.z);

    const xBYTE I_steps = 20;
    xQuaternion QT_step; QT_step.init(sin(PI / I_steps), 0,0, cos(PI / I_steps));
    xQuaternion QT_bigs; QT_bigs.init(0, 0, sin(PI / (2*I_steps)), cos(PI / (2*I_steps)));
    xMatrix  MX_bigs = xMatrixFromQuaternion(QT_bigs);

    xPoint3 P_points[I_steps];
    P_points[0] = xVector3::Create(0,1,0) * object.S_radius;
    for (int i = 1; i < I_steps; ++i)
        P_points[i] = xQuaternion::rotate(QT_step, P_points[i-1]);

    for (int i = 0; i < I_steps; ++i)
    {
        glBegin(GL_POLYGON);
        for (int j = 0; j < I_steps; ++j)
            glVertex3fv(P_points[j].xyz);
        glEnd();

        glMultMatrixf(&MX_bigs.x0);
    }

    glPopMatrix();
}

////////////////////////////// Capsule

void RenderCapsule(const xCapsule &object)
{
    glPushMatrix();
    glTranslatef(object.P_center.x,object.P_center.y,object.P_center.z);

    const xBYTE I_steps = 10;
    xVector3 N_side = xVector3::CrossProduct(object.N_top, xVector3::Create(0,1,0));
    if (N_side.lengthSqr() < EPSILON2)
        N_side = xVector3::CrossProduct(object.N_top, xVector3::Create(1,0,0));
    N_side.normalize();

    xQuaternion QT_step; QT_step.init(xVector3::CrossProduct(object.N_top, N_side) * -sin(PI / (2*I_steps)),
                                   cos(PI / (2*I_steps)));
    xQuaternion QT_bigs = QT_bigs.init(object.N_top * sin(PI / (2*I_steps)), cos(PI / (2*I_steps)));
    xMatrix     MX_bigs = xMatrixFromQuaternion(QT_bigs);

    xVector3 P_points[I_steps * 2];
    P_points[0] = N_side * object.S_radius;
    for (int i = 1; i < I_steps; ++i)
        P_points[i] = xQuaternion::rotate(QT_step, P_points[i-1]);

    P_points[I_steps] = -N_side * object.S_radius;
    for (int i = I_steps+1; i < 2*I_steps; ++i)
        P_points[i] = xQuaternion::rotate(QT_step, P_points[i-1]);

    xVector3 NW_top = object.N_top * object.S_top;

    for (int i = 0; i < I_steps; ++i)
    {
        glBegin(GL_POLYGON);
        for (int j = 0; j < I_steps; ++j)
            glVertex3fv((P_points[j] + NW_top).xyz);
        for (int j = I_steps; j < 2*I_steps; ++j)
            glVertex3fv((P_points[j] - NW_top).xyz);
        glEnd();

        glMultMatrixf(&MX_bigs.x0);
    }

    glPopMatrix();
}

////////////////////////////// BoxO

void RenderBoxO(const xBoxO &object, int I_level)
{
    glPushMatrix();
    glTranslatef(object.P_center.x,object.P_center.y,object.P_center.z);

    if (I_level == 0)
    {
        glBegin(GL_LINES);
        {
            glVertex3f(0,0,0);
            glVertex3fv((object.N_top*object.S_top).xyz);
            glVertex3f(0,0,0);
            glVertex3fv((object.N_side*object.S_side).xyz);
            glVertex3f(0,0,0);
            glVertex3fv((object.N_front*object.S_front).xyz);
        }
        glEnd();
    }

    const xBYTE I_steps = 1;
    xVector3 P_points[4];
    P_points[0] = object.N_top*object.S_top + object.N_side*object.S_side;
    P_points[1] = object.N_top*object.S_top - object.N_side*object.S_side;
    P_points[2] = -object.N_top*object.S_top - object.N_side*object.S_side;
    P_points[3] = -object.N_top*object.S_top + object.N_side*object.S_side;

    xVector3 NW_shift = object.N_front*object.S_front;
    glPushMatrix();
    glTranslatef(NW_shift.x,NW_shift.y,NW_shift.z);
    NW_shift /= -I_steps*0.5f;
    for (int i = 0; i < I_steps+1; ++i)
    {
        glBegin(GL_QUADS);
        for (int j = 0; j < 4; ++j)
            glVertex3fv(P_points[j].xyz);
        glEnd();

        glTranslatef(NW_shift.x,NW_shift.y,NW_shift.z);
    }
    glPopMatrix();

    P_points[0] = object.N_top*object.S_top + object.N_front*object.S_front;
    P_points[1] = object.N_top*object.S_top - object.N_front*object.S_front;
    P_points[2] = -object.N_top*object.S_top - object.N_front*object.S_front;
    P_points[3] = -object.N_top*object.S_top + object.N_front*object.S_front;

    NW_shift = object.N_side*object.S_side;
    glTranslatef(NW_shift.x,NW_shift.y,NW_shift.z);
    NW_shift /= -I_steps*0.5f;
    for (int i = 0; i < I_steps+1; ++i)
    {
        glBegin(GL_QUADS);
        for (int j = 0; j < 4; ++j)
            glVertex3fv(P_points[j].xyz);
        glEnd();

        glTranslatef(NW_shift.x,NW_shift.y,NW_shift.z);
    }

    glPopMatrix();
}

////////////////////////////// Cylinder

void RenderCylinder(const xCylinder &object)
{
    glPushMatrix();
    glTranslatef(object.P_center.x,object.P_center.y,object.P_center.z);

    glPopMatrix();
}

////////////////////////////// Mesh

void RenderMesh(const xMesh &object)
{
    xDWORD  *L_FaceIndex_Itr = object.L_FaceIndices;
/*
    glVertexPointer(object.MeshData->I_VertexCount, GL_FLOAT, object.MeshData->I_VertexStride, object.MeshData->L_VertexData);
    for (int i = object.I_FaceIndices; i; --i, ++L_FaceIndex_Itr)
        glDrawElements (GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, object.MeshData->GetFacePtr(*L_FaceIndex_Itr));
*/

    if(object.MeshData->L_BoneData && object.MeshData->MX_Bones && object.MeshData->FL_VertexIsTransf)
    {
        //glPushMatrix();
        //xMatrix MX = object.MeshData->MX_MeshToLocal;
        //glMultMatrixf( & MX.invert().x0 );

        glBegin(GL_TRIANGLES);
        {
            for (int i = object.I_FaceIndices; i; --i, ++L_FaceIndex_Itr)
            {
                xWORD3 &Face = object.MeshData->GetFace(*L_FaceIndex_Itr);
                glVertex3fv(object.MeshData->GetVertexTransf(Face[0]).xyz);
                glVertex3fv(object.MeshData->GetVertexTransf(Face[1]).xyz);
                glVertex3fv(object.MeshData->GetVertexTransf(Face[2]).xyz);
            }
        }
        glEnd();

        //glPopMatrix();
    }
    else
    {
        glBegin(GL_TRIANGLES);
        {
            for (int i = object.I_FaceIndices; i; --i, ++L_FaceIndex_Itr)
            {
                xWORD3 &Face = object.MeshData->GetFace(*L_FaceIndex_Itr);
                glVertex3fv(object.MeshData->GetVertex(Face[0]).xyz);
                glVertex3fv(object.MeshData->GetVertex(Face[1]).xyz);
                glVertex3fv(object.MeshData->GetVertex(Face[2]).xyz);
            }
        }
        glEnd();
    }
}

// BVH

void RendererGL :: RenderBVHExt ( xBVHierarchy &bvh, const xMatrix &MX_LocalToWorld,
                                  xBYTE I_level, xBYTE &ID, xBYTE ID_selected, bool FL_selection )
{
    if (I_level == 0)
    {
        glPushMatrix();
        //glMultMatrixf(& bvh.MX_LocalToWorld_Get().x0);
    }
    else
    if (I_level == 1)
    {
        glPushMatrix();
        //glMultMatrixf(& bvh.MX_RawToLocal_Get().x0);
    }

    if (FL_selection)
        glLoadName(ID);
    else
    if (ID == ID_selected)
        glColor3f(1.f,1.f,0.f);
    else
        glColor3f(1.f,1.f,1.f);

    // Sphere
    if (bvh.Figure->Type == xIFigure3d::Sphere)
        RenderSphere(*(xSphere*)bvh.GetTransformed(MX_LocalToWorld));
    else

    // Capsule
    if (bvh.Figure->Type == xIFigure3d::Capsule)
        RenderCapsule(*(xCapsule*)bvh.GetTransformed(MX_LocalToWorld));
    else

    // BoxO
    if (bvh.Figure->Type == xIFigure3d::BoxOriented)
        RenderBoxO(*(xBoxO*)bvh.GetTransformed(MX_LocalToWorld), I_level);
    else

    // Cylinder
    if (bvh.Figure->Type == xIFigure3d::Cylinder)
        RenderCylinder(*(xCylinder*)bvh.GetTransformed(MX_LocalToWorld));
    else

    // Mesh
    if (bvh.Figure->Type == xIFigure3d::Mesh)
        RenderMesh(*(xMesh*)bvh.GetTransformed(MX_LocalToWorld));

    xBVHierarchy *bvh_cur = bvh.L_items,
                 *bvh_end = bvh.L_items + bvh.I_items;
    for (; bvh_cur != bvh_end; ++bvh_cur)
        RenderBVHExt ( *bvh_cur, MX_LocalToWorld, I_level + 1, ++ID, ID_selected, FL_selection );

    if (I_level == 0 || I_level == 1)
        glPopMatrix();
}
