#ifndef Core_Math_Types
#    error Include "Math/types.h" instead of this
#else

struct Matrix {
    static const Uint sm_nRows  = 4;
    static const Uint sm_nCols  = 4;
    static const Uint sm_nCells = sm_nRows * sm_nCols;

    union {
        Float32     cells [sm_nCells];
        Float32_4x4 matrix;
        struct {
            Float32 x0, y0, z0, w0;
            Float32 x1, y1, z1, w1;
            Float32 x2, y2, z2, w2;
            Float32 x3, y3, z3, w3;
        };
    };
          Vector4 & row ( Uint i )       { assert( i < sm_nRows ); return *reinterpret_cast<Vector4*>( cells + (i << 2 ) ); }
    const Vector4 & row ( Uint i ) const { assert( i < sm_nRows ); return *reinterpret_cast<const Vector4*>( cells + (i << 2 ) ); }

          Vector4 & row0 ()       { return *reinterpret_cast<Vector4*>( &x0  ); }
    const Vector4 & row0 () const { return *reinterpret_cast<const Vector4*>( &x0 ); }
          Vector4 & row1 ()       { return *reinterpret_cast<Vector4*>( &x1  ); }
    const Vector4 & row1 () const { return *reinterpret_cast<const Vector4*>( &x1 ); }
          Vector4 & row2 ()       { return *reinterpret_cast<Vector4*>( &x2  ); }
    const Vector4 & row2 () const { return *reinterpret_cast<const Vector4*>( &x2 ); }
          Vector4 & row3 ()       { return *reinterpret_cast<Vector4*>( &x3  ); }
    const Vector4 & row3 () const { return *reinterpret_cast<const Vector4*>( &x3 ); }

    Matrix() {}

    Quaternion ToQuaternion() const;

    Matrix & initIdentity       ()                        { return *this = Identity(); }
    Matrix & initFromQuaternion ( const Quaternion &q );
    Matrix & initFromQuaternionT( const Quaternion &q );
    Matrix & initAxisRotate     ( Float32 radX, Float32 radY, Float32 radZ );
    Matrix & initTranslation    ( Float32 x, Float32 y, Float32 z );
    Matrix & initTranslationT   ( Float32 x, Float32 y, Float32 z );
    Matrix & initOrthogonal     ( const Vector3 &forward, const Vector3 &up );
    Matrix & initOrthogonal     ( const Vector3 &forward, const Vector3 &up, const Vector3 &center )
    { return initOrthogonal(forward, up).postTranslateT(-center); }

    // Transpose the Matrix
    Matrix & transpose();
    Matrix   Transpose()         const { return Matrix(*this).transpose(); }
    // Invert the Matrix
    Matrix & invert();
    Matrix   Invert()            const { return Matrix(*this).invert();    }
    Matrix & Invert(Matrix &dst) const { return (dst = *this).invert();      }

    // PreMultiply Matrix
    Matrix & preMultiply   (const Matrix &m);
    Matrix & postMultiply  (const Matrix &m) { return *this *= m; }
    // Translate Matrix
    Matrix & preTranslate  (const Vector3& pos);
    Matrix & preTranslateT (const Vector3& pos);
    Matrix & postTranslate (const Vector3& pos);
    Matrix & postTranslateT(const Vector3& pos);

    // Transform Point
    Point3 preTransformP   (const Point3& point) const;
    Point3 postTransformP  (const Point3& point) const;
    // Transform Vector
    Vector3 preTransformV  (const Vector3& vec) const;
    Vector3 postTransformV (const Vector3& vec) const;

    static const Matrix& Identity()
    {
        static Bool   init = true;
        static Matrix ONE;
        if (init)
        {
            init = false;
            memset(&ONE, 0, sizeof(ONE));
            ONE.x0 = ONE.y1 = ONE.z2 = ONE.w3 = 1.0f;
        }
        return ONE;
    }
    Bool IsIdentity() const { return EqualsNearly(*this, Identity()); }

    // Array indexing
    Vector4 &operator [] (Uint i) {
        assert (i < sm_nRows);
        return row(i);
    }
    const Vector4 &operator [] (Uint i) const {
        assert (i < sm_nRows);
        return row(i);
    }

    // Add a Matrix to this one
    Matrix &operator += (const Matrix &source) {
        const Float32 *src = source.cells - 1;
              Float32 *dst = this->cells  - 1;
        for(unsigned i=0; i < sm_nCells; ++i)
            *(++dst) += *(++src);
        return *this;
    }
    // Subtract a Matrix from this one
    Matrix &operator -= (const Matrix &source) {
        const Float32 *src = source.cells - 1;
              Float32 *dst = this->cells  - 1;
        for(unsigned i=0; i < sm_nCells; ++i)
            *(++dst) -= *(++src);
        return *this;
    }
    // Multiply the Matrix by another Matrix
    Matrix &operator *= (const Matrix &m);
    // Multiply the Matrix by a Float32
    Matrix &operator *= (Float32 scalar) {
        Float32 *dst = this->cells - 1;
        for(unsigned i=0; i < sm_nCells; ++i)
            *(++dst) *= scalar;
        return *this;
    }
    // Are these two Matrices equal?
    friend Bool operator == (const Matrix &a, const Matrix &b) {
        const Float32 *src = a.cells - 1;
        const Float32 *dst = b.cells - 1;
        for(unsigned i=0; i < sm_nCells; ++i)
            if (*(++dst) != *(++src)) return false;
        return true;
    }
    static Bool EqualsNearly (const Matrix &a, const Matrix &b) {
        const Float32 *src = a.cells - 1;
        const Float32 *dst = b.cells - 1;
        for(unsigned i=0; i < sm_nCells; ++i)
            if ( NotEquals(*(++dst), *(++src)) ) return false;
        return true;
    }
     // Are these two Matrices not equal?
    friend Bool operator != (const Matrix &a, const Matrix &b) {
        return ! ( a == b );
    }
    // Add two Matrices
    friend Matrix operator + (const Matrix &a, const Matrix &b) {
        Matrix ret(a);
        ret += b;
        return ret;
    }
    // Subtract one Matrix from another
    friend Matrix operator - (const Matrix &a, const Matrix &b) {
        Matrix ret(a);
        ret -= b;
        return ret;
    }
    // Multiply Matrix by another Matrix
    friend Matrix operator * (const Matrix &a, const Matrix &b) {
        Matrix ret(a);
        ret *= b;
        return ret;
    }
    // Multiply Matrix by a Vector3
    friend Vector3 operator * (const Matrix &b, const Vector3 &a) {
        return ( b * Vector4(a, 0.f) ).vector3();
    }
    // Multiply Vector3 by a Matrix
    friend Vector3 operator * (const Vector3 &a, const Matrix &b) {
        return ( Vector4(a, 0.f) * b ).vector3();
    }
    // Multiply Matrix by a Vector4
    friend Vector4 operator * (const Matrix &a, const Vector4 &b) {
        Vector4 ret;
        ret.x = Vector4::DotProduct(a.row0(),b);
        ret.y = Vector4::DotProduct(a.row1(),b);
        ret.z = Vector4::DotProduct(a.row2(),b);
        ret.w = Vector4::DotProduct(a.row3(),b);
        return ret;
    }
    // Multiply Vector4 by a Matrix
    friend Vector4 operator * (const Vector4 &a, const Matrix &b) {
        Vector4 ret;
        ret.x = a.x * b.x0 + a.y * b.x1 + a.z * b.x2 + a.w * b.x3;
        ret.y = a.x * b.y0 + a.y * b.y1 + a.z * b.y2 + a.w * b.y3;
        ret.z = a.x * b.z0 + a.y * b.z1 + a.z * b.z2 + a.w * b.z3;
        ret.w = a.x * b.w0 + a.y * b.w1 + a.z * b.w2 + a.w * b.w3;
        return ret;
    }
    // Multiply Matrix by a Float32
    friend Matrix operator * (const Matrix &m, Float32 f) { return Matrix(m) * f; }
    // Multiply Float32 by a Matrix
    friend Matrix operator * (Float32 f, const Matrix &m) { return f * Matrix(m); }
};

Vector3 operator * (const Matrix &a,  const Vector3 &b);
Vector3 operator * (const Vector3 &a, const Matrix &b);
Matrix  operator + (const Matrix &a, const Matrix &b);
Vector4 operator * (const Matrix &a,  const Vector4 &b);
Vector4 operator * (const Vector4 &a, const Matrix &b);
Matrix  operator * (const Matrix &a, const Matrix &b);
Matrix  operator * (const Matrix &m, Float32 f);
Matrix  operator * (Float32 f, const Matrix &m);

Bool operator == (const Matrix &a, const Matrix &b);
Bool operator != (const Matrix &a, const Matrix &b);

#endif

