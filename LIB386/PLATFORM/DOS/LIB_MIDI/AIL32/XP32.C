//лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
//лл                                                                        лл
//лл  XP32.C                                                                лл
//лл                                                                        лл
//лл  Standard XMIDI file performance utility                               лл
//лл                                                                        лл
//лл  V1.00 of  5-Aug-92: 32-bit conversion of XPLAY.C (John Lemberger)     лл
//лл  V1.01 of  1-May-93: Zortech C++ v3.1 compatibility added              лл
//лл                                                                        лл
//лл  Project: IBM Audio Interface Library for 32-bit DPMI (AIL/32)         лл
//лл   Author: John Miles                                                   лл
//лл                                                                        лл
//лл  C source compatible with Watcom C386 v9.0 or later                    лл
//лл  C source compatible with Zortech C++ v3.1 or later                    лл
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
#include <string.h>
#include <conio.h>

#include "ail32.h"      // Audio Interface Library API function header
#include "dll.h"

const char VERSION[] = "1.01";

/***************************************************************/

//
// Standard C routine for Global Timbre Library access
//

void *load_global_timbre(FILE *GTL, unsigned short bank, unsigned short patch)
{
   unsigned short *timb_ptr;
   static unsigned short len;

   static struct                  // GTL file header entry structure
   {
      signed char patch;
      signed char bank;
      unsigned long offset;
   }
   GTL_hdr;

   if (GTL==NULL) return NULL;    // if no GTL, return failure

   rewind(GTL);                   // else rewind to GTL header

   do                             // search file for requested timbre
      {
      fread(&GTL_hdr,sizeof(GTL_hdr),1,GTL);

      if (GTL_hdr.bank == -1) 
         return NULL;             // timbre not found, return NULL
      }
   while ((GTL_hdr.bank != bank) ||
          (GTL_hdr.patch != patch));       

   fseek(GTL,GTL_hdr.offset,SEEK_SET);    
   fread(&len,2,1,GTL);           // timbre found, read its length

   timb_ptr = malloc(len);        // allocate memory for timbre ..
   *timb_ptr = len;
                                  // and load it
   fread((timb_ptr+1),len-2,1,GTL);       
                           
   if (ferror(GTL))               // return NULL if any errors
      return NULL;                // occurred
   else
      return timb_ptr;            // else return pointer to timbre
}
   
/***************************************************************/
void main(int argc, char *argv[])
{
   HDRIVER hdriver;
   HSEQUENCE hseq;
   drvr_desc *desc;
   FILE *GTL;
   char GTL_filename[32];
   char *state;
   char *drvr,*dll;
   char *timb;
   char *tc_addr;
   unsigned char *buffer;
   unsigned long state_size;
   unsigned short bank,patch,tc_size,seqnum;
   unsigned short treq;

   setbuf(stdout,NULL);

   printf("\nXP32 version %s                   Copyright (C) 1991, 1992 Miles Design, Inc.\n",VERSION);
   printf("-------------------------------------------------------------------------------\n\n");

   if (argc < 3)
      {
      printf("This program plays an Extended MIDI (XMIDI) sequence through a \n");
      printf("specified AIL/32 sound driver.\n\n");
      printf("Usage: XP32 XMIDI_filename driver_filename [sequence_number]\n");
      exit(1);
      }

   seqnum = 0;
   if (argc == 4) seqnum = atoi(argv[3]);

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

   //
   // Register the driver with the API
   //

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
   // driver is not capable of interpreting MIDI files
   //

   desc = AIL_describe_driver(hdriver);

   if (desc->drvr_type != XMIDI_DRVR)
      {
      printf("Driver %s not an XMIDI driver.\n",argv[2]);
      AIL_shutdown(NULL);
      exit(1);
      }

   //
   // Verify presence of driver's sound hardware and prepare 
   // driver/hardware for use
   //

   if (!AIL_detect_device(hdriver,desc->default_IO,desc->default_IRQ,
      desc->default_DMA,desc->default_DRQ))
         {
         printf("Sound hardware not found.\n");
         AIL_shutdown(NULL);
         exit(1);
         }


   AIL_init_driver(hdriver,desc->default_IO,desc->default_IRQ,
      desc->default_DMA,desc->default_DRQ);

   state_size = AIL_state_table_size(hdriver);

   //
   // Load XMIDI data file
   //

   buffer = FILE_read(argv[1],NULL);
   if (buffer == NULL)
      {
      printf("Can't load XMIDI file %s.\n",argv[1]);
      AIL_shutdown(NULL);
      exit(1);
      }

   //
   // Get name of Global Timbre Library file by appending suffix 
   // supplied by XMIDI driver to GTL filename prefix "SAMPLE."
   //

   strcpy(GTL_filename,"SAMPLE.");
   strcat(GTL_filename,desc->data_suffix);

   //
   // Set up local timbre cache; open Global Timbre Library file
   //

   tc_size = AIL_default_timbre_cache_size(hdriver);

   if (tc_size)
      {
      tc_addr = malloc((unsigned long) tc_size);
      AIL_define_timbre_cache(hdriver,tc_addr,tc_size);
      }

   GTL = fopen(GTL_filename,"rb");

   //
   // Look up and register desired sequence in XMIDI file, loading
   // timbres if needed
   //

   state = malloc(state_size);
   if ((hseq = AIL_register_sequence(hdriver,buffer,seqnum,state,
      NULL)) == -1)
      {
      printf("Sequence %u not present in XMIDI file \"%s\".\n",
         seqnum,argv[1]);
      AIL_shutdown(NULL);
      exit(1);
      }

   while ((treq=AIL_timbre_request(hdriver,hseq)) != 0xffff)
      {
      bank = treq / 256; patch = treq % 256;
 
      timb = load_global_timbre(GTL,bank,patch);

      if (timb != NULL)
         {
         AIL_install_timbre(hdriver,bank,patch,timb);
         free(timb);
         printf("Installed timbre bank %u, patch %u\n",bank,patch);
         }
      else
         {
         printf("Timbre bank %u, patch %u not found ",bank,patch);
         printf("in Global Timbre Library %s\n",GTL_filename);
         AIL_shutdown(NULL);
         exit(1);
         }
      }

   if (GTL != NULL) fclose(GTL);         

   //
   // Start music playback
   //

   printf("Playing sequence %u from XMIDI file \"%s\" ...\n\n",
      seqnum,argv[1]);

   AIL_start_sequence(hdriver,hseq);

#ifdef __HIGHC__

   printf("Press any key to stop ... \n");

   while (AIL_sequence_status(hdriver,hseq) != SEQ_DONE)
      {
      if (kbhit())
         {
         getch();
         break;
         }
      }

#else
   printf("Launching DOS shell.  Type 'EXIT' to stop playback ");
   printf("and exit XP32.");

   system("command");
#endif

   //
   // Shut down API and all installed drivers; write XMIDI filename 
   // to any front-panel displays
   //

   printf("XP32 stopped.\n");

   AIL_shutdown(argv[1]);
}
