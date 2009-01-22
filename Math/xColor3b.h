union xColor3b {
    xBYTE3 rgb;
    struct {
        xBYTE r, g, b;
    };

    static xColor3b Create(xBYTE R, xBYTE G, xBYTE B) { xColor3b res;        return res.init(R,G,B); }
    xColor3b       &init  (xBYTE R, xBYTE G, xBYTE B) { r = R; g = G; b = B; return *this; }
    xColor3b       &zero  ()                          { r = g = b = 0;       return *this; }

    // Array indexing
    xBYTE &operator [] (unsigned int i) {
        assert (i<3);
        return rgb[i];
    }
    const xBYTE operator [] (unsigned int i) const {
        assert (i<3);
        return rgb[i];
    }

    void ToArray(xBYTE3 arr) const {
        memcpy(arr, this, sizeof(xBYTE3));
    }

    xColor3b &operator += (const xColor3b &v) {
        r += v.r; g += v.g; b += v.b;
        return *this;
    }
    xColor3b &operator -= (const xColor3b &v) {
        r -= v.r; g -= v.g; b -= v.b;
        return *this;
    }
    xColor3b &operator *= (float f) {
        r = (xBYTE)(r * f); g = (xBYTE)(g * f); b = (xBYTE)(b * f);
        return *this;
    }
    xColor3b &operator /= (float f) {
        *this *= 1.f/f;
        return *this;
    }
    friend xColor3b operator + (const xColor3b &a, const xColor3b &b) {
        xColor3b ret(a);
        ret += b;
        return ret;
    }
    friend xColor3b operator - (const xColor3b &a, const xColor3b &b) {
        xColor3b ret(a);
        ret -= b;
        return ret;
    }
    friend xColor3b operator * (const xColor3b &v, float f) {
        return xColor3b::Create((xBYTE)(v.r * f), (xBYTE)(v.g * f), (xBYTE)(v.b * f));
    }
    friend xColor3b operator * (float f, const xColor3b &v) {
        return xColor3b::Create((xBYTE)(v.r * f), (xBYTE)(v.g * f), (xBYTE)(v.b * f));
    }
    friend xColor3b operator / (const xColor3b &v, float f) {
        f = 1 / f;
        return xColor3b::Create((xBYTE)(v.r * f), (xBYTE)(v.g * f), (xBYTE)(v.b * f));
    }
    friend bool operator == (const xColor3b &a, const xColor3b &b) {
        return a.r == b.r && a.g == b.g && a.b == b.b;
    }
    friend bool operator != (const xColor3b &a, const xColor3b &b) {
        return !(a == b);
    }
    friend bool operator <= (const xColor3b &a, const xColor3b &b) {
        return a.r <= b.r && a.g <= b.g && a.b <= b.b;
    }
    friend bool operator >= (const xColor3b &a, const xColor3b &b) {
        return b <= a;
    }

    bool isZero() const {
        return (r == 0 && g == 0 && b == 0);
    }

};

xColor3b  operator + (const xColor3b &a, const xColor3b &b);
xColor3b  operator - (const xColor3b &a, const xColor3b &b);
xColor3b  operator * (float f, const xColor3b &v);
xColor3b  operator * (const xColor3b &v, float f);
xColor3b  operator / (const xColor3b &v, float f);
xColor3b  operator - (const xColor3b &a);
bool      operator ==(const xColor3b &a, const xColor3b &b);
bool      operator !=(const xColor3b &a, const xColor3b &b);
bool      operator <=(const xColor3b &a, const xColor3b &b);
bool      operator >=(const xColor3b &a, const xColor3b &b);
