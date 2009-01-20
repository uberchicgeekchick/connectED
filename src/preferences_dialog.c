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


#include "preferences_dialog.h"
#include "preferences.h"
#include "main_window.h"
#include "tab_php.h"
#include "tab.h"
#include "templates.h"
#include "edit_template.h"
#include <gtkscintilla.h>

#define IS_FONT_NAME(name1, name2) strncmp(name1, name2, MIN(strlen(name1), strlen(name2))) == 0

PreferencesDialog preferences_dialog;
Preferences temp_preferences;
gchar *current_highlighting_element = NULL;
gchar *current_key = NULL;

static int cmp_families(const void *a, const void *b)
{
	const char *a_name = pango_font_family_get_name(*(PangoFontFamily **)a);
	const char *b_name = pango_font_family_get_name(*(PangoFontFamily **)b);
  
	return g_utf8_collate(a_name, b_name);
}

static GList * get_font_names()
{
	PangoFontFamily **families;
	gint n_families, i;
	GList *fonts = NULL;
  
	pango_context_list_families(gtk_widget_get_pango_context(GTK_WIDGET(main_window.window)),
		&families, &n_families);
	qsort(families, n_families, sizeof(PangoFontFamily *), cmp_families);

	for(i=0; i<n_families; i++) {
		const gchar *name = pango_font_family_get_name(families[i]);
		fonts = g_list_append(fonts,(gchar *)name);
    }
	
	return fonts;
}

static GList *get_font_sizes()
{
	GList *sizes = NULL;
	
	// Again, there's gotta be a nicer way to do this!!!!  AJ 2004-02-09
	
	sizes = g_list_append(sizes, "6");
	sizes = g_list_append(sizes, "8");
	sizes = g_list_append(sizes, "10");
	sizes = g_list_append(sizes, "12");
	sizes = g_list_append(sizes, "14");
	sizes = g_list_append(sizes, "16");
	sizes = g_list_append(sizes, "18");
	sizes = g_list_append(sizes, "20");
	sizes = g_list_append(sizes, "22");
	sizes = g_list_append(sizes, "24");
	sizes = g_list_append(sizes, "26");
	sizes = g_list_append(sizes, "32");
	sizes = g_list_append(sizes, "48");
	sizes = g_list_append(sizes, "72");
	
	return sizes;
}

GString *create_sample_text()
{
	GString *ret;
	
	ret = g_string_new("<?php\n\n");

	ret = g_string_append(ret, "/* A class to implement a car\n");
	ret = g_string_append(ret, "   by Mr Somebody */\n\n");
	ret = g_string_append(ret, "class Car extends Vehicle\n");
	ret = g_string_append(ret, "	implements EAccident\n");
	ret = g_string_append(ret, "{\n");
	ret = g_string_append(ret, "	private $PetrolTankFull = true;\n");
	ret = g_string_append(ret, "	protected $Name = \"betty\";\n");
	ret = g_string_append(ret, "	public $YearMade = 1999;\n\n");
	
	ret = g_string_append(ret, "	function __construct()\n");
	ret = g_string_append(ret, "	{\n");
	ret = g_string_append(ret, "		parent::__construct();\n");
	ret = g_string_append(ret, "		print \"Made $Name!\";\n");
	ret = g_string_append(ret, "	}\n");
	ret = g_string_append(ret, "	\n");
	ret = g_string_append(ret, "	private function go()\n");
	ret = g_string_append(ret, "	{\n");
	ret = g_string_append(ret, "		// Just go for it!\n");
	ret = g_string_append(ret, "		print 'Go!!!';\n");
	ret = g_string_append(ret, "	}\n");
	ret = g_string_append(ret, "}\n\n");

	ret = g_string_append(ret, "?>\n\n");

	ret = g_string_append(ret, "<html>\n");
	ret = g_string_append(ret, " <head>\n");
	ret = g_string_append(ret, "  <title>My test page</title>\n");
	ret = g_string_append(ret, " </head>\n\n");
 
	ret = g_string_append(ret, " <body>\n");
	ret = g_string_append(ret, "  <h1 class='winter'>Test</h1>\n");
	ret = g_string_append(ret, " </body>\n");
	ret = g_string_append(ret, "</html>	\n");
	return ret;
}

void get_current_preferences(void)
{
	GList *highlighting_list = NULL;

	// Copy the global preferences struct in to a local copy for editing(in case they click cancel)
	memcpy(&temp_preferences, &preferences, sizeof(preferences));

	// Font drop down elements : General
	highlighting_list = g_list_append(highlighting_list, _("Default"));
	highlighting_list = g_list_append(highlighting_list, _("Line Number(margin)"));

	// Font drop down elements : HTML
	highlighting_list = g_list_append(highlighting_list, _("HTML Tag"));
	highlighting_list = g_list_append(highlighting_list, _("HTML Unknown Tag"));
	highlighting_list = g_list_append(highlighting_list, _("HTML Attribute"));
	highlighting_list = g_list_append(highlighting_list, _("HTML Unknown Attribute"));
	highlighting_list = g_list_append(highlighting_list, _("HTML Number"));
	highlighting_list = g_list_append(highlighting_list, _("HTML Single-quoted String"));
	highlighting_list = g_list_append(highlighting_list, _("HTML Double-quoted String"));
	highlighting_list = g_list_append(highlighting_list, _("HTML Comment"));
	highlighting_list = g_list_append(highlighting_list, _("HTML Entity"));
	highlighting_list = g_list_append(highlighting_list, _("HTML Script"));
	highlighting_list = g_list_append(highlighting_list, _("HTML Question"));
	highlighting_list = g_list_append(highlighting_list, _("HTML Value"));

	// Font drop down elements : JavaScript
	highlighting_list = g_list_append(highlighting_list, _("JavaScript Multiple-line Comment"));
	highlighting_list = g_list_append(highlighting_list, _("JavaScript Single-line Comment"));
	highlighting_list = g_list_append(highlighting_list, _("JavaScript Document Comment"));
	highlighting_list = g_list_append(highlighting_list, _("JavaScript Word"));
	highlighting_list = g_list_append(highlighting_list, _("JavaScript Keyword"));
	highlighting_list = g_list_append(highlighting_list, _("JavaScript Single-quoted String"));
	highlighting_list = g_list_append(highlighting_list, _("JavaScript Double-quoted String"));
	highlighting_list = g_list_append(highlighting_list, _("JavaScript Symbol"));

	// Font drop down elements : PHP
	highlighting_list = g_list_append(highlighting_list, _("PHP Default"));
	highlighting_list = g_list_append(highlighting_list, _("PHP 'HString'"));
	highlighting_list = g_list_append(highlighting_list, _("PHP Simple String"));
	highlighting_list = g_list_append(highlighting_list, _("PHP Word"));
	highlighting_list = g_list_append(highlighting_list, _("PHP Number"));
	highlighting_list = g_list_append(highlighting_list, _("PHP Variable"));
	highlighting_list = g_list_append(highlighting_list, _("PHP Single-line Comment"));
	highlighting_list = g_list_append(highlighting_list, _("PHP Multiple-line Comment"));

	// Font drop down elements : CSS
	highlighting_list = g_list_append(highlighting_list, _("CSS Tag"));
	highlighting_list = g_list_append(highlighting_list, _("CSS Class"));
	highlighting_list = g_list_append(highlighting_list, _("CSS Psuedoclass"));
	highlighting_list = g_list_append(highlighting_list, _("CSS Unknown Pseudoclass"));
	highlighting_list = g_list_append(highlighting_list, _("CSS Operator"));
	highlighting_list = g_list_append(highlighting_list, _("CSS Identifier"));
	highlighting_list = g_list_append(highlighting_list, _("CSS Unknown Identifier"));
	highlighting_list = g_list_append(highlighting_list, _("CSS Value"));
	highlighting_list = g_list_append(highlighting_list, _("CSS Comment"));
	highlighting_list = g_list_append(highlighting_list, _("CSS ID"));
	highlighting_list = g_list_append(highlighting_list, _("CSS Important"));
	highlighting_list = g_list_append(highlighting_list, _("CSS Directive"));

	// Font drop down elements : SQL
	highlighting_list = g_list_append(highlighting_list, _("SQL Word"));
	highlighting_list = g_list_append(highlighting_list, _("SQL Identifier"));
	highlighting_list = g_list_append(highlighting_list, _("SQL Number"));
	highlighting_list = g_list_append(highlighting_list, _("SQL String"));
	highlighting_list = g_list_append(highlighting_list, _("SQL Operator"));
	highlighting_list = g_list_append(highlighting_list, _("SQL Comment"));

	preferences_dialog.highlighting_elements = highlighting_list;
}


void set_current_preferences(void)
{
	// Copy the local copy back to the global list
	memcpy(&preferences, &temp_preferences, sizeof(preferences));

	// Save the preferences
	preferences_save();
}

void set_controls_to_highlight(gchar *setting_name, gchar *fontname, gint fontsize, gboolean bold, gboolean italic, gint fore, gint back)
{
	gchar *sfontsize;
	gint row;
	
	// Debug print for preferences being set
	// g_print("Getting %s: %s %d %d %d %d %d\n", setting_name, fontname, fontsize, bold, italic, fore, back);
	
	preferences_dialog.changing_highlight_element=TRUE;
	
	fontname++; // Ignore the initial ! for Pango rendering
	
	//gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(preferences_dialog.font_combo)->entry), fontname);
	row = GPOINTER_TO_INT(g_object_get_qdata(G_OBJECT(preferences_dialog.font_combo), g_quark_from_string(fontname)));
	gtk_combo_box_set_active(GTK_COMBO_BOX(preferences_dialog.font_combo), row);
	sfontsize = g_strdup_printf("%d", fontsize);

	//gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(preferences_dialog.size_combo)->entry), sfontsize);
	row = GPOINTER_TO_INT(g_object_get_qdata(G_OBJECT(preferences_dialog.size_combo), g_quark_from_string(sfontsize)));
	gtk_combo_box_set_active(GTK_COMBO_BOX(preferences_dialog.size_combo), row);
	g_free(sfontsize);
	
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(preferences_dialog.bold_button), bold);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(preferences_dialog.italic_button), italic);
	
	gnome_color_picker_set_i8(GNOME_COLOR_PICKER(preferences_dialog.foreground_colour), 
		(fore & 0xff),(fore & 0xff00) >> 8,(fore & 0xff0000) >> 16, 0);
	
	gnome_color_picker_set_i8(GNOME_COLOR_PICKER(preferences_dialog.background_colour), 
		(back & 0xff),(back & 0xff00) >> 8,(back & 0xff0000) >> 16, 0);
	preferences_dialog.changing_highlight_element=FALSE;
}

void get_current_highlighting_settings(gchar *name)
{
	if(strlen(name)==0) {
		return;
	}

	if(IS_FONT_NAME(current_highlighting_element, _("Default"))) {
		set_controls_to_highlight(_("Default"), temp_preferences.default_font, temp_preferences.default_size, 
			temp_preferences.default_bold, temp_preferences.default_italic,
			temp_preferences.default_fore, temp_preferences.default_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("Line Number(margin)"))) {
		set_controls_to_highlight(_("Line Number(margin)"), temp_preferences.line_number_font, temp_preferences.line_number_size, 
			temp_preferences.line_number_bold, temp_preferences.line_number_italic,
			temp_preferences.line_number_fore, temp_preferences.line_number_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("HTML Tag"))) {
		set_controls_to_highlight(_("HTML Tag"), temp_preferences.html_tag_font, temp_preferences.html_tag_size, 
			temp_preferences.html_tag_bold, temp_preferences.html_tag_italic,
			temp_preferences.html_tag_fore, temp_preferences.html_tag_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("HTML Unknown Tag"))) {
		set_controls_to_highlight(_("HTML Unknown Tag"), temp_preferences.html_tag_unknown_font, temp_preferences.html_tag_unknown_size, 
			temp_preferences.html_tag_unknown_bold, temp_preferences.html_tag_unknown_italic,
			temp_preferences.html_tag_unknown_fore, temp_preferences.html_tag_unknown_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("HTML Attribute"))) {
		set_controls_to_highlight(_("HTML Attribute"), temp_preferences.html_attribute_font, temp_preferences.html_attribute_size, 
			temp_preferences.html_attribute_bold, temp_preferences.html_attribute_italic,
			temp_preferences.html_attribute_fore, temp_preferences.html_attribute_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("HTML Unknown Attribute"))) {
		set_controls_to_highlight(_("HTML Unknown Attribute"), temp_preferences.html_attribute_unknown_font, temp_preferences.html_attribute_unknown_size, 
			temp_preferences.html_attribute_unknown_bold, temp_preferences.html_attribute_unknown_italic,
			temp_preferences.html_attribute_unknown_fore, temp_preferences.html_attribute_unknown_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("HTML Number"))) {
		set_controls_to_highlight(_("HTML Number"), temp_preferences.html_number_font, temp_preferences.html_number_size, 
			temp_preferences.html_number_bold, temp_preferences.html_number_italic,
			temp_preferences.html_number_fore, temp_preferences.html_number_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("HTML Single-quoted String"))) {
		set_controls_to_highlight(_("HTML Single-quoted String"), temp_preferences.html_single_string_font, temp_preferences.html_single_string_size, 
			temp_preferences.html_single_string_bold, temp_preferences.html_single_string_italic,
			temp_preferences.html_single_string_fore, temp_preferences.html_single_string_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("HTML Double-quoted String"))) {
		set_controls_to_highlight(_("HTML Double-quoted String"), temp_preferences.html_double_string_font, temp_preferences.html_double_string_size, 
			temp_preferences.html_double_string_bold, temp_preferences.html_double_string_italic,
			temp_preferences.html_double_string_fore, temp_preferences.html_double_string_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("HTML Comment"))) {
		set_controls_to_highlight(_("HTML Comment"), temp_preferences.html_comment_font, temp_preferences.html_comment_size, 
			temp_preferences.html_comment_bold, temp_preferences.html_comment_italic,
			temp_preferences.html_comment_fore, temp_preferences.html_comment_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("HTML Entity"))) {
		set_controls_to_highlight(_("HTML Entity"), temp_preferences.html_entity_font, temp_preferences.html_entity_size, 
			temp_preferences.html_entity_bold , temp_preferences.html_entity_italic,
			temp_preferences.html_entity_fore, temp_preferences.html_entity_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("HTML Script"))) {
		set_controls_to_highlight(_("HTML Script"), temp_preferences.html_script_font, temp_preferences.html_script_size, 
			temp_preferences.html_script_bold, temp_preferences.html_script_italic,
			temp_preferences.html_script_fore, temp_preferences.html_script_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("HTML Question"))) {
		set_controls_to_highlight(_("HTML Question"), temp_preferences.html_question_font, temp_preferences.html_question_size, 
			temp_preferences.html_question_bold, temp_preferences.html_question_italic,
			temp_preferences.html_question_fore, temp_preferences.html_question_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("HTML Value"))) {
		set_controls_to_highlight(_("HTML Value"), temp_preferences.html_value_font, temp_preferences.html_value_size, 
			temp_preferences.html_value_bold, temp_preferences.html_value_italic,
			temp_preferences.html_value_fore, temp_preferences.html_value_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("JavaScript Multiple-line Comment"))) {
		set_controls_to_highlight(_("JavaScript Multiple-line Comment"), temp_preferences.javascript_comment_font, temp_preferences.javascript_comment_size, 
			temp_preferences.javascript_comment_bold, temp_preferences.javascript_comment_italic,
			temp_preferences.javascript_comment_fore, temp_preferences.javascript_comment_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("JavaScript Single-line Comment"))) {
		set_controls_to_highlight(_("JavaScript Single-line Comment"), temp_preferences.javascript_comment_line_font, temp_preferences.javascript_comment_line_size, 
			temp_preferences.javascript_comment_line_bold, temp_preferences.javascript_comment_line_italic,
			temp_preferences.javascript_comment_line_fore, temp_preferences.javascript_comment_line_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("JavaScript Document Comment"))) {
		set_controls_to_highlight(_("JavaScript Document Comment"), temp_preferences.javascript_comment_doc_font, temp_preferences.javascript_comment_doc_size, 
			temp_preferences.javascript_comment_doc_bold, temp_preferences.javascript_comment_doc_italic,
			temp_preferences.javascript_comment_doc_fore, temp_preferences.javascript_comment_doc_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("JavaScript Word"))) {
		set_controls_to_highlight(_("JavaScript Word"), temp_preferences.javascript_word_font, temp_preferences.javascript_word_size, 
			temp_preferences.javascript_word_bold, temp_preferences.javascript_word_italic,
			temp_preferences.javascript_word_fore, temp_preferences.javascript_word_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("JavaScript Keyword"))) {
		set_controls_to_highlight(_("JavaScript Keyword"), temp_preferences.javascript_keyword_font, temp_preferences.javascript_keyword_size, 
			temp_preferences.javascript_keyword_bold, temp_preferences.javascript_keyword_italic,
			temp_preferences.javascript_keyword_fore, temp_preferences.javascript_keyword_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("JavaScript Single-quoted String"))) {
		set_controls_to_highlight(_("JavaScript Single-quoted String"), temp_preferences.javascript_doublestring_font, temp_preferences.javascript_doublestring_size, 
			temp_preferences.javascript_doublestring_bold, temp_preferences.javascript_doublestring_italic,
			temp_preferences.javascript_doublestring_fore, temp_preferences.javascript_doublestring_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("JavaScript Double-quoted String"))) {
		set_controls_to_highlight(_("JavaScript Double-quoted String"), temp_preferences.javascript_singlestring_font, temp_preferences.javascript_singlestring_size, 
			temp_preferences.javascript_singlestring_bold, temp_preferences.javascript_singlestring_italic,
			temp_preferences.javascript_singlestring_fore, temp_preferences.javascript_singlestring_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("JavaScript Symbol"))) {
		set_controls_to_highlight(_("JavaScript Symbol"), temp_preferences.javascript_symbols_font, temp_preferences.javascript_symbols_size, 
			temp_preferences.javascript_symbols_bold, temp_preferences.javascript_symbols_italic,
			temp_preferences.javascript_symbols_fore, temp_preferences.javascript_symbols_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("PHP Default"))) {
		set_controls_to_highlight(_("PHP Default"), temp_preferences.php_default_font, temp_preferences.php_default_size, 
			temp_preferences.php_default_bold, temp_preferences.php_default_italic,
			temp_preferences.php_default_fore, temp_preferences.php_default_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("PHP 'HString'"))) {
		set_controls_to_highlight(_("PHP 'HString'"), temp_preferences.php_hstring_font, temp_preferences.php_hstring_size, 
			temp_preferences.php_hstring_bold, temp_preferences.php_hstring_italic,
			temp_preferences.php_hstring_fore, temp_preferences.php_hstring_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("PHP Simple String"))) {
		set_controls_to_highlight(_("PHP Simple String"), temp_preferences.php_simplestring_font, temp_preferences.php_simplestring_size, 
			temp_preferences.php_simplestring_bold, temp_preferences.php_simplestring_italic,
			temp_preferences.php_simplestring_fore, temp_preferences.php_simplestring_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("PHP Word"))) {
		set_controls_to_highlight(_("PHP Word"), temp_preferences.php_word_font, temp_preferences.php_word_size, 
			temp_preferences.php_word_bold, temp_preferences.php_word_italic,
			temp_preferences.php_word_fore, temp_preferences.php_word_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("PHP Number"))) {
		set_controls_to_highlight(_("PHP Number"), temp_preferences.php_number_font, temp_preferences.php_number_size, 
			temp_preferences.php_number_bold, temp_preferences.php_number_italic,
			temp_preferences.php_number_fore, temp_preferences.php_number_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("PHP Variable"))) {
		set_controls_to_highlight(_("PHP Variable"), temp_preferences.php_variable_font, temp_preferences.php_variable_size, 
			temp_preferences.php_variable_bold, temp_preferences.php_variable_italic,
			temp_preferences.php_variable_fore, temp_preferences.php_variable_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("PHP Single-line Comment"))) {
		set_controls_to_highlight(_("PHP Single-line Comment"), temp_preferences.php_comment_line_font, temp_preferences.php_comment_line_size, 
			temp_preferences.php_comment_line_bold, temp_preferences.php_comment_line_italic,
			temp_preferences.php_comment_line_fore, temp_preferences.php_comment_line_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("PHP Multiple-line Comment"))) {
		set_controls_to_highlight(_("PHP Multiple-line Comment"), temp_preferences.php_comment_font, temp_preferences.php_comment_size, 
			temp_preferences.php_comment_bold, temp_preferences.php_comment_italic,
			temp_preferences.php_comment_fore, temp_preferences.php_comment_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("CSS Tag"))) {
		set_controls_to_highlight(_("CSS Tag"), temp_preferences.css_tag_font, temp_preferences.css_tag_size, 
			temp_preferences.css_tag_bold, temp_preferences.css_tag_italic,
			temp_preferences.css_tag_fore, temp_preferences.css_tag_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("CSS Class"))) {
		set_controls_to_highlight(_("CSS Class"), temp_preferences.css_class_font, temp_preferences.css_class_size, 
			temp_preferences.css_class_bold, temp_preferences.css_class_italic,
			temp_preferences.css_class_fore, temp_preferences.css_class_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("CSS Psuedoclass"))) {
		set_controls_to_highlight(_("CSS Psuedoclass"), temp_preferences.css_pseudoclass_font, temp_preferences.css_pseudoclass_size, 
			temp_preferences.css_pseudoclass_bold, temp_preferences.css_pseudoclass_italic,
			temp_preferences.css_pseudoclass_fore, temp_preferences.css_pseudoclass_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("CSS Unknown Pseudoclass"))) {
		set_controls_to_highlight(_("CSS Unknown Pseudoclass"), temp_preferences.css_unknown_pseudoclass_font, temp_preferences.css_unknown_pseudoclass_size, 
			temp_preferences.css_unknown_pseudoclass_bold, temp_preferences.css_unknown_pseudoclass_italic,
			temp_preferences.css_unknown_pseudoclass_fore, temp_preferences.css_unknown_pseudoclass_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("CSS Operator"))) {
		set_controls_to_highlight(_("CSS Operator"), temp_preferences.css_operator_font, temp_preferences.css_operator_size, 
			temp_preferences.css_operator_bold, temp_preferences.css_operator_italic,
			temp_preferences.css_operator_fore, temp_preferences.css_operator_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("CSS Identifier"))) {
		set_controls_to_highlight(_("CSS Identifier"), temp_preferences.css_identifier_font, temp_preferences.css_identifier_size, 
			temp_preferences.css_identifier_bold, temp_preferences.css_identifier_italic,
			temp_preferences.css_identifier_fore, temp_preferences.css_identifier_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("CSS Unknown Identifier"))) {
		set_controls_to_highlight(_("CSS Unknown Identifier"), temp_preferences.css_unknown_identifier_font, temp_preferences.css_unknown_identifier_size, 
			temp_preferences.css_unknown_identifier_bold, temp_preferences.css_unknown_identifier_italic,
			temp_preferences.css_unknown_identifier_fore, temp_preferences.css_unknown_identifier_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("CSS Value"))) {
		set_controls_to_highlight(_("CSS Value"), temp_preferences.css_value_font, temp_preferences.css_value_size, 
			temp_preferences.css_value_bold, temp_preferences.css_value_italic,
			temp_preferences.css_value_fore, temp_preferences.css_value_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("CSS Comment"))) {
		set_controls_to_highlight(_("CSS Comment"), temp_preferences.css_comment_font, temp_preferences.css_comment_size, 
			temp_preferences.css_comment_bold, temp_preferences.css_comment_italic,
			temp_preferences.css_comment_fore, temp_preferences.css_comment_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("CSS ID"))) {
		set_controls_to_highlight(_("CSS ID"), temp_preferences.css_id_font, temp_preferences.css_id_size, 
			temp_preferences.css_id_bold, temp_preferences.css_id_italic,
			temp_preferences.css_id_fore, temp_preferences.css_id_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("CSS Important"))) {
		set_controls_to_highlight(_("CSS Important"), temp_preferences.css_important_font, temp_preferences.css_important_size, 
			temp_preferences.css_important_bold, temp_preferences.css_important_italic,
			temp_preferences.css_important_fore, temp_preferences.css_important_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("CSS Directive"))) {
		set_controls_to_highlight(_("CSS Directive"), temp_preferences.css_directive_font, temp_preferences.css_directive_size, 
			temp_preferences.css_directive_bold, temp_preferences.css_directive_italic,
			temp_preferences.css_directive_fore, temp_preferences.css_directive_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("SQL Word"))) {
		set_controls_to_highlight(_("SQL Word"), temp_preferences.sql_word_font, temp_preferences.sql_word_size, 
			temp_preferences.sql_word_bold, temp_preferences.sql_word_italic,
			temp_preferences.sql_word_fore, temp_preferences.sql_word_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("SQL Identifier"))) {
		set_controls_to_highlight(_("SQL Identifier"), temp_preferences.sql_identifier_font, temp_preferences.sql_identifier_size, 
			temp_preferences.sql_identifier_bold, temp_preferences.sql_identifier_italic,
			temp_preferences.sql_identifier_fore, temp_preferences.sql_identifier_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("SQL Number"))) {
		set_controls_to_highlight(_("SQL Number"), temp_preferences.sql_number_font, temp_preferences.sql_number_size, 
			temp_preferences.sql_number_bold, temp_preferences.sql_number_italic,
			temp_preferences.sql_number_fore, temp_preferences.sql_number_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("SQL String"))) {
		set_controls_to_highlight(_("SQL String"), temp_preferences.sql_string_font, temp_preferences.sql_string_size, 
			temp_preferences.sql_string_bold, temp_preferences.sql_string_italic,
			temp_preferences.sql_string_fore, temp_preferences.sql_string_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("SQL Operator"))) {
		set_controls_to_highlight(_("SQL Operator"), temp_preferences.sql_operator_font, temp_preferences.sql_operator_size, 
			temp_preferences.sql_operator_bold, temp_preferences.sql_operator_italic,
			temp_preferences.sql_operator_fore, temp_preferences.sql_operator_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("SQL Comment"))) {
		set_controls_to_highlight(_("SQL Comment"), temp_preferences.sql_comment_font, temp_preferences.sql_comment_size, 
			temp_preferences.sql_comment_bold, temp_preferences.sql_comment_italic,
			temp_preferences.sql_comment_fore, temp_preferences.sql_comment_back);
	}
}


void on_element_entry_changed(GtkEntry *entry, gpointer data)
{
	current_highlighting_element =(gchar *)gtk_entry_get_text(GTK_ENTRY(entry));
	get_current_highlighting_settings(current_highlighting_element);
}



void apply_preferences(GtkButton *button, gpointer data)
{
	GSList *walk;
	Editor *editor;

	set_current_preferences();
	for(walk = editors; walk!=NULL; walk = g_slist_next(walk)) {
		editor = walk->data;
		tab_set_configured_scintilla_properties(GTK_SCINTILLA(editor->scintilla), preferences);
		tab_check_php_file(editor);
		tab_check_css_file(editor);
		tab_check_cxx_file(editor);
		tab_check_perl_file(editor);
		tab_check_python_file(editor);
		tab_check_sql_file(editor);
	}
}


void ok_clicked(GtkButton *button, gpointer data)
{
	apply_preferences(button, data);
	//g_print("OK\n");
	gtk_widget_destroy(preferences_dialog.window);
}


void change_font_global_callback(gint reply,gpointer data)
{
	gchar *fontname;

	//g_print("change_font_global: %d, YES=%d, NO=%d\n", reply, GTK_RESPONSE_YES, GTK_RESPONSE_NO);
	if(reply==GTK_RESPONSE_YES) {
		fontname = g_strdup((gchar *)data); 
		
		temp_preferences.default_font = fontname;
		temp_preferences.line_number_font = fontname;
		temp_preferences.html_tag_font = fontname;
		temp_preferences.html_tag_unknown_font = fontname;
		temp_preferences.html_attribute_font = fontname;
		temp_preferences.html_attribute_unknown_font = fontname;
		temp_preferences.html_number_font = fontname;
		temp_preferences.html_single_string_font = fontname;
		temp_preferences.html_double_string_font = fontname;
		temp_preferences.html_comment_font = fontname;
		temp_preferences.html_entity_font = fontname;
		temp_preferences.html_script_font = fontname;
		temp_preferences.html_question_font = fontname;
		temp_preferences.html_value_font = fontname;
		temp_preferences.javascript_comment_font = fontname;
		temp_preferences.javascript_comment_line_font = fontname;
		temp_preferences.javascript_comment_doc_font = fontname;
		temp_preferences.javascript_word_font = fontname;
		temp_preferences.javascript_keyword_font = fontname;
		temp_preferences.javascript_doublestring_font = fontname;
		temp_preferences.javascript_singlestring_font = fontname;
		temp_preferences.javascript_symbols_font = fontname;
		temp_preferences.php_default_font = fontname;
		temp_preferences.php_hstring_font = fontname;
		temp_preferences.php_simplestring_font = fontname;
		temp_preferences.php_word_font = fontname;
		temp_preferences.php_number_font = fontname;
		temp_preferences.php_variable_font = fontname;
		temp_preferences.php_comment_font = fontname;
		temp_preferences.php_comment_line_font = fontname;
		temp_preferences.css_tag_font = fontname;
		temp_preferences.css_class_font = fontname;
		temp_preferences.css_pseudoclass_font = fontname;
		temp_preferences.css_unknown_pseudoclass_font = fontname;
		temp_preferences.css_operator_font = fontname;
		temp_preferences.css_identifier_font = fontname;
		temp_preferences.css_unknown_identifier_font = fontname;
		temp_preferences.css_value_font = fontname;
		temp_preferences.css_comment_font = fontname;
		temp_preferences.css_id_font = fontname;
		temp_preferences.css_important_font = fontname;
		temp_preferences.css_directive_font = fontname;
		temp_preferences.sql_word_font = fontname;
		temp_preferences.sql_string_font = fontname;
		temp_preferences.sql_operator_font = fontname;
		temp_preferences.sql_comment_font = fontname;
		temp_preferences.sql_number_font = fontname;
		temp_preferences.sql_identifier_font = fontname;
	}
	
	scintilla_php_set_lexer(GTK_SCINTILLA(preferences_dialog.highlighting_editor->scintilla), temp_preferences);
}


void change_size_global_callback(gint reply,gpointer data)
{
	gint fontsize;

	//g_print("change_size_global: %d\n", reply);
	if(reply==GTK_RESPONSE_YES) {
		fontsize =(gulong)data; // was(gint)
		
		temp_preferences.default_size = fontsize;
		temp_preferences.line_number_size = fontsize;
		temp_preferences.html_tag_size = fontsize;
		temp_preferences.html_tag_unknown_size = fontsize;
		temp_preferences.html_attribute_size = fontsize;
		temp_preferences.html_attribute_unknown_size = fontsize;
		temp_preferences.html_number_size = fontsize;
		temp_preferences.html_single_string_size = fontsize;
		temp_preferences.html_double_string_size = fontsize;
		temp_preferences.html_comment_size = fontsize;
		temp_preferences.html_entity_size = fontsize;
		temp_preferences.html_script_size = fontsize;
		temp_preferences.html_question_size = fontsize;
		temp_preferences.html_value_size = fontsize;
		temp_preferences.javascript_comment_size = fontsize;
		temp_preferences.javascript_comment_line_size = fontsize;
		temp_preferences.javascript_comment_doc_size = fontsize;
		temp_preferences.javascript_word_size = fontsize;
		temp_preferences.javascript_keyword_size = fontsize;
		temp_preferences.javascript_doublestring_size = fontsize;
		temp_preferences.javascript_singlestring_size = fontsize;
		temp_preferences.javascript_symbols_size = fontsize;
		temp_preferences.php_default_size = fontsize;
		temp_preferences.php_hstring_size = fontsize;
		temp_preferences.php_simplestring_size = fontsize;
		temp_preferences.php_word_size = fontsize;
		temp_preferences.php_number_size = fontsize;
		temp_preferences.php_variable_size = fontsize;
		temp_preferences.php_comment_size = fontsize;
		temp_preferences.php_comment_line_size = fontsize;
		temp_preferences.css_tag_size = fontsize;
		temp_preferences.css_class_size = fontsize;
		temp_preferences.css_pseudoclass_size = fontsize;
		temp_preferences.css_unknown_pseudoclass_size = fontsize;
		temp_preferences.css_operator_size = fontsize;
		temp_preferences.css_identifier_size = fontsize;
		temp_preferences.css_unknown_identifier_size = fontsize;
		temp_preferences.css_value_size = fontsize;
		temp_preferences.css_comment_size = fontsize;
		temp_preferences.css_id_size = fontsize;
		temp_preferences.css_important_size = fontsize;
		temp_preferences.css_directive_size = fontsize;
		temp_preferences.sql_word_size = fontsize;
		temp_preferences.sql_string_size = fontsize;
		temp_preferences.sql_operator_size = fontsize;
		temp_preferences.sql_comment_size = fontsize;
		temp_preferences.sql_number_size = fontsize;
		temp_preferences.sql_identifier_size = fontsize;
	}
	scintilla_php_set_lexer(GTK_SCINTILLA(preferences_dialog.highlighting_editor->scintilla), temp_preferences);
}


void get_control_values_to_highlight(gchar *setting_name, gchar **fontname, gint *fontsize, gboolean *bold, gboolean *italic, gint *fore, gint *back)
{
	GString *tempfontname;
	guint8 red, blue, green, alpha;
	GtkWidget *dialog;
	GString *message;
	gint newfontsize; // Back to being a gint, g_string_printf complains
	gint result;

	if(preferences_dialog.changing_highlight_element) {
		return;
	}

	tempfontname = g_string_new(gtk_combo_box_get_active_text(GTK_COMBO_BOX(preferences_dialog.font_combo)));
	tempfontname = g_string_prepend(tempfontname, "!");
	
	if(g_strncasecmp(*fontname, tempfontname->str, MIN(strlen(*fontname), strlen((tempfontname->str))))!=0) {
		message = g_string_new(NULL);
		g_string_printf(message, _("You have just changed the font to %s\n\nWould you like to use this font as the default for every element?"), gtk_combo_box_get_active_text(GTK_COMBO_BOX(preferences_dialog.font_combo)));
		//gtk_combo_box_popdown(GTK_COMBO_BOX(preferences_dialog.font_combo));
		dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION,
				  GTK_BUTTONS_YES_NO, message->str);
		result = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		change_font_global_callback(result, tempfontname->str);		
		g_string_free(message, TRUE);
		*fontname = tempfontname->str;
	}
	g_string_free(tempfontname, FALSE);
	
	newfontsize = atoi(gtk_combo_box_get_active_text(GTK_COMBO_BOX(preferences_dialog.size_combo)));
	if(*fontsize != newfontsize &&
		newfontsize != 0) {
		message = g_string_new(NULL);
		g_string_printf(message, _("You have just changed the font size to %dpt\n\nWould you like to use this font size as the default for every element?"), newfontsize);
		//gtk_combo_box_popdown(GTK_COMBO_BOX(preferences_dialog.size_combo));
		dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION,
				  GTK_BUTTONS_YES_NO, message->str);
		result = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		change_size_global_callback(result,(gpointer)newfontsize);		
		g_string_free(message, TRUE);
		*fontsize = atoi(gtk_combo_box_get_active_text(GTK_COMBO_BOX(preferences_dialog.size_combo)));
	}
	*bold = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(preferences_dialog.bold_button));
	*italic = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(preferences_dialog.italic_button));
	
	gnome_color_picker_get_i8(GNOME_COLOR_PICKER(preferences_dialog.foreground_colour), &red, &blue, &green, &alpha);
	*fore =(green<<16) |(blue<<8) | red;
	
	gnome_color_picker_get_i8(GNOME_COLOR_PICKER(preferences_dialog.background_colour), &red, &blue, &green, &alpha);
	*back =(green<<16) |(blue<<8) | red;

	// Debug print for preferences being set
	//g_print("Setting %s: %s %d %d %d %d %d\n", setting_name, *fontname, *fontsize, *bold, *italic, *fore, *back);	
}

void set_current_highlighting_font()
{
	if(IS_FONT_NAME(current_highlighting_element, _("Default"))) {
		get_control_values_to_highlight(_("Default"), &temp_preferences.default_font, &temp_preferences.default_size,
			&temp_preferences.default_bold, &temp_preferences.default_italic,
			&temp_preferences.default_fore, &temp_preferences.default_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("Line Number(margin)"))) {
		get_control_values_to_highlight(_("Line Number(margin)"), &temp_preferences.line_number_font, &temp_preferences.line_number_size,
			&temp_preferences.line_number_bold, &temp_preferences.line_number_italic,
			&temp_preferences.line_number_fore, &temp_preferences.line_number_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("HTML Tag"))) {
		get_control_values_to_highlight(_("HTML Tag"), &temp_preferences.html_tag_font, &temp_preferences.html_tag_size,
			&temp_preferences.html_tag_bold, &temp_preferences.html_tag_italic,
			&temp_preferences.html_tag_fore, &temp_preferences.html_tag_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("HTML Unknown Tag"))) {
		get_control_values_to_highlight(_("HTML Unknown Tag"), &temp_preferences.html_tag_unknown_font, &temp_preferences.html_tag_unknown_size,
			&temp_preferences.html_tag_unknown_bold, &temp_preferences.html_tag_unknown_italic,
			&temp_preferences.html_tag_unknown_fore, &temp_preferences.html_tag_unknown_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("HTML Attribute"))) {
		get_control_values_to_highlight(_("HTML Attribute"), &temp_preferences.html_attribute_font, &temp_preferences.html_attribute_size,
			&temp_preferences.html_attribute_bold, &temp_preferences.html_attribute_italic,
			&temp_preferences.html_attribute_fore, &temp_preferences.html_attribute_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("HTML Unknown Attribute"))) {
		get_control_values_to_highlight(_("HTML Unknown Attribute"), &temp_preferences.html_attribute_unknown_font, &temp_preferences.html_attribute_unknown_size,
			&temp_preferences.html_attribute_unknown_bold, &temp_preferences.html_attribute_unknown_italic,
			&temp_preferences.html_attribute_unknown_fore, &temp_preferences.html_attribute_unknown_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("HTML Number"))) {
		get_control_values_to_highlight(_("HTML Number"), &temp_preferences.html_number_font, &temp_preferences.html_number_size,
			&temp_preferences.html_number_bold, &temp_preferences.html_number_italic,
			&temp_preferences.html_number_fore, &temp_preferences.html_number_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("HTML Single-quoted String"))) {
		get_control_values_to_highlight(_("HTML Single-quoted String"), &temp_preferences.html_single_string_font, &temp_preferences.html_single_string_size,
			&temp_preferences.html_single_string_bold, &temp_preferences.html_single_string_italic,
			&temp_preferences.html_single_string_fore, &temp_preferences.html_single_string_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("HTML Double-quoted String"))) {
		get_control_values_to_highlight(_("HTML Double-quoted String"), &temp_preferences.html_double_string_font, &temp_preferences.html_double_string_size,
			&temp_preferences.html_double_string_italic, &temp_preferences.html_double_string_italic,
			&temp_preferences.html_double_string_fore, &temp_preferences.html_double_string_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("HTML Comment"))) {
		get_control_values_to_highlight(_("HTML Comment"), &temp_preferences.html_comment_font, &temp_preferences.html_comment_size,
			&temp_preferences.html_comment_bold, &temp_preferences.html_comment_italic,
			&temp_preferences.html_comment_fore, &temp_preferences.html_comment_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("HTML Entity"))) {
		get_control_values_to_highlight(_("HTML Entity"), &temp_preferences.html_entity_font, &temp_preferences.html_entity_size,
			&temp_preferences.html_entity_bold, &temp_preferences.html_entity_italic,
			&temp_preferences.html_entity_fore, &temp_preferences.html_entity_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("HTML Script"))) {
		get_control_values_to_highlight(_("HTML Script"), &temp_preferences.html_script_font, &temp_preferences.html_script_size,
			&temp_preferences.html_script_bold, &temp_preferences.html_script_italic,
			&temp_preferences.html_script_fore, &temp_preferences.html_script_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("HTML Question"))) {
		get_control_values_to_highlight(_("HTML Question"), &temp_preferences.html_question_font, &temp_preferences.html_question_size,
			&temp_preferences.html_question_bold, &temp_preferences.html_question_italic,
			&temp_preferences.html_question_fore, &temp_preferences.html_question_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("HTML Value"))) {
		get_control_values_to_highlight(_("HTML Value"), &temp_preferences.html_value_font, &temp_preferences.html_value_size,
			&temp_preferences.html_value_bold, &temp_preferences.html_value_italic,
			&temp_preferences.html_value_fore, &temp_preferences.html_value_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("JavaScript Multiple-line Comment"))) {
		get_control_values_to_highlight(_("JavaScript Multiple-line Comment"), &temp_preferences.javascript_comment_font, &temp_preferences.javascript_comment_size,
			&temp_preferences.javascript_comment_bold, &temp_preferences.javascript_comment_italic,
			&temp_preferences.javascript_comment_fore, &temp_preferences.javascript_comment_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("JavaScript Single-line Comment"))) {
		get_control_values_to_highlight(_("JavaScript Single-line Comment"), &temp_preferences.javascript_comment_line_font, &temp_preferences.javascript_comment_line_size,
			&temp_preferences.javascript_comment_line_bold, &temp_preferences.javascript_comment_line_italic,
			&temp_preferences.javascript_comment_line_fore, &temp_preferences.javascript_comment_line_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("JavaScript Document Comment"))) {
		get_control_values_to_highlight(_("JavaScript Document Comment"), &temp_preferences.javascript_comment_doc_font, &temp_preferences.javascript_comment_doc_size,
			&temp_preferences.javascript_comment_doc_bold, &temp_preferences.javascript_comment_doc_italic,
			&temp_preferences.javascript_comment_doc_fore, &temp_preferences.javascript_comment_doc_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("JavaScript Word"))) {
		get_control_values_to_highlight(_("JavaScript Word"), &temp_preferences.javascript_word_font, &temp_preferences.javascript_word_size,
			&temp_preferences.javascript_word_bold, &temp_preferences.javascript_word_italic,
			&temp_preferences.javascript_word_fore, &temp_preferences.javascript_word_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("JavaScript Keyword"))) {
		get_control_values_to_highlight(_("JavaScript Keyword"), &temp_preferences.javascript_keyword_font, &temp_preferences.javascript_keyword_size,
			&temp_preferences.javascript_keyword_bold, &temp_preferences.javascript_keyword_italic,
			&temp_preferences.javascript_keyword_fore, &temp_preferences.javascript_keyword_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("JavaScript Single-quoted String"))) {
		get_control_values_to_highlight(_("JavaScript Single-quoted String"), &temp_preferences.javascript_doublestring_font, &temp_preferences.javascript_doublestring_size,
			&temp_preferences.javascript_doublestring_bold, &temp_preferences.javascript_doublestring_italic,
			&temp_preferences.javascript_doublestring_fore, &temp_preferences.javascript_doublestring_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("JavaScript Double-quoted String"))) {
		get_control_values_to_highlight(_("JavaScript Double-quoted String"), &temp_preferences.javascript_singlestring_font, &temp_preferences.javascript_singlestring_size,
			&temp_preferences.javascript_singlestring_bold, &temp_preferences.javascript_singlestring_italic,
			&temp_preferences.javascript_singlestring_fore, &temp_preferences.javascript_singlestring_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("JavaScript Symbol"))) {
		get_control_values_to_highlight(_("JavaScript Symbol"), &temp_preferences.javascript_symbols_font, &temp_preferences.javascript_symbols_size,
			&temp_preferences.javascript_symbols_bold, &temp_preferences.default_italic,
			&temp_preferences.javascript_symbols_fore, &temp_preferences.javascript_symbols_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("PHP Default"))) {
		get_control_values_to_highlight(_("PHP Default"), &temp_preferences.php_default_font, &temp_preferences.php_default_size,
			&temp_preferences.php_default_bold, &temp_preferences.php_default_italic,
			&temp_preferences.php_default_fore, &temp_preferences.php_default_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("PHP 'HString'"))) {
		get_control_values_to_highlight(_("PHP 'HString'"), &temp_preferences.php_hstring_font, &temp_preferences.php_hstring_size,
			&temp_preferences.php_hstring_bold, &temp_preferences.php_hstring_italic,
			&temp_preferences.php_hstring_fore, &temp_preferences.php_hstring_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("PHP Simple String"))) {
		get_control_values_to_highlight(_("PHP Simple String"), &temp_preferences.php_simplestring_font, &temp_preferences.php_simplestring_size,
			&temp_preferences.php_simplestring_bold, &temp_preferences.php_simplestring_italic,
			&temp_preferences.php_simplestring_fore, &temp_preferences.php_simplestring_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("PHP Word"))) {
		get_control_values_to_highlight(_("PHP Word"), &temp_preferences.php_word_font, &temp_preferences.php_word_size,
			&temp_preferences.php_word_bold, &temp_preferences.php_word_italic,
			&temp_preferences.php_word_fore, &temp_preferences.php_word_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("PHP Number"))) {
		get_control_values_to_highlight(_("PHP Number"), &temp_preferences.php_number_font, &temp_preferences.php_number_size,
			&temp_preferences.php_number_bold, &temp_preferences.php_number_italic,
			&temp_preferences.php_number_fore, &temp_preferences.php_number_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("PHP Variable"))) {
		get_control_values_to_highlight(_("PHP Variable"), &temp_preferences.php_variable_font, &temp_preferences.php_variable_size,
			&temp_preferences.php_variable_bold, &temp_preferences.php_variable_italic,
			&temp_preferences.php_variable_fore, &temp_preferences.php_variable_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("PHP Single-line Comment"))) {
		get_control_values_to_highlight(_("PHP Single-line Comment"), &temp_preferences.php_comment_line_font, &temp_preferences.php_comment_line_size,
			&temp_preferences.php_comment_line_bold, &temp_preferences.php_comment_line_italic,
			&temp_preferences.php_comment_line_fore, &temp_preferences.php_comment_line_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("PHP Multiple-line Comment"))) {
		get_control_values_to_highlight(_("PHP Multiple-line Comment"), &temp_preferences.php_comment_font, &temp_preferences.php_comment_size,
			&temp_preferences.php_comment_bold, &temp_preferences.php_comment_italic,
			&temp_preferences.php_comment_fore, &temp_preferences.php_comment_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("CSS Tag"))) {
		get_control_values_to_highlight(_("CSS Tag"), &temp_preferences.css_tag_font, &temp_preferences.css_tag_size,
			&temp_preferences.css_tag_bold, &temp_preferences.css_tag_italic,
			&temp_preferences.css_tag_fore, &temp_preferences.css_tag_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("CSS Class"))) {
		get_control_values_to_highlight(_("CSS Class"), &temp_preferences.css_class_font, &temp_preferences.css_class_size,
			&temp_preferences.css_class_bold, &temp_preferences.css_class_italic,
			&temp_preferences.css_class_fore, &temp_preferences.css_class_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("CSS Psuedoclass"))) {
		get_control_values_to_highlight(_("CSS Psuedoclass"), &temp_preferences.css_pseudoclass_font, &temp_preferences.css_pseudoclass_size,
			&temp_preferences.css_pseudoclass_bold, &temp_preferences.css_pseudoclass_italic,
			&temp_preferences.css_pseudoclass_fore, &temp_preferences.css_pseudoclass_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("CSS Unknown Pseudoclass"))) {
		get_control_values_to_highlight(_("CSS Unknown Pseudoclass"), &temp_preferences.css_unknown_pseudoclass_font, &temp_preferences.css_unknown_pseudoclass_size,
			&temp_preferences.css_unknown_pseudoclass_bold, &temp_preferences.css_unknown_pseudoclass_italic,
			&temp_preferences.css_unknown_pseudoclass_fore, &temp_preferences.css_unknown_pseudoclass_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("CSS Operator"))) {
		get_control_values_to_highlight(_("CSS Operator"), &temp_preferences.css_operator_font, &temp_preferences.css_operator_size,
			&temp_preferences.css_operator_bold, &temp_preferences.css_operator_italic,
			&temp_preferences.css_operator_fore, &temp_preferences.css_operator_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("CSS Identifier"))) {
		get_control_values_to_highlight(_("CSS Identifier"), &temp_preferences.css_identifier_font, &temp_preferences.css_identifier_size,
			&temp_preferences.css_identifier_bold, &temp_preferences.css_identifier_italic,
			&temp_preferences.css_identifier_fore, &temp_preferences.css_identifier_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("CSS Unknown Identifier"))) {
		get_control_values_to_highlight(_("CSS Unknown Identifier"), &temp_preferences.css_unknown_identifier_font, &temp_preferences.css_unknown_identifier_size,
			&temp_preferences.css_unknown_identifier_bold, &temp_preferences.css_unknown_identifier_italic,
			&temp_preferences.css_unknown_identifier_fore, &temp_preferences.css_unknown_identifier_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("CSS Value"))) {
		get_control_values_to_highlight(_("CSS Value"), &temp_preferences.css_value_font, &temp_preferences.css_value_size,
			&temp_preferences.css_value_bold, &temp_preferences.css_value_italic,
			&temp_preferences.css_value_fore, &temp_preferences.css_value_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("CSS Comment"))) {
		get_control_values_to_highlight(_("CSS Comment"), &temp_preferences.css_comment_font, &temp_preferences.css_comment_size,
			&temp_preferences.css_comment_bold, &temp_preferences.css_comment_italic,
			&temp_preferences.css_comment_fore, &temp_preferences.css_comment_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("CSS ID"))) {
		get_control_values_to_highlight(_("CSS ID"), &temp_preferences.css_id_font, &temp_preferences.css_id_size,
			&temp_preferences.css_id_bold, &temp_preferences.css_id_italic,
			&temp_preferences.css_id_fore, &temp_preferences.css_id_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("CSS Important"))) {
		get_control_values_to_highlight(_("CSS Important"), &temp_preferences.css_important_font, &temp_preferences.css_important_size,
			&temp_preferences.css_important_bold, &temp_preferences.css_important_italic,
			&temp_preferences.css_important_fore, &temp_preferences.css_important_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("CSS Directive"))) {
		get_control_values_to_highlight(_("CSS Directive"), &temp_preferences.css_directive_font, &temp_preferences.css_directive_size,
			&temp_preferences.css_directive_bold, &temp_preferences.css_directive_italic,
			&temp_preferences.css_directive_fore, &temp_preferences.css_directive_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("SQL Word"))) {
		get_control_values_to_highlight(_("SQL Word"), &temp_preferences.sql_word_font, &temp_preferences.sql_word_size,
			&temp_preferences.sql_word_bold, &temp_preferences.sql_word_italic,
			&temp_preferences.sql_word_fore, &temp_preferences.sql_word_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("SQL Identifier"))) {
		get_control_values_to_highlight(_("SQL Identifier"), &temp_preferences.sql_identifier_font, &temp_preferences.sql_identifier_size,
			&temp_preferences.sql_identifier_bold, &temp_preferences.sql_identifier_italic,
			&temp_preferences.sql_identifier_fore, &temp_preferences.sql_identifier_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("SQL Number"))) {
		get_control_values_to_highlight(_("SQL Number"), &temp_preferences.sql_number_font, &temp_preferences.sql_number_size,
			&temp_preferences.sql_number_bold, &temp_preferences.sql_number_italic,
			&temp_preferences.sql_number_fore, &temp_preferences.sql_number_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("SQL String"))) {
		get_control_values_to_highlight(_("SQL String"), &temp_preferences.sql_string_font, &temp_preferences.sql_string_size,
			&temp_preferences.sql_string_bold, &temp_preferences.sql_string_italic,
			&temp_preferences.sql_string_fore, &temp_preferences.sql_string_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("SQL Operator"))) {
		get_control_values_to_highlight(_("SQL Operator"), &temp_preferences.sql_operator_font, &temp_preferences.sql_operator_size,
			&temp_preferences.sql_operator_bold, &temp_preferences.sql_operator_italic,
			&temp_preferences.sql_operator_fore, &temp_preferences.sql_operator_back);
	}
	if(IS_FONT_NAME(current_highlighting_element, _("SQL Comment"))) {
		get_control_values_to_highlight(_("SQL Comment"), &temp_preferences.sql_comment_font, &temp_preferences.sql_comment_size,
			&temp_preferences.sql_comment_bold, &temp_preferences.sql_comment_italic,
			&temp_preferences.sql_comment_fore, &temp_preferences.sql_comment_back);
	}
	scintilla_php_set_lexer(GTK_SCINTILLA(preferences_dialog.highlighting_editor->scintilla), temp_preferences);
}

void on_bold_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
	set_current_highlighting_font();
}


void on_italic_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
	set_current_highlighting_font();
}


void on_fontname_entry_changed(GtkEntry *Entry, gpointer data)
{
	set_current_highlighting_font();
}


void on_fontsize_entry_changed(GtkEntry *Entry, gpointer data)
{
	set_current_highlighting_font();
}


void preferences_window_destroyed(GtkWidget *widget, gpointer data)
{
	gtk_grab_remove(preferences_dialog.window);
	preferences_dialog.window = NULL;
}


void on_fore_changed(GnomeColorPicker *cp, guint red, guint green, guint blue, guint alpha, gpointer userdate)
{
	set_current_highlighting_font();
}


void on_back_changed(GnomeColorPicker *cp, guint red, guint green, guint blue, guint alpha, gpointer userdate)
{
	set_current_highlighting_font();
}


void on_edge_colour_changed(GnomeColorPicker *cp, guint red, guint green, guint blue, guint alpha, gpointer userdate)
{
	guint8 ired, iblue, igreen, ialpha;
	
	gnome_color_picker_get_i8(GNOME_COLOR_PICKER(preferences_dialog.edge_colour), &ired, &iblue, &igreen, &ialpha);
	temp_preferences.edge_colour =(igreen<<16) |(iblue<<8) | ired;

}


void on_tab_size_changed(GtkRange *range, gpointer user_data)
{
	temp_preferences.tab_size =(int)(gtk_range_get_adjustment(range)->value);	
	temp_preferences.indentation_size = temp_preferences.tab_size;	
}


void on_calltip_delay_changed(GtkRange *range, gpointer user_data)
{
	temp_preferences.calltip_delay =(int)(gtk_range_get_adjustment(range)->value);	
	temp_preferences.auto_complete_delay =(int)(gtk_range_get_adjustment(range)->value);	
}


void on_edge_column_changed(GtkRange *range, gpointer user_data)
{
	temp_preferences.edge_column =(int)(gtk_range_get_adjustment(range)->value);	
}


void on_show_indentation_guides_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
	temp_preferences.show_indentation_guides = gtk_toggle_button_get_active(togglebutton);	
}


void on_edge_mode_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
	temp_preferences.edge_mode = gtk_toggle_button_get_active(togglebutton);	
}


void on_line_wrapping_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
	temp_preferences.line_wrapping = gtk_toggle_button_get_active(togglebutton);	
}

void on_use_tabs_instead_spaces_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
	temp_preferences.use_tabs_instead_spaces = gtk_toggle_button_get_active(togglebutton);	
}


void on_save_session_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
	temp_preferences.save_session = gtk_toggle_button_get_active(togglebutton);
}

void on_single_instance_only_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
	temp_preferences.single_instance_only = gtk_toggle_button_get_active(togglebutton);
}


/* Version for gnome_file_entry
void on_php_binary_location_changed(GtkEditable *editable, gpointer user_data)
{
	temp_preferences.php_binary_location = g_strdup(gtk_entry_get_text(GTK_ENTRY(editable)));
}*/

/* Version for GtkEntry */
void on_php_binary_location_changed(GtkEntry *entry, gpointer user_data)
{
	temp_preferences.php_binary_location = g_strdup(gtk_entry_get_text(entry));
}


void on_php_file_extensions_changed(GtkEntry *entry, gpointer user_data)
{
	temp_preferences.php_file_extensions = g_strdup(gtk_entry_get_text(entry));
}

void on_shared_source_changed(GtkEntry *entry, gpointer user_data)
{
	temp_preferences.shared_source_location = g_strdup(gtk_entry_get_text(entry));
}


void update_template_display(gchar *template)
{
  GtkTextBuffer *buffer;

  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(preferences_dialog.template_sample));

  gtk_text_buffer_set_text(buffer, template, -1);	
}


static void add_to_template_list(gpointer key, gpointer value, gpointer user_data)
{
	GtkTreeIter iter;

	gtk_list_store_append(preferences_dialog.template_store, &iter);
	gtk_list_store_set(preferences_dialog.template_store, &iter,0,(gchar *)key, -1);
}


void add_templates_to_store(void)
{
	g_hash_table_foreach(templates, add_to_template_list, NULL);
}

void add_template_clicked(GtkButton *button, gpointer data)
{
	gchar *name;
	gchar *template;
	GtkTreeIter iter;
	GtkTextBuffer *buffer;
	GtkTextIter begin, end;
	gchar *content;

	// create dialog
	create_edit_template_dialog();

	// Run and wait for OK
	if(gtk_dialog_run(GTK_DIALOG(edit_template_dialog.window1)) == GTK_RESPONSE_ACCEPT) {
		// convert content to template format
		buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(edit_template_dialog.textview1));
		gtk_text_buffer_get_start_iter(buffer, &begin);
		gtk_text_buffer_get_end_iter(buffer, &end);
		content = gtk_text_buffer_get_text(buffer, &begin, &end, TRUE);
		
		template = template_convert_to_template(content);

		// add to templates
		name =(gchar *)gtk_entry_get_text(GTK_ENTRY(edit_template_dialog.entry1));
		template_delete(current_key); // Just in case you accidentally type the name of an existing template
		template_replace(name, template);
		
		// add to treeview
		gtk_list_store_append(preferences_dialog.template_store, &iter);
		gtk_list_store_set(preferences_dialog.template_store, &iter,0,(gchar *)name, -1);
	}
	
	// destroy/null dialog
	gtk_widget_destroy(edit_template_dialog.window1);
	edit_template_dialog.window1 = NULL;
}

void edit_template_clicked(GtkButton *button, gpointer data)
{
	gchar *name;
	gchar *template;
	GtkTextBuffer *buffer;
	GtkTextIter begin, end;
	gchar *content;
	GtkTreeIter iter;

	// create dialog
	create_edit_template_dialog();

	// fill in existing content
	gtk_entry_set_text(GTK_ENTRY(edit_template_dialog.entry1), current_key);
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(preferences_dialog.template_sample));
	gtk_text_buffer_get_start_iter(buffer, &begin);
	gtk_text_buffer_get_end_iter(buffer, &end);
	content = gtk_text_buffer_get_text(buffer, &begin, &end, TRUE);
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(edit_template_dialog.textview1));
	gtk_text_buffer_set_text(buffer, content, -1);	
	
	// Run and wait for OK
	if(gtk_dialog_run(GTK_DIALOG(edit_template_dialog.window1)) == GTK_RESPONSE_ACCEPT) {
		// convert content to template format
		buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(edit_template_dialog.textview1));
		gtk_text_buffer_get_start_iter(buffer, &begin);
		gtk_text_buffer_get_end_iter(buffer, &end);
		content = gtk_text_buffer_get_text(buffer, &begin, &end, TRUE);
		
		template = template_convert_to_template(content);

		// add to templates
		name =(gchar *)gtk_entry_get_text(GTK_ENTRY(edit_template_dialog.entry1));
		template_delete(current_key);
		template_replace(name, template);
		
		// replace in treeview
		gtk_tree_selection_get_selected(preferences_dialog.template_selection, NULL, &iter);
		gtk_list_store_set(preferences_dialog.template_store, &iter,0,(gchar *)name, -1);
		
		// replace in display
		update_template_display(content);
	}
	
	// destroy/null dialog
	gtk_widget_destroy(edit_template_dialog.window1);
	edit_template_dialog.window1 = NULL;
}

void delete_template_clicked(GtkButton *button, gpointer data)
{
	GtkWidget *confirm_dialog;
	GtkTreeIter iter;

	confirm_dialog = gtk_message_dialog_new(GTK_WINDOW(main_window.window),GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
        _("Are you sure you want to delete template %s?"),current_key);
	// confirm deletion with dialog
	if(gtk_dialog_run(GTK_DIALOG(confirm_dialog)) == GTK_RESPONSE_YES) {
		// delete from templates
		template_delete(current_key);

		// delete from treeview
		gtk_tree_selection_get_selected(preferences_dialog.template_selection, NULL, &iter);
		gtk_list_store_remove(preferences_dialog.template_store, &iter);
		current_key = NULL;
		/*
		gtk_tree_selection_unselect_all(GTK_TREE_SELECTION(preferences_dialog.template_selection));
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(preferences_dialog.Templates));
		g_object_ref(model);
		gtk_tree_view_set_model(GTK_TREE_VIEW(preferences_dialog.Templates), NULL);
		//gtk_list_store_remove(preferences_dialog.template_store, &current_template_iter);
		gtk_list_store_clear(preferences_dialog.template_store);
		gtk_tree_view_set_model(GTK_TREE_VIEW(preferences_dialog.Templates), model);
		g_object_unref(model);
		
		
		add_templates_to_store();*/
		
	}
	
	// destroy dialog
	gtk_widget_destroy(confirm_dialog);
}

void template_row_activated(GtkTreeSelection *selection, gpointer data)
{
	GtkTreeModel *model;
	gchar *content, *template;
	GtkTreeIter iter;

	if(current_key) {
		g_free(current_key);
	}
	//g_print("a\n");
    if(gtk_tree_selection_get_selected(selection, &model, &iter)) {
		gtk_tree_model_get(model, &iter, 0, &current_key, -1);

		// display template content
		template = template_find(current_key);
		if(template) {
			content = template_convert_to_display(template);
			update_template_display(content);
			g_free(content);
		}
	}
}

static void templates_treeview_add_column(void)
{  
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	/* column for description */
	renderer = gtk_cell_renderer_text_new();
 	column = gtk_tree_view_column_new_with_attributes(_("Name/Shortcut"),
						     renderer,
						     "text",
						     0,
						     NULL);
	gtk_tree_view_column_set_sort_column_id(column, 0);
 	gtk_tree_view_append_column(GTK_TREE_VIEW(preferences_dialog.Templates), column);
}


void preferences_dialog_create(void){
	GList *comboitems = NULL;
	GList *items = NULL;
	GString *sample_text;
	
	get_current_preferences();

	preferences_dialog.window = gtk_dialog_new_with_buttons(_("Preferences"),
		GTK_WINDOW(main_window.window), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL);
	
	preferences_dialog.notebook1 = gtk_notebook_new();
	gtk_widget_show(preferences_dialog.notebook1);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(preferences_dialog.window)->vbox),preferences_dialog.notebook1);
	
	preferences_dialog.vbox6 = gtk_vbox_new(FALSE, 8);
	gtk_widget_show(preferences_dialog.vbox6);
	gtk_container_add(GTK_CONTAINER(preferences_dialog.notebook1), preferences_dialog.vbox6);
	gtk_container_set_border_width(GTK_CONTAINER(preferences_dialog.vbox6), 8);
	
	preferences_dialog.hbox14 = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(preferences_dialog.hbox14);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.vbox6), preferences_dialog.hbox14, FALSE, TRUE, 0);
	
	preferences_dialog.label32 = gtk_label_new(_("Tab size:"));
	gtk_widget_show(preferences_dialog.label32);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.hbox14), preferences_dialog.label32, FALSE, FALSE, 8);
	
	preferences_dialog.tab_size = gtk_hscale_new(GTK_ADJUSTMENT(gtk_adjustment_new(temp_preferences.tab_size, 0, 16, 1, 0, 0)));
	gtk_widget_show(preferences_dialog.tab_size);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.hbox14), preferences_dialog.tab_size, TRUE, TRUE, 0);
	gtk_scale_set_digits(GTK_SCALE(preferences_dialog.tab_size), 0);
	gtk_signal_connect(GTK_OBJECT(GTK_HSCALE(preferences_dialog.tab_size)), "value_changed",
                    GTK_SIGNAL_FUNC(on_tab_size_changed), NULL);
	
	preferences_dialog.show_indentation_guides = gtk_check_button_new_with_mnemonic(_("Show indentation guides"));
	gtk_widget_show(preferences_dialog.show_indentation_guides);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.vbox6), preferences_dialog.show_indentation_guides, FALSE, FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(preferences_dialog.show_indentation_guides), 8);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(preferences_dialog.show_indentation_guides), temp_preferences.show_indentation_guides);
	gtk_signal_connect(GTK_OBJECT(GTK_CHECK_BUTTON(preferences_dialog.show_indentation_guides)), "toggled", GTK_SIGNAL_FUNC(on_show_indentation_guides_toggle), NULL);
	
	preferences_dialog.line_wrapping = gtk_check_button_new_with_mnemonic(_("Wrap long lines"));
	gtk_widget_show(preferences_dialog.line_wrapping);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.vbox6), preferences_dialog.line_wrapping, FALSE, FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(preferences_dialog.line_wrapping), 8);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(preferences_dialog.line_wrapping), temp_preferences.line_wrapping);
	gtk_signal_connect(GTK_OBJECT(GTK_CHECK_BUTTON(preferences_dialog.line_wrapping)), "toggled", GTK_SIGNAL_FUNC(on_line_wrapping_toggle), NULL);
	
	preferences_dialog.use_tabs_instead_spaces = gtk_check_button_new_with_mnemonic(_("Use tabs instead of spaces for indentation"));
	gtk_widget_show(preferences_dialog.use_tabs_instead_spaces);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.vbox6), preferences_dialog.use_tabs_instead_spaces, FALSE, FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(preferences_dialog.use_tabs_instead_spaces), 8);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(preferences_dialog.use_tabs_instead_spaces), temp_preferences.use_tabs_instead_spaces);
	gtk_signal_connect(GTK_OBJECT(GTK_CHECK_BUTTON(preferences_dialog.use_tabs_instead_spaces)), "toggled", GTK_SIGNAL_FUNC(on_use_tabs_instead_spaces_toggle), NULL);
	
	preferences_dialog.edge_mode = gtk_check_button_new_with_mnemonic(_("Show right hand edge guide"));
	gtk_widget_show(preferences_dialog.edge_mode);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.vbox6), preferences_dialog.edge_mode, FALSE, FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(preferences_dialog.edge_mode), 8);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(preferences_dialog.edge_mode), temp_preferences.edge_mode);
	gtk_signal_connect(GTK_OBJECT(GTK_CHECK_BUTTON(preferences_dialog.edge_mode)), "toggled", GTK_SIGNAL_FUNC(on_edge_mode_toggle), NULL);
	
	preferences_dialog.hbox15 = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(preferences_dialog.hbox15);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.vbox6), preferences_dialog.hbox15, FALSE, TRUE, 0);
	
	preferences_dialog.label33 = gtk_label_new(_("Right hand edge colour:"));
	gtk_widget_show(preferences_dialog.label33);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.hbox15), preferences_dialog.label33, FALSE, FALSE, 8);
	
	preferences_dialog.edge_colour = gnome_color_picker_new();
	gtk_widget_show(preferences_dialog.edge_colour);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.hbox15), preferences_dialog.edge_colour, FALSE, FALSE, 0);
	gnome_color_picker_set_i8(GNOME_COLOR_PICKER(preferences_dialog.edge_colour), 
		(temp_preferences.edge_colour & 0xff),(temp_preferences.edge_colour & 0xff00) >> 8,(temp_preferences.edge_colour & 0xff0000) >> 16, 0);
	gtk_signal_connect(GTK_OBJECT(GNOME_COLOR_PICKER(preferences_dialog.edge_colour)), "color-set", GTK_SIGNAL_FUNC(on_edge_colour_changed), NULL);	
	
	preferences_dialog.hbox16 = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(preferences_dialog.hbox16);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.vbox6), preferences_dialog.hbox16, FALSE, TRUE, 0);
	
	preferences_dialog.label34 = gtk_label_new(_("Right hand edge column"));
	gtk_widget_show(preferences_dialog.label34);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.hbox16), preferences_dialog.label34, FALSE, FALSE, 8);
	
	preferences_dialog.edge_column = gtk_hscale_new(GTK_ADJUSTMENT(gtk_adjustment_new(temp_preferences.edge_column, 0, 160, 0, 0, 0)));
	gtk_widget_show(preferences_dialog.edge_column);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.hbox16), preferences_dialog.edge_column, TRUE, TRUE, 0);
	gtk_scale_set_digits(GTK_SCALE(preferences_dialog.edge_column), 0);
	gtk_signal_connect(GTK_OBJECT(GTK_HSCALE(preferences_dialog.edge_column)), "value_changed",
                    GTK_SIGNAL_FUNC(on_edge_column_changed), NULL);
	
	preferences_dialog.save_session = gtk_check_button_new_with_mnemonic(_("Save session(including open files) on exit"));
	gtk_widget_show(preferences_dialog.save_session);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.vbox6), preferences_dialog.save_session, FALSE, FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(preferences_dialog.save_session), 8);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(preferences_dialog.save_session), temp_preferences.save_session);
	gtk_signal_connect(GTK_OBJECT(GTK_CHECK_BUTTON(preferences_dialog.save_session)), "toggled", GTK_SIGNAL_FUNC(on_save_session_toggle), NULL);
	
	preferences_dialog.single_instance_only = gtk_check_button_new_with_mnemonic(_("Only ever run 1 copy of connectED at a time"));
	gtk_widget_show(preferences_dialog.single_instance_only);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.vbox6), preferences_dialog.single_instance_only, FALSE, FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(preferences_dialog.single_instance_only), 8);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(preferences_dialog.single_instance_only), temp_preferences.single_instance_only);
	gtk_signal_connect(GTK_OBJECT(GTK_CHECK_BUTTON(preferences_dialog.single_instance_only)), "toggled", GTK_SIGNAL_FUNC(on_single_instance_only_toggle), NULL);
	
	preferences_dialog.label29 = gtk_label_new(_("Editor"));
	gtk_widget_show(preferences_dialog.label29);
	gtk_notebook_set_tab_label(GTK_NOTEBOOK(preferences_dialog.notebook1), gtk_notebook_get_nth_page(GTK_NOTEBOOK(preferences_dialog.notebook1), 0), preferences_dialog.label29);
	
	preferences_dialog.vbox10 = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(preferences_dialog.vbox10);
	gtk_container_add(GTK_CONTAINER(preferences_dialog.notebook1), preferences_dialog.vbox10);
	gtk_container_set_border_width(GTK_CONTAINER(preferences_dialog.vbox10), 8);
	
	preferences_dialog.hbox22 = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(preferences_dialog.hbox22);
	gtk_box_pack_end(GTK_BOX(preferences_dialog.vbox10), preferences_dialog.hbox22, FALSE, TRUE, 8);
	
	preferences_dialog.frame2 = gtk_frame_new(NULL);
	gtk_widget_show(preferences_dialog.frame2);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.hbox22), preferences_dialog.frame2, TRUE, TRUE, 8);
	
	preferences_dialog.vbox11 = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(preferences_dialog.vbox11);
	gtk_container_add(GTK_CONTAINER(preferences_dialog.frame2), preferences_dialog.vbox11);
	
	preferences_dialog.bold_button = gtk_check_button_new_with_mnemonic(_("Bold"));
	gtk_widget_show(preferences_dialog.bold_button);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.vbox11), preferences_dialog.bold_button, FALSE, FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(preferences_dialog.bold_button), 8);
	gtk_signal_connect(GTK_OBJECT(GTK_TOGGLE_BUTTON(preferences_dialog.bold_button)), "toggled", GTK_SIGNAL_FUNC(on_bold_toggle), NULL);
	
	preferences_dialog.italic_button = gtk_check_button_new_with_mnemonic(_("Italic"));
	gtk_widget_show(preferences_dialog.italic_button);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.vbox11), preferences_dialog.italic_button, FALSE, FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(preferences_dialog.italic_button), 8);
	gtk_signal_connect(GTK_OBJECT(GTK_TOGGLE_BUTTON(preferences_dialog.italic_button)), "toggled", GTK_SIGNAL_FUNC(on_italic_toggle), NULL);
	
	preferences_dialog.label40 = gtk_label_new(_("Attributes"));
	gtk_widget_show(preferences_dialog.label40);
	gtk_frame_set_label_widget(GTK_FRAME(preferences_dialog.frame2), preferences_dialog.label40);
	
	preferences_dialog.frame3 = gtk_frame_new(NULL);
	gtk_widget_show(preferences_dialog.frame3);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.hbox22), preferences_dialog.frame3, TRUE, TRUE, 8);
	
	preferences_dialog.vbox12 = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(preferences_dialog.vbox12);
	gtk_container_add(GTK_CONTAINER(preferences_dialog.frame3), preferences_dialog.vbox12);
	gtk_container_set_border_width(GTK_CONTAINER(preferences_dialog.vbox12), 8);
	
	preferences_dialog.hbox23 = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(preferences_dialog.hbox23);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.vbox12), preferences_dialog.hbox23, FALSE, TRUE, 0);
	
	preferences_dialog.label42 = gtk_label_new(_("Foreground:"));
	gtk_widget_show(preferences_dialog.label42);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.hbox23), preferences_dialog.label42, FALSE, FALSE, 0);
	
	preferences_dialog.foreground_colour = gnome_color_picker_new();
	gtk_widget_show(preferences_dialog.foreground_colour);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.hbox23), preferences_dialog.foreground_colour, FALSE, FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(preferences_dialog.foreground_colour), 8);
	gtk_signal_connect(GTK_OBJECT(GNOME_COLOR_PICKER(preferences_dialog.foreground_colour)), "color-set", GTK_SIGNAL_FUNC(on_fore_changed), NULL);	
	
	preferences_dialog.hbox24 = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(preferences_dialog.hbox24);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.vbox12), preferences_dialog.hbox24, FALSE, TRUE, 0);
	
	preferences_dialog.label43 = gtk_label_new(_("Background:"));
	gtk_widget_show(preferences_dialog.label43);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.hbox24), preferences_dialog.label43, FALSE, FALSE, 0);
	
	preferences_dialog.background_colour = gnome_color_picker_new();
	gtk_widget_show(preferences_dialog.background_colour);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.hbox24), preferences_dialog.background_colour, FALSE, FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(preferences_dialog.background_colour), 8);
	gtk_signal_connect(GTK_OBJECT(GNOME_COLOR_PICKER(preferences_dialog.background_colour)), "color-set", GTK_SIGNAL_FUNC(on_back_changed), NULL);	
	
	preferences_dialog.label41 = gtk_label_new(_("Colours"));
	gtk_widget_show(preferences_dialog.label41);
	gtk_frame_set_label_widget(GTK_FRAME(preferences_dialog.frame3), preferences_dialog.label41);
	
	preferences_dialog.highlighting_editor = g_new0(Editor,1);
	preferences_dialog.highlighting_editor->scintilla = gtk_scintilla_new();
	sample_text = create_sample_text();
	gtk_scintilla_add_text(GTK_SCINTILLA(preferences_dialog.highlighting_editor->scintilla), sample_text->len, sample_text->str);
	gtk_scintilla_goto_pos(GTK_SCINTILLA(preferences_dialog.highlighting_editor->scintilla), 0);
	gtk_scintilla_set_read_only(GTK_SCINTILLA(preferences_dialog.highlighting_editor->scintilla), 1);
	tab_php_set_lexer(preferences_dialog.highlighting_editor);

	preferences_dialog.code_sample = preferences_dialog.highlighting_editor->scintilla;	
	gtk_widget_set_size_request(preferences_dialog.code_sample, 200, 200);
	gtk_widget_show(preferences_dialog.code_sample);
	gtk_box_pack_end(GTK_BOX(preferences_dialog.vbox10), preferences_dialog.code_sample, TRUE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(preferences_dialog.code_sample), 8);
	
	preferences_dialog.frame4 = gtk_frame_new(NULL);
	gtk_widget_show(preferences_dialog.frame4);
	gtk_box_pack_end(GTK_BOX(preferences_dialog.vbox10), preferences_dialog.frame4, TRUE, TRUE, 0);
	
	preferences_dialog.hbox25 = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(preferences_dialog.hbox25);
	gtk_container_add(GTK_CONTAINER(preferences_dialog.frame4), preferences_dialog.hbox25);
	
	preferences_dialog.font_combo = gtk_combo_box_entry_new_text();
	gtk_widget_show(preferences_dialog.font_combo);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.hbox25), preferences_dialog.font_combo, FALSE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(preferences_dialog.font_combo), 8);
	
	//preferences_dialog.combo_entry2 = GTK_COMBO(preferences_dialog.font_combo)->entry;
	//gtk_widget_show(preferences_dialog.combo_entry2);
	//gtk_combo_set_use_arrows_always(GTK_COMBO(preferences_dialog.font_combo), TRUE);
	comboitems = get_font_names();
	for(items = g_list_first(comboitems); items != NULL; items = g_list_next(items)) {
		// Suggested by__tim in #Gtk+/Freenode to be able to find the item again from set_control_to_highlight
		g_object_set_qdata(G_OBJECT(preferences_dialog.font_combo), g_quark_from_string(items->data), 
			GINT_TO_POINTER(gtk_tree_model_iter_n_children(gtk_combo_box_get_model(GTK_COMBO_BOX(preferences_dialog.font_combo)), NULL)));
		gtk_combo_box_append_text(GTK_COMBO_BOX(preferences_dialog.font_combo), items->data);
	}
	//gtk_combo_set_popdown_strings(GTK_COMBO(preferences_dialog.font_combo), comboitems);
	gtk_signal_connect(GTK_OBJECT(GTK_COMBO_BOX(preferences_dialog.font_combo)), "changed",
                      GTK_SIGNAL_FUNC(on_fontname_entry_changed),
                      NULL);
	g_list_free(comboitems);

	preferences_dialog.size_combo = gtk_combo_box_entry_new_text();
	gtk_widget_show(preferences_dialog.size_combo);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.hbox25), preferences_dialog.size_combo, FALSE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(preferences_dialog.size_combo), 8);
	
	//preferences_dialog.combo_entry3 = GTK_COMBO(preferences_dialog.size_combo)->entry;
	//gtk_widget_show(preferences_dialog.combo_entry3);
	//gtk_combo_set_use_arrows_always(GTK_COMBO(preferences_dialog.size_combo), TRUE);
	comboitems = get_font_sizes();
	for(items = g_list_first(comboitems); items != NULL; items = g_list_next(items)) {
		// Suggested by__tim in #Gtk+/Freenode to be able to find the item again from set_control_to_highlight
		g_object_set_qdata(G_OBJECT(preferences_dialog.size_combo), g_quark_from_string(items->data), 
			GINT_TO_POINTER(gtk_tree_model_iter_n_children(gtk_combo_box_get_model(GTK_COMBO_BOX(preferences_dialog.size_combo)), NULL)));
		gtk_combo_box_append_text(GTK_COMBO_BOX(preferences_dialog.size_combo), items->data);
		//g_print("Appending Font Size: %s, %d\n", items->data, g_quark_from_string(items->data));
	}
	//gtk_combo_set_popdown_strings(GTK_COMBO(preferences_dialog.size_combo), comboitems);
	gtk_signal_connect(GTK_OBJECT(GTK_COMBO_BOX(preferences_dialog.size_combo)), "changed",
                      GTK_SIGNAL_FUNC(on_fontsize_entry_changed),
                      NULL);
	g_list_free(comboitems);
	
	preferences_dialog.label44 = gtk_label_new(_("Font"));
	gtk_widget_show(preferences_dialog.label44);
	gtk_frame_set_label_widget(GTK_FRAME(preferences_dialog.frame4), preferences_dialog.label44);
	
	preferences_dialog.frame1 = gtk_frame_new(NULL);
	gtk_widget_show(preferences_dialog.frame1);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.vbox10), preferences_dialog.frame1, TRUE, TRUE, 0);
	
	preferences_dialog.element_combo = gtk_combo_new();
	gtk_widget_show(preferences_dialog.element_combo);
	gtk_container_add(GTK_CONTAINER(preferences_dialog.frame1), preferences_dialog.element_combo);
	gtk_container_set_border_width(GTK_CONTAINER(preferences_dialog.element_combo), 8);
	gtk_combo_set_popdown_strings(GTK_COMBO(preferences_dialog.element_combo), preferences_dialog.highlighting_elements);
	current_highlighting_element = preferences_dialog.highlighting_elements->data;
	gtk_signal_connect(GTK_OBJECT(GTK_COMBO(preferences_dialog.element_combo)->entry), "changed",
                      GTK_SIGNAL_FUNC(on_element_entry_changed),
                      NULL);
	g_list_free(preferences_dialog.highlighting_elements);
	
	preferences_dialog.combo_entry4 = GTK_COMBO(preferences_dialog.element_combo)->entry;
	gtk_widget_show(preferences_dialog.combo_entry4);
	
	preferences_dialog.label39 = gtk_label_new(_("Element"));
	gtk_widget_show(preferences_dialog.label39);
	gtk_frame_set_label_widget(GTK_FRAME(preferences_dialog.frame1), preferences_dialog.label39);
	
	preferences_dialog.label30 = gtk_label_new(_("Colour"));
	gtk_widget_show(preferences_dialog.label30);
	gtk_notebook_set_tab_label(GTK_NOTEBOOK(preferences_dialog.notebook1), gtk_notebook_get_nth_page(GTK_NOTEBOOK(preferences_dialog.notebook1), 1), preferences_dialog.label30);
	
	preferences_dialog.vbox7 = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(preferences_dialog.vbox7);
	gtk_container_add(GTK_CONTAINER(preferences_dialog.notebook1), preferences_dialog.vbox7);
	gtk_container_set_border_width(GTK_CONTAINER(preferences_dialog.vbox7), 8);
	
	preferences_dialog.hbox17 = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(preferences_dialog.hbox17);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.vbox7), preferences_dialog.hbox17, FALSE, TRUE, 8);
	
	preferences_dialog.label35 = gtk_label_new(_("Syntax check program:"));
	gtk_widget_show(preferences_dialog.label35);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.hbox17), preferences_dialog.label35, FALSE, FALSE, 8);
	
	preferences_dialog.php_file_entry = gtk_entry_new();
	gtk_widget_show(preferences_dialog.php_file_entry);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.hbox17), preferences_dialog.php_file_entry, TRUE, TRUE, 0);
	gtk_entry_set_text(GTK_ENTRY(preferences_dialog.php_file_entry), temp_preferences.php_binary_location);
	gtk_signal_connect(GTK_OBJECT(preferences_dialog.php_file_entry),
                       "changed",
                       GTK_SIGNAL_FUNC(on_php_binary_location_changed),
                       NULL);

	/* Version for file_entry 
	preferences_dialog.php_file_entry = gnome_file_entry_new(NULL, NULL);
	gtk_widget_show(preferences_dialog.php_file_entry);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.hbox17), preferences_dialog.php_file_entry, TRUE, TRUE, 0);
	gtk_entry_set_text(GTK_ENTRY(gnome_file_entry_gtk_entry(GNOME_FILE_ENTRY(preferences_dialog.php_file_entry))), temp_preferences.php_binary_location);
	gtk_signal_connect(GTK_OBJECT(gnome_file_entry_gtk_entry(GNOME_FILE_ENTRY(preferences_dialog.php_file_entry))),
                       "changed",
                       GTK_SIGNAL_FUNC(on_php_binary_location_changed),
                       NULL);*/

	/*preferences_dialog.combo_entry1 = gnome_file_entry_gtk_entry(GNOME_FILE_ENTRY(preferences_dialog.php_file_entry));
	gtk_widget_show(preferences_dialog.combo_entry1);
	gtk_entry_set_text(GTK_ENTRY(preferences_dialog.php_file_entry), preferences.php_binary_location);*/
					
	preferences_dialog.hbox18 = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(preferences_dialog.hbox18);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.vbox7), preferences_dialog.hbox18, FALSE, TRUE, 8);
	
	preferences_dialog.label36 = gtk_label_new(_("Recognised PHP file extensions:"));
	gtk_widget_show(preferences_dialog.label36);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.hbox18), preferences_dialog.label36, FALSE, FALSE, 8);
	
	preferences_dialog.file_extensions = gtk_entry_new();
	gtk_widget_show(preferences_dialog.file_extensions);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.hbox18), preferences_dialog.file_extensions, TRUE, TRUE, 0);
	gtk_entry_set_text(GTK_ENTRY(preferences_dialog.file_extensions), temp_preferences.php_file_extensions);
	gtk_signal_connect(GTK_OBJECT(preferences_dialog.file_extensions),
    		"changed", GTK_SIGNAL_FUNC(on_php_file_extensions_changed),NULL);

	preferences_dialog.hbox26 = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(preferences_dialog.hbox26);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.vbox7), preferences_dialog.hbox26, FALSE, TRUE, 8);
	
	preferences_dialog.label45 = gtk_label_new(_("Shared library path(s):"));
	gtk_widget_show(preferences_dialog.label45);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.hbox26), preferences_dialog.label45, FALSE, FALSE, 8);
	
	preferences_dialog.shared_source = gtk_entry_new();
	gtk_widget_show(preferences_dialog.shared_source);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.hbox26), preferences_dialog.shared_source, TRUE, TRUE, 0);
	gtk_entry_set_text(GTK_ENTRY(preferences_dialog.shared_source), temp_preferences.shared_source_location);
	gtk_signal_connect(GTK_OBJECT(preferences_dialog.shared_source),
    		"changed", GTK_SIGNAL_FUNC(on_shared_source_changed),NULL);

	preferences_dialog.hbox19 = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(preferences_dialog.hbox19);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.vbox7), preferences_dialog.hbox19, FALSE, TRUE, 8);
	
	preferences_dialog.label37 = gtk_label_new(_("Delay(calltip/function list):"));
	gtk_widget_show(preferences_dialog.label37);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.hbox19), preferences_dialog.label37, FALSE, FALSE, 8);
					
	preferences_dialog.delay = gtk_hscale_new(GTK_ADJUSTMENT(gtk_adjustment_new(temp_preferences.calltip_delay, 0, 2500, 0, 0, 0)));
	gtk_widget_show(preferences_dialog.delay);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.hbox19), preferences_dialog.delay, TRUE, TRUE, 0);
	gtk_signal_connect(GTK_OBJECT(GTK_HSCALE(preferences_dialog.delay)), "value_changed",
                    GTK_SIGNAL_FUNC(on_calltip_delay_changed), NULL);
	
	preferences_dialog.hbox20 = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(preferences_dialog.hbox20);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.vbox7), preferences_dialog.hbox20, TRUE, TRUE, 8);
	
	preferences_dialog.label38 = gtk_label_new(_("Templates:"));
	gtk_widget_show(preferences_dialog.label38);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.hbox20), preferences_dialog.label38, FALSE, FALSE, 8);
	
	preferences_dialog.vbox8 = gtk_vbox_new(TRUE, 0);
	gtk_widget_show(preferences_dialog.vbox8);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.hbox20), preferences_dialog.vbox8, TRUE, TRUE, 0);
	
	preferences_dialog.hbox21 = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(preferences_dialog.hbox21);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.vbox8), preferences_dialog.hbox21, TRUE, TRUE, 0);
	
	preferences_dialog.scrolledwindow1 = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_show(preferences_dialog.scrolledwindow1);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.hbox21), preferences_dialog.scrolledwindow1, TRUE, TRUE, 0);
	
	preferences_dialog.template_store = gtk_list_store_new(1, G_TYPE_STRING);
	add_templates_to_store();
	
	preferences_dialog.Templates = gtk_tree_view_new_with_model(GTK_TREE_MODEL(preferences_dialog.template_store));
	// g_object_unref(preferences_dialog.template_store);
	// Can't unref it because I need to add to it later....
	templates_treeview_add_column();
	
    gtk_tree_view_set_search_column(GTK_TREE_VIEW(preferences_dialog.Templates),0);
	gtk_widget_show(preferences_dialog.Templates);
	gtk_container_add(GTK_CONTAINER(preferences_dialog.scrolledwindow1), preferences_dialog.Templates);

	preferences_dialog.template_selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(preferences_dialog.Templates));
	gtk_tree_selection_set_mode(preferences_dialog.template_selection, GTK_SELECTION_SINGLE);
	g_signal_connect(G_OBJECT(preferences_dialog.template_selection), "changed",
		G_CALLBACK(template_row_activated), NULL);

	preferences_dialog.vbox9 = gtk_vbox_new(TRUE, 0);
	gtk_widget_show(preferences_dialog.vbox9);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.hbox21), preferences_dialog.vbox9, FALSE, TRUE, 8);
	gtk_container_set_border_width(GTK_CONTAINER(preferences_dialog.vbox9), 8);
	
	preferences_dialog.add_template_button = gtk_button_new_with_mnemonic(_("Add..."));
	gtk_widget_show(preferences_dialog.add_template_button);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.vbox9), preferences_dialog.add_template_button, FALSE, FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(preferences_dialog.add_template_button), 2);
	gtk_signal_connect(GTK_OBJECT(preferences_dialog.add_template_button), "clicked", GTK_SIGNAL_FUNC(add_template_clicked), NULL);
	
	preferences_dialog.edit_template_button = gtk_button_new_with_mnemonic(_("Edit..."));
	gtk_widget_show(preferences_dialog.edit_template_button);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.vbox9), preferences_dialog.edit_template_button, FALSE, FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(preferences_dialog.edit_template_button), 2);
	gtk_signal_connect(GTK_OBJECT(preferences_dialog.edit_template_button), "clicked", GTK_SIGNAL_FUNC(edit_template_clicked), NULL);
	
	preferences_dialog.delete_template_button = gtk_button_new_with_mnemonic(_("Delete"));
	gtk_widget_show(preferences_dialog.delete_template_button);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.vbox9), preferences_dialog.delete_template_button, FALSE, FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(preferences_dialog.delete_template_button), 2);
	gtk_signal_connect(GTK_OBJECT(preferences_dialog.delete_template_button), "clicked", GTK_SIGNAL_FUNC(delete_template_clicked), NULL);
	
	preferences_dialog.template_sample_scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_show(preferences_dialog.template_sample_scrolled);
	gtk_box_pack_start(GTK_BOX(preferences_dialog.vbox8), preferences_dialog.template_sample_scrolled, TRUE, TRUE, 0);
	
	preferences_dialog.template_sample = gtk_text_view_new();
	gtk_widget_show(preferences_dialog.template_sample);
	gtk_container_add(GTK_CONTAINER(preferences_dialog.template_sample_scrolled), preferences_dialog.template_sample);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(preferences_dialog.template_sample), FALSE);
	preferences_dialog.label31 = gtk_label_new(_("PHP Coding"));
	gtk_widget_show(preferences_dialog.label31);
	gtk_notebook_set_tab_label(GTK_NOTEBOOK(preferences_dialog.notebook1), gtk_notebook_get_nth_page(GTK_NOTEBOOK(preferences_dialog.notebook1), 2), preferences_dialog.label31);
	
	preferences_dialog.apply_button = gtk_button_new_with_mnemonic(_("Apply"));
	gtk_widget_show(preferences_dialog.apply_button);
	//gtk_box_pack_end(GTK_BOX(GTK_DIALOG(preferences_dialog.window)->action_area), preferences_dialog.apply_button, FALSE, FALSE, 4);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(preferences_dialog.window)->action_area),preferences_dialog.apply_button);
	gtk_signal_connect(GTK_OBJECT(preferences_dialog.apply_button),
	                    "clicked", GTK_SIGNAL_FUNC(apply_preferences), NULL);
	
	get_current_highlighting_settings(current_highlighting_element);
}//preferences_dialog_create
