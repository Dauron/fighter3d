#ifndef Core_Graphics_Types
#    error Include "Graphics/types.h" instead of this
#else

union Color3b {

    Uint8_3 rgb;
    struct {
        Uint8 r, g, b;
    };

    Color3b() {}
    Color3b(Uint8 R, Uint8 G, Uint8 B) : r(R), g(G), b(B)          {}

    static Color3b Black() { static Color3b res(0x00,0x00,0x00); return res; }
    static Color3b White() { static Color3b res(0xFF,0xFF,0xFF); return res; }

    static Color3b Create(Uint8 R, Uint8 G, Uint8 B) { return Color3b(R, G, B); }
    Color3b      & init  (Uint8 R, Uint8 G, Uint8 B) { r = R; g = G; b = B;  return *this; }

    Color3b      & black ()                          { r = g = b = 0x00;     return *this; }
    Color3b      & white ()                          { r = g = b = 0xFF;     return *this; }

    // Array indexing
    Uint8 &operator [] (Uint i) {
        assert (i<3);
        return rgb[i];
    }
    const Uint8 operator [] (Uint i) const {
        assert (i<3);
        return rgb[i];
    }

    Color3b &operator += (const Color3b &v) {
        r += v.r; g += v.g; b += v.b;
        return *this;
    }
    Color3b &operator -= (const Color3b &v) {
        r -= v.r; g -= v.g; b -= v.b;
        return *this;
    }
    Color3b &operator *= (Float32 f) {
        r = static_cast<Uint8>(r * f); g = static_cast<Uint8>(g * f); b = static_cast<Uint8>(b * f);
        return *this;
    }
    Color3b &operator /= (Float32 f) {
        *this *= 1.f/f;
        return *this;
    }
    friend Color3b operator + (const Color3b &a, const Color3b &b) {
        Color3b ret(a);
        ret += b;
        return ret;
    }
    friend Color3b operator - (const Color3b &a, const Color3b &b) {
        Color3b ret(a);
        ret -= b;
        return ret;
    }
    friend Color3b operator * (const Color3b &v, Float32 f) {
        return Color3b((Uint8)(v.r * f), (Uint8)(v.g * f), (Uint8)(v.b * f));
    }
    friend Color3b operator * (Float32 f, const Color3b &v) {
        return Color3b((Uint8)(v.r * f), (Uint8)(v.g * f), (Uint8)(v.b * f));
    }
    friend Color3b operator / (const Color3b &v, Float32 f) {
        f = 1 / f;
        return Color3b((Uint8)(v.r * f), (Uint8)(v.g * f), (Uint8)(v.b * f));
    }
    friend Bool operator == (const Color3b &a, const Color3b &b) {
        return a.r == b.r && a.g == b.g && a.b == b.b;
    }
    friend Bool operator != (const Color3b &a, const Color3b &b) {
        return !(a == b);
    }
    friend Bool operator <= (const Color3b &a, const Color3b &b) {
        return a.r <= b.r && a.g <= b.g && a.b <= b.b;
    }
    friend Bool operator >= (const Color3b &a, const Color3b &b) {
        return b <= a;
    }
};

Color3b  operator + (const Color3b &a, const Color3b &b);
Color3b  operator - (const Color3b &a, const Color3b &b);
Color3b  operator * (Float32 f, const Color3b &v);
Color3b  operator * (const Color3b &v, Float32 f);
Color3b  operator / (const Color3b &v, Float32 f);
Color3b  operator - (const Color3b &a);
Bool     operator ==(const Color3b &a, const Color3b &b);
Bool     operator !=(const Color3b &a, const Color3b &b);
Bool     operator <=(const Color3b &a, const Color3b &b);
Bool     operator >=(const Color3b &a, const Color3b &b);

#endif

