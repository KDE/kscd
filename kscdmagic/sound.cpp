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
  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

  The author may be contacted at:
    pfh@yoyo.cc.monash.edu.au
  or
    27 Bond St., Mt. Waverley, 3149, Melbourne, Australia
*/

#if defined(__linux__) || defined(__svr4__)

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#if defined (__linux__)
#include <linux/soundcard.h>
#ifndef __GNUC__
#define __GNUC__ 1
#endif
#undef __STRICT_ANSI__
#include <asm/types.h>
#include <linux/cdrom.h>
#endif

#if defined (__svr4__)
#include <sys/soundcard.h>
#endif


// who knows when we'll need that...
#if defined (FreeBSD)
#include <sys/soundcard.h>
#include <sys/cdio.h>
#define CDROM_LEADOUT 0xAA
#define CD_FRAMES 75 /* frames per second */
#define CDROM_DATA_TRACK 0x4
#endif

#include <time.h>

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "syna.h"
#include "magicconf.h"


/* Sound Recording ================================================= */

#ifdef LITTLEENDIAN
#define SOUNDFORMAT AFMT_S16_LE
#else
#define SOUNDFORMAT AFMT_S16_BE
#endif

//If kernel starts running out of sound memory playing mp3s, this could
//be the problem. OTOH if it is too small, it will start ticking on slow
//computers
#define MAXWINDOWSIZE 32

static SoundSource source;
static int inFrequency, downFactor, windowSize, pipeIn, device;
static short *dataIn;
static char *mixer;

void 
openSound(SoundSource source, int inFrequency, const char *dspName, 
               char *mixerName) 
{
  ::source = source;
  ::inFrequency = inFrequency; 
  ::windowSize = 1;
  mixer = mixerName;
  downFactor = inFrequency / frequency; 
  if (downFactor <= 0)
    downFactor = 1;

  int format, stereo, fragment, fqc;

#ifdef __FreeBSD__
  attempt(device = open(dspName,O_WRONLY),"opening dsp device",true);
  format = SOUNDFORMAT;
  attempt(ioctl(device,SNDCTL_DSP_SETFMT,&format),"setting format",true);
  if (format != SOUNDFORMAT) error("setting format (2)");
  close(device);
#endif
  if (source == SourcePipe)
    attempt(device = open(dspName,O_WRONLY),"opening dsp device",true);
  else
    attempt(device = open(dspName,O_RDONLY),"opening dsp device",true);
    
  //Probably not needed
  //attemptNoDie(ioctl(device,SNDCTL_DSP_RESET,0),"reseting dsp");
  format = SOUNDFORMAT;
  fqc = (source == SourcePipe ? inFrequency : frequency);
  stereo = 1;
  
  //int logWindowSize = -1, tmp = windowSize*downFactor;
  //while(tmp) {
  //  tmp /= 2;
  //  logWindowSize++;
  //}

  if (source == SourcePipe)
    //fragment = 0x00020000 + (m-overlap+1)+logWindowSize;
    fragment = 0x00010000*(MAXWINDOWSIZE+1) + (m-overlap+1);//+logWindowSize;
                                                 //Soundcard should read in windowSize 
                                                 // blocks of sound before blocking
  else
    //fragment = 0x00020000 + (m-overlap+1); //2 fragments of size 2*(2^(m-overlap+1)) bytes
  
  //Added extra fragments to allow recording overrun (9/7/98)
    fragment = 0x00080000 + (m-overlap+1); //8 fragments of size 2*(2^(m-overlap+1)) bytes
  
  
  
  
  //Was 0x00010000 + m; 

  attemptNoDie(ioctl(device,SNDCTL_DSP_SETFRAGMENT,&fragment),"setting fragment",true);
#ifndef __FreeBSD__
  attempt(ioctl(device,SNDCTL_DSP_SETFMT,&format),"setting format",true);
  if (format != SOUNDFORMAT) error("setting format (2)");
#endif
  attempt(ioctl(device,SNDCTL_DSP_STEREO,&stereo),"setting stereo",true);
  attemptNoDie(ioctl(device,SNDCTL_DSP_SPEED,&fqc),"setting frequency",true);
   
  data = new short[n*2];  

  if (source == SourcePipe) {
    dataIn = new short[n*2*downFactor*MAXWINDOWSIZE];
    memset(dataIn,0,n*4*downFactor*MAXWINDOWSIZE);
    pipeIn = dup(0);
    close(0);
  }
}

void closeSound() {
  delete data;
  if (source == SourcePipe) {
    delete dataIn;
    close(pipeIn);
  }
  close(device);
}

int readWholeBlock(int pipe,char *dest,int length) {
  while(length > 0) {
    int result = read(pipe,dest,length);
    if (result < 1)
      return -1;
    dest += result;
    length -= result;
  }
  return 0;
}

int getNextFragment(void) {
  if (source == SourcePipe) {
    static int lastTime = 0;
    int nowTime;
    timeval timeVal1, timeVal2;

    gettimeofday(&timeVal1,0);
    write(device, (char*)dataIn, n*4*downFactor*windowSize); 
    gettimeofday(&timeVal2,0);

    nowTime = timeVal1.tv_usec + timeVal1.tv_sec * 1000000;
    if (nowTime > lastTime) {
      int optimumFrags =
         int(double(nowTime-lastTime)*inFrequency/1000000.0/(n*downFactor))
	     +1;
      if (optimumFrags > MAXWINDOWSIZE)
        optimumFrags = MAXWINDOWSIZE;

      windowSize = optimumFrags;
    }
    
    lastTime = timeVal2.tv_usec + timeVal2.tv_sec * 1000000;
  
    if (readWholeBlock(pipeIn, ((char*)dataIn), n*4*downFactor*windowSize) == -1)
      return -1;
    
    int i,j;
    for(i=0,j=0;i<n;i++,j+=downFactor) 
      ((long*)data)[i] = ((long*)dataIn)[j]; 
  } else {
    int i;
    count_info info;
    if (-1 == ioctl(device,SNDCTL_DSP_GETIPTR,&info))
      info.blocks = 1;
    if (info.blocks > 8 || info.blocks < 1) /* Sanity check */
      info.blocks = 1;

    for(i=0;i<info.blocks;i++) {
      if (recSize != n)
        memmove((char*)data,(char*)data+recSize*4,(n-recSize)*4);

      attemptNoDie(
        readWholeBlock(device,(char*)data+n*4-recSize*4, recSize*4),
	"reading from soundcard", true);
    }
  }
  return 0;
}

#else

// generic dummy implementation

#include "syna.h"

int getNextFragment(void) {
    return 0;
}

void openSound(SoundSource source, int inFrequency, const char *dspName,
               char *mixerName)
{
}

void closeSound()
{
}

#endif // linux || svr4

