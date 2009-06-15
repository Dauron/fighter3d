#ifndef __Core_Stream_stream_h
#define __Core_Stream_stream_h

#include "../types.h"
class CObject;
class IProperty;

class IStream
{
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

class CStringWriter : public IStream
{
    String &m_buffer;

public:

    CStringWriter( String & out_string )
      : m_buffer( out_string ) {}

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

#endif

