#ifndef __Core_Rtti_rtti_h
#define __Core_Rtti_rtti_h

#include "../types.h"

#include "classid.h"
#include "class.h"
#include "property.h"

class CObject
{
    DECLARE_ROOT_CLASS( CObject );

protected:
    typedef TDArray<IProperty*> TPropertyVector;
    static  TPropertyVector     sm_Properties;

    /* Used by Rtti */
    void InitClassInfo();

    /* Add properties here */
    virtual void InitProperties() {}

public:
    virtual void Serialize( IStream & stream );
};

class CRtti
{
    typedef TDArray< CObject* > TObjectPVector;

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

