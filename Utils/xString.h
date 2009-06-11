#ifndef __incl_Utils_xString_h
#define __incl_Utils_xString_h

#include <cassert>
#include <list>
#include <cstring>

typedef unsigned long UInt32;

class xString;

class xStringMgr
{
    friend class xString;
    friend xString operator+ (const char *text1,      const xString &string2);

    struct xStringData
    {
        char  *data;
        UInt32 refs;
        UInt32 size;
        UInt32 length;

        xStringData(UInt32 nSize) : refs(0), size(nSize), length(0)
        {
            data = new char[nSize+1];
        }

        ~xStringData()
        {
            delete[] data;
        }
    };

    template<UInt32 STRING_SIZE, UInt32 BUCKET_SIZE>
    class xStringDataBucket
    {
        typedef std::list< xStringData* > TLstString;

        TLstString m_lstStrings;
        UInt32     m_nAcquired;

    public:
        xStringDataBucket() : m_nAcquired(0)
        {
            if (BUCKET_SIZE)
                for (int i = BUCKET_SIZE; i > 0; --i)
                    m_lstStrings.push_back(new xStringData(STRING_SIZE));
        }

        UInt32 GetAcquired() { return m_nAcquired; }

        xStringData *Acquire()
        {
            ++m_nAcquired;

            if (m_lstStrings.empty())
            {
                xStringData *res = new xStringData(STRING_SIZE);
                res->refs = 1;
                return res;
            }

            xStringData *res = m_lstStrings.front();
            m_lstStrings.pop_front();
            res->refs = 1;
            return res;
        }

        void Release(xStringData *data)
        {
            assert(data);
            assert(data->refs);

            --data->refs;

            if (!data->refs)
            {
                --m_nAcquired;
                if (m_lstStrings.size() < BUCKET_SIZE)
                    m_lstStrings.push_back(data);
                else
                    delete data;
            }
        }
    };

    template<UInt32 STRING_SIZE>
    class xStringDataBucket <STRING_SIZE, 0>
    {
        UInt32 m_nAcquired;

    public:
        xStringDataBucket() : m_nAcquired(0)
        {}

        UInt32 GetAcquired() { return m_nAcquired; }

        xStringData *Acquire()
        {
            assert(STRING_SIZE);

            ++m_nAcquired;

            xStringData *res = new xStringData(STRING_SIZE);
            res->refs = 1;
            return res;
        }

        xStringData *Acquire(UInt32 nSize)
        {
            assert(nSize);

            ++m_nAcquired;

            xStringData *res = new xStringData(nSize);
            res->refs = 1;
            return res;
        }

        void Release(xStringData *data)
        {
            assert(data);
            assert(data->refs);

            --data->refs;

            if (!data->refs)
            {
                --m_nAcquired;
                delete data;
            }
        }
    };

    typedef xStringDataBucket<16,   50> TString16Bucket;
    typedef xStringDataBucket<64,   50> TString64Bucket;
    typedef xStringDataBucket<256,  25> TString256Bucket;
    typedef xStringDataBucket<1024, 25> TString1024Bucket;
    typedef xStringDataBucket<0,    0>  TStringDynBucket;

    TString16Bucket   m_cString16Bucket;
    TString64Bucket   m_cString64Bucket;
    TString256Bucket  m_cString256Bucket;
    TString1024Bucket m_cString1024Bucket;
    TStringDynBucket  m_cStringDynBucket;

    xStringData *Acquire(UInt32 nSize)
    {
        if (nSize <= 16)
            return m_cString16Bucket.Acquire();
        if (nSize <= 64)
            return m_cString64Bucket.Acquire();
        if (nSize <= 256)
            return m_cString256Bucket.Acquire();
        if (nSize <= 1024)
            return m_cString1024Bucket.Acquire();

        return m_cStringDynBucket.Acquire(nSize);
    }

    void Release(xStringData *data)
    {
        if (!data) return;

        if (data->size <= 16)
            return m_cString16Bucket.Release(data);
        if (data->size <= 64)
            return m_cString64Bucket.Release(data);
        if (data->size <= 256)
            return m_cString256Bucket.Release(data);
        if (data->size <= 1024)
            return m_cString1024Bucket.Release(data);

        return m_cStringDynBucket.Release(data);
    }
};

extern xStringMgr g_xStringMgr;

class xString
{
    xStringMgr::xStringData *m_psData;

    UInt32 reserved() const { return m_psData ? m_psData->size : 0; }
    void   ensure_unique();

    xString(xStringMgr::xStringData *psData) : m_psData(psData)
    {}

public:

    static const xString& Empty()
    {
        static const xString sEmpty;
        return sEmpty;
    }

    xString() : m_psData(0)
    {}
    xString(const xString &string)
    {
        m_psData = string.m_psData;
        if (m_psData) ++m_psData->refs;
    }
    xString(const char *text);

    operator bool() const
    { return !empty(); }
    operator const char *() const
    { return m_psData ? m_psData->data : 0; }

    xString &operator= (const xString &string)
    {
        g_xStringMgr.Release( m_psData );
        m_psData = string.m_psData;
        if (m_psData) ++m_psData->refs;
        return *this;
    }
    xString &operator= (const char *text);

    xString &operator+= (const xString &string);
    xString &operator+= (const char *text);

    friend xString operator+ (const xString &string1, const xString &string2)
    {
        xString res(string1);
        res += string2;
        return res;
    }
    friend xString operator+ (const xString &string1, const char *text2)
    {
        xString res(string1);
        res += text2;
        return res;
    }
    friend xString operator+ (const char *text1,      const xString &string2);

    friend bool    operator== (const xString &string1, const xString &string2)
    {
        if (string1.m_psData == string2.m_psData)
            return true;
        if (!string1.m_psData || !string2.m_psData)
            return false;
        if (string1.m_psData->length != string2.m_psData->length)
            return false;
        return !strcmp(string1.m_psData->data, string2.m_psData->data);
    }
    friend bool    operator== (const xString &string1, const char *text2)
    {
        if (!string1.m_psData)
            return !text2 || !text2[0];
        if (!text2 || !text2[0])
            return false;
        return !strcmp(string1.m_psData->data, text2);
    }
    friend bool    operator== (const char *text1,      const xString &string2)
    { return string2 == text1; }

    friend bool    operator!= (const xString &string1, const xString &string2)
    { return !(string1 == string2); }
    friend bool    operator!= (const xString &string1, const char *text2)
    { return !(string1 == text2); }
    friend bool    operator!= (const char *text1,      const xString &string2)
    { return !(text1 == string2); }

    const char *cstr()   const { return m_psData ? m_psData->data : ""; }
    UInt32      length() const { return m_psData ? m_psData->length : 0; }
    bool        empty()  const { return length() == 0; }
};

xString operator+ (const xString &string1, const xString &string2);
xString operator+ (const xString &string1, const char *text2);
xString operator+ (const char *text1,      const xString &string2);

bool    operator== (const xString &string1, const xString &string2);
bool    operator== (const xString &string1, const char *text2);
bool    operator== (const char *text1,      const xString &string2);

bool    operator!= (const xString &string1, const xString &string2);
bool    operator!= (const xString &string1, const char *text2);
bool    operator!= (const char *text1,      const xString &string2);

#endif // __incl_Utils_xString_h
