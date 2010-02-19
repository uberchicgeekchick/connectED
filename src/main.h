/* This file is part of http://connectED/, a GNOME2 PHP Editor.
 
   Copyright (C) 2008 Kaity G. B.
 uberChick@uberChicGeekChick.Com
	  
   For more information or to find the latest release, visit our 
   website at http://uberchicgeekchick.com/
 
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.
 
   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
 
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307, USA.
 
   The GNU General Public License is contained in the file COPYING.*/


#ifndef MAIN_H
#define MAIN_H

#include <gnome.h>
#include <glib.h>
#include <gtkscintilla.h>
#include <libgnome/gnome-i18n.h>

#define connectED_PIXMAP_ICON "connectED.svg"

#ifndef	uber_free
#	define		uber_free(mem)		{ g_free(mem); mem=NULL; }
#endif

#ifndef	G_STR_EMPTY
#	define		G_STR_EMPTY(str)	( ((str)==NULL) || (str)[0]=='\0' )
#endif

#ifndef	G_STR_N_EMPTY
#	define		G_STR_N_EMPTY(str)	( ((str)!=NULL) && (str)[0]!='\0' )
#endif

extern GnomeProgram* connectED_program;
void connectED_deinit(void);
gboolean add (gpointer bla);

#endif
