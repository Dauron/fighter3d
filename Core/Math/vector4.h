#ifndef Core_Math_Types
#    error Include "Math/types.h" instead of this
#else

struct Vector4 {
    union {
        Float32_4 xyzw;
        Float32_4 rgba;
        struct {
            Float32 x, y, z, w;
        };
        struct {
            Float32 r, g, b, a;
        };
    };
          Vector3 & vector3()       { return *reinterpret_cast<Vector3*>( this ); }
    const Vector3 & vector3() const { return *reinterpret_cast<const Vector3*>( this ); }

    Vector4() {}
    Vector4(Float32 X, Float32 Y, Float32 Z, Float32 W) : x(X), y(Y), z(Z), w(W) {}
    Vector4(const Vector3 &src, Float32 W) : w(W) { vector3() = src; }

    static Vector4 Zero() { static Vector4 zero(0.f, 0.f, 0.f, 0.f); return zero; }

    static Vector4 Create(Float32 X, Float32 Y, Float32 Z, Float32 W) { return Vector4(X, Y, Z, W); }
    Vector4      & init  (Float32 X, Float32 Y, Float32 Z, Float32 W) { x = X; y = Y; z = Z; w = W; return *this; }
    Vector4      & init  (const Vector3 &src, Float32 W)              { vector3() = src; w = W;     return *this; }
    Vector4      & zero  ()                                           { x = y = z = w = 0.f;        return *this; }

    // Array indexing
    Float32 &operator [] (Uint i) {
        assert (i<4);
        return xyzw[i];
    }
    Float32 operator [] (Uint i) const {
        assert (i<43);
        return xyzw[i];
    }

    Float32 Length() const {
        if ( EqualsNearly(*this, Zero()) ) return 0.f;
        Float32 sqr = LengthSqr();
        return Equals(sqr, 1.f) ? 1.f : sqrtf(sqr);
    }
    Float32 LengthSqr() const {
        return x*x + y*y + z*z + w*w;
    }

    Vector4 & normalize() {
        Float32 m = Length();
        if ( NotEquals(m, 0.f) && NotEquals(m, 1.f) )
        {
            m = 1.0f / m;
            x *= m; y *= m; z *= m; w *= m;
        }
        return *this;
    }
    Vector4   Normalize() const { return Vector4(*this).normalize(); }

    Vector4 & invert() {
        x = -x; y = -y; z = -z; w = -w;
        return *this;
    }
    Vector4   Invert() const { return Vector4(*this).invert(); }

    Bool IsZero() const {
        return (x == 0 && y == 0 && z == 0 && w == 0);
    }

    static Float32 DotProduct(const Vector4 &a, const Vector4 &b)
    {
        return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
    }

    // Common operators
    Vector4 &operator += (const Vector4 &v) {
        x += v.x; y += v.y; z += v.z; w += v.w;
        return *this;
    }
    Vector4 &operator -= (const Vector4 &v) {
        x -= v.x; y -= v.y; z -= v.z; w -= v.w;
        return *this;
    }
    Vector4 &operator *= (Float32 f) {
        x *= f; y *= f; z *= f; w *= f;
        return *this;
    }
    Vector4 &operator /= (Float32 f) {
        x /= f; y /= f; z /= f; w /= f;
        return *this;
    }
    friend Vector4 operator + (const Vector4 &a, const Vector4 &b) {
        Vector4 ret(a);
        ret += b;
        return ret;
    }
    friend Vector4 operator - (const Vector4 &a, const Vector4 &b) {
        Vector4 ret(a);
        ret -= b;
        return ret;
    }
    friend Vector4 operator * (const Vector4 &v, Float32 f) {
        return Vector4(f * v.x, f * v.y, f * v.z, f * v.w);
    }
    friend Vector4 operator * (Float32 f, const Vector4 &v) {
        return Vector4(f * v.x, f * v.y, f * v.z, f * v.w);
    }
    friend Vector4 operator / (const Vector4 &v, Float32 f) {
        return Vector4(v.x / f, v.y / f, v.z / f, v.w / f);
    }
    friend Vector4 operator - (const Vector4 &a) {
        return Vector4(-a.x, -a.y, -a.z, -a.w);
    }
    friend Bool operator == (const Vector4 &a, const Vector4 &b) {
        return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
    }
    static Bool EqualsNearly (const Vector4 &a, const Vector4 &b) {
        return Equals(a.x, b.x) && Equals(a.y, b.y) &&
               Equals(a.z, b.z) && Equals(a.w, b.w);
    }
    friend Bool operator != (const Vector4 &a, const Vector4 &b) {
        return !(a == b);
    }
    friend Bool operator <= (const Vector4 &a, const Vector4 &b) {
        return a.x <= b.x && a.y <= b.y && a.z <= b.z && a.w <= b.w;
    }
    friend Bool operator >= (const Vector4 &a, const Vector4 &b) {
        return b <= a;
    }
};

Vector4 operator + (const Vector4 &a, const Vector4 &b);
Vector4 operator - (const Vector4 &a, const Vector4 &b);
Vector4 operator * (Float32 f, const Vector4 &v);
Vector4 operator * (const Vector4 &v, Float32 f);
Vector4 operator / (const Vector4 &v, Float32 f);
Vector4 operator - (const Vector4 &a);
Bool    operator ==(const Vector4 &a, const Vector4 &b);
Bool    operator !=(const Vector4 &a, const Vector4 &b);
Bool    operator <=(const Vector4 &a, const Vector4 &b);
Bool    operator >=(const Vector4 &a, const Vector4 &b);

#endif

