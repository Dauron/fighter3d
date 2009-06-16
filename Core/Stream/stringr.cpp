#include "../core.h"

IStream & CStringReader::Append( CObject &object, IProperty &property )
{
    ASSERT( m_buffer < m_bufferEnd );

    IStream::Append( object, property );
    ++m_buffer;
    return *this;
}

IStream & CStringReader::operator<< ( Int8  &val )
{
    ASSERT( m_buffer < m_bufferEnd );

    FromString( m_buffer, val, m_buffer );
    ++m_buffer;

    return *this;
}

IStream & CStringReader::operator<< ( Int16 &val )
{
    ASSERT( m_buffer < m_bufferEnd );

    FromString( m_buffer, val, m_buffer );
    ++m_buffer;

    return *this;
}

IStream & CStringReader::operator<< ( Int32 &val )
{
    ASSERT( m_buffer < m_bufferEnd );

    FromString( m_buffer, val, m_buffer );
    ++m_buffer;

    return *this;
}

IStream & CStringReader::operator<< ( Uint8  &val )
{
    ASSERT( m_buffer < m_bufferEnd );

    FromString( m_buffer, val, m_buffer );
    ++m_buffer;

    return *this;
}

IStream & CStringReader::operator<< ( Uint16 &val )
{
    ASSERT( m_buffer < m_bufferEnd );

    FromString( m_buffer, val, m_buffer );
    ++m_buffer;

    return *this;
}

IStream & CStringReader::operator<< ( Uint32 &val )
{
    ASSERT( m_buffer < m_bufferEnd );

    FromString( m_buffer, val, m_buffer );
    ++m_buffer;

    return *this;
}

IStream & CStringReader::operator<< ( Float32 &val )
{
    ASSERT( m_buffer < m_bufferEnd );

    FromString( m_buffer, val, m_buffer );
    ++m_buffer;

    return *this;
}

IStream & CStringReader::operator<< ( Float64 &val )
{
    ASSERT( m_buffer < m_bufferEnd );

    FromString( m_buffer, val, m_buffer );
    ++m_buffer;

    return *this;
}

IStream & CStringReader::operator<< ( Bool &val )
{
    ASSERT( m_buffer < m_bufferEnd );

    FromString( m_buffer, val, m_buffer );
    ++m_buffer;

    return *this;
}

IStream & CStringReader::operator<< ( String &val )
{
    ASSERT( m_buffer < m_bufferEnd );

    Uint32 len = 0;
    *this << len;
    val = String( m_buffer, len );
    m_buffer += len + 1;
    return *this;
}

