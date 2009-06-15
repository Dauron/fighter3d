#include "types.h"

using namespace Math;

// Multiply the Matrix by another Matrix
Matrix &Matrix::operator *= (const Matrix &m)
{
    Matrix res;
    for (Uint r = 0; r < sm_nRows; ++r)
    for (Uint c = 0; c < sm_nCols; ++c)
    {
        Float f = 0;
        f += (matrix[r][0] * m.matrix[0][c]);
        f += (matrix[r][1] * m.matrix[1][c]);
        f += (matrix[r][2] * m.matrix[2][c]);
        f += (matrix[r][3] * m.matrix[3][c]);
        res.matrix[r][c] = f;
    }
    return (*this = res);
}

Matrix &Matrix::preMultiply(const Matrix &m)
{
    Matrix res;
    for (Uint r = 0; r < sm_nRows; ++r)
    for (Uint c = 0; c < sm_nCols; ++c)
    {
        Float f = 0.f;
        f += (m.matrix[r][0] * matrix[0][c]);
        f += (m.matrix[r][1] * matrix[1][c]);
        f += (m.matrix[r][2] * matrix[2][c]);
        f += (m.matrix[r][3] * matrix[3][c]);
        res.matrix[r][c] = f;
    }
    return (*this = res);
}

Matrix &Matrix::postTranslate(const Vector3& pos)
{
    w0 += x0*pos.x + y0*pos.y + z0*pos.z;
    w1 += x1*pos.x + y1*pos.y + z1*pos.z;
    w2 += x2*pos.x + y2*pos.y + z2*pos.z;
    w3 += x3*pos.x + y3*pos.y + z3*pos.z;
    return *this;
}
Matrix &Matrix::preTranslate(const Vector3& pos)
{
    x0 += x3*pos.x; y0 += y3*pos.x; z0 += z3*pos.x; w0 += w3*pos.x;
    x1 += x3*pos.y; y1 += y3*pos.y; z1 += z3*pos.y; w1 += w3*pos.y;
    x2 += x3*pos.z; y2 += y3*pos.z; z2 += z3*pos.z; w2 += w3*pos.z;
    return *this;
}

Matrix &Matrix::postTranslateT(const Vector3& pos)
{
    x0 += w0*pos.x; y0 += w0*pos.y; z0 += w0*pos.z;
    x1 += w1*pos.x; y1 += w1*pos.y; z1 += w1*pos.z;
    x2 += w2*pos.x; y2 += w2*pos.y; z2 += w2*pos.z;
    x3 += w3*pos.x; y3 += w3*pos.y; z3 += w3*pos.z;
    return *this;
}
Matrix &Matrix::preTranslateT(const Vector3& pos)
{
    x3 += x0*pos.x + x1*pos.y + x2*pos.z;
    y3 += y0*pos.x + y1*pos.y + y2*pos.z;
    z3 += z0*pos.x + z1*pos.y + z2*pos.z;
    w3 += w0*pos.x + w1*pos.y + w2*pos.z;
    return *this;
}

// Transform point
Point3 Matrix::postTransformP(const Point3& point) const
{
    Vector3 res;
    res.x = x0*point.x + y0*point.y + z0*point.z + w0;
    res.y = x1*point.x + y1*point.y + z1*point.z + w1;
    res.z = x2*point.x + y2*point.y + z2*point.z + w2;
    Float w = x3*point.x + y3*point.y + z3*point.z + w3;
    if (w != 0.f && w != 1.f)
        res *= 1.f/w;
    return res;
}
Point3 Matrix::preTransformP(const Point3& point) const
{
    Vector3 res;
    res.x = x0*point.x + x1*point.y + x2*point.z + x3;
    res.y = y0*point.x + y1*point.y + y2*point.z + y3;
    res.z = z0*point.x + z1*point.y + z2*point.z + z3;
    Float w = w0*point.x + w1*point.y + w2*point.z + w3;
    if (w != 0.f && w != 1.f)
        res *= 1.f/w;
    return res;
}

// Transform vector
Vector3 Matrix::postTransformV(const Vector3& vec) const
{
    Vector3 res;
    res.x = x0*vec.x + y0*vec.y + z0*vec.z;
    res.y = x1*vec.x + y1*vec.y + z1*vec.z;
    res.z = x2*vec.x + y2*vec.y + z2*vec.z;
    Float w = x3*vec.x + y3*vec.y + z3*vec.z;
    if (w != 0.f && w != 1.f)
        res *= 1.f/w;
    return res;
}
Vector3 Matrix::preTransformV(const Vector3& vec) const
{
    Vector3 res;
    res.x = x0*vec.x + x1*vec.y + x2*vec.z;
    res.y = y0*vec.x + y1*vec.y + y2*vec.z;
    res.z = z0*vec.x + z1*vec.y + z2*vec.z;
    Float w = w0*vec.x + w1*vec.y + w2*vec.z;
    if (w != 0.f && w != 1.f)
        res *= 1.f/w;
    return res;
}

// Transpose the Matrix
Matrix &Matrix::transpose()
{
    for (Uint c = 0;     c < sm_nCols; ++c)
    for (Uint r = c + 1; r < sm_nRows; ++r)
    {
        Float t = matrix[r][c];
        matrix[r][c] = matrix[c][r];
        matrix[c][r] = t;
    }
    return *this;
}

// Invert the Matrix
Matrix &Matrix::invert()
{
    Matrix &a = *this;
    Matrix  b = Identity();

    Uint r, c;
    Uint cc;
    Uint rowMax; // Points to max abs value row in this column
    Float tmp;

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
            return(initIdentity());

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

Matrix &Matrix::initFromQuaternion(const Quaternion &q)
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

    x0 = 1.f - 2.f * (yy + zz);
    y0 =       2.f * (xy - zw);
    z0 =       2.f * (xz + yw);
    w0 =       0.f;
    x1 =       2.f * (xy + zw);
    y1 = 1.f - 2.f * (xx + zz);
    z1 =       2.f * (yz - xw);
    w1 =       0.f;
    x2 =       2.f * (xz - yw);
    y2 =       2.f * (yz + xw);
    z2 = 1.f - 2.f * (xx + yy);
    w2 =       0.f;
    x3 = y3 = z3 = 0.f;
    w3 =           1.f;
    return *this;
}

Matrix &Matrix::initFromQuaternionT(const Quaternion &q)
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

    x0 = 1.f - 2.f * (yy + zz);
    x1 =       2.f * (xy + zw);
    x2 =       2.f * (xz - yw);
    x3 =       0.f;
    y0 =       2.f * (xy - zw);
    y1 = 1.f - 2.f * (xx + zz);
    y2 =       2.f * (yz + xw);
    y3 =       0.f;
    z0 =       2.f * (xz + yw);
    z1 =       2.f * (yz - xw);
    z2 = 1.f - 2.f * (xx + yy);
    z3 =       0.f;
    w0 = w1 = w2 = 0.f;
    w3 =           1.f;
    return *this;
}

Quaternion Matrix::ToQuaternion() const
{
    const Matrix &m = *this;
    Quaternion    q;
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

    float angle = (q.w > 0 ? acos(q.w) : Pi-acos(q.w));
    float inv = angle > Pi*1.f/3.f ? Sign(q.w) : -1.f;
    Vector3 a = inv * q.vector3();
    a.normalize() *= sin(angle);
    return Quaternion(a, cos(angle));
}

Matrix &Matrix::initOrthogonal(const Vector3 &forward, const Vector3 &up)
{
    /* Side = forward x up */
    Vector3 &side     = Vector3::CrossProduct(forward, up).normalize();
    /* Recompute up as: forward = side x up */
    Vector3 &forward2 = Vector3::CrossProduct(up, side).normalize();
    /* Fill matrix */
    row0().init(side,       0.f);
    row1().init(forward2,   0.f);
    row2().init(up,         0.f);
    row3().init(0.f,0.f,0.f,1.f);
    return *this;
}

// Return a 3D axis-rotation Matrix
// Pass in 'x', 'y', or 'z' for the axis.
Matrix &Matrix::initAxisRotate(Float32 radX, Float32 radY, Float32 radZ)
{
    Matrix &ret = *this;
    Matrix  tmp;
    bool filled = false;
    Float sinA, cosA;

    if (!Math::IsZero(radX))
    {
        sinA = (Float)sin(radX);
        cosA = (Float)cos(radX);

        ret.matrix[0][0] =  1.0F; ret.matrix[1][0] =  0.0F; ret.matrix[2][0] =  0.0F;
        ret.matrix[0][1] =  0.0F; ret.matrix[1][1] =  cosA; ret.matrix[2][1] = -sinA;
        ret.matrix[0][2] =  0.0F; ret.matrix[1][2] =  sinA; ret.matrix[2][2] =  cosA;

        ret.matrix[0][3] =  0.0F; ret.matrix[1][3] =  0.0F; ret.matrix[2][3] =  0.0F;
        ret.matrix[3][0] =  0.0F; ret.matrix[3][1] =  0.0F; ret.matrix[3][2] =  0.0F; ret.matrix[3][3] = 1.0F;

        filled = true;
    }

    if (!Math::IsZero(radY))
    {
        sinA = (Float)sin(radY);
        cosA = (Float)cos(radY);

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

    if (!Math::IsZero(radZ))
    {
        sinA = (Float)sin(radZ);
        cosA = (Float)cos(radZ);

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

    if (!filled) initIdentity();
    return ret;
}

// Return a 3D translation Matrix
Matrix &Matrix::initTranslation(Float x, Float y, Float z)
{
  Matrix &ret = *this;
  ret.initIdentity();
  ret.w0 = x;
  ret.w1 = y;
  ret.w2 = z;
  return ret;
}
Matrix &Matrix::initTranslationT(Float x, Float y, Float z)
{
  Matrix &ret = *this;
  ret.initIdentity();
  ret.x3 = x;
  ret.y3 = y;
  ret.z3 = z;
  return ret;
}

