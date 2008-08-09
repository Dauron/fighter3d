union xMatrix {
    xFLOAT4x4 matrix;
    struct {
        xVector4 row[4];
    };
    struct {
        xVector4 row0;
        xVector4 row1;
        xVector4 row2;
        xVector4 row3;
    };
    struct {
        xFLOAT x0, y0, z0, w0;
        xFLOAT x1, y1, z1, w1;
        xFLOAT x2, y2, z2, w2;
        xFLOAT x3, y3, z3, w3;
    };

    xMatrix() {}
    xMatrix(const xMatrix &src) {
        *this = src;
    }

    // Array indexing
    xVector4 &operator [] (unsigned int i) {
        assert (i<4);
        return row[i];
    }
    const xVector4 &operator [] (unsigned int i) const {
        assert (i<4);
        return row[i];
    }

    // Assign
    xMatrix &operator = (const xMatrix &source) {
        memcpy(this, &source, sizeof(xMatrix));
        return *this;
    }
    // Add a xMatrix to this one
    xMatrix &operator += (const xMatrix &source) {
        xFLOAT *src = (xFLOAT*) &source - 1,
              *dst = (xFLOAT*) this - 1;
        for(int i=0; i < 4*4; ++i)
            *(++dst) += *(++src);
        return *this;
    }
    // Subtract a xMatrix from this one
    xMatrix &operator -= (const xMatrix &source) {
        xFLOAT *src = (xFLOAT*) &source - 1,
              *dst = (xFLOAT*) this - 1;
        for(int i=0; i < 4*4; ++i)
            *(++dst) -= *(++src);
        return *this;
    }
    // Multiply the xMatrix by another xMatrix
    xMatrix &operator *= (const xMatrix &m);
    // Multiply the xMatrix by a xFLOAT
    xMatrix &operator *= (xFLOAT scalar) {
        xFLOAT *dst = (xFLOAT*) this - 1;
        for(int i=0; i < 4*4; ++i)
            *(++dst) *= scalar;
        return *this;
    }
    // Are these two xMatrix's equal?
    friend bool operator == (const xMatrix &a, const xMatrix &b) {
        xFLOAT *src = (xFLOAT*) &a - 1,
               *dst = (xFLOAT*) &b - 1;
        for(int i=0; i < 4*4; ++i)
            if (*(++dst) != *(++src)) return false;
        return true;
    }
    // Are these two xMatrix's not equal?
    friend bool operator != (const xMatrix &a, const xMatrix &b) {
        xFLOAT *src = (xFLOAT*) &a - 1,
               *dst = (xFLOAT*) &b - 1;
        for(int i=0; i < 4*4; ++i)
            if (*(++dst) != *(++src)) return true;
        return false;
    }
    // Add two xMatrix's
    friend xMatrix operator + (const xMatrix &a, const xMatrix &b) {
        xMatrix ret(a);
        ret += b;
        return ret;
    }
    // Subtract one xMatrix from another
    friend xMatrix operator - (const xMatrix &a, const xMatrix &b) {
        xMatrix ret(a);
        ret -= b;
        return ret;
    }
    // Multiply xMatrix by another xMatrix
    friend xMatrix operator * (const xMatrix &a, const xMatrix &b) {
        xMatrix ret(a);
        ret *= b;
        return ret;
    }
    // Multiply xMatrix by a xVector3
    friend xVector3 operator * (const xMatrix &b, const xVector3 &a) {
        xVector4 ret; ret.init(a, 0.f);
        ret = b * ret;
        return ret.vector3;
    }
    // Multiply xVector3 by a xMatrix
    friend xVector3 operator * (const xVector3 &a, const xMatrix &b) {
        xVector4 ret; ret.init(a, 0.f);
        ret = ret * b;
        return ret.vector3;
    }
    // Multiply xMatrix by a xVector4
    friend xVector4 operator * (const xMatrix &a, const xVector4 &b) {
        xVector4 ret;
        ret.x = xVector4::DotProduct(a.row0,b);
        ret.y = xVector4::DotProduct(a.row1,b);
        ret.z = xVector4::DotProduct(a.row2,b);
        ret.w = xVector4::DotProduct(a.row3,b);
        return ret;
    }
    // Multiply xVector4 by a xMatrix
    friend xVector4 operator * (const xVector4 &a, const xMatrix &b) {
        xVector4 ret;
        ret.x = a.x * b.x0 + a.y * b.x1 + a.z * b.x2 + a.w * b.x3;
        ret.y = a.x * b.y0 + a.y * b.y1 + a.z * b.y2 + a.w * b.y3;
        ret.z = a.x * b.z0 + a.y * b.z1 + a.z * b.z2 + a.w * b.z3;
        ret.w = a.x * b.w0 + a.y * b.w1 + a.z * b.w2 + a.w * b.w3;
        return ret;
    }
    // Multiply xMatrix by a xFLOAT
    friend xMatrix operator * (const xMatrix &m, xFLOAT f) {
        xMatrix ret(m);
        ret *= f;
        return ret;
    }
    // Multiply xFLOAT by a xMatrix
    friend xMatrix operator * (xFLOAT f, const xMatrix &m) {
        xMatrix ret(m);
        ret *= f;
        return ret;
    }

    // Set xMatrix to the identity matrix
    xMatrix &identity() {
        memset(this, 0, sizeof(xMatrix));
        x0 = y1 = z2 = w3 = 1.0;
        return *this;
    }
    // Transpose the xMatrix
    xMatrix &transpose();
    // Invert the xMatrix
    xMatrix &invert();
    // PreMultiply xMatrix
    xMatrix &preMultiply(const xMatrix &m);
    // Translate xMatrix
    xMatrix &preTranslate  (const xVector3& pos);
    xMatrix &preTranslateT (const xVector3& pos);
    xMatrix &postTranslate (const xVector3& pos);
    xMatrix &postTranslateT(const xVector3& pos);

    // Transform point
    xPoint3 postTransformP(const xPoint3& point) const;
    xPoint3 preTransformP(const xPoint3& point) const;
    // Transform vector
    xVector3 postTransformV(const xVector3& vec) const;
    xVector3 preTransformV(const xVector3& vec) const;

    static xMatrix Identity()
    {
        xMatrix ret;
        return ret.identity();
    }
    static xMatrix Transpose(const xMatrix &m)
    {
        xMatrix ret(m);
        return ret.transpose();
    }
    static xMatrix Invert(const xMatrix &m)
    {
        xMatrix ret(m);
        return ret.invert();
    }
    static xMatrix &Invert(const xMatrix &src, xMatrix &dst)
    {
        dst = src;
        return dst.invert();
    }
};

xVector3 operator * (const xMatrix &a,  const xVector3 &b);
xVector3 operator * (const xVector3 &a, const xMatrix &b);
xMatrix  operator + (const xMatrix &a, const xMatrix &b);
xVector4 operator * (const xMatrix &a,  const xVector4 &b);
xVector4 operator * (const xVector4 &a, const xMatrix &b);
xMatrix  operator * (const xMatrix &a, const xMatrix &b);
xMatrix  operator * (const xMatrix &m, xFLOAT f);
xMatrix  operator * (xFLOAT f, const xMatrix &m);

bool operator == (const xMatrix &a, const xMatrix &b);
bool operator != (const xMatrix &a, const xMatrix &b);

xMatrix     xMatrixFromQuaternion(xQuaternion q);
xQuaternion xMatrixToQuaternion(const xMatrix &m);
xMatrix     xMatrixRotateRad(xFLOAT radX, xFLOAT radY, xFLOAT radZ);
xMatrix     xMatrixTranslate(xFLOAT x, xFLOAT y, xFLOAT z);
xMatrix     xMatrixTranslate(const xFLOAT3 xyz);
xMatrix     xMatrixTranslateT(xFLOAT x, xFLOAT y, xFLOAT z);
xMatrix     xMatrixTranslateT(const xFLOAT3 xyz);
xMatrix     xMatrixFromVectors(const xVector3 &forward, const xVector3 &up);

inline xMatrix xMatrixFromVectors(const xVector3 &forward, const xVector3 &up, const xVector3 &center)
{
    return xMatrixFromVectors(forward, up).postTranslateT(-center);
}
