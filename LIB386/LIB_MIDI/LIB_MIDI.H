#ifndef _MAX_PATH
#define _MAX_PATH 260
#endif

extern char Midi_Driver[_MAX_PATH];
extern char Midi_Driver_Name[80];
extern WORD Midi_Driver_Enable;

extern char GTL_filename[_MAX_PATH]; // fichier des sons

extern void AskMidiVars(char ***, long **);
extern long InitMidiDLL(char *);
extern long InitMidi(void);
extern void InitPathMidiSampleFile(unsigned char *);
extern void ClearMidi(void);
extern void PlayMidi(unsigned char *);
extern void StopMidi(void);
extern long IsMidiPlaying(void);
extern void FadeMidiDown(short);
extern void FadeMidiUp(short);
extern void WaitFadeMidi(void);
extern void VolumeMidi(short);
extern void SetLoopMidi(short);
extern void DoLoopMidi(void);

extern void InitMidiTimer(void);
