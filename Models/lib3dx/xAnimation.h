#ifndef __incl_lib3dx_xAnimation_h
#define __incl_lib3dx_xAnimation_h

#include "../../Math/xMath.h"
#include "xModel.h"
#include <string>

struct xKeyFrame
{
    xQuaternion *QT_bones;   // bone transformations of the current key frame
    xWORD        T_freeze;   // ms of waiting before animation
    xWORD        T_duration; // ms of animation duration

    xKeyFrame   *Prev;   // prev key frame
    xKeyFrame   *Next;   // next key frame

    void LoadFromSkeleton(const xSkeleton &spine);
};

struct xAnimationInfo
{
    int I_frameNo;
    int T_progress;
    int T_duration;
};

struct xAnimation
{
    std::string Name;
    xWORD       I_bones;      // no of bones per frame
    xBYTE       I_priority;   // higher priority matrices replace matrices of lower priority

    xKeyFrame  *L_frames;     // key frame list or cycle
    xWORD       I_frames;     // no of key frames

    xKeyFrame  *CurrentFrame; // current frame pointer
    xLONG       T_progress;   // ms of progress of the current frame

    xAnimation() { Clear(); }

    void Clear()
    {
        Name.clear();
        I_bones        = 0;
        L_frames       = NULL;
        I_frames       = 0;
        CurrentFrame   = NULL;
        T_progress     = 0;
    }

    void Create(xWORD I_bones)
    {
        assert ( Name.size() );
        this->I_bones  = I_bones;
        InsertKeyFrame();
    }

    void Destroy()
    {
        while (L_frames && I_frames)
        {
            xKeyFrame *KF_next = L_frames->Next;
            if (L_frames->QT_bones) delete[] L_frames->QT_bones;
            delete L_frames;
            L_frames = KF_next;
            --I_frames;
        }
        Clear();
    }

    xAnimationInfo GetInfo();

    xKeyFrame   * InsertKeyFrame();
    void          DeleteKeyFrame();

    void          UpdatePosition();
    xQuaternion   GetTransformation(xBYTE ID_bone);
    xQuaternion * GetTransformations();
    void          SaveToSkeleton(xSkeleton &spine);

    bool          Save(const char *fileName);
    bool          Load(const char *fileName);

    static xQuaternion * Interpolate(xQuaternion *pCurr, xQuaternion *pNext,
                                     xFLOAT progress, xWORD boneC);
    static void          Combine    (xQuaternion *pCurr, xQuaternion *pNext,
                                     xWORD boneC, xQuaternion *&bones);
    static void          Average    (xQuaternion *pCurr, xQuaternion *pNext,
                                     xWORD boneC, xFLOAT progress, xQuaternion *&bones);
    static void          Average    (xQuaternion *pCurr, xQuaternion *pNext,
                                     xWORD boneC, xFLOAT *progress, xQuaternion *&bones);
};

#endif
