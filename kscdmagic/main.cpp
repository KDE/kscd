/*

  $Id$

  kscdmagic 2.0   Dirk Försterling <milliByte@gmx.de>

  based on:

  kscdmagic 1.0   Bernd Johannes Wuebben <wuebben@kde.org>

  based on:
 
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
  675 Mass Ave, Cambridge, MA 02139, USA.

  The author may be contacted at:
  pfh@yoyo.cc.monash.edu.au
  27 Bond St., Mt. Waverley, 3149, Melbourne, Australia

*/

#include <stdlib.h>
#include <stdio.h>

#if defined(__linux__) || defined(__svr4__)

#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <pwd.h>
#include <string.h>
#if defined(__linux__)
#include <getopt.h>
#endif

#include "logo.h"
#include "magicconf.h"
#include "syna.h"

volatile short *data;

int outWidth, outHeight;
double brightnessTwiddler;
SymbolID fadeMode = Stars;
double starSize = 0.125;
bool pointsAreDiamonds = true;


const int numRows = 4;
const int rowHeight = 50;
const int leftColWidth = 40;
const int rowMaxWidth = 310;
const int sliderBorder = 20;
const int sliderWidth = rowMaxWidth - leftColWidth - sliderBorder*2;
const int numberSpacing = 15;
const int uiWidth = 330;
const int uiHeight = 135;


static int isExpanded = 0;
static double bright = 1.0;

Bitmap<unsigned short> outputBmp, lastOutputBmp, lastLastOutputBmp;
PolygonEngine<unsigned short,combiner,2> polygonEngine;

void 
allocOutput(int w,int h) 
{
  outputBmp.size(w,h);
  lastOutputBmp.size(w,h);
  lastLastOutputBmp.size(w,h);
  polygonEngine.size(w,h);
  outWidth = w;
  outHeight = h;
} // allocOutput()

void 
setBrightness(double bright) 
{
  brightnessTwiddler = bright;
}  // setBrightness()



static void 
cleanup( int sig )
{
  (void) sig;
  closeSound();
  exit(0);
} // cleanup()


// make sure the pid file is cleaned up when exiting unexpectedly.

void 
catchSignals()
{
	signal(SIGHUP, cleanup);		/* Hangup */
	signal(SIGINT, cleanup);		/* Interrupt */
	signal(SIGTERM, cleanup);		/* Terminate */
	signal(SIGPIPE, cleanup);
	signal(SIGQUIT, cleanup);
} // catchSignals()

void 
usage(char*)
{
  fprintf(stderr, "Valid command line options:\n");
  fprintf(stderr, " -b set brightness (1 - 10)\n");
  fprintf(stderr, " -w set width\n");
  fprintf(stderr, " -h set height\n");
  exit(1);
} // usage()

void 
error(const char *str, bool syscall) { 
  fprintf(stderr, PROGNAME ": Error %s\n",str); 
  if (syscall)
    fprintf(stderr,"(reason for error: %s)\n",strerror(errno));
  exit(1);
} // error()

void 
warning(const char *str, bool syscall) { 
  fprintf(stderr, PROGNAME ": Possible error %s\n",str); 
  if (syscall)
    fprintf(stderr,"(reason for error: %s)\n",strerror(errno));
} // warning()



int 
processUserInput() 
{

  int mouseX, mouseY, mouseButtons;
  char keyHit;

  inputUpdate(mouseX,mouseY,mouseButtons,keyHit);

  if( keyHit == 'q' )
    return -1;


  if (sizeUpdate()) 
    {
      isExpanded = 0;
    }

  return 0;
} // processUserInput()

int 
main(int argc, char **argv) 
{
  int windX=10;
  int windY=30;
  int windWidth=uiWidth;
  int windHeight=uiHeight;
  int c;
  int xx, xy;
  opterr = 0;

  //  openSound(configUseCD);
  openSound(SourceCD, 22050, "/dev/dsp", NULL);

  catchSignals();

  while ((c = getopt(argc, argv, "b:h:w:")) != -1){
    switch (c)
      {
      case '?':
	fprintf(stderr, "%s: unknown option \"%s\"\n", 
		argv[0], argv[optind-1]);
	usage(argv[0]);
	exit(1);	
      case 'b':
	bright = (double) atoi(optarg);
	bright = bright/10;
	break;
      case 'w':
	windWidth = atoi(optarg);
	break;
      case 'h':
	windHeight = atoi(optarg);
	break;
      }
  }
  
  if (bright > 1.0)
    bright = 1.0;
  else if (bright < 0.0)
    bright = 0.0;
  
  if (windWidth < 1)
    windWidth = uiWidth;
  if (windHeight < 1)
    windHeight = uiHeight;

  screenInit(windX,windY,windWidth,windHeight);

  allocOutput(outWidth,outHeight);

  coreInit();
  

  setStarSize(starSize);
  setBrightness(bright);

  time_t timer = time(NULL);
  
  int frames = 0;

  for(;;) {
    fade();
    if (-1 == coreGo())
      break;

    polygonEngine.clear();

    for( xy = 0; xy < 48; xy++)
      {
	for( xx = 0; xx < 48; xx++)
	  {
	    if ( logo[xy][xx] != 0)
	      {
		polygonEngine.add(32769, xx+10, xy+3);
	      }
	  }
      }
    polygonEngine.apply(outputBmp.data);
    screenShow(); 



    frames++;
    if(processUserInput() == -1)
      break;
  } 

  
  timer = time(NULL) - timer;
  delete ucoutput;
  closeSound();
  
  if (timer > 10)
    fprintf(stderr,"Frames per second: %f\n", double(frames)/timer);
  
  return 0;
} // main() /* linux */

#else

int main() 
{
  fprintf(stderr,"KSCD Magic works currently only on Linux.\n"\
	  "It should however be trivial to port it to other platforms ...\n");
} // main() /* non-linux */

#endif

