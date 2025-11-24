//лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
//лл                                                                        лл
//лл  VP32.C                                                                лл
//лл                                                                        лл
//лл  Creative Voice File (.VOC) performance utility                        лл
//лл                                                                        лл
//лл  V1.00 of 15-Nov-92: Derived from VP16.C v1.00                         лл
//лл  V1.01 of  1-May-93: Zortech C++ v3.1 compatibility added              лл
//лл  V1.02 of 18-Nov-93: MetaWare / Phar Lap compatibility added           лл
//лл                                                                        лл
//лл  Project: IBM Audio Interface Library for 32-bit DOS                   лл
//лл   Author: John Miles                                                   лл
//лл                                                                        лл
//лл  C source compatible with Watcom C386 v9.0 or later                    лл
//лл                           Zortech C++ v3.1 or later                    лл
//лл                           MetaWare High C/C++ v3.1 or later            лл
//лл                                                                        лл
//лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
//лл                                                                        лл
//лл  Copyright (C) 1991-1993 Miles Design, Inc.                            лл
//лл                                                                        лл
//лл  Miles Design, Inc.                                                    лл
//лл  6702 Cat Creek Trail                                                  лл
//лл  Austin, TX 78731                                                      лл
//лл  (512) 345-2642 / FAX (512) 338-9630 / BBS (512) 454-9990              лл
//лл                                                                        лл
//лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <malloc.h>
#include <io.h>
#include <conio.h>
#include <ctype.h>

#include "ail32.h"
#include "dll.h"

const char VERSION[] = "1.02";

/*****************************************************************/
void main(int argc, char *argv[])
{
   HDRIVER hdriver;
   char *drvr;
   char *dll;
   drvr_desc *desc;
   union REGS inregs,outregs;
   unsigned VOCseg;
   char *VOCbuf;

#ifdef __HIGHC__
   unsigned char *buffer;
   FARPTR VOCfar;
   ULONG i;
#endif

   setbuf(stdout,NULL);

   printf("\nVP32 version %s                   Copyright (C) 1991, 1992 Miles Design, Inc.\n",VERSION);
   printf("-------------------------------------------------------------------------------\n\n");

   if (argc != 3)
      {
      printf("This program plays a Creative Voice File (.VOC) through any Audio Interface\n");
      printf("Library digital sound driver.\n\n");
      printf("Usage: VP32 filename.voc driver.dll\n");
      exit(1);
      }

   //
   // Allocate a buffer for the .VOC file in real-mode (lower 1MB) memory
   //

#ifdef DPMI          // Rational Systems DOS/4GW

   inregs.x.eax = 0x100;
   inregs.x.ebx = ((FILE_size(argv[1])+16) / 16);

   int386(0x31,&inregs,&outregs);

   if (outregs.x.cflag)
      {
      printf("Insufficient memory to load %s.\n",argv[1]);
      AIL_shutdown(NULL);
      exit(1);
      }

   VOCseg = outregs.x.eax << 16;
   VOCbuf = (char *) (outregs.x.eax * 16);

#else
#ifdef INT21         // Flashtek X32 / Phar Lap 386

#ifdef __ZTC__                      // Zortech C++
   inregs.e.eax = 0x4800;
   inregs.e.ebx = ((FILE_size(argv[1])+16) / 16);

   int86(0x21,&inregs,&outregs);

   if (outregs.e.cflag)
      {
      printf("Insufficient memory to load %s.\n",argv[1]);
      AIL_shutdown(NULL);
      exit(1);
      }

   VOCseg = outregs.e.eax << 16;
   VOCbuf = (char *) outregs.e.ebx;

#else
#ifdef __HIGHC__                    // MetaWare C++
   inregs.w.eax = 0x25c0;
   inregs.w.ebx = ((FILE_size(argv[1])+16) / 16);

   int86(0x21,&inregs,&outregs);

   if (outregs.w.cflag)
      {
      printf("Insufficient memory to load %s.\n",argv[1]);
      AIL_shutdown(NULL);
      exit(1);
      }

   VOCseg = outregs.w.eax << 16;
   VOCbuf = (char *) ((outregs.w.eax & 0xffff) * 16);

#else                               // Watcom C++
   inregs.x.eax = 0x4800;
   inregs.x.ebx = ((FILE_size(argv[1])+16) / 16);

   int386(0x21,&inregs,&outregs);

   if (outregs.x.cflag)
      {
      printf("Insufficient memory to load %s.\n",argv[1]);
      AIL_shutdown(NULL);
      exit(1);
      }

   VOCseg = outregs.x.eax << 16;
   VOCbuf = (char *) outregs.x.ebx;
#endif
#endif
#endif
#endif

   //
   // Read the entire .VOC file into memory
   //
   // Under MetaWare/Phar Lap, we will buffer it down into lower (far) memory
   // since FILE_read() requires a near pointer
   //

#ifdef __HIGHC__

   buffer = FILE_read(argv[1],NULL);

   if (buffer == NULL)
      {
      printf("Couldn't load %s.\n",argv[1]);
      AIL_shutdown(NULL);
      exit(1);
      }

   VOCfar.part.seg = 0x34;
   VOCfar.part.off = (ULONG) VOCbuf;
   
   for (i=FILE_size(argv[1]);i;i--)
      {
      ((_Far UBYTE *) VOCfar.ptr)[i-1] = buffer[i-1];
      }

   free(buffer);

#else

   VOCbuf = FILE_read(argv[1],VOCbuf);

   if (VOCbuf == NULL)
      {
      printf("Couldn't load %s.\n",argv[1]);
      AIL_shutdown(NULL);
      exit(1);
      }

#endif

   //
   // Load driver file
   //

   dll = FILE_read(argv[2],NULL);
   if (dll==NULL)
      {
      printf("Could not load driver '%s'.\n",argv[2]);
      exit(1);
      }

   drvr=DLL_load(dll,DLLMEM_ALLOC | DLLSRC_MEM,NULL);
   if (drvr==NULL)     
      {
      printf("Invalid DLL image.\n");
      exit(1);
      }

   free(dll);

   //
   // Initialize API before calling any Library functions
   //

   AIL_startup();

   hdriver = AIL_register_driver(drvr);
   if (hdriver==-1)
      {
      printf("Driver %s not compatible with linked API version.\n",
         argv[2]);
      AIL_shutdown(NULL);
      exit(1);
      }

   //
   // Get driver type and factory default I/O parameters; exit if
   // driver is not capable of interpreting PCM sound data
   //

   desc = AIL_describe_driver(hdriver);

   if (desc->drvr_type != DSP_DRVR)
      {
      printf("%s is not a digital sound driver.\n",argv[2]);
      AIL_shutdown(NULL);
      exit(1);
      }

   if (!AIL_detect_device(hdriver,desc->default_IO,desc->default_IRQ,
      desc->default_DMA,desc->default_DRQ))
         {
         printf("Sound hardware not found.\n");
         AIL_shutdown(NULL);
         exit(1);
         }

   AIL_init_driver(hdriver,desc->default_IO,desc->default_IRQ,
      desc->default_DMA,desc->default_DRQ);

   //
   // Format .VOC file data and begin playback
   //

#ifdef __HIGHC__
   AIL_format_VOC_file(hdriver,VOCfar.ptr,-1);

   AIL_play_VOC_file(hdriver,VOCfar.ptr,VOCseg,-1);
#else
   AIL_format_VOC_file(hdriver,(void far *) VOCbuf,-1);

   AIL_play_VOC_file(hdriver,(void far *) VOCbuf,VOCseg,-1);
#endif

   AIL_start_digital_playback(hdriver);

#ifdef __HIGHC__

   printf("Press any key to stop ... \n");

   while (AIL_VOC_playback_status(hdriver) != DAC_DONE)
      {
      if (kbhit())
         {
         getch();
         break;
         }
      }

#else
   printf("Launching DOS shell.  Type 'EXIT' to stop playback and ");
   printf("exit VP32.");

   system("command");
#endif

   printf("VP32 stopped.\n");
   AIL_shutdown(NULL);
}

