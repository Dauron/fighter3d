#ifndef Core_Graphics_Types
#    error Include "Graphics/types.h" instead of this
#else

union Color4b {

    Uint8_4 rgba;
    struct {
        Uint8 r, g, b, a;
    };

    Color4b() {}
    Color4b(Uint8 R, Uint8 G, Uint8 B, Uint8 A = 0xFF) : r(R), g(G), b(B), a(A)             {}

    static Color4b Black() { static Color4b res(0x00,0x00,0x00,0xFF); return res; }
    static Color4b White() { static Color4b res(0xFF,0xFF,0xFF,0xFF); return res; }

    static Color4b Create(Uint8 R, Uint8 G, Uint8 B, Uint8 A = 0xFF) { return Color4b(R, G, B, A); }
    Color4b      & init  (Uint8 R, Uint8 G, Uint8 B, Uint8 A = 0xFF) { r = R; g = G; b = B; a = A; return *this; }

    Color4b      & black ()                          { r = g = b = 0x00; a = 0xFF; return *this; }
    Color4b      & white ()                          { r = g = b =       a = 0xFF; return *this; }

    // Array indexing
    Uint8 &operator [] (Uint i) {
        assert (i<4);
        return rgba[i];
    }
    const Uint8 operator [] (Uint i) const {
        assert (i<4);
        return rgba[i];
    }

    Color4b &operator += (const Color4b &v) {
        r += v.r; g += v.g; b += v.b; a += v.a;
        return *this;
    }
    Color4b &operator -= (const Color4b &v) {
        r -= v.r; g -= v.g; b -= v.b; b -= v.a;
        return *this;
    }
    Color4b &operator *= (Float32 f) {
        r = static_cast<Uint8>(r * f); g = static_cast<Uint8>(g * f);
        b = static_cast<Uint8>(b * f); a = static_cast<Uint8>(a * f);
        return *this;
    }
    Color4b &operator /= (Float32 f) {
        *this *= 1.f/f;
        return *this;
    }
    friend Color4b operator + (const Color4b &a, const Color4b &b) {
        Color4b ret(a);
        ret += b;
        return ret;
    }
    friend Color4b operator - (const Color4b &a, const Color4b &b) {
        Color4b ret(a);
        ret -= b;
        return ret;
    }
    friend Color4b operator * (const Color4b &v, Float32 f) {
        return Color4b((Uint8)(v.r * f), (Uint8)(v.g * f), (Uint8)(v.b * f), (Uint8)(v.a * f));
    }
    friend Color4b operator * (Float32 f, const Color4b &v) {
        return Color4b((Uint8)(v.r * f), (Uint8)(v.g * f), (Uint8)(v.b * f), (Uint8)(v.a * f));
    }
    friend Color4b operator / (const Color4b &v, Float32 f) {
        f = 1 / f;
        return Color4b((Uint8)(v.r * f), (Uint8)(v.g * f), (Uint8)(v.b * f), (Uint8)(v.a * f));
    }
    friend Bool operator == (const Color4b &a, const Color4b &b) {
        return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
    }
    friend Bool operator != (const Color4b &a, const Color4b &b) {
        return !(a == b);
    }
    friend Bool operator <= (const Color4b &a, const Color4b &b) {
        return a.r <= b.r && a.g <= b.g && a.b <= b.b && a.a <= b.a;
    }
    friend Bool operator >= (const Color4b &a, const Color4b &b) {
        return b <= a;
    }
};

Color4b  operator + (const Color4b &a, const Color4b &b);
Color4b  operator - (const Color4b &a, const Color4b &b);
Color4b  operator * (Float32 f, const Color4b &v);
Color4b  operator * (const Color4b &v, Float32 f);
Color4b  operator / (const Color4b &v, Float32 f);
Color4b  operator - (const Color4b &a);
Bool     operator ==(const Color4b &a, const Color4b &b);
Bool     operator !=(const Color4b &a, const Color4b &b);
Bool     operator <=(const Color4b &a, const Color4b &b);
Bool     operator >=(const Color4b &a, const Color4b &b);

#endif

