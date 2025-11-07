#include "DEFINES.H"

/* define _FAR before including any definitions, so it can be used */
#ifdef __DOS__
#define _FAR __far
#else
#define _FAR
#endif

/*══════════════════════════════════════════════════════════════════════════*/
/*──────────────────────────────────────────────────────────────────────────*/

/* prototype */

char *GetMultiText(LONG text, char *dst);

WORD GiveObjSelect(WORD type, LONG xm, LONG ym);

UBYTE WorldColBrick(WORD xw, WORD yw, WORD zw);

WORD ThrowExtra(WORD owner,
				WORD x, WORD y, WORD z, WORD num,
				WORD alpha, WORD beta, WORD vitesse, WORD poids,
				WORD hitforce);
WORD GetValue();

WORD GetListObjet(WORD x0, WORD y0);

LONG SceReadWord();
WORD SearchLastZoneSce();

ULONG HQ_MixSample(WORD numsample, WORD decalage, WORD repeat, WORD volleft, WORD volright);
void HQ_StopOneSample(WORD num);

void *LoadTestMalloc(char *filename);

WORD InitAnim(UBYTE gen_anim, WORD flag, UBYTE gen_nextanim, WORD numobj);
WORD SearchAnim(UBYTE numanim, WORD numobj);
WORD SearchBody(UBYTE numbody, WORD numobj);
void LoadFic(UBYTE indexfileobjet, WORD numobj);

/*--------- divers ---------------------------------------------*/

extern UWORD *BufOrder;
extern UBYTE *BufText;
extern UBYTE *BufSpeak;

extern UBYTE GamePathname[];

extern WORD ComportementDebug;

extern UBYTE *LbaFont;
extern UBYTE *Screen;
extern UBYTE PalettePcx[];
extern UBYTE *PtrPal;

extern WORD MyJoy;
extern WORD MyFire;
extern WORD MyKey;

extern WORD LastFire;

extern LONG FlagVsync;
extern WORD FlagMCGA;
extern LONG FlagCredits;

extern UBYTE String[];
extern UBYTE EditString[];
extern WORD Value;

extern LONG SampleVolume, MusicVolume, CDVolume, LineVolume, MasterVolume;
extern WORD SamplesEnable;
extern UBYTE *PtrSample[];

extern WORD QuickMessage;
extern LONG FlecheForcee;

extern WORD FlagPalettePcx;
extern WORD FlagFade;
extern WORD FlagMessageShade;
extern WORD Bulle;
extern WORD FlagBlackPal;

extern WORD FlaFromCD;
extern UBYTE PathFla[];

extern LONG Version_US;

extern WORD FlagWindowsFilenameSaving;

extern void ChangeBalance(short, unsigned char, unsigned char, unsigned char);
extern void LoadNextFrameFla(void);
extern void DrawNextFrameFla(void);
extern short InitFla(char *);
extern void ClearFla(void);
extern void GestionPalette(void);
extern void PlayAnimFla(char *);
extern void CutPoint(unsigned char *);
extern short SearchFla(unsigned char *, unsigned char *, long, unsigned char *);
extern void PlayDiskFla(unsigned char *);

/*-------- RESSOURCES ---------------------------------------------*/

extern T_HQR_HEADER *HQR_Samples;
extern T_HQR_HEADER *HQR_Midi;
extern T_HQR_HEADER *InventoryObj;

/*-------- DISKFUNC.C ---------------------------------------------*/

extern char PleaseWait[];

extern UBYTE ProgDrive[];
extern UBYTE ProgDir[];

extern UINT OrgDrive;
extern UBYTE OrgDir[];

extern UBYTE Drive[];
extern UBYTE Dir[];
extern UBYTE Name[];
extern UBYTE Ext[];

extern UBYTE OrgPathname[];
extern UBYTE ProgramPathname[];

extern UBYTE Pathname[];

extern UBYTE FliPathname[_MAX_PATH];

extern void GetDiskEnv(unsigned char *);
extern void RestoreDiskEnv(void);
extern short LoadScene(short);

/*-------- SCRIPT.C ---------------------------------------------*/

extern UBYTE FicName[];
extern UBYTE *PtrFiche;
extern UBYTE *PtrCommand;
extern UBYTE *PtrFicName;

/*-------- FLIP_BOX.C ---------------------------------------------*/

extern WORD NbPhysBox;
extern WORD NbOptPhysBox;

extern void AddOptBox(short, short, short, short);
extern void AddPhysBox(short, short, short, short);
extern void FlipOptList(void);
extern void FlipBoxes(void);
extern void ClsBoxes(void);

/*-------- LIB_SVGA ---------------------------------------------*/

extern WORD NbPolyPoints;

/*-------- life tool ---------------------------------------------*/

extern WORD ErrorFlag;

extern WORD FlagInfos;

extern WORD Modif;

extern UBYTE DefPathname[];
extern UBYTE ScePathname[];
extern UBYTE PcxPathname[];

extern WORD FlagFrame;

extern T_ZONE *ListZone;
extern WORD NbZones;

extern WORD IndexZone;
extern WORD EditNumZone;
extern WORD EditTypeZone;
extern WORD VisuTypeZone;

extern UBYTE *ListTypeZone[];

extern T_TRACK *ListBrickTrack;
extern WORD NbBrickTrack;

extern WORD IndexBrickTrack;
extern WORD EditNumTrack;

extern WORD ActiveEdit;

/*
extern	T_FLAG	ListFlagCube[] ;
extern	T_FLAG	ListFlagGame[] ;
*/
extern UBYTE ListFlagCube[];
extern UBYTE ListFlagGame[];

extern T_AUX_FLAG_CUBE ListAuxFlagCube[]; // auxiliary cube flags set aside from the original ListFlagCube, to aid in manual saving

extern UBYTE ListFlagInventory[];

extern UBYTE *ListMove[];

extern void DoFuncLife(T_OBJET *);
extern short DoTest(void);
extern void DoLife(short);

extern void DoTrack(short);

/*--------- holomap ----------------------------*/

// extern	T_HOLO_OBJ	ListHoloObj[MAX_HOLO_OBJ] ;
// extern	WORD		NbHoloObj ;

extern T_HOLO_POS *ListHoloPos; // [MAX_HOLO_POS] ;
extern UBYTE TabHoloPos[];

extern void ComputeCoorMapping(void);
extern void ComputeCoorGlobe(void);
extern int z_sort(void const *, void const *);
extern void ComputeGlobeProj(void);
extern void DrawHoloMap(short, short, short);
extern void DrawListPos(short, short, short, short);
extern long SearchNextArrow(long);
extern long SearchPrevArrow(long);
extern void SetHoloPos(unsigned char);
extern void ClrHoloPos(unsigned char);
extern void DrawTitle(short, short, short);
extern void DrawCurseur(void);
extern void InitHoloDatas(void);
extern void HoloMap(void);
extern void DrawHoloObj(unsigned char *, short, short, short);
extern short *GiveTrajPtr(long);
extern void HoloTraj(short);

/*--------- menus -----------------------------*/

extern WORD GameChoice;
extern WORD GameNbChoices;
extern WORD GameListChoice[];

extern void Init3DGame(void);
extern void DrawFire(short, short, short, short, short);
extern void WaitReleaseSpace(void);
extern void DrawObj3D(short, short, short, short, short, short, unsigned char *);
extern void DrawCadre(short, short, short, short);
extern void DrawComportement(short, short, short);
extern void DrawInfoMenu(short, short);
extern void DrawMenuComportement(short);
extern void MenuComportement(void);
extern void Draw3dObject(short, short, short *, short, short);
extern void DrawOneInventory(short);
extern void DrawListInventory(void);
extern void Inventory(void);
extern void DoFoundObj(short);
extern void ExtInitMcga(void);
extern void ExtInitSvga(void);
extern void TestRestoreModeSVGA(long);
extern void CopyBlockPhysMCGA(long, long, long, long);
extern short PlayerGameList(unsigned char **, unsigned char *, short, short);
extern short FindPlayerFile(void);
extern void SaveGame(void);
extern void LoadGame(void);
extern void DrawOneString(short, short, unsigned char *, short);
extern void DrawSingleString(short, short, unsigned char *);
extern void DrawOneLetter(short, short, short);
extern void DrawAllLetters(void);
extern short InputPlayerName(short, short);
extern short ChoosePlayerName(short, short, short);
extern void DrawOneChoice(short, short, short, short, short);
extern void InfoWallCollisionDamage();
extern void DrawGameMenu(unsigned short *, short);
extern void SetVolumes(long, long, long, long, long);
extern short DoGameMenu(unsigned short *);
extern long OptionsMenu(void);
extern void VolumeOptions(void);
extern void SavedGameManagement(void);
extern void AdvancedOptions(void);
extern long MainGameMenu(void);
extern long QuitMenu(short);
extern void GameAskChoice(short);
extern void GameOver(void);
extern void Credits(void);
extern void RessPict(short);
extern void TimerPause(short);
extern void AdelineLogo(void);
extern void MenuDemo(void);

/*--------- perso ----------------------------*/

extern WORD PersoInvulnerable;

extern WORD FlagTheEnd;

extern WORD ActionNormal;
extern WORD InventoryAction;
extern WORD Weapon;

extern WORD Comportement;
extern WORD AnimRien;
extern WORD AnimRienNormal, AnimRienSportif, AnimRienAgressif, AnimRienDiscret, AnimRienProtopack;

extern UBYTE *PtrFile3dNormal;
extern UBYTE *PtrFile3dSportif;
extern UBYTE *PtrFile3dAgressif;
extern UBYTE *PtrFile3dDiscret;
extern UBYTE *PtrFile3dProtopack;

extern WORD SceZoom;
extern WORD Shadow;
extern WORD ShadowX, ShadowY, ShadowZ;
extern UBYTE ShadowCol;

extern WORD NumHolomapTraj;
extern WORD FlagChgCube, NewCube, NumCube;
extern WORD NewPosX, NewPosY, NewPosZ;
extern WORD SceneStartX, SceneStartY, SceneStartZ;
extern WORD CubeStartX, CubeStartY, CubeStartZ;
extern WORD GameOverCube;

extern WORD CombatAuto;
extern WORD MagicBall;
extern WORD MagicBallType;
extern WORD MagicBallCount;

extern WORD MagicLevel;
extern WORD MagicPoint;
extern WORD NbGoldPieces;
extern WORD NbLittleKeys;
extern WORD Chapitre;
extern UBYTE Island;
extern WORD Fuel;
extern WORD NumPingouin;

extern WORD HasLoadedSave;
extern WORD HasLoadedListObjetsOnSave;
extern WORD HasLoadedListObjetTracksOnSave;
extern WORD HasLoadedListExtraOnSave;
extern WORD HasLoadedListZoneOnSave;
extern WORD HasLoadedKeysOnSave;
extern WORD HasLoadedInventoryOnSave;
extern WORD HasLoadedListFlagCubeOnSave;
extern WORD HasLoadedListAuxFlagCubeOnSave;
extern WORD HasLoadedLastValidPersoOnSave;
extern WORD DisableAutoSave;

extern WORD NbFourLeafClover;
extern WORD NbCloverBox;

extern WORD FlagClimbing;
extern WORD StartYFalling;
extern T_REAL_VALUE RealFalling;
extern WORD StepFalling;

extern UBYTE CodeJeu;
extern WORD FlagWater;

// perso sauvegarde entrée cube

extern WORD SaveCube;
extern WORD SaveComportement;

extern char SaveStringBody[SIZE_NAME + 1];
extern BYTE SaveGenBody;

extern WORD SaveBeta;
extern WORD SaveNbGoldPieces;
extern WORD SaveMagicLevel;
extern WORD SaveMagicPoint;
extern WORD SaveLifePoint;
extern WORD SaveChapitre;
extern WORD SaveNbCloverBox;
extern WORD SaveNbFourLeafClover;

extern WORD WallColDamageEnabled; // 0: disabled, 1: enabled

extern void InitGameLists(void);
extern void InitGame(int, unsigned char **);
extern void Introduction(void);
extern long MainLoop(void);
extern void ReadVolumeSettings(void);
extern void WriteVolumeSettings(void);
extern void InitProgram(void);
extern void TheEnd(short, unsigned char *);
extern int _FAR Critical_Error_Handler(unsigned int, unsigned int, unsigned int _FAR *);
extern void Message(unsigned char *, short);
extern void main(int, unsigned char **);

/*--------- Extra objs ----------------------------*/

extern T_EXTRA ListExtra[];

// extern	UBYTE	*PtrSpriteExtra ;
extern T_HQR_HEADER *HQRPtrSpriteExtra;
extern WORD *PtrZvExtra;

extern void InitFly(T_EXTRA *, short, short, short, short);
extern void BounceExtra(T_EXTRA *, short, short, short);
extern short ExtraBonus(short, short, short, short, short, short, short);
extern short ExtraExplo(short, short, short);
extern short SearchBonusKey(void);
extern short ExtraSearch(short, short, short, short, short, short, short, short);
extern short ExtraSearchKey(short, short, short, short, short, short);
extern short ExtraCheckObjCol(T_EXTRA *, short);
extern short ExtraCheckExtraCol(T_EXTRA *, short);
extern unsigned char FullWorldColBrick(short, short, short, short, short, short);
extern void ClearScreenMinMax(void);
extern void AdjustScreenMax(void);
extern void Aff2DShape(short *, short, short, short, short, short);
extern void AffSpecial(short, short, short);
extern void InitSpecial(short, short, short, short);
extern long CoulRetourBalle(T_EXTRA *);
extern void ClearExtra(void);
extern short ThrowExtra(short, short, short, short, short, short, short, short, short, short);
extern void GiveExtraBonus(T_OBJET *);
extern void ZoneGiveExtraBonus(T_ZONE *);
extern void GereExtras(void);

extern void ThrowMagicBall(short, short, short, short, short, short, unsigned char);
extern void GereAnimAction(T_OBJET *, short);
extern short SearchAnim(unsigned char, short);
extern short SearchBody(unsigned char, short);

/*---------------- incrust display ----------------*/

extern T_INCRUST_DISP ListIncrustDisp[];

extern void InitIncrustDisp(short, short, short, short, short, short, short);
extern void ChangeIncrustPos(short, short, short, short);
extern void DrawBulle(short);

/*-------- Moteur 3D ---------------------------------------------*/

extern UBYTE *BufferShadow;

extern UBYTE *BufferAnim;
extern UBYTE *PtrBufferAnim;

extern LONG NbBodys;
extern UBYTE *PtrBody[];
extern T_HQR_HEADER *HQR_Anims;

extern LONG NbObjets;
extern T_OBJET ListObjet[];
extern T_OBJET LastValidPerso;

extern T_SORT ListTri[];

extern WORD NumObjFollow;

extern void InitObject(short);
extern void StartInitObj(short);
extern void StartInitAllObjs(void);
extern void LoadFicPerso(void);
extern void InitPerso(void);
extern void SetComportement(short);
extern void RestartPerso(void);
extern void ClearFlagsCube(void);
extern void ClearScene(void);
extern void ChangeCube(void);
extern void HitObj(short, short, short, short);
extern long CheckZvOnZv(short, short);
extern short CheckObjCol(short);
extern void CheckCarrier(short);
extern long CheckValidObjPos(short);
extern void InitBody(unsigned char, short);
extern void InitSprite(short, short);
extern short InitAnim(unsigned char, short, unsigned char, short);
extern void ReajustPos(unsigned char);
extern void ReceptionObj(void);
extern void DoCornerReajust(short, short, short, short);
extern void DoCornerReajustTwinkel(short, short, short, short);
extern void DoAnim(short);
extern void ManualRealAngle(T_OBJET *);
extern void ClearRealAngle(T_OBJET *);
extern void DoDir(short);
extern void AffScene(long);
extern void CheckZoneSce(T_OBJET *, short);

/*--------- ambiance ----------------------------*/

extern WORD SampleAmbiance[];
extern WORD SampleRepeat[];
extern WORD SampleRnd[];
extern WORD SamplePlayed;
extern ULONG TimerNextAmbiance;
extern WORD SecondEcart;
extern WORD SecondMin;
extern WORD CubeJingle;
extern UBYTE *PtrXmi;
extern WORD NumXmi;

extern unsigned long HQ_MixSample(short, short, short, short, short);
extern void HQ_StopSample(void);
extern void HQ_StopOneSample(short);
extern short GiveBalance(short, short, long, unsigned long *, unsigned long *);
extern void HQ_3D_MixSample(short, short, short, short, short, short);
extern void HQ_ChangeBalanceSamples(short, short);
extern void GereAmbiance(void);
extern void FadePal(unsigned char, unsigned char, unsigned char, unsigned char *, short);
extern void FadeToBlack(unsigned char *);
extern void WhiteFade(void);
extern void FadeWhiteToPal(unsigned char *);
extern void FadeToPal(unsigned char *);
extern void SetBlackPal(void);
extern void FadePalToPal(unsigned char *, unsigned char *);
extern void FadeToRed(unsigned char *);
extern void FadeRedToPal(unsigned char *);
extern void FadeMusicMidi(unsigned long);
extern void StopMusicMidi(void);
extern void PlayMusic(short);
extern void PlayMidiFile(short);

/*-------- Brick ISO ---------------------------------------------*/

#ifdef BRICK_HQR
extern UBYTE *BufferBrick;
#endif
extern UBYTE *BufCube;

extern WORD FirstTime;
extern WORD CameraZone;

extern LONG StartXCube;
extern LONG StartYCube;
extern LONG StartZCube;

extern LONG WorldXCube;
extern LONG WorldYCube;
extern LONG WorldZCube;

extern WORD XpOrgw;
extern WORD YpOrgw;

extern WORD LastXmBrickSelected;
extern WORD LastYmBrickSelected;
extern WORD LastZmBrickSelected;

extern UBYTE *BufCollis;

extern WORD CubeClip;
extern WORD CubeClipX, CubeClipY, CubeClipZ;

extern WORD FlagAffGrille;

extern long LoadUsedBrick(unsigned long);
extern long InitGrille(unsigned short);
extern void FreeGrille(void);
extern unsigned char *GetAdrColonneMap(long, long);
extern unsigned char *GetAdrColonneCube(long, long);
extern void GetShadow(short, short, short);
extern unsigned long CreateMaskGph(unsigned char *, unsigned char *);
extern void MixteMapToCube(unsigned char *);
extern void CopyMapToCube(void);
extern void InitBufferCube(void);
extern void DrawOverBrick(short, short, short);
extern void DrawOverBrick3(short, short, short);
extern void AffBrickBlock(long, long, long, long, long);
extern void AffGrille(void);
extern void IncrustGrm(short);

/*--------- Message ---------------------------------------------*/

extern WORD NumObjSpeak;
extern LONG FlagSpeak;

extern void TimeBar(unsigned long, unsigned long);
extern void InitLanguage(void);
extern long FindText(long);
extern void InitDial(long);
extern void ClearDial(void);
extern void GetNextWord(char *, char *);
extern void PushCar(long, long, long);
extern void AffOneCar(long, long, long, long);
extern void AffAllCar(void);
extern void CoulDial(long, long, long);
extern void TestCoulDial(short);
extern long GetText(long);
extern void AffFleche(void);
extern void InitDialWindow(void);
extern void SecondInitDialWindow(void);
extern void NormalWinDial(void);
extern void BigWinDial(void);
extern void CommonOpenDial(long);
extern void OpenDial(long);
extern void OpenDialNoWindow(long);
extern void GetNextLine(void);
extern void InitEndPage(void);
extern long NextDialCar(void);
extern void CloseDial(void);
extern void Dial(long);
extern void MyDial(short);
extern char *GetMultiText(long, char *);
extern char *GetCurrentListLanguage();

/*--------- custom text ---------------------------------------------*/

extern char *GetCustomizedMultiText(LONG numParam);
