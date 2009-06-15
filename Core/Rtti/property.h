#ifndef __Core_Rtti_property_h
#define __Core_Rtti_property_h

#define ADD_PROPERTY( PROPERTY, DESCRIPTION ) \
    TThisClass::sm_Properties.push_back( new CProperty< typeof(PROPERTY) >( this, this->PROPERTY, #PROPERTY, DESCRIPTION ) );

#include "../Stream/stream.h"

class CObject;

class IProperty
{
protected:
    Uint          m_nOffset;

    String        m_sName;
    String        m_sDescription;

public:
    IProperty( Uint nOffset, const String &sName, const String &sDescription )
      : m_nOffset     ( nOffset )
      , m_sName       ( sName )
      , m_sDescription( sDescription)
    {
    }
    virtual ~IProperty() {}

    const String & GetName()        { return m_sName; }
    const String & GetDescription() { return m_sDescription; }

    virtual void Serialize( CObject *owner, IStream &stream ) const = 0;
};

template< typename PropertyType >
class CProperty : public IProperty
{
public:
    CProperty( CObject *owner, PropertyType &property, const String &sName, const String &sDescription )
      : IProperty( reinterpret_cast<char*>( &property ) - reinterpret_cast<char*>( owner )
                 , sName, sDescription )
    {
    }
    virtual ~CProperty() {}

    PropertyType &Get( CObject *owner ) const
    {
        return *reinterpret_cast<PropertyType*>( reinterpret_cast<char*>( owner ) + m_nOffset );
    }
    const PropertyType &Get( const CObject *owner ) const
    {
        return *reinterpret_cast<const PropertyType*>( reinterpret_cast<const char*>( owner ) + m_nOffset );
    }

    void Set( CObject *owner, const PropertyType &value ) const
    {
        Get( owner ) = value;
    }

    virtual void Serialize( CObject *owner, IStream &stream ) const
    {
        stream << Get( owner );
    }
};

#endif

