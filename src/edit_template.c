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

#include "config.h"
#include "main_window.h"
#include "templates.h"
#include "edit_template.h"

EditTemplateDialog edit_template_dialog;

void create_edit_template_dialog (void)
{
	edit_template_dialog.window1 = gtk_dialog_new_with_buttons(_("Add/Edit Template"),
		main_window_get_window(), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,NULL);

	edit_template_dialog.hbox2 = gtk_hbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(edit_template_dialog.window1)->vbox),edit_template_dialog.hbox2);

	gtk_container_set_border_width (GTK_CONTAINER (edit_template_dialog.hbox2), 8);

  	edit_template_dialog.label2 = gtk_label_new (_("Name/Shortcut:"));
  	gtk_box_pack_start (GTK_BOX (edit_template_dialog.hbox2), edit_template_dialog.label2, FALSE, FALSE, 0);

	edit_template_dialog.entry1 = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (edit_template_dialog.hbox2), edit_template_dialog.entry1, FALSE, FALSE, 8);

	edit_template_dialog.hbox3 = gtk_hbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(edit_template_dialog.window1)->vbox),edit_template_dialog.hbox3);
	gtk_container_set_border_width (GTK_CONTAINER (edit_template_dialog.hbox3), 8);

	edit_template_dialog.label3 = gtk_label_new (_("Code:"));
	gtk_box_pack_start (GTK_BOX (edit_template_dialog.hbox3), edit_template_dialog.label3, FALSE, FALSE, 0);

  	edit_template_dialog.scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
  	gtk_box_pack_start (GTK_BOX (edit_template_dialog.hbox3), edit_template_dialog.scrolledwindow1, TRUE, TRUE, 0);
  	gtk_container_set_border_width (GTK_CONTAINER (edit_template_dialog.scrolledwindow1), 8);

  	edit_template_dialog.textview1 = gtk_text_view_new ();
  	gtk_container_add (GTK_CONTAINER (edit_template_dialog.scrolledwindow1), edit_template_dialog.textview1);

	gtk_widget_show_all(edit_template_dialog.window1);
}
