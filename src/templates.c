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

/* a lot of the code in this unit is from:
	http://www-106.ibm.com/developerworks/linux/library/l-glib2.html
*/


#include "main_window.h"
#include "tab.h"
#include "main_window_callbacks.h"
#include "preferences.h"
#include "classbrowser.h"
#include "plugin.h"
#include "templates.h"

#define BACKSLASH 92
#define NEWLINE 10
#define TAB 9


GHashTable *templates = NULL;
gboolean templates_updated = FALSE;

FILE *template_db_create(gchar *filename);
GString *template_get_filename(void);
static void template_save_entry(gpointer key, gpointer value, gpointer user_data);
void template_db_save(void);
void template_find_and_insert(void);

FILE *template_db_create(gchar *filename)
{
	FILE *fp;
	fp = fopen(filename, "w+");
	fputs("class\tclass |\\n{\\n\\t\\n}\n", fp);
	fputs("for\tfor (|; ; )\n{\\n\\t\\n}\n", fp);
	fputs("function\tfunction |\\n{\\n\\t\\n}\n", fp);
	fputs("if\tif (|) {\\n\\t\\n}\n", fp);
	fputs("ife\tif (|) {\\n\\t\\n}\\nelse {\\n\\t\\n}\n", fp);
	fputs("switch\tswitch(|) {\\n\\tcase ():;break;\\n\\tcase ():;break;\\n}\n", fp);
	fputs("while\twhile (|)\\n{\\n\\t\\n}\n", fp);
	rewind(fp);
	return fp;
}

GString *template_get_filename(void)
{
	GString *ret;
	ret = g_string_new( g_get_home_dir());
	ret = g_string_append(ret, "/.connectED/templates.txt");
	
	return ret;
}

void template_db_open(void)
{
	GString *template_filename;
	FILE *fp;
	char buf[16384];
	
	template_filename = template_get_filename();
	  
    templates = g_hash_table_new(g_str_hash, g_str_equal);
	 
	fp = fopen(template_filename->str, "r");
  	if (!fp) {	
		fp = template_db_create(template_filename->str);
		if (!fp) {
			g_print(_("CANNOT Create templates file: %s\n"), template_filename->str);
			return;
		}
	}

	while(fgets(buf, sizeof(buf), fp)) {
        gchar *key, *value;
		gpointer *old_key=NULL, *old_value=NULL;
        
		/* get the first and the second field */
        key = (gpointer)strtok(buf, "\t");
        if(!key) continue;
        value = (gpointer)strtok(NULL, "\t");
        if(!value) continue;

		/* try looking up this key */
  		if(g_hash_table_lookup_extended(templates, key, old_key, old_value)) {
          /* insert the new value */
          g_hash_table_insert(templates, g_strdup(key), g_strdup(value));

          /* just free the key and value */
          g_free(old_key);
          g_free(old_value);
  		}
		else {
          /* insert the new value */
          g_hash_table_insert(templates, g_strdup(key), g_strdup(value));
		}
  	}
  	fclose(fp);
	
	g_string_free(template_filename, TRUE);
}

static void template_save_entry(gpointer key, gpointer value, gpointer user_data)
{
	FILE *fp;
	GString *entry;
	
	fp = (FILE *)user_data;
	entry = g_string_new(key);
	entry = g_string_append(entry, "\t");
	entry = g_string_append(entry, value);
	
	fputs(entry->str, fp);
	
	g_string_free(entry, TRUE);
}

void template_db_save(void)
{
	GString *template_filename;
	FILE *fp;
	
	template_filename = template_get_filename();
	  
	fp = fopen(template_filename->str, "w");
  	if (!fp) {
		g_print(_("CANNOT Create templates file: %s\n"), template_filename->str);
		return;
	}

  	g_hash_table_foreach(templates, template_save_entry, (gpointer) fp);
	
  	fclose(fp);
	
	g_string_free(template_filename, TRUE);
}


static void free_a_hash_table_entry(gpointer key, gpointer value, gpointer user_data)
{
	g_free(key);
	g_free(value);
}

void template_db_close(void)
{
	if (templates) {
		if (templates_updated) {
			template_db_save();	
		}

  		g_hash_table_foreach(templates, free_a_hash_table_entry, NULL);
		g_hash_table_destroy(templates);
	}
}

gchar *template_find(gchar *key)
{
	return g_hash_table_lookup(templates, key);
}

void template_replace(gchar *key, gchar *value)
{
	// ALSO USED TO INSERT
	gpointer *old_key=NULL, *old_value=NULL;
	
	/* try looking up this key */
  	if(g_hash_table_lookup_extended(templates, key, old_key, old_value)) {
    	/* insert the new value */
        g_hash_table_insert(templates, g_strdup(key), g_strdup(value));

        /* just free the key and value */
        g_free(old_key);
        g_free(old_value);
  	}
	else {
        /* insert the new value */
        g_hash_table_insert(templates, g_strdup(key), g_strdup(value));
	}
	templates_updated = TRUE;
}

void template_delete(gchar *key)
{
	gpointer *old_key=NULL, *old_value=NULL;

	if (key == NULL) {
		return;
	}
	
	/* try looking up this key */
	if(g_hash_table_lookup_extended(templates, key, old_key, old_value)) {
	    /* remove the entry in the hash table */
        g_hash_table_remove(templates, key);

        /* just free the key and value */
        g_free(old_key);
        g_free(old_value);
		templates_updated = TRUE;
	}
}

void template_find_and_insert(void)
{
	gint current_pos;
	gint wordStart;
	gint wordEnd;
	gchar *buffer = NULL;
	gint length;
	gchar *template = NULL;
	char buf[16384];
	gint indentation;
	gint current_line;
	gint new_cursor_pos = 0;
	gint buffer_pos;
	Editor *editor=main_window_get_current_editor();
	
	current_pos = gtk_scintilla_get_current_pos(GTK_SCINTILLA(editor->scintilla));
	wordStart = gtk_scintilla_word_start_position(GTK_SCINTILLA(editor->scintilla), current_pos-1, TRUE);
	wordEnd = gtk_scintilla_word_end_position(GTK_SCINTILLA(editor->scintilla), current_pos-1, TRUE);

	buffer = gtk_scintilla_get_text_range (GTK_SCINTILLA(editor->scintilla), wordStart, wordEnd, &length);
	template = template_find(buffer);
	if (template) {
		strncpy(buf, template, 16383);
		gtk_scintilla_begin_undo_action(GTK_SCINTILLA(editor->scintilla));
		
		// Remove template-key
		gtk_scintilla_set_selection_start(GTK_SCINTILLA(editor->scintilla), wordStart);
		gtk_scintilla_set_selection_end(GTK_SCINTILLA(editor->scintilla), wordEnd);
		gtk_scintilla_replace_sel(GTK_SCINTILLA(editor->scintilla), "");
		
		// Get current indentation level
		current_line = gtk_scintilla_line_from_position(GTK_SCINTILLA(editor->scintilla), current_pos);
		indentation = gtk_scintilla_get_line_indentation(GTK_SCINTILLA(editor->scintilla), current_line);
		
		// Insert template, line by line, taking in to account indentation
		buffer_pos = 0;
		while (buf[buffer_pos] && buffer_pos<16380) {// 16384 - a few to account for lookaheads
			if (buf[buffer_pos] == BACKSLASH && buf[buffer_pos+1] == 'n') { 
				gtk_scintilla_add_text(GTK_SCINTILLA(editor->scintilla), 1, "\n");			
				current_pos = gtk_scintilla_get_current_pos(GTK_SCINTILLA(editor->scintilla));
				current_line = gtk_scintilla_line_from_position(GTK_SCINTILLA(editor->scintilla), current_pos);
				gtk_scintilla_set_line_indentation(GTK_SCINTILLA(editor->scintilla), current_line, indentation);
				gtk_scintilla_set_current_pos(GTK_SCINTILLA(editor->scintilla), gtk_scintilla_get_line_end_position(GTK_SCINTILLA(editor->scintilla), current_line));
				buffer_pos++; buffer_pos++;
			}
			else if (buf[buffer_pos] == '|') { // Current choice of cursor pos character
				new_cursor_pos = gtk_scintilla_get_current_pos(GTK_SCINTILLA(editor->scintilla));
				buffer_pos++;
			}
			else if (buf[buffer_pos] == BACKSLASH && buf[buffer_pos+1] == 't') { 
				gtk_scintilla_add_text(GTK_SCINTILLA(editor->scintilla), 1, "\t");			
				buffer_pos++; buffer_pos++;
			}
			else {
				gtk_scintilla_add_text(GTK_SCINTILLA(editor->scintilla), 1, &buf[buffer_pos]);			
				buffer_pos++;
			}
		}
		
		// If there was a cursor pos character in there, put the cursor there
		if (new_cursor_pos) {
			gtk_scintilla_set_current_pos(GTK_SCINTILLA(editor->scintilla), new_cursor_pos);
			gtk_scintilla_set_selection_start(GTK_SCINTILLA(editor->scintilla), new_cursor_pos);
			gtk_scintilla_set_selection_end(GTK_SCINTILLA(editor->scintilla), new_cursor_pos);
		}
		gtk_scintilla_end_undo_action(GTK_SCINTILLA(editor->scintilla));
	}
}


gchar *template_convert_to_display(gchar *content)
{
	char buf_in[16384];
	char buf_out[16384];
	gint read_buffer_pos;
	gint write_buffer_pos;
	gchar *ret;
	
	strncpy(buf_in, content, 16383);
	read_buffer_pos = 0;
	write_buffer_pos = 0;
	while (buf_in[read_buffer_pos] && read_buffer_pos<16380) {// 16384 - a few to account for lookaheads
		if (buf_in[read_buffer_pos] == BACKSLASH && buf_in[read_buffer_pos+1] == 'n') { 
			buf_out[write_buffer_pos] = NEWLINE;
			read_buffer_pos++; 
		}
		else if (buf_in[read_buffer_pos] == BACKSLASH && buf_in[read_buffer_pos+1] == 't') { 
			buf_out[write_buffer_pos] = TAB;
			read_buffer_pos++;
		}
		else {
			buf_out[write_buffer_pos] = buf_in[read_buffer_pos];
		}
		write_buffer_pos++;
		read_buffer_pos++;
	}
	buf_out[write_buffer_pos] = 0;
	ret = g_malloc(write_buffer_pos+1);
	memcpy(ret, buf_out, write_buffer_pos+1);
	
	return ret;
}

gchar *template_convert_to_template(gchar *content)
{
	char buf_in[16384];
	char buf_out[16384];
	gint read_buffer_pos;
	gint write_buffer_pos;
	gchar *ret;
	
	strncpy(buf_in, content, 16383);
	read_buffer_pos = 0;
	write_buffer_pos = 0;
	while (buf_in[read_buffer_pos] && read_buffer_pos<16380) {// 16384 - a few to account for lookaheads
		if (buf_in[read_buffer_pos] == NEWLINE) { 
			buf_out[write_buffer_pos] = BACKSLASH;
			buf_out[write_buffer_pos+1] = 'n';
			write_buffer_pos++; 
		}
		else if (buf_in[read_buffer_pos] == TAB) { 
			buf_out[write_buffer_pos] = BACKSLASH;
			buf_out[write_buffer_pos+1] = 't';
			write_buffer_pos++; 
		}
		else {
			buf_out[write_buffer_pos] = buf_in[read_buffer_pos];
		}
		write_buffer_pos++;
		read_buffer_pos++;
	}
	if (buf_out[write_buffer_pos-1] != NEWLINE) {
		buf_out[write_buffer_pos] = NEWLINE;
		write_buffer_pos++;
	}
	buf_out[write_buffer_pos] = 0;
	ret = g_malloc(write_buffer_pos+1);
	memcpy(ret, buf_out, write_buffer_pos+1);

	return ret;
}
