#include "xString.h"

xStringMgr g_xStringMgr;

xString::xString(const char *text)
{
    if (!text || !text[0])
        m_psData = 0;
    else
    {
        UInt32 nLength = strlen( text );
        m_psData = g_xStringMgr.Acquire( nLength );
        strcpy(m_psData->data, text);
        m_psData->length = nLength;
    }
}

void xString::ensure_unique()
{
    if (m_psData && m_psData->refs > 1)
    {
        --m_psData->refs;

        xStringMgr::xStringData *psNewData = g_xStringMgr.Acquire(m_psData->length);
        strcpy(psNewData->data, m_psData->data);
        psNewData->length = m_psData->length;
        m_psData = psNewData;
    }
}

xString &xString::operator= (const char *text)
{
    g_xStringMgr.Release( m_psData );
    if (!text || !text[0])
        m_psData = 0;
    else
    {
        UInt32 nLength = strlen( text );
        m_psData = g_xStringMgr.Acquire( nLength );
        strcpy(m_psData->data, text);
        m_psData->length = nLength;
    }
    return *this;
}

xString &xString::operator+= (const xString &string)
{
    UInt32 nOldLength   = length();
    UInt32 nTotalLength = nOldLength + string.length();

    if (nTotalLength > reserved())
    {
        xStringMgr::xStringData *psNewData = g_xStringMgr.Acquire(nTotalLength);

        strcpy(psNewData->data, cstr());
        strcpy(psNewData->data + nOldLength, string.cstr());
        psNewData->length = nTotalLength;

        g_xStringMgr.Release(m_psData);
        m_psData = psNewData;

        return *this;
    }

    if (nTotalLength > nOldLength)
    {
        ensure_unique();

        strcpy(m_psData->data + nOldLength, string.cstr());
        m_psData->length = nTotalLength;
    }
    return *this;
}

xString &xString::operator+= (const char *text)
{
    UInt32 nOldLength   = length();
    UInt32 nTotalLength = nOldLength + strlen(text);

    if (nTotalLength > reserved())
    {
        xStringMgr::xStringData *psNewData = g_xStringMgr.Acquire(nTotalLength);

        strcpy(psNewData->data, cstr());
        strcpy(psNewData->data + nOldLength, text);
        psNewData->length = nTotalLength;

        g_xStringMgr.Release(m_psData);
        m_psData = psNewData;

        return *this;
    }

    if (nTotalLength > nOldLength)
    {
        ensure_unique();

        strcpy(m_psData->data + nOldLength, text);
        m_psData->length = nTotalLength;
    }
    return *this;
}

xString operator+ (const char *text1,      const xString &string2)
{
    if (!text1 || !text1[0])
        return string2;

    if (!string2.length())
        return xString(text1);

    UInt32 nLeftLength  = strlen(text1);
    UInt32 nTotalLength = nLeftLength + string2.length();

    xStringMgr::xStringData *psNewData = g_xStringMgr.Acquire(nTotalLength);
    strcpy(psNewData->data, text1);
    strcpy(psNewData->data + nLeftLength, string2.cstr());
    psNewData->length = nTotalLength;

    return xString(psNewData);
}
