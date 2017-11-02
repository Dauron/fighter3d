#ifndef __incl_HandleDst_h
#define __incl_HandleDst_h

#include <cassert>
#include <string>

struct Resource
{
private:
    unsigned int m_References; // num of references
public:

    Resource()          { Clear(); }
    virtual ~Resource() = default;// { Destroy(); }

    virtual void Clear() { m_References = 0; }

    virtual bool Create()  { return false; }
    virtual void Destroy() { Dispose(); Clear(); }

    virtual bool Recreate()
    {
        Dispose();
        return Create();
    }
    virtual void Dispose()    = 0; // disposes data
    virtual void Invalidate() {}   // invalidates data (data has been disposed some other way, ex. by OpenGL)
    virtual bool IsDisposed() = 0;

    virtual const std::string &Identifier() = 0;

    void Lock()
    { ++m_References; }

    void Release()
    {
        assert( m_References );
        --m_References;
    }
    bool IsLocked()
    { return m_References; }
    unsigned int CountReferences()
    { return m_References; }
};

#endif
