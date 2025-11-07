#include "lib_sys/adeline.h"
#include "lib_sys/lib_sys.h"
#include "lib_midi/lib_midi.h"

// Stub implementations for when AIL32 is not available

WORD Midi_Driver_Enable = FALSE;
LONG MaxVolume = 100;

void AskMidiVars(char ***listidentifier, LONG **ptrvars)
{
    *listidentifier = NULL;
    *ptrvars = NULL;
}

LONG InitMidiDLL(UBYTE *driverpathname)
{
    return FALSE;
}

LONG InitMidi(void)
{
    return FALSE;
}

void InitPathMidiSampleFile(UBYTE *path)
{
    // Stub
}

void ClearMidi(void)
{
    // Stub
}

void PlayMidi(UBYTE *ail_buffer)
{
    // Stub
}

void StopMidi(void)
{
    // Stub
}

LONG IsMidiPlaying(void)
{
    return FALSE;
}

void FadeMidiDown(WORD nbsec)
{
    // Stub
}

void FadeMidiUp(WORD nbsec)
{
    // Stub
}

void WaitFadeMidi(void)
{
    // Stub
}

void VolumeMidi(WORD volume)
{
    // Stub
}

void DoLoopMidi(void)
{
    // Stub
}
