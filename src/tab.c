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


#define PLAT_GTK 1
#include <gtkscintilla.h>

#include "tab.h"
#include "tab_php.h"
#include "tab_css.h"
#include "tab_sql.h"
#include "tab_cxx.h"
#include "tab_perl.h"
#include "tab_python.h"
#include "calltip.h"
#include "main_window.h"
#include "main_window_callbacks.h"
#include "preferences.h"
#include "grel2abs.h"


GSList *editors;
guint completion_timer_id;
gboolean completion_timer_set;
guint calltip_timer_id;
gboolean calltip_timer_set;
guint gotoline_after_reload = 0;
guint gotoline_after_create_tab = 0;

static void save_point_reached(GtkWidget *w);
static void save_point_left(GtkWidget *w);
static void char_added(GtkWidget *scintilla, guint ch);


void debug_dump_editors(void)
{
	Editor *editor;
	GSList *walk;
	gint editor_number = 0;

	g_print("--------------------------------------------------------------\n");
	g_print("connectED Debug Output\n\n");

	
	for (walk = editors; walk!=NULL; walk = g_slist_next(walk)) {
		editor = walk->data;
		editor_number++;
		
		g_print("Editor number    :%d\n", editor_number);
		g_print("Scintilla widget :%p\n", editor->scintilla);
		g_print("Scintilla lexer  :%d\n", gtk_scintilla_get_lexer(GTK_SCINTILLA(editor->scintilla)));
		g_print("Scintilla ID     :%d\n", editor->scintilla_id);
		g_print("File mtime       :%d\n", editor->file_mtime);
		g_print("File shortname   :%s\n", editor->short_filename);
		g_print("File fullname    :%s\n", editor->filename->str);
		if (editor->opened_from) {
			g_print("Opened from      :%s\n", editor->opened_from->str);
		}
		g_print("Saved?           :%d\n", editor->saved);
		g_print("Converted to UTF?:%d\n\n", editor->saved);
	}
	g_print("--------------------------------------------------------------\n");
}

void tab_set_general_scintilla_properties(Editor *editor)
{
	gtk_scintilla_set_backspace_unindents(GTK_SCINTILLA(editor->scintilla), 1);

	gtk_scintilla_autoc_set_choose_single(GTK_SCINTILLA (editor->scintilla), FALSE);
	gtk_scintilla_autoc_set_ignore_case(GTK_SCINTILLA (editor->scintilla), TRUE);
	gtk_scintilla_autoc_set_drop_rest_of_word(GTK_SCINTILLA (editor->scintilla), FALSE);

	gtk_scintilla_set_code_page(GTK_SCINTILLA(editor->scintilla), 65001); // Unicode code page
	
	gtk_signal_connect (GTK_OBJECT (editor->scintilla), "save_point_reached", GTK_SIGNAL_FUNC (save_point_reached), NULL);
	gtk_signal_connect (GTK_OBJECT (editor->scintilla), "save_point_left", GTK_SIGNAL_FUNC (save_point_left), NULL);
	gtk_signal_connect (GTK_OBJECT (editor->scintilla), "macro_record", GTK_SIGNAL_FUNC (macro_record), NULL);

	gtk_scintilla_set_sel_back(GTK_SCINTILLA(editor->scintilla), 1, 13434879);

	tab_set_configured_scintilla_properties(GTK_SCINTILLA(editor->scintilla), preferences);
	gtk_widget_show (editor->scintilla);
}

void tab_set_configured_scintilla_properties(GtkScintilla *scintilla, Preferences prefs)
{
	gint width;

	width = gtk_scintilla_text_width(scintilla, STYLE_LINENUMBER, "_99999");
	gtk_scintilla_set_margin_width_n(scintilla, 0, width);
	gtk_scintilla_set_margin_width_n (scintilla, 1, 0);
	gtk_scintilla_set_margin_width_n (scintilla, 2, 0);
	gtk_scintilla_set_wrap_mode(scintilla, prefs.line_wrapping);
	if (prefs.line_wrapping) {
		gtk_scintilla_set_h_scroll_bar(scintilla, 0);
	}
	else {
		gtk_scintilla_set_h_scroll_bar(scintilla, 1);
	}
	gtk_scintilla_style_set_font (scintilla, STYLE_LINENUMBER, prefs.line_number_font);
	gtk_scintilla_style_set_fore (scintilla, STYLE_LINENUMBER, prefs.line_number_fore);
	gtk_scintilla_style_set_back (scintilla, STYLE_LINENUMBER, prefs.line_number_back);
    gtk_scintilla_style_set_size (scintilla, STYLE_LINENUMBER, prefs.line_number_size);

    gtk_scintilla_set_indentation_guides (scintilla, prefs.show_indentation_guides);
    gtk_scintilla_set_edge_mode (scintilla, prefs.edge_mode);
    gtk_scintilla_set_edge_column (scintilla, prefs.edge_column);
    gtk_scintilla_set_edge_colour (scintilla, prefs.edge_colour);

    gtk_scintilla_set_caret_fore (scintilla, 0);
    gtk_scintilla_set_caret_width (scintilla, 2);
    gtk_scintilla_set_caret_period (scintilla, 250);

    gtk_scintilla_autoc_set_choose_single (scintilla, TRUE);
    gtk_scintilla_set_use_tabs (scintilla, prefs.use_tabs_instead_spaces);
    gtk_scintilla_set_tab_indents (scintilla, 1);
    gtk_scintilla_set_backspace_unindents (scintilla, 1);
	gtk_scintilla_set_tab_width (scintilla, prefs.indentation_size);
    gtk_scintilla_set_indent (scintilla, prefs.tab_size);

    gtk_scintilla_style_set_font (scintilla, STYLE_DEFAULT, prefs.default_font);
    gtk_scintilla_style_set_size (scintilla, STYLE_DEFAULT, prefs.default_size);
    gtk_scintilla_style_set_italic (scintilla, STYLE_DEFAULT, prefs.default_italic);
    gtk_scintilla_style_set_bold (scintilla, STYLE_DEFAULT, prefs.default_bold);
    gtk_scintilla_style_set_fore (scintilla, STYLE_DEFAULT, prefs.default_fore);
    gtk_scintilla_style_set_back (scintilla, STYLE_DEFAULT, prefs.default_back);
}


static void tab_set_folding(Editor *editor, gint folding)
{
	gint modeventmask;
	
	if (folding) {
		modeventmask = gtk_scintilla_get_mod_event_mask(GTK_SCINTILLA(editor->scintilla));
		gtk_scintilla_set_mod_event_mask(GTK_SCINTILLA(editor->scintilla), modeventmask | SC_MOD_CHANGEFOLD);
		gtk_scintilla_set_fold_flags(GTK_SCINTILLA(editor->scintilla), SC_FOLDFLAG_LINEAFTER_CONTRACTED);
		
		gtk_scintilla_set_margin_type_n(GTK_SCINTILLA(editor->scintilla), 2, SC_MARGIN_SYMBOL);
		gtk_scintilla_set_margin_mask_n(GTK_SCINTILLA(editor->scintilla), 2, SC_MASK_FOLDERS);
		gtk_scintilla_set_margin_sensitive_n(GTK_SCINTILLA(editor->scintilla), 2, 1);
		
		gtk_scintilla_marker_define(GTK_SCINTILLA(editor->scintilla), SC_MARKNUM_FOLDEROPEN, SC_MARK_BOXMINUS);
		gtk_scintilla_marker_set_fore(GTK_SCINTILLA(editor->scintilla), SC_MARKNUM_FOLDEROPEN,16777215);
		gtk_scintilla_marker_set_back(GTK_SCINTILLA(editor->scintilla), SC_MARKNUM_FOLDEROPEN,0);
		gtk_scintilla_marker_define(GTK_SCINTILLA(editor->scintilla), SC_MARKNUM_FOLDER, SC_MARK_BOXPLUS);
		gtk_scintilla_marker_set_fore(GTK_SCINTILLA(editor->scintilla), SC_MARKNUM_FOLDER,16777215);
		gtk_scintilla_marker_set_back(GTK_SCINTILLA(editor->scintilla), SC_MARKNUM_FOLDER,0);
		gtk_scintilla_marker_define(GTK_SCINTILLA(editor->scintilla), SC_MARKNUM_FOLDERSUB, SC_MARK_VLINE);
		gtk_scintilla_marker_set_fore(GTK_SCINTILLA(editor->scintilla), SC_MARKNUM_FOLDERSUB,16777215);
		gtk_scintilla_marker_set_back(GTK_SCINTILLA(editor->scintilla), SC_MARKNUM_FOLDERSUB,0);
		gtk_scintilla_marker_define(GTK_SCINTILLA(editor->scintilla), SC_MARKNUM_FOLDERTAIL, SC_MARK_LCORNER);
		gtk_scintilla_marker_set_fore(GTK_SCINTILLA(editor->scintilla), SC_MARKNUM_FOLDERTAIL,16777215);
		gtk_scintilla_marker_set_back(GTK_SCINTILLA(editor->scintilla), SC_MARKNUM_FOLDERTAIL,0);
		gtk_scintilla_marker_define(GTK_SCINTILLA(editor->scintilla), SC_MARKNUM_FOLDEREND, SC_MARK_BOXPLUSCONNECTED);
		gtk_scintilla_marker_set_fore(GTK_SCINTILLA(editor->scintilla), SC_MARKNUM_FOLDEREND,16777215);
		gtk_scintilla_marker_set_back(GTK_SCINTILLA(editor->scintilla), SC_MARKNUM_FOLDEREND,0);
		gtk_scintilla_marker_define(GTK_SCINTILLA(editor->scintilla), SC_MARKNUM_FOLDEROPENMID, SC_MARK_BOXMINUSCONNECTED);
		gtk_scintilla_marker_set_fore(GTK_SCINTILLA(editor->scintilla), SC_MARKNUM_FOLDEROPENMID,16777215);
		gtk_scintilla_marker_set_back(GTK_SCINTILLA(editor->scintilla), SC_MARKNUM_FOLDEROPENMID,0);
		gtk_scintilla_marker_define(GTK_SCINTILLA(editor->scintilla), SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNER);
		gtk_scintilla_marker_set_fore(GTK_SCINTILLA(editor->scintilla), SC_MARKNUM_FOLDERMIDTAIL,16777215);
		gtk_scintilla_marker_set_back(GTK_SCINTILLA(editor->scintilla), SC_MARKNUM_FOLDERMIDTAIL,0);
		
		gtk_scintilla_set_margin_width_n (GTK_SCINTILLA(editor->scintilla), 2, 14);
		
		//gtk_signal_connect (GTK_OBJECT (editor->scintilla), "fold_clicked", GTK_SIGNAL_FUNC (fold_clicked), NULL);
		gtk_signal_connect (GTK_OBJECT (editor->scintilla), "modified", GTK_SIGNAL_FUNC (handle_modified), NULL);
		gtk_signal_connect (GTK_OBJECT (editor->scintilla), "margin_click", GTK_SIGNAL_FUNC (margin_clicked), NULL);
	}
}

static void tab_set_event_handlers(Editor *editor)
{
	gtk_signal_connect (GTK_OBJECT (editor->scintilla), "char_added", GTK_SIGNAL_FUNC (char_added), NULL);
	gtk_signal_connect (GTK_OBJECT (editor->scintilla), "update_ui", GTK_SIGNAL_FUNC (update_ui), NULL);
}

void report_vfs_error(gchar *name, gchar *desc, GnomeVFSResult result, GtkWindow *win)
{
	GtkDialog *dialog;
	gchar *err_title, *err_message;
	
	if (name) {
		err_title = g_strdup_printf(_("File access error:\n %s: "), name);
	}
	else {
		err_title = g_strdup_printf(_("File access error: \n"));
	}
	
	if (result < GNOME_VFS_NUM_ERRORS) {
		err_message = g_strdup(gnome_vfs_result_to_string(result));
	}
	else {
		err_message = g_strdup_printf("%s\n", desc ? desc : "");
	}
	dialog = GTK_DIALOG(gtk_message_dialog_new(win, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s %s", err_title, err_message));
	g_signal_connect_swapped(dialog, "response", G_CALLBACK(gtk_widget_destroy), dialog);
	g_object_set(dialog, "title", "", NULL);
	gtk_widget_show_all(GTK_WIDGET(dialog));
	g_free(err_title);
	g_free(err_message);
}

void tab_file_closed(GnomeVFSAsyncHandle *fd, GnomeVFSResult result, gpointer li_ptr)
{
	//Editor *editor = (Editor *)li_ptr;

	if (result != GNOME_VFS_OK) {
		g_print(_("VFS Error: %s\n"), gnome_vfs_result_to_string (result));
		//gnome_vfs_print_error(result, editor->filename->str);
	}
	session_save();
}

void tab_file_write(GnomeVFSAsyncHandle *fd, GnomeVFSResult result, gconstpointer buffer, GnomeVFSFileSize bytes_requested, GnomeVFSFileSize bytes_received, gpointer li_ptr)
{
  	Editor *editor = (Editor *)li_ptr;
	
	if (result != GNOME_VFS_OK) {
		g_print(_("VFS Error: %s\n"), gnome_vfs_result_to_string (result));
		//gnome_vfs_print_error(result, editor->filename->str);
		return;
	}
	
	gnome_vfs_async_close (fd, tab_file_closed, li_ptr);
	gtk_scintilla_set_save_point (GTK_SCINTILLA(editor->scintilla));
	register_file_opened(editor->filename->str);
	classbrowser_update();
	session_save();
}

void tab_file_save_opened(GnomeVFSAsyncHandle *fd, GnomeVFSResult result, gpointer li_ptr)
{
	gchar *write_buffer = NULL;
	gsize text_length;
	GError *error = NULL;
	gchar *converted_text = NULL;
	gsize utf8_size; // was guint
	//GnomeVFSFileSize bytes_written;
	//Editor *editor = (Editor *)li_ptr;
	
	if (result != GNOME_VFS_OK) {
 		g_print(_("VFS Error: %s\n"), gnome_vfs_result_to_string (result));
		//gnome_vfs_print_error(result, editor->filename->str);
		return;
	}

	text_length = gtk_scintilla_get_length(GTK_SCINTILLA(main_window.current_editor->scintilla));

	write_buffer = g_malloc0(text_length+1); // Include terminating null

	if (write_buffer == NULL) {
		g_warning ("%s", _("Cannot allocate write buffer"));
		return;
	}

	gtk_scintilla_get_text(GTK_SCINTILLA(main_window.current_editor->scintilla), text_length+1, write_buffer);

	// If we converted to UTF-8 when loading, convert back to the locale to save
	if (main_window.current_editor->converted_to_utf8) {
		converted_text = g_locale_from_utf8(write_buffer, text_length, NULL, &utf8_size, &error);
		if (error != NULL) {
			g_print(_("UTF-8 Error: %s\n"), error->message);
			g_error_free(error);			
		}
		else {
			//g_print("DEBUG: Converted size: %d\n", utf8_size);
			g_free(write_buffer);
			write_buffer = converted_text;
			text_length = utf8_size;
		}
	}
		
	gnome_vfs_async_write (fd, write_buffer, text_length, tab_file_write, li_ptr);
	//g_free (write_buffer);
}

void tab_validate_buffer_and_insert(gpointer buffer, Editor *editor)
{
	gchar *converted_text;
	gsize utf8_size;// was guint
	GError *error = NULL;
	
	if (g_utf8_validate(buffer, editor->file_size, NULL)) {
		//g_print("Valid UTF8 according to gnome\n");
		gtk_scintilla_add_text(GTK_SCINTILLA (editor->scintilla), editor->file_size, buffer);
		editor->converted_to_utf8 = FALSE;
	}
	else {
		// Used for testing as my locale isn't set
		// converted_text = g_convert(buffer, nchars, "UTF-8", "ISO-8859-15", NULL, &utf8_size, &error);
		converted_text = g_locale_to_utf8(buffer, editor->file_size, NULL, &utf8_size, &error);
		if (error != NULL) {
			g_print(_("connectED UTF-8 Error: %s\n"), error->message);
			g_error_free(error);
			gtk_scintilla_add_text(GTK_SCINTILLA (editor->scintilla), editor->file_size, buffer);
		}
		else {
			g_print(_("Converted to UTF-8 size: %d\n"), utf8_size);
			gtk_scintilla_add_text(GTK_SCINTILLA (editor->scintilla), utf8_size, converted_text);
			g_free(converted_text);
			editor->converted_to_utf8 = TRUE;
		}
	}
}

void tab_reset_scintilla_after_open(Editor *editor)
{
	gtk_scintilla_empty_undo_buffer(GTK_SCINTILLA(editor->scintilla));
	gtk_scintilla_set_save_point(GTK_SCINTILLA(editor->scintilla));
	gtk_scintilla_goto_line(GTK_SCINTILLA(editor->scintilla), editor->current_line);
	gtk_scintilla_scroll_caret(GTK_SCINTILLA(editor->scintilla));
	
	gtk_scintilla_grab_focus(GTK_SCINTILLA(editor->scintilla));
}

void tab_file_read(GnomeVFSAsyncHandle *fd, GnomeVFSResult result, gpointer buffer, GnomeVFSFileSize bytes_requested, GnomeVFSFileSize bytes_received, gpointer li_ptr)
{
	Editor *editor = (Editor *)li_ptr;

	if ((result != GNOME_VFS_OK) && (result != GNOME_VFS_ERROR_EOF)) {
		g_print(_("VFS Error: %s\n"), gnome_vfs_result_to_string (result));
		//gnome_vfs_print_error(result, editor->filename->str);
		return;
	}

	// Clear scintilla buffer
	gtk_scintilla_clear_all(GTK_SCINTILLA (editor->scintilla));
	
	//g_print("BUFFER=\n%s\n-------------------------------------------\n", buffer);
	
	tab_validate_buffer_and_insert(buffer, editor);
	tab_reset_scintilla_after_open(editor);
	
	g_free(buffer);
	gnome_vfs_async_close(fd, tab_file_closed, li_ptr);
	if (gotoline_after_reload) {
		goto_line_int(gotoline_after_reload);
		gotoline_after_reload = 0;
	}
	tab_check_php_file(editor);
}

void tab_file_opened(GnomeVFSAsyncHandle *fd, GnomeVFSResult result, gpointer li_ptr)
{
	gpointer buffer;
	Editor *editor = (Editor *)li_ptr;
	
	if (result != GNOME_VFS_OK) {
		g_print(_("VFS Error: %s\n"), gnome_vfs_result_to_string (result));
		//gnome_vfs_print_error(result, editor->filename->str);
		return;
	}
	
	buffer = g_malloc(editor->file_size);
	gnome_vfs_async_read(fd, buffer, editor->file_size, tab_file_read, li_ptr);
}

 void tab_load_file(Editor *editor)
{
	GnomeVFSFileInfo *file_stats;
	GnomeVFSResult result;
	GnomeVFSAsyncHandle *fd;
	
	// Store current position for returning to
	editor->current_pos = gtk_scintilla_get_current_pos(GTK_SCINTILLA(editor->scintilla));
	editor->current_line = gtk_scintilla_line_from_position(GTK_SCINTILLA(editor->scintilla), editor->current_pos);

	// Try getting file size
	file_stats = gnome_vfs_file_info_new();
	result = gnome_vfs_get_file_info(editor->filename->str, file_stats, GNOME_VFS_FILE_INFO_FOLLOW_LINKS);
	if (result != GNOME_VFS_OK) {
		// Print error and return if no
		g_print(_("VFS Error: %s\n"), gnome_vfs_result_to_string (result));
		//gnome_vfs_print_error(result, editor->filename->str);
		return;
	}
	
	editor->file_size = file_stats->size;
	
	// Open file
	gnome_vfs_async_open(&fd, editor->filename->str, GNOME_VFS_OPEN_READ, GNOME_VFS_PRIORITY_DEFAULT, tab_file_opened, editor);
}


Editor *tab_new_editor(void)
{
	Editor *editor;
	editor = (Editor *)g_malloc0(sizeof(Editor));
	editors = g_slist_append(editors, editor);
	return editor;
}


void str_replace(char *Str, char ToRp, char WithC)
{
	int i = 0;

	while(i < strlen(Str)) {
		if(Str[i] == ToRp) {
			Str[i] = WithC;
		}
		i++;
	}

}


void tab_help_load_file(Editor *editor, GString *filename)
{
	struct stat st;
	size_t size;
	FILE *fp;
	gchar *buffer;
	guint nchars;
 
	if (stat (filename->str, &st) != 0)
	{
		g_warning (_("Could not stat the file %s"), filename->str);
		//die();
	}
	size = st.st_size;
 
	buffer = (gchar *)g_malloc (size);
	if (buffer == NULL && size != 0)
	{
		// This is funny in unix, but never hurts 
		g_warning (_("This file is too big. Unable to allocate memory."));
		//die();
	}
	fp = fopen (filename->str, "rb");
	if (!fp)
	{
		g_free (buffer);
		//die();
	}
	
	// Crude way of loading, but faster
	nchars = fread (buffer, 1, size, fp);
	
	if (size != nchars) g_warning (_("File size and loaded size not matching"));
	
	html_document_clear(editor->help_document);
	html_document_open_stream(editor->help_document, "text/html");
	html_document_write_stream(editor->help_document, buffer, nchars);
	html_document_close_stream(editor->help_document);
 
	g_free (buffer);
	fclose (fp);
	
}


/*static gboolean tab_help_url_requested(GtkWidget * html, const gchar * url, gpointer stream)
{
	Editor *data;
	GString *filename;

	g_print ("REQUESTED %s\n", url);

	filename = g_string_new(url);

	data = editor_find_from_help(html);
	tab_help_load_file(data, filename);

	data->filename = g_string_new(filename->str);
	data->filename = g_string_prepend(data->filename, "Help: ");

	data->short_filename = data->filename->str;
	update_app_title;
	return TRUE;
}*/

GString *tab_help_try_filename(gchar *prefix, gchar *command, gchar *suffix)
{
	GString *long_filename;
	
	long_filename = g_string_new(command);
	long_filename = g_string_prepend(long_filename, prefix);
	if (suffix) {
		long_filename = g_string_append(long_filename, suffix);
	}
	if (DEBUG_MODE) { g_print("DEBUG: tab.c:tab_help_try_filename:long_filename->str: %s\n", long_filename->str); }
	if (g_file_exists(long_filename->str)) {
		return long_filename;
	}
	else {
		g_string_free(long_filename, TRUE);
	}

	long_filename = g_string_new(command);
	str_replace(long_filename->str, '_', '-');
	long_filename = g_string_prepend(long_filename, prefix);
	if (suffix) {
		long_filename = g_string_append(long_filename, suffix);
	}
	if (DEBUG_MODE) { g_print("DEBUG: tab.c:tab_help_try_filename:long_filename->str: %s\n", long_filename->str); }
	if (g_file_exists(long_filename->str)) {
		return long_filename;
	}

	g_string_free(long_filename, TRUE);
	return NULL;
}


GString *tab_help_find_helpfile(gchar *command)
{
	if(G_STR_EMPTY(PHP_MANUAL_DIR)){
		g_print(_("Help for function not available: %s.  Support for PHP's Manual was not compiled.\n"), command);
		return NULL;
	}
	
	GString *long_filename=NULL;
	if(long_filename=tab_help_try_filename(PHP_MANUAL_DIR "/function.", command, ".html"))
		return long_filename;
	if(long_filename=tab_help_try_filename(PHP_MANUAL_DIR "/ref.", command, ".html"))
		return long_filename;
	if(long_filename=tab_help_try_filename(PHP_MANUAL_DIR "/", command, NULL))
		return long_filename;

	g_print(_("Help for function not found: %s\n"), command);
	return NULL;
}


static void tab_help_link_clicked(GObject *obj, const gchar *url)
{
	Editor *editor;
	GString *filename;

	filename = tab_help_find_helpfile((gchar *)url);
	
	if (filename) {
		editor = editor_find_from_help((HtmlDocument *)obj);
		tab_help_load_file(editor, filename);

		g_string_free(editor->filename, TRUE);
		
		editor->filename = g_string_new(filename->str);
		editor->filename = g_string_prepend(editor->filename, _("Help: "));

		editor->short_filename = editor->filename->str;
		
		gtk_label_set_text(GTK_LABEL(editor->label), editor->short_filename);
		
		update_app_title();
	}
}


gboolean tab_create_help(Editor *editor, GString *filename)
{
	GString *caption;
	GString *long_filename = NULL;
	GtkWidget *dialog;
 
	caption = g_string_new(filename->str);
	caption = g_string_prepend(caption, _("Help: "));

	long_filename = tab_help_find_helpfile(filename->str);
	if (!long_filename) {
		dialog = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
			_("Could not find the required command in the online help"));
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		return FALSE;
	}
	else {
		editor->short_filename = caption->str;
		editor->help_function = g_strdup(filename->str);
		editor->filename = caption;
		editor->label = gtk_label_new (caption->str);
		editor->is_untitled = FALSE;
		editor->saved = TRUE;
		gtk_widget_show (editor->label);
	
		editor->help_document = html_document_new();
		editor->help_view = html_view_new();
		editor->help_scrolled_window = gtk_scrolled_window_new(NULL, NULL);
		gtk_container_add(GTK_CONTAINER(editor->help_scrolled_window), editor->help_view);
	
		tab_help_load_file(editor, long_filename);
	
		g_signal_connect(G_OBJECT(editor->help_document), "link-clicked",
			 G_CALLBACK(tab_help_link_clicked), NULL);

		html_view_set_document(HTML_VIEW(editor->help_view), editor->help_document);
		gtk_widget_show_all(editor->help_scrolled_window);
		
		gtk_notebook_append_page (GTK_NOTEBOOK (main_window.notebook_editor), editor->help_scrolled_window, editor->label);
		gtk_notebook_set_current_page (GTK_NOTEBOOK (main_window.notebook_editor), -1);
	}
	return TRUE;
}

void info_dialog (gchar *title, gchar *message)
{
	GtkWidget *dialog, *label;
	int button;

	dialog = gnome_dialog_new (
	             title,
	             GNOME_STOCK_BUTTON_OK,
	             NULL);
	gnome_dialog_set_parent(GNOME_DIALOG (dialog), GTK_WINDOW(main_window.window));

	label = gtk_label_new (message);
	gtk_widget_show(label);
	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (dialog)->vbox), label, TRUE, TRUE, 0);

	/*
	 * Run the dialog and wait for the user to select yes or no.
	 * If the user closes the window with the window manager, we
	 * will get a -1 return value
	 */
	button = gnome_dialog_run_and_close (GNOME_DIALOG (dialog));
	
}

gint yes_no_dialog (gchar *title, gchar *message)
{
	GtkWidget *dialog, *label;
	int button;

	dialog = gnome_dialog_new (
	             title,
	             GNOME_STOCK_BUTTON_YES,
	             GNOME_STOCK_BUTTON_NO,
	             NULL);
	gnome_dialog_set_parent(GNOME_DIALOG (dialog), GTK_WINDOW(main_window.window));

	label = gtk_label_new (message);
	gtk_widget_show(label);
	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (dialog)->vbox), label, TRUE, TRUE, 0);

	/*
	 * Run the dialog and wait for the user to select yes or no.
	 * If the user closes the window with the window manager, we
	 * will get a -1 return value
	 */
	button = gnome_dialog_run_and_close (GNOME_DIALOG (dialog));

	return button;
}

gboolean is_php_file(Editor *editor)
{
	// New style function for configuration of what constitutes a PHP file
	gchar *file_extension;
	gchar **php_file_extensions;
	gboolean is_php = FALSE;
	gint i;
	gchar *buffer = NULL;
	gsize text_length;
	gchar **lines;
	gchar *filename;

	filename = editor->filename->str;

	file_extension = strrchr(filename, '.');
	if (file_extension) {
		file_extension++;
		
		php_file_extensions = g_strsplit(preferences.php_file_extensions,",",-1);
		
        for (i = 0; php_file_extensions[i] != NULL; i++) {
			if (strcmp(file_extension, php_file_extensions[i]) == 0) {
				is_php = TRUE;
				break;
			}
		}
				
		g_strfreev(php_file_extensions);
	}
	
	if (!is_php && editor) { // If it's not recognised as a PHP file, examine the contents for <?php and #!.*php
		
		text_length = gtk_scintilla_get_length(GTK_SCINTILLA(editor->scintilla));
		buffer = g_malloc0(text_length+1); // Include terminating null
		if (buffer == NULL) {
			g_warning ("%s", "Cannot allocate write buffer");
			return is_php;
		}
		gtk_scintilla_get_text(GTK_SCINTILLA(editor->scintilla), text_length+1, buffer);
		lines = g_strsplit(buffer, "\n", 10);
		if (lines[0][0] == '#' && lines[0][1] == '!' && strstr(lines[0], "php") != NULL) {
			is_php = TRUE;
		}
		else {
			for (i = 0; lines[i+1] != NULL; i++) {
				if (strstr (lines[i], "<?php") != NULL) {
					is_php = TRUE;
					break;
				}
			}
		}
		g_strfreev(lines);
	}
	
	return is_php;
}

gboolean is_php_file_from_filename(gchar *filename)
{
	// New style function for configuration of what constitutes a PHP file
	gchar *file_extension;
	gchar **php_file_extensions;
	gboolean is_php = FALSE;
	gint i;

	file_extension = strrchr(filename, '.');
	if (file_extension) {
		file_extension++;
		
		php_file_extensions = g_strsplit(preferences.php_file_extensions,",",-1);
		
        for (i = 0; php_file_extensions[i] != NULL; i++) {
			if (strcmp(file_extension, php_file_extensions[i]) == 0) {
				is_php = TRUE;
				break;
			}
		}
				
		g_strfreev(php_file_extensions);
	}
	
	return is_php;
}



gboolean is_css_file(gchar *filename)
{
	gchar *file_extension;

	file_extension = strrchr(filename, '.');
	if (file_extension) {
		file_extension++;
		if (strcmp(file_extension, "css") == 0) {
			return TRUE;
		}
	}
	return FALSE;
}

gboolean is_perl_file(gchar *filename)
{
	gchar *file_extension;

	file_extension = strrchr(filename, '.');
	if (file_extension) {
		file_extension++;
		if (strcmp(file_extension, "pl") == 0) {
			return TRUE;
		}
	}
	return FALSE;
}

gboolean is_python_file(gchar *filename)
{
	gchar *file_extension;

	file_extension = strrchr(filename, '.');
	if (file_extension) {
		file_extension++;
		if (strcmp(file_extension, "py") == 0) {
			return TRUE;
		}
	}
	return FALSE;
}

gboolean is_cxx_file(gchar *filename)
{
	gchar *file_extension;

	file_extension = strrchr(filename, '.');
	if (file_extension) {
		file_extension++;
		if (strcmp(file_extension, "cxx") == 0 ||
			strcmp(file_extension, "c") == 0 ||
			strcmp(file_extension, "h") == 0) {
			return TRUE;
		}
	}
	return FALSE;
}

gboolean is_sql_file(gchar *filename)
{
	gchar *file_extension;

	file_extension = strrchr(filename, '.');
	if (file_extension) {
		file_extension++;
		if (strcmp(file_extension, "sql") == 0) {
			return TRUE;
		}
	}
	return FALSE;
}

void set_editor_to_php(Editor *editor)
{
	tab_php_set_lexer(editor);
	editor->type = TAB_PHP;
	tab_set_folding(editor, TRUE);
}

void set_editor_to_css(Editor *editor)
{
	tab_css_set_lexer(editor);
	editor->type = TAB_CSS;
}

void set_editor_to_sql(Editor *editor)
{
	tab_sql_set_lexer(editor);
	editor->type = TAB_SQL;
}

void set_editor_to_cxx(Editor *editor)
{
	tab_cxx_set_lexer(editor);
	editor->type = TAB_CXX;
}

void set_editor_to_perl(Editor *editor)
{
	tab_perl_set_lexer(editor);
	editor->type = TAB_PERL;
}

void set_editor_to_python(Editor *editor)
{
	tab_python_set_lexer(editor);
	editor->type = TAB_PYTHON;
}

void tab_check_php_file(Editor *editor)
{
	if (is_php_file(editor)) {
		set_editor_to_php(editor);
	}
}


void tab_check_css_file(Editor *editor)
{
	if (is_css_file(editor->filename->str)) {
		set_editor_to_css(editor);
	}
}

void tab_check_perl_file(Editor *editor)
{
	if (is_perl_file(editor->filename->str)) {
		set_editor_to_perl(editor);
	}
}

void tab_check_python_file(Editor *editor)
{
	if (is_python_file(editor->filename->str)) {
		set_editor_to_python(editor);
	}
}

void tab_check_cxx_file(Editor *editor)
{
	if (is_cxx_file(editor->filename->str)) {
		set_editor_to_cxx(editor);
	}
}

void tab_check_sql_file(Editor *editor)
{
	if (is_sql_file(editor->filename->str)) {
		set_editor_to_sql(editor);
	}
}


void register_file_opened(gchar *filename)
{
	GString *tmp_filename;
	GString *folder;
	
	main_window_add_to_reopen_menu(filename);

	tmp_filename = g_string_new(filename);

	folder = get_folder(tmp_filename);
	gnome_config_set_string("connectED/general/last_opened_folder",  folder->str);
	g_string_free(folder, TRUE);
	gnome_config_sync();

	g_string_free(tmp_filename, TRUE);
}

gboolean switch_to_file_or_open(gchar *filename, gint line_number)
{
	Editor *editor;
	GSList *walk;
	GString *tmp_filename;
	
	// need to check if filename is local before adding to the listen
	filename = convert_to_full(filename);
	
	for (walk = editors; walk!=NULL; walk = g_slist_next(walk)) {
		editor = walk->data;
		if (strcmp(editor->filename->str, filename)==0) {
			gtk_notebook_set_current_page( GTK_NOTEBOOK(main_window.notebook_editor), gtk_notebook_page_num(GTK_NOTEBOOK(main_window.notebook_editor),editor->scintilla));
			main_window.current_editor = editor;
			gotoline_after_reload = line_number;
			on_reload1_activate(NULL);
			return TRUE;
		}
	}

	gotoline_after_create_tab = line_number;
	tmp_filename = g_string_new(filename);
	tab_create_new(TAB_FILE, tmp_filename);

	g_string_free(tmp_filename, TRUE);
	
	register_file_opened(filename);
	session_save();
	return TRUE;
}


void close_saved_empty_Untitled()
{
	Editor *editor;
	gint length;

	if (editors) {
		editor = editors->data;
		if (editor->is_untitled && editor->saved) {
			length = gtk_scintilla_get_length(GTK_SCINTILLA(editor->scintilla));
			if (length == 0) {
				try_close_page(editor);
			}
		}
	}
}


/* return TRUE if a tab was created */
gboolean tab_create_new(gint type, GString *filename)
{
	Editor *editor;
	GString *dialog_message;
	gchar abs_buffer[2048];
	gchar *abs_path = NULL;
	gchar *cwd;
	gboolean result;
	gboolean file_created = FALSE;
	GnomeVFSURI *uri = NULL;
//	gchar * buffer = NULL;

	if (DEBUG_MODE) { g_print("DEBUG: tab.c:tab_create_new:filename->str: %s\n", filename->str); }
	if (filename != NULL) {
		if (strstr(filename->str, ":")==NULL) {
			cwd = g_get_current_dir();
			abs_path = g_strdup(g_rel2abs (filename->str, cwd, abs_buffer, 2048));
			g_free(cwd);
		}
		else {
			abs_path = g_strdup(filename->str);
		}

		uri = gnome_vfs_uri_new(abs_path);
		if (!gnome_vfs_uri_exists(uri) && type!=TAB_HELP) {
			dialog_message = g_string_new("");
			g_string_sprintf(dialog_message, _("The file %s was not found.\n\nWould you like to create it as an empty document?"), filename->str);
			result = yes_no_dialog(_("File not found"), dialog_message->str);
			g_string_free(dialog_message, TRUE);
			gnome_vfs_uri_unref(uri);
			if (result != 0) {
				return FALSE;
			}
			file_created = TRUE;
		}
	}
	close_saved_empty_Untitled();
	
	editor = tab_new_editor();
	editor->type = type;

	if (editor->type == TAB_HELP) {
		if (!tab_create_help(editor, filename)) {
			// Couldn't find the help file, don't keep the editor
			editors = g_slist_remove(editors, editor);
		}
		else {
			editor->saved=TRUE;
		}
	}
	else {
		editor->type = TAB_FILE;
		editor->scintilla = gtk_scintilla_new();
		tab_set_general_scintilla_properties(editor);

		editor->label = gtk_label_new (_("Untitled"));
		gtk_widget_show (editor->label);

		if (abs_path != NULL) {
			editor->filename = g_string_new(abs_path);
			editor->short_filename = g_path_get_basename(editor->filename->str);
			if (!file_created) {
				tab_load_file(editor);
			}

			//tab_check_php_file(editor);
			tab_check_css_file(editor);
			tab_check_cxx_file(editor);
			tab_check_perl_file(editor);
			tab_check_python_file(editor);
			tab_check_sql_file(editor);
			classbrowser_update();
			editor->is_untitled=FALSE;
		}
		else {
			editor->filename = g_string_new(_("Untitled"));
			editor->short_filename = g_strdup(editor->filename->str);
			if (main_window.current_editor) {
				editor->opened_from = get_folder(main_window.current_editor->filename);
			}
			editor->is_untitled=TRUE;
		}

		// Hmmm, I had the same error as the following comment.  A reshuffle here and upgrading GtkScintilla2 to 0.1.0 seems to have fixed it
		if (!GTK_WIDGET_VISIBLE (editor->scintilla))
			gtk_widget_show (editor->scintilla);
		gtk_notebook_append_page (GTK_NOTEBOOK (main_window.notebook_editor), editor->scintilla, editor->label);
 		gtk_scintilla_set_save_point(GTK_SCINTILLA(editor->scintilla));
		tab_set_event_handlers(editor);
		
		/* Possible problem on the next line, one user reports: 
			assertion `GTK_WIDGET_ANCHORED (widget) || GTK_IS_INVISIBLE (widget)' failed */
		gtk_notebook_set_current_page (GTK_NOTEBOOK (main_window.notebook_editor), -1);

		gtk_scintilla_goto_pos(GTK_SCINTILLA(editor->scintilla), 0);
		gtk_scintilla_grab_focus(GTK_SCINTILLA(editor->scintilla));
		main_window.current_editor = editor;
		
		if (gotoline_after_create_tab) {
			goto_line_int(gotoline_after_create_tab);
			gotoline_after_create_tab = 0;
		}
		editor->saved=TRUE;
	}

	update_app_title();

	g_free(abs_path);
	
	return TRUE;
}


Editor *editor_find_from_scintilla(GtkWidget *scintilla)
{
	GSList *walk;
	Editor *editor;

	for (walk = editors; walk != NULL; walk = g_slist_next (walk)) {
		editor = walk->data;
		if (editor->scintilla == scintilla) {
			return walk->data;
		}
	}

	return NULL;
}


Editor *editor_find_from_help(void *help)
{
	GSList *walk;
	Editor *editor;

	for (walk = editors; walk != NULL; walk = g_slist_next (walk)) {
		editor = walk->data;
		//if (((void *)(editor->help_document) == (void *)help) || ((void *)(editor->help_scrolled_window) == (void *)help)) {
		if ((void *)(editor->help_document) == help || (void *)(editor->help_scrolled_window == help)) {
			return walk->data;
		}
	}

	return NULL;
}

static void save_point_reached(GtkWidget *scintilla)
{
	GString *label_caption;
	Editor *editor;

	editor = editor_find_from_scintilla(scintilla);
	if (editor->short_filename != NULL) {
		label_caption = g_string_new("<span color=\"black\">");
		label_caption = g_string_append(label_caption, editor->short_filename);
		label_caption = g_string_append(label_caption, "</span>");
		gtk_label_set_markup(GTK_LABEL (editor->label), label_caption->str);
		g_string_free(label_caption, TRUE);
		editor->saved=TRUE;
		update_app_title();
	}
}

static void save_point_left(GtkWidget *scintilla)
{
	GString *label_caption;
	Editor *editor;

	editor = editor_find_from_scintilla(scintilla);
	if (editor->short_filename != NULL) {
		label_caption = g_string_new("<span color=\"red\">");
		label_caption = g_string_append(label_caption, editor->short_filename);
		label_caption = g_string_append(label_caption, "</span>");
		gtk_label_set_markup(GTK_LABEL (editor->label), label_caption->str);
		g_string_free(label_caption, TRUE);
		editor->saved=FALSE;
		update_app_title();
	}
}

// All the folding functions are converted from QScintilla, released under the GPLv2 by
// Riverbank Computing Limited <info@riverbankcomputing.co.uk> and Copyright (c) 2003 by them.
void fold_clicked(GtkWidget *scintilla, guint lineClick, guint bstate)
{
	gint levelClick;
	 
	levelClick = gtk_scintilla_get_fold_level(GTK_SCINTILLA(scintilla), lineClick);

	if (levelClick & SC_FOLDLEVELHEADERFLAG)
	{
		if (bstate & SCMOD_SHIFT) {
			// Ensure all children are visible.
			gtk_scintilla_set_fold_expanded(GTK_SCINTILLA(scintilla), lineClick, 1);
			fold_expand(scintilla, lineClick, TRUE, TRUE, 100, levelClick);
		}
		else if (bstate & SCMOD_CTRL) {
			if (gtk_scintilla_get_fold_expanded(GTK_SCINTILLA(scintilla), lineClick)) {
				// Contract this line and all its children.
				gtk_scintilla_set_fold_expanded(GTK_SCINTILLA(scintilla), lineClick, 0);
				fold_expand(scintilla, lineClick, FALSE, TRUE, 0, levelClick);
			}
			else {
				// Expand this line and all its children.
				gtk_scintilla_set_fold_expanded(GTK_SCINTILLA(scintilla), lineClick, 1);
				fold_expand(scintilla, lineClick, TRUE, TRUE, 100, levelClick);
			}
		}
		else {
			// Toggle this line.
			gtk_scintilla_toggle_fold(GTK_SCINTILLA(scintilla), lineClick);
		}
	}
}

// All the folding functions are converted from QScintilla, released under the GPLv2 by
// Riverbank Computing Limited <info@riverbankcomputing.co.uk> and Copyright (c) 2003 by them.
void fold_expand(GtkWidget *scintilla, gint line, gboolean doExpand, gboolean force, gint visLevels, gint level)
{
	gint lineMaxSubord;
	gint levelLine;
	
	lineMaxSubord = gtk_scintilla_get_last_child(GTK_SCINTILLA(scintilla), line, level & SC_FOLDLEVELNUMBERMASK);

	line++;

	while (line <= lineMaxSubord) {
		if (force) {
			if (visLevels > 0) {
				gtk_scintilla_show_lines(GTK_SCINTILLA(scintilla), line, line);
			}
			else {
				gtk_scintilla_hide_lines(GTK_SCINTILLA(scintilla), line, line);
			}
		}
		else if (doExpand) {
			gtk_scintilla_show_lines(GTK_SCINTILLA(scintilla), line, line);
		}

		levelLine = level;

		if (levelLine == -1) {
			levelLine = gtk_scintilla_get_fold_level(GTK_SCINTILLA(scintilla), line);
		}

		if (levelLine & SC_FOLDLEVELHEADERFLAG) {
			if (force) {
				if (visLevels > 1) {
					gtk_scintilla_set_fold_expanded(GTK_SCINTILLA(scintilla), line, 1);
				}
				else {
					gtk_scintilla_set_fold_expanded(GTK_SCINTILLA(scintilla), line, 0);
				}

				fold_expand(scintilla, line, doExpand, force, visLevels - 1, 0); // Added last 0 param - AJ
			}
			else if (doExpand) {
				if (!gtk_scintilla_get_fold_expanded(GTK_SCINTILLA(scintilla), line)) {
					gtk_scintilla_set_fold_expanded(GTK_SCINTILLA(scintilla), line, 1);
				}

				fold_expand(scintilla, line, TRUE, force, visLevels - 1, 0); // Added last 0 param - AJ
			}
			else {
				fold_expand(scintilla, line, FALSE, force, visLevels - 1, 0); // Added last 0 param - AJ
			}
		}
		else {
			line++;
		}
	}
}

// All the folding functions are converted from QScintilla, released under the GPLv2 by
// Riverbank Computing Limited <info@riverbankcomputing.co.uk> and Copyright (c) 2003 by them.
void fold_changed(GtkWidget *scintilla, int line,int levelNow,int levelPrev)
{
        if (levelNow & SC_FOLDLEVELHEADERFLAG) {
                if (!(levelPrev & SC_FOLDLEVELHEADERFLAG))
                        gtk_scintilla_set_fold_expanded(GTK_SCINTILLA(scintilla), line, 1);
        }
	else if (levelPrev & SC_FOLDLEVELHEADERFLAG) {
                if (!gtk_scintilla_get_fold_expanded(GTK_SCINTILLA(scintilla), line)) {
                        // Removing the fold from one that has been contracted
			// so should expand.  Otherwise lines are left
			// invisible with no way to make them visible.
                        fold_expand(scintilla, line, TRUE, FALSE, 0, levelPrev);
                }
        }
}

// All the folding functions are converted from QScintilla, released under the GPLv2 by
// Riverbank Computing Limited <info@riverbankcomputing.co.uk> and Copyright (c) 2003 by them.
void handle_modified(GtkWidget *scintilla, gint pos,gint mtype,gchar *text,gint len,
				   gint added,gint line,gint foldNow,gint foldPrev)
{
	if (preferences.show_folding && (mtype & SC_MOD_CHANGEFOLD)) {
		fold_changed(scintilla, line, foldNow, foldPrev);
	}
}

void margin_clicked (GtkWidget *scintilla, gint modifiers, gint position, gint margin)
{
	gint line;
	
	line = gtk_scintilla_line_from_position(GTK_SCINTILLA(scintilla), position);

	if (preferences.show_folding && margin == 2) {
		fold_clicked(scintilla, line, modifiers);
	}
}


char *macro_message_to_string(gint message)
{
	switch (message) {
		case (2170) : return "REPLACE SELECTION";
		case (2177) : return "CLIPBOARD CUT";
		case (2178) : return "CLIPBOARD COPY";
		case (2179) : return "CLIPBOARD PASTE";
		case (2180) : return "CLEAR";
		case (2300) : return "LINE DOWN";
		case (2301) : return "LINE DOWN EXTEND";
		case (2302) : return "LINE UP";
		case (2303) : return "LINE UP EXTEND";
		case (2304) : return "CHAR LEFT";
		case (2305) : return "CHAR LEFT EXTEND";
		case (2306) : return "CHAR RIGHT";
		case (2307) : return "CHAR RIGHT EXTEND";
		case (2308) : return "WORD LEFT";
		case (2309) : return "WORD LEFT EXTEND";
		case (2310) : return "WORD RIGHT";
		case (2311) : return "WORD RIGHT EXTEND";
		case (2312) : return "HOME";
		case (2313) : return "HOME EXTEND";
		case (2314) : return "LINE END";
		case (2315) : return "LINE END EXTEND";
		case (2316) : return "DOCUMENT START";
		case (2317) : return "DOCUMENT START EXTEND";
		case (2318) : return "DOCUMENT END";
		case (2319) : return "DOCUMENT END EXTEND";
		case (2320) : return "PAGE UP";
		case (2321) : return "PAGE UP EXTEND";
		case (2322) : return "PAGE DOWN";
		case (2323) : return "PAGE DOWN EXTEND";
		default:
			return "-- UNKNOWN --";
		/*case (2324) : gtk_scintilla_edit_toggle_overtype(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
		case (2325) : gtk_scintilla_cancel(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
		case (2326) : gtk_scintilla_delete_back(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
		case (2327) : gtk_scintilla_tab(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
		case (2328) : gtk_scintilla_back_tab(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
		case (2329) : gtk_scintilla_new_line(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
		case (2330) : gtk_scintilla_form_feed(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
		case (2331) : gtk_scintilla_v_c_home(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
		case (2332) : gtk_scintilla_v_c_home_extend(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
		case (2333) : gtk_scintilla_zoom_in(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
		case (2334) : gtk_scintilla_zoom_out(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
		case (2335) : gtk_scintilla_del_word_left(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
		case (2336) : gtk_scintilla_del_word_right(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
		case (2337) : gtk_scintilla_line_cut(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
		case (2338) : gtk_scintilla_line_delete(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
		case (2339) : gtk_scintilla_line_transpose(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
		case (2340) : gtk_scintilla_lower_case(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
		case (2341) : gtk_scintilla_upper_case(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
		case (2342) : gtk_scintilla_line_scroll_down(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
		case (2343) : gtk_scintilla_line_scroll_up(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
		case (2344) : gtk_scintilla_delete_back_not_line(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
		case (2345) : gtk_scintilla_home_display(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
		case (2346) : gtk_scintilla_home_display_extend(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
		case (2347) : gtk_scintilla_line_end_display(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
		case (2348) : gtk_scintilla_line_end_display_extend(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
		default:
			g_print("Unhandle keyboard macro function %d, please report to uberChick@uberChicGeekChick.Com\n", event->message);*/
	}
}

void macro_record (GtkWidget *scintilla, gint message, gulong wparam, glong lparam)
{
	Editor *editor;
	MacroEvent *event;
	
	editor = editor_find_from_scintilla(scintilla);
	if (editor->is_macro_recording) {
		event = g_new0(MacroEvent, 1);
		event->message = message;
		event->wparam = wparam;
		// Special handling for text inserting, duplicate inserted string
		if (event->message == 2170 && editor->is_pasting) {
			event->message = 2179;
		}
		else if (event->message == 2170) {
			event->lparam = (glong) g_strdup((gchar *)lparam);
		}
		else {
			event->lparam = lparam;
		}
		if (DEBUG_MODE) { g_print("DEBUG: tab.c:macro_record:Message: %d (%s)\n", event->message, macro_message_to_string(event->message)); }
		
		editor->keyboard_macro_list = g_slist_append(editor->keyboard_macro_list, event);
	}
}

void keyboard_macro_empty_old(Editor *editor)
{
	GSList *current;
	MacroEvent *event;

	for (current = editor->keyboard_macro_list; current; current = g_slist_next(current)) {
		event = current->data;
		if (event->message == 2170) {
			// Special handling for text inserting, free inserted string
			g_free((gchar *)event->lparam);
		}
		g_free(event);
	}
	g_slist_free(editor->keyboard_macro_list);
	editor->keyboard_macro_list = NULL;
}

static void indent_line(GtkWidget *scintilla, gint line, gint indent)
{
	gint selStart;
	gint selEnd;
	gint posBefore;
	gint posAfter;
	gint posDifference;

	selStart = gtk_scintilla_get_selection_start(GTK_SCINTILLA(scintilla));
	selEnd = gtk_scintilla_get_selection_end(GTK_SCINTILLA(scintilla));
	posBefore = gtk_scintilla_get_line_indentation(GTK_SCINTILLA(scintilla), line);
	gtk_scintilla_set_line_indentation(GTK_SCINTILLA(scintilla), line, indent);
	posAfter = gtk_scintilla_get_line_indentation(GTK_SCINTILLA(scintilla), line);
	posDifference =  posAfter - posBefore;

	if (posAfter > posBefore) {
		// Move selection on
		if (selStart >= posBefore) {
			selStart += posDifference;
		}
		if (selEnd >= posBefore) {
			selEnd += posDifference;
		}
	}
	else if (posAfter < posBefore) {
		// Move selection back
		if (selStart >= posAfter) {
			if (selStart >= posBefore)
				selStart += posDifference;
			else
				selStart = posAfter;
		}
		if (selEnd >= posAfter) {
			if (selEnd >= posBefore)
				selEnd += posDifference;
			else
				selEnd = posAfter;
		}
	}
	gtk_scintilla_set_selection_start(GTK_SCINTILLA(scintilla), selStart);
	gtk_scintilla_set_selection_end(GTK_SCINTILLA(scintilla), selEnd);
}


gboolean auto_complete_callback(gpointer data)
{
	gint wordStart;
	gint wordEnd;
	gint current_pos;

	current_pos = gtk_scintilla_get_current_pos(GTK_SCINTILLA(main_window.current_editor->scintilla));

	if (current_pos == (gint)data) {
		wordStart = gtk_scintilla_word_start_position(GTK_SCINTILLA(main_window.current_editor->scintilla), current_pos-1, TRUE);
		wordEnd = gtk_scintilla_word_end_position(GTK_SCINTILLA(main_window.current_editor->scintilla), current_pos-1, TRUE);
		autocomplete_word(main_window.current_editor->scintilla, wordStart, wordEnd);
	}
	completion_timer_set=FALSE;
	return FALSE;
}


gboolean css_auto_complete_callback(gpointer data)
{
	gint wordStart;
	gint wordEnd;
	gint current_pos;

	current_pos = gtk_scintilla_get_current_pos(GTK_SCINTILLA(main_window.current_editor->scintilla));

	if (current_pos == (gint)data) {
		wordStart = gtk_scintilla_word_start_position(GTK_SCINTILLA(main_window.current_editor->scintilla), current_pos-1, TRUE);
		wordEnd = gtk_scintilla_word_end_position(GTK_SCINTILLA(main_window.current_editor->scintilla), current_pos-1, TRUE);
		css_autocomplete_word(main_window.current_editor->scintilla, wordStart, wordEnd);
	}
	completion_timer_set=FALSE;
	return FALSE;
}


gboolean sql_auto_complete_callback(gpointer data)
{
	gint wordStart;
	gint wordEnd;
	gint current_pos;

	current_pos = gtk_scintilla_get_current_pos(GTK_SCINTILLA(main_window.current_editor->scintilla));

	if (current_pos == (gint)data) {
		wordStart = gtk_scintilla_word_start_position(GTK_SCINTILLA(main_window.current_editor->scintilla), current_pos-1, TRUE);
		wordEnd = gtk_scintilla_word_end_position(GTK_SCINTILLA(main_window.current_editor->scintilla), current_pos-1, TRUE);
		sql_autocomplete_word(main_window.current_editor->scintilla, wordStart, wordEnd);
	}
	completion_timer_set=FALSE;
	return FALSE;
}


gboolean auto_memberfunc_complete_callback(gpointer data)
{
	gint wordStart;
	gint wordEnd;
	gint current_pos;

	current_pos = gtk_scintilla_get_current_pos(GTK_SCINTILLA(main_window.current_editor->scintilla));

	if (current_pos == (gint)data) {
		wordStart = gtk_scintilla_word_start_position(GTK_SCINTILLA(main_window.current_editor->scintilla), current_pos-1, TRUE);
		wordEnd = gtk_scintilla_word_end_position(GTK_SCINTILLA(main_window.current_editor->scintilla), current_pos-1, TRUE);
		autocomplete_member_function(main_window.current_editor->scintilla, wordStart, wordEnd);
	}
	completion_timer_set=FALSE;
	return FALSE;
}

gboolean calltip_callback(gpointer data)
{
	gint current_pos;

	current_pos = gtk_scintilla_get_current_pos(GTK_SCINTILLA(main_window.current_editor->scintilla));

	if (current_pos == (gint)data) {
		show_call_tip(main_window.current_editor->scintilla, current_pos);
	}
	calltip_timer_set=FALSE;
	return FALSE;

}


void update_ui(GtkWidget *scintilla)
{
	// ----------------------------------------------------
	// This code is based on that found in SciTE
	// Converted by AJ 2004-03-04
	// ----------------------------------------------------
	
	int current_brace_pos = -1;
	int matching_brace_pos = -1;
	int current_brace_column = -1;
	int matching_brace_column = -1;
	int current_line;
	int current_pos;
	char character_before = '\0';
	char character_after = '\0';
	//char style_before = '\0';
	
	if (gtk_scintilla_call_tip_active(GTK_SCINTILLA(scintilla))) {
		gtk_scintilla_call_tip_cancel(GTK_SCINTILLA(scintilla));
	}
		
	current_pos = gtk_scintilla_get_current_pos(GTK_SCINTILLA(scintilla));
	current_line = gtk_scintilla_line_from_position(GTK_SCINTILLA(scintilla), gtk_scintilla_get_current_pos(GTK_SCINTILLA(scintilla)));
	
	//FindMatchingBracePosition
	if (current_pos > 0) {
		character_before = gtk_scintilla_get_char_at(GTK_SCINTILLA(scintilla), current_pos-1);
		// style_before = gtk_scintilla_get_style_at(GTK_SCINTILLA(scintilla), current_pos-1);
		if (character_before && strchr("[](){}", character_before)) {
			current_brace_pos = current_pos-1;
		}
	}
	
	if (current_brace_pos < 0) {
		character_after = gtk_scintilla_get_char_at(GTK_SCINTILLA(scintilla), current_pos+1);
		// style_before = gtk_scintilla_get_style_at(GTK_SCINTILLA(scintilla), current_pos-1);
		if (character_after && strchr("[](){}", character_after)) {
			current_brace_pos = current_pos+1;
		}
	}
		
	if (current_brace_pos>=0) {
		matching_brace_pos = gtk_scintilla_brace_match(GTK_SCINTILLA(scintilla), current_brace_pos);
	}
	
	
	// If no brace has been found or we aren't editing PHP code
	if ((current_brace_pos==-1)) {
		gtk_scintilla_brace_bad_light(GTK_SCINTILLA(scintilla), -1);// Remove any existing highlight
		return;
	}
	
	// A brace has been found, but there isn't a matching one ...
	if (current_brace_pos!=1 && matching_brace_pos==-1) {
		// ... therefore send the bad_list message so it highlights the brace in red
		gtk_scintilla_brace_bad_light(GTK_SCINTILLA(scintilla), current_brace_pos);
	}
	else {
		// a brace has been found and a matching one, so highlight them both
		gtk_scintilla_brace_highlight(GTK_SCINTILLA(scintilla), current_brace_pos, matching_brace_pos);
		
		// and highlight the indentation marker
		current_brace_column = gtk_scintilla_get_column(GTK_SCINTILLA(scintilla), current_brace_pos);
		matching_brace_column = gtk_scintilla_get_column(GTK_SCINTILLA(scintilla), matching_brace_pos);
		
		gtk_scintilla_set_highlight_guide(GTK_SCINTILLA(scintilla), MIN(current_brace_column, matching_brace_pos));
	}
	
}

static void char_added(GtkWidget *scintilla, guint ch)
{
	gint current_pos;
	gint wordStart;
	gint wordEnd;
	gint current_word_length;
	gint current_line;
	gint previous_line;
	gint previous_line_indentation;
	gchar *ac_buffer = NULL;
	gint ac_length;
	gchar *member_function_buffer = NULL;
	gint member_function_length;
	gint previous_line_end;
	gchar *previous_char_buffer;
	gint previous_char_buffer_length;
	guint style;
	gint type;

	current_pos = gtk_scintilla_get_current_pos(GTK_SCINTILLA(scintilla));
	current_line = gtk_scintilla_line_from_position(GTK_SCINTILLA(scintilla), current_pos);
	wordStart = gtk_scintilla_word_start_position(GTK_SCINTILLA(scintilla), current_pos-1, TRUE);
	wordEnd = gtk_scintilla_word_end_position(GTK_SCINTILLA(scintilla), current_pos-1, TRUE);
	current_word_length = wordEnd - wordStart;
	style = gtk_scintilla_get_style_at(GTK_SCINTILLA(scintilla), current_pos);
	type = main_window.current_editor->type;

	if (gtk_scintilla_autoc_active(GTK_SCINTILLA(scintilla))==1) {
		style = 0; // Hack to get around the drop-down not showing in comments, but if it's been forced...	
	}

	if ( ( type != TAB_HELP && 
		(style != SCE_HPHP_SIMPLESTRING) && (style != SCE_HPHP_HSTRING) && 
		(style != SCE_HPHP_COMMENTLINE) && (style !=SCE_HPHP_COMMENT)) ) {

		if (ch=='\r' || ch=='\n') {
			//g_print("current_line: %d\n", current_line);
			if (current_line>0) {
				previous_line = current_line-1;
				previous_line_indentation = gtk_scintilla_get_line_indentation(GTK_SCINTILLA(scintilla), previous_line);

				if (1==1) {//TODO: preferences.auto_indent_after_brace) {
					previous_line_end = gtk_scintilla_get_line_end_position(GTK_SCINTILLA(scintilla), previous_line);
					previous_char_buffer = gtk_scintilla_get_text_range (GTK_SCINTILLA(scintilla), previous_line_end-1, previous_line_end, &previous_char_buffer_length);
					if (*previous_char_buffer=='{') {
						previous_line_indentation+=preferences.indentation_size;
					}
				}

				indent_line(scintilla, current_line, previous_line_indentation);

				if (DEBUG_MODE) { g_print("DEBUG: tab.c:char_added:previous_line=%d, previous_indent=%d\n", previous_line, previous_line_indentation); }
				//gtk_scintilla_goto_pos(GTK_SCINTILLA(scintilla), gtk_scintilla_get_line_end_position(GTK_SCINTILLA(scintilla), current_line));
				if (preferences.use_tabs_instead_spaces) {
					gtk_scintilla_goto_pos(GTK_SCINTILLA(scintilla), gtk_scintilla_position_from_line(GTK_SCINTILLA(scintilla), current_line)+(previous_line_indentation/gtk_scintilla_get_tab_width(GTK_SCINTILLA(scintilla))));
				}
				else {
					gtk_scintilla_goto_pos(GTK_SCINTILLA(scintilla), gtk_scintilla_position_from_line(GTK_SCINTILLA(scintilla), current_line)+(previous_line_indentation));
				}
			}
		}
		else {
			member_function_buffer = gtk_scintilla_get_text_range (GTK_SCINTILLA(scintilla), wordStart-2, wordStart, &member_function_length);
			if (gtk_scintilla_call_tip_active(GTK_SCINTILLA(scintilla)) && ch==')') {
				gtk_scintilla_call_tip_cancel(GTK_SCINTILLA(scintilla));
			}
			else if (type == TAB_PHP && (ch == '(') && 
				(gtk_scintilla_get_line_state(GTK_SCINTILLA(scintilla), current_line))==274 &&
				(calltip_timer_set==FALSE)) {
					calltip_timer_id = g_timeout_add(preferences.calltip_delay, calltip_callback, (gpointer) current_pos);
					calltip_timer_set=TRUE;
			}
			else if (type == TAB_PHP && strcmp(member_function_buffer, "->")==0 && 
				(gtk_scintilla_get_line_state(GTK_SCINTILLA(scintilla), current_line)==274)) {
				if (gtk_scintilla_autoc_active(GTK_SCINTILLA(scintilla))==1) {
					autocomplete_member_function(scintilla, wordStart, wordEnd);
				}
				else {
					if (completion_timer_set==FALSE) {
						completion_timer_id = g_timeout_add(preferences.auto_complete_delay, auto_memberfunc_complete_callback, (gpointer) current_pos);
						completion_timer_set=TRUE;
					}
				}
			}
			else if ((current_word_length>=3) && 
				( (gtk_scintilla_get_line_state(GTK_SCINTILLA(scintilla), current_line)==274 && type == TAB_PHP) ||
				  (type != TAB_HELP) )) {
				// check to see if they've typed <?php and if so do nothing
				if (wordStart>1) {
					ac_buffer = gtk_scintilla_get_text_range (GTK_SCINTILLA(scintilla), wordStart-2, wordEnd, &ac_length);
					if (strcmp(ac_buffer,"<?php")==0 && type == TAB_PHP) {
						return;
					}
				}
	
				if (gtk_scintilla_autoc_active(GTK_SCINTILLA(scintilla))==1) {
					switch(type) {
						case(TAB_PHP): autocomplete_word(scintilla, wordStart, wordEnd); break;
						case(TAB_CSS): css_autocomplete_word(scintilla, wordStart, wordEnd); break;
						case(TAB_SQL): sql_autocomplete_word(scintilla, wordStart, wordEnd); break;
					}
				}
				else {
					switch(type) {
						case(TAB_PHP): completion_timer_id = g_timeout_add(preferences.auto_complete_delay, auto_complete_callback, (gpointer) current_pos); break;
						case(TAB_CSS): completion_timer_id = g_timeout_add(preferences.auto_complete_delay, css_auto_complete_callback, (gpointer) current_pos); break;
						case(TAB_SQL): completion_timer_id = g_timeout_add(preferences.auto_complete_delay, sql_auto_complete_callback, (gpointer) current_pos); break;
					}
					
				}
			}
			// Drop down for HTML here (line_state = 272)
		}
	}
}

gboolean editor_is_local(Editor *editor)
{
	gchar *filename;
	
	filename = (editor->filename)->str;
	if (g_strncasecmp(filename, "file://", MIN(strlen(filename), 7))==0) {
		return TRUE;	
	}
	if (g_strncasecmp(filename, "/", MIN(strlen(filename), 1))==0) {
		return TRUE;	
	}

	g_print("FALSE - not local!!!");
	return FALSE;
}

gchar * editor_convert_to_local(Editor *editor)
{
	gchar *filename;
	
	if (!editor_is_local(editor)) {
		return NULL;
	}
	filename = editor->filename->str;
	if (g_strncasecmp(filename, "file://", MIN(strlen(filename), 7))==0) {
		filename += 7;
	}
	
	return filename;
}

gchar *convert_to_full(gchar *filename)
{
	gchar *new_filename;
	gchar *cwd;
	gchar abs_buffer[2048];
	GString *gstr_filename;
	
	if (strstr(filename, "://") != NULL) {
		return g_strdup(filename);	
	}

	cwd = g_get_current_dir();
	// g_rel2abs returns a pointer in to abs_buffer!
	new_filename = g_strdup(g_rel2abs (filename, cwd, abs_buffer, 2048));
	g_free(cwd);
	
	gstr_filename = g_string_new("file://");
	gstr_filename = g_string_append(gstr_filename, new_filename);
	g_free(new_filename);
		
	new_filename = gstr_filename->str;
	g_string_free(gstr_filename, FALSE);
	return new_filename;
}
