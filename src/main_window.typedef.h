/*
 * connectED is a colaborative and social networking IDE for GTK+/GNOME.
 * Copyright (c) 2007-2009 Kaity G. B.
 * 	<uberChick -at- uberChicGeekChick.Com>
 *
 * For more information or to find the latest release, visit our
 * website at: http://uberChicGeekChick.Com/?Project=connectED
 *
 * Writen by an uberChick, other uberChicks please meet me & others @:
 * 	http://uberChicks.Net/
 *
 * I'm also disabled. I live with a progressive neuro-muscular disease.
 * DYT1+ Early-Onset Generalized Dystonia, a type of Generalized Dystonia.
 * 	http://Dystonia-DREAMS.Org/
 */

/*
 * Unless explicitly acquired and licensed from Licensor under another
 * license, the contents of this file are subject to the Reciprocal Public
 * License ("RPL") Version 1.5, or subsequent versions as allowed by the RPL,
 * and You may not copy or use this file in either source code or executable
 * form, except in compliance with the terms and conditions of the RPL.
 *
 * All software distributed under the RPL is provided strictly on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, AND
 * LICENSOR HEREBY DISCLAIMS ALL SUCH WARRANTIES, INCLUDING WITHOUT
 * LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, QUIET ENJOYMENT, OR NON-INFRINGEMENT. See the RPL for specific
 * language governing rights and limitations under the RPL.
 *
 * ---------------------------------------------------------------------------
 * |	A copy of the RPL 1.5 may be found with this project or online @:    |
 * |		http://opensource.org/licenses/rpl1.5.txt                    |
 * ---------------------------------------------------------------------------
 */

#ifndef MAIN_WINDOW_TYPEDEF_H
#define MAIN_WINDOW_TYPEDEF_H

typedef struct _MainWindow MainWindow;

struct _MainWindow{
	GnomeApp	*app;
	GtkWindow *window;

	GtkWidget *toolbar_main;
	GtkWidget *toolbar_main_button_new;
	GtkWidget *toolbar_main_button_open;
	GtkWidget *toolbar_main_button_save;
	GtkWidget *toolbar_main_button_save_as;
	GtkWidget *toolbar_main_button_close;
	GtkWidget *toolbar_main_button_undo;
	GtkWidget *toolbar_main_button_redo;
	GtkWidget *toolbar_main_button_cut;
	GtkWidget *toolbar_main_button_copy;
	GtkWidget *toolbar_main_button_paste;
	GtkWidget *toolbar_main_button_find;
	GtkWidget *toolbar_main_button_replace;

	GtkWidget *toolbar_find;
	GtkWidget *toolbar_find_search_label;
	GtkWidget *toolbar_find_search_entry;
	GtkWidget *toolbar_find_goto_label;
	GtkWidget *toolbar_find_goto_entry;

	GtkWidget *main_vertical_pane;
	GtkHPaned *horizontal_pane;

	GtkWidget *appbar;

	GtkWidget *notebook_manager;
	GtkNotebook	*notebook_editor;

	GtkWidget		*scrolledwindow1;
	GtkListStore		*lint_store;
	GtkCellRenderer		*lint_renderer;
	GtkTreeView		*lint_view;
	GtkTreeViewColumn	*lint_column;
	GtkTreeSelection	*lint_select;

	Editor *current_editor;

	GtkScrolledWindow *class_browser_scrolled_window;
	GtkTreeStore *class_browser_tree_store;
	GtkTreeView *class_browser_tree_view;
	GtkTreeSelection *class_browser_tree_select;
	GtkLabel *tree_view_label;
	GtkLabel *label1;
	
	GtkClipboard	*clipboard;
};




#endif /* MAIN_WINDOW_TYPEDEF_H */



