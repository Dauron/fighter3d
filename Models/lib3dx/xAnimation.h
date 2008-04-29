#ifndef __incl_lib3dx_xAnimation_h
#define __incl_lib3dx_xAnimation_h

#include "../../Math/xMath.h"
#include "xModel.h"

struct xKeyFrame
{
    xVector4  *boneP;    // bone transformations of the current key frame
    xWORD      freeze;   // ms of waiting before animation
    xWORD      duration; // ms of animation duration

    xKeyFrame *prev;   // prev key frame
    xKeyFrame *next;   // next key frame

    void LoadFromSkeleton(xBone *spine);
};

struct xAnimationInfo
{
    int FrameNo;
    int Progress;
    int Duration;
};

struct xAnimation
{
    char      *name;
    xWORD      boneC;    // no of bones per frame
    xBYTE      priority; // higher priority matrices replace matrices of lower priority

    xKeyFrame *frameP;   // key frame list or cycle
    xWORD      frameC;   // no of key frames

    xKeyFrame *frameCurrent; // current frame pointer
    xLONG      progress;     // ms of progress of the current frame

    void Reset(xWORD boneC)
    {
        this->boneC  = boneC;
        name         = NULL;
        frameP       = NULL;
        frameC       = 0;
        frameCurrent = NULL;
        progress     = 0;
        InsertKeyFrame();
    }
    void Unload()
    {
        if (name)
            delete[] name;
        xKeyFrame *kfCur = frameP;
        xWORD cnt = frameC;
        while (kfCur && cnt)
        {
            frameP = kfCur->next;
            if (kfCur->boneP) delete[] kfCur->boneP;
            delete kfCur;
            kfCur = frameP;
            --cnt;
        }
    }

    xAnimationInfo GetInfo();

    xKeyFrame *InsertKeyFrame();
    void       DeleteKeyFrame();

    void       UpdatePosition();
    xVector4 * GetTransformations();
    void       SaveToSkeleton(xBone *spine);

    bool       Save(const char *fileName);
    bool       Load(const char *fileName);

    static void       SaveToSkeleton(xBone *spine, xVector4 *transf);
    static xVector4 * Interpolate(xVector4 *pCurr, xVector4 *pNext, xFLOAT progress, xWORD boneC);
    static void       Combine(xVector4 *pCurr, xVector4 *pNext, xWORD boneC, xVector4 *&bones);
};

#endif
