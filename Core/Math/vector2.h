#ifndef Core_Math_Types
#    error Include "Math/types.h" instead of this
#else

struct Vector2 {
    union {
        Float32_2 xy;
        Float32_2 uv;
        struct {
            Float32 x, y;
        };
        struct {
            Float32 u, v;
        };
    };

    Vector2() {}
    Vector2(Float32 X, Float32 Y) : x(X), y(Y) {}

    static Vector2 Zero() { static Vector2 zero(0.f, 0.f); return zero; }

    static Vector2 Create(Float32 X, Float32 Y) { return Vector2(X, Y); }
    Vector2      & init  (Float32 X, Float32 Y) { x = X; y = Y; return *this; }
    Vector2      & zero  ()                     { x = y = 0.f;  return *this; }

    // Array indexing
    Float32 &operator [] (Uint i) {
        assert (i<2);
        return xy[i];
    }
    Float32 operator [] (Uint i) const {
        assert (i<2);
        return xy[i];
    }

    Float32 Length() const {
        if ( EqualsNearly(*this, Zero()) ) return 0.f;
        Float32 sqr = LengthSqr();
        return Equals(sqr, 1.f) ? 1.f : sqrtf(sqr);
    }
    Float32 LengthSqr() const {
        return x*x + y*y;
    }

    Vector2 &normalize() {
        Float32 m = Length();
        if ( NotEquals(m, 0.f) && NotEquals(m, 1.f) )
        {
            m = 1.0f / m;
            x *= m; y *= m;
        }
        return *this;
    }
    Vector2 Normalize() const { return Vector2(*this).normalize(); }

    Vector2 &invert() {
        x = -x; y = -y;
        return *this;
    }
    Vector2 Invert() const { return Vector2(*this).invert(); }

    Vector2 & orthogonal() { Float32 tmp = x; x = -y; y = tmp;  return *this; }
    Vector2   Orthogonal() { return Vector2(-y, x); }

    Bool IsZero() const {
        return (x == 0 && y == 0);
    }
    static Float32 DotProduct(const Vector2 &a, const Vector2 &b)
    {
        return a.x*b.x + a.y*b.y;
    }

    // Common operators
    Vector2 &operator += (const Vector2 &v) {
        x += v.x; y += v.y;
        return *this;
    }
    Vector2 &operator -= (const Vector2 &v) {
        x -= v.x; y -= v.y;
        return *this;
    }
    Vector2 &operator += (Float32 f) {
        x += f; y += f;
        return *this;
    }
    Vector2 &operator *= (Float32 f) {
        x *= f; y *= f;
        return *this;
    }
    Vector2 &operator /= (Float32 f) {
        *this *= 1.f/f;
        return *this;
    }
    friend Vector2 operator + (const Vector2 &a, const Vector2 &b) {
        Vector2 ret(a);
        ret += b;
        return ret;
    }
    friend Vector2 operator - (const Vector2 &a, const Vector2 &b) {
        Vector2 ret(a);
        ret -= b;
        return ret;
    }
    friend Vector2 operator * (const Vector2 &v, Float32 f) {
        return Vector2::Create(f * v.x, f * v.y);
    }
    friend Vector2 operator * (Float32 f, const Vector2 &v) {
        return Vector2::Create(f * v.x, f * v.y);
    }
    friend Vector2 operator / (const Vector2 &v, Float32 f) {
        return Vector2::Create(v.x / f, v.y / f);
    }
    friend Vector2 operator - (const Vector2 &a) {
        return Vector2::Create(-a.x, -a.y);
    }
    friend Bool operator == (const Vector2 &a, const Vector2 &b) {
        return a.x == b.x && a.y == b.y;
    }
    static Bool EqualsNearly (const Vector2 &a, const Vector2 &b) {
        return Equals(a.x, b.x) && Equals(a.y, b.y);
    }
    friend Bool operator != (const Vector2 &a, const Vector2 &b) {
        return !(a == b);
    }
    friend Bool operator <= (const Vector2 &a, const Vector2 &b) {
        return a.x <= b.x && a.y <= b.y;
    }
    friend Bool operator >= (const Vector2 &a, const Vector2 &b) {
        return b <= a;
    }
};

Vector2  operator + (const Vector2 &a, const Vector2 &b);
Vector2  operator - (const Vector2 &a, const Vector2 &b);
Vector2  operator * (Float32 f, const Vector2 &v);
Vector2  operator * (const Vector2 &v, Float32 f);
Vector2  operator / (const Vector2 &v, Float32 f);
Vector2  operator - (const Vector2 &a);
Bool     operator ==(const Vector2 &a, const Vector2 &b);
Bool     operator !=(const Vector2 &a, const Vector2 &b);
Bool     operator <=(const Vector2 &a, const Vector2 &b);
Bool     operator >=(const Vector2 &a, const Vector2 &b);

#endif

