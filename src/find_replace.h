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


#ifndef FIND_REPLACE_H
#define FIND_REPLACE_H

#include "main.h"

typedef struct
{
	GtkWidget *window1;
	GtkWidget *vbox1;
	GtkWidget *frame1;
	GtkWidget *entry1;
	GtkWidget *label1;
	GtkWidget *alignment1;
	GtkWidget *frame2;
	GtkWidget *vbox2;
	GtkWidget *hbox1;
	GtkWidget *frame3;
	GtkWidget *vbox3;
	GtkWidget *radiobutton1;
	GSList *radiobutton1_group;
	GtkWidget *radiobutton4;
	GtkWidget *label3;
	GtkWidget *frame4;
	GtkWidget *vbox4;
	GtkWidget *radiobutton2;
	GSList *radiobutton2_group;
	GtkWidget *radiobutton3;
	GtkWidget *label4;
	GtkWidget *frame5;
	GtkWidget *vbox5;
	GtkWidget *radiobutton5;
	GSList *radiobutton5_group;
	GtkWidget *radiobutton6;
	GtkWidget *label5;
	GtkWidget *hbox3;
	GtkWidget *checkbutton1;
	GtkWidget *checkbutton2;
	GtkWidget *label2;
	GtkWidget *hseparator1;
	GtkWidget *hbox2;
	GtkWidget *button4;
	GtkWidget *button5;
}
FindDialog;

extern FindDialog find_dialog;

typedef struct
{
	GtkWidget *window2;
	GtkWidget *vbox10;
	GtkWidget *frame10;
	GtkWidget *alignment1;
	GtkWidget *entry1;
	GtkWidget *label10;
	GtkWidget *frame15;
	GtkWidget *alignment2;
	GtkWidget *entry2;
	GtkWidget *label15;
	GtkWidget *frame11;
	GtkWidget *vbox14;
	GtkWidget *hbox9;
	GtkWidget *frame16;
	GtkWidget *vbox15;
	GtkWidget *radiobutton17;
	GSList *radiobutton17_group;
	GtkWidget *radiobutton18;
	GSList *radiobutton18_group;
	GtkWidget *label16;
	GtkWidget *frame17;
	GtkWidget *vbox16;
	GtkWidget *radiobutton19;
	GSList *radiobutton19_group;
	GtkWidget *radiobutton20;
	GSList *radiobutton20_group;
	GtkWidget *label17;
	GtkWidget *frame18;
	GtkWidget *vbox17;
	GtkWidget *radiobutton21;
	GSList *radiobutton21_group;
	GtkWidget *radiobutton22;
	GSList *radiobutton22_group;
	GtkWidget *label18;
	GtkWidget *hbox10;
	GtkWidget *checkbutton6;
	GtkWidget *checkbutton7;
	GtkWidget *checkbutton8;
	GtkWidget *label14;
	GtkWidget *hseparator2;
	GtkWidget *hbox8;
	GtkWidget *button7;
	GtkWidget *button8;
	GtkWidget *button9;
}
ReplaceDialog;

extern ReplaceDialog replace_dialog;



void find_create(void);
void replace_create(void);
void replace_clicked(GtkButton *button, gpointer data);


#endif
