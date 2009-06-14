#ifndef __Core_Rtti_class_h
#define __Core_Rtti_class_h

#include <vector>

#include "classid.h"
#include "property.h"

template <class T, class F>
inline T *Cast( F *object )
{
    return object && object->IsA( T::GetClassId() )
        ? static_cast<T *>( object )
        : 0;
}

#define DECLARE_ROOT_CLASS( THIS_CLASS )            \
    friend class CRtti;                             \
                                                    \
  private:                                          \
    typedef THIS_CLASS TThisClass;                  \
                                                    \
    static const ClassId sm_ClassId;                \
                                                    \
    void InitClassInfo()                            \
    {                                               \
        sm_Properties.clear();                      \
        InitProperties();                           \
    }                                               \
                                                    \
  protected:                                        \
    Bool m_bCreated;                                \
                                                    \
    typedef std::vector<IProperty> TPropertyVector; \
    static TPropertyVector sm_Properties;           \
                                                    \
    /* Add properties here */                       \
    virtual void InitProperties() {}                \
                                                    \
  public:                                           \
    static const ClassId &GetClassId()              \
    { return sm_ClassId; }                          \
                                                    \
    THIS_CLASS()                                    \
      : m_bCreated(false)                           \
    {                                               \
        Create();                                   \
    }                                               \
    virtual ~THIS_CLASS()                           \
    {                                               \
        if ( m_bCreated )  Destroy();               \
    }                                               \
                                                    \
    virtual void Create()  { m_bCreated = true;  }  \
    virtual void Destroy() { m_bCreated = false; }  \
                                                    \
    virtual Bool IsA( const ClassId &classId )      \
    {                                               \
        return sm_ClassId == classId;               \
    }                                               \
                                                    \
  private:                                          \


#define DECLARE_CLASS( THIS_CLASS, BASE_CLASS )     \
  private:                                          \
    typedef BASE_CLASS TBaseClass;                  \
    typedef THIS_CLASS TThisClass;                  \
                                                    \
    static const ClassId sm_ClassId;                \
                                                    \
  public:                                           \
    THIS_CLASS() : BASE_CLASS() {}                  \
    virtual ~THIS_CLASS()       {}                  \
                                                    \
    virtual Bool IsA( const ClassId &classId )      \
    {                                               \
        return sm_ClassId == classId    ||          \
               TBaseClass::IsA(classId) ;           \
    }                                               \
                                                    \
   private:                                         \

#define IMPLEMENT_CLASS( THIS_CLASS )                                               \
    const ClassId THIS_CLASS::sm_ClassId( #THIS_CLASS );                            \
                                                                                    \
    static THIS_CLASS      THIS_CLASS##representative;                              \
    static CRttiRegistrant registerClass##THIS_CLASS( THIS_CLASS##representative ); \


#endif

