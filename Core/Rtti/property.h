#ifndef __Core_Rtti_property_h
#define __Core_Rtti_property_h

#define ADD_PROPERTY( PROPERTY, DESCRIPTION ) \
    TThisClass::sm_Properties.push_back( CProperty( this, this->PROPERTY, #PROPERTY, DESCRIPTION ) );

//#include "../stream.h"

class CObject;

class IProperty
{
protected:
    Uint          m_nOffset;

    String        m_sName;
    String        m_sDescription;

public:
    IProperty( Uint nOffset, String sName, String sDescription )
      : m_nOffset     ( nOffset )
      , m_sName       ( sName )
      , m_sDescription( sDescription)
    {
    }
    virtual ~IProperty() {}

//    virtual void Serialize( CObject *owner, IStream &stream ) const
//    {
        // stream << m_sName;
//    }
};

template< typename PropertyType >
class CProperty : public IProperty
{
public:
    CProperty( CObject *owner, PropertyType &property, String sName, String sDescription )
      : IProperty( static_cast<char*>( &property ) - static_cast<char*>( owner )
                 , sName, sDescription )
    {
    }
    virtual ~CProperty() {}

    PropertyType &Get( CObject *owner ) const
    {
        return *static_cast<PropertyType*>( static_cast<void*>( owner ) + m_nOffset );
    }
    const PropertyType &Get( const CObject *owner ) const
    {
        return *static_cast<const PropertyType*>( static_cast<const void*>( owner ) + m_nOffset );
    }

    void Set( CObject *owner, const PropertyType &value ) const
    {
        Get( owner ) = value;
    }

//    virtual void Serialize( CObject *owner, IStream &stream ) const
//    {
//        IProperty::Serialize( owner, stream );
//
//        Get( owner ).Serialize( stream );
//    }
};

#endif

