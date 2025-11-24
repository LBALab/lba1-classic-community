
//----------------------------------------------------------------------------
extern WORD BufferData[];
extern WORD DriveCDR; //     Lettre drive 0=A 1=B etc. apres InitCDR
extern ULONG LeadOut;
extern UBYTE LowTrack, HigTrack;
extern LONG FileCD_Start;
extern LONG FileCD_Sect;
extern LONG FileCD_Size;

//----------------------------------------------------------------------------
//      Initialiser le lesteur de cdrom, si pas verif alors chaine vide
extern LONG InitCDR(char *nameid);

//      Jouer seulement une track précise
extern LONG PlayTrackCDR(LONG track);

//      Arreter le PlayTrackCDR
extern void StopCDR();

//      Verouiller ou déverouiller la porte du lecteur
extern void LockCDR();
extern void UnLockCDR();

//      Reset, Init ou suite a une erreur par exemple
extern void ResetCDR();

//      Ejecter le cdrom quand c'est possible ( pas toujours motorisé )
extern void EjectCDR();

//      Fermer tiroir electrique quand c'est possible ( pas toujours motorisé )
extern void CloseTrayCDR();

//      Reprends une track apres un stopCDR
extern void ResumeCDR();

//-------------------------------------- Fonction bas niveau

//      Return pointeur sur Copyright File name ( pas forcement file )
extern char *GetCopyrightName();

//      Conversion Red to Hsg ( belle jambe non ;-) )
extern LONG Red2Hsg(LONG value);

//      Return la taille d'une track en HSG
extern LONG GetLengthTrackCDR(LONG track);

//      Return secteur depart Track
extern LONG StartTrackCDR(LONG track);

//      Return status of CD Drive
extern LONG StatusCDR();

//      Return Track currently play or 0
extern LONG GetMusicCDR();

//      Change volume selon chanel
extern void ControlCDR(WORD chan0, WORD vol0, WORD chan1, WORD vol1);

//      Return position de la tete
extern LONG LocateHeadCDR();

//      Play a part of cd
extern LONG PlayCDR(LONG debut, LONG duree);

//      Lecture data
extern LONG ReadLongCDR(LONG start, LONG nbsect, void *buffer);

//	Get Directory Entry
extern LONG GetFileCDR(char *name);

//----------------------------------------------------------------------------
