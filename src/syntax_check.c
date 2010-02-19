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


#include "syntax_check.h"
#include "preferences.h"
#include "main_window.h"
#include "main_window_callbacks.h"


gchar *run_php_lint(gchar *command_line, gboolean return_error);
void syntax_add_lines(gchar *output);
GString *save_as_temp_file(void);
int syntax_check_run(void);


gchar *run_php_lint(gchar *command_line, gboolean return_error){
	gchar *stdout;
	static gchar *stderr;
	gint exit_status;
	GError *error;

	if( return_error && stderr ) return stderr;
	
	error=NULL;

	if( !(g_spawn_command_line_sync(command_line, &stdout, &stderr, &exit_status, &error)) )
		return NULL;
	
	return ( return_error ? stderr : stdout );
}//run_php_lint


void syntax_add_lines(gchar *output){
	GtkTreeIter   iter;
	gchar *copy;
	gchar *token;
	int line_number;
	int first_error;
	gint line_start;
	gint line_end;
	gint indent;
	
	line_number=0;
	first_error=0;
	copy=output;

	Editor *editor=main_window_get_current_editor();
	while( (token = strtok(copy, "\n")) ) {
		if( ((strncmp(token, "PHP Warning:  ", MIN(strlen(token), 14))!=0)) && ((strncmp(token, "Content-type", MIN(strlen(token), 12))!=0)) ) { 
			gtk_list_store_append(main_window_get_lint_store(), &iter);
			gtk_list_store_set(main_window_get_lint_store(), &iter, 0, token, -1);
	
			line_number = (atoi( (strrchr(token, ' ')) )) -1;
	
			if(line_number<=0) g_print("Line number is %d\n", line_number);

			if(!first_error) first_error = line_number;

			indent = gtk_scintilla_get_line_indentation(GTK_SCINTILLA(editor->scintilla), line_number);

			line_start = gtk_scintilla_position_from_line(GTK_SCINTILLA(editor->scintilla), line_number);
			line_start += indent / preferences.indentation_size;
	
			line_end = gtk_scintilla_get_line_end_position(GTK_SCINTILLA(editor->scintilla), line_number);
	
			gtk_scintilla_start_styling(GTK_SCINTILLA(editor->scintilla), line_start, 128);
			gtk_scintilla_set_styling(GTK_SCINTILLA(editor->scintilla), line_end-line_start, INDIC2_MASK);
		}
		copy = NULL;
	}

	if(first_error) goto_line_int(first_error);
}//syntax_add_lines


GString *save_as_temp_file(void) {
	gchar *write_buffer = NULL;
	gsize text_length;
	gint status;
	gchar *rawfilename;
	GString *filename;
	int file_handle;
	Editor *editor=main_window_get_current_editor();

	file_handle = g_file_open_tmp("connectEDXXXXXX",&rawfilename,NULL);
	if(file_handle != -1) {
		filename = g_string_new(rawfilename);
		
		text_length = gtk_scintilla_get_length(GTK_SCINTILLA(editor->scintilla));
		write_buffer = g_malloc0(text_length+1); // Include terminating null

		if(write_buffer == NULL) {
			g_warning("%s", _("Cannot allocate write buffer"));
			return NULL;
		}
		
		gtk_scintilla_get_text(GTK_SCINTILLA(editor->scintilla), text_length+1, write_buffer);
	
		status = write(file_handle, write_buffer, text_length+1);
		
		g_free(write_buffer);
		g_free(rawfilename);
		close(file_handle);

		return filename;
	}
	
	return NULL;
}//save_as_temp_file

int syntax_check_run(void) {
	GtkTreeIter iter;
	GString *command_line;
	gchar *stderr;
	gchar *stdout;
	GString *filename;
	GError *error;
	gboolean using_temp;

	stdout=NULL;
	stderr=NULL;

	error=NULL;
	int return_code=0;
	
	Editor *editor=main_window_get_current_editor();
	GtkListStore *lint_store=main_window_get_lint_store();
	if(!editor) {
		// No 'working' files.
		lint_store=gtk_list_store_new(1, G_TYPE_STRING);
		gtk_list_store_clear(lint_store);
		gtk_list_store_append(lint_store, &iter);
		gtk_list_store_set(lint_store, &iter, 0, _("You don't have any files open to check\n"), -1);
		gtk_tree_view_set_model(GTK_TREE_VIEW(main_window_get_lint_view()), GTK_TREE_MODEL(lint_store));
		return E_PHP_SYNTAX_NO_OPEN_FILES;
	}

	if(editor->saved==TRUE) {
		filename = g_string_new(editor_convert_to_local(editor));
		using_temp = FALSE;
	} else {
		filename = save_as_temp_file();
		using_temp = TRUE;
	}

	command_line = g_string_new(preferences.php_binary_location);
	command_line = g_string_append(command_line, " -q -l -d html_errors=Off -f ");
	command_line = g_string_append(command_line, filename->str);

	stdout=run_php_lint(command_line->str, FALSE);
	stderr=run_php_lint(command_line->str, TRUE);
	g_string_free(command_line, TRUE);
	
	
	lint_store = gtk_list_store_new(1, G_TYPE_STRING);
	
	gtk_scintilla_start_styling(GTK_SCINTILLA(editor->scintilla), 0, INDIC2_MASK);
	gtk_scintilla_set_styling(GTK_SCINTILLA(editor->scintilla),
					gtk_scintilla_get_length(GTK_SCINTILLA(editor->scintilla)), 0);
	
	gtk_list_store_clear(lint_store);
	return_code=0;
	if(stdout||stderr) {
		gtk_list_store_append(lint_store, &iter);
		gtk_list_store_set(lint_store, &iter, 0, _(stdout), -1);
		syntax_add_lines(stderr);
		return_code = E_PHP_SYNTAX_ERROR;
	} else {
		gtk_list_store_append(lint_store, &iter);
		gtk_list_store_set(lint_store, &iter, 0, _("Error calling PHP CLI.  Is PHP command line binary installed? If so, check if it's in your path or set php_binary in ~/.gnome2/connectED.\n"), -1);
		return_code = E_PHP_SYNTAX_CLI_NOT_FOUND;
	}
	gtk_tree_view_set_model(GTK_TREE_VIEW(main_window_get_lint_view()), GTK_TREE_MODEL(lint_store));
		
	if(using_temp) unlink(filename->str);
	
	g_free(stdout);
	g_free(stderr);
		
	g_string_free(filename, TRUE);

	return return_code;
}//syntax_check
