#include <cstdio>
#include "xAnimation.h"

#ifdef WIN32
#pragma warning(disable : 4996) // deprecated
#endif

void xKeyFrame::LoadFromSkeleton(const xSkeleton &spine)
{
    xIKNode *node = spine.boneP;
    for (int i = spine.boneC; i; --i, ++node)
        boneP[node->id] = node->quaternion;
}

xAnimationInfo xAnimation::GetInfo()
{
    xAnimationInfo info;
    info.Duration = 0;
    info.FrameNo  = 0;
    info.Progress = progress;

    if (!frameP) return info;

    bool pastCurrent = false;
    int cnt = this->frameC;
    for (xKeyFrame *frame = frameP; frame && cnt; frame = frame->next, --cnt)
    {
        if (frameCurrent == frame)
            pastCurrent = true;
        if (!pastCurrent) {
            info.Progress += frame->freeze + frame->duration;
            ++info.FrameNo;
        }
        info.Duration += frame->freeze + frame->duration;
    }
    ++info.FrameNo;

    return info;
}

void           xAnimation::SaveToSkeleton(xSkeleton &spine, xVector4 *transf)
{
    xIKNode *node = spine.boneP;
    for (int i = spine.boneC; i; --i, ++node)
        node->quaternion = transf[node->id];
}
void           xAnimation::SaveToSkeleton(xSkeleton &spine)
{
    xVector4 *transf   = GetTransformations();
    xIKNode  *node = spine.boneP;
    for (int i = spine.boneC; i; --i, ++node)
        node->quaternion = transf[node->id];
    delete[] transf;
}
void           xAnimation::UpdatePosition()
{
    if (!this->frameCurrent)
    {
        this->progress = 0;
        return;
    }

    if (progress < 0)
    {
        this->frameCurrent = this->frameCurrent->prev;
        while (this->frameCurrent && progress < 0)
        {
            this->progress += this->frameCurrent->freeze + this->frameCurrent->duration;
            if (progress < 0)
                this->frameCurrent = this->frameCurrent->prev;
        }
        if (!this->frameCurrent)
            this->progress = 0;
    }
    else
    {
        while (this->frameCurrent &&
               this->progress > this->frameCurrent->freeze + this->frameCurrent->duration)
        {
            this->progress -= this->frameCurrent->freeze + this->frameCurrent->duration;
            this->frameCurrent = this->frameCurrent->next;
        }
        if (!this->frameCurrent)
            this->progress = 0;
    }
}

xVector4 *     xAnimation::GetTransformations()
{
    xVector4 *bones = new xVector4[this->boneC];
    xVector4 *pRes = bones;

    if (!this->frameCurrent)
    {
        for (int i = this->boneC; i; --i, ++pRes )
            pRes->zeroQ();
        return bones;
    }

    xKeyFrame *fCurrent = this->frameCurrent;
    xVector4  *pCurr    = fCurrent->boneP;

    xLONG time = this->progress - fCurrent->freeze;
    if (time <= 0)                         // still freeze
    {
        for (int i = this->boneC; i; --i, ++pRes, ++pCurr )
            *pRes = *pCurr;
        return bones;
    }

    float complement = time / (float)fCurrent->duration; // get percentage of the frame transition
    float progress = 1.f - complement;

    if (fCurrent->next)
    {
        xVector4 *pPCurr = fCurrent->prev ? fCurrent->prev->boneP : pCurr;
        xVector4 *pNext = fCurrent->next->boneP;
        xVector4 *pNNext = fCurrent->next->next ? fCurrent->next->next->boneP : pNext;

        for (int i = this->boneC; i; --i, ++pCurr, ++pNext, ++pRes, ++pPCurr, ++pNNext )
        {
            if (i == this->boneC) // root
            {
                *pRes = *pCurr * progress + *pNext * complement;
                pRes->w = 1.f;
                continue;
            }
            if (pCurr->w != 1.f && pNext->w != 1.f)
            {
                xVector4 a = xQuaternion::spline(*pPCurr, *pCurr, *pNext);
                xVector4 b = xQuaternion::spline(*pCurr, *pNext, *pNNext);
                *pRes = xQuaternion::squad(*pCurr, *pNext, a, b, complement);
                //*pRes = xQuaternion::slerp(*pCurr, *pNext, complement);
                /*
                xFLOAT cosA = xVector4::DotProduct(*pCurr, *pNext);
                if (cosA > 0.99f)
                {
                    *pRes = *pCurr * progress + *pNext * complement;
                    continue;
                }
                xFLOAT angle = acos(cosA);
                if (angle > PI/2)
                    angle = PI-angle;

                xVector4 q = *pCurr *sin(angle*progress) + *pNext *sin(angle*complement);
                *pRes = q / sin(angle);
                */
                continue;
            }
            if (pCurr->w != 1.f)
            {
                if (pCurr->w > 0.99f)
                {
                    pRes->init(pCurr->vector3 * progress, complement + pCurr->w * progress);
                    continue;
                }
                *pRes = xQuaternion::interpolate(*pCurr, progress);
                continue;
            }
            if (pNext->w != 1.f)
            {
                if (pNext->w > 0.99f)
                {
                    pRes->init(pNext->vector3 * complement, progress + pNext->w * complement);
                    continue;
                }
                *pRes = xQuaternion::interpolate(*pNext, complement);
                continue;
            }
            pRes->zeroQ();
        }
    }
    else
    {
        for (int i = this->boneC; i; --i, ++pCurr, ++pRes )
        {
            if (i == this->boneC) // root
            {
                *pRes = *pCurr * progress;
                pRes->w = 1.f;
                continue;
            }
            if (pCurr->w != 1.f)
            {
                if (pCurr->w > 0.99f)
                {
                    pRes->init(pCurr->vector3 * progress, complement + pCurr->w * progress);
                    continue;
                }
                *pRes = xQuaternion::interpolate(*pCurr, progress);
                continue;
            }
            pRes->zeroQ();
        }
    }

    return bones;
}

xVector4 *     xAnimation::Interpolate(xVector4 *pCurr, xVector4 *pNext, xFLOAT progress, xWORD boneC)
{

    xVector4 *bones = new xVector4[boneC];
    xVector4 *pRes = bones;

    if (!pCurr)
    {
        for (int i = boneC; i; --i, ++pRes )
            pRes->zeroQ();
        return bones;
    }

    if (progress == 0.f)
    {
        for (int i = boneC; i; --i, ++pRes, ++pCurr )
            *pRes = *pCurr;
        return bones;
    }

    float complement = progress;
    progress = 1.f - progress;

    if (pNext)
    {
        for (int i = boneC; i; --i, ++pCurr, ++pNext, ++pRes )
        {
            if (i == boneC) // root
            {
                *pRes = *pCurr * progress + *pNext * complement;
                pRes->w = 1.f;
                continue;
            }
            if (pCurr->w != 1.f && pNext->w != 1.f)
            {
                xFLOAT cosA = xVector4::DotProduct(*pCurr, *pNext);
                if (cosA > 0.99f)
                {
                    *pRes = *pCurr * progress + *pNext * complement;
                    continue;
                }
                xFLOAT angle = acos(cosA);
                if (angle > PI/2)
                    angle = PI-angle;

                xVector4 q = *pCurr *sin(angle*progress) + *pNext *sin(angle*complement);
                *pRes = q / sin(angle);
                continue;
            }
            if (pCurr->w != 1.f)
            {
                if (pCurr->w > 0.99f)
                {
                    *pRes = *pCurr * progress;
                    continue;
                }
                *pRes = xQuaternion::interpolate(*pCurr, progress);
                continue;
            }
            if (pNext->w != 1.f)
            {
                if (pNext->w > 0.99f)
                {
                    *pRes = *pNext * complement;
                    continue;
                }
                *pRes = xQuaternion::interpolate(*pNext, complement);
                continue;
            }
            pRes->zeroQ();
        }
    }
    else
    {
        for (int i = boneC; i; --i, ++pCurr, ++pRes )
        {
            if (i == boneC) // root
            {
                *pRes = *pCurr * progress;
                pRes->w = 1.f;
                continue;
            }
            if (pCurr->w != 1.f)
            {
                if (pCurr->w > 0.99f)
                {
                    *pRes = *pCurr * progress;
                    continue;
                }
                *pRes = xQuaternion::interpolate(*pCurr, progress);
                continue;
            }
            pRes->zeroQ();
        }
    }

    return bones;
}
void           xAnimation::Combine(xVector4 *pCurr, xVector4 *pNext, xWORD boneC, xVector4 *&bones)
{
    if (!bones) bones = new xVector4[boneC];
    xVector4 *pRes = bones;

    if (!pCurr)
    {
        for (int i = boneC; i; --i, ++pRes )
            pRes->zeroQ();
        return;
    }

    if (!pNext)
    {
        for (int i = boneC; i; --i, ++pCurr, ++pRes )
            *pRes = *pCurr;
        return;
    }

    for (int i = boneC; i; --i, ++pCurr, ++pNext, ++pRes )
    {
        if (i == boneC) // root
        {
            *pRes = *pCurr + *pNext;
            pRes->w = 1.f;
            continue;
        }
        *pRes = xQuaternion::product(*pCurr, *pNext);
    }
    return;
}
xKeyFrame *    xAnimation::InsertKeyFrame()
{
    xKeyFrame *kfNew = new xKeyFrame();
    kfNew->duration = 1000;
    kfNew->freeze   = 0;

    if (!frameP)
    {
        frameC = 1;
        kfNew->next     = NULL;
        kfNew->prev     = NULL;
        kfNew->boneP    = new xVector4[boneC];
        for (int i=0; i<boneC; ++i) kfNew->boneP[i].zeroQ();

        frameCurrent = frameP = kfNew;
        progress = 0;

        return kfNew;
    }

    if (!frameCurrent)
    {
        frameCurrent = frameP;
        progress = 0;
    }

    ++frameC;

    if (progress == frameCurrent->freeze + frameCurrent->duration)
    {
        kfNew->boneP = new xVector4[boneC];
        if (kfNew->next)
            for (int i=0; i<boneC; ++i) kfNew->boneP[i] = kfNew->next->boneP[i];
        else
            for (int i=0; i<boneC; ++i) kfNew->boneP[i].zeroQ();

        kfNew->next = frameCurrent->next;
        kfNew->prev = frameCurrent;
        frameCurrent->next = kfNew;
        if (kfNew->next) kfNew->next->prev = kfNew;
    }
    else if (progress == 0)
    {
        kfNew->boneP = new xVector4[boneC];
        for (int i=0; i<boneC; ++i) kfNew->boneP[i] = frameCurrent->boneP[i];

        kfNew->next = frameCurrent;
        kfNew->prev = frameCurrent->prev;
        frameCurrent->prev = kfNew;
        if (kfNew->prev) kfNew->prev->next = kfNew;
        if (frameP == frameCurrent) frameP = kfNew;
    }
    else
    {
        kfNew->boneP = GetTransformations();

        if (progress > frameCurrent->freeze)
        {
            kfNew->duration = frameCurrent->freeze + frameCurrent->duration
                - progress;
            kfNew->freeze   = 0;
            frameCurrent->duration = progress - frameCurrent->freeze;
        }
        else
        {
            kfNew->duration = 1;
            kfNew->freeze   = frameCurrent->freeze - progress;
            frameCurrent->duration = 1;
            frameCurrent->freeze = progress;
        }

        kfNew->next = frameCurrent->next;
        kfNew->prev = frameCurrent;
        frameCurrent->next = kfNew;
        if (kfNew->next) kfNew->next->prev = kfNew;
    }
    frameCurrent = kfNew;
    progress = 0;
    return kfNew;
}
void           xAnimation::DeleteKeyFrame()
{
    if (!frameCurrent)
        return;
    if (frameP == frameCurrent)
        frameP = frameP->next;
    if (frameCurrent->prev)
    {
        frameCurrent->prev->next = frameCurrent->next;
        progress = frameCurrent->prev->duration;
        frameCurrent->prev->duration = progress + frameCurrent->duration;
    }
    else
        progress = 0;
    if (frameCurrent->next) frameCurrent->next->prev = frameCurrent->prev;
    if (frameCurrent->boneP) delete[] frameCurrent->boneP;

    xKeyFrame *tmp = frameCurrent;
    frameCurrent = frameCurrent->prev ? frameCurrent->prev : frameP;
    delete tmp;

    --frameC;
}
bool           xAnimation::Save(const char *fileName)
{
    FILE *file;
    file = fopen(fileName, "wb");
    if (file)
    {
        xAnimationInfo info = GetInfo();

        xBYTE i = (name) ? strlen(name) : 0;
        fwrite(&i, sizeof(xBYTE), 1, file);
        fwrite(this->name,        sizeof(char), i, file);
        fwrite(&(this->boneC),    sizeof(this->boneC), 1, file);
        fwrite(&(this->priority), sizeof(this->priority), 1, file);
        fwrite(&(this->frameC),   sizeof(this->frameC), 1, file);
        fwrite(&(info.FrameNo),   sizeof(int), 1, file);
        fwrite(&(this->progress), sizeof(this->progress), 1, file);

        bool loop = this->frameP && this->frameP->prev;
        fwrite(&loop,             sizeof(bool), 1, file);

        int cnt = this->frameC;
        for (xKeyFrame *frame = frameP; frame && cnt; frame = frame->next, --cnt)
        {
            fwrite(frame->boneP,       sizeof(xVector4), this->boneC, file);
            fwrite(&(frame->freeze),   sizeof(frame->freeze), 1, file);
            fwrite(&(frame->duration), sizeof(frame->duration), 1, file);
        }

        fclose(file);
        return true;
    }
    return false;
}

bool           xAnimation::Load(const char *fileName)
{
    FILE *file;
    file = fopen(fileName, "rb");
    if (file)
    {
        xBYTE i;
        fread(&i, sizeof(xBYTE), 1, file);
        if (i) {
            this->name = new char[i];
            fread(this->name,        sizeof(char), i, file);
            delete[] this->name;
        }
        this->name = strdup(fileName); // overwrite name with path

        fread(&(this->boneC),    sizeof(this->boneC), 1, file);
        fread(&(this->priority), sizeof(this->priority), 1, file);
        fread(&(this->frameC),   sizeof(this->frameC), 1, file);
        int frameNo;
        fread(&frameNo,          sizeof(int), 1, file);
        fread(&(this->progress), sizeof(this->progress), 1, file);

        bool loop;
        fread(&loop,             sizeof(bool), 1, file);

        xKeyFrame *frame = NULL, *prevFrame = NULL;
        for (int i=0; i<this->frameC; ++i)
        {
            frame = new xKeyFrame();
            if (i == 0) this->frameP = frame;
            frame->boneP = new xVector4[this->boneC];
            fread(frame->boneP, sizeof(xVector4), this->boneC, file);
            fread(&(frame->freeze),   sizeof(frame->freeze), 1, file);
            fread(&(frame->duration), sizeof(frame->duration), 1, file);
            if (i == frameNo-1) this->frameCurrent = frame;
            if (prevFrame) prevFrame->next = frame;
            frame->prev = prevFrame;
            prevFrame = frame;
        }
        if (frame)
            if (loop)
            {
                frame->next = this->frameP;
                this->frameP->prev = frame->next;
            }
            else
                frame->next = NULL;

        fclose(file);
        return true;
    }
    return false;
}
