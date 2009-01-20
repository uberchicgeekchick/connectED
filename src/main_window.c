/* This file is part of http://connectED/, a GNOME2 PHP Editor.
 
   Copyright(C) 2008 Kaity G. B.
 uberChick@uberChicGeekChick.Com
	  
   For more information or to find the latest release, visit our 
   website at http://uberchicgeekchick.com/
 
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or(at your option) any later version.
 
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

#include "main_window.h"
#include "main_window_menu.h"
#include "tab.h"
#include "main_window_callbacks.h"
#include "preferences.h"
#include "classbrowser.h"
#include "plugin.h"
#include "templates.h"


MainWindow main_window;
GIOChannel* inter_connectED_io;
guint inter_connectED_event_id;
gboolean DEBUG_MODE = FALSE;


void create_untitled_if_empty(void)
{
	if(g_slist_length(editors) == 0) {
		tab_create_new(TAB_FILE, NULL);
	}
}


void main_window_open_command_line_files(char **argv, gint argc)
{
	guint i;

	if(argc>1) {
		i = 1;
		while(argv[i] != NULL) {
			switch_to_file_or_open(argv[i],0);
			
			++i;
		}
	}
}


void main_window_pass_command_line_files(char **argv)
{
	guint i;
	GError *error;
	gsize bytes_written;

	error = NULL;
	inter_connectED_io = g_io_channel_new_file("/tmp/connectED.sock","w",&error);
	if(argv) {
		i = 1;
		while(argv[i] != NULL) {
			//g_print("%s:%d\n", argv[i], strlen(argv[i]));
			g_io_channel_write_chars(inter_connectED_io, argv[i], strlen(argv[i]),
			                         &bytes_written, &error);
			++i;
		}
	}
}


gboolean channel_pass_filename_callback(GIOChannel *source, GIOCondition condition, gpointer data )
{
	guint size;
	gchar buf[1024];

	g_io_channel_read(inter_connectED_io, buf, sizeof(buf), &size);
	//g_print("Passed %s\n", buf);
	tab_create_new(TAB_FILE, g_string_new(buf));
	return FALSE;
}




/*void main_window_create_unix_socket(void)
{
	struct sockaddr_un name;
	int sock;
	size_t size;
 
	// Create the socket. 
  
	sock = socket(PF_UNIX, SOCK_STREAM, 0);
	if(sock < 0) {
      g_print("Socket creation failed\n");
      exit(EXIT_FAILURE);
    }
 
	// Bind a name to the socket. 
 
	name.sun_family = AF_FILE;
	strcpy(name.sun_path, "/tmp/connectED.sock");
 
	// The size of the address is the offset of the start of the filename, plus its length, plus one for the terminating null byte.
	size =(offsetof(struct sockaddr_un, sun_path)
          + strlen(name.sun_path) + 1);
 
	if(bind(sock,(struct sockaddr *) &name, size) < 0) {
      perror("bind");
      exit(EXIT_FAILURE);
    }
 
	unix_socket = sock;
}*/

void force_config_folder(void)
{
	gint ret;
	gchar *dir;

	dir = g_build_filename(g_get_home_dir(), ".connectED", NULL);
	ret = mkdir(dir,(S_IRUSR|S_IWUSR|S_IXUSR));
	g_free(dir);
	if(ret && errno != EEXIST) {
		g_print(_("Unable to create ~/.connectED/(%d)"), errno);
		exit(-1);
	}

	dir = g_build_filename(g_get_home_dir(), ".connectED", "plugins", NULL);
	ret = mkdir(dir,(S_IRUSR|S_IWUSR|S_IXUSR));
	g_free(dir);
	if(ret && errno != EEXIST) {
		g_print(_("Unable to create ~/.connectED/plugins/(%d)"), errno);
		exit(-1);
	}
}

static void main_window_create_toolbars(void)
{
	// Create the Main Toolbar
	main_window.toolbar_main = gtk_toolbar_new();
	gtk_widget_show(main_window.toolbar_main);
	gnome_app_add_toolbar(GNOME_APP(main_window.window), GTK_TOOLBAR(main_window.toolbar_main), "toolbar1",
	                       BONOBO_DOCK_ITEM_BEH_NORMAL, BONOBO_DOCK_TOP, 1, 0, 0);
	gtk_container_set_border_width(GTK_CONTAINER(main_window.toolbar_main), 1);
	gtk_toolbar_set_style(GTK_TOOLBAR(main_window.toolbar_main), GTK_TOOLBAR_ICONS);

	// Add the File operations to the Main Toolbar
	main_window.toolbar_main_button_new = gtk_toolbar_insert_stock(GTK_TOOLBAR(main_window.toolbar_main),GTK_STOCK_NEW, _("New File"), NULL, NULL, NULL, -1);
	gtk_widget_show(main_window.toolbar_main_button_new);
	main_window.toolbar_main_button_open = gtk_toolbar_insert_stock(GTK_TOOLBAR(main_window.toolbar_main),GTK_STOCK_OPEN, _("Open File"), NULL, NULL, NULL, -1);
	gtk_widget_show(main_window.toolbar_main_button_open);
	main_window.toolbar_main_button_save = gtk_toolbar_insert_stock(GTK_TOOLBAR(main_window.toolbar_main),GTK_STOCK_SAVE, _("Save File"), NULL, NULL, NULL, -1);
	gtk_widget_show(main_window.toolbar_main_button_save);
	main_window.toolbar_main_button_save_as = gtk_toolbar_insert_stock(GTK_TOOLBAR(main_window.toolbar_main),GTK_STOCK_SAVE_AS, _("Save File As..."), NULL, NULL, NULL, -1);
	gtk_widget_show(main_window.toolbar_main_button_save_as);
	main_window.toolbar_main_button_close = gtk_toolbar_insert_stock(GTK_TOOLBAR(main_window.toolbar_main),GTK_STOCK_CLOSE, _("Close File"), NULL, NULL, NULL, -1);
	gtk_widget_show(main_window.toolbar_main_button_close);
	gtk_signal_connect(GTK_OBJECT(main_window.toolbar_main_button_open), "clicked", GTK_SIGNAL_FUNC(on_open1_activate), NULL);
	gtk_signal_connect(GTK_OBJECT(main_window.toolbar_main_button_new), "clicked", GTK_SIGNAL_FUNC(on_new1_activate), NULL);
	gtk_signal_connect(GTK_OBJECT(main_window.toolbar_main_button_save), "clicked", GTK_SIGNAL_FUNC(on_save1_activate), NULL);
	gtk_signal_connect(GTK_OBJECT(main_window.toolbar_main_button_save_as), "clicked", GTK_SIGNAL_FUNC(on_save_as1_activate), NULL);
	gtk_signal_connect(GTK_OBJECT(main_window.toolbar_main_button_close), "clicked", GTK_SIGNAL_FUNC(on_close1_activate), NULL);
	gtk_toolbar_append_space(GTK_TOOLBAR(main_window.toolbar_main));

	// Add the Undo operations to the Main Toolbar
	main_window.toolbar_main_button_undo = gtk_toolbar_insert_stock(GTK_TOOLBAR(main_window.toolbar_main),GTK_STOCK_UNDO, _("Undo last change"), NULL, NULL, NULL, -1);
	gtk_widget_show(main_window.toolbar_main_button_undo);
	main_window.toolbar_main_button_redo = gtk_toolbar_insert_stock(GTK_TOOLBAR(main_window.toolbar_main),GTK_STOCK_REDO, _("Redo last change"), NULL, NULL, NULL, -1);
	gtk_widget_show(main_window.toolbar_main_button_redo);
	gtk_signal_connect(GTK_OBJECT(main_window.toolbar_main_button_undo), "clicked", GTK_SIGNAL_FUNC(on_undo1_activate), NULL);
	gtk_signal_connect(GTK_OBJECT(main_window.toolbar_main_button_redo), "clicked", GTK_SIGNAL_FUNC(on_redo1_activate), NULL);
	gtk_toolbar_append_space(GTK_TOOLBAR(main_window.toolbar_main));

	// Add the Clipboard operations to the Main Toolbar
	main_window.toolbar_main_button_cut = gtk_toolbar_insert_stock(GTK_TOOLBAR(main_window.toolbar_main),GTK_STOCK_CUT, _("Cut current selection"), NULL, NULL, NULL, -1);
	gtk_signal_connect(GTK_OBJECT(main_window.toolbar_main_button_cut), "clicked", GTK_SIGNAL_FUNC(on_cut1_activate), NULL);
	gtk_widget_show(main_window.toolbar_main_button_cut);
	main_window.toolbar_main_button_copy = gtk_toolbar_insert_stock(GTK_TOOLBAR(main_window.toolbar_main),GTK_STOCK_COPY, _("Copy current selection"), NULL, NULL, NULL, -1);
	gtk_signal_connect(GTK_OBJECT(main_window.toolbar_main_button_copy), "clicked", GTK_SIGNAL_FUNC(on_copy1_activate), NULL);
	gtk_widget_show(main_window.toolbar_main_button_save_as);
	main_window.toolbar_main_button_paste = gtk_toolbar_insert_stock(GTK_TOOLBAR(main_window.toolbar_main),GTK_STOCK_PASTE, _("Paste current selection"), NULL, NULL, NULL, -1);
	gtk_signal_connect(GTK_OBJECT(main_window.toolbar_main_button_paste), "clicked", GTK_SIGNAL_FUNC(on_paste1_activate), NULL);
	gtk_widget_show(main_window.toolbar_main_button_save_as);
	gtk_toolbar_append_space(GTK_TOOLBAR(main_window.toolbar_main));

	// Add the Search operations to the Main Toolbar
	main_window.toolbar_main_button_find = gtk_toolbar_insert_stock(GTK_TOOLBAR(main_window.toolbar_main),GTK_STOCK_FIND, _("Find text"), NULL, NULL, NULL, -1);
	gtk_widget_show(main_window.toolbar_main_button_find);
	gtk_signal_connect(GTK_OBJECT(main_window.toolbar_main_button_find), "clicked", GTK_SIGNAL_FUNC(on_find1_activate), NULL);
	main_window.toolbar_main_button_replace = gtk_toolbar_insert_stock(GTK_TOOLBAR(main_window.toolbar_main),GTK_STOCK_FIND_AND_REPLACE, _("Find/Replace text"), NULL, NULL, NULL, -1);
	gtk_signal_connect(GTK_OBJECT(main_window.toolbar_main_button_replace), "clicked", GTK_SIGNAL_FUNC(on_replace1_activate), NULL);
	gtk_widget_show(main_window.toolbar_main_button_replace);

	// Create the Search Toolbar
	main_window.toolbar_find = gtk_toolbar_new();
	gtk_widget_show(main_window.toolbar_find);
	gnome_app_add_toolbar(GNOME_APP(main_window.window), GTK_TOOLBAR(main_window.toolbar_find), "toolbar_search",
	                       BONOBO_DOCK_ITEM_BEH_NORMAL, BONOBO_DOCK_TOP, 2, 0, 0);
	gtk_container_set_border_width(GTK_CONTAINER(main_window.toolbar_find), 1);

	main_window.toolbar_find_search_label = gtk_label_new(_("Search for: "));
	gtk_widget_show(main_window.toolbar_find_search_label);
	gtk_toolbar_append_widget(GTK_TOOLBAR(main_window.toolbar_find), main_window.toolbar_find_search_label,NULL,NULL);

	main_window.toolbar_find_search_entry = gtk_entry_new();
	gtk_widget_show(main_window.toolbar_find_search_entry);
	gtk_toolbar_append_widget(GTK_TOOLBAR(main_window.toolbar_find), main_window.toolbar_find_search_entry, _("Incremental search"),NULL);
	gtk_signal_connect_after(GTK_OBJECT(main_window.toolbar_find_search_entry), "insert_text", GTK_SIGNAL_FUNC(inc_search_typed), NULL);
	gtk_signal_connect_after(GTK_OBJECT(main_window.toolbar_find_search_entry), "key_release_event", GTK_SIGNAL_FUNC(inc_search_key_release_event), NULL);
	gtk_signal_connect_after(GTK_OBJECT(main_window.toolbar_find_search_entry), "activate", GTK_SIGNAL_FUNC(inc_search_activate), NULL);
	gtk_toolbar_append_space(GTK_TOOLBAR(main_window.toolbar_find));

	main_window.toolbar_find_goto_label = gtk_label_new(_("Go to line: "));
	gtk_widget_show(main_window.toolbar_find_goto_label);
	gtk_toolbar_append_widget(GTK_TOOLBAR(main_window.toolbar_find), main_window.toolbar_find_goto_label,NULL,NULL);

	main_window.toolbar_find_goto_entry = gtk_entry_new_with_max_length(6);
	gtk_entry_set_width_chars(GTK_ENTRY(main_window.toolbar_find_goto_entry),7);
	gtk_widget_show(main_window.toolbar_find_goto_entry);
	gtk_toolbar_append_widget(GTK_TOOLBAR(main_window.toolbar_find), main_window.toolbar_find_goto_entry, _("Go to line"),NULL);
	gtk_signal_connect_after(GTK_OBJECT(main_window.toolbar_find_goto_entry), "activate", GTK_SIGNAL_FUNC(goto_line_activate), NULL);
}


static void main_window_create_appbar(void)
{
	main_window.appbar = gnome_appbar_new(TRUE, TRUE, GNOME_PREFERENCES_NEVER);
	gtk_widget_show(main_window.appbar);
	gnome_app_set_statusbar(GNOME_APP(main_window.window), main_window.appbar);
}


static void main_window_create_panes(void)
{
	main_window.main_vertical_pane = gtk_vpaned_new();
	gtk_widget_show(main_window.main_vertical_pane);
	gnome_app_set_contents(GNOME_APP(main_window.window), main_window.main_vertical_pane);

	main_window.main_horizontal_pane = gtk_hpaned_new();
	gtk_widget_show(main_window.main_horizontal_pane);
	gtk_paned_pack1(GTK_PANED(main_window.main_vertical_pane), main_window.main_horizontal_pane, FALSE, TRUE);

	gtk_signal_connect(GTK_OBJECT(main_window.window), "size_allocate", GTK_SIGNAL_FUNC(classbrowser_accept_size), NULL);
	move_classbrowser_position();
	if(gnome_config_get_int("connectED/main_window/classbrowser_hidden=0") == 1)
		classbrowser_hide();
}


static void main_window_fill_panes(void){
	GtkWidget *box;
	GtkWidget *box2;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	box = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(box);
	gtk_paned_pack1(GTK_PANED(main_window.main_horizontal_pane), box, FALSE, TRUE);

	main_window.scrolledwindow3 = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_show(main_window.scrolledwindow3);
	gtk_box_pack_start(GTK_BOX(box), main_window.scrolledwindow3, TRUE, TRUE, 0);
	gtk_paned_pack1(GTK_PANED(main_window.main_horizontal_pane), main_window.scrolledwindow3, FALSE, TRUE);

	box2 = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(box2);
	main_window.treeviewlabel = gtk_label_new(_("FILE: "));
	gtk_label_set_justify(GTK_LABEL(main_window.treeviewlabel), GTK_JUSTIFY_LEFT);
	gtk_widget_show(main_window.treeviewlabel);
	gtk_box_pack_start(GTK_BOX(box2), main_window.treeviewlabel, FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(box), box2, FALSE, FALSE, 4);

	main_window.classtreestore = gtk_tree_store_new(N_COLUMNS, G_TYPE_STRING,
	                             G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT);

	main_window.classtreeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(main_window.classtreestore));
	gtk_widget_show(main_window.classtreeview);
	gtk_container_add(GTK_CONTAINER(main_window.scrolledwindow3), main_window.classtreeview);

	main_window.classtreeselect = gtk_tree_view_get_selection(GTK_TREE_VIEW(main_window.classtreeview));
	gtk_tree_selection_set_mode(main_window.classtreeselect, GTK_SELECTION_SINGLE);
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(_("Name"),
	         renderer, "text", NAME_COLUMN, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(main_window.classtreeview), column);

	main_window.scrolledwindow1 = gtk_scrolled_window_new(NULL, NULL);
	gtk_paned_pack2(GTK_PANED(main_window.main_vertical_pane), main_window.scrolledwindow1, FALSE, TRUE);

	main_window.lint_view = gtk_tree_view_new();
	gtk_container_add(GTK_CONTAINER(main_window.scrolledwindow1), main_window.lint_view);
	main_window.lint_renderer = gtk_cell_renderer_text_new();
	main_window.lint_column = gtk_tree_view_column_new_with_attributes(_("Syntax Check Output"),
	                          main_window.lint_renderer,
	                          "text", 0,
	                          NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(main_window.lint_view), main_window.lint_column);
	gtk_widget_set_usize(main_window.lint_view,80,80);
	main_window.lint_select = gtk_tree_view_get_selection(GTK_TREE_VIEW(main_window.lint_view));
	gtk_tree_selection_set_mode(main_window.lint_select, GTK_SELECTION_SINGLE);
	g_signal_connect(G_OBJECT(main_window.lint_select), "changed",
	                  G_CALLBACK(lint_row_activated), NULL);

	main_window.notebook_editor = gtk_notebook_new();
	gtk_notebook_set_scrollable(GTK_NOTEBOOK(main_window.notebook_editor), TRUE);
	//GTK_WIDGET_UNSET_FLAGS(main_window.notebook_editor,  GTK_CAN_FOCUS | GTK_RECEIVES_DEFAULT);
	// Fix to scrollable list of tabs, however it then messes up grabbing of focus
	// Hence the focus-tab event(which GTK doesn't seem to recognise
	GTK_WIDGET_UNSET_FLAGS(main_window.notebook_editor, GTK_RECEIVES_DEFAULT);
	gtk_widget_show(main_window.notebook_editor);
	gtk_paned_pack2(GTK_PANED(main_window.main_horizontal_pane), main_window.notebook_editor, TRUE, TRUE);
	gtk_widget_set_usize(main_window.notebook_editor,400,400);
	g_signal_connect(G_OBJECT(main_window.notebook_editor), "switch_page", GTK_SIGNAL_FUNC(on_notebook_switch_page), NULL);
}

guint get_longest_matching_length(gchar *filename1, gchar *filename2){
	gchar *base1, *base1_alloc;
	gchar *base2, *base2_alloc;
	guint length;
	guint maxlength;

	base1 = g_path_get_dirname(filename1);
	base1_alloc = base1;
	base2 = g_path_get_dirname(filename2);
	base2_alloc = base2;

	maxlength = strlen(base1);
	if(strlen(base2)<maxlength) {
		maxlength = strlen(base2);
	}

	length = 0;
	if(strcmp(base2_alloc, ".")!=0 && strcmp(base2_alloc, ".")!=0) {
		while((*base1 == *base2) &&(*base1 != 0) &&(*base2 != 0) && length<=maxlength) {
			base1++;
			base2++;
			length++;
		}
	}

	g_free(base1_alloc);
	g_free(base2_alloc);

	return length;
}


GString *get_differing_part(GSList *filenames, gchar *file_requested){
	GSList *temp_list;
	gchar buffer[1024];
	guint longest_match;
	guint match;

	longest_match = 9999;

	// Seems to miss the first one - if that's not required, change to temp_list = filenames
	for(temp_list = g_slist_next(filenames); temp_list!= NULL; temp_list = g_slist_next(temp_list)) {
		match = get_longest_matching_length(temp_list->data, file_requested);
		if(match < longest_match) {
			longest_match = match;
		}
	}

	if(longest_match!=9999) {
		if(*(file_requested + longest_match) == '/') {
			strcpy(buffer,(file_requested + longest_match+1));
		}
		else {
			strcpy(buffer,(file_requested + longest_match));
		}
	}
	else {
		strcpy(buffer, file_requested);
	}

	return g_string_new(buffer);
}


GString *get_differing_part_editor(Editor *editor){
	gchar *cwd;
	GSList *list_editors;
	GSList *list_filenames;
	Editor *data;
	gchar *str;
	GString *result;

	if(editor == NULL) 
		return NULL;
	
	cwd = g_get_current_dir();

	list_filenames = NULL;
	list_filenames = g_slist_append(list_filenames, cwd);

	for(list_editors = editors; list_editors!= NULL; list_editors = g_slist_next(list_editors)) {
		data = list_editors->data;
		if(data->type == TAB_FILE) {
			str =((Editor *)data)->filename->str;
			list_filenames = g_slist_append(list_filenames, str);
		}
	}

	result = get_differing_part(list_filenames, editor->filename->str);
	g_free(cwd);
	return result;
}


void update_app_title(void){
	GString *title;

	if(main_window.current_editor) {
		if(main_window.current_editor->filename) {
			title = get_differing_part_editor(main_window.current_editor);
			if(title) {
				if(main_window.current_editor->saved == TRUE) {
					g_string_append(title, _(" - connectED"));
				}
				else {
					g_string_append(title, _("(modified) - connectED"));
				}
			}
			else {
				title = g_string_new("connectED");
			}
			gtk_window_set_title(GTK_WINDOW(main_window.window), title->str);
		}
		else {
			gtk_window_set_title(GTK_WINDOW(main_window.window), "connectED");
		}
	}
	else {
		gtk_window_set_title(GTK_WINDOW(main_window.window), "connectED");
	}
}


gint minimum(gint val1, gint val2){
	return (val1 < val2) ? val1 : val2;
}


gint maximum(gint val1, gint val2){
	return (val1 > val2) ? val1 : val2;
}

GList *Plugins = NULL;

/****/

int plugin_discover_type(GString *filename){
	GString *command_line;
	gchar *stdout = NULL;
	GError *error = NULL;
	gint exit_status;
	gint type = connectED_PLUGIN_TYPE_UNKNOWN;
	gint stdout_len;
	
	command_line = g_string_new(filename->str);
	command_line = g_string_prepend(command_line, "'");
	command_line = g_string_append(command_line, "' -type");
	
	if( !(g_spawn_command_line_sync(command_line->str,&stdout,NULL, &exit_status,&error)) ){
		g_print("Spawning %s gave error %s\n", filename->str, error->message);
		g_string_free(command_line, TRUE);
		return connectED_PLUGIN_TYPE_ERROR;
	}
				
	stdout_len = strlen(stdout);
		
	if(strncmp(stdout, "SELECTION", MIN(stdout_len, 9))==0) {
		type = connectED_PLUGIN_TYPE_SELECTION;
	} else if(strncmp(stdout, "NO-INPUT", MIN(stdout_len, 8))==0) {
		type = connectED_PLUGIN_TYPE_NOINPUT;
	} else if(strncmp(stdout, "FNAME", MIN(stdout_len, 5))==0) {
		type = connectED_PLUGIN_TYPE_FILENAME;
	} else if(strncmp(stdout, "DEBUG", MIN(stdout_len, 5))==0) {
		type = connectED_PLUGIN_TYPE_DEBUG;
	}
		
	g_free(stdout);
	g_string_free(command_line, TRUE);
	
	return type;
}//plugin_discover_type

gint sort_plugin_func(gconstpointer a, gconstpointer b){
	Plugin *plugina =(Plugin *)a;
	Plugin *pluginb =(Plugin *)b;

	if(strcmp(plugina->name, pluginb->name) < 0)
		return -1;
	
	return 1;
}//sort_plugin_func

void plugin_discover_available(void){
	GDir *dir;
	const gchar *plugin_name;
	Plugin *plugin;
	GString *user_plugin_dir;
	GString *filename;
	
	user_plugin_dir = g_string_new( g_get_home_dir());
	user_plugin_dir = g_string_append(user_plugin_dir, "/.connectED/plugins/");
	if(g_file_test(user_plugin_dir->str, G_FILE_TEST_IS_DIR)) {
		dir = g_dir_open(user_plugin_dir->str, 0,NULL);
		if(dir) {
			for(plugin_name = g_dir_read_name(dir); plugin_name != NULL; plugin_name = g_dir_read_name(dir)) {
				// Recommended by __tim in #gtk+ on irc.freenode.net 27/10/2004 11:30
				plugin = g_new0(Plugin, 1);
				plugin->name = g_strdup(plugin_name);
				// TODO: Could do with replacing ' in name with \' for spawn
				filename = g_string_new(plugin->name);
				filename = g_string_prepend(filename, user_plugin_dir->str);
				plugin->filename = filename;
				//g_print("PLUGIN FILENAME: %s\n", plugin->filename->str);
				plugin->type = plugin_discover_type(plugin->filename);
				Plugins = g_list_append(Plugins, plugin);
				//g_print("%s\n", plugin_name);
			}
			g_dir_close(dir);			
		}
	}
	g_string_free(user_plugin_dir, TRUE);

	if(g_file_test("/usr/share/connectED/plugins/", G_FILE_TEST_IS_DIR)) {
		dir = g_dir_open("/usr/share/connectED/plugins/", 0,NULL);
		if(dir) {
			for(plugin_name = g_dir_read_name(dir); plugin_name != NULL; plugin_name = g_dir_read_name(dir)) {
				// Recommended by __tim in #gtk+ on irc.freenode.net 27/10/2004 11:30
				plugin = g_new0(Plugin, 1);
				plugin->name = g_strdup(plugin_name);
				filename = g_string_new(plugin_name);
				filename = g_string_prepend(filename, "/usr/share/connectED/plugins/");
				plugin->filename = filename;
				plugin->type = plugin_discover_type(plugin->filename);
				Plugins = g_list_append(Plugins, plugin);
			}
			g_dir_close(dir);			
		}
	}

	Plugins = g_list_sort(Plugins, sort_plugin_func);
}//plugin_discover_available

void plugin_create_menu_items()
{
	GList *iterator;
	Plugin *plugin;
	guint num_plugin;
	guint hide_plugin;

	GtkBin *bin = NULL;
	GtkLabel *label;;
	
	num_plugin = 0;
	for(iterator = Plugins; iterator != NULL && num_plugin<NUM_PLUGINS_MAX; iterator = g_list_next(iterator)) {
		plugin =(Plugin *)(iterator->data);
		//g_print("Plugin %d:%s\n", num_plugin, plugin->filename);
		
		//g_print("Getting child widget\n");
				
		bin = GTK_BIN(plugin_menu[num_plugin].widget);
				
		//g_print("Bin is %p\n", bin);
		if(bin) {
			label = GTK_LABEL(gtk_bin_get_child(bin));
		
			gtk_label_set_text(label, plugin->name);

			// gtk_widget_hide(plugin_menu[num_plugin].widget);
			gtk_widget_show(plugin_menu[num_plugin].widget);
		}
		
		num_plugin++;
	}

	//g_print("Blanking all non-found plugin entries\n");

	for(hide_plugin=num_plugin; hide_plugin <NUM_PLUGINS_MAX; hide_plugin++) {
		gtk_widget_hide(plugin_menu[hide_plugin].widget);
	}
}

void plugin_exec(gint plugin_num)
{
	Plugin *plugin;
	gchar *stdout = NULL;
	GError *error = NULL;
	gint exit_status;
	GString *command_line = NULL;
	gint wordStart;
	gint wordEnd;
	gchar *current_selection;
	gint ac_length;
	gchar *data;
	
	if(main_window.current_editor == NULL) {
		return;
	}
	
	plugin =(Plugin *)g_list_nth_data(Plugins, plugin_num);
	if(!plugin) {
		g_print(_("Plugin is null!\n"));
	}
	//g_print("Plugin No: %d:%d(%s):%s\n", plugin_num, plugin->type, plugin->name, plugin->filename->str);
	command_line = g_string_new(plugin->filename->str);
	command_line = g_string_prepend(command_line, "'");
	command_line = g_string_append(command_line, "' '");
	
	if(plugin->type == connectED_PLUGIN_TYPE_SELECTION) {
		wordStart = gtk_scintilla_get_selection_start(GTK_SCINTILLA(main_window.current_editor->scintilla));
		wordEnd = gtk_scintilla_get_selection_end(GTK_SCINTILLA(main_window.current_editor->scintilla));
		current_selection = gtk_scintilla_get_text_range(GTK_SCINTILLA(main_window.current_editor->scintilla), wordStart, wordEnd, &ac_length);
		
		command_line = g_string_append(command_line, current_selection);
	}
	else if(plugin->type == connectED_PLUGIN_TYPE_FILENAME) {
		command_line = g_string_append(command_line, editor_convert_to_local(main_window.current_editor));		
	}
	command_line = g_string_append(command_line, "'");
	
	//g_print("SPAWNING: %s\n", command_line->str);
	
	if(g_spawn_command_line_sync(command_line->str,&stdout,NULL, &exit_status,&error)) {
		data = strstr(stdout, "\n");
		data++;
		
		//g_print("COMMAND: %s\nSTDOUT:%s\nOUTPUT: %s\n", command_line->str, stdout, data);
		
		if(g_strncasecmp(stdout, "INSERT", MIN(strlen(stdout), 6))==0) {
			if(data) {
				gtk_scintilla_insert_text(GTK_SCINTILLA(main_window.current_editor->scintilla), 
					gtk_scintilla_get_current_pos(GTK_SCINTILLA(main_window.current_editor->scintilla)), data);
			}
		}
		else if(g_strncasecmp(stdout, "REPLACE", MIN(strlen(stdout), 7))==0) {
			if(data) {
				gtk_scintilla_replace_sel(GTK_SCINTILLA(main_window.current_editor->scintilla), data);
			}
		}
		else if(g_strncasecmp(stdout, "MESSAGE", MIN(strlen(stdout),7))==0) {
			  info_dialog(plugin->name, data);
		}
		else if(g_strncasecmp(stdout, "OPEN", MIN(strlen(stdout), 4))==0) {
			if(DEBUG_MODE) { g_print("DEBUG: main_window.c:plugin_exec: Opening file :date: %s\n", data); }
			switch_to_file_or_open(data, 0);
		}
		else if(g_strncasecmp(stdout, "DEBUG", MIN(strlen(stdout), 5))==0) {
			debug_dump_editors();
			DEBUG_MODE = TRUE;
		}
		
		g_free(stdout);

	}
	else {
		g_print(_("Spawning %s gave error %s\n"), plugin->filename->str, error->message);
	}
}


void plugin_setup_menu(void)
{
	plugin_discover_available();
	if(g_list_length(Plugins)>0) {
		plugin_create_menu_items();
	}
	else {
		gtk_widget_hide(menubar1_uiinfo[3].widget);
	}
}

/****/

void main_window_update_reopen_menu(void)
{
	gchar *full_filename;
	GString *key;
	//gchar *short_filename;
	guint entry;
	GtkBin *bin = NULL;
	
	for(entry=0; entry<NUM_REOPEN_MAX; entry++) {
		key = g_string_new("connectED/recent/");
		g_string_append_printf(key, "%d=NOTFOUND", entry);
		full_filename = gnome_config_get_string(key->str);
		g_string_free(key, TRUE);
		
		//g_print("Recent DEBUG: Entry %d: %s\n", entry, full_filename);
		
		if(strcmp(full_filename, "NOTFOUND")!=0) {
			bin = GTK_BIN(recent_menu[entry].widget);
			if(bin) {
				gtk_label_set_text(GTK_LABEL(gtk_bin_get_child(bin)), full_filename);
				gtk_widget_show(recent_menu[entry].widget);
			}
		}
		else {
			gtk_widget_hide(recent_menu[entry].widget);
		}
	}
}

void main_window_add_to_reopen_menu(gchar *full_filename)
{
	guint entry;
	gchar *found;
	GString *key;
	guint found_id;
	
	// Find current filename in list
	found_id = -1;
	for(entry=0; entry<NUM_REOPEN_MAX; entry++) {
		key = g_string_new("connectED/recent/");
		g_string_append_printf(key, "%d=NOTFOUND", entry);
		found = gnome_config_get_string(key->str);
		g_string_free(key, TRUE);
		
		if(strcmp(full_filename, found)==0) {
			found_id = entry;
			break;
		}
	}
	
	// if not found, drop the last one off the end(i.e. pretend it was found in the last position)
	if(found_id == -1) {
		found_id = NUM_REOPEN_MAX-1;
	}

	// replace from found_id to 1 with entry above
	for(entry=found_id; entry > 0; entry--) {
		key = g_string_new("connectED/recent/");
		g_string_append_printf(key, "%d=NOTFOUND", entry-1);
		found = gnome_config_get_string(key->str);
		g_string_free(key, TRUE);

		key = g_string_new("connectED/recent/");
		g_string_append_printf(key, "%d", entry);
		gnome_config_set_string(key->str, found);
		g_string_free(key, TRUE);
	}

	// set entry 0 to be new entry
	gnome_config_set_string("connectED/recent/0", full_filename);
		
	main_window_update_reopen_menu();
	gnome_config_sync();	
}


void main_window_create(void)
{
	gnome_window_icon_set_default_from_file(PIXMAP_DIR "/" connectED_PIXMAP_ICON);

	main_window.window = gnome_app_new("connectED", "connectED");
	preferences_apply();
	
	gnome_app_create_menus(GNOME_APP(main_window.window), menubar1_uiinfo);
	main_window_create_appbar();
	gnome_app_install_menu_hints(GNOME_APP(main_window.window), menubar1_uiinfo);

	main_window_update_reopen_menu();
	
	plugin_setup_menu();
	
	main_window_create_toolbars();
	main_window_create_panes();
	main_window_fill_panes();

	function_list_prepare();

	gtk_signal_connect(GTK_OBJECT(main_window.window), "delete_event", GTK_SIGNAL_FUNC(main_window_delete_event), NULL);
	gtk_signal_connect(GTK_OBJECT(main_window.window), "destroy", GTK_SIGNAL_FUNC(main_window_destroy_event), NULL);
	gtk_signal_connect(GTK_OBJECT(main_window.window), "key_press_event", GTK_SIGNAL_FUNC(main_window_key_press_event), NULL);
	gtk_signal_connect(GTK_OBJECT(main_window.window), "size_allocate", GTK_SIGNAL_FUNC(main_window_resize), NULL);
	gtk_signal_connect(GTK_OBJECT(main_window.window), "window-state-event", GTK_SIGNAL_FUNC(main_window_state_changed), NULL);

	main_window.clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);

	gtk_widget_show(main_window.window);

	update_app_title();
}
