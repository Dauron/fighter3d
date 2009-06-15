#include "rtti.h"

IMPLEMENT_CLASS( CObject );

CObject::TPropertyVector CObject::sm_Properties;

void CObject::InitClassInfo()
{
    for (unsigned i = 0; i < sm_Properties.size(); ++i)
        delete sm_Properties[i];
    sm_Properties.clear();
    InitProperties();
}

void CObject::Serialize( IStream &stream )
{
    if ( stream.IsWriter() )
        for ( Uint i = 0; i < sm_Properties.size(); ++i )
        {
            IProperty &property = *sm_Properties[i];

            String propertyName = property.GetName();
            stream << propertyName;
            stream.Append( *this, property );
        }
}

