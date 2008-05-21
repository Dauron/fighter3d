union xVector3 {
    xFLOAT3 xyz;
    struct {
        xFLOAT x, y, z;
    };

//    xVector3() {}
//    xVector3(xFLOAT X, xFLOAT Y, xFLOAT Z) : x(X), y(Y), z(Z) {}
//    xVector3(const xVector3 &src) {
//        *this = src;
//    }
    static xVector3 Create(xFLOAT X, xFLOAT Y, xFLOAT Z) { xVector3 res;        return res.init(X,Y,Z); }
    xVector3       &init  (xFLOAT X, xFLOAT Y, xFLOAT Z) { x = X; y = Y; z = Z; return *this; }
    xVector3       &zero  ()                             { x = y = z = 0.f;     return *this; }

    // Array indexing
    xFLOAT &operator [] (unsigned int i) {
        assert (i<3);
        return xyz[i];
    }
    const xFLOAT operator [] (unsigned int i) const {
        assert (i<3);
        return xyz[i];
    }

    // Assign
//    xVector3 &operator = (const xVector3 &source) {
//        memcpy(this, &source, sizeof(xVector3));
//        return *this;
//    }
    void ToArray(xFLOAT3 arr)
    {
        memcpy(arr, this, sizeof(xFLOAT3));
    }

    xFLOAT length() const {
        return (xFLOAT) sqrt(x*x + y*y + z*z);
    }
    xVector3 &normalize() {
        xFLOAT m = length();
        m = (m > 0.0F) ? 1.0F / m : 0.0F;
        x *= m; y *= m; z *= m;
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
        x /= f; y /= f; z /= f;
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
    friend bool operator <= (const xVector3 &a, const xVector3 &b) {
        return a.x <= b.x && a.y <= b.y && a.z <= b.z;
    }
    friend bool operator >= (const xVector3 &a, const xVector3 &b) {
        return b <= a;
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
