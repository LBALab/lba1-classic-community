/*
 * Video Abstraction Layer - Windows GDI/DirectDraw Implementation
 * Little Big Adventure (LBA) Classic Community Edition
 */

/* Include Windows headers first to avoid type conflicts */
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static HWND g_hWnd = NULL;
static HDC g_hDC = NULL;
static unsigned char *g_screen_buffer = NULL;
static unsigned short g_screen_width = 0;
static unsigned short g_screen_height = 0;
static unsigned short g_scanline = 0;

/*-----------------------------------------------------------------------------
 * Video Mode Interface - Windows Implementation
 * Initialize VESA/SVGA mode (Windows GDI/DirectDraw)
 *---------------------------------------------------------------------------*/

void InitModeVesa(void)
{
	/* TODO: Initialize Windows GDI or DirectDraw */
	/* For now, this is a stub */
	
	/* Set default resolution */
	g_screen_width = 640;
	g_screen_height = 480;
	g_scanline = 640; /* 8-bit indexed color, 1 byte per pixel */
	
	/* Allocate screen buffer */
	g_screen_buffer = (unsigned char*)malloc(640 * 480);
	if (g_screen_buffer) {
		memset(g_screen_buffer, 0, 640 * 480);
	}
}

void NewBankVesa(unsigned short bank)
{
	/* Windows uses linear framebuffer - banking not needed */
	(void)bank;
}
