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
    {
        Uint32 numProperties = sm_Properties.size();
        stream << numProperties;

        for ( Uint i = 0; i < numProperties; ++i )
        {
            CSkippableBlock block( stream );

            IProperty &property = *sm_Properties[i];

            String propertyName = property.GetName();
            stream << propertyName;
            stream.Append( *this, property );
        }
    }
    else
    {
        Uint32 numProperties;
        stream << numProperties;

        while ( numProperties-- )
        {
            CSkippableBlock block( stream );

            String propertyName;
            stream << propertyName;

            for ( Uint i = 0; i < sm_Properties.size(); ++i )
            {
                IProperty &property = *sm_Properties[i];
                if ( property.GetName() == propertyName )
                {
                    stream.Append( *this, property );
                    break;
                }
            }
        }
     }
}

