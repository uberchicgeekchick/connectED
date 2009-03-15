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


#ifndef MAIN_WINDOW_CALLBACKS_H
#define MAIN_WINDOW_CALLBACKS_H

#include <libgtkhtml/gtkhtml.h>
#include <libgtkhtml/layout/htmlbox.h>
#include <libgtkhtml/layout/htmlboxtext.h>
#include "main.h"
#include "tab.h"
#include "main_window.h"
#include "preferences.h"
#include "syntax_check.h"
#include "classbrowser.h"

GString *get_folder(GString *filename);
void main_window_destroy_event(GtkWidget *widget, gpointer data);
gboolean main_window_delete_event(GtkWidget *widget,
                                  GdkEvent *event, gpointer user_data);
gint main_window_key_press_event(GtkWidget   *widget,
                                 GdkEventKey *event,gpointer user_data);
void on_new1_activate(GtkWidget *widget);
void open_file_ok(GtkFileChooser *file_selection);
void reopen_recent(GtkWidget *widget, gpointer data);
void on_open1_activate(GtkWidget *widget);
void on_openselected1_activate(GtkWidget *widget);
void save_file_as_confirm_overwrite(gint reply,gpointer filename);
void on_save1_activate(GtkWidget *widget);
void on_saveall1_activate(GtkWidget *widget);
void on_rename1_activate(GtkWidget *widget);
void get_first_tab(void);
void close_page(Editor *editor);
gboolean try_save_page(Editor *editor, gboolean close_if_can);
gboolean try_close_page(Editor *editor);
void on_close1_activate(GtkWidget *widget);
void on_quit1_activate(GtkWidget *widget);
void on_cut1_activate(GtkWidget *widget);
void on_copy1_activate(GtkWidget *widget);
void on_paste1_activate(GtkWidget *widget);
void on_selectall1_activate(GtkWidget *widget);
void on_find1_activate(GtkWidget *widget);
void on_replace1_activate(GtkWidget *widget);
void on_properties1_activate(GtkWidget *widget);
void on_undo1_activate(GtkWidget *widget);
void on_redo1_activate(GtkWidget *widget);
void on_preferences1_activate(GtkWidget *widget);
void context_help(GtkWidget *widget);
void on_about1_activate(GtkWidget *widget);
void force_php(GtkWidget *widget);
void force_css(GtkWidget *widget);
void force_sql(GtkWidget *widget);
void force_cxx(GtkWidget *widget);
void force_perl(GtkWidget *widget);
void force_python(GtkWidget *widget);

void on_notebook_switch_page (GtkNotebook *notebook, GtkNotebookPage *page,
                              gint page_num, gpointer user_data);
void inc_search_typed (GtkEntry *entry, const gchar *text, gint length,
                       gint *position, gpointer data);
gboolean inc_search_key_release_event(GtkWidget *widget,GdkEventKey *event,gpointer user_data);
void inc_search_activate(GtkEntry *entry,gpointer user_data);
gboolean is_valid_digits_only(gchar *text);
void goto_line_activate(GtkEntry *entry,gpointer user_data);
void move_block(gint indentation_size);
void block_indent(GtkWidget *widget);
void block_unindent(GtkWidget *widget);
void syntax_check(GtkWidget *widget);
void syntax_check_clear(GtkWidget *widget);
void lint_row_activated (GtkTreeSelection *selection, gpointer data);
void goto_line_int(gint line);
void goto_line(gchar *text);
gboolean on_notebook_focus_tab(GtkNotebook *notebook,
                               GtkNotebookTab arg1, gpointer user_data);
void on_reload1_activate(GtkWidget *widget);
gint treeview_double_click( GtkWidget *widget, GdkEventButton *event );
gint treeview_click_release( GtkWidget *widget, GdkEventButton *event );
void main_window_resize(GtkWidget *widget, GtkAllocation *allocation, gpointer user_data);
void main_window_state_changed(GtkWidget *widget, GdkEventWindowState *event, gpointer user_data);
gboolean classbrowser_accept_size(GtkPaned *paned, gpointer user_data);

void keyboard_macro_startstop(GtkWidget *widget);
void keyboard_macro_playback(GtkWidget *widget);
void classbrowser_show(void);
void classbrowser_hide(void);
void classbrowser_show_hide(GtkWidget *widget);
void run_plugin(GtkWidget *widget, gpointer data);
void close_all_tabs(void);
void session_reopen(void);
void session_save(void);

void on_save_as1_activate(void);
void save_file_as_ok( GtkFileChooser *file_selection_box );
GtkWidget *create_file_selection_dialog_and_select_files( const gchar *title, const char *called_from_method, gboolean for_saving );

#endif
