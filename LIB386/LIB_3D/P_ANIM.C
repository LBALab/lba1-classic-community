#include <stddef.h>
#include <string.h>

#include "ADELINE.H"
#include "LIB_SYS/LIB_SYS.H"
#include "LIB_3D/LIB_3D.H"

/*
 * Helper accessors for packed animation records.
 */

static WORD read_word(UBYTE *ptr)
{
    return *(WORD *)ptr;
}

static ULONG read_dword(UBYTE *ptr)
{
    return *(ULONG *)ptr;
}

static void write_word(UBYTE *ptr, WORD value)
{
    *(WORD *)ptr = value;
}

static void write_dword(UBYTE *ptr, ULONG value)
{
    *(ULONG *)ptr = value;
}

static UBYTE *Offset_Source;
static UBYTE *Offset_Dest;
static UBYTE *StartInfos;

ULONG MemoTimerRef = 0;
LONG CmptMemoTimerRef = 0;

WORD AnimStepX = 0;
WORD AnimStepY = 0;
WORD AnimStepZ = 0;

WORD AnimMasterRot = 0;
WORD AnimStepAlpha = 0;
WORD AnimStepBeta = 0;
WORD AnimStepGamma = 0;

static size_t get_frame_stride(WORD nbGroups)
{
    return (size_t)nbGroups * 8u + 8u;
}

static UBYTE *get_object_anim_base(UBYTE *ptrobj)
{
    return ptrobj + 14;
}

static UBYTE *get_object_anim_state(UBYTE *ptrobj)
{
    return ptrobj + 16;
}

static UBYTE *get_group_data(UBYTE *base, WORD *nbGroups)
{
    WORD paramLen;
    WORD nbPoints;
    WORD groups;
    UBYTE *ptr;

    paramLen = read_word(base);
    ptr = base + paramLen + 2;

    nbPoints = read_word(ptr);
    ptr += 2 + nbPoints * 6;

    groups = read_word(ptr);
    if (nbGroups != 0)
    {
        *nbGroups = groups;
    }

    return ptr + 10;
}

static WORD PatchType(UBYTE **groupPtr)
{
    WORD type = read_word(Offset_Dest);
    Offset_Dest += 2;

    write_word(*groupPtr, type);
    *groupPtr += 2;

    Offset_Source += 2;

    return type;
}

static void PatchInterStep(UBYTE **groupPtr, WORD currentStep, WORD totalSteps)
{
    WORD lastValue;
    WORD newValue;
    LONG delta;

    lastValue = read_word(Offset_Source);
    Offset_Source += 2;

    newValue = read_word(Offset_Dest);
    Offset_Dest += 2;

    if (newValue != lastValue)
    {
        delta = (LONG)newValue - (LONG)lastValue;
        delta = (delta * currentStep) / totalSteps;
        lastValue = (WORD)(lastValue + delta);
    }

    write_word(*groupPtr, lastValue);
    *groupPtr += 2;
}

static void PatchInterAngle(UBYTE **groupPtr, WORD currentStep, WORD totalSteps)
{
    WORD lastAngle;
    WORD targetAngle;
    INT base;
    INT diff;

    lastAngle = read_word(Offset_Source);
    Offset_Source += 2;

    targetAngle = read_word(Offset_Dest);
    Offset_Dest += 2;

    base = (INT)(lastAngle & 1023);
    diff = (INT)(targetAngle & 1023) - base;

    if (diff != 0)
    {
        if (diff < -512)
        {
            diff += 1024;
        }
        else if (diff > 512)
        {
            diff -= 1024;
        }

        diff = (diff * (INT)currentStep) / (INT)totalSteps;
        base = (base + diff) & 1023;
    }

    write_word(*groupPtr, (WORD)base);
    *groupPtr += 2;
}

void SaveTimer(void)
{
    if (CmptMemoTimerRef == 0)
    {
        MemoTimerRef = TimerRef;
    }
    CmptMemoTimerRef++;
}

void RestoreTimer(void)
{
    if (CmptMemoTimerRef > 0)
    {
        CmptMemoTimerRef--;
        if (CmptMemoTimerRef == 0)
        {
            TimerRef = MemoTimerRef;
        }
    }
}

void SetAngleGroupe(LONG numgroupe, WORD palpha, WORD pbeta, WORD pgamma, UBYTE *ptrobj)
{
    UBYTE *ptr;
    WORD info;
    WORD sizeInfo;
    WORD nbPoints;
    WORD nbGroupes;

    ptr = ptrobj;
    info = read_word(ptr);
    if ((info & INFO_ANIM) == 0)
    {
        return;
    }

    ptr += 14;

    sizeInfo = read_word(ptr);
    ptr += sizeInfo + 4;

    nbPoints = read_word(ptr - 2);
    ptr += nbPoints * 6;

    nbGroupes = read_word(ptr);
    if (numgroupe < 0 || numgroupe >= nbGroupes)
    {
        return;
    }

    ptr += 2 + (size_t)numgroupe * 38 + 10;

    write_word(ptr, palpha);
    write_word(ptr + 2, pbeta);
    write_word(ptr + 4, pgamma);
}

void GetAngleGroupe(LONG numgroupe, UBYTE *ptrobj)
{
    UBYTE *ptr;
    WORD info;
    WORD sizeInfo;
    WORD nbPoints;
    WORD nbGroupes;

    ptr = ptrobj;
    info = read_word(ptr);
    if ((info & INFO_ANIM) == 0)
    {
        return;
    }

    ptr += 14;

    sizeInfo = read_word(ptr);
    ptr += sizeInfo + 4;

    nbPoints = read_word(ptr - 2);
    ptr += nbPoints * 6;

    nbGroupes = read_word(ptr);
    if (numgroupe < 0 || numgroupe >= nbGroupes)
    {
        return;
    }

    ptr += 2 + (size_t)numgroupe * 38 + 10;

    AnimStepAlpha = read_word(ptr);
    AnimStepBeta = read_word(ptr + 2);
    AnimStepGamma = read_word(ptr + 4);
}

WORD GetNbFramesAnim(UBYTE *ptranim)
{
    return read_word(ptranim);
}

WORD GetBouclageAnim(UBYTE *ptranim)
{
    return read_word(ptranim + 4);
}

LONG SetAnimObjet(WORD numframe, UBYTE *ptranim, UBYTE *ptrobj)
{
    WORD nbFrames;
    WORD nbGroupsAnim;
    size_t frameStride;
    UBYTE *framePtr;
    UBYTE *animBase;
    WORD nbGroupsObj;
    UBYTE *groupDest;
    UBYTE *src;
    WORD remaining;

    nbFrames = read_word(ptranim);
    if ((UWORD)numframe >= (UWORD)nbFrames)
    {
        return 0;
    }

    nbGroupsAnim = read_word(ptranim + 2);
    frameStride = get_frame_stride(nbGroupsAnim);
    framePtr = ptranim + 8 + frameStride * numframe;

    if ((read_word(ptrobj) & INFO_ANIM) == 0)
    {
        return 0;
    }

    animBase = get_object_anim_base(ptrobj);
    write_dword(animBase + 2, (ULONG)(unsigned long)framePtr);
    write_dword(animBase + 6, TimerRef);

    groupDest = get_group_data(animBase, &nbGroupsObj);

    if (nbGroupsAnim > nbGroupsObj)
    {
        nbGroupsAnim = nbGroupsObj;
    }

    src = framePtr + 8;
    remaining = nbGroupsAnim;
    while (remaining > 0)
    {
        memcpy(groupDest, src, 8);
        src += 8;
        groupDest += 38;
        remaining--;
    }

    AnimStepX = read_word(framePtr + 2);
    AnimStepY = read_word(framePtr + 4);
    AnimStepZ = read_word(framePtr + 6);
    AnimMasterRot = read_word(framePtr + 8);
    AnimStepAlpha = read_word(framePtr + 10);
    AnimStepBeta = read_word(framePtr + 12);
    AnimStepGamma = read_word(framePtr + 14);

    return 1;
}

LONG SetInterAnimObjet(WORD framedest, UBYTE *ptranimdest, UBYTE *ptrobj)
{
    WORD nbGroupsAnim;
    size_t frameStride;
    UBYTE *framePtr;
    WORD timeToFrame;
    UBYTE *state;
    ULONG offsetSource;
    ULONG memoTicks;
    UBYTE *animBase;
    WORD nbGroupsObj;
    UBYTE *groupDest;
    ULONG elapsed;
    WORD totalSteps;
    WORD currentStep;
    UBYTE *savedFramePtr;
    WORD remaining;
    UBYTE *objGroupPtr;

    nbGroupsAnim = read_word(ptranimdest + 2);
    frameStride = get_frame_stride(nbGroupsAnim);
    framePtr = ptranimdest + 8 + frameStride * framedest;

    Offset_Dest = framePtr;

    timeToFrame = read_word(framePtr);

    if ((read_word(ptrobj) & INFO_ANIM) == 0)
    {
        return 0;
    }

    state = get_object_anim_state(ptrobj);
    StartInfos = state;

    offsetSource = read_dword(state + 0);
    memoTicks = read_dword(state + 4);

    if (offsetSource == 0)
    {
        memoTicks = timeToFrame;
        offsetSource = (ULONG)(unsigned long)framePtr;
    }
    Offset_Source = (UBYTE *)(unsigned long)offsetSource;

    animBase = state - 2;
    groupDest = get_group_data(animBase, &nbGroupsObj);

    if (nbGroupsAnim > nbGroupsObj)
    {
        nbGroupsAnim = nbGroupsObj;
    }

    elapsed = TimerRef - memoTicks;
    if (elapsed >= (ULONG)timeToFrame)
    {
        UBYTE *src;
        UBYTE *dest;
        WORD count;

        src = framePtr + 8;
        dest = groupDest;
        count = nbGroupsAnim;

        while (count > 0)
        {
            memcpy(dest, src, 8);
            src += 8;
            dest += 38;
            count--;
        }

        write_dword(state + 0, (ULONG)(unsigned long)framePtr);
        write_dword(state + 4, TimerRef);

        AnimStepX = read_word(framePtr + 2);
        AnimStepY = read_word(framePtr + 4);
        AnimStepZ = read_word(framePtr + 6);
        AnimMasterRot = read_word(framePtr + 8);
        AnimStepAlpha = read_word(framePtr + 10);
        AnimStepBeta = read_word(framePtr + 12);
        AnimStepGamma = read_word(framePtr + 14);

        return 1;
    }

    totalSteps = timeToFrame;
    currentStep = (WORD)elapsed;
    if (totalSteps == 0)
    {
        totalSteps = 1;
    }

    savedFramePtr = Offset_Dest;

    Offset_Source += 8;
    Offset_Dest += 8;

    AnimMasterRot = read_word(Offset_Dest);
    AnimStepAlpha = (WORD)((read_word(Offset_Dest + 2) * currentStep) / totalSteps);
    AnimStepBeta = (WORD)((read_word(Offset_Dest + 4) * currentStep) / totalSteps);
    AnimStepGamma = (WORD)((read_word(Offset_Dest + 6) * currentStep) / totalSteps);

    Offset_Source += 8;
    Offset_Dest += 8;

    remaining = nbGroupsAnim;
    objGroupPtr = groupDest;

    if (remaining > 0)
    {
        objGroupPtr += 38;
        remaining--;
    }

    while (remaining > 0)
    {
        WORD type;

        type = PatchType(&objGroupPtr);
        if (type == TYPE_ROTATE)
        {
            PatchInterAngle(&objGroupPtr, currentStep, totalSteps);
            PatchInterAngle(&objGroupPtr, currentStep, totalSteps);
            PatchInterAngle(&objGroupPtr, currentStep, totalSteps);
        }
        else if (type == TYPE_TRANSLATE || type == TYPE_ZOOM)
        {
            PatchInterStep(&objGroupPtr, currentStep, totalSteps);
            PatchInterStep(&objGroupPtr, currentStep, totalSteps);
            PatchInterStep(&objGroupPtr, currentStep, totalSteps);
        }
        else
        {
            objGroupPtr += 6;
        }

        objGroupPtr += 30;
        remaining--;
    }

    AnimStepX = (WORD)((read_word(savedFramePtr + 2) * currentStep) / totalSteps);
    AnimStepY = (WORD)((read_word(savedFramePtr + 4) * currentStep) / totalSteps);
    AnimStepZ = (WORD)((read_word(savedFramePtr + 6) * currentStep) / totalSteps);

    return 0;
}

LONG SetInterAnimObjet2(WORD framedest, UBYTE *ptranimdest, UBYTE *ptrobj)
{
    WORD nbGroupsAnim;
    size_t frameStride;
    UBYTE *framePtr;
    WORD timeToFrame;
    UBYTE *state;
    ULONG offsetSource;
    ULONG memoTicks;
    UBYTE *animBase;
    WORD nbGroupsObj;
    UBYTE *groupDest;
    ULONG elapsed;
    WORD totalSteps;
    WORD currentStep;
    UBYTE *objGroupPtr;

    nbGroupsAnim = read_word(ptranimdest + 2);
    frameStride = get_frame_stride(nbGroupsAnim);
    framePtr = ptranimdest + 8 + frameStride * framedest;

    Offset_Dest = framePtr;

    timeToFrame = read_word(framePtr);

    if ((read_word(ptrobj) & INFO_ANIM) == 0)
    {
        return 0;
    }

    state = get_object_anim_state(ptrobj);
    StartInfos = state;

    offsetSource = read_dword(state + 0);
    memoTicks = read_dword(state + 4);

    if (offsetSource == 0)
    {
        memoTicks = timeToFrame;
        offsetSource = (ULONG)(unsigned long)framePtr;
    }
    Offset_Source = (UBYTE *)(unsigned long)offsetSource;

    animBase = state - 2;
    groupDest = get_group_data(animBase, &nbGroupsObj);

    if (nbGroupsAnim > nbGroupsObj)
    {
        nbGroupsAnim = nbGroupsObj;
    }

    elapsed = TimerRef - memoTicks;
    if (elapsed >= (ULONG)timeToFrame)
    {
        UBYTE *src;
        UBYTE *dest;
        WORD count;

        src = framePtr + 8;
        dest = groupDest;
        count = nbGroupsAnim;

        while (count > 0)
        {
            memcpy(dest, src, 8);
            src += 8;
            dest += 38;
            count--;
        }

        write_dword(state + 0, (ULONG)(unsigned long)framePtr);

        return 1;
    }

    totalSteps = timeToFrame;
    currentStep = (WORD)elapsed;
    if (totalSteps == 0)
    {
        totalSteps = 1;
    }

    Offset_Dest += 16;
    Offset_Source += 16;

    objGroupPtr = groupDest;

    if (nbGroupsAnim > 0)
    {
        objGroupPtr += 38;
        nbGroupsAnim--;
    }

    while (nbGroupsAnim > 0)
    {
        WORD type;

        type = PatchType(&objGroupPtr);
        if (type == TYPE_ROTATE)
        {
            PatchInterAngle(&objGroupPtr, currentStep, totalSteps);
            PatchInterAngle(&objGroupPtr, currentStep, totalSteps);
            PatchInterAngle(&objGroupPtr, currentStep, totalSteps);
        }
        else if (type == TYPE_TRANSLATE || type == TYPE_ZOOM)
        {
            PatchInterStep(&objGroupPtr, currentStep, totalSteps);
            PatchInterStep(&objGroupPtr, currentStep, totalSteps);
            PatchInterStep(&objGroupPtr, currentStep, totalSteps);
        }
        else
        {
            objGroupPtr += 6;
        }

        objGroupPtr += 30;
        nbGroupsAnim--;
    }

    return 0;
}

LONG SetInterDepObjet(WORD framedest, UBYTE *ptranimdest, UBYTE *ptrobj)
{
    WORD nbGroupsAnim;
    size_t frameStride;
    UBYTE *framePtr;
    WORD timeToFrame;
    UBYTE *state;
    ULONG offsetSource;
    ULONG memoTicks;
    ULONG elapsed;
    WORD totalSteps;
    WORD currentStep;

    nbGroupsAnim = read_word(ptranimdest + 2);
    frameStride = get_frame_stride(nbGroupsAnim);
    framePtr = ptranimdest + 8 + frameStride * framedest;

    Offset_Dest = framePtr;

    timeToFrame = read_word(framePtr);

    if ((read_word(ptrobj) & INFO_ANIM) == 0)
    {
        return 0;
    }

    state = get_object_anim_state(ptrobj);
    StartInfos = state;

    offsetSource = read_dword(state + 0);
    memoTicks = read_dword(state + 4);

    if (offsetSource == 0)
    {
        offsetSource = (ULONG)(unsigned long)framePtr;
        memoTicks = timeToFrame;
    }
    Offset_Source = (UBYTE *)(unsigned long)offsetSource;

    elapsed = TimerRef - memoTicks;
    if (elapsed >= (ULONG)timeToFrame)
    {
        write_dword(state + 0, (ULONG)(unsigned long)Offset_Dest);
        write_dword(state + 4, TimerRef);

        AnimStepX = read_word(framePtr + 2);
        AnimStepY = read_word(framePtr + 4);
        AnimStepZ = read_word(framePtr + 6);
        AnimMasterRot = read_word(framePtr + 8);
        AnimStepAlpha = read_word(framePtr + 10);
        AnimStepBeta = read_word(framePtr + 12);
        AnimStepGamma = read_word(framePtr + 14);

        return 1;
    }

    totalSteps = timeToFrame;
    currentStep = (WORD)elapsed;
    if (totalSteps == 0)
    {
        totalSteps = 1;
    }

    Offset_Source += 8;
    Offset_Dest += 8;

    AnimMasterRot = read_word(Offset_Dest);
    AnimStepAlpha = (WORD)((read_word(Offset_Dest + 2) * currentStep) / totalSteps);
    AnimStepBeta = (WORD)((read_word(Offset_Dest + 4) * currentStep) / totalSteps);
    AnimStepGamma = (WORD)((read_word(Offset_Dest + 6) * currentStep) / totalSteps);

    Offset_Dest += 8;
    Offset_Source += 8;

    AnimStepX = (WORD)((read_word(framePtr + 2) * currentStep) / totalSteps);
    AnimStepY = (WORD)((read_word(framePtr + 4) * currentStep) / totalSteps);
    AnimStepZ = (WORD)((read_word(framePtr + 6) * currentStep) / totalSteps);

    return 0;
}

WORD StockInterAnim(UBYTE *ptranimbuf, UBYTE *ptrobj)
{
    UBYTE *state;
    UBYTE *animBase;
    WORD nbGroups;
    UBYTE *src;
    UBYTE *dest;
    WORD remaining;

    if ((read_word(ptrobj) & INFO_ANIM) == 0)
    {
        return 0;
    }

    state = get_object_anim_state(ptrobj);
    write_dword(state + 4, TimerRef);
    write_dword(state + 0, (ULONG)(unsigned long)ptranimbuf);

    animBase = state - 2;
    src = get_group_data(animBase, &nbGroups);

    dest = ptranimbuf + 8;
    remaining = nbGroups;

    while (remaining > 0)
    {
        memcpy(dest, src, 8);
        src += 38;
        dest += 8;
        remaining--;
    }

    return (WORD)get_frame_stride(nbGroups);
}

void CopyInterAnim(UBYTE *ptrobjs, UBYTE *ptrobjd)
{
    UBYTE *srcState;
    UBYTE *dstState;
    UBYTE *srcBase;
    UBYTE *dstBase;
    WORD srcGroups;
    WORD dstGroups;
    UBYTE *src;
    UBYTE *dst;
    WORD count;

    if ((read_word(ptrobjs) & INFO_ANIM) == 0)
    {
        return;
    }

    if ((read_word(ptrobjd) & INFO_ANIM) == 0)
    {
        return;
    }

    srcState = get_object_anim_state(ptrobjs);
    dstState = get_object_anim_state(ptrobjd);

    write_dword(dstState + 0, read_dword(srcState + 0));
    write_dword(dstState + 4, read_dword(srcState + 4));

    srcBase = srcState - 2;
    dstBase = dstState - 2;

    src = get_group_data(srcBase, &srcGroups);
    dst = get_group_data(dstBase, &dstGroups);

    count = srcGroups;
    if (dstGroups < count)
    {
        count = dstGroups;
    }

    while (count > 0)
    {
        memcpy(dst, src, 8);
        src += 38;
        dst += 38;
        count--;
    }
}
