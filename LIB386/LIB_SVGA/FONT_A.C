
#include "lib_sys/adeline.h"
#include "lib_svga/lib_svga.h"

static void *PtrFont = 0;
static LONG DxFont = 0;
static LONG InterLeave = 1;
static LONG InterSpace = 10;

void CoulFont(LONG coul) {
    CoulMask((UBYTE)coul);
}

void SetFont(void *ptfont, LONG inle, LONG insp) {
    PtrFont = ptfont;
    InterLeave = inle;
    InterSpace = insp;
}

LONG SizeFont(void *chaine) {
    UBYTE *str;
    LONG *font_offsets;
    UBYTE *glyph_data;
    UBYTE c;
    LONG width;

    if (PtrFont == 0) {
        return 0;
    }

    str = (UBYTE *)chaine;
    font_offsets = (LONG *)PtrFont;
    width = 0;

    while ((c = *str++) != 0) {
        if (c == 32) {
            width += InterSpace;
        } else {
            glyph_data = (UBYTE *)PtrFont + font_offsets[c];
            width += InterLeave + glyph_data[0];
        }
    }

    return width;
}

LONG CarFont(LONG xcar, LONG ycar, LONG car) {
    LONG *font_offsets;
    UBYTE *glyph_data;
    LONG width;

    if (car == 32) {
        return InterSpace;
    }

    if (PtrFont == 0) {
        return 0;
    }

    font_offsets = (LONG *)PtrFont;
    glyph_data = (UBYTE *)PtrFont + font_offsets[car];
    width = glyph_data[0];
    
    AffMask(car, xcar, ycar, PtrFont);
    
    return width + InterLeave;
}

void Font(LONG xfont, LONG yfont, void *chaine) {
    UBYTE *str;
    LONG *font_offsets;
    UBYTE *glyph_data;
    UBYTE c;
    LONG x;
    LONG y;

    if (PtrFont == 0) {
        return;
    }

    str = (UBYTE *)chaine;
    font_offsets = (LONG *)PtrFont;
    x = xfont;
    y = yfont;

    while ((c = *str++) != 0) {
        if (c == 32) {
            x += InterSpace;
        } else {
            glyph_data = (UBYTE *)PtrFont + font_offsets[c];
            DxFont = glyph_data[0];
            
            AffMask(c, x, y, PtrFont);
            
            x += InterLeave + DxFont;
        }
    }
}
