#ifndef __incl_HandleDst_h
#define __incl_HandleDst_h

#include <cassert>

struct HandleDst
{
    int m_References;    // num of references

    HandleDst() : m_References (0) { /* ... */ }

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
