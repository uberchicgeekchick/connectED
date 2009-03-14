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


#include "classbrowser.h"
#include "main_window.h"
#include "main_window_callbacks.h"


static GSList *filelist = NULL;
static GSList *dirlist = NULL;
static GSList *functionlist = NULL;
static GSList *classlist = NULL;

guint identifierid = 0;



gboolean classbrowser_filelist_find(gchar *filename)
{
	GSList *li;
	ClassBrowserFile *file;

	for(li = filelist; li!= NULL; li = g_slist_next(li)) {
		file = li->data;
		if (file) {
			if (g_ascii_strcasecmp(file->filename, filename) == 0) {
				return TRUE;
			}
		}
	}

	return FALSE;
}


gchar *strip_double_slashes(gchar *filename)
{
	gchar *result;
	gchar *result_running;
	gchar previous_char;

	result = g_malloc(strlen(filename)+1);
	result_running = result;

	previous_char = *filename;
	*result_running = *filename;
	filename++;
	result_running++;

	while (*filename) {
		if (previous_char=='/' && *filename == '/') {
			// Do nothing, but makes logic clearer
		}
		else {
			*result_running = *filename;
			result_running++;
		}
		previous_char = *filename;
		filename++;
	}
	*result_running = '\0';

	return result;
}


void classbrowser_filelist_clear(void)
{
	GSList *li;
	ClassBrowserFile *file;

	for(li = filelist; li!= NULL; li = g_slist_next(li)) {
		file = li->data;
		if (file) {
			g_free(file->filename);
		}
	}
	g_slist_free(filelist);
	filelist = NULL;
}


void classbrowser_dirlist_clear(void)
{
	GSList *li;
	gchar *dir;

	for(li = dirlist; li!= NULL; li = g_slist_next(li)) {
		dir = li->data;
		if (dir) {
			g_free(dir);
		}
	}
	g_slist_free(dirlist);
	dirlist = NULL;
}


void classbrowser_filelist_add(gchar *filename)
{
	ClassBrowserFile *file;

	if (strstr(filename, "file://")==filename) {
		filename += 6;
	}

	if (!classbrowser_filelist_find(filename) && is_php_file_from_filename(filename)) {
		file = g_malloc(sizeof(ClassBrowserFile));
		file->filename = strip_double_slashes(filename);
		file->accessible = TRUE;
		file->modified_time = 0;
		filelist = g_slist_append(filelist, file);
	}
}

#include <dirent.h>
void classbrowser_dirlist_add(gchar *dir)
{
	DIR *dir_iter;
	struct dirent *dir_data;
	GString *fullfilename;

	if (strstr(dir, "file://")==dir) {
		dir += 6;
	}

	dir_iter= opendir(dir);
	if (dir_iter) {
		errno=0;
		while ((dir_data = readdir(dir_iter))) {
			if (strcmp(dir_data->d_name, ".")!=0 &&
			        strcmp(dir_data->d_name, "..")!=0) {
				fullfilename = g_string_new(dir);
				fullfilename = g_string_append(fullfilename, "/");
				fullfilename = g_string_append(fullfilename, dir_data->d_name);
				if (g_file_test(fullfilename->str, G_FILE_TEST_IS_DIR)) {
					//classbrowser_dirlist_add(fullfilename->str);
				}
				else if (is_php_file_from_filename(fullfilename->str)){
					classbrowser_filelist_add(fullfilename->str);
				}
				g_string_free(fullfilename, TRUE);
			}
		}
		errno=0;
		closedir(dir_iter);
	}
	else {
		//g_print("Err no: %d\n", errno);
	}

	dirlist = g_slist_append(dirlist, g_strdup(dir));
}

void classbrowser_dirlist_add_shared_source()
{
	gchar **shared_source_locations;
	gint i;

	shared_source_locations = g_strsplit(preferences.shared_source_location,",",-1);

	for (i = 0; shared_source_locations[i] != NULL; i++) {
		classbrowser_dirlist_add(shared_source_locations[i]);
	}
			
	g_strfreev(shared_source_locations);
}

gboolean classbrowser_file_accessible(gchar *filename)
{
	FILE *file;

	file = fopen(filename, "r");
	if (file) {
		fclose(file);
		return TRUE;
	}
	return FALSE;
}


void classbrowser_filelist_update(void)
{
	GSList *li;
	ClassBrowserFile *file;

	for(li = filelist; li!= NULL; li = g_slist_next(li)) {
		file = li->data;
		if (file) {
			if (!g_file_exists(file->filename)) {
				classbrowser_filelist_remove(file);
				li = g_slist_remove(filelist, file);
			}
			else if (!classbrowser_file_accessible(file->filename)) {
				//classbrowser_functionlist_start_file(file->filename);
				//classbrowser_functionlist_remove_dead_wood();
				file->accessible = FALSE;
			}
			else {
				file->accessible = TRUE;
			}
		}
	}
}


ClassBrowserFile *classbrowser_filelist_getnext(void)
{
	GSList *li;
	ClassBrowserFile *file;
	struct stat buf;

	for(li = filelist; li!= NULL; li = g_slist_next(li)) {
		file = li->data;
		if (file) {
			if (file->accessible) {
				if ( stat(file->filename, &buf) == 0) {
					if (buf.st_mtime != file->modified_time) {
						return file;
					}
				}
			}
		}
	}

	return NULL;
}


void classbrowser_start_update(void)
{
	GSList *li;
	ClassBrowserFunction *function;
	ClassBrowserClass *class;

	for(li = functionlist; li!= NULL; li = g_slist_next(li)) {
		function = li->data;
		if (function) {
			function->remove = TRUE;
		}
	}
	for(li = classlist; li!= NULL; li = g_slist_next(li)) {
		class = li->data;
		if (class) {
			class->remove = TRUE;
		}
	}
}


gboolean classbrowser_safe_equality(gchar *a, gchar *b)
{
	if (!a && !b) {
		return TRUE;
	}

	if ((a && !b) || (!a && b)) {
		return FALSE;
	}

	// (a && b)

	return (strcmp(a, b)==0);
}


ClassBrowserClass *classbrowser_classlist_find(gchar *classname, gchar *filename)
{
	GSList *li;
	ClassBrowserClass *class;

	for(li = classlist; li!= NULL; li = g_slist_next(li)) {
		class = li->data;
		if (class) {
			if (classbrowser_safe_equality(class->filename, filename) &&
			        classbrowser_safe_equality(class->classname, classname))
			{
				return class;
			}
		}
	}

	return NULL;
}


ClassBrowserFunction *classbrowser_functionlist_find(gchar *funcname, gchar *param_list, gchar *filename, gchar *classname)
{
	GSList *li;
	ClassBrowserFunction *function;
	gboolean found;

	for(li = functionlist; li!= NULL; li = g_slist_next(li)) {
		function = li->data;
		if (function) {
			found = TRUE;
			if (classbrowser_safe_equality(function->functionname, funcname) &&
			        classbrowser_safe_equality(function->filename, filename) &&
			        classbrowser_safe_equality(function->paramlist, param_list) &&
			        classbrowser_safe_equality(function->classname, classname)) {
				return function;
			}
		}
	}

	return NULL;
}


void classbrowser_functionlist_free(ClassBrowserFunction *function, GtkTreeIter *iter)
{
	gtk_tree_store_remove(GTK_TREE_STORE(main_window.classtreestore),iter);
	g_free(function->filename);
	g_free(function->functionname);
	if (function->paramlist) {
		g_free(function->paramlist);
	}
	if (function->classname) {
		g_free(function->classname);
	}
	functionlist = g_slist_remove(functionlist, function);
	g_free(function);
}


gboolean classbrowser_tree_find_iter(GtkTreeIter *iter, ClassBrowserFunction *function)
{
	guint id;
	gboolean found;
	GtkTreeIter child;

	found = TRUE;
	while (found) {
		if (gtk_tree_model_iter_has_child(GTK_TREE_MODEL(main_window.classtreestore), iter)) {
			gtk_tree_model_iter_children(GTK_TREE_MODEL(main_window.classtreestore),
			                             &child, iter);
			if (classbrowser_tree_find_iter(&child, function)) {
				memcpy(iter, &child, sizeof(child));
				return TRUE;
			}
		}

		gtk_tree_model_get(GTK_TREE_MODEL(main_window.classtreestore),iter,
		                   ID_COLUMN, &id,-1);

		if (id == function->identifierid) {
			return TRUE;
		}
		// if we aren't looking at the right id, move on
		found = gtk_tree_model_iter_next(GTK_TREE_MODEL(main_window.classtreestore), iter);
	}
	return FALSE;
}


void classbrowser_functionlist_remove(ClassBrowserFunction *function)
{
	GtkTreeIter iter;

	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(main_window.classtreestore), &iter)) {
		if (classbrowser_tree_find_iter(&iter, function)) {
			classbrowser_functionlist_free(function, &iter);
		}
	}
}


void classbrowser_classlist_remove(ClassBrowserClass *class)
{
	GtkTreeIter iter;
	guint id;
	gboolean found;

	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(main_window.classtreestore), &iter))
	{
		found = TRUE;
		while (found) {
			gtk_tree_model_get(GTK_TREE_MODEL(main_window.classtreestore),&iter,
			                   ID_COLUMN, &id,-1);
			// AAARGGHHH!!! TODO!!!
			// According to the docs gtk_tree_store_remove removes it and moves on
			// according to the header file, it removes it and returns void
			// I'll assume for now that I have to move on!
			if (id == class->identifierid) {
				//found = gtk_tree_store_remove(GTK_TREE_STORE(main_window.classtreestore),&iter);
				gtk_tree_store_remove(GTK_TREE_STORE(main_window.classtreestore),&iter);
				break;
			}
			else {
				// if we aren't looking at the right id, move on
				found = gtk_tree_model_iter_next(GTK_TREE_MODEL(main_window.classtreestore), &iter);
			}
		}
	}

	g_free(class->filename);
	g_free(class->classname);
	classlist = g_slist_remove(classlist, class);
	g_free(class);
}


gboolean classbrowser_class_find_before_position(gchar *classname, GtkTreeIter *iter)
{
	gboolean found;
	gchar *classnamefound;
	guint type;

	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(main_window.classtreestore), iter)) {
		found = TRUE;
		while (found) {
			gtk_tree_model_get(GTK_TREE_MODEL(main_window.classtreestore),iter,
			                   NAME_COLUMN, &classnamefound, TYPE_COLUMN, &type, -1);
			if ((type==CB_ITEM_TYPE_CLASS) && g_ascii_strcasecmp(classname, classnamefound)<0) {
				return TRUE;
			}
			if (type==CB_ITEM_TYPE_FUNCTION) {
				return TRUE;
			}
			found = gtk_tree_model_iter_next(GTK_TREE_MODEL(main_window.classtreestore), iter);
		}
	}
	return FALSE;
}


void classbrowser_classlist_add(gchar *classname, gchar *filename, gint line_number)
{
	ClassBrowserClass *class;
	GtkTreeIter iter;
	GtkTreeIter before;

	if ((class = classbrowser_classlist_find(classname, filename))) {
		class->line_number = line_number;
		class->remove
					= FALSE;
	}
else {
		class = g_malloc0(sizeof(ClassBrowserClass));
		class->classname = g_strdup(classname);
		class->filename = g_strdup(filename);
		class->line_number = line_number;
		class->remove
					= FALSE;
		class->identifierid = identifierid++;
		classlist = g_slist_append(classlist, class);

if (classbrowser_class_find_before_position(classname, &before)) {
			gtk_tree_store_insert_before(main_window.classtreestore,
			                             &iter, NULL, &before);
		}
		else {
			gtk_tree_store_append (main_window.classtreestore, &iter, NULL);
		}

		gtk_tree_store_set (GTK_TREE_STORE(main_window.classtreestore), &iter,
		                    NAME_COLUMN, (class->classname), FILENAME_COLUMN, (class->filename),
		                    LINE_NUMBER_COLUMN, line_number, TYPE_COLUMN, CB_ITEM_TYPE_CLASS, ID_COLUMN, (class->identifierid), -1);
	}
}


gboolean classbrowser_classid_to_iter(guint classid, GtkTreeIter *iter)
{
	guint id;
	gboolean found;

	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(main_window.classtreestore), iter)) {
		found = TRUE;
		while (found) {
			gtk_tree_model_get(GTK_TREE_MODEL(main_window.classtreestore),iter,
			                   ID_COLUMN, &id,-1);
			if (id == classid) {
				return TRUE;
			}
			found = gtk_tree_model_iter_next(GTK_TREE_MODEL(main_window.classtreestore), iter);
		}
	}
	return FALSE;
}

void classbrowser_functionlist_add(gchar *classname, gchar *funcname, gchar *filename, guint line_number, gchar *param_list)
{
	ClassBrowserClass *class;
	ClassBrowserFunction *function;
	GtkTreeIter iter;
	GtkTreeIter class_iter;
	GString *function_decl;
	guint type;

	if ((function = classbrowser_functionlist_find(funcname, param_list, filename, classname))) {
		function->line_number = line_number;
		function->remove
		= FALSE;
	}
	else {
		function = g_malloc0(sizeof(ClassBrowserFunction));
		function->functionname = g_strdup(funcname);
		if (param_list) {
			function->paramlist = g_strdup(param_list);
		}
		function->filename = g_strdup(filename);
		function->line_number = line_number;
		function->remove
		= FALSE;
		function->identifierid = identifierid++;
		if (classname && (class = classbrowser_classlist_find(classname, filename))) {
			function->class_id = class->identifierid;
			function->classname = g_strdup(classname);
			classbrowser_classid_to_iter(function->class_id, &class_iter);
			type = CB_ITEM_TYPE_CLASS_METHOD;
			gtk_tree_store_append (main_window.classtreestore, &iter, &class_iter);
		}
		else {
			type = CB_ITEM_TYPE_FUNCTION;
			gtk_tree_store_append (main_window.classtreestore, &iter, NULL);
		}

		functionlist = g_slist_append(functionlist, function);

		function_decl = g_string_new(funcname);
		function_decl = g_string_append(function_decl, "(");
		if (param_list) {
			function_decl = g_string_append(function_decl, param_list);
		}
		function_decl = g_string_append(function_decl, ")");

		gtk_tree_store_set (main_window.classtreestore, &iter,
		                    NAME_COLUMN, function_decl->str, LINE_NUMBER_COLUMN, line_number, FILENAME_COLUMN, filename, TYPE_COLUMN, type, ID_COLUMN, function->identifierid, -1);
		g_string_free(function_decl, TRUE);
	}
}


void classbrowser_remove_dead_wood(void)
{
	GSList *orig;
	GSList *li;
	ClassBrowserFunction *function;
	ClassBrowserClass *class;

	orig = g_slist_copy(functionlist);
	for(li = orig; li!= NULL; li = g_slist_next(li)) {
		function = li->data;
		if (function) {
			if (function->remove
			   ) {
				classbrowser_functionlist_remove(function);
			}
		}
	}
	g_slist_free(orig);

	orig = g_slist_copy(classlist);
	for(li = orig; li!= NULL; li = g_slist_next(li)) {
		class = li->data;
		if (class) {
			if (class->remove
			   )
			{
				classbrowser_classlist_remove(class);
			}
		}
	}
	g_slist_free(orig);
}


void classbrowser_filelist_remove(ClassBrowserFile *file)
{
	filelist = g_slist_remove(filelist, file);
	g_free(file->filename);
	g_free(file);
}


void classbrowser_update(void)
{
	GSList *li;
	Editor *editor;
	ClassBrowserFile *file;
	struct stat buf;

	static guint press_event = 0;
	static guint release_event = 0;

	if (gtk_paned_get_position(GTK_PANED(main_window.main_horizontal_pane))==0)
		return;
	
	if (press_event) {
		g_signal_handler_disconnect(main_window.classtreeview, press_event);
	}
	if (release_event) {
		g_signal_handler_disconnect(main_window.classtreeview,release_event);
	}
	classbrowser_filelist_clear();
	classbrowser_dirlist_clear();
	classbrowser_dirlist_add_shared_source();

	// iterate open files
	for(li = editors; li!= NULL; li = g_slist_next(li)) {
		editor = li->data;
		if (editor) {
			classbrowser_filelist_add(editor->filename->str);
			classbrowser_dirlist_add(g_path_get_dirname(editor->filename->str));
		}
	}

	classbrowser_filelist_update();
	classbrowser_start_update();

	while ( (file = classbrowser_filelist_getnext() ) ) {
		while (gtk_events_pending())
			gtk_main_iteration();
		stat(file->filename, &buf);
		file->modified_time = buf.st_mtime;
		classbrowser_parse_file(file->filename);
	}

	classbrowser_remove_dead_wood();

	press_event = g_signal_connect(G_OBJECT(main_window.classtreeview), "button_press_event",
	                                 G_CALLBACK(treeview_double_click), NULL);
	release_event = g_signal_connect(G_OBJECT(main_window.classtreeview), "button_release_event",
	                                   G_CALLBACK(treeview_click_release), NULL);
}


gint member_function_list_sort(gconstpointer a, gconstpointer b)
{
	return (strcmp((gchar *)a, (gchar *)b));
}


GString *get_member_function_completion_list(GtkWidget *scintilla, gint wordStart, gint wordEnd)
{
	gchar *buffer = NULL;
	GSList *li;
	GList *li2;
	ClassBrowserFunction *function;
	GList* member_functions = NULL;
	GList* sorted_member_functions = NULL;
	GString *result = NULL;
	gint length;
	gchar *function_name;

	buffer = gtk_scintilla_get_text_range (GTK_SCINTILLA(scintilla), wordStart, wordEnd, &length);

	for(li = functionlist; li!= NULL; li = g_slist_next(li)) {
		function = li->data;
		if (function) {
			if ((strncmp(function->functionname, buffer, strlen(buffer))==0) || (wordStart==wordEnd)) {
				member_functions = g_list_append(member_functions, function->functionname);
			}
		}
	}

	sorted_member_functions = g_list_sort(member_functions, member_function_list_sort);
	member_functions = sorted_member_functions;

	for(li2 = member_functions; li2!= NULL; li2 = g_list_next(li2)) {
		function_name = li2->data;
		if (!result) {
			result = g_string_new(function_name);
		} else {
			result = g_string_append(result, " ");
			result = g_string_append(result, function_name);
		}
	}

	if(!result)
		result=g_string_new("");

	result = g_string_append(result, " ");
	return result;
}


void autocomplete_member_function(GtkWidget *scintilla, gint wordStart, gint wordEnd)
{
	GString *list;

	list = get_member_function_completion_list(scintilla, wordStart, wordEnd);

	if (list) {
		gtk_scintilla_autoc_show(GTK_SCINTILLA(scintilla), wordEnd-wordStart, list->str);
		g_string_free(list, FALSE);
	}
}


gboolean classbrowser_file_in_list_find(GSList *list, gchar *file)
{
	GSList *list_walk;
	gchar *data;

	for(list_walk = list; list_walk!= NULL; list_walk = g_slist_next(list_walk)) {
		data = list_walk->data;
		if (strcmp(data, file)==0) {
			return TRUE;
		}
	}
	return FALSE;
}


void classbrowser_update_selected_label(gchar *filename, gint line)
{
	GSList *filenames;
	GSList *function_walk;
	GString *new_label;
	ClassBrowserFunction *function;
	gchar *func_filename;

	filenames = NULL;
	for(function_walk = functionlist; function_walk!= NULL; function_walk = g_slist_next(function_walk)) {
		function = function_walk->data;
		if (function) {
			func_filename = function->filename;
			// g_slist_find and g_slist_index don't seem to work, always return NULL or -1 respec.
			if (!classbrowser_file_in_list_find(filenames, func_filename)) {
				filenames = g_slist_prepend(filenames, func_filename);
			}
		}
	}

	new_label = get_differing_part(filenames, filename);
	if (new_label) {
		new_label = g_string_prepend(new_label, _("FILE: "));
		g_string_append_printf(new_label, "(%d)", line);
		gtk_label_set_text(GTK_LABEL(main_window.treeviewlabel), new_label->str);
		g_string_free(new_label, TRUE);
		g_slist_free(filenames);
	}
}
