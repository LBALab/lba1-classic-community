/*
 * Video Initialization - Windows Implementation
 * Little Big Adventure (LBA) Classic Community Edition
 * 
 * This replaces INITSVGA.ASM for Windows builds, providing the same
 * global variables and initialization functions.
 */

/* Include Windows headers first to avoid type conflicts */
#include <windows.h>
#include <string.h>

/* Forward declare platform-specific video functions */
extern void InitModeVesa(void);
extern void NewBankVesa(unsigned short bank);

/*-----------------------------------------------------------------------------
 * Global Variables (exported from INITSVGA.ASM in DOS build)
 *---------------------------------------------------------------------------*/

/* VESA error flag (for compatibility with DOS build) */
unsigned char VESA_Error = 0;   /* 0 = no error, 1 = VESA initialization failed */

/* Framebuffer pointers */
unsigned long Log = 0;          /* Logical video buffer (software buffer) */
unsigned long Phys = 0;         /* Physical video memory (not used on Windows) */
unsigned long MemoLog = 0;      /* Saved Log pointer */

/* Screen dimensions */
unsigned long Screen_X = 640;
unsigned long Screen_Y = 480;

/* Clipping rectangle */
unsigned long ClipXmin = 0;
unsigned long ClipYmin = 0;
unsigned long ClipXmax = 639;
unsigned long ClipYmax = 479;

/* Line offset table for fast Y-coordinate to memory offset conversion */
unsigned long TabOffLine[480];

/* Text rendering */
unsigned char Text_Ink = 15;    /* Text foreground color */
unsigned char Text_Paper = 0xFF; /* Text background color (0xFF = transparent) */
unsigned short SizeCar = 8;     /* Character size */

/* Video state */
unsigned char OldVideo = (unsigned char)-1; /* Previous video mode */

/* Bank switching (not used on Windows - linear framebuffer) */
unsigned long BankChange = 0;
unsigned long BankCurrent = 0;

/* Function pointers for platform-specific implementations */
unsigned long InitSvgaMode = 0; /* Points to InitModeVesa */
unsigned long NewBank = 0;      /* Points to NewBankVesa */
unsigned long Enable = 0;       /* Optional enable function */

/*-----------------------------------------------------------------------------
 * Video Initialization Functions
 *---------------------------------------------------------------------------*/

/* Initialize SVGA/VESA mode with the specified screen dimensions */
void InitSvga(void)
{
    unsigned long i;
    unsigned long scanline;
    
    /* Call platform-specific mode initialization */
    InitModeVesa();
    
    /* Set up function pointers */
    InitSvgaMode = (unsigned long)InitModeVesa;
    NewBank = (unsigned long)NewBankVesa;
    
    /* Allocate logical video buffer */
    if (Log == 0) {
        Log = (unsigned long)malloc(Screen_X * Screen_Y);
        if (Log) {
            memset((void*)Log, 0, Screen_X * Screen_Y);
        }
    }
    
    MemoLog = Log;
    
    /* Build line offset table */
    scanline = Screen_X; /* 8-bit indexed color: 1 byte per pixel */
    for (i = 0; i < 480; i++) {
        TabOffLine[i] = i * scanline;
    }
    
    /* Reset clipping to full screen */
    ClipXmin = 0;
    ClipYmin = 0;
    ClipXmax = Screen_X - 1;
    ClipYmax = Screen_Y - 1;
}

/* Initialize MCGA mode (320x200x256) - Windows version */
void InitMcga(void)
{
    /* For Windows, treat MCGA same as SVGA but with smaller resolution */
    Screen_X = 320;
    Screen_Y = 200;
    InitSvga();
}

/* Simplified SVGA initialization */
void SimpleInitSvga(void)
{
    InitSvga();
}

/* Initialize MCGA mode (function pointer version) */
void InitMcgaMode(void)
{
    InitMcga();
}

/* Clear video mode and free resources */
void ClearVideo(void)
{
    if (Log) {
        free((void*)Log);
        Log = 0;
    }
    MemoLog = 0;
}

/*-----------------------------------------------------------------------------
 * Clipping Functions
 *---------------------------------------------------------------------------*/

/* Saved clipping values */
static unsigned long MemoClipXmin = 0;
static unsigned long MemoClipYmin = 0;
static unsigned long MemoClipXmax = 639;
static unsigned long MemoClipYmax = 479;

/* Set clipping rectangle */
void SetClip(long xmin, long ymin, long xmax, long ymax)
{
    ClipXmin = xmin;
    ClipYmin = ymin;
    ClipXmax = xmax;
    ClipYmax = ymax;
}

/* Restore clipping to full screen */
void UnSetClip(void)
{
    ClipXmin = 0;
    ClipYmin = 0;
    ClipXmax = Screen_X - 1;
    ClipYmax = Screen_Y - 1;
}

/* Save current clipping rectangle */
void MemoClip(void)
{
    MemoClipXmin = ClipXmin;
    MemoClipYmin = ClipYmin;
    MemoClipXmax = ClipXmax;
    MemoClipYmax = ClipYmax;
}

/* Restore saved clipping rectangle */
void RestoreClip(void)
{
    ClipXmin = MemoClipXmin;
    ClipYmin = MemoClipYmin;
    ClipXmax = MemoClipXmax;
    ClipYmax = MemoClipYmax;
}
