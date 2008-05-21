union xVector4 {
    xFLOAT4 xyzw;
    xFLOAT4 col;
    struct {
        xFLOAT  x, y, z, w;
    };
    struct {
        xFLOAT r, g, b, a;
    };
    struct {
        xVector3 vector3;
    };

    //xVector4() {}
    //xVector4(xFLOAT X, xFLOAT Y, xFLOAT Z, xFLOAT W) : x(X), y(Y), z(Z), w(W) {}
    //xVector4(const xVector4 &src) {
    //    *this = src;
    //}
    //xVector4(const xVector3 &src) : w(0.f) {
    //    *this = src;
    //}
    //xVector4(const xVector3 &src, xFLOAT W) : w(W) {
    //    memcpy(this, &src, sizeof(xVector3));
    //}
    static xVector4 Create(xFLOAT X, xFLOAT Y, xFLOAT Z, xFLOAT W) { xVector4 res; return res.init(X,Y,Z,W); }
    static xVector4 Create(const xVector3 &src, xFLOAT W)          { xVector4 res; return res.init(src,W); }
    static xVector4 Create(const xVector3 &src)                    { xVector4 res; return res.init(src); }
    xVector4       &init  (xFLOAT X, xFLOAT Y, xFLOAT Z, xFLOAT W) { x = X; y = Y; z = Z; w = W; return *this; }
    xVector4       &init  (const xVector3 &src, xFLOAT W)          { vector3 = src; w = W;       return *this; }
    xVector4       &init  (const xVector3 &src)                    { vector3 = src; w = 0.f;     return *this; }
    xVector4       &init  (const xFLOAT   *src)                    { memcpy(this, src, 4*sizeof(xFLOAT)); return *this; }
    xVector4       &zero  ()                                       { x = y = z = w = 0.f;        return *this; }
    xVector4       &zeroQ ()                                       { x = y = z = 0.f; w = 1.f;   return *this; }


    // Array indexing
    xFLOAT &operator [] (unsigned int i) {
        assert (i<4);
        return xyzw[i];
    }
    const xFLOAT operator [] (unsigned int i) const {
        assert (i<4);
        return xyzw[i];
    }
    // Assign
    //xVector4 &operator = (const xVector4 &source) {
    //    memcpy(this, &source, sizeof(xVector4));
    //    return *this;
    //}
    //xVector4 &operator = (const xVector3 &source) {
    //    memcpy(this, &source, sizeof(xVector3));
    //    w = 0.0F;
    //    return *this;
    //}

    xVector4 &operator += (const xVector4 &v) {
        x += v.x; y += v.y; z += v.z; w += v.w;
        return *this;
    }
    xVector4 &operator -= (const xVector4 &v) {
        x -= v.x; y -= v.y; z -= v.z; w -= v.w;
        return *this;
    }
    xVector4 &operator *= (float f) {
        x *= f; y *= f; z *= f; w *= f;
        return *this;
    }
    xVector4 &operator /= (float f) {
        x /= f; y /= f; z /= f; w /= f;
        return *this;
    }
    friend xVector4 operator + (const xVector4 &a, const xVector4 &b) {
        xVector4 ret(a);
        ret += b;
        return ret;
    }
    friend xVector4 operator - (const xVector4 &a, const xVector4 &b) {
        xVector4 ret(a);
        ret -= b;
        return ret;
    }
    friend xVector4 operator * (const xVector4 &v, float f) {
        return xVector4::Create(f * v.x, f * v.y, f * v.z, f * v.w);
    }
    friend xVector4 operator * (float f, const xVector4 &v) {
        return xVector4::Create(f * v.x, f * v.y, f * v.z, f * v.w);
    }
    friend xVector4 operator / (const xVector4 &v, float f) {
        return xVector4::Create(v.x / f, v.y / f, v.z / f, v.w / f);
    }
    friend xVector4 operator - (const xVector4 &a) {
        return xVector4::Create(-a.x, -a.y, -a.z, -a.w);
    }

    xFLOAT length() const {
        return (xFLOAT) sqrt(x*x + y*y + z*z + w*w);
    }
    xVector4 &invert() {
        x = -x; y = -y; z = -z; w = -w;
        return *this;
    }
    xVector4 &normalize() {
        xFLOAT m = length();
        m = (m > 0.0F) ? 1.0F / m : 0.0F;
        x *= m; y *= m; z *= m; w *= m;
        return *this;
    }
    static xVector4 Normalize(const xVector4 &a) {
        xVector4 res = a;
        return res.normalize();
    }
    static xFLOAT DotProduct(const xVector4 &a, const xVector4 &b)
    {
        return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
    }

    // Set xMatrix to the identity matrix
    xVector4 &zeroQuaternion() {
        memset(this, 0, sizeof(xVector4)-sizeof(w)); w = 1.0;
        return *this;
    }

    xVector4 &planeFromPoints(const xVector3 &p0, const xVector3 &p1, const xVector3 &p2)
    {
        this->vector3 = xVector3::CrossProduct( p1-p0, p2-p0 ).normalize();
        this->w = - xVector3::DotProduct( p0, this->vector3 );
        return *this;
    }
};

xVector4 operator + (const xVector4 &a, const xVector4 &b);
xVector4 operator - (const xVector4 &a, const xVector4 &b);
xVector4 operator * (float f, const xVector4 &v);
xVector4 operator * (const xVector4 &v, float f);
xVector4 operator / (const xVector4 &v, float f);
xVector4 operator - (const xVector4 &a);
