#pragma once
#define LONG_VERSION_IMPL(a,b,c,d) #a "." #b "." #c "." #d
#define LONG_VERSION(arg) LONG_VERSION_IMPL(arg)

#define NUM_VERSION_IMPL(a,b,c,d) (((a) << 12) + ((b) << 8) + ((c) << 4) + (d))
#define NUM_VERSION(arg) NUM_VERSION_IMPL(arg)

#define STRINGIFY_IMPL(arg) #arg
#define STRINGIFY(arg) STRINGIFY_IMPL(arg)

//////////////////////////////////////////////////////////////////////////////////////////////////////////

#define OL_NAME_RES			Open Legends
#define QD_NAME_RES			Quest Designer

#define OL_NAME				STRINGIFY(OL_NAME_RES)
#define QD_NAME				STRINGIFY(QD_NAME_RES)
#define KERNEL_NAME			OL_NAME"'s Kernel"
#define D3D8_NAME			"DirectX 8.0 2D Graphics Engine plugin"
#define D3D9_NAME			"DirectX 9.0c 2D Graphics Engine plugin"
#define OGL_NAME			"OpenGL 2D Graphics Engine plugin"
#define DEBUGGER_NAME		OL_NAME"'s Debugger"

#define OL_INTNAME			"OpenLegends"
#define QD_INTNAME			"QuestDesigner"
#define KERNEL_INTNAME		"Kernel"
#define D3D8_INTNAME		"GraphicsD3D8"
#define D3D9_INTNAME		"GraphicsD3D9"
#define OGL_INTNAME			"GraphicsOGL"
#define DEBUGGER_INTNAME	"Debugger"

#define OL_EMAIL			"kronuz@users.sourceforge.net"
#define OL_WEBSITE			"http://www.openlegends.com/"
#define OL_DONATION_URL		"https://www.paypal.com/xclick/business=kronuz%40hotmail.com"

#define OL_COMPANY			"by Kronuz"

#define OL_DESC				OL_NAME " is a powerful 2D game engine. " OL_WEBSITE
#define QD_DESC				QD_NAME " is a tool to create quests for " OL_NAME ". " OL_WEBSITE
#define KERNEL_DESC			KERNEL_NAME " designed for " OL_NAME ". " OL_WEBSITE
#define D3D8_DESC			D3D8_NAME " designed for " OL_NAME ". " OL_WEBSITE
#define D3D9_DESC			D3D9_NAME " designed for " OL_NAME ". " OL_WEBSITE
#define OGL_DESC			OGL_NAME " designed for " OL_NAME ". " OL_WEBSITE

#define OL_COPYRIGHT_ALT	"Copyright © 2001-2007. Open Legends Project"
#define OL_COPYRIGHT		"Copyright © 2003-2007. Germán Méndez Bravo (Kronuz). All rights reserved."

// Interface versions:
#define GAME_INTVER			NUM_VERSION_IMPL(1, 5, 0, 0)
#define GRAPHICS_INTVER		NUM_VERSION_IMPL(1, 4, 1, 0)

#define MAIN_FILEVERSION		0, 0, 8, 6
#define OL_FILEVERSION			0, 2, 0, 0
#define QD_FILEVERSION			0, 8, 6, 8
#define KERNEL_FILEVERSION		1, 5, 0, 0
#define D3D8_FILEVERSION		1, 6, 0, 0
#define D3D9_FILEVERSION		1, 4, 0, 0
#define OGL_FILEVERSION			1, 0, 0, 0
#define DEBUGGER_FILEVERSION	1, 0, 0, 0

#define MAIN_VERSION		"0.8.7 (beta)"
#define OL_VERSION			"0.2.0 (beta)"
#define QD_VERSION			"0.8.7 (beta)"
#define KERNEL_VERSION		"1.5 (beta)"
#define D3D8_VERSION		"1.4"
#define D3D9_VERSION		"1.4"
#define OGL_VERSION			"1.0"
#define DEBUGGER_VERSION	"1.0 (alfa)"

#define OL_FULLNAME			OL_NAME ", v." OL_VERSION
#define QD_FULLNAME			QD_NAME ", v." QD_VERSION
#define KERNEL_FULLNAME		KERNEL_NAME ", v." KERNEL_VERSION
#define D3D8_FULLNAME		D3D8_NAME ", v." D3D8_VERSION
#define D3D9_FULLNAME		D3D9_NAME ", v." D3D9_VERSION
#define OGL_FULLNAME		OGL_NAME ", v." OGL_VERSION
#define DEBUGGER_FULLNAME	DEBUGGER_NAME ", v." DEBUGGER_VERSION

#define QD_MSG_TITLE		QD_NAME

//////////////////////////////////////////////////////////////////////////////////////////////////////////

#define OL_LICENSE_NAME	"GNU General Public License"
#define OL_LICENSE _T("\
Open Legends Project\r\n\
Copyright (C) 2001-2007\r\n\
\r\n\
This program is free software; you can redistribute it and/or modify it \
under the terms of the GNU General Public License as published by the Free \
Software Foundation; either version 2 of the License, or (at your option) \
any later version.\r\n\
\r\n\
This program is distributed in the hope that it will be useful, \
but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY \
or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more \
details.\r\n\
\r\n\
You should have received a copy of the GNU General Public License along with this \
program; if not, write to the Free Software Foundation, Inc., 59 Temple Place, \
Suite 330, Boston, MA 02111-1307 USA")


#define OL_ACK _T("\
Code based in the conceptual design and original idea by Greg Denness.\r\n\
Portions of this software use:\r\n\
    CodeMax, Copyright 1997-2000 Barry Allyn,\r\n\
    CodeSense, Copyright 2000-2003 Nathan Lewis, and\r\n\
    Small Compiler, Copyright (c) 1997-2002 ITB CompuPhase.\r\n\
    All rights reserved.\r\n\
\r\n\
All trademarks used are properties of their respective owners.")

#define OL_THANKS _T("\
I want to give a very special thank you to those who have donated money to the cause:\r\n\
      So far, only Tanner Christensen, thank you :)\r\n\
\r\n\
Also, I want to thank all people who helped me and collaborated in one way or the other and helped OpenLegends to become what it is today:\r\n\
\r\n\
      + GD for his magnificent idea and original design;\r\n\
      + Strider for his support and all the new Sprite Sheets \r\n\
          he is working on;\r\n\
      + System Failure for the OpenGL plugin he is working on;\r\n\
      + Fenris (a.k.a. Sk8erHacker) for testing and \r\n\
          excellent support;\r\n\
      + Hylian for his support and the Open Legends \r\n\
          domain name;\r\n\
      + Lukex for maintaining the official website and \r\n\
          starting a very useful help file,\r\n\
      + Menne for his great Z3C editor and its nice \r\n\
          source code;\r\n\
      + LittleBuddy for his support as a consultant and \r\n\
          his scripting expertise;\r\n\
      + KingOfHeart, for testing and support;\r\n\
      + Alias Jargon;\r\n\
      + HocusPocus;\r\n\
      + GodGinrai;\r\n\
      + and last, but not least, the guys at GU who have \r\n\
          a great website that provided the project with a nice\r\n\
          and useful forum (http://www.thegaminguniverse.com)\r\n\
\r\n\
If you feel I'm missing sombody, please tell me so I can add her/him/you to the list.")
