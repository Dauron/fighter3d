#include "MapRender.h"
#include "Shader.h"
#include "../../Math/Cameras/Camera.h"
#include "../../Utils/Profiler.h"
#include "../../Utils/Debug.h"

using namespace Graphics::OGL;
using namespace World;

float MapRender:: LOD_MedDistance_Sqr = 2000.f;
float MapRender:: LOD_LowDistance_Sqr = 30000.f;
float MapRender:: LOD_FarDistance_Sqr = 60000.f;

void MapRender:: Destroy()
{
    DistrictInfo *DL_curr = L_districts,
                 *DL_last = L_districts + I_districts;
    for (; DL_curr < DL_last; ++DL_curr)
    {
        if (DL_curr->ListNorth && glIsList(DL_curr->ListNorth)) glDeleteLists(DL_curr->ListNorth, 1);
        if (DL_curr->ListSouth && glIsList(DL_curr->ListSouth)) glDeleteLists(DL_curr->ListSouth, 1);
        if (DL_curr->ListWest  && glIsList(DL_curr->ListWest))  glDeleteLists(DL_curr->ListWest, 1);
        if (DL_curr->ListEast  && glIsList(DL_curr->ListEast))  glDeleteLists(DL_curr->ListEast, 1);
        if (DL_curr->ListTop   && glIsList(DL_curr->ListTop))   glDeleteLists(DL_curr->ListTop, 1);
    }

    if (ListBirdHgh && glIsList(ListBirdHgh)) glDeleteLists(ListBirdHgh, 1);
    if (ListBirdMed && glIsList(ListBirdMed)) glDeleteLists(ListBirdMed, 1);
    if (ListBirdLow && glIsList(ListBirdLow)) glDeleteLists(ListBirdLow, 1);
    if (ListBirdFar && glIsList(ListBirdFar)) glDeleteLists(ListBirdFar, 1);

    if (!WallTexture.IsNull())
        g_TextureMgr.Release(WallTexture);
    if (!GroundTexture.IsNull())
        g_TextureMgr.Release(GroundTexture);
    if (!WallTexture_Normal.IsNull())
        g_TextureMgr.Release(WallTexture_Normal);
    if (!GroundTexture_Normal.IsNull())
        g_TextureMgr.Release(GroundTexture_Normal);
    Clear();
}

void MapRender:: RenderBirds(const Math::Cameras::FieldOfView &fov)
{
    // High LOD model
    if (!ListBirdHgh)
    {
        ListBirdHgh = glGenLists(1);
        glNewList(ListBirdHgh, GL_COMPILE);
        {
            GLUquadricObj *Bird = gluNewQuadric();
            gluQuadricNormals(Bird, GLU_SMOOTH);
            gluQuadricTexture(Bird, GL_FALSE);
            gluSphere(Bird,Bird::Size,15,15);
            gluDeleteQuadric(Bird);
        }
        glEndList();
    }
    // Medium LOD model
    if (!ListBirdMed)
    {
        ListBirdMed = glGenLists(1);
        glNewList(ListBirdMed, GL_COMPILE);
        {
            GLUquadricObj *Bird = gluNewQuadric();
            gluQuadricNormals(Bird, GLU_SMOOTH);
            gluQuadricTexture(Bird, GL_FALSE);
            gluSphere(Bird,Bird::Size,8,8);
            gluDeleteQuadric(Bird);
        }
        glEndList();
    }
    // Low LOD model
    if (!ListBirdLow)
    {
        ListBirdLow = glGenLists(1);
        glNewList(ListBirdLow, GL_COMPILE);
        {
            GLUquadricObj *Bird = gluNewQuadric();
            gluQuadricNormals(Bird, GLU_SMOOTH);
            gluQuadricTexture(Bird, GL_FALSE);
            gluSphere(Bird,Bird::Size,4,4);
            gluDeleteQuadric(Bird);
        }
        glEndList();
    }
    // Far LOD model
    if (!ListBirdFar)
    {
        ListBirdFar = glGenLists(1);
        glNewList(ListBirdFar, GL_COMPILE);
        {
            GLUquadricObj *Bird = gluNewQuadric();
            gluQuadricNormals(Bird, GLU_SMOOTH);
            gluQuadricTexture(Bird, GL_FALSE);
            gluSphere(Bird,Bird::Size,3,3);
            gluDeleteQuadric(Bird);
        }
        glEndList();
    }

    glMaterialf(GL_FRONT, GL_SHININESS, 2.0f);

    const xPoint3 &P_eye = fov.Camera_Get()->P_eye;

    // Render birds
    {
        Profile("Render birds");
        glColor3ub(255,255,0);

        LstBirdP::const_iterator BRD_curr = Map->Birds.begin(),
                                 BRD_last = Map->Birds.end();
        for(; BRD_curr != BRD_last; ++BRD_curr)
        {
            Bird &bird = **BRD_curr;
            if (fov.CheckSphere(bird.P_center_Trfm, Bird::Size))
            {
                ++Performance.RenderedBirds;
                glPushMatrix();
                {
                    //glMultMatrixf(&bird.MX_LocalToWorld_Get().x0);
                    glTranslatef(bird.P_center_Trfm.x,
                                 bird.P_center_Trfm.y,
                                 bird.P_center_Trfm.z);

                    float S_dist_Sqr = (P_eye - bird.P_center_Trfm).lengthSqr();
                    if (S_dist_Sqr > LOD_FarDistance_Sqr)
                        glCallList(ListBirdFar);
                    else
                    if (S_dist_Sqr > LOD_LowDistance_Sqr)
                        glCallList(ListBirdLow);
                    else
                    if (S_dist_Sqr > LOD_MedDistance_Sqr)
                        glCallList(ListBirdMed);
                    else
                        glCallList(ListBirdHgh);
                }
                glPopMatrix();
            }
        }
    }

    // Render hawks
    {
        Profile("Render hawks");

        LstHawkP::const_iterator HWK_curr = Map->Hawks.begin(),
                                 HWK_last = Map->Hawks.end();
        for(; HWK_curr != HWK_last; ++HWK_curr)
        {
            Hawk &bird = **HWK_curr;
            if (fov.CheckSphere(bird.P_center_Trfm, Bird::Size))
            {
                xBYTE g = 0;
                if (bird.Energy < Hawk::EatEnergy)
                    g = 255 - xBYTE(255 * bird.Energy / Hawk::EatEnergy);
                glColor3ub(255,g,0);

                glPushMatrix();
                {
                    //glMultMatrixf(&bird.MX_LocalToWorld_Get().x0);
                    glTranslatef(bird.P_center_Trfm.x,
                                 bird.P_center_Trfm.y,
                                 bird.P_center_Trfm.z);

                    float S_dist_Sqr = (P_eye - bird.P_center_Trfm).lengthSqr();
                    if (S_dist_Sqr > LOD_FarDistance_Sqr)
                        glCallList(ListBirdFar);
                    else
                    if (S_dist_Sqr > LOD_LowDistance_Sqr)
                        glCallList(ListBirdLow);
                    else
                    if (S_dist_Sqr > LOD_MedDistance_Sqr)
                        glCallList(ListBirdMed);
                    else
                        glCallList(ListBirdHgh);
                }
                glPopMatrix();
            }
        }
    }
}


void MapRender:: RenderCity(const Math::Cameras::FieldOfView &fov)
{
    Profile("Render buildings");

    bool FL_showNorth, FL_showSouth, FL_showWest, FL_showEast, FL_showTop;

    // Determine bulding wall visibility
    const Math::Cameras::Camera &camera = *fov.Camera_Get();
    if (fov.Projection == Math::Cameras::FieldOfView::PROJECT_PERSPECTIVE)
    {
        xVector3 view1 = (fov.Corners3D[0] - camera.P_eye);
        xVector3 view2 = (fov.Corners3D[1] - camera.P_eye);
        xVector3 view3 = (fov.Corners3D[2] - camera.P_eye);
        xVector3 view4 = (fov.Corners3D[3] - camera.P_eye);

        FL_showNorth = view1.y < 0.f || view2.y < 0.f || view3.y < 0.f || view4.y < 0.f;
        FL_showSouth = view1.y > 0.f || view2.y > 0.f || view3.y > 0.f || view4.y > 0.f;
        FL_showWest  = view1.x > 0.f || view2.x > 0.f || view3.x > 0.f || view4.x > 0.f;
        FL_showEast  = view1.x < 0.f || view2.x < 0.f || view3.x < 0.f || view4.x < 0.f;
        FL_showTop   = view1.z < 0.f || view2.z < 0.f || view3.z < 0.f || view4.z < 0.f;
    }
    else // Orhogonal projection
    {
        xVector3 view1 = (camera.P_center - camera.P_eye);
        FL_showNorth = view1.y < 0.f;
        FL_showSouth = view1.y > 0.f;
        FL_showWest  = view1.x < 0.f;
        FL_showEast  = view1.x > 0.f;
        FL_showTop   = view1.z < 0.f;
    }

    // Load textures
    if (WallTexture.IsNull())
        WallTexture = g_TextureMgr.GetTexture("Data/textures/wall.tga");
    if (GroundTexture.IsNull())
        GroundTexture = g_TextureMgr.GetTexture("Data/textures/quarter.tga");
    if (WallTexture_Normal.IsNull())
        WallTexture_Normal = g_TextureMgr.GetTexture("Data/textures/wall_normal.tga");
    if (GroundTexture_Normal.IsNull())
        GroundTexture_Normal = g_TextureMgr.GetTexture("Data/textures/quarter_normal.tga");

    // Render districts
    float DistrictSize = Quarter::SquareWidth * Map::DistrictSize;
    xPoint3 districtCorners[8] = {
        xPoint3::Create(Map->MinX, Map->MaxY, 0.f),
        xPoint3::Create(Map->MinX, Map->MaxY, 0.f),
        xPoint3::Create(Map->MinX, Map->MaxY - DistrictSize, 0.f),
        xPoint3::Create(Map->MinX, Map->MaxY - DistrictSize, 0.f),
        xPoint3::Create(Map->MinX + DistrictSize, Map->MaxY, 0.f),
        xPoint3::Create(Map->MinX + DistrictSize, Map->MaxY, 0.f),
        xPoint3::Create(Map->MinX + DistrictSize, Map->MaxY - DistrictSize, 0.f),
        xPoint3::Create(Map->MinX + DistrictSize, Map->MaxY - DistrictSize, 0.f)
    };

    DistrictInfo *lists = L_districts;
    for (int r = 0; r < Map->Rows; r += Map::DistrictSize)
    {
        for (int i = 0; i < 4; ++i) districtCorners[i].x = Map->MinX;
        for (int i = 4; i < 8; ++i) districtCorners[i].x = Map->MinX + DistrictSize;

        for (int c = 0; c < Map->Cols; c += Map::DistrictSize, ++lists)
        {
            for (int i = 0; i < 8; i += 2) districtCorners[i].z = lists->maxHeight;

            if (lists->maxHeight == 0.f || fov.CheckPoints(districtCorners, 8))
                RenderDistrict(fov, *lists, c, r, Map::DistrictSize, Map::DistrictSize,
                    FL_showNorth, FL_showSouth, FL_showWest, FL_showEast, FL_showTop);

            for (int i = 0; i < 8; ++i) districtCorners[i].x += DistrictSize;
        }

        for (int i = 0; i < 8; ++i) districtCorners[i].y -= DistrictSize;
    }
}

void MapRender:: RenderDistrict(const Math::Cameras::FieldOfView &fov,

                                DistrictInfo &lists,
                                int s_col, int s_row, int s_cols, int s_rows,

                                bool FL_showNorth, bool FL_showSouth,
                                bool FL_showWest,  bool FL_showEast,
                                bool FL_showTop)
{
    int s_col_max = s_col + s_cols;
    int s_row_max = s_row + s_rows;
    if (s_col_max > Map->Cols)
    {
        s_col_max = Map->Cols;
        s_cols = s_col_max - s_col;
    }
    if (s_row_max > Map->Rows)
    {
        s_row_max = Map->Rows;
        s_rows = s_row_max - s_row;
    }

    Profile("Render district");

    glMaterialf (GL_FRONT, GL_SHININESS, 2.0f);
    int IDtex = ((Graphics::OGL::Texture*) g_TextureMgr.GetResource(WallTexture).texture)->ID_GLTexture;
    Shader::UseDiffuseMap (IDtex);
    IDtex = ((Graphics::OGL::Texture*) g_TextureMgr.GetResource(WallTexture_Normal).texture)->ID_GLTexture;
    Shader::UseBumpMap    (IDtex);
    Shader::Start();

    if (FL_showNorth)
    {
        if (!lists.ListNorth)
        {
            lists.ListNorth = glGenLists(1);
            glNewList(lists.ListNorth, GL_COMPILE);
            {
                glNormal3f(0.f,1.f,0.f);
                glMultiTexCoord3f(GL_TEXTURE1, -1.f,0.f,0.f); // tangentS
                glMultiTexCoord3f(GL_TEXTURE2,  0.f,0.f,1.f); // tangentT
                glBegin(GL_QUADS);
                {
                    float x  = Map->MinX + Quarter::MarginWidth + s_col * Quarter::SquareWidth;
                    float y  = Map->MaxY - Quarter::MarginWidth - s_row * Quarter::SquareWidth;
                    float tx = Quarter::WallWidth * 0.2f;

                    for (int row = s_row; row < s_row_max; ++row, y -= Quarter::SquareWidth)
                    {
                        Quarter *QT_curr = Map->Grid + row * Map->Cols + s_col;
                        float x1 = x;
                        float x2 = x + Quarter::WallWidth;

                        for (int col = s_col; col < s_col_max; ++col, ++QT_curr,
                            x1 += Quarter::SquareWidth,
                            x2 += Quarter::SquareWidth)
                        {
                            if (QT_curr->Height == 0.f) continue;
                            if (QT_curr->Height > lists.maxHeight) lists.maxHeight = QT_curr->Height;
                            float ty = QT_curr->Height * 0.5f;

                            glColor3ubv(QT_curr->Color.rgb);
                            glTexCoord2f(0.f, 0.f); glVertex3f(x2, y, 0);
                            glTexCoord2f(tx,  0.f); glVertex3f(x1, y, 0);
                            glTexCoord2f(tx,  ty);  glVertex3f(x1, y, QT_curr->Height);
                            glTexCoord2f(0.f, ty);  glVertex3f(x2, y, QT_curr->Height);
                        }
                    }
                }
                glEnd();
            }
            glEndList();
        }
        glCallList(lists.ListNorth);
    }

    if (FL_showSouth)
    {
        if (!lists.ListSouth)
        {
            lists.ListSouth = glGenLists(1);
            glNewList(lists.ListSouth, GL_COMPILE);
            {
                glNormal3f(0.f,-1.f,0.f);
                glMultiTexCoord3f(GL_TEXTURE1, 1.f,0.f,0.f); // tangentS
                glMultiTexCoord3f(GL_TEXTURE2, 0.f,0.f,1.f); // tangentT
                glBegin(GL_QUADS);
                {
                    float x = Map->MaxX - Quarter::MarginWidth - Quarter::WallWidth - (Map->Cols - s_col_max) * Quarter::SquareWidth;
                    float y = Map->MinY + Quarter::MarginWidth + (Map->Rows - s_row_max) * Quarter::SquareWidth;
                    float tx = Quarter::WallWidth   * 0.2f;

                    for (int row = s_row_max-1; row >= s_row; --row, y += Quarter::SquareWidth)
                    {
                        Quarter *QT_curr = Map->Grid + row * Map->Cols + s_col_max - 1;
                        float x1 = x;
                        float x2 = x + Quarter::WallWidth;

                        for (int col = s_col; col < s_col_max; ++col, --QT_curr,
                            x1 -= Quarter::SquareWidth,
                            x2 -= Quarter::SquareWidth)
                        {
                            if (QT_curr->Height == 0.f) continue;
                            if (QT_curr->Height > lists.maxHeight) lists.maxHeight = QT_curr->Height;
                            float ty = QT_curr->Height * 0.5f;

                            glColor3ubv(QT_curr->Color.rgb);
                            glTexCoord2f(0.f, 0.f); glVertex3f(x1, y, 0);
                            glTexCoord2f(tx,  0.f); glVertex3f(x2, y, 0);
                            glTexCoord2f(tx,  ty);  glVertex3f(x2, y, QT_curr->Height);
                            glTexCoord2f(0.f, ty);  glVertex3f(x1, y, QT_curr->Height);
                        }
                    }
                }
                glEnd();
            }
            glEndList();
        }
        glCallList(lists.ListSouth);
    }

    if (FL_showWest)
    {
        if (!lists.ListWest)
        {
            lists.ListWest = glGenLists(1);
            glNewList(lists.ListWest, GL_COMPILE);
            {
                glNormal3f(-1.f,0.f,0.f);
                glMultiTexCoord3f(GL_TEXTURE1, 0.f,-1.f,0.f); // tangentS
                glMultiTexCoord3f(GL_TEXTURE2, 0.f, 0.f,1.f); // tangentT
                glBegin(GL_QUADS);
                {
                    float x = Map->MinX + Quarter::MarginWidth + s_col * Quarter::SquareWidth;
                    float y = Map->MaxY - Quarter::MarginWidth - Quarter::WallWidth - s_row * Quarter::SquareWidth;
                    float tx = Quarter::WallWidth   * 0.2f;

                    for (int col = s_col; col < s_col_max ; ++col, x += Quarter::SquareWidth)
                    {
                        Quarter *QT_curr = Map->Grid + s_row * Map->Cols + col;
                        float y1 = y;
                        float y2 = y + Quarter::WallWidth;

                        for (int row = s_row; row < s_row_max; ++row, QT_curr += Map->Cols,
                            y1 -= Quarter::SquareWidth,
                            y2 -= Quarter::SquareWidth)
                        {
                            if (QT_curr->Height == 0.f) continue;
                            if (QT_curr->Height > lists.maxHeight) lists.maxHeight = QT_curr->Height;
                            float ty = QT_curr->Height * 0.5f;

                            glColor3ubv(QT_curr->Color.rgb);
                            glTexCoord2f(tx,  0.f); glVertex3f(x, y2, 0);
                            glTexCoord2f(0.f, 0.f); glVertex3f(x, y1, 0);
                            glTexCoord2f(0.f, ty);  glVertex3f(x, y1, QT_curr->Height);
                            glTexCoord2f(tx,  ty);  glVertex3f(x, y2, QT_curr->Height);
                        }
                    }
                }
                glEnd();
            }
            glEndList();
        }
        glCallList(lists.ListWest);
    }

    if (FL_showEast)
    {
        if (!lists.ListEast)
        {
            lists.ListEast = glGenLists(1);
            glNewList(lists.ListEast, GL_COMPILE);
            {
                glNormal3f(1.f,0.f,0.f);
                glMultiTexCoord3f(GL_TEXTURE1, 0.f,1.f,0.f); // tangentS
                glMultiTexCoord3f(GL_TEXTURE2, 0.f,0.f,1.f); // tangentT
                glBegin(GL_QUADS);
                {
                    float x = Map->MaxX - Quarter::MarginWidth - (Map->Cols - s_col_max) * Quarter::SquareWidth;
                    float y = Map->MaxY - Quarter::MarginWidth - Quarter::WallWidth - s_row * Quarter::SquareWidth;
                    float tx = Quarter::WallWidth   * 0.2f;

                    for (int col = s_col_max-1; col >= s_col ; --col, x -= Quarter::SquareWidth)
                    {
                        Quarter *QT_curr = Map->Grid + s_row * Map->Cols + col;
                        float y1 = y;
                        float y2 = y + Quarter::WallWidth;

                        for (int row = s_row; row < s_row_max; ++row, QT_curr += Map->Cols,
                            y1 -= Quarter::SquareWidth,
                            y2 -= Quarter::SquareWidth)
                        {
                            if (QT_curr->Height == 0.f) continue;
                            if (QT_curr->Height > lists.maxHeight) lists.maxHeight = QT_curr->Height;
                            float ty = QT_curr->Height * 0.5f;

                            glColor3ubv(QT_curr->Color.rgb);
                            glTexCoord2f(tx,  0.f); glVertex3f(x, y1, 0);
                            glTexCoord2f(0.f, 0.f); glVertex3f(x, y2, 0);
                            glTexCoord2f(0.f, ty);  glVertex3f(x, y2, QT_curr->Height);
                            glTexCoord2f(tx,  ty);  glVertex3f(x, y1, QT_curr->Height);
                        }
                    }
                }
                glEnd();
            }
            glEndList();
        }
        glCallList(lists.ListEast);
    }

    if (FL_showTop)
    {
        // Render roofs
        Shader::UseDiffuseMap (0);
        Shader::UseBumpMap    (0);
        Shader::Start();

        if (!lists.ListTop)
        {
            lists.ListTop = glGenLists(1);
            glNewList(lists.ListTop, GL_COMPILE);
            {
                glNormal3f(0.f,0.f,1.f);
                glColor3ub(50,50,50);
                glBegin(GL_QUADS);
                {
                    float x  = Map->MinX + Quarter::MarginWidth + s_col * Quarter::SquareWidth;
                    float y1 = Map->MaxY - Quarter::MarginWidth - s_row * Quarter::SquareWidth;
                    float y2 = y1 - Quarter::WallWidth;

                    for (int row = s_row; row < s_row_max; ++row,
                        y1 -= Quarter::SquareWidth,
                        y2 -= Quarter::SquareWidth)
                    {
                        Quarter *QT_curr = Map->Grid + row * Map->Cols + s_col;
                        float x1 = x;
                        float x2 = x + Quarter::WallWidth;

                        for (int col = s_col; col < s_col_max; ++col, ++QT_curr,
                            x1 += Quarter::SquareWidth,
                            x2 += Quarter::SquareWidth)
                        {
                            if (QT_curr->Height == 0.f) continue;
                            if (QT_curr->Height > lists.maxHeight) lists.maxHeight = QT_curr->Height;

                            glVertex3f(x1, y1, QT_curr->Height);
                            glVertex3f(x1, y2, QT_curr->Height);
                            glVertex3f(x2, y2, QT_curr->Height);
                            glVertex3f(x2, y1, QT_curr->Height);
                        }
                    }

                }
                glEnd();
            }
            glEndList();
        }
        glCallList(lists.ListTop);

        // Render ground
        IDtex = ((Graphics::OGL::Texture*) g_TextureMgr.GetResource(GroundTexture).texture)->ID_GLTexture;
        Shader::UseDiffuseMap (IDtex);
        IDtex = ((Graphics::OGL::Texture*) g_TextureMgr.GetResource(GroundTexture_Normal).texture)->ID_GLTexture;
        Shader::UseBumpMap    (IDtex);
        Shader::Start();
        glNormal3f(0.f,0.f,1.f);
        glMultiTexCoord3f(GL_TEXTURE1, 1.f,0.f,0.f); // tangentS
        glMultiTexCoord3f(GL_TEXTURE2, 0.f,1.f,0.f); // tangentT
        glColor3ub(0,255,0);
        glBegin(GL_QUADS);
        {
            glTexCoord2i(0,      s_rows); glVertex3f(Map->MinX + s_col     * Quarter::SquareWidth, Map->MaxY - s_row_max * Quarter::SquareWidth, 0);
            glTexCoord2i(s_cols, s_rows); glVertex3f(Map->MinX + s_col_max * Quarter::SquareWidth, Map->MaxY - s_row_max * Quarter::SquareWidth, 0);
            glTexCoord2i(s_cols, 0);      glVertex3f(Map->MinX + s_col_max * Quarter::SquareWidth, Map->MaxY - s_row * Quarter::SquareWidth,     0);
            glTexCoord2i(0,      0);      glVertex3f(Map->MinX + s_col     * Quarter::SquareWidth, Map->MaxY - s_row * Quarter::SquareWidth,     0);
        }
        glEnd();
    }

    Shader::UseDiffuseMap (0);
    Shader::UseBumpMap    (0);
    Shader::Start();

    if (EPSILON > lists.maxHeight) lists.maxHeight = EPSILON;
}
