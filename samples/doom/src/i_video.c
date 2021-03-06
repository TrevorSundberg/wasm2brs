// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log:$
//
//-----------------------------------------------------------------------------
#define PROFILING_FRAMES 0

static const char
rcsid[] = "$Id: i_x.c,v 1.6 1997/02/03 22:45:10 b1 Exp $";

#include <stdlib.h>

#include "m_swap.h"
#include "doomstat.h"
#include "i_system.h"
#include "v_video.h"
#include "m_argv.h"
#include "d_main.h"

#include "doomdef.h"

#include <roku.h>

unsigned char pixels[SCREENWIDTH * SCREENHEIGHT * 4];
#define SCREENPITCH (SCREENWIDTH * 4)

// Fake mouse handling.
boolean		grabMouse;

//
//  Translates the key 
//
int xlatekey(unsigned int key)
{
    switch(key)
    {
      case ROKU_LEFT:           return KEY_LEFTARROW;
      case ROKU_RIGHT:          return KEY_RIGHTARROW;
      case ROKU_DOWN:           return KEY_DOWNARROW;
      case ROKU_UP:             return KEY_UPARROW;
      case ROKU_BACK:           return KEY_ESCAPE;
      case ROKU_OK:             return KEY_ENTER;	
      case ROKU_OPTIONS:        return KEY_TAB;	
      case ROKU_INSTANTREPLAY:  return KEY_SPACE;	
      case ROKU_FASTFORWARD:    return KEY_EQUALS;
      case ROKU_REWIND:         return KEY_MINUS;	
      case ROKU_PLAY:           return KEY_RCTRL;
    }
    // Missing: KEY_RSHIFT, KEY_RALT, KEY_PAUSE, KEY_F1 - KEY_F12
}

void I_ShutdownGraphics(void)
{
    exit(0);
}



//
// I_StartFrame
//
void I_StartFrame (void)
{
    // er?

}

void I_GetEvent(unsigned int ev)
{
    if (ev >= 1000) {
        return;
    }

    event_t event;
	event.type = (ev < 100) ? ev_keydown : ev_keyup;
	event.data1 = xlatekey((ev < 100) ? ev : ev - 100);
	D_PostEvent(&event);
}

//
// I_StartTic
//
void I_StartTic (void)
{
#if PROFILING_FRAMES == 0
    for (;;) {
        unsigned int button = roku_poll_button();
        if (button == ROKU_INVALID) {
            break;
        }
	    I_GetEvent(button);
    }
#else
	I_GetEvent(ROKU_OK);
#endif
}


//
// I_UpdateNoBlit
//
void I_UpdateNoBlit (void)
{
    // what is this?
}

//
// I_FinishUpdate
//
void I_FinishUpdate (void)
{

    static int	lasttic;
    int		tics;
    int		i;

    // draws little dots on the bottom of the screen
    if (devparm)
    {

	i = I_GetTime();
	tics = i - lasttic;
	lasttic = i;
	if (tics > 20) tics = 20;

	for (i=0 ; i<tics*2 ; i+=2)
	    screens[0][ (SCREENHEIGHT-1)*SCREENWIDTH + i] = 0xff;
	for ( ; i<20*2 ; i+=2)
	    screens[0][ (SCREENHEIGHT-1)*SCREENWIDTH + i] = 0x0;
    }

    roku_draw_surface(screens[0]);

#if PROFILING_FRAMES > 0
    static int counter = 0;
    ++counter;
    if (counter == PROFILING_FRAMES) {
        exit(0);
    }
#endif
}


//
// I_ReadScreen
//
void I_ReadScreen (byte* scr)
{
    memcpy (scr, screens[0], SCREENWIDTH*SCREENHEIGHT);
}

struct color {
	unsigned char b;
	unsigned char g;
	unsigned char r;
	unsigned char unused;
};

//
// I_SetPalette
//
void I_SetPalette (byte* palette)
{
    int i;
    struct color colors[256];

    for ( i=0; i<256; ++i ) {
	colors[i].r = gammatable[usegamma][*palette++];
	colors[i].g = gammatable[usegamma][*palette++];
	colors[i].b = gammatable[usegamma][*palette++];
	colors[i].unused = 0;
    }

    static int created = 0;
    if (!created) {
        roku_create_surface(8, SCREENWIDTH, SCREENHEIGHT);
        created = 1;
    }

    roku_set_surface_colors(colors);
}


void I_InitGraphics(void)
{
}
