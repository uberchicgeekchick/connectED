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


#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "prefix.h"
#include "main.h"
#include "calltip.h"
#include "tab.h"


#define CB_ITEM_TYPE_CLASS 1
#define CB_ITEM_TYPE_CLASS_METHOD 2
#define CB_ITEM_TYPE_FUNCTION 3

enum {
    NAME_COLUMN,
    LINE_NUMBER_COLUMN,
    FILENAME_COLUMN,
    TYPE_COLUMN,
    ID_COLUMN,
    N_COLUMNS
};


extern gboolean DEBUG_MODE;

extern GIOChannel* inter_connectED_io;
extern guint inter_connectED_event_id;

extern guint idle_id;



void force_config_folder(void);

void main_window_create(void);

GtkWindow *main_window_get_window(void);

Editor *main_window_get_current_editor(void);
void main_window_set_current_editor(Editor *editor);

GtkTreeStore *main_window_get_class_browser_tree_store(void);
GtkTreeView *main_window_get_class_browser_tree_view(void);
GtkScrolledWindow *main_window_get_class_browser_scrolled_window(void);
GtkTreeSelection *main_window_get_class_browser_tree_select(void);
GtkClipboard *main_window_get_clipboard(void);
GtkListStore *main_window_get_lint_store(void);
GtkTreeView *main_window_get_lint_view(void);

GtkHPaned *main_window_get_horizontal_pane(void);
GtkLabel *main_window_get_tree_view_label(void);
GtkNotebook *main_window_get_notebook_editor(void);

void main_window_open_command_line_files(char **argv, gint argc);
void update_app_title(void);
void main_window_create_unix_socket(void);
void main_window_pass_command_line_files(char **argv);
gboolean channel_pass_filename_callback(GIOChannel *source, GIOCondition condition, gpointer data );
GString *get_differing_part(GSList *filenames, gchar *file_requested);
void main_window_add_to_reopen_menu(gchar *full_filename);
void plugin_exec(gint plugin_num);
void create_untitled_if_empty(void);

void syntax_check(GtkWidget *widget);
void syntax_check_clear(GtkWidget *widget);

#endif
