#ifndef __Core_Rtti_classid_h
#define __Core_Rtti_classid_h

typedef const char * const ClassId;

template <class T>
inline const ClassId &CLASS_ID( const T &object ) { return object.GetClassId(); }
template <class T>
inline const ClassId &CLASS_ID() { return T::GetClassId(); }

template <>
inline const ClassId &CLASS_ID<int>() { static ClassId id("int"); return id; }

#endif

