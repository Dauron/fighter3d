union xVector3 {
    xFLOAT3 xyz;
    xFLOAT3 rgb;
    xFLOAT3 uvw;
    struct {
        xFLOAT x, y, z;
    };
    struct {
        xFLOAT r, g, b;
    };
    struct {
        xFLOAT u, v, w;
    };
    xVector2 vector2;

//    xVector3() {}
//    xVector3(xFLOAT X, xFLOAT Y, xFLOAT Z) : x(X), y(Y), z(Z) {}
//    xVector3(const xVector3 &src) {
//        *this = src;
//    }
    static xVector3 Create(xFLOAT X, xFLOAT Y, xFLOAT Z) { xVector3 res; res.init(X,Y,Z); return res; }
    xVector3       &init  (xFLOAT X, xFLOAT Y, xFLOAT Z) { x = X; y = Y; z = Z; return *this; }
    xVector3       &zero  ()                             { x = y = z = 0.f;     return *this; }

    // Array indexing
    xFLOAT &operator [] (unsigned int i) {
        assert (i<3);
        return xyz[i];
    }
    xFLOAT operator [] (unsigned int i) const {
        assert (i<3);
        return xyz[i];
    }

    // Assign
//    xVector3 &operator = (const xVector3 &source) {
//        memcpy(this, &source, sizeof(xVector3));
//        return *this;
//    }
    void ToArray(xFLOAT3 arr) const {
        memcpy(arr, this, sizeof(xFLOAT3));
    }

    xFLOAT length() const {
        if (isZero()) return 0.f;
        xFLOAT sqr = x*x + y*y + z*z;
        return (sqr < 1.f - EPSILON || sqr > 1.f + EPSILON) ? sqrtf(sqr) : 1.f;
    }
    xFLOAT lengthSqr() const {
        return x*x + y*y + z*z;
    }
    xVector3 &normalize() {
        xFLOAT m = length();
        if (m != 0.f && m != 1.f)
        {
            m = 1.0f / m;
            x *= m; y *= m; z *= m;
        }
        return *this;
    }
    xVector3 &invert() {
        x = -x; y = -y; z = -z;
        return *this;
    }

    xVector3 &operator += (const xVector3 &v) {
        x += v.x; y += v.y; z += v.z;
        return *this;
    }
    xVector3 &operator -= (const xVector3 &v) {
        x -= v.x; y -= v.y; z -= v.z;
        return *this;
    }
    xVector3 &operator *= (float f) {
        x *= f; y *= f; z *= f;
        return *this;
    }
    xVector3 &operator /= (float f) {
        *this *= 1.f/f;
        return *this;
    }
    friend xVector3 operator + (const xVector3 &a, const xVector3 &b) {
        xVector3 ret(a);
        ret += b;
        return ret;
    }
    friend xVector3 operator - (const xVector3 &a, const xVector3 &b) {
        xVector3 ret(a);
        ret -= b;
        return ret;
    }
    friend xVector3 operator * (const xVector3 &v, float f) {
        return xVector3::Create(f * v.x, f * v.y, f * v.z);
    }
    friend xVector3 operator * (float f, const xVector3 &v) {
        return xVector3::Create(f * v.x, f * v.y, f * v.z);
    }
    friend xVector3 operator / (const xVector3 &v, float f) {
        return xVector3::Create(v.x / f, v.y / f, v.z / f);
    }
    friend xVector3 operator - (const xVector3 &a) {
        return xVector3::Create(-a.x, -a.y, -a.z);
    }
    friend bool operator == (const xVector3 &a, const xVector3 &b) {
        return a.x == b.x && a.y == b.y && a.z == b.z;
    }
    friend bool operator != (const xVector3 &a, const xVector3 &b) {
        return !(a == b);
    }
    friend bool operator <= (const xVector3 &a, const xVector3 &b) {
        return a.x <= b.x && a.y <= b.y && a.z <= b.z;
    }
    friend bool operator >= (const xVector3 &a, const xVector3 &b) {
        return b <= a;
    }

    bool nearlyEqual (const xVector3 &vec) const {
        return fabs(x - vec.x) < EPSILON2 &&
               fabs(y - vec.y) < EPSILON2 &&
               fabs(z - vec.z) < EPSILON2;
    }
    bool isZero() const {
        return (x == 0 && y == 0 && z == 0);
    }
    static xVector3 Normalize(const xVector3 &a) {
        xVector3 res = a;
        return res.normalize();
    }
    static xFLOAT DotProduct(const xVector3 &a, const xVector3 &b)
    {
        return a.x*b.x + a.y*b.y + a.z*b.z;
    }
    static xVector3 CrossProduct(const xVector3 &a, const xVector3 &b)
    {
        return xVector3::Create(a.y*b.z - a.z*b.y,
                                a.z*b.x - a.x*b.z,
                                a.x*b.y - a.y*b.x);
    }
    static xVector3 Orthogonal(const xVector3 &source)
    {
        if (IsZero(source.x)) return xVector3::Create(1.f,0.f,0.f);
        if (IsZero(source.y)) return xVector3::Create(0.f,1.f,0.f);
        if (IsZero(source.z)) return xVector3::Create(0.f,0.f,1.f);
        return xVector3::Create(source.y*source.z, source.x*source.z, -2.f*source.x*source.y);
        //xVector3 dest(source.x, source.y, 0.f);
        //dest.z = -(source.x*source.x+source.y*source.y)/source.z;
        //return dest;
    }
};

xVector3  operator + (const xVector3 &a, const xVector3 &b);
xVector3  operator - (const xVector3 &a, const xVector3 &b);
xVector3  operator * (float f, const xVector3 &v);
xVector3  operator * (const xVector3 &v, float f);
xVector3  operator / (const xVector3 &v, float f);
xVector3  operator - (const xVector3 &a);
bool      operator ==(const xVector3 &a, const xVector3 &b);
bool      operator !=(const xVector3 &a, const xVector3 &b);
bool      operator <=(const xVector3 &a, const xVector3 &b);
bool      operator >=(const xVector3 &a, const xVector3 &b);
