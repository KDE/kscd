/*
   $Id$

   Synaesthesia - program to display sound graphically
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
  51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.

  The author may be contacted at:
    phar6@student.monash.edu.au
  or
    27 Bond St., Mt. Waverley, 3149, Melbourne, Australia
*/

#ifndef __SYNA_H__
#define __SYNA_H__


#if defined(__linux__) || defined(__svr4__) || defined(__osf__)

void error(const char *str, bool syscall=false); //Display error and exit
void warning(const char *str, bool syscall=false); //Display error

//void error(char *str,bool syscall=false);
inline void attempt(int x, const char *y, bool syscall=false) { if (x == -1) error(y,syscall); }  

//void warning(char *str,bool syscall=false);
inline void attemptNoDie(int x, const char *y, bool syscall=false) { if (x == -1) warning(y,syscall); } 

#include "polygon.h"
#include "magicconf.h"

/***************************************/


#define PROGNAME "kscdmagic"


#ifdef __FreeBSD__
 

typedef unsigned short sampleType;

#else

typedef short sampleType;

#ifndef __linux__

#warning This target has not been tested!

#endif
#endif

#ifdef __osf__
#include <machine/endian.h>
#else
#include <endian.h>
#endif
#if BYTE_ORDER == BIG_ENDIAN
#define BIGENDIAN
#else
#define LITTLEENDIAN
#endif


#define n (1<<m)
#define recSize (1<<m-overlap)

/* core */
extern volatile sampleType *data;
//extern unsigned char *output, *lastOutput, *lastLastOutput;
extern Bitmap<unsigned short> outputBmp, lastOutputBmp, lastLastOutputBmp;
#define ucoutput ((unsigned char*)outputBmp.data)
#define lastOutput ((unsigned char*)lastOutputBmp.data)
#define lastLastOutput ((unsigned char*)lastLastOutputBmp.data)

inline unsigned short combiner(unsigned short a,unsigned short b) {
  //Not that i want to give the compiler a hint or anything...
  unsigned char ah = a>>8, al = a&255, bh = b>>8, bl = b&255;
  if (ah < 64) ah *= 4; else ah = 255;
  if (al < 64) al *= 4; else al = 255;
  if (bh > ah) ah = bh;
  if (bl > al) al = bl;
  return ah*256+al;
}

extern PolygonEngine<unsigned short,combiner,2> polygonEngine;

extern int outWidth, outHeight;

void allocOutput(int w,int h);

void coreInit();
void setStarSize(double size);
int coreGo();
void fade();

/* *wrap */
void screenInit(int xHint,int yHint,int widthHint,int heightHint);
void screenSetPalette(unsigned char *palette);
void screenEnd(void);
void screenShow(void);
int sizeUpdate(void);

void inputUpdate(int &mouseX,int &mouseY,int &mouseButtons,char &keyHit);

/* ui */
void interfaceInit();
void interfaceSyncToState();
void interfaceEnd();
bool interfaceGo();

enum SymbolID {
  Speaker, Bulb,
  Play, Pause, Stop, SkipFwd, SkipBack,
  Handle, Pointer, Open, NoCD, Exit,
  Zero, One, Two, Three, Four,
  Five, Six, Seven, Eight, Nine,
  Slider, Selector, Plug, Loop, Box, Bar,
  Flame, Wave, Stars, Star, Diamond, Size, FgColor, BgColor,
  Save, Reset, TrackSelect,
  NotASymbol
};

/* State information */

extern SymbolID state;
extern int track, frames;
extern double trackProgress;
extern char **playList;
extern int playListLength, playListPosition;
extern SymbolID fadeMode;
extern bool pointsAreDiamonds;
extern double brightnessTwiddler; 
extern double starSize; 
extern double fgRedSlider, fgGreenSlider, bgRedSlider, bgGreenSlider;

extern double volume; 

void setStateToDefaults();
void saveConfig();

void putString(char *string,int x,int y,int red,int blue);

/* sound */
enum SoundSource { SourceLine, SourceCD, SourcePipe };

void cdOpen(char *cdromName);
void cdClose(void);
void cdGetStatus(int &track, int &frames, SymbolID &state);
void cdPlay(int trackFrame, int endFrame=-1); 
void cdStop(void);
void cdPause(void);
void cdResume(void);
void cdEject(void);
void cdCloseTray(void);
int cdGetTrackCount(void);
int cdGetTrackFrame(int track);
void openSound(SoundSource sound, int downFactor, const char *dspName, char *mixerName);
void closeSound();
void setupMixer(double &loudness);
void setVolume(double loudness);
int getNextFragment(void);


#endif // linux || svr4
#endif // __SYNA_H__
