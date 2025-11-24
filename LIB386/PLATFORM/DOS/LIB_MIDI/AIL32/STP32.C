//лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
//лл                                                                        лл
//лл  STP32.C                                                               лл
//лл                                                                        лл
//лл  Creative Voice File (.VOC) stereo performance utility with dual-      лл
//лл  buffer playback                                                       лл
//лл                                                                        лл
//лл  V1.00 of 15-Nov-92: Derived from STP16.C v1.00                        лл
//лл  V1.01 of  1-May-93: Zortech C++ v3.1 compatibility added              лл
//лл  V1.02 of 18-Nov-93: MetaWare / Phar Lap compatibility added           лл
//лл                                                                        лл
//лл  Project: IBM Audio Interface Library for 32-bit DPMI                  лл
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

#undef min
#define min(a,b) ((a) < (b) ? (a) : (b))

/*****************************************************************/
#ifdef __HIGHC__

void fcopy(FARPTR fp, ULONG len, FILE *vfile)
{
   ULONG i;
   UBYTE *filebuf;

   filebuf = malloc(len);

   fread(filebuf,len,1,vfile);

   for (i=0;i<len;i++)
      {
      ((_Far UBYTE *) fp.ptr)[i] = filebuf[i];
      }

   free(filebuf);
}

#endif

/*****************************************************************/
void main(int argc, char *argv[])
{
   FILE *vfile;
   HDRIVER hdriver;
   char *drvr;
   char *dll;
   drvr_desc *desc;
   sound_buff firstblock,tempblock;
   int i,ch,done,bufnum,paused;
   unsigned seg1;
   unsigned seg2;
   union REGS inregs,outregs;

#ifdef __HIGHC__
   FARPTR buf1,buf2;
#else
   char *buf1,*buf2;
#endif

   setbuf(stdout,NULL);

   printf("\nSTP32 version %s                  Copyright (C) 1991, 1992 Miles Design, Inc.\n",VERSION);
   printf("-------------------------------------------------------------------------------\n\n");

   if (argc != 3)
      {
      printf("This program plays a raw 8-bit stereo sample at 22 kHz\n");
      printf("through any Audio Interface Library digital driver.\n\n");
      printf("Usage: STP32 ST_filename driver_filename\n");
      exit(1);
      }

   //
   // Allocate two 16K buffers from real-mode (lower 1MB) memory
   //
   // *buf1, *buf2 -> protected-mode pointers to buffers (sel:0000)
   // *seg1, *seg2 -> real-mode (physical) pointers to buffers (seg:0000)
   //
   // Note: DPMI calculations assume flat model near pointer offset 0 = 
   // segment 0, offset 0 (Rational DOS4GW).  The reason -- our simple
   // file loader function can't use the far pointer formed by the selector
   // returned by the DPMI call.
   //
   // Under Flashtek (INT21), function 48H returns a near pointer which 
   // relies on 32-bit offset wraparound to reach low memory.
   //
   // Under MetaWare / Phar Lap, the data is copied from high (near) memory  
   // into the low-memory buffers, because low memory cannot be reached with
   // a near pointer under all conditions.
   //
   // Note that these examples do not implement out-of-memory error 
   // checking or page-locking for VMM environments
   //

#ifdef DPMI          // Rational Systems DOS/4GW (Watcom)

   inregs.x.eax = 0x100;
   inregs.x.ebx = (16384 / 16);
   int386(0x31,&inregs,&outregs);

   seg1 = outregs.x.eax << 16;
   buf1 = (char *) (outregs.x.eax * 16);

   inregs.x.eax = 0x100;
   inregs.x.ebx = (16384 / 16);
   int386(0x31,&inregs,&outregs);

   seg2 = outregs.x.eax << 16;
   buf2 = (char *) (outregs.x.eax * 16);

#else
#ifdef INT21         // Flashtek X32 / Phar Lap 386

#ifdef __ZTC__                      // Zortech C++

   inregs.e.eax = 0x4800;
   inregs.e.ebx = (16384 / 16);
   int86(0x21,&inregs,&outregs);

   seg1 = outregs.e.eax << 16;
   buf1 = (char *) outregs.e.ebx;

   inregs.e.eax = 0x4800;
   inregs.e.ebx = (16384 / 16);
   int86(0x21,&inregs,&outregs);

   seg2 = outregs.e.eax << 16;
   buf2 = (char *) outregs.e.ebx;

#else
#ifdef __HIGHC__                    // MetaWare C++ / Phar Lap 386

   inregs.w.eax = 0x25c0;
   inregs.w.ebx = 16384 / 16;
   int86(0x21,&inregs,&outregs);

   seg1 = outregs.w.eax << 16;
   buf1.part.seg = 0x34;
   buf1.part.off = ((outregs.w.eax & 0xffff) * 16);

   inregs.w.eax = 0x25c0;
   inregs.w.ebx = 16384 / 16;
   int86(0x21,&inregs,&outregs);

   seg2 = outregs.w.eax << 16;
   buf2.part.seg = 0x34;
   buf2.part.off = ((outregs.w.eax & 0xffff) * 16);

#else                               // Watcom C++ / FlashTek
   inregs.x.eax = 0x4800;
   inregs.x.ebx = (16384 / 16);
   int386(0x21,&inregs,&outregs);

   seg1 = outregs.x.eax << 16;
   buf1 = (char *) outregs.x.ebx;

   inregs.x.eax = 0x4800;
   inregs.x.ebx = (16384 / 16);
   int386(0x21,&inregs,&outregs);

   seg2 = outregs.x.eax << 16;
   buf2 = (char *) outregs.x.ebx;

#endif
#endif

#endif
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

   vfile = fopen(argv[1],"rb");
   if (vfile == NULL)
      {
      printf("Couldn't open %s.\n",argv[1]);
      AIL_shutdown(NULL);
      exit(1);
      }

   //
   // Play the block as a series of double-buffered 16K chunks
   //

   printf("U D to increase/decrease volume\n");
   printf("P R to pause/resume playback\n");
   printf("< > to pan left, right\n");
   printf("ESC to stop playback\n\n");

   //
   // Copy sample rate and packing type to working sound buffer 
   // structure
   //
   // Sound Blaster standard sample rate: 256-(1000000-freq in Hz.)
   //

   firstblock.sample_rate = 234;     // 22.050 kHz (stereo 44.1 kHz)
   firstblock.pack_type = 0 | 0x80;  // 8-bit stereo sample
   firstblock.len = filelength(fileno(vfile));
   bufnum = 0;

   tempblock = firstblock;

   //
   // Sample application main loop ...
   //

   done = paused = 0;
   do
      {

      //
      // (Application-specific events here)
      //

      if (kbhit())                                 
         {
         ch = getch();

         switch (toupper(ch))
            {
            case 'P':
               AIL_pause_digital_playback(hdriver);
               paused = 1;
               break;

            case 'R':
               AIL_resume_digital_playback(hdriver);
               paused = 0;
               break;

            case 'U':
               i = AIL_digital_playback_volume(hdriver);
               i+=10;
               if (i>127) i=127;
               AIL_set_digital_playback_volume(hdriver,i);
               break;

            case 'D':
               i = AIL_digital_playback_volume(hdriver);
               i-=10;
               if (i<0) i=0;
               AIL_set_digital_playback_volume(hdriver,i);
               break;

            case ',':
            case '<':
               i = AIL_digital_playback_panpot(hdriver);
               i+=10;
               if (i>127) i=127;
               AIL_set_digital_playback_panpot(hdriver,i);
               break;

            case '.':
            case '>':
               i = AIL_digital_playback_panpot(hdriver);
               i-=10;
               if (i<0) i=0;
               AIL_set_digital_playback_panpot(hdriver,i);
               break;

            case 27:
               done=1;
               break;
            }
         }


      //
      // Update sound DMA buffers and ensure sound output is active
      //                          

      if (!paused)
         {
         for (i=0;i<2;i++)
            if ((AIL_sound_buffer_status(hdriver,i) == DAC_DONE)
               && firstblock.len)
               {
               tempblock.len = min(16384L,firstblock.len);
               firstblock.len -= tempblock.len;
               if (!(bufnum ^= 1))
                  {
#ifdef __HIGHC__
                  fcopy(buf1,(unsigned) tempblock.len,vfile);
#else
                  fread(buf1,(unsigned) tempblock.len,1,vfile);
#endif

                  tempblock.sel_data = buf1;
                  tempblock.seg_data = seg1;
                  }
               else
                  {
#ifdef __HIGHC__
                  fcopy(buf2,(unsigned) tempblock.len,vfile);
#else
                  fread(buf2,(unsigned) tempblock.len,1,vfile);
#endif
                  tempblock.sel_data = buf2;
                  tempblock.seg_data = seg2;
                  }

               AIL_register_sound_buffer(hdriver,i,&tempblock);
               AIL_format_sound_buffer(hdriver,&tempblock);
               printf(".");
               }
         AIL_start_digital_playback(hdriver);
         }

      //
      // Playback ends when no bytes are left in the source data and
      // the status of both buffers equals DAC_DONE
      //

      if (!firstblock.len)
         if ((AIL_sound_buffer_status(hdriver,0) == DAC_DONE)
            && (AIL_sound_buffer_status(hdriver,1) == DAC_DONE))
               done = 1;
      }
   while (!done);

   while (kbhit()) getch();

   fclose(vfile);
   printf("\n\nST32 stopped.\n");
   AIL_shutdown(NULL);
}

