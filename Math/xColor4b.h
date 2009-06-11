union xColor4b {
    xBYTE4 rgba;
    struct {
        xBYTE r, g, b, a;
    };
    xColor3b color3b;

    static xColor4b Create(xBYTE R, xBYTE G, xBYTE B, xBYTE A) { xColor4b res;                   return res.init(R,G,B,A); }
    xColor4b       &init  (xBYTE R, xBYTE G, xBYTE B, xBYTE A) { r = R; g = G; b = B; a = A;     return *this; }
    xColor4b       &init  (xColor3b color3b, xBYTE A)          { this->color3b = color3b; a = A; return *this; }
    xColor4b       &zero  ()                                   { r = g = b = a = 0;              return *this; }

    // Array indexing
    xBYTE &operator [] (unsigned int i) {
        assert (i<4);
        return rgba[i];
    }
    xBYTE operator [] (unsigned int i) const {
        assert (i<4);
        return rgba[i];
    }

    void ToArray(xBYTE4 arr) const {
        memcpy(arr, this, sizeof(xBYTE4));
    }

    xColor4b &operator += (const xColor4b &v) {
        r += v.r; g += v.g; b += v.b; a += v.a;
        return *this;
    }
    xColor4b &operator -= (const xColor4b &v) {
        r -= v.r; g -= v.g; b -= v.b; a += v.a;
        return *this;
    }
    xColor4b &operator *= (float f) {
        r = (xBYTE)(r * f); g = (xBYTE)(g * f); b = (xBYTE)(b * f); a = (xBYTE)(a * f);
        return *this;
    }
    xColor4b &operator /= (float f) {
        *this *= 1.f/f;
        return *this;
    }
    friend xColor4b operator + (const xColor4b &a, const xColor4b &b) {
        xColor4b ret(a);
        ret += b;
        return ret;
    }
    friend xColor4b operator - (const xColor4b &a, const xColor4b &b) {
        xColor4b ret(a);
        ret -= b;
        return ret;
    }
    friend xColor4b operator * (const xColor4b &v, float f) {
        return xColor4b::Create((xBYTE)(v.r * f), (xBYTE)(v.g * f), (xBYTE)(v.b * f), (xBYTE)(v.a * f));
    }
    friend xColor4b operator * (float f, const xColor4b &v) {
        return xColor4b::Create((xBYTE)(v.r * f), (xBYTE)(v.g * f), (xBYTE)(v.b * f), (xBYTE)(v.a * f));
    }
    friend xColor4b operator / (const xColor4b &v, float f) {
        f = 1 / f;
        return xColor4b::Create((xBYTE)(v.r * f), (xBYTE)(v.g * f), (xBYTE)(v.b * f), (xBYTE)(v.a * f));
    }
    friend bool operator == (const xColor4b &a, const xColor4b &b) {
        return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
    }
    friend bool operator != (const xColor4b &a, const xColor4b &b) {
        return !(a == b);
    }
    friend bool operator <= (const xColor4b &a, const xColor4b &b) {
        return a.r <= b.r && a.g <= b.g && a.b <= b.b;
    }
    friend bool operator >= (const xColor4b &a, const xColor4b &b) {
        return b <= a;
    }

    bool isZero() const {
        return (r == 0 && g == 0 && b == 0 && a == 0);
    }

};

xColor4b  operator + (const xColor4b &a, const xColor4b &b);
xColor4b  operator - (const xColor4b &a, const xColor4b &b);
xColor4b  operator * (float f,           const xColor4b &v);
xColor4b  operator * (const xColor4b &v, float f);
xColor4b  operator / (const xColor4b &v, float f);
xColor4b  operator - (const xColor4b &a);
bool      operator ==(const xColor4b &a, const xColor4b &b);
bool      operator !=(const xColor4b &a, const xColor4b &b);
bool      operator <=(const xColor4b &a, const xColor4b &b);
bool      operator >=(const xColor4b &a, const xColor4b &b);
