#include <cstdio>
#include "xAnimation.h"

#ifdef WIN32
#pragma warning(disable : 4996) // deprecated
#endif

void xKeyFrame::LoadFromSkeleton(const xSkeleton &spine)
{
    spine.QuatsToArray(QT_bones);
}

xAnimationInfo xAnimation::GetInfo()
{
    xAnimationInfo info;
    info.T_duration = 0;
    info.I_frameNo  = 0;
    info.T_progress = T_progress;

    if (!L_frames) return info;

    bool pastCurrent = false;
    int cnt = this->I_frames;
    for (xKeyFrame *frame = L_frames; frame && cnt; frame = frame->Next, --cnt)
    {
        if (CurrentFrame == frame)
            pastCurrent = true;
        if (!pastCurrent) {
            info.T_progress += frame->T_freeze + frame->T_duration;
            ++info.I_frameNo;
        }
        info.T_duration += frame->T_freeze + frame->T_duration;
    }
    ++info.I_frameNo;

    return info;
}

void           xAnimation::SaveToSkeleton(xSkeleton &spine)
{
    xQuaternion *transf   = GetTransformations();
    spine.QuatsFromArray(transf);
    delete[] transf;
}
void           xAnimation::UpdatePosition()
{
    if (!this->CurrentFrame)
    {
        this->T_progress = 0;
        return;
    }

    if (T_progress < 0)
    {
        this->CurrentFrame = this->CurrentFrame->Prev;
        while (this->CurrentFrame && T_progress < 0)
        {
            this->T_progress += this->CurrentFrame->T_freeze + this->CurrentFrame->T_duration;
            if (T_progress < 0)
                this->CurrentFrame = this->CurrentFrame->Prev;
        }
        if (!this->CurrentFrame)
            this->T_progress = 0;
    }
    else
    {
        while (this->CurrentFrame &&
               this->T_progress > this->CurrentFrame->T_freeze + this->CurrentFrame->T_duration)
        {
            this->T_progress -= this->CurrentFrame->T_freeze + this->CurrentFrame->T_duration;
            this->CurrentFrame = this->CurrentFrame->Next;
        }
        if (!this->CurrentFrame)
            this->T_progress = 0;
    }
}

xQuaternion *     xAnimation::GetTransformations()
{
    xQuaternion *bones = new xQuaternion[this->I_bones];
    xQuaternion *pRes = bones;

    if (!this->CurrentFrame)
    {
        for (int i = this->I_bones; i; --i, ++pRes )
            pRes->zeroQ();
        return bones;
    }

    xKeyFrame *fCurrent = this->CurrentFrame;
    xQuaternion  *pCurr    = fCurrent->QT_bones;

    xLONG time = this->T_progress - fCurrent->T_freeze;
    if (time <= 0)                         // still freeze
    {
        for (int i = this->I_bones; i; --i, ++pRes, ++pCurr )
            *pRes = *pCurr;
        return bones;
    }

    float complement = time / (float)fCurrent->T_duration; // get percentage of the frame transition
    float progress = 1.f - complement;

    if (fCurrent->Next)
    {
        xQuaternion *pPCurr = fCurrent->Prev ? fCurrent->Prev->QT_bones : pCurr;
        xQuaternion *pNext = fCurrent->Next->QT_bones;
        xQuaternion *pNNext = fCurrent->Next->Next ? fCurrent->Next->Next->QT_bones : pNext;

        for (int i = this->I_bones; i; --i, ++pCurr, ++pNext, ++pRes, ++pPCurr, ++pNNext )
        {
            if (i == this->I_bones) // root
            {
                *pRes = *pCurr * progress + *pNext * complement;
                pRes->w = 1.f;
                continue;
            }
            if (pCurr->w != 1.f && pNext->w != 1.f)
            {
                //xVector4 a = xQuaternion::spline(*pPCurr, *pCurr, *pNext);
                //xVector4 b = xQuaternion::spline(*pCurr, *pNext, *pNNext);
                //*pRes = xQuaternion::squad(*pCurr, *pNext, a, b, complement);

                *pRes = xQuaternion::SLerp(*pCurr, *pNext, complement);
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
                *pRes = xQuaternion::Interpolate(*pCurr, progress);
                continue;
            }
            if (pNext->w != 1.f)
            {
                if (pNext->w > 0.99f)
                {
                    pRes->init(pNext->vector3 * complement, progress + pNext->w * complement);
                    continue;
                }
                *pRes = xQuaternion::Interpolate(*pNext, complement);
                continue;
            }
            pRes->zeroQ();
        }
    }
    else
    {
        for (int i = this->I_bones; i; --i, ++pCurr, ++pRes )
        {
            if (i == this->I_bones) // root
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
                *pRes = xQuaternion::Interpolate(*pCurr, progress);
                continue;
            }
            pRes->zeroQ();
        }
    }

    return bones;
}

xQuaternion *     xAnimation::Interpolate(xQuaternion *pCurr, xQuaternion *pNext, xFLOAT progress, xWORD I_bones)
{

    xQuaternion *bones = new xQuaternion[I_bones];
    xQuaternion *pRes = bones;

    if (!pCurr)
    {
        for (int i = I_bones; i; --i, ++pRes )
            pRes->zeroQ();
        return bones;
    }

    if (progress == 0.f)
    {
        for (int i = I_bones; i; --i, ++pRes, ++pCurr )
            *pRes = *pCurr;
        return bones;
    }

    float complement = progress;
    progress = 1.f - progress;

    if (pNext)
    {
        for (int i = I_bones; i; --i, ++pCurr, ++pNext, ++pRes )
        {
            if (i == I_bones) // root
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

                xQuaternion q = xQuaternionCast (*pCurr *sin(angle*progress) + *pNext *sin(angle*complement));
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
                *pRes = xQuaternion::Interpolate(*pCurr, progress);
                continue;
            }
            if (pNext->w != 1.f)
            {
                if (pNext->w > 0.99f)
                {
                    *pRes = *pNext * complement;
                    continue;
                }
                *pRes = xQuaternion::Interpolate(*pNext, complement);
                continue;
            }
            pRes->zeroQ();
        }
    }
    else
    {
        for (int i = I_bones; i; --i, ++pCurr, ++pRes )
        {
            if (i == I_bones) // root
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
                *pRes = xQuaternion::Interpolate(*pCurr, progress);
                continue;
            }
            pRes->zeroQ();
        }
    }

    return bones;
}
void           xAnimation::Combine(xQuaternion *pCurr, xQuaternion *pNext, xWORD I_bones, xQuaternion *&bones)
{
    if (!bones) bones = new xQuaternion[I_bones];
    xQuaternion *pRes = bones;

    if (!pCurr)
    {
        for (int i = I_bones; i; --i, ++pRes )
            pRes->zeroQ();
        return;
    }

    if (!pNext)
    {
        for (int i = I_bones; i; --i, ++pCurr, ++pRes )
            *pRes = *pCurr;
        return;
    }

    for (int i = I_bones; i; --i, ++pCurr, ++pNext, ++pRes )
    {
        if (i == I_bones) // root
        {
            *pRes = *pCurr + *pNext;
            pRes->w = 1.f;
            continue;
        }
        *pRes = xQuaternion::Product(*pCurr, *pNext);
    }
    return;
}
void           xAnimation::Average(xQuaternion *pCurr, xQuaternion *pNext, xWORD I_bones, xFLOAT progress, xQuaternion *&bones)
{
    if (!bones) bones = new xQuaternion[I_bones];
    xQuaternion *pRes = bones;

    float complement = progress;
    progress = 1.f - progress;

    if (!pCurr)
    {
        for (int i = I_bones; i; --i, ++pRes )
            pRes->zeroQ();
        return;
    }

    if (!pNext)
    {
        for (int i = I_bones; i; --i, ++pCurr, ++pRes )
            *pRes = *pCurr * progress;
        return;
    }

    for (int i = I_bones; i; --i, ++pCurr, ++pNext, ++pRes )
    {
        if (i == I_bones) // root
        {
            *pRes = *pCurr * progress + *pNext * complement;
            pRes->w = 1.f;
            continue;
        }
        *pRes = xQuaternion::SLerp(*pCurr, *pNext, complement);
    }
    return;
}
xKeyFrame *    xAnimation::InsertKeyFrame()
{
    xKeyFrame *kfNew = new xKeyFrame();
    kfNew->T_duration = 1000;
    kfNew->T_freeze   = 0;

    if (!L_frames)
    {
        I_frames = 1;
        kfNew->Next     = NULL;
        kfNew->Prev     = NULL;
        kfNew->QT_bones    = new xQuaternion[I_bones];
        for (int i=0; i<I_bones; ++i) kfNew->QT_bones[i].zeroQ();

        CurrentFrame = L_frames = kfNew;
        T_progress = 0;

        return kfNew;
    }

    if (!CurrentFrame)
    {
        CurrentFrame = L_frames;
        T_progress = 0;
    }

    ++I_frames;

    if (T_progress == CurrentFrame->T_freeze + CurrentFrame->T_duration)
    {
        kfNew->QT_bones = new xQuaternion[I_bones];
        if (kfNew->Next)
            for (int i=0; i<I_bones; ++i) kfNew->QT_bones[i] = kfNew->Next->QT_bones[i];
        else
            for (int i=0; i<I_bones; ++i) kfNew->QT_bones[i].zeroQ();

        kfNew->Next = CurrentFrame->Next;
        kfNew->Prev = CurrentFrame;
        CurrentFrame->Next = kfNew;
        if (kfNew->Next) kfNew->Next->Prev = kfNew;
    }
    else if (T_progress == 0)
    {
        kfNew->QT_bones = new xQuaternion[I_bones];
        for (int i=0; i<I_bones; ++i) kfNew->QT_bones[i] = CurrentFrame->QT_bones[i];

        kfNew->Next = CurrentFrame;
        kfNew->Prev = CurrentFrame->Prev;
        CurrentFrame->Prev = kfNew;
        if (kfNew->Prev) kfNew->Prev->Next = kfNew;
        if (L_frames == CurrentFrame) L_frames = kfNew;
    }
    else
    {
        kfNew->QT_bones = GetTransformations();

        if (T_progress > CurrentFrame->T_freeze)
        {
            kfNew->T_duration = CurrentFrame->T_freeze + CurrentFrame->T_duration
                - T_progress;
            kfNew->T_freeze   = 0;
            CurrentFrame->T_duration = T_progress - CurrentFrame->T_freeze;
        }
        else
        {
            kfNew->T_duration = 1;
            kfNew->T_freeze   = CurrentFrame->T_freeze - T_progress;
            CurrentFrame->T_duration = 1;
            CurrentFrame->T_freeze = T_progress;
        }

        kfNew->Next = CurrentFrame->Next;
        kfNew->Prev = CurrentFrame;
        CurrentFrame->Next = kfNew;
        if (kfNew->Next) kfNew->Next->Prev = kfNew;
    }
    CurrentFrame = kfNew;
    T_progress = 0;
    return kfNew;
}
void           xAnimation::DeleteKeyFrame()
{
    if (!CurrentFrame)
        return;
    if (L_frames == CurrentFrame)
        L_frames = L_frames->Next;
    if (CurrentFrame->Prev)
    {
        CurrentFrame->Prev->Next = CurrentFrame->Next;
        T_progress = CurrentFrame->Prev->T_duration;
        CurrentFrame->Prev->T_duration = T_progress + CurrentFrame->T_duration;
    }
    else
        T_progress = 0;
    if (CurrentFrame->Next) CurrentFrame->Next->Prev = CurrentFrame->Prev;
    if (CurrentFrame->QT_bones) delete[] CurrentFrame->QT_bones;

    xKeyFrame *tmp = CurrentFrame;
    CurrentFrame = CurrentFrame->Prev ? CurrentFrame->Prev : L_frames;
    delete tmp;

    --I_frames;
}
bool           xAnimation::Save(const char *fileName)
{
    FILE *file;
    file = fopen(fileName, "wb");
    if (file)
    {
        xAnimationInfo info = GetInfo();

        xBYTE i = (Name) ? strlen(Name) : 0;
        fwrite(&i, sizeof(xBYTE), 1, file);
        fwrite(this->Name,          sizeof(char), i, file);
        fwrite(&(this->I_bones),    sizeof(this->I_bones), 1, file);
        fwrite(&(this->I_priority), sizeof(this->I_priority), 1, file);
        fwrite(&(this->I_frames),   sizeof(this->I_frames), 1, file);
        fwrite(&(info.I_frameNo),   sizeof(int), 1, file);
        fwrite(&(this->T_progress), sizeof(this->T_progress), 1, file);

        bool loop = this->L_frames && this->L_frames->Prev;
        fwrite(&loop,               sizeof(bool), 1, file);

        int cnt = this->I_frames;
        for (xKeyFrame *frame = L_frames; frame && cnt; frame = frame->Next, --cnt)
        {
            fwrite(frame->QT_bones,      sizeof(xQuaternion), this->I_bones, file);
            fwrite(&(frame->T_freeze),   sizeof(frame->T_freeze), 1, file);
            fwrite(&(frame->T_duration), sizeof(frame->T_duration), 1, file);
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
            this->Name = new char[i];
            fread(this->Name,        sizeof(char), i, file);
            delete[] this->Name;
        }
        this->Name = strdup(fileName); // overwrite name with path

        fread(&(this->I_bones),    sizeof(this->I_bones), 1, file);
        fread(&(this->I_priority), sizeof(this->I_priority), 1, file);
        fread(&(this->I_frames),   sizeof(this->I_frames), 1, file);
        int frameNo;
        fread(&frameNo,            sizeof(int), 1, file);
        fread(&(this->T_progress), sizeof(this->T_progress), 1, file);

        bool loop;
        fread(&loop,               sizeof(bool), 1, file);

        xKeyFrame *frame = NULL, *prevFrame = NULL;
        for (int i=0; i<this->I_frames; ++i)
        {
            frame = new xKeyFrame();
            if (i == 0) this->L_frames = frame;
            frame->QT_bones = new xQuaternion[this->I_bones];
            fread(frame->QT_bones, sizeof(xQuaternion), this->I_bones, file);
            fread(&(frame->T_freeze),   sizeof(frame->T_freeze), 1, file);
            fread(&(frame->T_duration), sizeof(frame->T_duration), 1, file);
            if (i == frameNo-1) this->CurrentFrame = frame;
            if (prevFrame) prevFrame->Next = frame;
            frame->Prev = prevFrame;
            prevFrame = frame;
        }
        if (frame)
            if (loop)
            {
                frame->Next = this->L_frames;
                this->L_frames->Prev = frame->Next;
            }
            else
                frame->Next = NULL;

        fclose(file);
        return true;
    }
    return false;
}
