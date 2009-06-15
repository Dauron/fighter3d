#ifndef Core_Math_Types
#    error Include "Math/types.h" instead of this
#else

struct Vector3 {
    union {
        Float32_3 xyz;
        Float32_3 rgb;
        Float32_3 uvw;
        struct {
            Float32 x, y, z;
        };
        struct {
            Float32 r, g, b;
        };
        struct {
            Float32 u, v, w;
        };
    };
          Vector2 & vector2()       { return *reinterpret_cast<Vector2*>( this ); }
    const Vector2 & vector2() const { return *reinterpret_cast<const Vector2*>( this ); }

    Vector3() {}
    Vector3(Float32 X, Float32 Y, Float32 Z) : x(X), y(Y), z(Z) {}

    static Vector3 Zero() { static Vector3 zero(0.f, 0.f, 0.f); return zero; }

    static Vector3 Create(Float32 X, Float32 Y, Float32 Z) { return Vector3(X, Y, Z); }
    Vector3      & init  (Float32 X, Float32 Y, Float32 Z) { x = X; y = Y; z = Z; return *this; }
    Vector3      & zero  ()                                { x = y = z = 0.f;     return *this; }

    // Array indexing
    Float32 &operator [] (Uint i) {
        assert (i<3);
        return xyz[i];
    }
    Float32 operator [] (Uint i) const {
        assert (i<3);
        return xyz[i];
    }

    Float32 Length() const {
        if ( EqualsNearly(*this, Zero()) ) return 0.f;
        Float32 sqr = LengthSqr();
        return Equals(sqr, 1.f) ? 1.f : sqrtf(sqr);
    }
    Float32 LengthSqr() const {
        return x*x + y*y + z*z;
    }

    Vector3 & normalize() {
        Float32 m = Length();
        if ( NotEquals(m, 0.f) && NotEquals(m, 1.f) )
        {
            m = 1.0f / m;
            x *= m; y *= m; z *= m;
        }
        return *this;
    }
    Vector3  Normalize() const { return Vector3(*this).normalize(); }

    Vector3 & invert() {
        x = -x; y = -y; z = -z;
        return *this;
    }
    Vector3  Invert() const { return Vector3(*this).invert(); }

    Vector3 & orthogonal()
    {
        if (Math::IsZero(x)) return init(1.f,0.f,0.f);
        if (Math::IsZero(y)) return init(0.f,1.f,0.f);
        if (Math::IsZero(z)) return init(0.f,0.f,1.f);
        return init(y*z, x*z, -2.f*x*y);
    }
    Vector3    Orthogonal() { return Vector3(*this).orthogonal(); }

    Bool IsZero() const {
        return (x == 0 && y == 0 && z == 0);
    }

    static Float32 DotProduct(const Vector3 &a, const Vector3 &b)
    {
        return a.x*b.x + a.y*b.y + a.z*b.z;
    }
    static Vector3 CrossProduct(const Vector3 &a, const Vector3 &b)
    {
        return Vector3(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
    }

    // Common operators
    Vector3 &operator += (const Vector3 &v) {
        x += v.x; y += v.y; z += v.z;
        return *this;
    }
    Vector3 &operator -= (const Vector3 &v) {
        x -= v.x; y -= v.y; z -= v.z;
        return *this;
    }
    Vector3 &operator *= (Float32 f) {
        x *= f; y *= f; z *= f;
        return *this;
    }
    Vector3 &operator /= (Float32 f) {
        *this *= 1.f/f;
        return *this;
    }
    friend Vector3 operator + (const Vector3 &a, const Vector3 &b) {
        Vector3 ret(a);
        ret += b;
        return ret;
    }
    friend Vector3 operator - (const Vector3 &a, const Vector3 &b) {
        Vector3 ret(a);
        ret -= b;
        return ret;
    }
    friend Vector3 operator * (const Vector3 &v, Float32 f) {
        return Vector3(f * v.x, f * v.y, f * v.z);
    }
    friend Vector3 operator * (Float32 f, const Vector3 &v) {
        return Vector3(f * v.x, f * v.y, f * v.z);
    }
    friend Vector3 operator / (const Vector3 &v, Float32 f) {
        return Vector3(v.x / f, v.y / f, v.z / f);
    }
    friend Vector3 operator - (const Vector3 &a) {
        return Vector3(-a.x, -a.y, -a.z);
    }
    friend Bool operator == (const Vector3 &a, const Vector3 &b) {
        return a.x == b.x && a.y == b.y && a.z == b.z;
    }
    static Bool EqualsNearly (const Vector3 &a, const Vector3 &b) {
        return Equals(a.x, b.x) && Equals(a.y, b.y) && Equals(a.z, b.z);
    }
    friend Bool operator != (const Vector3 &a, const Vector3 &b) {
        return !(a == b);
    }
    friend Bool operator <= (const Vector3 &a, const Vector3 &b) {
        return a.x <= b.x && a.y <= b.y && a.z <= b.z;
    }
    friend Bool operator >= (const Vector3 &a, const Vector3 &b) {
        return b <= a;
    }
};

Vector3  operator + (const Vector3 &a, const Vector3 &b);
Vector3  operator - (const Vector3 &a, const Vector3 &b);
Vector3  operator * (Float32 f, const Vector3 &v);
Vector3  operator * (const Vector3 &v, Float32 f);
Vector3  operator / (const Vector3 &v, Float32 f);
Vector3  operator - (const Vector3 &a);
Bool     operator ==(const Vector3 &a, const Vector3 &b);
Bool     operator !=(const Vector3 &a, const Vector3 &b);
Bool     operator <=(const Vector3 &a, const Vector3 &b);
Bool     operator >=(const Vector3 &a, const Vector3 &b);

#endif

