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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <libgnomevfs/gnome-vfs.h>
#include <glib.h>

#include "main_window.h"
#include "main_window.typedef.h"
#include "main_window_callbacks.h"
#include "find_replace.h"
#include "preferences_dialog.h"
#include "tab.h"
#include "templates.h"

static gboolean can_all_tabs_be_saved(void);
static gchar *get_gnome_vfs_dirname(gchar *filename);
static void on_reload_confirm(gint reply,gpointer filename);
static void rename_file(GString *newfilename);
/*static void rename_file_overwrite_confirm(gint reply,gpointer filename);*/
static void rename_file_ok(GtkFileChooser *file_selection);
/*static void goto_line_activate(GtkEntry *entry,gpointer user_data);
static void goto_line(gchar *text);
static void goto_line_int(gint line);
static gboolean is_valid_digits_only(gchar *text);
static void inc_search_activate(GtkEntry *entry,gpointer user_data);
static gboolean inc_search_key_release_event(GtkWidget *widget,GdkEventKey *event,gpointer user_data);
static void inc_search_typed (GtkEntry *entry, const gchar *text, gint length, gint *position, gpointer data);
static gboolean on_notebook_focus_tab(GtkNotebook *notebook, GtkNotebookTab arg1, gpointer user_data);
static void on_notebook_switch_page (GtkNotebook *notebook, GtkNotebookPage *page, gint page_num, gpointer user_data);
static void on_about1_activate(GtkWidget *widget);
static void context_help(GtkWidget *widget);
static void on_preferences1_activate(GtkWidget *widget);
static void keyboard_macro_playback(GtkWidget *widget);
static void keyboard_macro_startstop(GtkWidget *widget);
static void on_redo1_activate(GtkWidget *widget);
static void on_undo1_activate(GtkWidget *widget);
static void on_properties1_activate(GtkWidget *widget);
static void on_replace1_activate(GtkWidget *widget);
static void on_find1_activate(GtkWidget *widget);
static void on_selectall1_activate(GtkWidget *widget);
static void on_paste1_activate(GtkWidget *widget);
static void on_paste_got_from_cliboard(GtkClipboard *clipboard, const gchar *text, gpointer data);
static void on_copy1_activate(GtkWidget *widget);
static gchar *gtkhtml2_selection_get_text (HtmlView *view);
static void on_cut1_activate(GtkWidget *widget);
static void on_quit1_activate(GtkWidget *widget);
static void on_close1_activate(GtkWidget *widget);
static gboolean try_close_page(Editor *editor);
static gboolean try_save_page(Editor *editor, gboolean close_if_can);
static void close_page(Editor *editor);*/
static void set_active_tab(int page_num);
gchar *gtkhtml2_selection_get_text (HtmlView *view);
static void on_paste_got_from_cliboard(GtkClipboard *clipboard, const gchar *text, gpointer data);
/*static GtkWidget *create_file_selection_dialog_and_select_files( const gchar *title, const char *called_from_method, gboolean for_saving );
static void on_rename1_activate(GtkWidget *widget);
static void rename_file_ok(GtkFileChooser *file_selection);
static void rename_file(GString *newfilename);*/

gboolean is_app_closing = FALSE;
gint classbrowser_hidden_position;

// session_save relies on the fact that all tabs can be closed without 
// prompting, they should already be saved.  Also, the title won't be set
// afterwards.
void session_save(void)
{
	GSList *walk;
	Editor *editor;
	Editor *current_focus_editor;
	GString *session_file;
	FILE *fp;

	session_file = g_string_new( g_get_home_dir());
	session_file = g_string_append(session_file, "/.connectED/session");
	
	unlink(session_file->str);
	
	if (preferences.save_session && (g_slist_length(editors) > 0)) {
		current_focus_editor = main_window_get_current_editor();
	
		fp = fopen(session_file->str, "w");
  		if (!fp) {	
			g_print(_("ERROR: cannot save session to %s\n"), session_file->str);
			return;
		}

		for(walk = editors; walk!= NULL; walk = g_slist_next(walk)) {
			editor = walk->data;
			if (editor) {
				if (!editor->is_untitled) {
					if (editor == current_focus_editor) {
						fputs("*", fp);
					}
					if (GTK_IS_SCINTILLA(editor->scintilla)) {
						fputs(editor->filename->str, fp);
						fputs("\n", fp);
					}
					else { // it's a help page
						fputs("phphelp:", fp);
						fputs(editor->help_function, fp);
						fputs("\n", fp);
					}
				}
			}
		}
		
		fclose(fp);
	}
}


void session_reopen(void)
{
	GString *session_file;
	FILE *fp;
	char buf[16384];
	char *filename;
	int focus_tab=-1;
	GString *target;
	Editor *editor=main_window_get_current_editor();
	GtkNotebook *notebook_editor=main_window_get_notebook_editor();

	session_file = g_string_new( g_get_home_dir());
	session_file = g_string_append(session_file, "/.connectED/session");
	
	if (g_file_exists(session_file->str)) {
		fp = fopen(session_file->str, "r");
  		if (!fp) {	
			g_print(_("ERROR: cannot open session file (%s)\n"), session_file->str);
			return;
		}

		while (fgets(buf, sizeof(buf), fp)) {
			gboolean focus_this_one=FALSE;
			/* buf contains possibly:
				file:///blah\n
				*file:///blah\n
				phphelp:function\n
				*phphelp:function\n
			*/
			
			filename = buf;
			str_replace(filename, 10, 0);
			if (buf[0]=='*') {
				filename++;
				focus_this_one=TRUE;
			}

			if(!strstr(filename, "phphelp:")){
				switch_to_file_or_open(filename, 0);
			}else{
				filename += 8;
				target = g_string_new(filename);
				tab_create_new(TAB_HELP, target);
				g_string_free(target, TRUE);
			}
			if(editor && focus_this_one)
				focus_tab=gtk_notebook_page_num(notebook_editor, editor->help_scrolled_window);
			
		}
		
		fclose(fp);
		gtk_notebook_set_current_page(notebook_editor, focus_tab);
		
		unlink(session_file->str);
	}
}


/* Actual action functions */


// This procedure relies on the fact that all tabs will be closed without prompting
// for whether they need saving beforehand.  If in doubt, call can_all_tabs_be_saved
// and pay attention to the return value.
void close_all_tabs(void)
{
	GSList *walk;
	Editor *editor;

	is_app_closing = TRUE;
	
	while (g_slist_length(editors)>0) {
		walk = editors;
		editor = walk->data;
		if (editor) {
			close_page(editor);
			editors = g_slist_remove(editors, editor);
		}
	}
	editors = NULL;
	main_window_set_current_editor(NULL);
	is_app_closing = FALSE;
	return;
}


// Returns true if all tabs are either saved or closed
static gboolean can_all_tabs_be_saved(void)
{
	GSList *walk;
	Editor *editor;
	gboolean saved;

	is_app_closing = TRUE;
	for(walk = editors; walk!= NULL; walk = g_slist_next(walk)) {
		if (g_slist_length(editors) == 0) {
			break;
		}
		editor = walk->data;
		if (editor && (editor->scintilla || editor->help_scrolled_window)) {
			if (!editor->saved && editor->type!=TAB_HELP) {
				saved = try_save_page(editor, FALSE);
				if (saved==FALSE) {
					is_app_closing = FALSE;
					return FALSE;
				}
			}
		}
	}
	is_app_closing = FALSE;
	return TRUE;
}


gboolean main_window_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data){
	if(!can_all_tabs_be_saved()){
		update_app_title();
		return TRUE;
	}
	return FALSE;
}


void main_window_resize(GtkWidget *widget, GtkAllocation *allocation, gpointer user_data)
{
	//main_window_size_save_details(allocation->x, allocation->y, allocation->width, allocation->height);
	main_window_size_save_details();
}

void main_window_state_changed(GtkWidget *widget, GdkEventWindowState *event, gpointer user_data)
{
	gboolean maximized = GDK_WINDOW_STATE_MAXIMIZED && event->new_window_state;
	gnome_config_set_bool("connectED/main_window/maximized", maximized);
	gnome_config_sync();
}

gboolean classbrowser_accept_size(GtkPaned *paned, gpointer user_data)
{
	save_classbrowser_position();
	return TRUE;
}

void on_new1_activate(GtkWidget *widget)
{
	// Create a new untitled tab
	tab_create_new(TAB_FILE, NULL);
}


void open_file_ok(GtkFileChooser *file_selection)
{
	GSList *filenames; 
	GSList *walk;

	// Extract filename from the file selection dialog
	filenames = gtk_file_chooser_get_uris(file_selection);
	
	for(walk = filenames; walk!= NULL; walk = g_slist_next(walk)) {
		switch_to_file_or_open(walk->data, 0);
	}
	g_slist_free(filenames);
}

void reopen_recent(GtkWidget *widget, gpointer data)
{
	gchar *filename;
	
	filename=g_strdup_printf("connectED/recent/%d=NOTFOUND", GPOINTER_TO_INT(data));
	if (DEBUG_MODE) { g_print("DEBUG: main_window_callbacks.c:reopen_recent:filename: %s\n", filename); }
	
	switch_to_file_or_open(filename, 0);
	uber_free(filename);
}

void run_plugin(GtkWidget *widget, gpointer data){
	plugin_exec((gulong) data);// was (gint)
}

static gchar *get_gnome_vfs_dirname(gchar *filename)
{
	gchar *end = NULL;
	gchar *dirname = NULL;
	gint length=0;
	
	end = strrchr(filename, '/');
	if (end) {
		length = (end-filename);
		dirname = g_malloc0(length+1);
		strncpy(dirname, filename, length);
	}
	return dirname;
}

GString *get_folder(GString *filename)
{
	gchar *dir;
	GString *folder;

	if(( g_strrstr(filename->str, "/" )) )
		dir=get_gnome_vfs_dirname( filename->str );
	else
		dir=g_get_current_dir();
	
	folder=g_string_new(dir);
	g_free(dir);

	return g_string_append( folder, "/" );
}

void on_openselected1_activate(GtkWidget *widget)
{
	GSList *li;

	gint current_pos;
	gint wordStart;
	gint wordEnd;
	gchar *ac_buffer;
	gint ac_length;
	GString *file;
	GnomeVFSURI *uri;
	Editor *editor=main_window_get_current_editor();

	if (editor == NULL || editor->type == TAB_HELP) 
		return;
	
	current_pos = gtk_scintilla_get_current_pos(GTK_SCINTILLA(editor->scintilla));
	wordStart = gtk_scintilla_get_selection_start(GTK_SCINTILLA(editor->scintilla));
	wordEnd = gtk_scintilla_get_selection_end(GTK_SCINTILLA(editor->scintilla));
	ac_buffer = gtk_scintilla_get_text_range (GTK_SCINTILLA(editor->scintilla), wordStart, wordEnd, &ac_length);

	for(li = editors; li!= NULL; li = g_slist_next(li)) {
		editor = li->data;
		if(!editor)
			continue;
		
		if (editor->opened_from) {
			file = get_folder(editor->opened_from);
			if (DEBUG_MODE) { g_print("DEBUG: main_window_callbacks.c:on_selected1_activate (opened_from) :file->str: %s\n", file->str); }
		}
		else {
			file = get_folder(editor->filename);
			if (DEBUG_MODE) { g_print("DEBUG: main_window_callbacks.c:on_selected1_activate:file->str: %s\n", file->str); }
		}
		
		if (!strstr(ac_buffer, "://") && file->str) {
			file = g_string_append(file, ac_buffer);
		}
		else if (strstr(ac_buffer, "://")) {
			file = g_string_new(ac_buffer);
		}
		
		uri = gnome_vfs_uri_new (file->str);
		if (gnome_vfs_uri_exists (uri))
			switch_to_file_or_open(file->str,0);
		gnome_vfs_uri_unref (uri);
		
		
		if (file) {
			g_string_free(file, TRUE);
		}
	}
}

void on_open1_activate(GtkWidget *widget)
{
	GtkWidget *file_selection_box=NULL;
	if(file_selection_box=create_file_selection_dialog_and_select_files( _("Please select files for editing"), "on_open1_activate", ((gboolean)FALSE)))
		open_file_ok(GTK_FILE_CHOOSER(file_selection_box));
	
	if(file_selection_box)
		gtk_widget_destroy(file_selection_box);
}


void save_file_as_confirm_overwrite(gint reply,gpointer data)
{
	Editor *editor=main_window_get_current_editor();

	if(reply)
		return;
	
	GString *filename=data; //g_string_new(gtk_file_selection_get_filename(GTK_FILE_SELECTION(data)));
	
	// Set the filename of the current editor to be that
	if (editor->filename) {
		g_string_free(editor->filename, TRUE);
	}
	editor->filename=g_string_new(filename->str);
	editor->short_filename = g_path_get_basename(filename->str);
	
	tab_check_php_file(editor);
	
	if (editor->opened_from) {
		g_string_free(editor->opened_from, TRUE);
		editor->opened_from = NULL;
	}
	
	// Call Save method to actually save it now it has a filename
	on_save1_activate(NULL);
}


void save_file_as_ok(GtkFileChooser *file_selection_box)
{
	GString *filename;
	GtkWidget *file_exists_dialog;
	struct stat st;

	// Extract filename from the file selection dialog
	filename = g_string_new(gtk_file_chooser_get_uri(file_selection_box));

	if (stat (filename->str, &st) == 0) {
		file_exists_dialog = gnome_question_dialog_modal(_("This file already exists, are you sure you want to overwrite it?"),
            save_file_as_confirm_overwrite,filename);
		gnome_dialog_run_and_close(GNOME_DIALOG(file_exists_dialog));
	}
	else {
		Editor *editor=main_window_get_current_editor();
		// Set the filename of the current editor to be that
		if (editor->filename) {
			g_string_free(editor->filename, TRUE);
		}
		editor->filename=g_string_new(filename->str);
		editor->is_untitled=FALSE;
		editor->short_filename = g_path_get_basename(filename->str);
		tab_check_php_file(editor);
		tab_check_sql_file(editor);
		tab_check_css_file(editor);

		// Call Save method to actually save it now it has a filename
		on_save1_activate(NULL);

		if (editor->opened_from) {
			g_string_free(editor->opened_from, TRUE);
			editor->opened_from = NULL;
		}
		g_string_free(filename, FALSE);
	}
}


void on_save1_activate(GtkWidget *widget)
{
	gchar *filename = NULL;
	GnomeVFSAsyncHandle *fg;
	Editor *editor=main_window_get_current_editor();
	
	if (editor) {
		filename = editor->filename->str;

		//if filename is Untitled
		if (editor->is_untitled) {// && !editor->saved) {
			on_save_as1_activate();
		}
		else {
			gnome_vfs_async_create(&fg, filename, GNOME_VFS_OPEN_WRITE, FALSE, 0644, GNOME_VFS_PRIORITY_DEFAULT, tab_file_save_opened, editor);
		}
	}
}


void on_saveall1_activate(GtkWidget *widget)
{
	FILE *stream;
	gchar *write_buffer = NULL;
	gsize text_length;
	gint status;
	gchar *filename;
	GSList *li;
	Editor *editor;

	for(li = editors; li!= NULL; li = g_slist_next(li)) {
		editor = li->data;
		if (editor) {
			filename = editor->filename->str;
			if (strcmp(filename, _("Untitled"))!=0) {
				text_length = gtk_scintilla_get_length(GTK_SCINTILLA(editor->scintilla));
	
				write_buffer = g_malloc0(text_length+1); // Include terminating null
	
				if (write_buffer == NULL) {
					g_warning ("%s", _("Cannot allocate write buffer"));
					return;
				}
	
				if ((stream = fopen (filename, "w")) == NULL) {
					g_message (_("Cannot open '%s' for writing"), editor->filename->str);
					return;
				}
	
				gtk_scintilla_get_text(GTK_SCINTILLA(editor->scintilla), text_length+1, write_buffer);
	
				status = fwrite (write_buffer, text_length, 1, stream);
	
				fclose (stream);
	
				g_free (write_buffer);
	
				gtk_scintilla_set_save_point (GTK_SCINTILLA(editor->scintilla));
			}
		}
	}
	classbrowser_update();
}


void on_save_as1_activate(void){
	if(!main_window_get_current_editor())
		return;
	
	GtkWidget *file_selection_box=NULL;
	if(file_selection_box=create_file_selection_dialog_and_select_files( _("Please type the filename to save as..."), "on_save_as1_activate", (gboolean)TRUE ))
		save_file_as_ok(GTK_FILE_CHOOSER(file_selection_box));

	if(file_selection_box)
		gtk_widget_destroy(file_selection_box);
}

static void on_reload_confirm(gint reply,gpointer filename)
{
	if (reply==0) {
		tab_load_file(main_window_get_current_editor());
	}
}


void on_reload1_activate(GtkWidget *widget){
	GtkWidget *file_revert_dialog;
	Editor *editor=main_window_get_current_editor();
	if (editor == NULL || editor->type == TAB_HELP) 
		return;

	if (editor && !editor->saved){
		file_revert_dialog = gnome_question_dialog_modal(_("Are you sure you wish to reload the current file, losing your changes?"),
		                     on_reload_confirm,NULL);
		gnome_dialog_run_and_close(GNOME_DIALOG(file_revert_dialog));
	}
	else if (editor) {
		tab_load_file(editor);
	}
}


void rename_file(GString *newfilename){
	Editor *editor=main_window_get_current_editor();
	gnome_vfs_unlink((const gchar *)editor->filename->str);
	editor->filename=newfilename;
	editor->short_filename = g_path_get_basename(newfilename->str);

	// save as new filename
	on_save1_activate(NULL);
}


/*static void rename_file_overwrite_confirm(gint reply,gpointer filename){
	if (reply==0) {
		// Call rename_file
		rename_file(filename);
	}
}*/



void rename_file_ok(GtkFileChooser *file_selection){
	GString *filename;
	GtkWidget *file_exists_dialog;
	struct stat st;

	// Extract filename from the file chooser dialog
	filename = g_string_new(gtk_file_chooser_get_uri(file_selection));

	if (stat (filename->str, &st) == 0) {
		file_exists_dialog = gnome_question_dialog_modal(_("This file already exists, are you sure you want to overwrite it?"),
		                     save_file_as_confirm_overwrite,filename);
		gnome_dialog_run_and_close(GNOME_DIALOG(file_exists_dialog));
	}

	rename_file(filename);
}


void on_rename1_activate(GtkWidget *widget){
	if(!main_window_get_current_editor())
		return;
	
	GtkWidget *file_selection_box=NULL;
	if(!( file_selection_box=create_file_selection_dialog_and_select_files( _("Please type the filename to rename this file to..."), "on_rename1_activate", ((gboolean)TRUE )) ))
		return;
	
	rename_file_ok(GTK_FILE_CHOOSER(file_selection_box));
	gtk_widget_destroy(file_selection_box);
}


GtkWidget *create_file_selection_dialog_and_select_files( const gchar *title, const char *called_from_method, gboolean for_saving ){
	Editor *editor=main_window_get_current_editor();
	if( for_saving && !editor)
		return NULL;
	
	gchar *location=NULL;
	
	if( editor && !editor->is_untitled )
		if(!for_saving)
			location=get_gnome_vfs_dirname( editor->filename->str );
		else
			location=editor->filename->str;
	else if(!( ((location=gnome_config_get_string( "connectED/general/last_opened_folder=NOTFOUND" ))) && (strcmp( location, "NOTFOUND" )) ))
		location=NULL;

	GtkWidget *file_selection_box=gtk_file_chooser_dialog_new(
						title, GTK_WINDOW(main_window_get_window()),
						( for_saving ? GTK_FILE_CHOOSER_ACTION_SAVE : GTK_FILE_CHOOSER_ACTION_OPEN ),
						GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						( for_saving ? GTK_STOCK_SAVE : GTK_STOCK_OPEN ), GTK_RESPONSE_ACCEPT,
						NULL
					);
	
	gtk_file_chooser_set_local_only( GTK_FILE_CHOOSER(file_selection_box), FALSE );
	gtk_dialog_set_default_response( GTK_DIALOG(file_selection_box), GTK_RESPONSE_ACCEPT );
	
	if(DEBUG_MODE)
		g_print( "DEBUG: main_window_callbacks.c:%s:Setting current_folder_uri to %s\n", called_from_method, location );
	
	if(location)
		if(!for_saving){
			gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(file_selection_box), TRUE);
			gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(file_selection_box), location);
		}else{
			gtk_file_chooser_set_do_overwrite_confirmation( GTK_FILE_CHOOSER(file_selection_box), TRUE );
			gtk_file_chooser_set_uri(GTK_FILE_CHOOSER(file_selection_box), location);
		}

	if( gtk_dialog_run( GTK_DIALOG(file_selection_box) ) == GTK_RESPONSE_ACCEPT )
		return file_selection_box;
	
	gtk_widget_destroy(file_selection_box);
	return NULL;
	
}/*create_file_selection_dialog_and_select_files(...);*/


static void set_active_tab(int page_num){
	Editor *new_current_editor=(Editor *)g_slist_nth_data(editors, page_num);

	if(new_current_editor)
		gtk_notebook_set_current_page(main_window_get_notebook_editor(), page_num);

	main_window_set_current_editor(new_current_editor);
}


void close_page(Editor *editor){
	gint page_num;
	gint page_num_closing;
	GtkNotebook *notebook_editor=main_window_get_notebook_editor();

	if (GTK_IS_SCINTILLA(editor->scintilla)) {
		page_num_closing = gtk_notebook_page_num(notebook_editor, editor->scintilla);
		g_free(editor->short_filename);
	}
	else {
		page_num_closing = gtk_notebook_page_num(notebook_editor, editor->help_scrolled_window);
	}

	page_num = page_num_closing-1;
	if (page_num<0) {
		page_num=0;
	}
	set_active_tab(page_num);

	g_string_free(editor->filename, TRUE);
	g_free(editor);
	gtk_notebook_remove_page(notebook_editor, page_num_closing);
}


gboolean try_save_page(Editor *editor, gboolean close_if_can){
	GtkWidget *confirm_dialog;
	int ret;
	GString *string;

	string=g_string_new("");
	g_string_printf(string, _("The file '%s' has not been saved since your last changes, are you sure you want to close it and lose these changes?"), editor->short_filename);
	confirm_dialog=gnome_message_box_new( string->str,GNOME_MESSAGE_BOX_WARNING,
		_("Close _without saving"), _("_Save file"), _("_Cancel closing"), NULL);
	g_string_printf(string,_("Unsaved changes to '%s'"), editor->filename->str);
	gtk_window_set_title(GTK_WINDOW(confirm_dialog), string->str);
	ret = gnome_dialog_run (GNOME_DIALOG (confirm_dialog));
	switch (ret) {
		case 0:
			if (close_if_can) {
				close_page(editor);
				editors = g_slist_remove(editors, editor);
				if (g_slist_length(editors) == 0) {
					editors=NULL;
					main_window_set_current_editor(NULL);
				}
			}
			return TRUE;
		case 1:
			on_save1_activate(NULL);
			// If chose neither of these, dialog either cancelled or closed.  Do nothing.
	}
	return FALSE;
}


gboolean try_close_page(Editor *editor){
	if (!editor->saved && editor->type!=TAB_HELP)
		return try_save_page(editor, TRUE);
	
	close_page(editor);
	editors = g_slist_remove(editors, editor);
	if (g_slist_length(editors) == 0) {
		editors = NULL;
		main_window_set_current_editor(NULL);
	}
	return TRUE;
}


void on_close1_activate(GtkWidget *widget){
	GtkTreeIter iter;
	Editor *editor=NULL;

	if(!(editor=main_window_get_current_editor()))
		return;
	
	try_close_page(editor);
	classbrowser_update();
	update_app_title();
	if (!gtk_tree_selection_get_selected(main_window_get_class_browser_tree_select(), NULL, &iter))
			gtk_label_set_text(GTK_LABEL(main_window_get_tree_view_label()), _("FILE:"));
}


void on_quit1_activate(GtkWidget *widget){
	if(!can_all_tabs_be_saved()){
		update_app_title();
		return;
	}
	
	gtk_main_quit();
}

void on_cut1_activate(GtkWidget *widget){
	gint wordStart;
	gint wordEnd;
	gint length;
	gchar *buffer;
	Editor *editor=main_window_get_current_editor();
	
	if (editor == NULL || editor->type == TAB_HELP)
		return;
	
	wordStart = gtk_scintilla_get_selection_start(GTK_SCINTILLA(editor->scintilla));
	wordEnd = gtk_scintilla_get_selection_end(GTK_SCINTILLA(editor->scintilla));
	if (wordStart != wordEnd) {
		buffer = gtk_scintilla_get_text_range (GTK_SCINTILLA(editor->scintilla), wordStart, wordEnd, &length);

		gtk_clipboard_set_text(main_window_get_clipboard(), buffer, length);
		gtk_scintilla_replace_sel(GTK_SCINTILLA(editor->scintilla), "");
	}
}

gchar *gtkhtml2_selection_get_text (HtmlView *view){
	GSList *list = view->sel_list;
	GString *str = g_string_new ("");
	gchar *ptr;

	if(view->sel_list == NULL)
		return NULL;
	
	while (list) {
		HtmlBoxText *text = HTML_BOX_TEXT (list->data);

		list = list->next;
		/*
		 * Some boxes may not have any text
		 */
		if (text->canon_text == NULL)
			continue;
		switch (text->selection) {
		case HTML_BOX_TEXT_SELECTION_NONE:
			g_assert_not_reached ();
			break;
		case HTML_BOX_TEXT_SELECTION_END:
			g_string_append_len (str, (gchar *)text->canon_text, g_utf8_offset_to_pointer ((gchar *)text->canon_text, (glong)text->sel_end_index) - (gchar *)text->canon_text);
			break;
		case HTML_BOX_TEXT_SELECTION_START:
			g_string_append_len (str, g_utf8_offset_to_pointer ((gchar *)text->canon_text, (glong)text->sel_start_index),
					 g_utf8_offset_to_pointer ((gchar *)text->canon_text, (glong)text->length) - g_utf8_offset_to_pointer ((gchar *)text->canon_text, (glong)text->sel_start_index));
			break;
		case HTML_BOX_TEXT_SELECTION_FULL:
			g_string_append_len (str, (gchar *)text->canon_text, g_utf8_offset_to_pointer ((gchar *)text->canon_text, (glong)text->length) - (gchar *)text->canon_text);
			break;
		case HTML_BOX_TEXT_SELECTION_BOTH:
			g_string_append_len (str, g_utf8_offset_to_pointer ((gchar *)text->canon_text, (glong)MIN (text->sel_start_index, text->sel_end_index)),
					g_utf8_offset_to_pointer ((gchar *)text->canon_text, (glong)MAX (text->sel_end_index, text->sel_start_index)) - 
					g_utf8_offset_to_pointer ((gchar *)text->canon_text, (glong)MIN (text->sel_start_index, text->sel_end_index)));
			break;
		}
	}
	ptr = str->str;
	g_string_free (str, FALSE);
	return ptr;
}

void on_copy1_activate(GtkWidget *widget){
	gint wordStart;
	gint wordEnd;
	gint length;
	gchar *buffer;
	Editor *editor=main_window_get_current_editor();
	
	if (editor == NULL)
		return;
	
	if (editor->type == TAB_HELP) {
		buffer = gtkhtml2_selection_get_text(HTML_VIEW(editor->help_view));
		gtk_clipboard_set_text(main_window_get_clipboard(), buffer, 1);
		g_free(buffer);
	}
	else {
		wordStart = gtk_scintilla_get_selection_start(GTK_SCINTILLA(editor->scintilla));
		wordEnd = gtk_scintilla_get_selection_end(GTK_SCINTILLA(editor->scintilla));
		if (wordStart != wordEnd) {
			buffer = gtk_scintilla_get_text_range (GTK_SCINTILLA(editor->scintilla), wordStart, wordEnd, &length);
	
			gtk_clipboard_set_text(main_window_get_clipboard(),buffer,length);
		}
	}
	macro_record(editor->scintilla, 2178, 0, 0); // As copy doesn't change the buffer it doesn't get recorded, so do it manually
}


static void on_paste_got_from_cliboard(GtkClipboard *clipboard, const gchar *text, gpointer data){
	Editor *editor=editor_find_from_scintilla(data);
	editor->is_pasting = editor->is_macro_recording;
	gtk_scintilla_replace_sel(GTK_SCINTILLA(data), text);
	editor->is_pasting = FALSE;

	// Possible fix for rendering issues after pasting
	gtk_scintilla_colourise(GTK_SCINTILLA(main_window_get_current_editor()->scintilla), 0, -1);
}

void on_paste1_activate(GtkWidget *widget){
	Editor *editor=main_window_get_current_editor();
	if (editor == NULL || editor->type == TAB_HELP)
		return;
	
	gtk_clipboard_request_text(main_window_get_clipboard(), on_paste_got_from_cliboard, editor->scintilla);
}


void on_selectall1_activate(GtkWidget *widget){
	Editor *editor=main_window_get_current_editor();
	if (editor == NULL)
		return;
	gtk_scintilla_select_all(GTK_SCINTILLA(editor->scintilla));
}


void on_find1_activate(GtkWidget *widget){
	Editor *editor=main_window_get_current_editor();
	if (editor && editor->type != TAB_HELP)
		find_show();
}


void on_replace1_activate(GtkWidget *widget){
	Editor *editor=main_window_get_current_editor();
	if (editor && editor->type != TAB_HELP)
		replace_show();
}


void on_properties1_activate(GtkWidget *widget){
}


void on_undo1_activate(GtkWidget *widget)
{
	Editor *editor=main_window_get_current_editor();
	if (editor && editor->type != TAB_HELP){
		gtk_scintilla_undo(GTK_SCINTILLA(editor->scintilla));
	}
}


void on_redo1_activate(GtkWidget *widget)
{
	Editor *editor=main_window_get_current_editor();
	if (editor && editor->type != TAB_HELP){
		gtk_scintilla_redo(GTK_SCINTILLA(editor->scintilla));
	}
}


void keyboard_macro_startstop(GtkWidget *widget)
{
	Editor *editor=main_window_get_current_editor();
	if (editor && editor->type != TAB_HELP)
		return;
	
	if (editor->is_macro_recording) {
		gtk_scintilla_stop_record(GTK_SCINTILLA(editor->scintilla));
		editor->is_macro_recording = FALSE;
	} else {
		if (editor->keyboard_macro_list) {
			keyboard_macro_empty_old(editor);
		}
		gtk_scintilla_start_record(GTK_SCINTILLA(editor->scintilla));
		editor->is_macro_recording = TRUE;
	}
}

void keyboard_macro_playback(GtkWidget *widget)
{
	GSList *current;
	MacroEvent *event;
	Editor *editor=main_window_get_current_editor();
	if(!(editor && editor->type != TAB_HELP))
		return;
	
	gtk_scintilla_begin_undo_action(GTK_SCINTILLA(editor->scintilla));
	if (editor->keyboard_macro_list) {
		for (current = editor->keyboard_macro_list; current; current = g_slist_next(current)) {
			event = current->data;
			if (DEBUG_MODE)
				g_print("DEBUG: main_window_callbacks.c:keyboard_macro_playback:Message: %d (%s)\n", event->message, macro_message_to_string(event->message));
			switch (event->message) {
				case (2170) : gtk_scintilla_replace_sel(GTK_SCINTILLA(editor->scintilla), (gchar *)event->lparam); break;
				case (2177) : gtk_scintilla_cut(GTK_SCINTILLA(editor->scintilla)); break;
				case (2178) : gtk_scintilla_copy(GTK_SCINTILLA(editor->scintilla)); break;
				case (2179) : gtk_scintilla_paste(GTK_SCINTILLA(editor->scintilla)); break;
				case (2180) : gtk_scintilla_clear(GTK_SCINTILLA(editor->scintilla)); break;
				case (2300) : gtk_scintilla_line_down(GTK_SCINTILLA(editor->scintilla)); break;
				case (2301) : gtk_scintilla_line_down_extend(GTK_SCINTILLA(editor->scintilla)); break;
				case (2302) : gtk_scintilla_line_up(GTK_SCINTILLA(editor->scintilla)); break;
				case (2303) : gtk_scintilla_line_up_extend(GTK_SCINTILLA(editor->scintilla)); break;
				case (2304) : gtk_scintilla_char_left(GTK_SCINTILLA(editor->scintilla)); break;
				case (2305) : gtk_scintilla_char_left_extend(GTK_SCINTILLA(editor->scintilla)); break;
				case (2306) : gtk_scintilla_char_right(GTK_SCINTILLA(editor->scintilla)); break;
				case (2307) : gtk_scintilla_char_right_extend(GTK_SCINTILLA(editor->scintilla)); break;
				case (2308) : gtk_scintilla_word_left(GTK_SCINTILLA(editor->scintilla)); break;
				case (2309) : gtk_scintilla_word_left_extend(GTK_SCINTILLA(editor->scintilla)); break;
				case (2310) : gtk_scintilla_word_right(GTK_SCINTILLA(editor->scintilla)); break;
				case (2311) : gtk_scintilla_word_right_extend(GTK_SCINTILLA(editor->scintilla)); break;
				case (2312) : gtk_scintilla_home(GTK_SCINTILLA(editor->scintilla)); break;
				case (2313) : gtk_scintilla_home_extend(GTK_SCINTILLA(editor->scintilla)); break;
				case (2314) : gtk_scintilla_line_end(GTK_SCINTILLA(editor->scintilla)); break;
				case (2315) : gtk_scintilla_line_end_extend(GTK_SCINTILLA(editor->scintilla)); break;
				case (2316) : gtk_scintilla_document_start(GTK_SCINTILLA(editor->scintilla)); break;
				case (2317) : gtk_scintilla_document_start_extend(GTK_SCINTILLA(editor->scintilla)); break;
				case (2318) : gtk_scintilla_document_end(GTK_SCINTILLA(editor->scintilla)); break;
				case (2319) : gtk_scintilla_document_end_extend(GTK_SCINTILLA(editor->scintilla)); break;
				case (2320) : gtk_scintilla_page_up(GTK_SCINTILLA(editor->scintilla)); break;
				case (2321) : gtk_scintilla_page_up_extend(GTK_SCINTILLA(editor->scintilla)); break;
				case (2322) : gtk_scintilla_page_down(GTK_SCINTILLA(editor->scintilla)); break;
				case (2323) : gtk_scintilla_page_down_extend(GTK_SCINTILLA(editor->scintilla)); break;
				case (2324) : gtk_scintilla_edit_toggle_overtype(GTK_SCINTILLA(editor->scintilla)); break;
				case (2325) : gtk_scintilla_cancel(GTK_SCINTILLA(editor->scintilla)); break;
				case (2326) : gtk_scintilla_delete_back(GTK_SCINTILLA(editor->scintilla)); break;
				case (2327) : gtk_scintilla_tab(GTK_SCINTILLA(editor->scintilla)); break;
				case (2328) : gtk_scintilla_back_tab(GTK_SCINTILLA(editor->scintilla)); break;
				case (2329) : gtk_scintilla_new_line(GTK_SCINTILLA(editor->scintilla)); break;
				case (2330) : gtk_scintilla_form_feed(GTK_SCINTILLA(editor->scintilla)); break;
				case (2331) : gtk_scintilla_v_c_home(GTK_SCINTILLA(editor->scintilla)); break;
				case (2332) : gtk_scintilla_v_c_home_extend(GTK_SCINTILLA(editor->scintilla)); break;
				case (2333) : gtk_scintilla_zoom_in(GTK_SCINTILLA(editor->scintilla)); break;
				case (2334) : gtk_scintilla_zoom_out(GTK_SCINTILLA(editor->scintilla)); break;
				case (2335) : gtk_scintilla_del_word_left(GTK_SCINTILLA(editor->scintilla)); break;
				case (2336) : gtk_scintilla_del_word_right(GTK_SCINTILLA(editor->scintilla)); break;
				case (2337) : gtk_scintilla_line_cut(GTK_SCINTILLA(editor->scintilla)); break;
				case (2338) : gtk_scintilla_line_delete(GTK_SCINTILLA(editor->scintilla)); break;
				case (2339) : gtk_scintilla_line_transpose(GTK_SCINTILLA(editor->scintilla)); break;
				case (2340) : gtk_scintilla_lower_case(GTK_SCINTILLA(editor->scintilla)); break;
				case (2341) : gtk_scintilla_upper_case(GTK_SCINTILLA(editor->scintilla)); break;
				case (2342) : gtk_scintilla_line_scroll_down(GTK_SCINTILLA(editor->scintilla)); break;
				case (2343) : gtk_scintilla_line_scroll_up(GTK_SCINTILLA(editor->scintilla)); break;
				case (2344) : gtk_scintilla_delete_back_not_line(GTK_SCINTILLA(editor->scintilla)); break;
				case (2345) : gtk_scintilla_home_display(GTK_SCINTILLA(editor->scintilla)); break;
				case (2346) : gtk_scintilla_home_display_extend(GTK_SCINTILLA(editor->scintilla)); break;
				case (2347) : gtk_scintilla_line_end_display(GTK_SCINTILLA(editor->scintilla)); break;
				case (2348) : gtk_scintilla_line_end_display_extend(GTK_SCINTILLA(editor->scintilla)); break;
				default:
					g_print(_("Unhandle keyboard macro function %d, please report to uberChick@uberChicGeekChick.Com\n"), event->message);
			}
		}
	}
	gtk_scintilla_end_undo_action(GTK_SCINTILLA(editor->scintilla));
}


void on_preferences1_activate(GtkWidget *widget)
{
	preferences_dialog_create();

	if (gtk_dialog_run(GTK_DIALOG(preferences_dialog.window)) == GTK_RESPONSE_ACCEPT) {
		apply_preferences(NULL, NULL);
	}
	
	// destroy/null dialog
	gtk_widget_destroy(preferences_dialog.window);
	preferences_dialog.window = NULL;
}


void context_help(GtkWidget *widget)
{
	gchar *buffer = NULL;
	gint length;
	GString *function;
	gint wordStart;
	gint wordEnd;
	Editor *editor=main_window_get_current_editor();
	
	if(!(editor && editor->type != TAB_HELP))
		return;
	
	wordStart = gtk_scintilla_get_selection_start(GTK_SCINTILLA(editor->scintilla));
	wordEnd = gtk_scintilla_get_selection_end(GTK_SCINTILLA(editor->scintilla));
	if (wordStart != wordEnd) {
		buffer = gtk_scintilla_get_text_range (GTK_SCINTILLA(editor->scintilla), wordStart, wordEnd, &length);
		
		function = g_string_new(buffer);
		tab_create_new(TAB_HELP, function);
		g_string_free(function, TRUE);
	}

	uber_free(buffer);
}


void on_about1_activate(GtkWidget *widget)
{
	const gchar *authors[] = {
					"Main Open Source Artist:",
						"Kaity G. B. <uberChick@uberChicGeekChick.Com>",
					NULL
				};
	const gchar *documenters[] = {
					NULL
				};
	gchar *translator_credits = _("translator_credits");
	GtkWidget *about;
	GdkPixbuf *pixbuf = NULL;
	GError *error = NULL;

	pixbuf = gdk_pixbuf_new_from_file (PIXMAP_DIR "/" connectED_PIXMAP_ICON, &error);

	if(error) {
		g_warning (G_STRLOC ": cannot open icon: %s", error->message);
		g_error_free (error);
	}
	about = gnome_about_new( "connectED", VERSION,
				_("Copyright  2006-2009 Kaity G. B."),
				_("connectED is a GNOME2 PHP IDE\nconnectED is also an IDE for Perl, XML, XHTML, CSS, JavaScript source code."),
				(const gchar **) authors,
				(const gchar **) documenters,
				strcmp (translator_credits, "translator_credits") != 0 ? translator_credits : NULL,
				pixbuf);
	gtk_widget_show(about);

	if(pixbuf)
		gdk_pixbuf_unref (pixbuf);
	
	gtk_window_set_transient_for( GTK_WINDOW(about), NULL);
	gtk_widget_show(about);
}

void on_notebook_switch_page (GtkNotebook *notebook, GtkNotebookPage *page, gint page_num, gpointer user_data){
	Editor *data;
	GtkWidget *child;
	Editor *editor=main_window_get_current_editor();

	child = gtk_notebook_get_nth_page(GTK_NOTEBOOK(main_window_get_notebook_editor()), page_num);
	data = editor_find_from_scintilla(child);
	if (data) {
		if (GTK_IS_SCINTILLA(data->scintilla)) {
			// Grab the focus in to the editor
			gtk_scintilla_grab_focus(GTK_SCINTILLA(data->scintilla));

			// Store it in the global editor value
			editor = data;
		}
	}
	else {
		data = editor_find_from_help((void *)child);
		if (data) {
			editor = data;
		}
		else {
 			g_print(_("Unable to get data for page %d\n"), page_num);
		}
	}
	
	// Change the title of the main application window to the full filename
	if (!is_app_closing)
		update_app_title();
}

gboolean on_notebook_focus_tab(GtkNotebook *notebook, GtkNotebookTab arg1, gpointer user_data){
	gtk_scintilla_grab_focus(GTK_SCINTILLA(main_window_get_current_editor()->scintilla));
	return TRUE;
}

void inc_search_typed (GtkEntry *entry, const gchar *text, gint length, gint *position, gpointer data){
	gint found_pos;
	glong text_min, text_max;
	gchar *current_text;
	Editor *editor=main_window_get_current_editor();

	if(!editor)
		return;
	
	current_text = (gchar *)gtk_entry_get_text(entry);
	
	if( ( found_pos=gtk_scintilla_find_text(GTK_SCINTILLA(editor->scintilla), 0, current_text, 0, gtk_scintilla_get_length(GTK_SCINTILLA(editor->scintilla)), &text_min, &text_max)) != 1 )
		gtk_scintilla_set_sel(GTK_SCINTILLA(editor->scintilla), text_min, text_max);
}


gboolean inc_search_key_release_event(GtkWidget *widget,GdkEventKey *event,gpointer user_data)
{
	if(event->keyval != GDK_Escape)
		return FALSE;
	
	Editor *editor=main_window_get_current_editor();
	gtk_scintilla_grab_focus(GTK_SCINTILLA(editor->scintilla));
	return TRUE;
}


void inc_search_activate(GtkEntry *entry,gpointer user_data){
	Editor *editor=main_window_get_current_editor();
	if(!editor) return;
	
	glong text_min, text_max;

	gint current_pos = gtk_scintilla_get_current_pos(GTK_SCINTILLA(editor->scintilla));
	gchar *current_text = (gchar *)gtk_entry_get_text(entry);

	if( (gtk_scintilla_find_text(GTK_SCINTILLA(editor->scintilla), 0, current_text, current_pos+1, gtk_scintilla_get_length(GTK_SCINTILLA(editor->scintilla)), &text_min, &text_max)) != -1)
		gtk_scintilla_set_sel(GTK_SCINTILLA(editor->scintilla), text_min, text_max);
	
	else if( (gtk_scintilla_find_text(GTK_SCINTILLA(editor->scintilla), 0, current_text, 0, gtk_scintilla_get_length(GTK_SCINTILLA(editor->scintilla)), &text_min, &text_max)) != 1 )
		gtk_scintilla_set_sel(GTK_SCINTILLA(editor->scintilla), text_min, text_max);
}


gboolean is_valid_digits_only(gchar *text){
	while(*text)
		if(*text < 48 || *text > 57)
			return FALSE;

	return TRUE;
}

void goto_line_int(gint line){
	Editor *editor=main_window_get_current_editor();
	gtk_scintilla_grab_focus(GTK_SCINTILLA(editor->scintilla));
	gtk_scintilla_goto_line(GTK_SCINTILLA(editor->scintilla), line-1);
	//gint current_pos=gtk_scintilla_get_current_pos(GTK_SCINTILLA(editor->scintilla));
	gtk_scintilla_scroll_caret(GTK_SCINTILLA(editor->scintilla));
}


void goto_line(gchar *text){
	goto_line_int( (atoi( text )) );
}

void goto_line_activate(GtkEntry *entry,gpointer user_data){
	Editor *editor=main_window_get_current_editor();
	if(!editor) return;
	goto_line_int( (atoi( ((gchar *)gtk_entry_get_text( entry )) )) );
}


void move_block(gint indentation_size){
	gint startline;
	gint endline;
	gint line;
	gint indent;
	Editor *editor=main_window_get_current_editor();

	if(!editor) return;
	
	gtk_scintilla_begin_undo_action(GTK_SCINTILLA(editor->scintilla));
	
	startline = gtk_scintilla_line_from_position(GTK_SCINTILLA(editor->scintilla), gtk_scintilla_get_selection_start(GTK_SCINTILLA(editor->scintilla)));
	endline = gtk_scintilla_line_from_position(GTK_SCINTILLA(editor->scintilla), gtk_scintilla_get_selection_end(GTK_SCINTILLA(editor->scintilla)));
	
	for (line = startline; line < endline; line++) {
		indent = gtk_scintilla_get_line_indentation(GTK_SCINTILLA(editor->scintilla), line);
		gtk_scintilla_set_line_indentation(GTK_SCINTILLA(editor->scintilla), line, indent+indentation_size);
	}
	gtk_scintilla_end_undo_action(GTK_SCINTILLA(editor->scintilla));
}


void block_indent(GtkWidget *widget){
	move_block(preferences.indentation_size);
}


void block_unindent(GtkWidget *widget){
	move_block(0-preferences.indentation_size);
}

void classbrowser_show(void){
	gtk_paned_set_position(GTK_PANED(main_window_get_horizontal_pane()),classbrowser_hidden_position);
	gnome_config_set_int ("connectED/main_window/classbrowser_hidden",0);
	gnome_config_sync();
	classbrowser_update();
}


void classbrowser_hide(void)
{
	classbrowser_hidden_position = gnome_config_get_int ("connectED/main_window/classbrowser_size=100");
	gtk_paned_set_position(GTK_PANED(main_window_get_horizontal_pane()),0);
	gnome_config_set_int ("connectED/main_window/classbrowser_hidden",1);
	gnome_config_sync();
}

void classbrowser_show_hide(GtkWidget *widget)
{
	gint hidden;
	
	hidden = gnome_config_get_int ("connectED/main_window/classbrowser_hidden=0");
	if (hidden == 1)
		classbrowser_show();
	else
		classbrowser_hide();
}

void lint_row_activated (GtkTreeSelection *selection, gpointer data){
	GtkTreeIter iter;
	GtkTreeModel *model;
	gchar *line;
	gchar *space;

	if(!gtk_tree_selection_get_selected (selection, &model, &iter)) return;
	
	gtk_tree_model_get (model, &iter, 0, &line, -1);
	// Get the line
	space = strrchr(line, ' ');
	space++;
	// Go to that line
	goto_line(space);
	g_free (line);
}


gint treeview_double_click(GtkWidget *widget, GdkEventButton *event ){
	GtkTreeIter iter;
	gchar *filename = NULL;
	guint line_number;

	if(!( (( event->type==GDK_2BUTTON_PRESS || event->type==GDK_3BUTTON_PRESS )) && (gtk_tree_selection_get_selected(main_window_get_class_browser_tree_select(), NULL, &iter)) )) return FALSE;
	
	gtk_tree_model_get(GTK_TREE_MODEL(main_window_get_class_browser_tree_store()), &iter, FILENAME_COLUMN, &filename, LINE_NUMBER_COLUMN, &line_number, -1);
	if(!filename) return FALSE;
	
	switch_to_file_or_open(filename, line_number);
	g_free (filename);

	return FALSE;
}


gint treeview_click_release( GtkWidget *widget, GdkEventButton *event ){
	GtkTreeIter iter;
	gchar *filename=NULL;
	guint line_number;

	if( (gtk_tree_selection_get_selected(main_window_get_class_browser_tree_select(), NULL, &iter )) ) {
		gtk_tree_model_get(GTK_TREE_MODEL(main_window_get_class_browser_tree_store()), &iter, FILENAME_COLUMN, &filename, LINE_NUMBER_COLUMN, &line_number, -1);
		if (filename) {
			classbrowser_update_selected_label(filename, line_number);
			g_free(filename);
		}
	}

	Editor *editor=main_window_get_current_editor();
	if(!editor) return FALSE;

	gtk_scintilla_grab_focus(GTK_SCINTILLA(editor->scintilla));
	gtk_scintilla_scroll_caret(GTK_SCINTILLA(editor->scintilla));
	gtk_scintilla_grab_focus(GTK_SCINTILLA(editor->scintilla));
	
	return FALSE;
}

void force_php(GtkWidget *widget){
	Editor *editor=main_window_get_current_editor();
	if(!editor) return;
	set_editor_to_php(editor);
}

void force_css(GtkWidget *widget){
	Editor *editor=main_window_get_current_editor();
	if(!editor) return;
	set_editor_to_css(editor);
}

void force_sql(GtkWidget *widget){
	Editor *editor=main_window_get_current_editor();
	if(!editor) return;
	set_editor_to_sql(editor);
}

void force_cxx(GtkWidget *widget){
	Editor *editor=main_window_get_current_editor();
	if(!editor) return;
	set_editor_to_cxx(editor);
}

void force_perl(GtkWidget *widget){
	Editor *editor=main_window_get_current_editor();
	if(!editor) return;
	set_editor_to_perl(editor);
}

void force_python(GtkWidget *widget){
	Editor *editor=main_window_get_current_editor();
	if(!editor) return;
	set_editor_to_python(editor);
}
