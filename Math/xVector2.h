struct xVector2 {
    union {
        xFLOAT2 xy;
        xFLOAT2 uv;
        struct {
            xFLOAT x, y;
        };
        struct {
            xFLOAT u, v;
        };
    };

    static xVector2 Create(xFLOAT X, xFLOAT Y) { xVector2 res; return res.init(X,Y); }
    xVector2       &init  (xFLOAT X, xFLOAT Y) { x = X; y = Y; return *this; }
    xVector2       &zero  ()                   { x = y = 0.f;  return *this; }

    // Array indexing
    xFLOAT &operator [] (unsigned int i) {
        assert (i<2);
        return xy[i];
    }
    const xFLOAT operator [] (unsigned int i) const {
        assert (i<2);
        return xy[i];
    }

    void ToArray(xFLOAT2 arr) const {
        memcpy(arr, this, sizeof(xFLOAT2));
    }

    xFLOAT length() const {
        if (isZero()) return 0.f;
        xFLOAT sqr = x*x + y*y;
        return (sqr < 1.f - EPSILON || sqr > 1.f + EPSILON) ? sqrtf(sqr) : 1.f;
    }
    xFLOAT lengthSqr() const {
        return x*x + y*y;
    }
    xVector2 &normalize() {
        xFLOAT m = length();
        if (m != 0.f && m != 1.f)
        {
            m = 1.0f / m;
            x *= m; y *= m;
        }
        return *this;
    }
    xVector2 &invert() {
        x = -x; y = -y;
        return *this;
    }

    xVector2 &operator += (const xVector2 &v) {
        x += v.x; y += v.y;
        return *this;
    }
    xVector2 &operator -= (const xVector2 &v) {
        x -= v.x; y -= v.y;
        return *this;
    }
    xVector2 &operator *= (float f) {
        x *= f; y *= f;
        return *this;
    }
    xVector2 &operator /= (float f) {
        *this *= 1.f/f;
        return *this;
    }
    friend xVector2 operator + (const xVector2 &a, const xVector2 &b) {
        xVector2 ret(a);
        ret += b;
        return ret;
    }
    friend xVector2 operator - (const xVector2 &a, const xVector2 &b) {
        xVector2 ret(a);
        ret -= b;
        return ret;
    }
    friend xVector2 operator * (const xVector2 &v, float f) {
        return xVector2::Create(f * v.x, f * v.y);
    }
    friend xVector2 operator * (float f, const xVector2 &v) {
        return xVector2::Create(f * v.x, f * v.y);
    }
    friend xVector2 operator / (const xVector2 &v, float f) {
        return xVector2::Create(v.x / f, v.y / f);
    }
    friend xVector2 operator - (const xVector2 &a) {
        return xVector2::Create(-a.x, -a.y);
    }
    friend bool operator == (const xVector2 &a, const xVector2 &b) {
        return a.x == b.x && a.y == b.y;
    }
    friend bool operator != (const xVector2 &a, const xVector2 &b) {
        return !(a == b);
    }
    friend bool operator <= (const xVector2 &a, const xVector2 &b) {
        return a.x <= b.x && a.y <= b.y;
    }
    friend bool operator >= (const xVector2 &a, const xVector2 &b) {
        return b <= a;
    }

    bool nearlyEqual (const xVector2 &vec) const {
        return fabs(x - vec.x) < EPSILON2 &&
               fabs(y - vec.y) < EPSILON2;
    }
    bool isZero() const {
        return (x == 0 && y == 0);
    }
    static xVector2 Normalize(const xVector2 &a) {
        xVector2 res = a;
        return res.normalize();
    }
    static xFLOAT DotProduct(const xVector2 &a, const xVector2 &b)
    {
        return a.x*b.x + a.y*b.y;
    }
    static xVector2 Orthogonal(const xVector2 &source)
    {
        return xVector2::Create(-source.y, source.x);
    }
};

xVector2  operator + (const xVector2 &a, const xVector2 &b);
xVector2  operator - (const xVector2 &a, const xVector2 &b);
xVector2  operator * (float f, const xVector2 &v);
xVector2  operator * (const xVector2 &v, float f);
xVector2  operator / (const xVector2 &v, float f);
xVector2  operator - (const xVector2 &a);
bool      operator ==(const xVector2 &a, const xVector2 &b);
bool      operator !=(const xVector2 &a, const xVector2 &b);
bool      operator <=(const xVector2 &a, const xVector2 &b);
bool      operator >=(const xVector2 &a, const xVector2 &b);
