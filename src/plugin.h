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


#ifndef PLUGIN_H
#define PLUGIN_H

#include "main.h"

void plugin_setup_menu(void);
void plugin_exec(gint plugin_num);

#define connectED_PLUGIN_TYPE_ERROR -1
#define connectED_PLUGIN_TYPE_UNKNOWN 0
#define connectED_PLUGIN_TYPE_NOINPUT 1
#define connectED_PLUGIN_TYPE_SELECTION 2
#define connectED_PLUGIN_TYPE_FILENAME 3
#define connectED_PLUGIN_TYPE_DEBUG 4

typedef struct
{
	guint type;
	GtkWidget *menu_item;
	GString *filename;
	gchar *name;
} Plugin;

#endif
