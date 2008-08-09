#include "xMath.h"

#define IsZero(a)   (fabs(a) <= EPSILON)

// Multiply the xMatrix by another xMatrix
xMatrix &xMatrix::operator *= (const xMatrix &m)
{
    xMatrix res;
    for (unsigned int r = 0; r < 4; ++r)
    for (unsigned int c = 0; c < 4; ++c)
    {
        xFLOAT f = 0;
        f += (matrix[r][0] * m.row0.xyzw[c]);
        f += (matrix[r][1] * m.row1.xyzw[c]);
        f += (matrix[r][2] * m.row2.xyzw[c]);
        f += (matrix[r][3] * m.row3.xyzw[c]);
        res.matrix[r][c] = f;
    }
    return (*this = res);
}

xMatrix &xMatrix::preMultiply(const xMatrix &m)
{
    xMatrix res;
    for (unsigned int r = 0; r < 4; ++r)
    for (unsigned int c = 0; c < 4; ++c)
    {
        xFLOAT f = 0.f;
        f += (m.matrix[r][0] * row0.xyzw[c]);
        f += (m.matrix[r][1] * row1.xyzw[c]);
        f += (m.matrix[r][2] * row2.xyzw[c]);
        f += (m.matrix[r][3] * row3.xyzw[c]);
        res.matrix[r][c] = f;
    }
    return (*this = res);
}

xMatrix &xMatrix::postTranslate(const xVector3& pos)
{
    w0 += x0*pos.x + y0*pos.y + z0*pos.z;
    w1 += x1*pos.x + y1*pos.y + z1*pos.z;
    w2 += x2*pos.x + y2*pos.y + z2*pos.z;
    w3 += x3*pos.x + y3*pos.y + z3*pos.z;
    return *this;
}
xMatrix &xMatrix::preTranslate(const xVector3& pos)
{
    x0 += x3*pos.x; y0 += y3*pos.x; z0 += z3*pos.x; w0 += w3*pos.x;
    x1 += x3*pos.y; y1 += y3*pos.y; z1 += z3*pos.y; w1 += w3*pos.y;
    x2 += x3*pos.z; y2 += y3*pos.z; z2 += z3*pos.z; w2 += w3*pos.z;
    return *this;
}

xMatrix &xMatrix::postTranslateT(const xVector3& pos)
{
    x0 += w0*pos.x; y0 += w0*pos.y; z0 += w0*pos.z;
    x1 += w1*pos.x; y1 += w1*pos.y; z1 += w1*pos.z;
    x2 += w2*pos.x; y2 += w2*pos.y; z2 += w2*pos.z;
    x3 += w3*pos.x; y3 += w3*pos.y; z3 += w3*pos.z;
    return *this;
}
xMatrix &xMatrix::preTranslateT(const xVector3& pos)
{
    x3 += x0*pos.x + x1*pos.y + x2*pos.z;
    y3 += y0*pos.x + y1*pos.y + y2*pos.z;
    z3 += z0*pos.x + z1*pos.y + z2*pos.z;
    w3 += w0*pos.x + w1*pos.y + w2*pos.z;
    return *this;
}

// Transform point
xPoint3 xMatrix::postTransformP(const xPoint3& point) const
{
    xVector3 res;
    res.x = x0*point.x + y0*point.y + z0*point.z + w0;
    res.y = x1*point.x + y1*point.y + z1*point.z + w1;
    res.z = x2*point.x + y2*point.y + z2*point.z + w2;
    xFLOAT w = x3*point.x + y3*point.y + z3*point.z + w3;
    if (w != 0.f && w != 1.f)
        res *= 1.f/w;
    return res;
}
xPoint3 xMatrix::preTransformP(const xPoint3& point) const
{
    xVector3 res;
    res.x = x0*point.x + x1*point.y + x2*point.z + x3;
    res.y = y0*point.x + y1*point.y + y2*point.z + y3;
    res.z = z0*point.x + z1*point.y + z2*point.z + z3;
    xFLOAT w = w0*point.x + w1*point.y + w2*point.z + w3;
    if (w != 0.f && w != 1.f)
        res *= 1.f/w;
    return res;
}

// Transform vector
xVector3 xMatrix::postTransformV(const xVector3& vec) const
{
    xVector3 res;
    res.x = x0*vec.x + y0*vec.y + z0*vec.z;
    res.y = x1*vec.x + y1*vec.y + z1*vec.z;
    res.z = x2*vec.x + y2*vec.y + z2*vec.z;
    xFLOAT w = x3*vec.x + y3*vec.y + z3*vec.z;
    if (w != 0.f && w != 1.f)
        res *= 1.f/w;
    return res;
}
xVector3 xMatrix::preTransformV(const xVector3& vec) const
{
    xVector3 res;
    res.x = x0*vec.x + x1*vec.y + x2*vec.z;
    res.y = y0*vec.x + y1*vec.y + y2*vec.z;
    res.z = z0*vec.x + z1*vec.y + z2*vec.z;
    xFLOAT w = w0*vec.x + w1*vec.y + w2*vec.z;
    if (w != 0.f && w != 1.f)
        res *= 1.f/w;
    return res;
}

// Transpose the xMatrix
xMatrix &xMatrix::transpose()
{
    for (unsigned int c = 0; c < 4; ++c)
    for (unsigned int r = c + 1; r < 4; ++r)
    {
        xFLOAT t = matrix[r][c];
        matrix[r][c] = matrix[c][r];
        matrix[c][r] = t;
    }
    return *this;
}

// Invert the xMatrix
xMatrix &xMatrix::invert()
{
    xMatrix &a = *this;
    xMatrix  b; b.identity();

    unsigned int r, c;
    unsigned int cc;
    unsigned int rowMax; // Points to max abs value row in this column
    xFLOAT tmp;

    // Go through columns
    for (c=0; c<4; ++c)
    {
        // Find the row with max value in this column
        rowMax = c;
        for (r=c+1; r<4; ++r)
            if (fabs(a.matrix[r][c]) > fabs(a.matrix[rowMax][c]))
                rowMax = r;

        // If the max value here is 0, we can't invert.  Return identity.
        if (a.matrix[rowMax][c] == 0.0F)
            return(identity());

        // Swap row "rowMax" with row "c"
        if (rowMax != c)
            for (cc=0; cc<4; ++cc)
            {
                tmp = a.matrix[c][cc];
                a.matrix[c][cc] = a.matrix[rowMax][cc];
                a.matrix[rowMax][cc] = tmp;
                tmp = b.matrix[c][cc];
                b.matrix[c][cc] = b.matrix[rowMax][cc];
                b.matrix[rowMax][cc] = tmp;
            }

        // Now everything we do is on row "c".
        // Set the max cell to 1 by dividing the entire row by that value
        tmp = a.matrix[c][c];
        for (cc=0; cc<4; ++cc)
        {
            a.matrix[c][cc] /= tmp;
            b.matrix[c][cc] /= tmp;
        }

        // Now do the other rows, so that this column only has a 1 and 0's
        for (r = 0; r < 4; ++r)
            if (r != c)
            {
                tmp = a.matrix[r][c];
                for (cc=0; cc<4; ++cc)
                {
                    a.matrix[r][cc] -= a.matrix[c][cc] * tmp;
                    b.matrix[r][cc] -= b.matrix[c][cc] * tmp;
                }
            }
    }
    return (*this = b);
}

xMatrix xMatrixFromQuaternion(xQuaternion q)
{
    float xx = q.x * q.x;
    float xy = q.x * q.y;
    float xz = q.x * q.z;
    float xw = q.x * q.w;
    float yy = q.y * q.y;
    float yz = q.y * q.z;
    float yw = q.y * q.w;
    float zz = q.z * q.z;
    float zw = q.z * q.w;

    xMatrix res;
    res.row0.init(
             1.f - 2.f * (yy + zz),
                   2.f * (xy - zw),
                   2.f * (xz + yw),
                   0.f);
    res.row1.init(
                   2.f * (xy + zw),
             1.f - 2.f * (xx + zz),
                   2.f * (yz - xw),
                   0.f);
    res.row2.init(
                   2.f * (xz - yw),
                   2.f * (yz + xw),
             1.f - 2.f * (xx + yy),
                   0.f);
    res.row3.init(0.f, 0.f, 0.f, 1.f);
    return res;
}

xQuaternion xMatrixToQuaternion(const xMatrix &m)
{
    xQuaternion q;
    /*
    This code can be optimized for m[kW][kW] = 1, which
    should always be true.  This optimization is excluded
    here for clarity.
    */
    double Tr = m.x0 + m.y1 + m.z2 + m.w3, fourD;
    int i,j,k;
    /* w >= 0.5 ? */
    if (Tr >= 1.0)
    {
    fourD = 2.0*sqrt(Tr);
        q.w = fourD/4.0;
    q.x = (m.z1 - m.y2)/fourD;
    q.y = (m.x2 - m.z0)/fourD;
    q.z = (m.y0 - m.x1)/fourD;
    }
    else
    {
    /* Find the largest component. */
    if (m.x0 > m.y1)
        i = 0;
    else
        i = 1;
        if (m.z2 > m.matrix[i][i])
        i = 2;

    /* Set j and k to point to the next two components */
    j = (i+1)%3;
    k = (j+1)%3;
    /* fourD = 4 * largest component */
        fourD = 2.0*sqrt(m.matrix[i][i] - m.matrix[j][j] - m.matrix[k][k] + 1.0 );
    /* Set the largest component */
        q.xyzw[i] = fourD/4.0;
    /* Calculate remaining components */
    q.xyzw[j]  = (m.matrix[j][i] + m.matrix[i][j])/fourD;
        q.xyzw[k]  = (m.matrix[k][i] + m.matrix[i][k])/fourD;
        q.w = (m.matrix[k][j] - m.matrix[j][k])/fourD;
    }

    float angle = (q.w > 0 ? acos(q.w) : PI-acos(q.w));
    float inv = angle > PI*1.f/3.f ? Sign(q.w) : -1.f;
    xVector3 a = inv * q.vector3;
    a.normalize() *= sin(angle);
    return xQuaternion::Create(a, cos(angle));
}

xMatrix xMatrixFromVectors(const xVector3 &forward, const xVector3 &up)
{
    /* Side = forward x up */
    xVector3 &side     = xVector3::CrossProduct(forward, up).normalize();
    /* Recompute up as: forward = side x up */
    xVector3 &forward2 = xVector3::CrossProduct(up, side).normalize();
    /* Fill matrix */
    xMatrix transformation;
    transformation.x0 = side.x;     transformation.x1 = side.y;     transformation.x2 = side.z;
    transformation.y0 = forward2.x; transformation.y1 = forward2.y; transformation.y2 = forward2.z;
    transformation.z0 = up.x;       transformation.z1 = up.y;       transformation.z2 = up.z;
    transformation.w0 = transformation.w1 = transformation.w2 = 0.f;
    transformation.row3.init(0.f,0.f,0.f,1.f);
    return transformation.transpose();
}

// Return a 3D axis-rotation xMatrix
// Pass in 'x', 'y', or 'z' for the axis.
xMatrix xMatrixRotateRad(xFLOAT radX, xFLOAT radY, xFLOAT radZ)
{
    xMatrix ret;
    xMatrix tmp;
    bool filled = false;
    xFLOAT sinA, cosA;

    if (!IsZero(radX))
    {
        sinA = (xFLOAT)sin(radX);
        cosA = (xFLOAT)cos(radX);

        ret.matrix[0][0] =  1.0F; ret.matrix[1][0] =  0.0F; ret.matrix[2][0] =  0.0F;
        ret.matrix[0][1] =  0.0F; ret.matrix[1][1] =  cosA; ret.matrix[2][1] = -sinA;
        ret.matrix[0][2] =  0.0F; ret.matrix[1][2] =  sinA; ret.matrix[2][2] =  cosA;

        ret.matrix[0][3] =  0.0F; ret.matrix[1][3] =  0.0F; ret.matrix[2][3] =  0.0F;
        ret.matrix[3][0] =  0.0F; ret.matrix[3][1] =  0.0F; ret.matrix[3][2] =  0.0F; ret.matrix[3][3] = 1.0F;

        filled = true;
    }

    if (!IsZero(radY))
    {
        sinA = (xFLOAT)sin(radY);
        cosA = (xFLOAT)cos(radY);

        tmp.matrix[0][0] =  cosA; tmp.matrix[1][0] =  0.0F; tmp.matrix[2][0] =  sinA;
        tmp.matrix[0][1] =  0.0F; tmp.matrix[1][1] =  1.0F; tmp.matrix[2][1] =  0.0F;
        tmp.matrix[0][2] = -sinA; tmp.matrix[1][2] =  0.0F; tmp.matrix[2][2] =  cosA;

        tmp.matrix[0][3] =  0.0F; tmp.matrix[1][3] =  0.0F; tmp.matrix[2][3] =  0.0F;
        tmp.matrix[3][0] =  0.0F; tmp.matrix[3][1] =  0.0F; tmp.matrix[3][2] =  0.0F; tmp.matrix[3][3] = 1.0F;

        if (filled)
            ret *= tmp;
        else
        {
            ret = tmp;
            filled = true;
        }
    }

    if (!IsZero(radZ))
    {
        sinA = (xFLOAT)sin(radZ);
        cosA = (xFLOAT)cos(radZ);

        tmp.matrix[0][0] =  cosA; tmp.matrix[1][0] = -sinA; tmp.matrix[2][0] =  0.0F;
        tmp.matrix[0][1] =  sinA; tmp.matrix[1][1] =  cosA; tmp.matrix[2][1] =  0.0F;
        tmp.matrix[0][2] =  0.0F; tmp.matrix[1][2] =  0.0F; tmp.matrix[2][2] =  1.0F;

        tmp.matrix[0][3] =  0.0F; tmp.matrix[1][3] =  0.0F; tmp.matrix[2][3] =  0.0F;
        tmp.matrix[3][0] =  0.0F; tmp.matrix[3][1] =  0.0F; tmp.matrix[3][2] =  0.0F; tmp.matrix[3][3] = 1.0F;

        if (filled)
            ret *= tmp;
        else
        {
            ret = tmp;
            filled = true;
        }
    }

    if (!filled) ret.identity();

    return ret;
}

// Return a 3D translation xMatrix
xMatrix xMatrixTranslate(xFLOAT x, xFLOAT y, xFLOAT z)
{
  xMatrix ret;
  ret.identity();
  ret.w0 = x;
  ret.w1 = y;
  ret.w2 = z;
  return ret;
}
xMatrix xMatrixTranslate(const xFLOAT3 xyz)
{
  xMatrix ret;
  ret.identity();
  ret.w0 = xyz[0];
  ret.w1 = xyz[1];
  ret.w2 = xyz[2];
  return ret;
}
xMatrix xMatrixTranslateT(xFLOAT x, xFLOAT y, xFLOAT z)
{
  xMatrix ret;
  ret.identity();
  ret.x3 = x;
  ret.y3 = y;
  ret.z3 = z;
  return ret;
}
xMatrix xMatrixTranslateT(const xFLOAT3 xyz)
{
  xMatrix ret;
  ret.identity();
  ret.x3 = xyz[0];
  ret.y3 = xyz[1];
  ret.z3 = xyz[2];
  return ret;
}
