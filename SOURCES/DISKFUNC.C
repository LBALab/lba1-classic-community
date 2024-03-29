#include	"c_extern.h"
#include	"direct.h"

#include	<dos.h>

UBYTE	*PtrSce ;
UBYTE	*PtrScene ;

#define	GET_BYTE	(*PtrSce++)
#define	GET_WORD	(*(WORD*)PtrSce) ; PtrSce+=2

/*══════════════════════════════════════════════════════════════════════════*
	     █▀▀▀▄  █    ██▀▀▀ █  ▄▀       █▀▀▀▀ █   █ ██▄ █ █▀▀▀▀
	     ██  █  ██   ▀▀▀▀█ ██▀▄        ██▀▀  ██  █ ██▀██ ██
	     ▀▀▀▀   ▀▀   ▀▀▀▀▀ ▀▀  ▀ ▀▀▀▀▀ ▀▀    ▀▀▀▀▀ ▀▀  ▀ ▀▀▀▀▀
 *══════════════════════════════════════════════════════════════════════════*/
/*──────────────────────────────────────────────────────────────────────────*/

void	GetDiskEnv( UBYTE *progpath )
{
	_splitpath( (char*)progpath, ProgDrive, ProgDir, Name, Ext ) ;
	getcwd( OrgDir, _MAX_DIR ) ;
	_dos_getdrive( &OrgDrive ) ;
}

void	RestoreDiskEnv()
{
	UINT	total ;

	_dos_setdrive( OrgDrive, &total ) ;
	chdir( OrgDir ) ;
}

/*══════════════════════════════════════════════════════════════════════════*
	  ██▀▀▀ █▀▀▀▀ █▀▀▀▀ ██▄ █ █▀▀▀▀       █▀▀▀▀  █    █     █▀▀▀▀
	  ▀▀▀▀█ ██    ██▀▀  ██▀██ ██▀▀        ██▀▀   ██   ██    ██▀▀
	  ▀▀▀▀▀ ▀▀▀▀▀ ▀▀▀▀▀ ▀▀  ▀ ▀▀▀▀▀ ▀▀▀▀▀ ▀▀     ▀▀   ▀▀▀▀▀ ▀▀▀▀▀
 *══════════════════════════════════════════════════════════════════════════*/
/*──────────────────────────────────────────────────────────────────────────*/

WORD	LoadScene( WORD numscene )
{
	T_OBJET	*ptrobj ;
	WORD	n, info3 ;
	WORD	indexfile3d;
	UBYTE	string[256] ;
	UBYTE	mess[256] ;
	WORD	sizetoload ;

//	PtrScene = PtrSce = LoadMalloc_HQR( PATH_RESSOURCE"scene.hqr", numscene ) ;

	HQRM_Load( PATH_RESSOURCE"scene.hqr", numscene, (void**)&PtrScene ) ;
	CHECK_MEMORY

	PtrSce = PtrScene ;

	if( !PtrSce )
	{
		Message( "Scene.Hqr introuvable ou pas assez mem", TRUE ) ;
	}

// info world: INFO_WORLD

		NewCube = numscene ;

		Island = GET_BYTE ;

		GameOverCube = GET_BYTE ;

		n = GET_WORD ;
		n = GET_WORD ;

//		ListHoloObj[NUM_PERSO].Alpha = GET_WORD ;
//		ListHoloObj[NUM_PERSO].Beta = GET_WORD ;
//		ListHoloObj[NUM_PERSO].Size = GET_WORD ;

// ambiance: AMBIANCE

		AlphaLight = GET_WORD ;
		BetaLight = GET_WORD ;

		SampleAmbiance[0] = GET_WORD ;
		SampleRepeat[0] = GET_WORD ;
		SampleRnd[0] = GET_WORD ;
		SampleAmbiance[1] = GET_WORD ;
		SampleRepeat[1] = GET_WORD ;
		SampleRnd[1] = GET_WORD ;
		SampleAmbiance[2] = GET_WORD ;
		SampleRepeat[2] = GET_WORD ;
		SampleRnd[2] = GET_WORD ;
		SampleAmbiance[3] = GET_WORD ;
		SampleRepeat[3] = GET_WORD ;
		SampleRnd[3] = GET_WORD ;
		SecondMin = GET_WORD ;
		SecondEcart = GET_WORD ;

		CubeJingle = GET_BYTE ;
//		PlayMidiFile( CubeJingle ) ;

// hero inits: HERO_START

		ptrobj = ListObjet ;

		CubeStartX = GET_WORD ;
		CubeStartY = GET_WORD ;
		CubeStartZ = GET_WORD ;

		sizetoload = GET_WORD ;
		ptrobj->PtrTrack = PtrSce ;
		PtrSce += sizetoload ;

		sizetoload = GET_WORD ;
		ptrobj->PtrLife = PtrSce ;
		PtrSce += sizetoload ;

		ptrobj++ ;

// objets: OBJECT

		NbObjets = GET_WORD ;

		for (n = 1; n < NbObjets; n++, ptrobj++)
		{
			T_OBJET* savedPtrObj;
			
			if (HasLoadedListObjetsOnSave)
			{
				savedPtrObj = malloc(sizeof(T_OBJET));

				if (savedPtrObj)
					*savedPtrObj = *ptrobj;
			}

			InitObject(n);

			ptrobj->Flags = GET_WORD;

			indexfile3d = GET_WORD;

			if (!(ptrobj->Flags & SPRITE_3D))
			{

				HQRM_Load(PATH_RESSOURCE"File3D.hqr", indexfile3d, (void**)&ptrobj->PtrFile3D);
				CHECK_MEMORY

					/*				ptrobj->PtrFile3D =
										LoadMalloc_HQR(
											PATH_RESSOURCE"File3D.hqr",
											indexfile3d ) ;
					*/

			}

			ptrobj->GenBody = GET_BYTE;
			ptrobj->GenAnim = GET_BYTE;
			/*
			if( n==5 )
			{
				CoulText( 15,0 ) ;
				Text( 10, 300, "obj 5 org anim: %d", ptrobj->GenAnim ) ;
			}
			*/

			ptrobj->Sprite = GET_WORD;

			ptrobj->OldPosX = ptrobj->PosObjX = GET_WORD;
			ptrobj->OldPosY = ptrobj->PosObjY = GET_WORD;
			ptrobj->OldPosZ = ptrobj->PosObjZ = GET_WORD;
			ptrobj->HitForce = GET_BYTE;
			ptrobj->OptionFlags = GET_WORD;
			ptrobj->OptionFlags &= ~EXTRA_GIVE_NOTHING;
			ptrobj->Beta = GET_WORD;
			ptrobj->SRot = GET_WORD;
			ptrobj->Move = GET_WORD;

			ptrobj->Info = GET_WORD;
			ptrobj->Info1 = GET_WORD;
			ptrobj->Info2 = GET_WORD;
			ptrobj->Info3 = GET_WORD;

			ptrobj->NbBonus = GET_BYTE;
			ptrobj->CoulObj = GET_BYTE;
			ptrobj->Armure = GET_BYTE;
			ptrobj->LifePoint = GET_BYTE;

			sizetoload = GET_WORD;
			ptrobj->PtrTrack = PtrSce;
			PtrSce = PtrSce + sizetoload;

			sizetoload = GET_WORD;
			ptrobj->PtrLife = PtrSce;
			PtrSce = PtrSce + sizetoload;

			//If loading from a save, replace content with the saved one (but keep the SCENE.HQR file reading flow from LoadScene for other necessary readings afterwards) 
			if (HasLoadedListObjetsOnSave && savedPtrObj)
			{
				//If object was saved while getting hit, starting animation will be the one from SCENE.HQR file (this is to avoid a glitch when a NPC is in the middle of an animation when a save is made)
				if (savedPtrObj->GenAnim == GEN_ANIM_CHOC || savedPtrObj->GenAnim == GEN_ANIM_CHOC2)
					savedPtrObj->GenAnim = ptrobj->GenAnim;

				//This will retain NPC and object position and other states when loading a save
				*ptrobj = *savedPtrObj;

				//Do not render object if it's marked as dead on save.
				if (savedPtrObj->LifePoint <= 0)
				{
					ptrobj->GenBody = NO_BODY;
					ptrobj->GenAnim = NO_ANIM;
					ptrobj->Anim = NO_ANIM;
					ptrobj->WorkFlags |= OBJ_DEAD;
					ptrobj->Sprite = -1;
					ptrobj->Body = -1;

					ptrobj->ZoneSce = -1;
					ptrobj->OffsetTrack = -1;

					//Do not reset OffsetLife if the object is a meca penguin (because if it's in inventory, it's marked as dead in code, and it's still needed when used)
					if (n != NumPingouin)
						ptrobj->OffsetLife = -1;
				}

				free(savedPtrObj);
			}
		}

// zone declechement: ZONE
		NbZones = GET_WORD ;
		if (!HasLoadedListZoneOnSave) //add flag for zones
			ListZone = (T_ZONE*)PtrSce ;
		PtrSce += NbZones * 12 * 2 ;

// point track: TRACK

		NbBrickTrack = GET_WORD ;
		ListBrickTrack = (T_TRACK*)PtrSce ;

		HasLoadedListZoneOnSave = 0;

		return TRUE ;
}

/*══════════════════════════════════════════════════════════════════════════*
			 █▀▀▀▄ █▀▀▀▀ █▀▀█  █   █ █▀▀▀▀
			 ██  █ ██▀▀  ██▀▀█ ██  █ ██ ▀█
			 ▀▀▀▀  ▀▀▀▀▀ ▀▀▀▀▀ ▀▀▀▀▀ ▀▀▀▀▀
 *══════════════════════════════════════════════════════════════════════════*/
/*──────────────────────────────────────────────────────────────────────────*/

/*
void	*LoadTestMalloc( char *filename )
{
	void	*ptr ;
	UBYTE	string[256] ;

	ptr = LoadMalloc( filename ) ;

	if( !ptr )
	{
		strcpy( string, "ARG ! Mem:" ) ;
		strcat( string, Itoa( (ULONG)Malloc(-1) ) ) ;
		strcat( string, " fichier:" ) ;
		strcat( string, filename ) ;

		Message( string, TRUE ) ;
	}
	return ptr ;
}
*/

/*══════════════════════════════════════════════════════════════════════════*/
/*══════════════════════════════════════════════════════════════════════════*/

