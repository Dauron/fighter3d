#include "stream.h"
#include "../Rtti/rtti.h"

IStream & IStream::operator<< ( CObject &object )
{
    object.Serialize( *this );
    return *this;
}

IStream & IStream::Append( CObject &object, IProperty &property )
{
    property.Serialize( &object, *this );
    return *this;
}

IStream & CStringWriter::Append( CObject &object, IProperty &property )
{
    IStream::Append( object, property );
    m_buffer.append( TXT("\n") );
    return *this;
}

IStream & CStringWriter::operator<< ( Int8  &val )
{
    m_buffer.append( ToString( val ) );
    return *this;
}

IStream & CStringWriter::operator<< ( Int16 &val )
{
    m_buffer.append( ToString( val ) );
    return *this;
}

IStream & CStringWriter::operator<< ( Int32 &val )
{
    m_buffer.append( ToString( val ) );
    return *this;
}

IStream & CStringWriter::operator<< ( Uint8  &val )
{
    m_buffer.append( ToString( val ) );
    return *this;
}

IStream & CStringWriter::operator<< ( Uint16 &val )
{
    m_buffer.append( ToString( val ) );
    return *this;
}

IStream & CStringWriter::operator<< ( Uint32 &val )
{
    m_buffer.append( ToString( val ) );
    return *this;
}

IStream & CStringWriter::operator<< ( Float32 &val )
{
    m_buffer.append( ToString( val ) );
    return *this;
}

IStream & CStringWriter::operator<< ( Float64 &val )
{
    m_buffer.append( ToString( val ) );
    return *this;
}

IStream & CStringWriter::operator<< ( Bool &val )
{
    m_buffer.append( ToString( val ) );
    return *this;
}

IStream & CStringWriter::operator<< ( String &val )
{
    Uint len = val.length();
    *this << len;
    m_buffer.append( val );
    return *this;
}

