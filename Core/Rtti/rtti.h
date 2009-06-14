#ifndef __Core_Rtti_rtti_h
#define __Core_Rtti_rtti_h

#include <vector>

#include <string>
typedef std::string String;

//#include "../types.h"
typedef bool Bool;
typedef unsigned int Uint;

#include "classid.h"
#include "class.h"
#include "property.h"

class CObject
{
    DECLARE_ROOT_CLASS( CObject );
};

class CRtti
{
    typedef std::vector< CObject* > TObjectPVector;

    TObjectPVector m_ObjectTypes;

public:

    void RegisterObjectType( CObject &object )
    {
        object.InitClassInfo();
        m_ObjectTypes.push_back( &object );
    }

    static CRtti &GetInstance()
    {
        static CRtti instance;
        return instance;
    }
};

class CRttiRegistrant
{
public:
    CRttiRegistrant( CObject &classToRegister )
    {
        CRtti::GetInstance().RegisterObjectType( classToRegister );
    }
};

#endif

