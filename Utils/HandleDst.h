#ifndef __incl_HandleDst_h
#define __incl_HandleDst_h

#include <cassert>
#include <string>

struct HandleDst
{
    int m_References;    // num of references

    HandleDst() : m_References (0) { /* ... */ }

    virtual const std::string &GetId()         = 0;
    virtual       void         Invalidate()    = 0;
    virtual       bool         IsValid() const = 0;
    virtual       bool         ReLoad ()       = 0;

    void IncReferences()
    {
        ++m_References;
    }

    void DecReferences()
    {
        assert( m_References );
        --m_References;
    }
};

#endif
