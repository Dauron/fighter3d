#ifndef __Core_Stream_stream_h
#define __Core_Stream_stream_h

#include "../types.h"
class CObject;
class IProperty;

class IStream
{
protected:
    friend class CSkippableBlock;

    virtual Uint32 BeginSkippableBlock()                   = 0;
    virtual void   EndSkippableBlock  ( Uint32 prevBlock ) = 0;

public:

    virtual Bool IsReader() = 0;
    virtual Bool IsWriter() = 0;

    virtual IStream & operator<< ( CObject &object );
    virtual IStream & Append     ( CObject &object, IProperty &property );

    virtual IStream & operator<< ( Int8  &val ) = 0;
    virtual IStream & operator<< ( Int16 &val ) = 0;
    virtual IStream & operator<< ( Int32 &val ) = 0;

    virtual IStream & operator<< ( Uint8  &val ) = 0;
    virtual IStream & operator<< ( Uint16 &val ) = 0;
    virtual IStream & operator<< ( Uint32 &val ) = 0;

    virtual IStream & operator<< ( Float32 &val ) = 0;
    virtual IStream & operator<< ( Float64 &val ) = 0;

    virtual IStream & operator<< ( Bool &val ) = 0;

    virtual IStream & operator<< ( String &val ) = 0;
};

class CSkippableBlock
{
    IStream & m_stream;
    Uint32    m_prevBlock;

public:
    CSkippableBlock ( IStream & stream ) : m_stream(stream) { m_prevBlock = m_stream.BeginSkippableBlock(); }
    ~CSkippableBlock()                                      { m_stream.EndSkippableBlock( m_prevBlock ); }
};

class CStringWriter : public IStream
{
protected:
    String &m_buffer;
    Uint32  m_block;

    virtual Uint32 BeginSkippableBlock()
    {
        Uint32 prevBlock = m_block;
        m_block = m_buffer.length();
        return prevBlock;
    }
    virtual void   EndSkippableBlock  ( Uint32 prevBlock )
    {
        String block = m_buffer.substr( m_block );
        m_buffer     = m_buffer.substr( 0, m_block );

        m_block = block.length();
        *this << m_block;
        m_buffer.append( block );

        m_block = prevBlock;
    }

public:

    CStringWriter( String & out_string )
      : m_buffer ( out_string )
      , m_block  ( 0 )
    {}

    virtual Bool IsReader() { return false; }
    virtual Bool IsWriter() { return true;  }

    virtual IStream & operator<< ( CObject &object ) { return IStream::operator<<( object ); }
    virtual IStream & Append     ( CObject &object, IProperty &property );

    virtual IStream & operator<< ( Int8  &val );
    virtual IStream & operator<< ( Int16 &val );
    virtual IStream & operator<< ( Int32 &val );

    virtual IStream & operator<< ( Uint8  &val );
    virtual IStream & operator<< ( Uint16 &val );
    virtual IStream & operator<< ( Uint32 &val );

    virtual IStream & operator<< ( Float32 &val );
    virtual IStream & operator<< ( Float64 &val );

    virtual IStream & operator<< ( Bool &val );

    virtual IStream & operator<< ( String &val );
};

class CStringReader : public IStream
{
protected:
    const Char *m_buffer;
    const Char *m_bufferStart;
    const Char *m_bufferEnd;

    virtual Uint32 BeginSkippableBlock()
    {
        Uint32 prevBlock = m_bufferEnd - m_bufferStart;

        Uint32 currBlock;
        *this << currBlock;
        m_bufferEnd = m_buffer + currBlock;

        return prevBlock;
    }
    virtual void   EndSkippableBlock  ( Uint32 prevBlock )
    {
        m_buffer    = m_bufferEnd;
        m_bufferEnd = m_bufferStart + prevBlock;
    }

public:

    CStringReader( String & in_string )
      : m_buffer      ( in_string.data() )
      , m_bufferStart ( m_buffer )
      , m_bufferEnd   ( m_buffer + in_string.length() )
    {}

    virtual Bool IsReader() { return true; }
    virtual Bool IsWriter() { return false;  }

    virtual IStream & operator<< ( CObject &object ) { return IStream::operator<<( object ); }
    virtual IStream & Append     ( CObject &object, IProperty &property );

    virtual IStream & operator<< ( Int8  &val );
    virtual IStream & operator<< ( Int16 &val );
    virtual IStream & operator<< ( Int32 &val );

    virtual IStream & operator<< ( Uint8  &val );
    virtual IStream & operator<< ( Uint16 &val );
    virtual IStream & operator<< ( Uint32 &val );

    virtual IStream & operator<< ( Float32 &val );
    virtual IStream & operator<< ( Float64 &val );

    virtual IStream & operator<< ( Bool &val );

    virtual IStream & operator<< ( String &val );
};

#endif

