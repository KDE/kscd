/* Synaesthesia - program to display sound graphically
   Copyright (C) 1997  Paul Francis Harrison

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  675 Mass Ave, Cambridge, MA 02139, USA.

  The author may be contacted at:
    pfh@yoyo.cc.monash.edu.au
  or
    27 Bond St., Mt. Waverley, 3149, Melbourne, Australia
*/

#if defined(__linux__) || defined (__svr4__) || defined(__osf__)

#include <string.h>

extern "C" {
#include "xlib.h"
}

#include "magicconf.h"
#include "syna.h"

static xlibparam xparams = { 0, 0, 0 };
static xdisplay *d;

static bool lowColor, paletteInstalled;
static unsigned char mapping[64];

int setPalette(int ,int r,int g,int b) {
  return xalloc_color(d,r*257,g*257,b*257,0);
}

void screenInit(int xHint,int yHint,int widthHint,int heightHint) 
{

  int i;

  d = xalloc_display("KSCD Magic",xHint,yHint,widthHint,heightHint,&xparams);

  if (d == 0) 
    error("setting up a window");

  if (!alloc_image(d)) 
    error("allocating window buffer");
  outWidth = widthHint;
  outHeight = heightHint;

  //XMoveWindow(d->display,d->window,xHint,yHint);

#define BOUND(x) ((x) > 255 ? 255 : (x))
#define PEAKIFY(x) BOUND((x) - (x)*(255-(x))/255/2)

  lowColor = (d->depth <= 8);

  /* Set up palette immediately.
   * Original code has the following commented out.
   */
  if (!lowColor) { 
    for(i=0;i<256;i++)
      attempt(setPalette(i,PEAKIFY((i&15*16)),
                   PEAKIFY((i&15)*16+(i&15*16)/4),
                   PEAKIFY((i&15)*16)),
              " in X: could not allocate sufficient palette entries");
  } else {
    for(i=0;i<64;i++)
      attempt(mapping[i] = setPalette(i,PEAKIFY((i&7*8)*4),
                                        PEAKIFY((i&7)*32+(i&7*8)*2),
                                        PEAKIFY((i&7)*32)),
        " in X: could not allocate sufficient palette entries");
  }
}

void screenSetPalette(unsigned char *palette) {
  int i;

  if (paletteInstalled)
    xfree_colors(d);

  if (!lowColor) {
    for(i=0;i<256;i++)
      attempt(setPalette(i,palette[i*3],palette[i*3+1],palette[i*3+2]),
              " in X: could not allocate sufficient palette entries");
  } else {
    const int array[8] = {0,2,5,7,9,11,13,15};
    for(i=0;i<64;i++) {
      int p = (array[i&7]+array[i/8]*16) *3;
      attempt(mapping[i] = setPalette(i,palette[p],palette[p+1],palette[p+2]),
        " in X: could not allocate sufficient palette entries");
    }
  }
  
  paletteInstalled = true;
}

void screenEnd() {
  if (paletteInstalled)
    xfree_colors(d);
  xfree_display(d);
}

int sizeUpdate() 
{
  int newWidth,newHeight;

  if (xsize_update(d,&newWidth,&newHeight)) 
    {
      if (newWidth == outWidth && newHeight == outHeight)
	return 0;
      //delete[] output;
      //outWidth = newWidth;
      //outHeight = newHeight;
      //output = new unsigned char [outWidth*outHeight*2];
    //memset(output,32,outWidth*outHeight*2);
      allocOutput(newWidth,newHeight);
      return 1;
    }
  return 0;
}

void inputUpdate(int &mouseX,int &mouseY,int &mouseButtons,char &keyHit) {
  xmouse_update(d);
  mouseX = xmouse_x(d);
  mouseY = xmouse_y(d);
  mouseButtons = xmouse_buttons(d);
  keyHit = xkeyboard_query(d);
}
 
void screenShow(void) { 
  register unsigned long *ptr2 = (unsigned long*)ucoutput;
  unsigned long *ptr1 = (unsigned long*)d->back;
  int i = outWidth*outHeight/4;
  if (lowColor)
    do {
      register unsigned int const r1 = *(ptr2++);
      register unsigned int const r2 = *(ptr2++);
    
      //if (r1 || r2) {
#ifdef LITTLEENDIAN
        register unsigned int const v = 
             mapping[((r1&0xe0ul)>>5)|((r1&0xe000ul)>>10)]
            |mapping[((r1&0xe00000ul)>>21)|((r1&0xe0000000ul)>>26)]*256U; 
        *(ptr1++) = v | 
             mapping[((r2&0xe0ul)>>5)|((r2&0xe000ul)>>10)]*65536U
            |mapping[((r2&0xe00000ul)>>21)|((r2&0xe0000000ul)>>26)]*16777216U; 
#else
        register unsigned int const v = 
             mapping[((r2&0xe0ul)>>5)|((r2&0xe000ul)>>10)]
            |mapping[((r2&0xe00000ul)>>21)|((r2&0xe0000000ul)>>26)]*256U; 
        *(ptr1++) = v | 
             mapping[((r1&0xe0ul)>>5)|((r1&0xe000ul)>>10)]*65536U
            |mapping[((r1&0xe00000ul)>>21)|((r1&0xe0000000ul)>>26)]*16777216U; 
#endif
      //} else ptr1++;
    } while (--i); 
  else
    do {
      // Asger Alstrup Nielsen's (alstrup@diku.dk)
      // optimized 32 bit screen loop
      register unsigned int const r1 = *(ptr2++);
      register unsigned int const r2 = *(ptr2++);
    
      //if (r1 || r2) {
#ifdef LITTLEENDIAN
        register unsigned int const v = 
            ((r1 & 0x000000f0ul) >> 4)
          | ((r1 & 0x0000f000ul) >> 8)
          | ((r1 & 0x00f00000ul) >> 12)
          | ((r1 & 0xf0000000ul) >> 16);
        *(ptr1++) = v | 
            ((r2 & 0x000000f0ul) << 16 -4)
          | ((r2 & 0x0000f000ul) << 16 -8)
          | ((r2 & 0x00f00000ul) << 16 -12)
          | ((r2 & 0xf0000000ul) << 16 -16);
#else
        register unsigned int const v = 
            ((r2 & 0x000000f0ul) >> 4)
          | ((r2 & 0x0000f000ul) >> 8)
          | ((r2 & 0x00f00000ul) >> 12)
          | ((r2 & 0xf0000000ul) >> 16);
        *(ptr1++) = v | 
            ((r1 & 0x000000f0ul) << 16 -4)
          | ((r1 & 0x0000f000ul) << 16 -8)
          | ((r1 & 0x00f00000ul) << 16 -12)
          | ((r1 & 0xf0000000ul) << 16 -16);
#endif
      //} else ptr1++;
    } while (--i); 
  
  xflip_buffers(d);
  draw_screen(d);
  XFlush(d->display);
}



#endif // linux || svr4

