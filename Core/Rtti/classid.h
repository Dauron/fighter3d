#ifndef __Core_Rtti_classid_h
#define __Core_Rtti_classid_h

typedef const char * const ClassId;

template <class T>
inline const ClassId &CLASS_ID( const T &object ) { return object.GetClassId(); }
template <class T>
inline const ClassId &CLASS_ID() { return T::GetClassId(); }

template <>
inline const ClassId &CLASS_ID<Bool>() { static ClassId id("Bool"); return id; }

template <>
inline const ClassId &CLASS_ID<Int8>() { static ClassId id("Int8"); return id; }
template <>
inline const ClassId &CLASS_ID<Int16>() { static ClassId id("Int16"); return id; }
template <>
inline const ClassId &CLASS_ID<Int32>() { static ClassId id("Int32"); return id; }

template <>
inline const ClassId &CLASS_ID<Uint8>() { static ClassId id("Uint8"); return id; }
template <>
inline const ClassId &CLASS_ID<Uint16>() { static ClassId id("Uint16"); return id; }
template <>
inline const ClassId &CLASS_ID<Uint32>() { static ClassId id("Uint32"); return id; }

template <>
inline const ClassId &CLASS_ID<Float32>() { static ClassId id("Float32"); return id; }
template <>
inline const ClassId &CLASS_ID<Float64>() { static ClassId id("Float64"); return id; }

template <>
inline const ClassId &CLASS_ID<String>() { static ClassId id("String"); return id; }

#endif

