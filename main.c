/*
PSPSchedule v0.2 by Quinnsoft

©2005 Quinnsoft

http://www.quinnsoft.com/pspschedule.php

Quinnsoft is not responsible for anything that might go wrong by using this software.
You may make derivative works based on this code, provided the source for such derivative works is released.

This program contains portions of P-Sprint 0.30, (c) 2005 Arwin van Arum
*/

#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pspctrl.h>
#include <pspiofilemgr.h>
#include <time.h>
#include "pg.h"
#include "images.c"

PSP_MODULE_INFO("Schedule", 0, 1, 1);

#define printf pspDebugScreenPrintf
#define SIZE 256
#define putChar(x, y, c) pspDebugScreenPutChar((x)*7, (y)*8, 0x0FFFFFFF, c)
#define putCharC(x, y, color, c) pspDebugScreenPutChar(x, y, color, c)
int schedtime[500];
char schedstr[500][100];
int schedentries=0;
char addrname[500][100];
char addrphone[500][50];
char addraddr[500][100];
char addrcity[500][100];
char addremail[500][100];
int addrentries=0;
int formatampm;
struct tm caltime2;

 /* Exit callback */
int exit_callback(int arg1, int arg2, void *common) {
    sceKernelExitGame();
    return 0;
}

/* Callback thread */
int CallbackThread(SceSize args, void *argp) {
    int cbid;

    cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
    sceKernelRegisterExitCallback(cbid);

    sceKernelSleepThreadCB();

    return 0;
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks(void) {
    int thid = 0;

    thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
    if(thid >= 0) {
            sceKernelStartThread(thid, 0, 0);
    }

    return thid;
}

char textentered[SIZE];

//Begin P-Sprint 0.30 code

struct structKeyValues {
	unsigned char l;
	unsigned char u;
	unsigned char r;
	unsigned char d;
	unsigned char s;
	unsigned char t;
	unsigned char o;
	unsigned char x;
};

static u32* g_vram_base = (u32*)(0x04000000+0x40000000);

void putPixel(int x, int y, int color)
{
	g_vram_base[x + y * 512] = color;
}

void drawLine(int x0, int y0, int x1, int y1, int color)
{
	#define SWAP(a, b) tmp = a; a = b; b = tmp;
	int x, y, e, dx, dy, tmp;
	if (x0 > x1) {
		SWAP(x0, x1);
		SWAP(y0, y1);
	}
	e = 0;
	x = x0;
	y = y0;
	dx = x1 - x0;
	dy = y1 - y0;
	if (dy >= 0) {
		if (dx >= dy) {
			for (x = x0; x <= x1; x++) {
				putPixel(x, y, color);
				if (2 * (e + dy) < dx) {
					e += dy;
				} else {
					y++;
					e += dy - dx;
				}
			}
		} else {
			for (y = y0; y <= y1; y++) {
				putPixel(x, y, color);
				if (2 * (e + dx) < dy) {
					e += dx;
				} else {
					x++;
					e += dx - dy;
				}
			}
		}
	} else {
		if (dx >= -dy) {
			for (x = x0; x <= x1; x++) {
				putPixel(x, y, color);
				if (2 * (e + dy) > -dx) {
					e += dy;
				} else {
					y--;
					e += dy + dx;
				}
			}
		} else {   	
			SWAP(x0, x1);
			SWAP(y0, y1);
			x = x0;
			dx = x1 - x0;
			dy = y1 - y0;
			for (y = y0; y <= y1; y++) {
				putPixel(x, y, color);
				if (2 * (e + dx) > -dy) {
					e += dx;
				} else {
					x--;
					e += dx + dy;
				}
			}
		}
	}
}

/*
void writeByte(int fd, unsigned char data)
{
	sceIoWrite(fd, &data, 1);
} */

/*
void screenshot()
{
	const char tgaHeader[] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	const int width = 480;
	const int lineWidth = 512;
	const int height = 272;
	unsigned char lineBuffer[width*4];
	u32* vram = g_vram_base;
	int x, y;
	int fd = sceIoOpen("ms0:/screenshot.tga", PSP_O_CREAT | PSP_O_TRUNC | PSP_O_WRONLY, 0777);
	if (!fd) return;
	sceIoWrite(fd, tgaHeader, sizeof(tgaHeader));
	writeByte(fd, width & 0xff);
	writeByte(fd, width >> 8);
	writeByte(fd, height & 0xff);
	writeByte(fd, height >> 8);
	writeByte(fd, 24);
	writeByte(fd, 0);
	for (y = height - 1; y >= 0; y--) {
		for (x = 0; x < width; x++) {
			u32 color = vram[y * lineWidth + x];
			unsigned char red = color & 0xff;
			unsigned char green = (color >> 8) & 0xff;
			unsigned char blue = (color >> 16) & 0xff;
			lineBuffer[3*x] = blue;
			lineBuffer[3*x+1] = green;
			lineBuffer[3*x+2] = red;
		}
		sceIoWrite(fd, lineBuffer, width * 3);
	}
	sceIoClose(fd);
}  */

void drawRect(int x0, int y0, int x1, int y1, int color, int fill)
{
	int i = 0;
	if(fill)
	{
		
		while((i+y0)<=y1)
		{
			drawLine(x0, y0+i, x1, y0+i, color);
			i++;
		}
	}
	else
	{
		drawLine(x0, y0, x1, y0, color);
		drawLine(x1, y0, x1, y1, color);
		drawLine(x1, y1, x0, y1, color);
		drawLine(x0, y1, x0, y0, color);
	}
}

/*
void write2TextFile(unsigned char writeLine[256])
{
	int i = 0;	
	int pos = 0;

	int fd = sceIoOpen("ms0:/p-spout.txt", PSP_O_APPEND | PSP_O_WRONLY, 0777);
	if (!fd) return;
	
	//pos = sceIoLseek(fd, -1, SEEK_END);

	while(!writeLine[i]==0)
	{
		writeByte(fd,writeLine[i]);
		i++;
		if (i>254){break;}
	}
	writeByte(fd,13);
	writeByte(fd,10);
	//writeByte(fd,0);
	sceIoClose(fd);
}  */


/*

void loadKeys(unsigned int iButtons, int iMode)
{
}

*/
void drawLower()
{
	putCharC(1*7,4*8,0x00000FFF,'l');
	putCharC(2*7,4*8,0x00000FFF,':');
	putCharC(3*7,4*8,0x09999DDF,'.');
	putCharC(4*7,4*8-3,0x09999DDF,'!');
	putCharC(5*7,4*8,0x09999DDF,',');
	putCharC(6*7,4*8,0x0FFFFFFF,'|');
	putCharC(7*7,4*8,0x0DD9999F,':');
	putCharC(8*7,4*8-3,0x0DD9999F,'?');
	putCharC(9*7,4*8,0x0DD9999F,';');

	putCharC(11*7,3*8,0x00000FFF,'u');
	putCharC(12*7,3*8,0x00000FFF,':');
	putCharC(13*7,3*8,0x09999DDF,'(');
	putCharC(14*7,3*8-3,0x09999DDF,'-');
	putCharC(15*7,3*8,0x09999DDF,')');
	putCharC(16*7,3*8,0x0FFFFFFF,'|');
	putCharC(17*7,3*8,0x0DD9999F,39);
	putCharC(18*7,3*8-3,0x0DD9999F,'a');
	putCharC(19*7,3*8,0x0DD9999F,'b');

	putCharC(21*7,4*8,0x00000FFF,'r');
	putCharC(22*7,4*8,0x00000FFF,':');
	putCharC(23*7,4*8,0x09999DDF,'c');
	putCharC(24*7,4*8-3,0x09999DDF,'d');
	putCharC(25*7,4*8,0x09999DDF,'e');
	putCharC(26*7,4*8,0x0FFFFFFF,'|');
	putCharC(27*7,4*8,0x0DD9999F,'f');
	putCharC(28*7,4*8-3,0x0DD9999F,'g');
	putCharC(29*7,4*8,0x0DD9999F,'h');

	putCharC(33*7,4*8,0x09999DDF,'i');
	putCharC(34*7,4*8-3,0x09999DDF,'j');
	putCharC(35*7,4*8,0x09999DDF,'k');
	putCharC(36*7,4*8,0x0FFFFFFF,'|');
	putCharC(37*7,4*8,0x0DD9999F,'l');
	putCharC(38*7,4*8-3,0x0DD9999F,'m');
	putCharC(39*7,4*8,0x0DD9999F,'n');
	putCharC(41*7,4*8,0x0FF0000F,'s');
	putCharC(40*7,4*8,0x0FF0000F,':');

	putCharC(43*7,3*8,0x09999DDF,'o');
	putCharC(44*7,3*8-3,0x09999DDF,'p');
	putCharC(45*7,3*8,0x09999DDF,'q');
	putCharC(46*7,3*8,0x0FFFFFFF,'|');
	putCharC(47*7,3*8,0x0DD9999F,'r');
	putCharC(48*7,3*8-3,0x0DD9999F,'s');
	putCharC(49*7,3*8,0x0DD9999F,'t');
	putCharC(51*7,3*8,0x0FF0000F,'t');
	putCharC(50*7,3*8,0x0FF0000F,':');

	putCharC(53*7,4*8,0x09999DDF,'u');
	putCharC(54*7,4*8-3,0x09999DDF,'v');
	putCharC(55*7,4*8,0x09999DDF,'w');
	putCharC(56*7,4*8,0x0FFFFFFF,'|');
	putCharC(57*7,4*8,0x0DD9999F,'x');
	putCharC(58*7,4*8-3,0x0DD9999F,'y');
	putCharC(59*7,4*8,0x0DD9999F,'z');
	putCharC(61*7,4*8,0x0FF0000F,'o');
	putCharC(60*7,4*8,0x0FF0000F,':');
}

void drawUpper()
{
	putCharC(1*7,4*8,0x00000FFF,'l');
	putCharC(2*7,4*8,0x00000FFF,':');
	putCharC(3*7,4*8,0x09999DDF,'<');
	putCharC(4*7,4*8-3,0x09999DDF,'@');
	putCharC(5*7,4*8,0x09999DDF,'>');
	putCharC(6*7,4*8,0x0FFFFFFF,'|');
	putCharC(7*7,4*8,0x0DD9999F,47);
	putCharC(8*7,4*8-3,0x0DD9999F,'€');
	putCharC(9*7,4*8,0x0DD9999F,92);

	putCharC(11*7,3*8,0x00000FFF,'u');
	putCharC(12*7,3*8,0x00000FFF,':');
	putCharC(13*7,3*8,0x09999DDF,'[');
	putCharC(14*7,3*8-3,0x09999DDF,'$');
	putCharC(15*7,3*8,0x09999DDF,']');
	putCharC(16*7,3*8,0x0FFFFFFF,'|');
	putCharC(17*7,3*8,0x0DD9999F,34);
	putCharC(18*7,3*8-3,0x0DD9999F,'A');
	putCharC(19*7,3*8,0x0DD9999F,'B');

	putCharC(21*7,4*8,0x00000FFF,'r');
	putCharC(22*7,4*8,0x00000FFF,':');
	putCharC(23*7,4*8,0x09999DDF,'C');
	putCharC(24*7,4*8-3,0x09999DDF,'D');
	putCharC(25*7,4*8,0x09999DDF,'E');
	putCharC(26*7,4*8,0x0FFFFFFF,'|');
	putCharC(27*7,4*8,0x0DD9999F,'F');
	putCharC(28*7,4*8-3,0x0DD9999F,'G');
	putCharC(29*7,4*8,0x0DD9999F,'H');

	putCharC(33*7,4*8,0x09999DDF,'I');
	putCharC(34*7,4*8-3,0x09999DDF,'J');
	putCharC(35*7,4*8,0x09999DDF,'K');
	putCharC(36*7,4*8,0x0FFFFFFF,'|');
	putCharC(37*7,4*8,0x0DD9999F,'L');
	putCharC(38*7,4*8-3,0x0DD9999F,'M');
	putCharC(39*7,4*8,0x0DD9999F,'N');
	putCharC(41*7,4*8,0x0FF0000F,'s');
	putCharC(40*7,4*8,0x0FF0000F,':');

	putCharC(43*7,3*8,0x09999DDF,'O');
	putCharC(44*7,3*8-3,0x09999DDF,'P');
	putCharC(45*7,3*8,0x09999DDF,'Q');
	putCharC(46*7,3*8,0x0FFFFFFF,'|');
	putCharC(47*7,3*8,0x0DD9999F,'R');
	putCharC(48*7,3*8-3,0x0DD9999F,'S');
	putCharC(49*7,3*8,0x0DD9999F,'T');
	putCharC(51*7,3*8,0x0FF0000F,'t');
	putCharC(50*7,3*8,0x0FF0000F,':');

	putCharC(53*7,4*8,0x09999DDF,'U');
	putCharC(54*7,4*8-3,0x09999DDF,'V');
	putCharC(55*7,4*8,0x09999DDF,'W');
	putCharC(56*7,4*8,0x0FFFFFFF,'|');
	putCharC(57*7,4*8,0x0DD9999F,'X');
	putCharC(58*7,4*8-3,0x0DD9999F,'Y');
	putCharC(59*7,4*8,0x0DD9999F,'Z');
	putCharC(61*7,4*8,0x0FF0000F,'o');
	putCharC(60*7,4*8,0x0FF0000F,':');
}

void drawNumeric()
{
	putCharC(1*7,4*8,0x00000FFF,'l');
	putCharC(2*7,4*8,0x00000FFF,':');
	putCharC(3*7,4*8,0x09999DDF,'1');
	putCharC(4*7,4*8-3,0x09999DDF,'2');
	putCharC(5*7,4*8,0x09999DDF,'3');
	putCharC(6*7,4*8,0x0FFFFFFF,'|');
	putCharC(7*7,4*8,0x0DD9999F,'4');
	putCharC(8*7,4*8-3,0x0DD9999F,'5');
	putCharC(9*7,4*8,0x0DD9999F,'6');

	putCharC(11*7,3*8,0x00000FFF,'u');
	putCharC(12*7,3*8,0x00000FFF,':');
	putCharC(13*7,3*8,0x09999DDF,'7');
	putCharC(14*7,3*8-3,0x09999DDF,'8');
	putCharC(15*7,3*8,0x09999DDF,'9');
	putCharC(16*7,3*8,0x0FFFFFFF,'|');
	putCharC(17*7,3*8,0x0DD9999F,'0');
	putCharC(18*7,3*8-3,0x0DD9999F,'+');
	putCharC(19*7,3*8,0x0DD9999F,'-');

	putCharC(21*7,4*8,0x00000FFF,'r');
	putCharC(22*7,4*8,0x00000FFF,':');
	putCharC(23*7,4*8,0x09999DDF,'#');
	putCharC(24*7,4*8-3,0x09999DDF,'*');
	putCharC(25*7,4*8,0x09999DDF,47);
	putCharC(26*7,4*8,0x0FFFFFFF,'|');
	putCharC(27*7,4*8,0x0DD9999F,'=');
	putCharC(28*7,4*8-3,0x0DD9999F,'<');
	putCharC(29*7,4*8,0x0DD9999F,'>');

	putCharC(33*7,4*8,0x09999DDF,'(');
	putCharC(34*7,4*8-3,0x09999DDF,')');
	putCharC(35*7,4*8,0x09999DDF,'{');
	putCharC(36*7,4*8,0x0FFFFFFF,'|');
	putCharC(37*7,4*8,0x0DD9999F,'}');
	putCharC(38*7,4*8-3,0x0DD9999F,'[');
	putCharC(39*7,4*8,0x0DD9999F,']');
	putCharC(41*7,4*8,0x0FF0000F,'s');
	putCharC(40*7,4*8,0x0FF0000F,':');

	putCharC(43*7,3*8,0x09999DDF,'.');
	putCharC(44*7,3*8-3,0x09999DDF,',');
	putCharC(45*7,3*8,0x09999DDF,':');
	putCharC(46*7,3*8,0x0FFFFFFF,'|');
	putCharC(47*7,3*8,0x0DD9999F,';');
	putCharC(48*7,3*8-3,0x0DD9999F,'@');
	putCharC(49*7,3*8,0x0DD9999F,'$');
	putCharC(51*7,3*8,0x0FF0000F,'t');
	putCharC(50*7,3*8,0x0FF0000F,':');

	putCharC(53*7,4*8,0x09999DDF,92);
	putCharC(54*7,4*8-3,0x09999DDF,'%');
	putCharC(55*7,4*8,0x09999DDF,'^');
	putCharC(56*7,4*8,0x0FFFFFFF,'|');
	putCharC(57*7,4*8,0x0DD9999F,'&');
	putCharC(58*7,4*8-3,0x0DD9999F,'_');
	putCharC(59*7,4*8,0x0DD9999F,'|');
	putCharC(61*7,4*8,0x0FF0000F,'o');
	putCharC(60*7,4*8,0x0FF0000F,':');
}


void initScreen()
{
	pspDebugScreenInit();
	printf ("\n                p-sprint 0.30a - by Arwin van Arum\n\n\n\n\n\n\n\n");
	printf ("To confirm entry, press START.\n\n\n");
	/*printf (" l: .!,|:?; u: (-)|'ab r: cde|fgh s: ijk|lmn t: opq|rst o: uvw|xyz\n\n\n\n\n\n");
*/
	drawLower();
	drawLine(0,42,479,42,0x00000FFF);

}

int getpsprintinput()
{
	int iButton = 0;
	unsigned int buttonsold;
	const iMaxLO = 256;
	unsigned char lineOutput[iMaxLO];
	int iLO = 0;
	int iPLO = 0;
	int iDone = 0;
	int iGroup = 0;
	int iCaps = 0;
	int iNumeric = 0;
	int iKeyRepeat = 0;
	int iLines = 0;
	int iPos = 0;
	int iTemp = 0;

	/*
        int fd = sceIoOpen("ms0:/p-spout.txt", PSP_O_CREAT | PSP_O_WRONLY, 0777);
	sceIoClose(fd);
        */

	SceCtrlData pad;
	struct structKeyValues kvButtons;

	SetupCallbacks();
	initScreen();
	/* init Keypad input */
	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_DIGITAL);

	/* debug welcome message */
	sceDisplayWaitVblankStart(); 
	
	/* main loop waiting for input from pad*/
	while(!iDone)
	{
		
		iKeyRepeat++;
		if ((iKeyRepeat > 10) & !iButton)
		{
			buttonsold = 0;
			iKeyRepeat = 0;
		}
		
		sceCtrlReadBufferPositive(&pad, 1);
		if (pad.Buttons != buttonsold) 
		{
			if (iButton)
			{
				switch (pad.Buttons) 
				{
					case PSP_CTRL_SELECT:
						/* change input mode */
						iButton = 0;
						if(iNumeric==1)
						{
							drawLower();
							iNumeric =0;
						}
						else if(iCaps==1)
						{
							drawNumeric();
							iNumeric = 1;
							iCaps = 0;
						}
						else if(iCaps==0)
						{
							drawUpper();
							iCaps = 1;

						}

						break;
					case PSP_CTRL_RTRIGGER:
						/* normally reserved for mouse button sim */
						//screenshot();
						break;
					case PSP_CTRL_START:
						/* end line input */
						/*iDone = 1;*/
						iButton = 0;
						lineOutput[iLO]=0;
						printf (" You entered: %s\n",lineOutput);
						strcpy(textentered,lineOutput);
						return 0;
						//write2TextFile(lineOutput);
						drawRect(0,52,479,71,0,1);
						iLO=0;
						iPLO=0;
						break;
					case PSP_CTRL_DOWN:
						/* backspace */
						lineOutput[iLO]=0;
						iButton = 0;
						if (iLO>0)
						{
							iLO--;
						}
						drawLine(0,41,479,41,0);

						break;
					case PSP_CTRL_CROSS:					
						/* space */
						lineOutput[iLO]=' ';
						iLO++;
						iButton = 0;
						break;
					case PSP_CTRL_LEFT:
						lineOutput[iLO]=kvButtons.l;
						iLO++;
						iButton = 0;
						break;
					case PSP_CTRL_UP:
						lineOutput[iLO]=kvButtons.u;
						iLO++;
						iButton = 0;
						break;
					case PSP_CTRL_RIGHT:
						lineOutput[iLO]=kvButtons.r;
						iLO++;
						iButton = 0;
						break;
					case PSP_CTRL_SQUARE:
						lineOutput[iLO]=kvButtons.s;
						iLO++;
						iButton = 0;
						break;
					case PSP_CTRL_TRIANGLE:
						lineOutput[iLO]=kvButtons.t;
						iLO++;
						iButton = 0;
						break;
					case PSP_CTRL_CIRCLE:
						lineOutput[iLO]=kvButtons.o;
						iLO++;
						iButton = 0;
						break;
					
						/* case else: ignore the rest */
				}


			/* get value for 2 button combo */
				
			}
			else
			{
			/* check for important first buttons */
				switch (pad.Buttons) 
				{
					case PSP_CTRL_SELECT:
						/* change input mode */
						iButton = 0;
						if(iNumeric==1)
						{
							drawLower();
							iNumeric =0;
						}
						else if(iCaps==1)
						{
							drawNumeric();
							iNumeric = 1;
							iCaps = 0;
						}
						else
						{
							drawUpper();
							iCaps = 1;

						}

						break;
					case PSP_CTRL_RTRIGGER:
						/* normally reserved for mouse button sim */
						//screenshot();
						break;
					case PSP_CTRL_START:
						/* end line input */
						/*iDone = 1;*/
						iButton = 0;
						lineOutput[iLO]=0;
						printf (" You entered: %s\n",lineOutput);
						strcpy(textentered,lineOutput);
						return 0;
						//write2TextFile(lineOutput);
						sceDisplayWaitVblankStart(); 

						drawRect(0,52,479,71,0,1);
						iLO=0;
						iPLO=0;
						break;
					case PSP_CTRL_DOWN:
						/* backspace */
						lineOutput[iLO]=0;
						iButton = 0;
						if (iLO>0)
						{
							iLO--;
						}						
						break;
					case PSP_CTRL_CROSS:					
						/* space */
						iButton = 0;
						lineOutput[iLO]=' ';
						iLO++;
						break;
					case PSP_CTRL_LEFT:
						/* preload button vals */
						if(iCaps)
						{
							kvButtons.l = '<';
							kvButtons.u = '@';
							kvButtons.r = '>';
							kvButtons.s = 47;
							kvButtons.t = '€';
							kvButtons.o = 92;
						}
						else if (iNumeric)
						{
							kvButtons.l = '1';
							kvButtons.u = '2';
							kvButtons.r = '3';
							kvButtons.s = '4';
							kvButtons.t = '5';
							kvButtons.o = '6';
						}
						else
						{
							kvButtons.l = '.';
							kvButtons.u = '!';
							kvButtons.r = ',';
							kvButtons.s = ':';
							kvButtons.t = '?';
							kvButtons.o = ';';
						}
						iButton++;
						iGroup = 1;
						break;
					case PSP_CTRL_UP:
						/* preload button vals */
						if(iCaps)
						{
							kvButtons.l = '[';
							kvButtons.u = '$';
							kvButtons.r = ']';
							kvButtons.s = 34;
							kvButtons.t = 'A';
							kvButtons.o = 'B';
						}
						else if (iNumeric)
						{
							kvButtons.l = '7';
							kvButtons.u = '8';
							kvButtons.r = '9';
							kvButtons.s = '0';
							kvButtons.t = '+';
							kvButtons.o = '-';
						}
						else
						{
							kvButtons.l = '(';
							kvButtons.u = '-';
							kvButtons.r = ')';
							kvButtons.s = 39;
							kvButtons.t = 'a';
							kvButtons.o = 'b';
						}
						iButton++;
						iGroup = 2;

						break;

					case PSP_CTRL_RIGHT:
						/* preload button vals */
						if(iCaps)
						{
							kvButtons.l = 'C';
							kvButtons.u = 'D';
							kvButtons.r = 'E';
							kvButtons.s = 'F';
							kvButtons.t = 'G';
							kvButtons.o = 'H';
						}
						else if (iNumeric)
						{
							kvButtons.l = '#';
							kvButtons.u = '*';
							kvButtons.r = 47;
							kvButtons.s = '=';
							kvButtons.t = '<';
							kvButtons.o = '>';
						}
						else
						{						
							kvButtons.l = 'c';
							kvButtons.u = 'd';
							kvButtons.r = 'e';
							kvButtons.s = 'f';
							kvButtons.t = 'g';
							kvButtons.o = 'h';
						}
						iButton++;
						iGroup = 3;
						break;

					case PSP_CTRL_SQUARE:
						/* preload button vals */
												if(iCaps)
						{
							kvButtons.l = 'I';
							kvButtons.u = 'J';
							kvButtons.r = 'K';
							kvButtons.s = 'L';
							kvButtons.t = 'M';
							kvButtons.o = 'N';
						}
						else if (iNumeric)
						{
							kvButtons.l = '(';
							kvButtons.u = ')';
							kvButtons.r = '{';
							kvButtons.s = '}';
							kvButtons.t = '[';
							kvButtons.o = ']';
						}
						else
						{
							kvButtons.l = 'i';
							kvButtons.u = 'j';
							kvButtons.r = 'k';
							kvButtons.s = 'l';
							kvButtons.t = 'm';
							kvButtons.o = 'n';
						}
						iButton++;
						iGroup = 4;
						break;
					case PSP_CTRL_TRIANGLE:
						/* preload button vals */
												if(iCaps)
						{
							kvButtons.l = 'O';
							kvButtons.u = 'P';
							kvButtons.r = 'Q';
							kvButtons.s = 'R';
							kvButtons.t = 'S';
							kvButtons.o = 'T';
						}
						else if (iNumeric)
						{
							kvButtons.l = '.';
							kvButtons.u = ',';
							kvButtons.r = ':';
							kvButtons.s = ';';
							kvButtons.t = '@';
							kvButtons.o = '$';
						}
						else
						{
							kvButtons.l = 'o';
							kvButtons.u = 'p';
							kvButtons.r = 'q';
							kvButtons.s = 'r';
							kvButtons.t = 's';
							kvButtons.o = 't';
						}
						iButton++;
						iGroup = 5;
						break;
					case PSP_CTRL_CIRCLE:
						/* preload button vals */
						if(iCaps)
						{
							kvButtons.l = 'U';
							kvButtons.u = 'V';
							kvButtons.r = 'W';
							kvButtons.s = 'X';
							kvButtons.t = 'Y';
							kvButtons.o = 'Z';
						}
						else if (iNumeric)
						{
							kvButtons.l = 92;
							kvButtons.u = '%';
							kvButtons.r = '^';
							kvButtons.s = '&';
							kvButtons.t = '_';
							kvButtons.o = '|';
						}
						else
						{
							kvButtons.l = 'u';
							kvButtons.u = 'v';
							kvButtons.r = 'w';
							kvButtons.s = 'x';
							kvButtons.t = 'y';
							kvButtons.o = 'z';
						}
						iButton++;
						iGroup = 6;
						break;
					
						/* case else:ignore the rest */
				}
				if (iGroup>0){
					drawLine((iGroup*21)+((iGroup-1)*7*7),41,(iGroup*21)+(iGroup*7*7),41,0xFFFFFFFF);
					iGroup = 0;
				}

			}
			if (!(iLO==iPLO))
			{

				/* output key */
				if(iLO<iPLO) 
				{
					/* backspace */
					putChar(iPLO,7+iLines,' ');
					putChar(iLO,7,'_');
				}
				else
				{
					putChar(iLO-1,7,lineOutput[iPLO]);
					putChar(iLO,7,'_');
				}
				iKeyRepeat =0;

				drawLine(0,41,479,41,0);

				iPLO = iLO;

			}
			buttonsold = pad.Buttons;
		}
		sceDisplayWaitVblankStart(); 

	}
	return "";
}

//End P-Sprint 0.30 code

void selectcalbox(struct tm caltime,int dayofmonth){
    int i;
    int j;
    int boxesadded=1;
    int selectdone=0;
    char strprint[SIZE];
    for (j=0;j<6;j++){
         for (i=0;i<7;i++){
             if ((j>0 || i>=caltime.tm_wday) && (!selectdone)) {
                if (dayofmonth==boxesadded){
                    pgScreenFlipV();
                    pgBitBlt(68 * i,31 + (40 * j),69,41,1,img_calrect2);
                    sprintf(strprint, "%i", boxesadded);
                    caltime2.tm_mday = boxesadded;
                    if (dayschedentries(mktime(&caltime2))>0){
                        pgPrint(6.75 + (8.5 * i),4 + (5 * j),0x1F,strprint);
                    } else {
                        pgPrint(6.75 + (8.5 * i),4 + (5 * j),0x7FFF,strprint);
                    }
                    pgScreenFlipV();
                    selectdone=1;
                }
                boxesadded++;
             }
         }
    }
    return;
}

void deselectcalbox(struct tm caltime,int dayofmonth){
    int i;
    int j;
    int boxesadded=1;
    int selectdone=0;
    char strprint[SIZE];
    for (j=0;j<6;j++){
         for (i=0;i<7;i++){
             if ((j>0 || i>=caltime.tm_wday) && (!selectdone)) {
                if (dayofmonth==boxesadded){
                    pgScreenFlipV();
                    pgBitBlt(68 * i,31 + (40 * j),69,41,1,img_calrect);
                    sprintf(strprint, "%i", boxesadded);
                    caltime2.tm_mday = boxesadded;
                    if (dayschedentries(mktime(&caltime2))>0){
                        pgPrint(6.75 + (8.5 * i),4 + (5 * j),0x1F,strprint);
                    } else {
                        pgPrint(6.75 + (8.5 * i),4 + (5 * j),0x7FFF,strprint);
                    }
                    pgScreenFlipV();
                    selectdone=1;
                }
                boxesadded++;
             }
         }
    }
    return;
}

int printschedule(int datestamp, int selection){
    int iteration=0;
    int currentline = 3;
    char tempstr[SIZE];
    char tempstr2[10];
    struct tm *timestruct;
    struct tm timestruct2;
    time_t schtime;
    int firstiteration=-1;
    while (schedtime[iteration] > 0){
        if ((schedtime[iteration] >= datestamp) && (schedtime[iteration] < (datestamp + 86400))){
            schtime = (time_t)schedtime[iteration];
            timestruct = localtime (&schtime);
            timestruct2 = *timestruct;
            if (formatampm){
                strftime (tempstr2, SIZE, "%I:%M %p", &timestruct2);
            } else {
                strftime (tempstr2, SIZE, "%H:%M", &timestruct2);
            }
            sprintf(tempstr,"%s: %s", tempstr2, schedstr[iteration]);
            if (selection == (currentline - 3)){
                pgPrint(0,currentline,0x7FFF,"                                                            ");
                pgPrint(0,currentline,0x1F,tempstr);
            } else {
                pgPrint(0,currentline,0x7FFF,"                                                            ");
                pgPrint(0,currentline,0x7FFF,tempstr);
            }
            currentline++;
        }
        iteration++;
    }
    pgPrint(0,currentline,0x7FFF,"                                                            ");
    return (currentline - 4);
}

int dayschedentries(int datestamp){
    int iteration=0;
    int currentline = 4;
    int firstiteration=-1;
    while (schedtime[iteration] > 0){
        if ((schedtime[iteration] >= datestamp) && (schedtime[iteration] < (datestamp + 86400))){
            currentline++;
        }
        iteration++;
    }
    return (currentline - 4);
}

void deleteschitem(int datestamp, int selection){
    int iteration=0;
    int currentline = 3;
    int i;
    while (schedtime[iteration] > 0){
        if ((schedtime[iteration] >= datestamp) && (schedtime[iteration] < (datestamp + 86400))){
            if (selection == (currentline - 3)){
                schedtime[iteration] = -1;
                strcpy(schedstr[iteration],"");
            } else {
            }
            currentline++;
        }
        iteration++;
    }
    for(i=0;i<schedentries;i++){
        if (schedtime[i] == -1){
            schedtime[i] = schedtime[i+1];
            schedtime[i+1] = -1;
            strcpy(schedstr[i],schedstr[i+1]);
        }
    }
    schedtime[schedentries-1] = -1;
    strcpy(schedstr[schedentries-1], "");
    schedentries--;
    return;
}

void deleteaddr(int selection){
    int i;
    for(i=0;i<addrentries;i++){
        if (i==selection){
           strcpy(addrname[i],"|DELETE|");
        } else {
        }
    }
    for(i=0;i<addrentries;i++){
        if (strcmp(addrname[i],"|DELETE|") == 0){
            strcpy(addrname[i],addrname[i+1]);
            strcpy(addrname[i+1],"|DELETE|");
            strcpy(addraddr[i],addraddr[i+1]);
            strcpy(addrphone[i],addrphone[i+1]);
            strcpy(addrcity[i],addrcity[i+1]);
            strcpy(addremail[i],addremail[i+1]);
        }
    }
    addrentries--;
    return;
}


void printhome(int datestamp){
    int iteration=0;
    int currentline = 17;
    char tempstr[SIZE];
    char tempstr2[10];
    struct tm *timestruct;
    struct tm timestruct2;
    time_t schtime;
    while (schedtime[iteration] > 0){
        if ((schedtime[iteration] >= datestamp) && (schedtime[iteration] < (datestamp + 86400))){
            schtime = (time_t)schedtime[iteration];
            timestruct = localtime (&schtime);
            timestruct2 = *timestruct;
            if (formatampm){
                strftime (tempstr2, SIZE, "%I:%M %p", &timestruct2);
            } else {
                strftime (tempstr2, SIZE, "%H:%M", &timestruct2);
            }
            sprintf(tempstr,"%s: %s", tempstr2, schedstr[iteration]);
            pgPrint(0,currentline,0x7FFF,tempstr);
            currentline++;
        }
        iteration++;
    }
    pgPrint(0,currentline,0x7FFF,"                                                            ");
    return;
}

void printaddr(int selection){
    int i;
    int page=selection/25;

    for(i=(page*25);i<addrentries;i++){
        if ((i==selection) && (i-(page*25))<25){
            pgPrint(0,i-(page*25)+3,0x7FFF,"                                                            ");
            pgPrint(0,i-(page*25)+3,0x1F,addrname[i]);
        } else if ((i-(page*25))<25){
            pgPrint(0,i-(page*25)+3,0x7FFF,"                                                            ");
            pgPrint(0,i-(page*25)+3,0x7FFF,addrname[i]);
        }
    }
    return;
}

void sortschedule(){
    int tempts[500];
    char tempschstr[500][SIZE];
    int lowestindex;
    int i;
    int j;
    for(i=0;i<schedentries;i++){
        for(j=0;j<schedentries;j++){
            if(j==0){
                lowestindex = 0;
            } else {
                if (schedtime[j]<schedtime[lowestindex]){
                   lowestindex = j;
                }
            }
        }
        tempts[i] = schedtime[lowestindex];
        strcpy(tempschstr[i],schedstr[lowestindex]);
        schedtime[lowestindex] = 2147483647;
    }
    for(i=0;i<schedentries;i++){
        schedtime[i] = tempts[i];
        strcpy(schedstr[i],tempschstr[i]);
    }
    return;
}

void sortaddr(){
    char tempname[500][100];
    char tempaddr[500][100];
    char tempphone[500][50];
    char tempcity[500][100];
    char tempemail[500][100];
    int lowestindex;
    int i;
    int j;
    for(i=0;i<addrentries;i++){
        for(j=0;j<addrentries;j++){
            if(j==0){
                lowestindex = 0;
            } else {
                if (strcmp(addrname[j],addrname[lowestindex])<0){
                   lowestindex = j;
                }
            }
        }
        strcpy(tempname[i],addrname[lowestindex]);
        strcpy(addrname[lowestindex],"zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz");
        strcpy(tempaddr[i],addraddr[lowestindex]);
        strcpy(tempphone[i],addrphone[lowestindex]);
        strcpy(tempcity[i],addrcity[lowestindex]);
        strcpy(tempemail[i],addremail[lowestindex]);
    }
    for(i=0;i<addrentries;i++){
        strcpy(addrname[i],tempname[i]);
        strcpy(addrphone[i],tempphone[i]);
        strcpy(addraddr[i],tempaddr[i]);
        strcpy(addrcity[i],tempcity[i]);
        strcpy(addremail[i],tempemail[i]);
    }
    return;
}

int main(int argc, char * argv[]) {
        int fd;
        char teststr[SIZE] = "0,0,0";
        char buffer[SIZE];
        char tempstr[SIZE];
        char tempstr2[SIZE];
        char * pch;
        time_t curtime;
        time_t lastupdate;
        struct tm *loctime;
        struct tm tztime;
        struct tm caltime;
        int done;
        int done2;
        int done3;
        int done4;
        int done5;
	SceCtrlData pad;
	unsigned int buttonsold;
	int selection;
	int selection2;
	int selection3;
	int calselection;
	int addrselection;
	int daysinmonth;
	int boxesadded;
	int refreshmain;
	int daylightsavings;
	int gmtoffset;
	int timestamp;
	int selectedschitem;
	int totschitems;
	int caldatestamp;
	char tsbuffer[5500];
	char schbuffer[50000];
	char addrbuffer[50000];

        pspDebugScreenInit();
        SetupCallbacks();
	pgInit();
	pgScreenFrame(2,0);

        //Get current directory
        char psp_filename[1024 + 1];
        char psp_full_path[1024 + 1];
        char *psp_eboot_path;
        strncpy(psp_full_path, argv[0], sizeof(psp_full_path) - 1);
        psp_full_path[sizeof(psp_full_path) - 1] = '\0';
        psp_eboot_path = strrchr(psp_full_path, '/');
        if (psp_eboot_path != NULL)
        {
           *psp_eboot_path = '\0';
        }
        char basepath[SIZE];
        sprintf(basepath, "%s/%s", psp_full_path, psp_eboot_path);
        //End get current directory

        SceIoStat filestat;
        sprintf(tempstr2, "%sconfig",basepath);
        sceIoGetstat(tempstr2, &filestat);
        fd = sceIoOpen(tempstr2, PSP_O_RDONLY, 0777);
        sceIoRead(fd, teststr, filestat.st_size);
        sceIoClose(fd);
        teststr[filestat.st_size + 1]= NULL;

        sprintf(tempstr2, "%stimestamps",basepath);
        sceIoGetstat(tempstr2, &filestat);
        fd = sceIoOpen(tempstr2, PSP_O_RDONLY, 0777);
        if (filestat.st_size>0){
            sceIoRead(fd, tsbuffer, filestat.st_size - 1);
            tsbuffer[filestat.st_size]= NULL;
        } else {
            sceIoRead(fd, tsbuffer, filestat.st_size);
            tsbuffer[filestat.st_size + 1]= NULL;
        }
        sceIoClose(fd);
        schedentries = 0;
        pch = strtok (tsbuffer,",");
        while (pch != NULL){
            if ((int)strtol(pch, NULL, 10) > 0){
                schedtime[schedentries] = (int)strtol(pch, NULL, 10);
                schedentries++;
            }
            pch = strtok (NULL,",");
        }

        sprintf(tempstr2, "%sscheditems",basepath);
        sceIoGetstat(tempstr2, &filestat);
        fd = sceIoOpen(tempstr2, PSP_O_RDONLY, 0777);
        if (filestat.st_size>0){
           sceIoRead(fd, schbuffer, filestat.st_size - 1);
           schbuffer[filestat.st_size-1]= NULL;
        } else {
           sceIoRead(fd, schbuffer, filestat.st_size);
           schbuffer[filestat.st_size]= NULL;
        }
        sceIoClose(fd);
        schedentries = 0;
        pch = strtok (schbuffer,"|");
        while (pch != NULL){
            strcpy(schedstr[schedentries],pch);
            schedentries++;
            pch = strtok (NULL,"|");
        }
        
        sprintf(tempstr2, "%saddrnames",basepath);
        sceIoGetstat(tempstr2, &filestat);
        fd = sceIoOpen(tempstr2, PSP_O_RDONLY, 0777);
        if (filestat.st_size>0){
           sceIoRead(fd, addrbuffer, filestat.st_size - 1);
           addrbuffer[filestat.st_size-1]= NULL;
        } else {
           sceIoRead(fd, addrbuffer, filestat.st_size);
           addrbuffer[filestat.st_size]= NULL;
        }
        sceIoClose(fd);
        addrentries = 0;
        pch = strtok (addrbuffer,"|");
        while (pch != NULL){
            strcpy(addrname[addrentries],pch);
            addrentries++;
            pch = strtok (NULL,"|");
        }
        
        sprintf(tempstr2, "%saddraddresses",basepath);
        sceIoGetstat(tempstr2, &filestat);
        fd = sceIoOpen(tempstr2, PSP_O_RDONLY, 0777);
        if (filestat.st_size>0){
           sceIoRead(fd, addrbuffer, filestat.st_size - 1);
           addrbuffer[filestat.st_size-1]= NULL;
        } else {
           sceIoRead(fd, addrbuffer, filestat.st_size);
           addrbuffer[filestat.st_size]= NULL;
        }
        sceIoClose(fd);
        addrentries = 0;
        pch = strtok (addrbuffer,"|");
        while (pch != NULL){
            strcpy(addraddr[addrentries],pch);
            addrentries++;
            pch = strtok (NULL,"|");
        }
        
        sprintf(tempstr2, "%saddrcities",basepath);
        sceIoGetstat(tempstr2, &filestat);
        fd = sceIoOpen(tempstr2, PSP_O_RDONLY, 0777);
        if (filestat.st_size>0){
           sceIoRead(fd, addrbuffer, filestat.st_size - 1);
           addrbuffer[filestat.st_size-1]= NULL;
        } else {
           sceIoRead(fd, addrbuffer, filestat.st_size);
           addrbuffer[filestat.st_size]= NULL;
        }
        sceIoClose(fd);
        addrentries = 0;
        pch = strtok (addrbuffer,"|");
        while (pch != NULL){
            strcpy(addrcity[addrentries],pch);
            addrentries++;
            pch = strtok (NULL,"|");
        }
        
        sprintf(tempstr2, "%saddrphone",basepath);
        sceIoGetstat(tempstr2, &filestat);
        fd = sceIoOpen(tempstr2, PSP_O_RDONLY, 0777);
        if (filestat.st_size>0){
           sceIoRead(fd, addrbuffer, filestat.st_size - 1);
           addrbuffer[filestat.st_size-1]= NULL;
        } else {
           sceIoRead(fd, addrbuffer, filestat.st_size);
           addrbuffer[filestat.st_size]= NULL;
        }
        sceIoClose(fd);
        addrentries = 0;
        pch = strtok (addrbuffer,"|");
        while (pch != NULL){
            strcpy(addrphone[addrentries],pch);
            addrentries++;
            pch = strtok (NULL,"|");
        }

        sprintf(tempstr2, "%saddremail",basepath);
        sceIoGetstat(tempstr2, &filestat);
        fd = sceIoOpen(tempstr2, PSP_O_RDONLY, 0777);
        if (filestat.st_size>0){
           sceIoRead(fd, addrbuffer, filestat.st_size - 1);
           addrbuffer[filestat.st_size-1]= NULL;
        } else {
           sceIoRead(fd, addrbuffer, filestat.st_size);
           addrbuffer[filestat.st_size]= NULL;
        }
        sceIoClose(fd);
        addrentries = 0;
        pch = strtok (addrbuffer,"|");
        while (pch != NULL){
            strcpy(addremail[addrentries],pch);
            addrentries++;
            pch = strtok (NULL,"|");
        }

        pgFillvram(0);

        pch = strtok (teststr,",");
        formatampm = (int)strtol(pch, NULL, 10);
        pch = strtok (NULL, ",");
        gmtoffset = (int)strtol(pch, NULL, 10);
        pch = strtok (NULL, ",");
        daylightsavings = (int)strtol(pch, NULL, 10);


	pgBitBlt(0,0,100,100,1,img_calendar);
	pgBitBlt(105,0,100,100,1,img_address2);
	pgBitBlt(210,0,100,100,1,img_settings2);


        curtime = time (NULL);

        lastupdate = curtime;
        loctime = localtime (&curtime);
        tztime = *loctime;
        tztime.tm_hour += gmtoffset + daylightsavings;
        printhome(mktime(&tztime));
        if (formatampm){
            strftime (buffer, SIZE, "%A, %B %d, %Y, %I:%M %p", &tztime);
        } else {
            strftime (buffer, SIZE, "%A, %B %d, %Y, %H:%M", &tztime);
        }

        pgPrint (0,13,0x7FFF,buffer);
        pgPrint (0,15,0x7FFF,"Scheduled items in the next 24 hours:");
        pgPrint (36,32,0x7FFF,"D-Pad L/R: Choose option");
        pgPrint (36,33,0x7FFF,"X Button : Confirm");
	pgScreenFlipV();
        done = 0;
        done2 = 0;
        refreshmain = 0;
	selection = 1;
	while (done == 0){
            done = 0;
            done2 = 0;
            if (refreshmain>0){
                 pgFillvram(0);
                 if (refreshmain == 1){
                     pgBitBlt(0,0,100,100,1,img_calendar);
               	     pgBitBlt(105,0,100,100,1,img_address2);
               	     pgBitBlt(210,0,100,100,1,img_settings2);
               	 } else if (refreshmain == 2) {
                     pgBitBlt(0,0,100,100,1,img_calendar2);
               	     pgBitBlt(105,0,100,100,1,img_address);
               	     pgBitBlt(210,0,100,100,1,img_settings2);
               	 } else {
                     pgBitBlt(0,0,100,100,1,img_calendar2);
               	     pgBitBlt(105,0,100,100,1,img_address2);
               	     pgBitBlt(210,0,100,100,1,img_settings);
               	 }
            	 curtime = time (NULL);
                 lastupdate = curtime;
                 loctime = localtime (&curtime);
                 tztime = *loctime;
                 tztime.tm_hour += gmtoffset + daylightsavings;
                 printhome(mktime(&tztime));
                 if (formatampm){
                     strftime (buffer, SIZE, "%A, %B %d, %Y, %I:%M %p", &tztime);
                 } else {
                     strftime (buffer, SIZE, "%A, %B %d, %Y, %H:%M", &tztime);
                 }
                 pgPrint (0,13,0x7FFF,buffer);
                 pgPrint (0,15,0x7FFF,"Scheduled items in the next 24 hours:");
                 pgPrint (36,32,0x7FFF,"D-Pad L/R: Choose option");
                 pgPrint (36,33,0x7FFF,"X Button : Confirm");
            	 pgScreenFlipV();
            	 pgFillvram(0);
            	 refreshmain = 0;
            }
            sceCtrlReadBufferPositive(&pad, 1);
            if (difftime(time(NULL),lastupdate)>=60) {
                curtime = time (NULL);
                lastupdate = curtime;
                loctime = localtime (&curtime);
                tztime = *loctime;
                tztime.tm_hour += gmtoffset + daylightsavings;
                mktime(&tztime);
                if (formatampm){
                    strftime (buffer, SIZE, "%A, %B %d, %Y, %I:%M %p", &tztime);
                } else {
                    strftime (buffer, SIZE, "%A, %B %d, %Y, %H:%M", &tztime);
                }
                pgScreenFlipV();
                pgPrint (0,13,0x7FFF,buffer);
                pgScreenFlipV();
            }
            if (pad.Buttons != buttonsold)
	    {
		switch (pad.Buttons)
		{
                        case PSP_CTRL_RIGHT:
                             if (selection == 1) {
                                pgScreenFlipV();
                                pgBitBlt(0,0,100,100,1,img_calendar2);
                                pgBitBlt(105,0,100,100,1,img_address);
                                pgScreenFlipV();
                                selection = 2;
                             } else if (selection == 2) {
                                pgScreenFlipV();
                                pgBitBlt(105,0,100,100,1,img_address2);
                                pgBitBlt(210,0,100,100,1,img_settings);
                                pgScreenFlipV();
                                selection = 3;
                             }
                             break;
                        case PSP_CTRL_LEFT:
                             if (selection == 2) {
                                pgScreenFlipV();
                                pgBitBlt(0,0,100,100,1,img_calendar);
                                pgBitBlt(105,0,100,100,1,img_address2);
                                pgScreenFlipV();
                                selection = 1;
                             } else if (selection == 3) {
                                pgScreenFlipV();
                                pgBitBlt(105,0,100,100,1,img_address);
                                pgBitBlt(210,0,100,100,1,img_settings2);
                                pgScreenFlipV();
                                selection = 2;
                             }
                             break;
                        case PSP_CTRL_CROSS:
                             if (selection == 1){
                                 pgFillvram(0);
                                 strftime (buffer, SIZE, "%B, %Y", &tztime);
                                 pgPrint2 (10,0,0x7FFF,buffer);
                                 pgBitBlt(0,20,477,12,1,img_calhead);
                                 caltime.tm_mon = tztime.tm_mon;
                                 caltime.tm_year = tztime.tm_year;
                                 caltime.tm_mday = 1;
                                 caltime.tm_sec = 1;
                                 caltime.tm_min = 0;
                                 caltime.tm_hour = 0;
                                 mktime(&caltime);
                                 caltime2.tm_mon = tztime.tm_mon;
                                 caltime2.tm_year = tztime.tm_year;
                                 caltime2.tm_mday = 1;
                                 caltime2.tm_sec = 1;
                                 caltime2.tm_min = 0;
                                 caltime2.tm_hour = 0;
                                 boxesadded = 1;
                                 if ((caltime.tm_mon == 0) || (caltime.tm_mon == 2) || (caltime.tm_mon == 4) || (caltime.tm_mon == 6) || (caltime.tm_mon == 7) || (caltime.tm_mon == 9) || (caltime.tm_mon == 11)){
                                     daysinmonth = 31;
                                 } else if (caltime.tm_mon == 1) {
                                     if (((caltime.tm_year + 1900) % 4 == 0) && (caltime.tm_year != 200)){
                                         daysinmonth = 29;
                                     } else {
                                         daysinmonth = 28;
                                     }
                                 } else {
                                     daysinmonth = 30;
                                 }
                                 int i;
                                 int j;
                                 for (j=0;j<6;j++){
                                     for (i=0;i<7;i++){
                                         if ((j>0 || i>=caltime.tm_wday) && (boxesadded<=daysinmonth)) {
                                            pgBitBlt(68 * i,31 + (40 * j),69,41,1,img_calrect);
                                            sprintf(tempstr2, "%i", boxesadded);
                                            caltime2.tm_mday = boxesadded;
                                            if (dayschedentries(mktime(&caltime2))>0){
                                                pgPrint(6.75 + (8.5 * i),4 + (5 * j),0x1F,tempstr2);
                                            } else {
                                                pgPrint(6.75 + (8.5 * i),4 + (5 * j),0x7FFF,tempstr2);
                                            }
                                            boxesadded++;
                                         }
                                     }
                                 }
                                 pgPrint(21,30,0x7FFF,"D-Pad      : Select date");
                                 pgPrint(21,31,0x7FFF,"X Button   : View/Set Schedule");
                                 pgPrint(21,32,0x7FFF,"L/R Trigger: Change month");
                                 pgPrint(21,33,0x7FFF,"Triangle   : Back");
                                 pgScreenFlipV();
                                 pgFillvram(0);
                                 buttonsold = pad.Buttons;
                                 calselection = tztime.tm_mday;
                                 selectcalbox(caltime,calselection);
                                 while (done2 == 0){
                                     done2 = 0;
                                     sceCtrlReadBufferPositive(&pad, 1);
                                     if (pad.Buttons != buttonsold) {
                                         switch (pad.Buttons)
    	                                 {
    			                    case PSP_CTRL_TRIANGLE:
                                                 done2 = 1;
                                                 refreshmain = 1;
    			                         break;
		                            case PSP_CTRL_CROSS:
		                                 done3 = 0;
		                                 pgFillvram(0);
		                                 caltime.tm_mday = calselection;
		                                 caltime.tm_hour = 0;
		                                 caltime.tm_min = 0;
		                                 caldatestamp = mktime(&caltime);
		                                 strftime (buffer, SIZE, "%A, %B %d, %Y", &caltime);
		                                 pgPrint2(0,0,0x7FFF,buffer);
		                                 pgPrint(0,30,0x7FFF,"D-Pad Up/Down: Select schedule item");
                                                 pgPrint(0,31,0x7FFF,"Square       : New schedule item");
                                                 pgPrint(0,32,0x7FFF,"Circle       : Delete schedule item");
                                                 pgPrint(0,33,0x7FFF,"Triangle     : Back");
		                                 totschitems = printschedule(caldatestamp,0);
		                                 selectedschitem = 0;
		                                 buttonsold = pad.Buttons;
		                                 pgScreenFlipV();
                                                 pgFillvram(0);
                                                 while (done3 == 0){
                                                     done3 = 0;
                                                     sceCtrlReadBufferPositive(&pad, 1);
                                                     if (pad.Buttons != buttonsold) {
                                                        switch (pad.Buttons)
                                                        {
                                                            case PSP_CTRL_DOWN:
                                                                 if (selectedschitem<totschitems){
                                                                     selectedschitem++;
                                                                     pgScreenFlipV();
                                                                     printschedule(caldatestamp,selectedschitem);
                                                                     pgScreenFlipV();
                                                                 }
                                                                 break;
                                                            case PSP_CTRL_UP:
                                                                 if (selectedschitem>0){
                                                                     selectedschitem--;
                                                                     pgScreenFlipV();
                                                                     printschedule(caldatestamp,selectedschitem);
                                                                     pgScreenFlipV();
                                                                 }
                                                                 break;
                                                            case PSP_CTRL_CIRCLE:
                                                                 deleteschitem(caldatestamp,selectedschitem);
                                                                 totschitems = printschedule(caldatestamp,0);
                                                                 sprintf(tempstr2, "%stimestamps",basepath);
                                                                 fd = sceIoOpen(tempstr2, PSP_O_WRONLY|PSP_O_CREAT, 0777);
                                                                 for (i=0;i<schedentries;i++){
                                                                      sprintf(tempstr2, "%i,",schedtime[i]);
                                                                      sceIoWrite(fd, tempstr2, (strlen(tempstr2)));
                                                                 }
                                                                 sceIoClose(fd);
                                                                 sprintf(tempstr2, "%sscheditems",basepath);
                                                                 fd = sceIoOpen(tempstr2, PSP_O_WRONLY|PSP_O_CREAT, 0777);
                                                                 for (i=0;i<schedentries;i++){
                                                                      sprintf(tempstr2, "%s|",schedstr[i]);
                                                                      sceIoWrite(fd, tempstr2, (strlen(tempstr2)));
                                                                 }
                                                                 sceIoClose(fd);
                                                                 pgScreenFlipV();
                                                                 totschitems = printschedule(caldatestamp,0);
                                                                 pgScreenFlipV();
                                                                 selectedschitem = 0;
                                                                 break;
                                                            case PSP_CTRL_SQUARE:
                                                                 done4 = 0;
                                                                 pgFillvram(0);
                                                                 strcpy(schedstr[schedentries],"(PRESS X)");
                                                                 pgPrint2(0,0,0x7FFF, "Add new schedule item");
                                                                 pgPrint(0,6,0x7FFF,"Date: ");
                                                                 strftime (tempstr2, SIZE, "%m/%d/%Y", &caltime);
                                                                 pgPrint(6,6,0x1F, tempstr2);
                                                                 if (formatampm){
                                                                     strftime (tempstr2, SIZE, "%I:%M %p", &caltime);
                                                                 } else {
                                                                     strftime (tempstr2, SIZE, "%H:%M", &caltime);
                                                                 }
                                                                 sprintf(tempstr, "Time: %s", tempstr2);
                                                                 pgPrint(0,8,0x7FFF,tempstr);
                                                                 sprintf(tempstr2, "Details: %s", schedstr[schedentries]);
                                                                 pgPrint(0,10,0x7FFF,tempstr2);
                                                                 pgPrint(0,31,0x7FFF,"D-Pad Up/Down: Choose setting ");
                                                                 pgPrint(0,32,0x7FFF,"X Button     : Edit setting   ");
                                                                 pgPrint(0,33,0x7FFF,"Press Select to confirm entry.");
                                                                 selection2 = 0;
                                                                 buttonsold = pad.Buttons;
                                                                 pgScreenFlipV();
                                                                 pgFillvram(0);
                                                                 while (done4 == 0){
                                                                     done4 = 0;
                                                                     sceCtrlReadBufferPositive(&pad, 1);
                                                                     if (pad.Buttons != buttonsold) {
                                                                         switch (pad.Buttons)
                                    	                                 {
                                                                                 case PSP_CTRL_SELECT:
                                                                                      timestamp = mktime(&caltime);
                                                                                      schedtime[schedentries] = timestamp;
                                                                                      schedentries++;
                                                                                      sortschedule();
                                                                                      sprintf(tempstr2, "%stimestamps",basepath);
                                                                                      fd = sceIoOpen(tempstr2, PSP_O_WRONLY|PSP_O_CREAT, 0777);
                                                                                      for (i=0;i<schedentries;i++){
                                                                                          sprintf(tempstr2, "%i,",schedtime[i]);
                                                                                          sceIoWrite(fd, tempstr2, (strlen(tempstr2)));
                                                                                      }
                                                                                      sceIoClose(fd);
                                                                                      sprintf(tempstr2, "%sscheditems",basepath);
                                                                                      fd = sceIoOpen(tempstr2, PSP_O_WRONLY|PSP_O_CREAT, 0777);
                                                                                      for (i=0;i<schedentries;i++){
                                                                                          sprintf(tempstr2, "%s|",schedstr[i]);
                                                                                          sceIoWrite(fd, tempstr2, (strlen(tempstr2)));
                                                                                      }
                                                                                      sceIoClose(fd);
                                                                                      done4 = 1;
                                                                                      pgFillvram(0);
                                  		                                      caltime.tm_mday = calselection;
                                  		                                      caltime.tm_hour = 0;
                                        		                              caltime.tm_min = 0;
                                     		                                      caldatestamp = mktime(&caltime);
                                     		                                      strftime (buffer, SIZE, "%A, %B %d, %Y", &caltime);
                                     		                                      pgPrint2(0,0,0x7FFF,buffer);
                                     		                                      pgPrint(0,30,0x7FFF,"D-Pad Up/Down: Select schedule item");
                                     		                                      pgPrint(0,31,0x7FFF,"Square       : New schedule item");
                                     		                                      pgPrint(0,32,0x7FFF,"Circle       : Delete schedule item");
                                     		                                      pgPrint(0,33,0x7FFF,"Triangle     : Back");
                                     		                                      totschitems = printschedule(caldatestamp,0);
                                     		                                      buttonsold = pad.Buttons;
                                     		                                      pgScreenFlipV();
                                                                                      pgFillvram(0);
                                                                                      break;
                                                                                 case PSP_CTRL_DOWN:
                                                                                      if (selection2 == 0){
                                                                                          selection2 = 1;
                                                                                          pgScreenFlipV();
                                                                                          strftime (tempstr2, SIZE, "%m/%d/%Y", &caltime);
                                                                                          pgPrint(6,6,0x7FFF,tempstr2);
                                                                                          if (formatampm){
                                                                                              strftime (tempstr2, SIZE, "%I:%M %p", &caltime);
                                                                                              pgPrint(6,8,0x1F,tempstr2);
                                                                                              strftime (tempstr2, SIZE, "%p", &caltime);
                                                                                              pgPrint(12,8,0x7FFF,tempstr2);
                                                                                          } else {
                                                                                              strftime (tempstr2, SIZE, "%H:%M", &caltime);
                                                                                              pgPrint(6,8,0x1F,tempstr2);
                                                                                          }

                                                                                          pgScreenFlipV();
                                                                                      } else if (selection2 == 1){
                                                                                          selection2 = 2;
                                                                                          pgScreenFlipV();
                                                                                          if (formatampm){
                                                                                              strftime (tempstr2, SIZE, "%I:%M %p", &caltime);
                                                                                          } else {
                                                                                              strftime (tempstr2, SIZE, "%H:%M", &caltime);
                                                                                          }
                                                                                          pgPrint(6,8,0x7FFF,tempstr2);
                                                                                          pgPrint(9,10,0x1F,schedstr[schedentries]);
                                                                                          pgScreenFlipV();
                                                                                      }
                                                                                      break;
                                                                                 case PSP_CTRL_UP:
                                                                                      if (selection2 == 1){
                                                                                          selection2 = 0;
                                                                                          pgScreenFlipV();
                                                                                          strftime (tempstr, SIZE, "%m/%d/%Y", &caltime);
                                                                                          pgPrint(6,6,0x1F,tempstr);
                                                                                          if (formatampm){
                                                                                              strftime (tempstr2, SIZE, "%I:%M %p", &caltime);
                                                                                          } else {
                                                                                              strftime (tempstr2, SIZE, "%H:%M", &caltime);
                                                                                          }
                                                                                          pgPrint(6,8,0x7FFF,tempstr2);
                                                                                          pgScreenFlipV();
                                                                                      } else if (selection2 == 2){
                                                                                          selection2 = 1;
                                                                                          pgScreenFlipV();
                                                                                          pgPrint(9,10,0x7FFF,schedstr[schedentries]);
                                                                                          if (formatampm){
                                                                                              strftime (tempstr2, SIZE, "%I:%M %p", &caltime);
                                                                                              pgPrint(6,8,0x1F,tempstr2);
                                                                                              strftime (tempstr2, SIZE, "%p", &caltime);
                                                                                              pgPrint(12,8,0x7FFF,tempstr2);
                                                                                          } else {
                                                                                              strftime (tempstr2, SIZE, "%H:%M", &caltime);
                                                                                              pgPrint(6,8,0x1F,tempstr);
                                                                                          }
                                                                                          pgScreenFlipV();
                                                                                      }
                                                                                      break;
                                                                                 case PSP_CTRL_CROSS:
                                                                                      if (selection2 == 0){
                                                                                          pgScreenFlipV();
                                                                                          strftime (tempstr, SIZE, "%m", &caltime);
                                                                                          pgPrint(6,6,0x1F,tempstr);
                                                                                          strftime (tempstr, SIZE, "/%d/%Y", &caltime);
                                                                                          pgPrint(8,6,0x7FFF,tempstr);
                                                                                          pgPrint(0,31,0x7FFF,"D-Pad Left/Right: Select D/M/Y");
                                                                                          pgPrint(0,32,0x7FFF,"D-Pad Up/Down   : Edit setting");
                                                                                          pgPrint(0,33,0x7FFF,"X Button        : Confirm     ");
                                                                                          pgScreenFlipV();
                                                                                          buttonsold = pad.Buttons;
                                                                                          done5 = 0;
                                                                                          selection3 = 0;
                                                                                          int buttonrep = 0;
                                                                                          int daysinmonthsch = daysinmonth;
                                                                                          while (done5 == 0){
                                                                                              done5 = 0;
                                                                                              sceCtrlReadBufferPositive(&pad, 1);
                                                                                              if ((pad.Buttons != buttonsold) || (buttonrep > 10)) {
                                                                                                  if (pad.Buttons != buttonsold){
                                                                                                      buttonrep = 0;
                                                                                                  }
                                                                                                  switch (pad.Buttons)
                                                            	                                  {
                                                                                                         case PSP_CTRL_CROSS:
                                                                                                              done5 = 1;
                                                                                                              strftime (tempstr, SIZE, "%m/%d/%Y", &caltime);
                                                                                                              pgScreenFlipV();
                                                                                                              pgPrint(6,6,0x1F,tempstr);
                                                                                                              pgPrint(0,31,0x7FFF,"D-Pad Up/Down: Choose setting ");
                                                                                                              pgPrint(0,32,0x7FFF,"X Button     : Edit setting   ");
                                                                                                              pgPrint(0,33,0x7FFF,"Press Select to confirm entry.");
                                                                                                              pgScreenFlipV();
                                                                                                              break;
                                                                                                         case PSP_CTRL_RIGHT:
                                                                                                              if (selection3 == 0){
                                                                                                                  selection3 = 1;
                                                                                                                  pgScreenFlipV();
                                                                                                                  strftime (tempstr, SIZE, "%m", &caltime);
                                                                                                                  pgPrint(6,6,0x7FFF,tempstr);
                                                                                                                  strftime (tempstr, SIZE, "%d", &caltime);
                                                                                                                  pgPrint(9,6,0x1F,tempstr);
                                                                                                                  pgScreenFlipV();
                                                                                                              } else if (selection3 == 1){
                                                                                                                  selection3 = 2;
                                                                                                                  pgScreenFlipV();
                                                                                                                  strftime (tempstr, SIZE, "%d", &caltime);
                                                                                                                  pgPrint(9,6,0x7FFF,tempstr);
                                                                                                                  strftime (tempstr, SIZE, "%Y", &caltime);
                                                                                                                  pgPrint(12,6,0x1F,tempstr);
                                                                                                                  pgScreenFlipV();
                                                                                                              }
                                                                                                              break;
                                                                                                         case PSP_CTRL_LEFT:
                                                                                                              if (selection3 == 1){
                                                                                                                  selection3 = 0;
                                                                                                                  pgScreenFlipV();
                                                                                                                  strftime (tempstr, SIZE, "%m", &caltime);
                                                                                                                  pgPrint(6,6,0x1F,tempstr);
                                                                                                                  strftime (tempstr, SIZE, "%d", &caltime);
                                                                                                                  pgPrint(9,6,0x7FFF,tempstr);
                                                                                                                  pgScreenFlipV();
                                                                                                              } else if (selection3 == 2){
                                                                                                                  selection3 = 1;
                                                                                                                  pgScreenFlipV();
                                                                                                                  strftime (tempstr, SIZE, "%d", &caltime);
                                                                                                                  pgPrint(9,6,0x1F,tempstr);
                                                                                                                  strftime (tempstr, SIZE, "%Y", &caltime);
                                                                                                                  pgPrint(12,6,0x7FFF,tempstr);
                                                                                                                  pgScreenFlipV();
                                                                                                              }
                                                                                                              break;
                                                                                                         case PSP_CTRL_UP:
                                                                                                              if (selection3 == 0){
                                                                                                                  if (caltime.tm_mon < 11){
                                                                                                                      caltime.tm_mon++;
                                                                                                                      if ((caltime.tm_mon == 0) || (caltime.tm_mon == 2) || (caltime.tm_mon == 4) || (caltime.tm_mon == 6) || (caltime.tm_mon == 7) || (caltime.tm_mon == 9) || (caltime.tm_mon == 11)){
                                                                                                                         daysinmonthsch = 31;
                                                                                                                      } else if (caltime.tm_mon == 1) {
                                                                                                                         if (((caltime.tm_year + 1900) % 4 == 0) && (caltime.tm_year != 200)){
                                                                                                                             daysinmonthsch = 29;
                                                                                                                         } else {
                                                                                                                             daysinmonthsch = 28;
                                                                                                                         }
                                                                                                                      } else {
                                                                                                                         daysinmonthsch = 30;
                                                                                                                      }
                                                                                                                      pgScreenFlipV();
                                                                                                                      strftime (tempstr, SIZE, "%m", &caltime);
                                                                                                                      pgPrint(6,6,0x1F,tempstr);
                                                                                                                      pgScreenFlipV();
                                                                                                                  }
                                                                                                              } else if (selection3 == 1){
                                                                                                                  if (caltime.tm_mday < daysinmonthsch) {
                                                                                                                      caltime.tm_mday++;
                                                                                                                      pgScreenFlipV();
                                                                                                                      strftime (tempstr, SIZE, "%d", &caltime);
                                                                                                                      pgPrint(9,6,0x1F,tempstr);
                                                                                                                      pgScreenFlipV();
                                                                                                                  }
                                                                                                              } else if (selection3 == 2){
                                                                                                                  caltime.tm_year++;
                                                                                                                  if (caltime.tm_mon == 1){
                                                                                                                      if (((caltime.tm_year + 1900) % 4 == 0) && (caltime.tm_year != 200)){
                                                                                                                          daysinmonthsch = 29;
                                                                                                                      } else {
                                                                                                                          daysinmonthsch = 28;
                                                                                                                          if (caltime.tm_mday == 29){
                                                                                                                              caltime.tm_mday = 28;
                                                                                                                              pgScreenFlipV();
                                                                                                                              pgPrint(9,6,0x7FFF,"28");
                                                                                                                              pgScreenFlipV();
                                                                                                                          }
                                                                                                                      }
                                                                                                                  }
                                                                                                                  strftime (tempstr, SIZE, "%Y", &caltime);
                                                                                                                  pgScreenFlipV();
                                                                                                                  pgPrint(12,6,0x1F,tempstr);
                                                                                                                  pgScreenFlipV();
                                                                                                              }
                                                                                                              break;
                                                                                                         case PSP_CTRL_DOWN:
                                                                                                              if (selection3 == 0){
                                                                                                                  if (caltime.tm_mon > 0){
                                                                                                                      caltime.tm_mon--;
                                                                                                                      if ((caltime.tm_mon == 0) || (caltime.tm_mon == 2) || (caltime.tm_mon == 4) || (caltime.tm_mon == 6) || (caltime.tm_mon == 7) || (caltime.tm_mon == 9) || (caltime.tm_mon == 11)){
                                                                                                                         daysinmonthsch = 31;
                                                                                                                      } else if (caltime.tm_mon == 1) {
                                                                                                                         if (((caltime.tm_year + 1900) % 4 == 0) && (caltime.tm_year != 200)){
                                                                                                                             daysinmonthsch = 29;
                                                                                                                         } else {
                                                                                                                             daysinmonthsch = 28;
                                                                                                                         }
                                                                                                                      } else {
                                                                                                                         daysinmonthsch = 30;
                                                                                                                      }
                                                                                                                      pgScreenFlipV();
                                                                                                                      strftime (tempstr, SIZE, "%m", &caltime);
                                                                                                                      pgPrint(6,6,0x1F,tempstr);
                                                                                                                      pgScreenFlipV();
                                                                                                                  }
                                                                                                              } else if (selection3 == 1){
                                                                                                                  if (caltime.tm_mday > 1) {
                                                                                                                      caltime.tm_mday--;
                                                                                                                      pgScreenFlipV();
                                                                                                                      strftime (tempstr, SIZE, "%d", &caltime);
                                                                                                                      pgPrint(9,6,0x1F,tempstr);
                                                                                                                      pgScreenFlipV();
                                                                                                                  }
                                                                                                              } else if (selection3 == 2){
                                                                                                                  caltime.tm_year--;
                                                                                                                  if (caltime.tm_mon == 1){
                                                                                                                      if (((caltime.tm_year + 1900) % 4 == 0) && (caltime.tm_year != 200)){
                                                                                                                          daysinmonthsch = 29;
                                                                                                                      } else {
                                                                                                                          daysinmonthsch = 28;
                                                                                                                          if (caltime.tm_mday == 29){
                                                                                                                              caltime.tm_mday = 28;
                                                                                                                              pgScreenFlipV();
                                                                                                                              pgPrint(9,6,0x7FFF,"28");
                                                                                                                              pgScreenFlipV();
                                                                                                                          }
                                                                                                                      }
                                                                                                                  }
                                                                                                                  strftime (tempstr, SIZE, "%Y", &caltime);
                                                                                                                  pgScreenFlipV();
                                                                                                                  pgPrint(12,6,0x1F,tempstr);
                                                                                                                  pgScreenFlipV();
                                                                                                              }
                                                                                                              break;
                                                                                                  }
                                                                                              } else {
                                                                                                  buttonrep++;
                                                                                              }
                                                                                              buttonsold = pad.Buttons;
                                                                                          }
                                                                                      } else if (selection2 == 1){
                                                                                          pgScreenFlipV();
                                                                                          if (formatampm){
                                                                                             strftime (tempstr2, SIZE, "%I:%M %p", &caltime);
                                                                                              pgPrint(6,8,0x1F,tempstr2);
                                                                                              strftime (tempstr2, SIZE, "%p", &caltime);
                                                                                              pgPrint(12,8,0x7FFF,tempstr2);
                                                                                          } else {
                                                                                             strftime (tempstr, SIZE, "%H:%M", &caltime);
                                                                                             pgPrint(6,8,0x1F,tempstr);
                                                                                          }

                                                                                          strftime (tempstr, SIZE, ":%M", &caltime);
                                                                                          pgPrint(8,8,0x7FFF,tempstr);
                                                                                          pgPrint(0,31,0x7FFF,"D-Pad Left/Right: Select H/M  ");
                                                                                          pgPrint(0,32,0x7FFF,"D-Pad Up/Down   : Edit setting");
                                                                                          pgPrint(0,33,0x7FFF,"X Button        : Confirm     ");
                                                                                          pgScreenFlipV();
                                                                                          buttonsold = pad.Buttons;
                                                                                          done5 = 0;
                                                                                          selection3 = 0;
                                                                                          int buttonrep = 0;
                                                                                          while (done5 == 0){
                                                                                              done5 = 0;
                                                                                              sceCtrlReadBufferPositive(&pad, 1);
                                                                                              if ((pad.Buttons != buttonsold) || (buttonrep > 10)) {
                                                                                                  if (pad.Buttons != buttonsold){
                                                                                                      buttonrep = 0;
                                                                                                  }
                                                                                                  switch (pad.Buttons)
                                                            	                                  {
                                                                                                      case PSP_CTRL_LEFT:
                                                                                                      case PSP_CTRL_RIGHT:
                                                                                                           if (selection3 == 0){
                                                                                                               selection3 = 1;
                                                                                                               pgScreenFlipV();
                                                                                                               if (formatampm){
                                                                                                                   strftime(tempstr,SIZE,"%I",&caltime);
                                                                                                               } else {
                                                                                                                   strftime(tempstr,SIZE,"%H",&caltime);
                                                                                                               }
                                                                                                               pgPrint(6,8,0x7FFF,tempstr);
                                                                                                               strftime(tempstr,SIZE,"%M",&caltime);
                                                                                                               pgPrint(9,8,0x1F,tempstr);
                                                                                                               if (formatampm){
                                                                                                                   strftime(tempstr,SIZE,"%p",&caltime);
                                                                                                                   pgPrint(12,8,0x7FFF,tempstr);
                                                                                                               }
                                                                                                               pgScreenFlipV();
                                                                                                           } else {
                                                                                                               selection3 = 0;
                                                                                                               pgScreenFlipV();
                                                                                                               if (formatampm){
                                                                                                                   strftime(tempstr,SIZE,"%I",&caltime);
                                                                                                               } else {
                                                                                                                   strftime(tempstr,SIZE,"%H",&caltime);
                                                                                                               }
                                                                                                               pgPrint(6,8,0x1F,tempstr);
                                                                                                               strftime(tempstr,SIZE,"%M",&caltime);
                                                                                                               pgPrint(9,8,0x7FFF,tempstr);
                                                                                                               if (formatampm){
                                                                                                                   strftime(tempstr,SIZE,"%p",&caltime);
                                                                                                                   pgPrint(12,8,0x7FFF,tempstr);
                                                                                                               }
                                                                                                               pgScreenFlipV();
                                                                                                           }
                                                                                                           break;
                                                                                                      case PSP_CTRL_UP:
                                                                                                           if (selection3 == 0){
                                                                                                               caltime.tm_hour++;
                                                                                                               caltime.tm_hour = caltime.tm_hour % 24;
                                                                                                               if (formatampm){
                                                                                                                   strftime(tempstr,SIZE,"%I",&caltime);
                                                                                                               } else {
                                                                                                                   strftime(tempstr,SIZE,"%H",&caltime);
                                                                                                               }
                                                                                                               pgScreenFlipV();
                                                                                                               pgPrint(6,8,0x1F,tempstr);
                                                                                                               if (formatampm){
                                                                                                                   strftime(tempstr,SIZE,"%p",&caltime);
                                                                                                                   pgPrint(12,8,0x7FFF,tempstr);
                                                                                                               }
                                                                                                               pgScreenFlipV();
                                                                                                           } else {
                                                                                                               caltime.tm_min++;
                                                                                                               caltime.tm_min = caltime.tm_min % 60;
                                                                                                               strftime(tempstr,SIZE,"%M",&caltime);
                                                                                                               pgScreenFlipV();
                                                                                                               pgPrint(9,8,0x1F,tempstr);
                                                                                                               pgScreenFlipV();
                                                                                                           }
                                                                                                           break;
                                                                                                       case PSP_CTRL_DOWN:
                                                                                                           if (selection3 == 0){
                                                                                                               caltime.tm_hour--;
                                                                                                               if (caltime.tm_hour < 0){
                                                                                                                   caltime.tm_hour += 24;
                                                                                                               }
                                                                                                               if (formatampm){
                                                                                                                   strftime(tempstr,SIZE,"%I",&caltime);
                                                                                                               } else {
                                                                                                                   strftime(tempstr,SIZE,"%H",&caltime);
                                                                                                               }
                                                                                                               pgScreenFlipV();
                                                                                                               pgPrint(6,8,0x1F,tempstr);
                                                                                                               if (formatampm){
                                                                                                                   strftime(tempstr,SIZE,"%p",&caltime);
                                                                                                                   pgPrint(12,8,0x7FFF,tempstr);
                                                                                                               }
                                                                                                               pgScreenFlipV();
                                                                                                           } else {
                                                                                                               caltime.tm_min--;
                                                                                                               if (caltime.tm_min < 0){
                                                                                                                   caltime.tm_min += 60;
                                                                                                               }
                                                                                                               strftime(tempstr,SIZE,"%M",&caltime);
                                                                                                               pgScreenFlipV();
                                                                                                               pgPrint(9,8,0x1F,tempstr);
                                                                                                               pgScreenFlipV();
                                                                                                           }
                                                                                                           break;
                                                                                                       case PSP_CTRL_CROSS:
                                                                                                           done5 = 1;
                                                                                                           if (formatampm){
                                                                                                               strftime (tempstr, SIZE, "%I:%M %p", &caltime);
                                                                                                           } else {
                                                                                                               strftime (tempstr, SIZE, "%H:%M", &caltime);
                                                                                                           }
                                                                                                           pgScreenFlipV();
                                                                                                           pgPrint(6,8,0x1F,tempstr);
                                                                                                           pgPrint(0,31,0x7FFF,"D-Pad Up/Down: Choose setting ");
                                                                                                           pgPrint(0,32,0x7FFF,"X Button     : Edit setting   ");
                                                                                                           pgPrint(0,33,0x7FFF,"Press Select to confirm entry.");
                                                                                                           pgScreenFlipV();
                                                                                                           break;
                                                                                                  }
                                                                                              } else {
                                                                                                  buttonrep++;
                                                                                              }
                                                                                              buttonsold = pad.Buttons;
                                                                                          }
                                                                                      } else {
                                                                                          strcpy(tempstr2,"");
                                                                                          getpsprintinput();
                                                                                          strcpy(schedstr[schedentries],textentered);
                                                                                          pgScreenFrame(2,0);
                                                                                          pgFillvram(0);
                                                                                          pgPrint2(0,0,0x7FFF, "Add new schedule item");
                                                                                          pgPrint(0,6,0x7FFF,"Date: ");
                                                                                          strftime (tempstr2, SIZE, "%m/%d/%Y", &caltime);
                                                                                          pgPrint(6,6,0x7FFF, tempstr2);
                                                                                          if (formatampm){
                                                                                              strftime (tempstr2, SIZE, "%I:%M %p", &caltime);
                                                                                          } else {
                                                                                              strftime (tempstr2, SIZE, "%H:%M", &caltime);
                                                                                          }
                                                                                          sprintf(tempstr, "Time: %s", tempstr2);
                                                                                          pgPrint(0,8,0x7FFF,tempstr);
                                                                                          pgPrint(0,10,0x7FFF,"Details: ");
                                                                                          pgPrint(9,10,0x1F,schedstr[schedentries]);
                                                                                          pgPrint(0,31,0x7FFF,"D-Pad Up/Down: Choose setting ");
                                                                                          pgPrint(0,32,0x7FFF,"X Button     : Edit setting   ");
                                                                                          pgPrint(0,33,0x7FFF,"Press Select to confirm entry.");
                                                                                          pgScreenFlipV();
                                                                                          pgFillvram(0);
                                                                                          buttonsold = PSP_CTRL_START;
                                                                                      }
                                                                                      break;

                                                                         }
                                                                     }
                                                                     buttonsold = pad.Buttons;
                                                                 }
                                                                 break;
                                                            case PSP_CTRL_TRIANGLE:
                                                                 done3 = 1;
                                                                 caltime.tm_mon = tztime.tm_mon;
                                                                 caltime.tm_year = tztime.tm_year;
                                                                 caltime.tm_mday = 1;
                                                                 caltime.tm_sec = 1;
                                                                 caltime.tm_min = 0;
                                                                 caltime.tm_hour = 0;
                                                                 mktime(&caltime);
                                                                 caltime2.tm_mon = tztime.tm_mon;
                                                                 caltime2.tm_year = tztime.tm_year;
                                                                 caltime2.tm_mday = 1;
                                                                 caltime2.tm_sec = 1;
                                                                 caltime2.tm_min = 0;
                                                                 caltime2.tm_hour = 0;
                                                                 boxesadded = 1;
                                                                 if ((caltime.tm_mon == 0) || (caltime.tm_mon == 2) || (caltime.tm_mon == 4) || (caltime.tm_mon == 6) || (caltime.tm_mon == 7) || (caltime.tm_mon == 9) || (caltime.tm_mon == 11)){
                                                                     daysinmonth = 31;
                                                                 } else if (caltime.tm_mon == 1) {
                                                                     if (((caltime.tm_year + 1900) % 4 == 0) && (caltime.tm_year != 200)){
                                                                         daysinmonth = 29;
                                                                     } else {
                                                                         daysinmonth = 28;
                                                                     }
                                                                 } else {
                                                                     daysinmonth = 30;
                                                                 }
                                                                 pgScreenFlipV();
                                                                 pgFillvram(0);
                                                                 strftime (buffer, SIZE, "%B, %Y", &caltime);
                                                                 pgPrint2 (10,0,0x7FFF,buffer);
                                                                 pgBitBlt(0,20,477,12,1,img_calhead);
                                                                 for (j=0;j<6;j++){
                                                                     for (i=0;i<7;i++){
                                                                         if ((j>0 || i>=caltime.tm_wday) && (boxesadded<=daysinmonth)) {
                                                                            pgBitBlt(68 * i,31 + (40 * j),69,41,1,img_calrect);
                                                                            sprintf(tempstr2, "%i", boxesadded);
                                                                            caltime2.tm_mday = boxesadded;
                                                                            if (dayschedentries(mktime(&caltime2))>0){
                                                                                pgPrint(6.75 + (8.5 * i),4 + (5 * j),0x1F,tempstr2);
                                                                            } else {
                                                                                pgPrint(6.75 + (8.5 * i),4 + (5 * j),0x7FFF,tempstr2);
                                                                            }
                                                                            boxesadded++;
                                                                         }
                                                                     }
                                                                 }
                                                                 pgPrint(21,30,0x7FFF,"D-Pad      : Select date");
                                                                 pgPrint(21,31,0x7FFF,"X Button   : View/Set Schedule");
                                                                 pgPrint(21,32,0x7FFF,"L/R Trigger: Change month");
                                                                 pgPrint(21,33,0x7FFF,"Triangle   : Back");
                                                                 pgScreenFlipV();
                                                                 selectcalbox(caltime,calselection);
                                                                 break;
                                                        }
                                                     }
                                                     buttonsold = pad.Buttons;
                                                 }
		                                 break;
		                            case PSP_CTRL_UP:
		                                 if (calselection > 7) {
                                                     deselectcalbox(caltime,calselection);
                                                     calselection -= 7;
                                                     selectcalbox(caltime,calselection);
                                                 } else if (calselection != 1) {
                                                     deselectcalbox(caltime,calselection);
                                                     calselection = 1;
                                                     selectcalbox(caltime,calselection);
                                                 }
                                                 break;
                                            case PSP_CTRL_DOWN:
                                                 if (calselection <= (daysinmonth - 7)){
                                                     deselectcalbox(caltime,calselection);
                                                     calselection += 7;
                                                     selectcalbox(caltime,calselection);
                                                 } else if (calselection != daysinmonth) {
                                                     deselectcalbox(caltime,calselection);
                                                     calselection = daysinmonth;
                                                     selectcalbox(caltime,daysinmonth);
                                                 }
                                                 break;
                                            case PSP_CTRL_LEFT:
                                                 if (calselection > 1){
                                                     deselectcalbox(caltime,calselection);
                                                     calselection -= 1;
                                                     selectcalbox(caltime,calselection);
                                                 }
                                                 break;
                                            case PSP_CTRL_RIGHT:
                                                 if (calselection < daysinmonth){
                                                     deselectcalbox(caltime,calselection);
                                                     calselection += 1;
                                                     selectcalbox(caltime,calselection);
                                                 }
                                                 break;
                                            case PSP_CTRL_LTRIGGER:
                                                 if (caltime.tm_mon == 0) {
                                                     caltime.tm_mon = 11;
                                                     caltime.tm_year-=1;
                                                 } else {
                                                     caltime.tm_mon -= 1;
                                                 }
                                                 caltime.tm_mday = 1;
                                                 caltime.tm_sec = 1;
                                                 caltime.tm_min = 0;
                                                 caltime.tm_hour = 0;
                                                 caltime.tm_isdst = -1;
                                                 mktime(&caltime);
                                                 caltime2.tm_year=caltime.tm_year;
                                                 caltime2.tm_mon = caltime.tm_mon;
                                                 caltime2.tm_mday = caltime.tm_mday;
                                                 caltime2.tm_sec = 1;
                                                 caltime2.tm_min = 0;
                                                 caltime2.tm_hour = 0;
                                                 caltime2.tm_isdst = -1;
                                                 boxesadded = 1;
                                                 if ((caltime.tm_mon == 0) || (caltime.tm_mon == 2) || (caltime.tm_mon == 4) || (caltime.tm_mon == 6) || (caltime.tm_mon == 7) || (caltime.tm_mon == 9) || (caltime.tm_mon == 11)){
                                                     daysinmonth = 31;
                                                 } else if (caltime.tm_mon == 1) {
                                                     if (((caltime.tm_year + 1900) % 4 == 0) && (caltime.tm_year != 200)){
                                                         daysinmonth = 29;
                                                     } else {
                                                         daysinmonth = 28;
                                                     }
                                                 } else {
                                                     daysinmonth = 30;
                                                 }
                                                 pgScreenFlipV();
                                                 pgFillvram(0);
                                                 strftime (buffer, SIZE, "%B, %Y", &caltime);
                                                 pgPrint2 (10,0,0x7FFF,buffer);
                                                 pgBitBlt(0,20,477,12,1,img_calhead);
                                                 for (j=0;j<6;j++){
                                                     for (i=0;i<7;i++){
                                                         if ((j>0 || i>=caltime.tm_wday) && (boxesadded<=daysinmonth)) {
                                                            pgBitBlt(68 * i,31 + (40 * j),69,41,1,img_calrect);
                                                            sprintf(tempstr2, "%i", boxesadded);
                                                            caltime2.tm_mday = boxesadded;
                                                            if (dayschedentries(mktime(&caltime2))>0){
                                                                pgPrint(6.75 + (8.5 * i),4 + (5 * j),0x1F,tempstr2);
                                                            } else {
                                                                pgPrint(6.75 + (8.5 * i),4 + (5 * j),0x7FFF,tempstr2);
                                                            }
                                                            boxesadded++;
                                                         }
                                                     }
                                                 }
                                                 pgPrint(21,30,0x7FFF,"D-Pad      : Select date");
                                                 pgPrint(21,31,0x7FFF,"X Button   : View/Set Schedule");
                                                 pgPrint(21,32,0x7FFF,"L/R Trigger: Change month");
                                                 pgPrint(21,33,0x7FFF,"Triangle   : Back");
                                                 pgScreenFlipV();
                                                 calselection = 1;
                                                 selectcalbox(caltime,calselection);
                                                 break;
                                            case PSP_CTRL_RTRIGGER:
                                                 caltime.tm_mon += 1;
                                                 caltime.tm_mday = 1;
                                                 caltime.tm_sec = 1;
                                                 caltime.tm_min = 0;
                                                 caltime.tm_hour = 0;
                                                 mktime(&caltime);
                                                 caltime2.tm_year=caltime.tm_year;
                                                 caltime2.tm_mon = caltime.tm_mon;
                                                 caltime2.tm_mday = caltime.tm_mday;
                                                 caltime2.tm_sec = 1;
                                                 caltime2.tm_min = 0;
                                                 caltime2.tm_hour = 0;
                                                 caltime2.tm_isdst = -1;
                                                 boxesadded = 1;
                                                 if ((caltime.tm_mon == 0) || (caltime.tm_mon == 2) || (caltime.tm_mon == 4) || (caltime.tm_mon == 6) || (caltime.tm_mon == 7) || (caltime.tm_mon == 9) || (caltime.tm_mon == 11)){
                                                     daysinmonth = 31;
                                                 } else if (caltime.tm_mon == 1) {
                                                     if (((caltime.tm_year + 1900) % 4 == 0) && (caltime.tm_year != 200)){
                                                         daysinmonth = 29;
                                                     } else {
                                                         daysinmonth = 28;
                                                     }
                                                 } else {
                                                     daysinmonth = 30;
                                                 }
                                                 pgScreenFlipV();
                                                 pgFillvram(0);
                                                 strftime (buffer, SIZE, "%B, %Y", &caltime);
                                                 pgPrint2 (10,0,0x7FFF,buffer);
                                                 pgBitBlt(0,20,477,12,1,img_calhead);
                                                 for (j=0;j<6;j++){
                                                     for (i=0;i<7;i++){
                                                         if ((j>0 || i>=caltime.tm_wday) && (boxesadded<=daysinmonth)) {
                                                            pgBitBlt(68 * i,31 + (40 * j),69,41,1,img_calrect);
                                                            sprintf(tempstr2, "%i", boxesadded);
                                                            caltime2.tm_mday = boxesadded;
                                                            if (dayschedentries(mktime(&caltime2))>0){
                                                                pgPrint(6.75 + (8.5 * i),4 + (5 * j),0x1F,tempstr2);
                                                            } else {
                                                                pgPrint(6.75 + (8.5 * i),4 + (5 * j),0x7FFF,tempstr2);
                                                            }
                                                            boxesadded++;
                                                         }
                                                     }
                                                 }
                                                 pgPrint(21,30,0x7FFF,"D-Pad      : Select date");
                                                 pgPrint(21,31,0x7FFF,"X Button   : View/Set Schedule");
                                                 pgPrint(21,32,0x7FFF,"L/R Trigger: Change month");
                                                 pgPrint(21,33,0x7FFF,"Triangle   : Back");
                                                 pgScreenFlipV();
                                                 calselection = 1;
                                                 selectcalbox(caltime,calselection);
                                                 break;
                                         }
                                     }
                                     buttonsold = pad.Buttons;
                                 }
                             } else if (selection == 2) {
                                 pgFillvram(0);
                                 pgPrint2(9,0,0x7FFF,"Address Book");
                                 pgPrint(0,29,0x7FFF,"D-Pad Up/Down: Select name");
                                 pgPrint(0,30,0x7FFF,"X Button     : View details");
                                 pgPrint(0,31,0x7FFF,"Square       : New entry");
                                 pgPrint(0,32,0x7FFF,"Circle       : Delete entry");
                                 pgPrint(0,33,0x7FFF,"Triangle     : Back");
                                 addrselection = 0;
                                 printaddr(0);
                                 pgScreenFlipV();
                                 pgFillvram(0);
                                 buttonsold = pad.Buttons;
                                 done2 = 0;
                                 selection2 = 0;
                                 while (done2 == 0){
                                     done2 = 0;
                                     sceCtrlReadBufferPositive(&pad, 1);
                                     if (pad.Buttons != buttonsold) {
                                         switch (pad.Buttons)
    	                                 {
                                             case PSP_CTRL_LTRIGGER:
                                                  if (addrselection>24){
                                                      addrselection-=25;
                                                      pgScreenFlipV();
                                                      pgFillvram(0);
                                                      pgPrint2(9,0,0x7FFF,"Address Book");
                                                      pgPrint(0,29,0x7FFF,"D-Pad Up/Down: Select name");
                                                      pgPrint(0,30,0x7FFF,"X Button     : View details");
                                                      pgPrint(0,31,0x7FFF,"Square       : New entry");
                                                      pgPrint(0,32,0x7FFF,"Circle       : Delete entry");
                                                      pgPrint(0,33,0x7FFF,"Triangle     : Back");
                                                      printaddr(addrselection);
                                                      pgScreenFlipV();
                                                  }
                                                  break;
                                             case PSP_CTRL_RTRIGGER:
                                                  if (addrselection<(addrentries-25)){
                                                      addrselection+=25;
                                                      pgScreenFlipV();
                                                      pgFillvram(0);
                                                      pgPrint2(9,0,0x7FFF,"Address Book");
                                                      pgPrint(0,29,0x7FFF,"D-Pad Up/Down: Select name");
                                                      pgPrint(0,30,0x7FFF,"X Button     : View details");
                                                      pgPrint(0,31,0x7FFF,"Square       : New entry");
                                                      pgPrint(0,32,0x7FFF,"Circle       : Delete entry");
                                                      pgPrint(0,33,0x7FFF,"Triangle     : Back");
                                                      printaddr(addrselection);
                                                      pgScreenFlipV();
                                                  }
                                                  break;
                                             case PSP_CTRL_UP:
                                                  if (addrselection>0){
                                                      addrselection--;
                                                      pgScreenFlipV();
                                                      pgFillvram(0);
                                                      pgPrint2(9,0,0x7FFF,"Address Book");
                                                      pgPrint(0,29,0x7FFF,"D-Pad Up/Down: Select name");
                                                      pgPrint(0,30,0x7FFF,"X Button     : View details");
                                                      pgPrint(0,31,0x7FFF,"Square       : New entry");
                                                      pgPrint(0,32,0x7FFF,"Circle       : Delete entry");
                                                      pgPrint(0,33,0x7FFF,"Triangle     : Back");
                                                      printaddr(addrselection);
                                                      pgScreenFlipV();
                                                  }
                                                  break;
                                             case PSP_CTRL_DOWN:
                                                  if (addrselection<(addrentries-1)){
                                                      addrselection++;
                                                      pgScreenFlipV();
                                                      pgFillvram(0);
                                                      pgPrint2(9,0,0x7FFF,"Address Book");
                                                      pgPrint(0,29,0x7FFF,"D-Pad Up/Down: Select name");
                                                      pgPrint(0,30,0x7FFF,"X Button     : View details");
                                                      pgPrint(0,31,0x7FFF,"Square       : New entry");
                                                      pgPrint(0,32,0x7FFF,"Circle       : Delete entry");
                                                      pgPrint(0,33,0x7FFF,"Triangle     : Back");
                                                      printaddr(addrselection);
                                                      pgScreenFlipV();
                                                  }
                                                  break;
                                             case PSP_CTRL_CIRCLE:
                                                  if (addrentries>0){
                                                      deleteaddr(addrselection);
                                                      sprintf(tempstr2, "%saddrnames",basepath);

                                                      fd = sceIoOpen(tempstr2, PSP_O_WRONLY|PSP_O_CREAT, 0777);
                                                      int i;
                                                      for (i=0;i<addrentries;i++){
                                                          sprintf(tempstr2, "%s|",addrname[i]);
                                                          sceIoWrite(fd, tempstr2, (strlen(tempstr2)));
                                                      }
                                                      sceIoClose(fd);

                                                      sprintf(tempstr2, "%saddraddresses",basepath);
                                                      fd = sceIoOpen(tempstr2, PSP_O_WRONLY|PSP_O_CREAT, 0777);
                                                      for (i=0;i<addrentries;i++){
                                                          sprintf(tempstr2, "%s|",addraddr[i]);
                                                          sceIoWrite(fd, tempstr2, (strlen(tempstr2)));
                                                      }
                                                      sceIoClose(fd);

                                                      sprintf(tempstr2, "%saddrcities",basepath);
                                                      fd = sceIoOpen(tempstr2, PSP_O_WRONLY|PSP_O_CREAT, 0777);
                                                      for (i=0;i<addrentries;i++){
                                                          sprintf(tempstr2, "%s|",addrcity[i]);
                                                          sceIoWrite(fd, tempstr2, (strlen(tempstr2)));
                                                      }
                                                      sceIoClose(fd);

                                                      sprintf(tempstr2, "%saddrphone",basepath);
                                                      fd = sceIoOpen(tempstr2, PSP_O_WRONLY|PSP_O_CREAT, 0777);
                                                      for (i=0;i<addrentries;i++){
                                                          sprintf(tempstr2, "%s|",addrphone[i]);
                                                          sceIoWrite(fd, tempstr2, (strlen(tempstr2)));
                                                      }
                                                      sceIoClose(fd);

                                                      sprintf(tempstr2, "%saddremail",basepath);
                                                      fd = sceIoOpen(tempstr2, PSP_O_WRONLY|PSP_O_CREAT, 0777);
                                                      for (i=0;i<addrentries;i++){
                                                          sprintf(tempstr2, "%s|",addremail[i]);
                                                          sceIoWrite(fd, tempstr2, (strlen(tempstr2)));
                                                      }
                                                      sceIoClose(fd);

                                                      pgScreenFlipV();
                                                      if (addrselection > 0){
                                                          addrselection--;
                                                          printaddr(addrselection);
                                                      }
                                                      pgScreenFlipV();
                                                  }
                                                  break;
                                             case PSP_CTRL_CROSS:
                                                  if (addrentries>0){
                                                      pgScreenFlipV();
                                                      pgFillvram(0);
                                                      pgPrint2(9,0,0x7FFF,"Address Book");
                                                      pgPrint(0,33,0x7FFF,"Square  : Edit");
                                                      pgPrint(0,33,0x7FFF,"Triangle: Back");
                                                      pgPrint(0,4,0x7FFF,"Name: ");
                                                      pgPrint(0,6,0x7FFF,"Address: ");
                                                      pgPrint(0,8,0x7FFF,"City/State: ");
                                                      pgPrint(0,10,0x7FFF,"Phone: ");
                                                      pgPrint(0,12,0x7FFF,"Email: ");
                                                      pgPrint(6,4,0x7FFF,addrname[addrselection]);
                                                      pgPrint(9,6,0x7FFF,addraddr[addrselection]);
                                                      pgPrint(12,8,0x7FFF,addrcity[addrselection]);
                                                      pgPrint(7,10,0x7FFF,addrphone[addrselection]);
                                                      pgPrint(7,12,0x7FFF,addremail[addrselection]);
                                                      pgScreenFlipV();
                                                      pgFillvram(0);
                                                      buttonsold = pad.Buttons;
                                                      done3 = 0;
                                                      while (done3 == 0){
                                                         done3 = 0;
                                                         sceCtrlReadBufferPositive(&pad, 1);
                                                         if (pad.Buttons != buttonsold) {
                                                             switch (pad.Buttons)
                       	                                     {
                                                                 case PSP_CTRL_TRIANGLE:
                                                                      pgScreenFlipV();
                                                                      pgFillvram(0);
                                                                      pgPrint2(9,0,0x7FFF,"Address Book");
                                                                      pgPrint(0,29,0x7FFF,"D-Pad Up/Down: Select name");
                                                                      pgPrint(0,30,0x7FFF,"X Button     : View details");
                                                                      pgPrint(0,31,0x7FFF,"Square       : New entry");
                                                                      pgPrint(0,32,0x7FFF,"Circle       : Delete entry");
                                                                      pgPrint(0,33,0x7FFF,"Triangle     : Back");
                                                                      printaddr(addrselection);
                                                                      pgScreenFlipV();
                                                                      pgFillvram(0);
                                                                      done3 = 1;
                                                                      buttonsold = pad.Buttons;
                                                                      break;
                                                                 case PSP_CTRL_SQUARE:
                                                                      pgFillvram(0);
                                                                      pgPrint(0,31,0x7FFF,"D-Pad Up/Down   : Select field");
                                                                      pgPrint(0,32,0x7FFF,"X Button        : Edit field");
                                                                      pgPrint(0,33,0x7FFF,"Select          : Save");
                                                                      pgPrint2(4,0,0x7FFF,"Edit Address Book Entry");
                                                                      pgPrint(0,4,0x7FFF,"Name: ");
                                                                      pgPrint(0,6,0x7FFF,"Address: ");
                                                                      pgPrint(0,8,0x7FFF,"City/State: ");
                                                                      pgPrint(0,10,0x7FFF,"Phone: ");
                                                                      pgPrint(0,12,0x7FFF,"Email: ");
                                                                      pgPrint(6,4,0x1F,addrname[addrselection]);
                                                                      pgPrint(9,6,0x7FFF,addraddr[addrselection]);
                                                                      pgPrint(12,8,0x7FFF,addrcity[addrselection]);
                                                                      pgPrint(7,10,0x7FFF,addrphone[addrselection]);
                                                                      pgPrint(7,12,0x7FFF,addremail[addrselection]);
                                                                      pgScreenFlipV();
                                                                      pgFillvram(0);
                                                                      buttonsold = pad.Buttons;
                                                                      selection3 = 0;
                                                                      done3 = 0;
                                                                      while (done3 == 0){
                                                                         done3 = 0;
                                                                         sceCtrlReadBufferPositive(&pad, 1);
                                                                         if (pad.Buttons != buttonsold) {
                                                                             switch (pad.Buttons)
                                        	                                 {
                                                                                  case PSP_CTRL_SELECT:
                                                                                      sortaddr();
                                                                                      sprintf(tempstr2, "%saddrnames",basepath);
                                                                                      fd = sceIoOpen(tempstr2, PSP_O_WRONLY|PSP_O_CREAT, 0777);
                                                                                      int i;
                                                                                      for (i=0;i<addrentries;i++){
                                                                                          sprintf(tempstr2, "%s|",addrname[i]);
                                                                                          sceIoWrite(fd, tempstr2, (strlen(tempstr2)));
                                                                                      }
                                                                                      sceIoClose(fd);
                                                                                      
                                                                                      sprintf(tempstr2, "%saddraddresses",basepath);
                                                                                      fd = sceIoOpen(tempstr2, PSP_O_WRONLY|PSP_O_CREAT, 0777);
                                                                                      for (i=0;i<addrentries;i++){
                                                                                          sprintf(tempstr2, "%s|",addraddr[i]);
                                                                                          sceIoWrite(fd, tempstr2, (strlen(tempstr2)));
                                                                                      }
                                                                                      sceIoClose(fd);
                                                                                      
                                                                                      sprintf(tempstr2, "%saddrcities",basepath);
                                                                                      fd = sceIoOpen(tempstr2, PSP_O_WRONLY|PSP_O_CREAT, 0777);
                                                                                      for (i=0;i<addrentries;i++){
                                                                                          sprintf(tempstr2, "%s|",addrcity[i]);
                                                                                          sceIoWrite(fd, tempstr2, (strlen(tempstr2)));
                                                                                      }
                                                                                      sceIoClose(fd);
                                                                                      
                                                                                      sprintf(tempstr2, "%saddrphone",basepath);
                                                                                      fd = sceIoOpen(tempstr2, PSP_O_WRONLY|PSP_O_CREAT, 0777);
                                                                                      for (i=0;i<addrentries;i++){
                                                                                          sprintf(tempstr2, "%s|",addrphone[i]);
                                                                                          sceIoWrite(fd, tempstr2, (strlen(tempstr2)));
                                                                                      }
                                                                                      sceIoClose(fd);
                                                                                      
                                                                                      sprintf(tempstr2, "%saddremail",basepath);
                                                                                      fd = sceIoOpen(tempstr2, PSP_O_WRONLY|PSP_O_CREAT, 0777);
                                                                                      for (i=0;i<addrentries;i++){
                                                                                          sprintf(tempstr2, "%s|",addremail[i]);
                                                                                          sceIoWrite(fd, tempstr2, (strlen(tempstr2)));
                                                                                      }
                                                                                      sceIoClose(fd);
                    
                                                                                      done3 = 1;
                                                                                      pgFillvram(0);
                                                                                      pgPrint2(9,0,0x7FFF,"Address Book");
                                                                                      pgPrint(0,29,0x7FFF,"D-Pad Up/Down   : Select name");
                                                                                      pgPrint(0,30,0x7FFF,"X Button        : View details");
                                                                                      pgPrint(0,31,0x7FFF,"Square          : New entry");
                                                                                      pgPrint(0,32,0x7FFF,"Circle          : Delete entry");
                                                                                      pgPrint(0,33,0x7FFF,"Triangle        : Back");
                                                                                      addrselection = 0;
                                                                                      printaddr(0);
                                                                                      pgScreenFlipV();
                                                                                      pgFillvram(0);
                                                                                      break;
                                                                                 case PSP_CTRL_CROSS:
                                                                                      if (selection3 == 0){
                                                                                          getpsprintinput();
                                                                                          strcpy(addrname[addrselection],textentered);
                                                                                          pgScreenFrame(2,0);
                                                                                          pgFillvram(0);
                                                                                          pgPrint(0,4,0x7FFF,"Name: ");
                                                                                          pgPrint(0,6,0x7FFF,"Address: ");
                                                                                          pgPrint(0,8,0x7FFF,"City/State: ");
                                                                                          pgPrint(0,10,0x7FFF,"Phone: ");
                                                                                          pgPrint(0,12,0x7FFF,"Email: ");
                                                                                          pgPrint(6,4,0x1F,addrname[addrselection]);
                                                                                          pgPrint(9,6,0x7FFF,addraddr[addrselection]);
                                                                                          pgPrint(12,8,0x7FFF,addrcity[addrselection]);
                                                                                          pgPrint(7,10,0x7FFF,addrphone[addrselection]);
                                                                                          pgPrint(7,12,0x7FFF,addremail[addrselection]);
                                                                                      } else if (selection3 == 1){
                                                                                          getpsprintinput();
                                                                                          strcpy(addraddr[addrselection],textentered);
                                                                                          pgScreenFrame(2,0);
                                                                                          pgFillvram(0);
                                                                                          pgPrint(0,4,0x7FFF,"Name: ");
                                                                                          pgPrint(0,6,0x7FFF,"Address: ");
                                                                                          pgPrint(0,8,0x7FFF,"City/State: ");
                                                                                          pgPrint(0,10,0x7FFF,"Phone: ");
                                                                                          pgPrint(0,12,0x7FFF,"Email: ");
                                                                                          pgPrint(6,4,0x7FFF,addrname[addrselection]);
                                                                                          pgPrint(9,6,0x1F,addraddr[addrselection]);
                                                                                          pgPrint(12,8,0x7FFF,addrcity[addrselection]);
                                                                                          pgPrint(7,10,0x7FFF,addrphone[addrselection]);
                                                                                          pgPrint(7,12,0x7FFF,addremail[addrselection]);
                                                                                      } else if (selection3 == 2){
                                                                                          getpsprintinput();
                                                                                          strcpy(addrcity[addrselection],textentered);
                                                                                          pgScreenFrame(2,0);
                                                                                          pgFillvram(0);
                                                                                          pgPrint(0,4,0x7FFF,"Name: ");
                                                                                          pgPrint(0,6,0x7FFF,"Address: ");
                                                                                          pgPrint(0,8,0x7FFF,"City/State: ");
                                                                                          pgPrint(0,10,0x7FFF,"Phone: ");
                                                                                          pgPrint(0,12,0x7FFF,"Email: ");
                                                                                          pgPrint(6,4,0x7FFF,addrname[addrselection]);
                                                                                          pgPrint(9,6,0x7FFF,addraddr[addrselection]);
                                                                                          pgPrint(12,8,0x1F,addrcity[addrselection]);
                                                                                          pgPrint(7,10,0x7FFF,addrphone[addrselection]);
                                                                                          pgPrint(7,12,0x7FFF,addremail[addrselection]);
                                                                                      } else if (selection3 == 3){
                                                                                          getpsprintinput();
                                                                                          strcpy(addrphone[addrentries],textentered);
                                                                                          pgScreenFrame(2,0);
                                                                                          pgFillvram(0);
                                                                                          pgPrint(0,4,0x7FFF,"Name: ");
                                                                                          pgPrint(0,6,0x7FFF,"Address: ");
                                                                                          pgPrint(0,8,0x7FFF,"City/State: ");
                                                                                          pgPrint(0,10,0x7FFF,"Phone: ");
                                                                                          pgPrint(0,12,0x7FFF,"Email: ");
                                                                                          pgPrint(6,4,0x7FFF,addrname[addrselection]);
                                                                                          pgPrint(9,6,0x7FFF,addraddr[addrselection]);
                                                                                          pgPrint(12,8,0x7FFF,addrcity[addrselection]);
                                                                                          pgPrint(7,10,0x1F,addrphone[addrselection]);
                                                                                          pgPrint(7,12,0x7FFF,addremail[addrselection]);
                                                                                      } else if (selection3 == 4){
                                                                                          getpsprintinput();
                                                                                          strcpy(addremail[addrentries],textentered);
                                                                                          pgScreenFrame(2,0);
                                                                                          pgFillvram(0);
                                                                                          pgPrint(0,4,0x7FFF,"Name: ");
                                                                                          pgPrint(0,6,0x7FFF,"Address: ");
                                                                                          pgPrint(0,8,0x7FFF,"City/State: ");
                                                                                          pgPrint(0,10,0x7FFF,"Phone: ");
                                                                                          pgPrint(0,12,0x7FFF,"Email: ");
                                                                                          pgPrint(6,4,0x7FFF,addrname[addrselection]);
                                                                                          pgPrint(9,6,0x7FFF,addraddr[addrselection]);
                                                                                          pgPrint(12,8,0x7FFF,addrcity[addrselection]);
                                                                                          pgPrint(7,10,0x7FFF,addrphone[addrselection]);
                                                                                          pgPrint(7,12,0x1F,addremail[addrselection]);
                                                                                      }
                                                                                      pgPrint(0,31,0x7FFF,"D-Pad Up/Down   : Select field");
                                                                                      pgPrint(0,32,0x7FFF,"X Button        : Edit field");
                                                                                      pgPrint(0,33,0x7FFF,"Select          : Save");
                                                                                      pgPrint2(4,0,0x7FFF,"Edit Address Book Entry");
                                                                                      pgScreenFlipV();
                                                                                      pgFillvram(0);
                                                                                      break;
                                                                                 case PSP_CTRL_DOWN:
                                                                                      if (selection3 == 0){
                                                                                          selection3 = 1;
                                                                                          pgScreenFlipV();
                                                                                          pgPrint(6,4,0x7FFF,addrname[addrselection]);
                                                                                          pgPrint(9,6,0x1F,addraddr[addrselection]);
                                                                                          pgScreenFlipV();
                                                                                      } else if (selection3 == 1){
                                                                                          selection3 = 2;
                                                                                          pgScreenFlipV();
                                                                                          pgPrint(9,6,0x7FFF,addraddr[addrselection]);
                                                                                          pgPrint(12,8,0x1F,addrcity[addrselection]);
                                                                                          pgScreenFlipV();
                                                                                      } else if (selection3 == 2){
                                                                                          selection3 = 3;
                                                                                          pgScreenFlipV();
                                                                                          pgPrint(12,8,0x7FFF,addrcity[addrselection]);
                                                                                          pgPrint(7,10,0x1F,addrphone[addrselection]);
                                                                                          pgScreenFlipV();
                                                                                      } else if (selection3 == 3){
                                                                                          selection3 = 4;
                                                                                          pgScreenFlipV();
                                                                                          pgPrint(7,10,0x7FFF,addrphone[addrselection]);
                                                                                          pgPrint(7,12,0x1F,addremail[addrselection]);
                                                                                          pgScreenFlipV();
                                                                                      }
                                                                                      break;
                                                                                 case PSP_CTRL_UP:
                                                                                      if (selection3 == 1){
                                                                                          selection3 = 0;
                                                                                          pgScreenFlipV();
                                                                                          pgPrint(6,4,0x1F,addrname[addrselection]);
                                                                                          pgPrint(9,6,0x7FFF,addraddr[addrselection]);
                                                                                          pgScreenFlipV();
                                                                                      } else if (selection3 == 2){
                                                                                          selection3 = 1;
                                                                                          pgScreenFlipV();
                                                                                          pgPrint(9,6,0x1F,addraddr[addrselection]);
                                                                                          pgPrint(12,8,0x7FFF,addrcity[addrselection]);
                                                                                          pgScreenFlipV();
                                                                                      } else if (selection3 == 3){
                                                                                          selection3 = 2;
                                                                                          pgScreenFlipV();
                                                                                          pgPrint(12,8,0x1F,addrcity[addrselection]);
                                                                                          pgPrint(7,10,0x7FFF,addrphone[addrselection]);
                                                                                          pgScreenFlipV();
                                                                                      } else if (selection3 == 4){
                                                                                          selection3 = 3;
                                                                                          pgScreenFlipV();
                                                                                          pgPrint(7,10,0x1F,addrphone[addrselection]);
                                                                                          pgPrint(7,12,0x7FFF,addremail[addrselection]);
                                                                                          pgScreenFlipV();
                                                                                      }
                                                                                      break;
                                                                             }
                                                                         }
                                                                         buttonsold = pad.Buttons;
                                                                      }
                                                                      break;
                                                             }
                                                         }
                                                         buttonsold = pad.Buttons;
                                                      }
                                                  }
                                                  break;
                                             case PSP_CTRL_TRIANGLE:
                                                  done2 = 1;
                                                  refreshmain = 2;
                                                  break;
                                             case PSP_CTRL_SQUARE:
                                                  pgFillvram(0);
                                                  pgPrint(0,31,0x7FFF,"D-Pad Up/Down   : Select field");
                                                  pgPrint(0,32,0x7FFF,"X Button        : Edit field");
                                                  pgPrint(0,33,0x7FFF,"Select          : Save");
                                                  pgPrint2(4,0,0x7FFF,"New Address Book Entry");
                                                  strcpy(addrname[addrentries],"(Press X)");
                                                  strcpy(addrphone[addrentries],"(Press X)");
                                                  strcpy(addraddr[addrentries],"(Press X)");
                                                  strcpy(addrcity[addrentries],"(Press X)");
                                                  strcpy(addremail[addrentries],"(Press X)");
                                                  pgPrint(0,4,0x7FFF,"Name: ");
                                                  pgPrint(0,6,0x7FFF,"Address: ");
                                                  pgPrint(0,8,0x7FFF,"City/State: ");
                                                  pgPrint(0,10,0x7FFF,"Phone: ");
                                                  pgPrint(0,12,0x7FFF,"Email: ");
                                                  pgPrint(6,4,0x1F,addrname[addrentries]);
                                                  pgPrint(9,6,0x7FFF,addraddr[addrentries]);
                                                  pgPrint(12,8,0x7FFF,addrcity[addrentries]);
                                                  pgPrint(7,10,0x7FFF,addrphone[addrentries]);
                                                  pgPrint(7,12,0x7FFF,addremail[addrentries]);
                                                  pgScreenFlipV();
                                                  pgFillvram(0);
                                                  buttonsold = pad.Buttons;
                                                  selection3 = 0;
                                                  done3 = 0;
                                                  while (done3 == 0){
                                                     done3 = 0;
                                                     sceCtrlReadBufferPositive(&pad, 1);
                                                     if (pad.Buttons != buttonsold) {
                                                         switch (pad.Buttons)
                    	                                 {
                                                              case PSP_CTRL_SELECT:
                                                                  addrentries++;
                                                                  sortaddr();
                                                                  sprintf(tempstr2, "%saddrnames",basepath);
                                                                  fd = sceIoOpen(tempstr2, PSP_O_WRONLY|PSP_O_CREAT, 0777);
                                                                  int i;
                                                                  for (i=0;i<addrentries;i++){
                                                                      sprintf(tempstr2, "%s|",addrname[i]);
                                                                      sceIoWrite(fd, tempstr2, (strlen(tempstr2)));
                                                                  }
                                                                  sceIoClose(fd);
                                                                  
                                                                  sprintf(tempstr2, "%saddraddresses",basepath);
                                                                  fd = sceIoOpen(tempstr2, PSP_O_WRONLY|PSP_O_CREAT, 0777);
                                                                  for (i=0;i<addrentries;i++){
                                                                      sprintf(tempstr2, "%s|",addraddr[i]);
                                                                      sceIoWrite(fd, tempstr2, (strlen(tempstr2)));
                                                                  }
                                                                  sceIoClose(fd);
                                                                  
                                                                  sprintf(tempstr2, "%saddrcities",basepath);
                                                                  fd = sceIoOpen(tempstr2, PSP_O_WRONLY|PSP_O_CREAT, 0777);
                                                                  for (i=0;i<addrentries;i++){
                                                                      sprintf(tempstr2, "%s|",addrcity[i]);
                                                                      sceIoWrite(fd, tempstr2, (strlen(tempstr2)));
                                                                  }
                                                                  sceIoClose(fd);
                                                                  
                                                                  sprintf(tempstr2, "%saddrphone",basepath);
                                                                  fd = sceIoOpen(tempstr2, PSP_O_WRONLY|PSP_O_CREAT, 0777);
                                                                  for (i=0;i<addrentries;i++){
                                                                      sprintf(tempstr2, "%s|",addrphone[i]);
                                                                      sceIoWrite(fd, tempstr2, (strlen(tempstr2)));
                                                                  }
                                                                  sceIoClose(fd);
                                                                  
                                                                  sprintf(tempstr2, "%saddremail",basepath);
                                                                  fd = sceIoOpen(tempstr2, PSP_O_WRONLY|PSP_O_CREAT, 0777);
                                                                  for (i=0;i<addrentries;i++){
                                                                      sprintf(tempstr2, "%s|",addremail[i]);
                                                                      sceIoWrite(fd, tempstr2, (strlen(tempstr2)));
                                                                  }
                                                                  sceIoClose(fd);

                                                                  done3 = 1;
                                                                  pgFillvram(0);
                                                                  pgPrint2(9,0,0x7FFF,"Address Book");
                                                                  pgPrint(0,29,0x7FFF,"D-Pad Up/Down   : Select name");
                                                                  pgPrint(0,30,0x7FFF,"X Button        : View details");
                                                                  pgPrint(0,31,0x7FFF,"Square          : New entry");
                                                                  pgPrint(0,32,0x7FFF,"Circle          : Delete entry");
                                                                  pgPrint(0,33,0x7FFF,"Triangle        : Back");
                                                                  addrselection = 0;
                                                                  printaddr(0);
                                                                  pgScreenFlipV();
                                                                  pgFillvram(0);
                                                                  break;
                                                             case PSP_CTRL_CROSS:
                                                                  if (selection3 == 0){
                                                                      getpsprintinput();
                                                                      strcpy(addrname[addrentries],textentered);
                                                                      pgScreenFrame(2,0);
                                                                      pgFillvram(0);
                                                                      pgPrint(0,4,0x7FFF,"Name: ");
                                                                      pgPrint(0,6,0x7FFF,"Address: ");
                                                                      pgPrint(0,8,0x7FFF,"City/State: ");
                                                                      pgPrint(0,10,0x7FFF,"Phone: ");
                                                                      pgPrint(0,12,0x7FFF,"Email: ");
                                                                      pgPrint(6,4,0x1F,addrname[addrentries]);
                                                                      pgPrint(9,6,0x7FFF,addraddr[addrentries]);
                                                                      pgPrint(12,8,0x7FFF,addrcity[addrentries]);
                                                                      pgPrint(7,10,0x7FFF,addrphone[addrentries]);
                                                                      pgPrint(7,12,0x7FFF,addremail[addrentries]);
                                                                  } else if (selection3 == 1){
                                                                      getpsprintinput();
                                                                      strcpy(addraddr[addrentries],textentered);
                                                                      pgScreenFrame(2,0);
                                                                      pgFillvram(0);
                                                                      pgPrint(0,4,0x7FFF,"Name: ");
                                                                      pgPrint(0,6,0x7FFF,"Address: ");
                                                                      pgPrint(0,8,0x7FFF,"City/State: ");
                                                                      pgPrint(0,10,0x7FFF,"Phone: ");
                                                                      pgPrint(0,12,0x7FFF,"Email: ");
                                                                      pgPrint(6,4,0x7FFF,addrname[addrentries]);
                                                                      pgPrint(9,6,0x1F,addraddr[addrentries]);
                                                                      pgPrint(12,8,0x7FFF,addrcity[addrentries]);
                                                                      pgPrint(7,10,0x7FFF,addrphone[addrentries]);
                                                                      pgPrint(7,12,0x7FFF,addremail[addrentries]);
                                                                  } else if (selection3 == 2){
                                                                      getpsprintinput();
                                                                      strcpy(addrcity[addrentries],textentered);
                                                                      pgScreenFrame(2,0);
                                                                      pgFillvram(0);
                                                                      pgPrint(0,4,0x7FFF,"Name: ");
                                                                      pgPrint(0,6,0x7FFF,"Address: ");
                                                                      pgPrint(0,8,0x7FFF,"City/State: ");
                                                                      pgPrint(0,10,0x7FFF,"Phone: ");
                                                                      pgPrint(0,12,0x7FFF,"Email: ");
                                                                      pgPrint(6,4,0x7FFF,addrname[addrentries]);
                                                                      pgPrint(9,6,0x7FFF,addraddr[addrentries]);
                                                                      pgPrint(12,8,0x1F,addrcity[addrentries]);
                                                                      pgPrint(7,10,0x7FFF,addrphone[addrentries]);
                                                                      pgPrint(7,12,0x7FFF,addremail[addrentries]);
                                                                  } else if (selection3 == 3){
                                                                      getpsprintinput();
                                                                      strcpy(addrphone[addrentries],textentered);
                                                                      pgScreenFrame(2,0);
                                                                      pgFillvram(0);
                                                                      pgPrint(0,4,0x7FFF,"Name: ");
                                                                      pgPrint(0,6,0x7FFF,"Address: ");
                                                                      pgPrint(0,8,0x7FFF,"City/State: ");
                                                                      pgPrint(0,10,0x7FFF,"Phone: ");
                                                                      pgPrint(0,12,0x7FFF,"Email: ");
                                                                      pgPrint(6,4,0x7FFF,addrname[addrentries]);
                                                                      pgPrint(9,6,0x7FFF,addraddr[addrentries]);
                                                                      pgPrint(12,8,0x7FFF,addrcity[addrentries]);
                                                                      pgPrint(7,10,0x1F,addrphone[addrentries]);
                                                                      pgPrint(7,12,0x7FFF,addremail[addrentries]);
                                                                  } else if (selection3 == 4){
                                                                      getpsprintinput();
                                                                      strcpy(addremail[addrentries],textentered);
                                                                      pgScreenFrame(2,0);
                                                                      pgFillvram(0);
                                                                      pgPrint(0,4,0x7FFF,"Name: ");
                                                                      pgPrint(0,6,0x7FFF,"Address: ");
                                                                      pgPrint(0,8,0x7FFF,"City/State: ");
                                                                      pgPrint(0,10,0x7FFF,"Phone: ");
                                                                      pgPrint(0,12,0x7FFF,"Email: ");
                                                                      pgPrint(6,4,0x7FFF,addrname[addrentries]);
                                                                      pgPrint(9,6,0x7FFF,addraddr[addrentries]);
                                                                      pgPrint(12,8,0x7FFF,addrcity[addrentries]);
                                                                      pgPrint(7,10,0x7FFF,addrphone[addrentries]);
                                                                      pgPrint(7,12,0x1F,addremail[addrentries]);
                                                                  }
                                                                  pgPrint(0,31,0x7FFF,"D-Pad Up/Down   : Select field");
                                                                  pgPrint(0,32,0x7FFF,"X Button        : Edit field");
                                                                  pgPrint(0,33,0x7FFF,"Select          : Save");
                                                                  pgPrint2(4,0,0x7FFF,"New Address Book Entry");
                                                                  pgScreenFlipV();
                                                                  pgFillvram(0);
                                                                  break;
                                                             case PSP_CTRL_DOWN:
                                                                  if (selection3 == 0){
                                                                      selection3 = 1;
                                                                      pgScreenFlipV();
                                                                      pgPrint(6,4,0x7FFF,addrname[addrentries]);
                                                                      pgPrint(9,6,0x1F,addraddr[addrentries]);
                                                                      pgScreenFlipV();
                                                                  } else if (selection3 == 1){
                                                                      selection3 = 2;
                                                                      pgScreenFlipV();
                                                                      pgPrint(9,6,0x7FFF,addraddr[addrentries]);
                                                                      pgPrint(12,8,0x1F,addrcity[addrentries]);
                                                                      pgScreenFlipV();
                                                                  } else if (selection3 == 2){
                                                                      selection3 = 3;
                                                                      pgScreenFlipV();
                                                                      pgPrint(12,8,0x7FFF,addrcity[addrentries]);
                                                                      pgPrint(7,10,0x1F,addrphone[addrentries]);
                                                                      pgScreenFlipV();
                                                                  } else if (selection3 == 3){
                                                                      selection3 = 4;
                                                                      pgScreenFlipV();
                                                                      pgPrint(7,10,0x7FFF,addrphone[addrentries]);
                                                                      pgPrint(7,12,0x1F,addremail[addrentries]);
                                                                      pgScreenFlipV();
                                                                  }
                                                                  break;
                                                             case PSP_CTRL_UP:
                                                                  if (selection3 == 1){
                                                                      selection3 = 0;
                                                                      pgScreenFlipV();
                                                                      pgPrint(6,4,0x1F,addrname[addrentries]);
                                                                      pgPrint(9,6,0x7FFF,addraddr[addrentries]);
                                                                      pgScreenFlipV();
                                                                  } else if (selection3 == 2){
                                                                      selection3 = 1;
                                                                      pgScreenFlipV();
                                                                      pgPrint(9,6,0x1F,addraddr[addrentries]);
                                                                      pgPrint(12,8,0x7FFF,addrcity[addrentries]);
                                                                      pgScreenFlipV();
                                                                  } else if (selection3 == 3){
                                                                      selection3 = 2;
                                                                      pgScreenFlipV();
                                                                      pgPrint(12,8,0x1F,addrcity[addrentries]);
                                                                      pgPrint(7,10,0x7FFF,addrphone[addrentries]);
                                                                      pgScreenFlipV();
                                                                  } else if (selection3 == 4){
                                                                      selection3 = 3;
                                                                      pgScreenFlipV();
                                                                      pgPrint(7,10,0x1F,addrphone[addrentries]);
                                                                      pgPrint(7,12,0x7FFF,addremail[addrentries]);
                                                                      pgScreenFlipV();
                                                                  }
                                                                  break;
                                                         }
                                                     }
                                                     buttonsold = pad.Buttons;
                                                  }
                                         }
                                     }
                                     buttonsold = pad.Buttons;
                                 }
                             } else if (selection == 3) {
                                 pgFillvram(0);
                                 pgPrint(0,31,0x7FFF,"D-Pad Up/Down   : Choose setting");
                                 pgPrint(0,32,0x7FFF,"D-Pad Left/Right: Change value");
                                 pgPrint(0,33,0x7FFF,"X Button        : Confirm");
                                 pgPrint4 (3,0,0x7FFF,"Settings");
                                 pgPrint2 (0,4,0x7FFF,"Time format      ");
                                 if (formatampm){
                                     pgPrint2 (17,4,0x1F,"12-Hour");
                                 } else {
                                     pgPrint2 (17,4,0x1F,"24-Hour");
                                 }
                                 pgPrint2 (0,6,0x7FFF,"Time zone        ");

                                 if (gmtoffset<0){
                                     sprintf(tempstr2, "GMT %i  ", gmtoffset);
                                 } else {
                                     sprintf(tempstr2, "GMT +%i  ", gmtoffset);
                                 }
                                 pgPrint2 (17,6,0x7FFF,tempstr2);
                                 pgPrint2 (0,8,0x7FFF,"Daylight Savings ");
                                 if (daylightsavings){
                                     pgPrint2 (17,8,0x7FFF,"Enabled ");
                                 } else {
                                     pgPrint2 (17,8,0x7FFF,"Disabled");
                                 }
                                 pgPrint2 (12,11,0x7FFF,"Done");
                                 pgScreenFlipV();
                                 pgFillvram(0);
                                 buttonsold = pad.Buttons;
                                 done2 = 0;
                                 selection2 = 0;
                                 while (done2 == 0){
                                     done2 = 0;
                                     sceCtrlReadBufferPositive(&pad, 1);
                                     if (pad.Buttons != buttonsold) {
                                         switch (pad.Buttons)
    	                                 {
    			                    case PSP_CTRL_DOWN:
    			                         pgScreenFlipV();
                                                 if (selection2 == 0) {
                                                     selection2 = 1;
                                                     if (formatampm){
                                                         pgPrint2 (17,4,0x7FFF,"12-Hour");
                                                     } else {
                                                         pgPrint2 (17,4,0x7FFF,"24-Hour");
                                                     }
                                                     pgPrint2 (17,6,0x1F,tempstr2);
                                                 } else if (selection2 == 1) {
                                                     selection2 = 2;
                                                     pgPrint2 (17,6,0x7FFF,tempstr2);
                                                     if (daylightsavings){
                                                         pgPrint2 (17,8,0x1F,"Enabled ");
                                                     } else {
                                                         pgPrint2 (17,8,0x1F,"Disabled");
                                                     }
                                                 } else if (selection2 == 2) {
                                                     selection2 = 3;
                                                     if (daylightsavings){
                                                         pgPrint2 (17,8,0x7FFF,"Enabled");
                                                     } else {
                                                         pgPrint2 (17,8,0x7FFF,"Disabled");
                                                     }
                                                     pgPrint2 (12,11,0x1F,"Done");
                                                 }
                                                 pgScreenFlipV();
                                                 break;
                                            case PSP_CTRL_UP:
                                                 pgScreenFlipV();
                                                 if (selection2 == 2) {
                                                     selection2 = 1;
                                                     if (daylightsavings){
                                                         pgPrint2 (17,8,0x7FFF,"Enabled ");
                                                     } else {
                                                         pgPrint2 (17,8,0x7FFF,"Disabled");
                                                     }
                                                     pgPrint2 (17,6,0x1F,tempstr2);
                                                 } else if (selection2 == 3) {
                                                     selection2 = 2;
                                                     pgPrint2 (12,11,0x7FFF,"Done");
                                                     if (daylightsavings){
                                                         pgPrint2 (17,8,0x1F,"Enabled ");
                                                     } else {
                                                         pgPrint2 (17,8,0x1F,"Disabled");
                                                     }
                                                 } else if (selection2 == 1) {
                                                     selection2 = 0;
                                                     if (formatampm){
                                                         pgPrint2 (17,4,0x1F,"12-Hour");
                                                     } else {
                                                         pgPrint2 (17,4,0x1F,"24-Hour");
                                                     }
                                                     pgPrint2 (17,6,0x7FFF,tempstr2);
                                                 }
                                                 pgScreenFlipV();
                                                 break;
                                            case PSP_CTRL_LEFT:
                                                 pgScreenFlipV();
                                                 if (selection2 == 0){
                                                     if (formatampm){
                                                         formatampm = 0;
                                                         pgPrint2 (17,4,0x1F,"24-Hour");
                                                     } else {
                                                         formatampm = 1;
                                                         pgPrint2 (17,4,0x1F,"12-Hour");
                                                     }
                                                 } else if (selection2 == 1) {
                                                     gmtoffset-=1;
                                                     if (gmtoffset<-15) { gmtoffset = -15; }
                                                     if (gmtoffset<0){
                                                         sprintf(tempstr2, "GMT %i", gmtoffset);
                                                     } else {
                                                         sprintf(tempstr2, "GMT +%i", gmtoffset);
                                                     }
                                                     pgPrint2 (17,6,0x1F,tempstr2);
                                                 } else if (selection2 == 2) {
                                                     if (daylightsavings){
                                                         daylightsavings = 0;
                                                         pgPrint2 (17,8,0x1F,"Disabled");
                                                     } else {
                                                         daylightsavings = 1;
                                                         pgPrint2 (17,8,0x1F,"Enabled ");
                                                     }
                                                 }
                                                 pgScreenFlipV();
                                                 break;
                                             case PSP_CTRL_RIGHT:
                                                 pgScreenFlipV();
                                                 if (selection2 == 0){
                                                     if (formatampm){
                                                         formatampm = 0;
                                                         pgPrint2 (17,4,0x1F,"24-Hour");
                                                     } else {
                                                         formatampm = 1;
                                                         pgPrint2 (17,4,0x1F,"12-Hour");
                                                     }
                                                 } else if (selection2 == 1) {
                                                     gmtoffset+=1;
                                                     if (gmtoffset>15) { gmtoffset = 15; }
                                                     if (gmtoffset<0){
                                                         sprintf(tempstr2, "GMT %i", gmtoffset);
                                                     } else {
                                                         sprintf(tempstr2, "GMT +%i", gmtoffset);
                                                     }
                                                     pgPrint2 (17,6,0x1F,tempstr2);
                                                 } else if (selection2 == 2) {
                                                     if (daylightsavings){
                                                         daylightsavings = 0;
                                                         pgPrint2 (17,8,0x1F,"Disabled");
                                                     } else {
                                                         daylightsavings = 1;
                                                         pgPrint2 (17,8,0x1F,"Enabled ");
                                                     }
                                                 }
                                                 pgScreenFlipV();
                                                 break;
                                             case PSP_CTRL_CROSS:
                                                 if (selection2 == 3){
                                                     sprintf(tempstr, "%i,%i,%i", formatampm, gmtoffset, daylightsavings);
                                                     sprintf(tempstr2, "%sconfig",basepath);
                                                     fd = sceIoOpen(tempstr2, PSP_O_WRONLY, 0777);
                                                     sceIoWrite(fd, tempstr, strlen(tempstr));
                                                     sceIoClose(fd);
                                                     done2 = 1;
                                                     refreshmain = 3;
                                                 }
                                                 break;
                                         }
                                     }
                                 buttonsold = pad.Buttons;
                                 }
                                 break;
                             }
                             break;
	        }
             }
             buttonsold = pad.Buttons;
         }
    return 0;
}
