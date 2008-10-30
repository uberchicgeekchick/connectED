/* GtkScintilla2: Wrapper widget for the Scintilla editing component.
 *
 * Copyright (c) 2002  Dennis J Houy <djhouy@paw.co.za>
 * Copyright (c) 2001  Michele Campeotto <micampe@micampe.it>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, 
 * Boston, MA  02111-1307  USA.
 */

#include "gtkscintilla.h"
#include "marshal.h"

#define PLAT_GTK 2
#include "scintilla/include/Scintilla.h"
#include "scintilla/include/ScintillaWidget.h"
#define GPOINTER_TO_LONG(p)  ((glong) (p))

enum {
    STYLE_NEEDED,
    CHAR_ADDED,
    SAVE_POINT_REACHED,
    SAVE_POINT_LEFT,
    MODIFY_ATTEMPT_RO,
    KEY,
    DOUBLE_CLICK,
    UPDATE_UI,
    MODIFIED,
    MACRO_RECORD,
    MARGIN_CLICK,
    NEED_SHOWN,
    PAINTED,
    USER_LIST_SELECTION,
    URI_DROPPED,
    DWELL_START,
    DWELL_END,
    ZOOM,
    LAST_SIGNAL
};

static void gtk_scintilla_class_init (GtkScintillaClass *klass);
static void gtk_scintilla_init       (GtkScintilla      *sci);
static void gtk_scintilla_destroy    (GtkObject         *object);
static void notify_cb                (GtkWidget         *w,
                                      gint               param,
                                      gpointer           notif,
                                      gpointer           data);
static void pass_throug_key          (GtkScintilla      *sci,
                                      gint               ch,
                                      gint               modifiers);

static gpointer parent_class;
static guint signals[LAST_SIGNAL] = { 0 };

GType
gtk_scintilla_get_type (void)
{
    static GType our_type = 0;
    
    if (!our_type) {
        static const GTypeInfo our_info =
        {
            sizeof (GtkScintillaClass),
            NULL,               /* base_init */
            NULL,               /* base_finalize */
            (GClassInitFunc) gtk_scintilla_class_init,
            NULL,               /* class_finalize */
            NULL,               /* class_data */
            sizeof (GtkScintilla),
            0,                  /* n_preallocs */
            (GInstanceInitFunc) gtk_scintilla_init,
        };

        our_type = g_type_register_static (GTK_TYPE_FRAME, "GtkScintilla",
                                           &our_info, 0);
    }
    
    return our_type;
}

static void
gtk_scintilla_class_init (GtkScintillaClass *klass)
{
    GtkObjectClass *object_class;
    
    object_class = (GtkObjectClass *) klass;
    parent_class = g_type_class_peek_parent (klass);

    object_class->destroy = gtk_scintilla_destroy;
    
    signals[STYLE_NEEDED] =
        g_signal_new ("style_needed",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GtkScintillaClass, style_needed),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__INT,
                      G_TYPE_NONE, 1,
                      G_TYPE_INT);

    signals[CHAR_ADDED] =
        g_signal_new ("char_added",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GtkScintillaClass, char_added),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__INT,
                      G_TYPE_NONE, 1,
                      G_TYPE_INT);

    signals[SAVE_POINT_REACHED] =
        g_signal_new ("save_point_reached",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GtkScintillaClass, save_point_reached),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    signals[SAVE_POINT_LEFT] =
        g_signal_new ("save_point_left",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GtkScintillaClass, save_point_left),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    signals[MODIFY_ATTEMPT_RO] =
        g_signal_new ("modify_attempt_ro",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GtkScintillaClass, modify_attempt_ro),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    signals[KEY] =
        g_signal_new ("key",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GtkScintillaClass, key),
                      NULL, NULL,
                      scintilla_marshal_VOID__INT_INT,
                      G_TYPE_NONE, 2,
                      G_TYPE_INT, G_TYPE_INT);

    signals[DOUBLE_CLICK] =
        g_signal_new ("double_click",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GtkScintillaClass, double_click),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    signals[UPDATE_UI] =
        g_signal_new ("update_ui",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GtkScintillaClass, update_ui),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    signals[MODIFIED] =
        g_signal_new ("modified",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GtkScintillaClass, modified),
                      NULL, NULL,
                      scintilla_marshal_VOID__INT_INT_STRING_INT_INT_INT_INT_INT,
                      G_TYPE_NONE, 8,
                      G_TYPE_INT, G_TYPE_INT, G_TYPE_STRING,
                      G_TYPE_INT, G_TYPE_INT, G_TYPE_INT,
                      G_TYPE_INT, G_TYPE_INT);

    signals[MACRO_RECORD] =
        g_signal_new ("macro_record",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GtkScintillaClass, macro_record),
                      NULL, NULL,
                      scintilla_marshal_VOID__INT_ULONG_LONG,
                      G_TYPE_NONE, 3,
                      G_TYPE_INT, G_TYPE_ULONG, G_TYPE_LONG);

    signals[MARGIN_CLICK] =
        g_signal_new ("margin_click",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GtkScintillaClass, margin_click),
                      NULL, NULL,
                      scintilla_marshal_VOID__INT_INT_INT,
                      G_TYPE_NONE, 3,
                      G_TYPE_INT, G_TYPE_INT, G_TYPE_INT);

    signals[NEED_SHOWN] =
        g_signal_new ("need_shown",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GtkScintillaClass, need_shown),
                      NULL, NULL,
                      scintilla_marshal_VOID__INT_INT,
                      G_TYPE_NONE, 2,
                      G_TYPE_INT, G_TYPE_INT);

    signals[PAINTED] =
        g_signal_new ("painted",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GtkScintillaClass, painted),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    signals[USER_LIST_SELECTION] =
        g_signal_new ("user_list_selection",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GtkScintillaClass, user_list_selection),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 2,
                      G_TYPE_INT, G_TYPE_STRING);

    signals[URI_DROPPED] =
        g_signal_new ("uri_dropped",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GtkScintillaClass, uri_dropped),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__STRING,
                      G_TYPE_NONE, 1,
                      G_TYPE_STRING);

    signals[DWELL_START] =
        g_signal_new ("dwell_start",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GtkScintillaClass, dwell_start),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__INT,
                      G_TYPE_NONE, 1,
                      G_TYPE_INT);

    signals[DWELL_END] =
        g_signal_new ("dwell_end",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GtkScintillaClass, dwell_end),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__INT,
                      G_TYPE_NONE, 1,
                      G_TYPE_INT);

    signals[ZOOM] =
        g_signal_new ("zoom",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GtkScintillaClass, zoom),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);
}

static void
gtk_scintilla_init (GtkScintilla *sci)
{
    sci->scintilla = GTK_WIDGET (scintilla_new ());
    g_signal_connect (G_OBJECT (sci->scintilla), "sci-notify",
                      G_CALLBACK (notify_cb), GTK_WIDGET (sci));
    
    gtk_container_add (GTK_CONTAINER (sci), sci->scintilla);
    
    gtk_widget_set (GTK_WIDGET (sci->scintilla),
                    "visible", TRUE, NULL);
    
    gtk_widget_show (GTK_WIDGET (sci->scintilla));
    gtk_widget_show_all (GTK_WIDGET(sci));
}

static void
gtk_scintilla_destroy (GtkObject *object)
{
    g_return_if_fail (object != NULL);
    g_return_if_fail (GTK_IS_SCINTILLA (object));
}

GtkWidget *gtk_scintilla_new (void)
{
    GtkScintilla *scintilla;
    
    scintilla = (GtkScintilla *) gtk_type_new (gtk_scintilla_get_type ());
    
    return GTK_WIDGET (scintilla);
}

glong
gtk_scintilla_find_text (GtkScintilla *sci, gint flags, gchar *text,
                         glong chrg_min, glong chrg_max,
                         glong *text_min, glong *text_max)
{
    glong find_pos;
    struct TextToFind ft = {{0, 0}, 0, {0, 0}};
    
    ft.chrg.cpMin = chrg_min;
    ft.chrg.cpMax = chrg_max;
    ft.chrgText.cpMin = *text_min;
    ft.chrgText.cpMax = *text_max;
    ft.lpstrText = text;
    
    find_pos = scintilla_send_message (SCINTILLA (sci->scintilla),
                                       SCI_FINDTEXT,
                                       (long) flags,
                                       GPOINTER_TO_LONG (&ft));
    
    *text_min = ft.chrgText.cpMin;
    *text_max = ft.chrgText.cpMax;
    
    return find_pos;
}

gchar *
gtk_scintilla_get_text_range (GtkScintilla *sci,
                              gint start, gint end, gint *length)
{
    gchar *buffer = NULL;
    struct TextRange tr = {{0, 0}, 0};

    buffer = g_strnfill (end - start + 2, 0);

    tr.chrg.cpMin = (long) start;
    tr.chrg.cpMax = (long) end;
    tr.lpstrText = (char *) buffer;

    *length = scintilla_send_message (SCINTILLA (sci->scintilla),
                                      SCI_GETTEXTRANGE, 0,
                                      GPOINTER_TO_LONG (&tr));

    return buffer;
}

/* --- Start of autogenerated code --- */

void gtk_scintilla_add_text(GtkScintilla *sci, int length, const gchar * text)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2001, (gulong) length, (gulong) text);
}

void gtk_scintilla_insert_text(GtkScintilla *sci, glong pos, const gchar * text)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2003, (gulong) pos, (gulong) text);
}

void gtk_scintilla_clear_all(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2004, 0, 0);
}

void gtk_scintilla_clear_document_style(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2005, 0, 0);
}

int gtk_scintilla_get_length(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2006, 0, 0);
}

int gtk_scintilla_get_char_at(GtkScintilla *sci, glong pos)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2007, (gulong) pos, 0);
}

glong gtk_scintilla_get_current_pos(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2008, 0, 0);
}

glong gtk_scintilla_get_anchor(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2009, 0, 0);
}

int gtk_scintilla_get_style_at(GtkScintilla *sci, glong pos)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2010, (gulong) pos, 0);
}

void gtk_scintilla_redo(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2011, 0, 0);
}

void gtk_scintilla_set_undo_collection(GtkScintilla *sci, gint collect_undo)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2012, (gulong) collect_undo, 0);
}

void gtk_scintilla_select_all(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2013, 0, 0);
}

void gtk_scintilla_set_save_point(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2014, 0, 0);
}

gint gtk_scintilla_can_redo(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2016, 0, 0);
}

int gtk_scintilla_marker_line_from_handle(GtkScintilla *sci, int handle)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2017, (gulong) handle, 0);
}

void gtk_scintilla_marker_delete_handle(GtkScintilla *sci, int handle)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2018, (gulong) handle, 0);
}

gint gtk_scintilla_get_undo_collection(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2019, 0, 0);
}

int gtk_scintilla_get_view_ws(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2020, 0, 0);
}

void gtk_scintilla_set_view_ws(GtkScintilla *sci, int view_ws)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2021, (gulong) view_ws, 0);
}

glong gtk_scintilla_position_from_point(GtkScintilla *sci, int x, int y)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2022, (gulong) x, (gulong) y);
}

glong gtk_scintilla_position_from_point_close(GtkScintilla *sci, int x, int y)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2023, (gulong) x, (gulong) y);
}

void gtk_scintilla_goto_line(GtkScintilla *sci, int line)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2024, (gulong) line, 0);
}

void gtk_scintilla_goto_pos(GtkScintilla *sci, glong pos)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2025, (gulong) pos, 0);
}

void gtk_scintilla_set_anchor(GtkScintilla *sci, glong pos_anchor)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2026, (gulong) pos_anchor, 0);
}

int gtk_scintilla_get_cur_line(GtkScintilla *sci, int length, gchar * text)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2027, (gulong) length, (gulong) text);
}

glong gtk_scintilla_get_end_styled(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2028, 0, 0);
}

void gtk_scintilla_convert_eols(GtkScintilla *sci, int eol_mode)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2029, (gulong) eol_mode, 0);
}

int gtk_scintilla_get_eol_mode(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2030, 0, 0);
}

void gtk_scintilla_set_eol_mode(GtkScintilla *sci, int eol_mode)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2031, (gulong) eol_mode, 0);
}

void gtk_scintilla_start_styling(GtkScintilla *sci, glong pos, int mask)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2032, (gulong) pos, (gulong) mask);
}

void gtk_scintilla_set_styling(GtkScintilla *sci, int length, int style)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2033, (gulong) length, (gulong) style);
}

gint gtk_scintilla_get_buffered_draw(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2034, 0, 0);
}

void gtk_scintilla_set_buffered_draw(GtkScintilla *sci, gint buffered)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2035, (gulong) buffered, 0);
}

void gtk_scintilla_set_tab_width(GtkScintilla *sci, int tab_width)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2036, (gulong) tab_width, 0);
}

int gtk_scintilla_get_tab_width(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2121, 0, 0);
}

void gtk_scintilla_set_code_page(GtkScintilla *sci, int code_page)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2037, (gulong) code_page, 0);
}

void gtk_scintilla_set_use_palette(GtkScintilla *sci, gint use_palette)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2039, (gulong) use_palette, 0);
}

void gtk_scintilla_marker_define(GtkScintilla *sci, int marker_number, int marker_symbol)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2040, (gulong) marker_number, (gulong) marker_symbol);
}

void gtk_scintilla_marker_set_fore(GtkScintilla *sci, int marker_number, glong fore)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2041, (gulong) marker_number, (gulong) fore);
}

void gtk_scintilla_marker_set_back(GtkScintilla *sci, int marker_number, glong back)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2042, (gulong) marker_number, (gulong) back);
}

int gtk_scintilla_marker_add(GtkScintilla *sci, int line, int marker_number)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2043, (gulong) line, (gulong) marker_number);
}

void gtk_scintilla_marker_delete(GtkScintilla *sci, int line, int marker_number)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2044, (gulong) line, (gulong) marker_number);
}

void gtk_scintilla_marker_delete_all(GtkScintilla *sci, int marker_number)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2045, (gulong) marker_number, 0);
}

int gtk_scintilla_marker_get(GtkScintilla *sci, int line)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2046, (gulong) line, 0);
}

int gtk_scintilla_marker_next(GtkScintilla *sci, int line_start, int marker_mask)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2047, (gulong) line_start, (gulong) marker_mask);
}

int gtk_scintilla_marker_previous(GtkScintilla *sci, int line_start, int marker_mask)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2048, (gulong) line_start, (gulong) marker_mask);
}

void gtk_scintilla_marker_define_pixmap(GtkScintilla *sci, int marker_number, const gchar * pixmap)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2049, (gulong) marker_number, (gulong) pixmap);
}

void gtk_scintilla_marker_add_set(GtkScintilla *sci, int line, int set)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2466, (gulong) line, (gulong) set);
}

void gtk_scintilla_marker_set_alpha(GtkScintilla *sci, int marker_number, int alpha)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2476, (gulong) marker_number, (gulong) alpha);
}

void gtk_scintilla_set_margin_type_n(GtkScintilla *sci, int margin, int margin_type)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2240, (gulong) margin, (gulong) margin_type);
}

int gtk_scintilla_get_margin_type_n(GtkScintilla *sci, int margin)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2241, (gulong) margin, 0);
}

void gtk_scintilla_set_margin_width_n(GtkScintilla *sci, int margin, int pixel_width)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2242, (gulong) margin, (gulong) pixel_width);
}

int gtk_scintilla_get_margin_width_n(GtkScintilla *sci, int margin)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2243, (gulong) margin, 0);
}

void gtk_scintilla_set_margin_mask_n(GtkScintilla *sci, int margin, int mask)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2244, (gulong) margin, (gulong) mask);
}

int gtk_scintilla_get_margin_mask_n(GtkScintilla *sci, int margin)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2245, (gulong) margin, 0);
}

void gtk_scintilla_set_margin_sensitive_n(GtkScintilla *sci, int margin, gint sensitive)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2246, (gulong) margin, (gulong) sensitive);
}

gint gtk_scintilla_get_margin_sensitive_n(GtkScintilla *sci, int margin)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2247, (gulong) margin, 0);
}

void gtk_scintilla_style_clear_all(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2050, 0, 0);
}

void gtk_scintilla_style_set_fore(GtkScintilla *sci, int style, glong fore)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2051, (gulong) style, (gulong) fore);
}

void gtk_scintilla_style_set_back(GtkScintilla *sci, int style, glong back)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2052, (gulong) style, (gulong) back);
}

void gtk_scintilla_style_set_bold(GtkScintilla *sci, int style, gint bold)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2053, (gulong) style, (gulong) bold);
}

void gtk_scintilla_style_set_italic(GtkScintilla *sci, int style, gint italic)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2054, (gulong) style, (gulong) italic);
}

void gtk_scintilla_style_set_size(GtkScintilla *sci, int style, int size_points)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2055, (gulong) style, (gulong) size_points);
}

void gtk_scintilla_style_set_font(GtkScintilla *sci, int style, const gchar * font_name)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2056, (gulong) style, (gulong) font_name);
}

void gtk_scintilla_style_set_eol_filled(GtkScintilla *sci, int style, gint filled)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2057, (gulong) style, (gulong) filled);
}

void gtk_scintilla_style_reset_default(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2058, 0, 0);
}

void gtk_scintilla_style_set_underline(GtkScintilla *sci, int style, gint underline)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2059, (gulong) style, (gulong) underline);
}

void gtk_scintilla_style_set_case(GtkScintilla *sci, int style, int case_force)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2060, (gulong) style, (gulong) case_force);
}

void gtk_scintilla_style_set_character_set(GtkScintilla *sci, int style, int character_set)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2066, (gulong) style, (gulong) character_set);
}

void gtk_scintilla_style_set_hot_spot(GtkScintilla *sci, int style, gint hotspot)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2409, (gulong) style, (gulong) hotspot);
}

void gtk_scintilla_set_sel_fore(GtkScintilla *sci, gint use_setting, glong fore)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2067, (gulong) use_setting, (gulong) fore);
}

void gtk_scintilla_set_sel_back(GtkScintilla *sci, gint use_setting, glong back)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2068, (gulong) use_setting, (gulong) back);
}

int gtk_scintilla_get_sel_alpha(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2477, 0, 0);
}

void gtk_scintilla_set_sel_alpha(GtkScintilla *sci, int alpha)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2478, (gulong) alpha, 0);
}

void gtk_scintilla_set_caret_fore(GtkScintilla *sci, glong fore)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2069, (gulong) fore, 0);
}

void gtk_scintilla_clear_all_cmd_keys(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2072, 0, 0);
}

void gtk_scintilla_set_styling_ex(GtkScintilla *sci, int length, const gchar * styles)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2073, (gulong) length, (gulong) styles);
}

void gtk_scintilla_style_set_visible(GtkScintilla *sci, int style, gint visible)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2074, (gulong) style, (gulong) visible);
}

int gtk_scintilla_get_caret_period(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2075, 0, 0);
}

void gtk_scintilla_set_caret_period(GtkScintilla *sci, int period_milliseconds)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2076, (gulong) period_milliseconds, 0);
}

void gtk_scintilla_set_word_chars(GtkScintilla *sci, const gchar * characters)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2077, 0, (gulong) characters);
}

void gtk_scintilla_begin_undo_action(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2078, 0, 0);
}

void gtk_scintilla_end_undo_action(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2079, 0, 0);
}

void gtk_scintilla_indic_set_style(GtkScintilla *sci, int indic, int style)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2080, (gulong) indic, (gulong) style);
}

int gtk_scintilla_indic_get_style(GtkScintilla *sci, int indic)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2081, (gulong) indic, 0);
}

void gtk_scintilla_indic_set_fore(GtkScintilla *sci, int indic, glong fore)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2082, (gulong) indic, (gulong) fore);
}

glong gtk_scintilla_indic_get_fore(GtkScintilla *sci, int indic)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2083, (gulong) indic, 0);
}

void gtk_scintilla_set_whitespace_fore(GtkScintilla *sci, gint use_setting, glong fore)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2084, (gulong) use_setting, (gulong) fore);
}

void gtk_scintilla_set_whitespace_back(GtkScintilla *sci, gint use_setting, glong back)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2085, (gulong) use_setting, (gulong) back);
}

void gtk_scintilla_set_style_bits(GtkScintilla *sci, int bits)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2090, (gulong) bits, 0);
}

int gtk_scintilla_get_style_bits(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2091, 0, 0);
}

void gtk_scintilla_set_line_state(GtkScintilla *sci, int line, int state)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2092, (gulong) line, (gulong) state);
}

int gtk_scintilla_get_line_state(GtkScintilla *sci, int line)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2093, (gulong) line, 0);
}

int gtk_scintilla_get_max_line_state(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2094, 0, 0);
}

gint gtk_scintilla_get_caret_line_visible(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2095, 0, 0);
}

void gtk_scintilla_set_caret_line_visible(GtkScintilla *sci, gint show)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2096, (gulong) show, 0);
}

glong gtk_scintilla_get_caret_line_back(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2097, 0, 0);
}

void gtk_scintilla_set_caret_line_back(GtkScintilla *sci, glong back)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2098, (gulong) back, 0);
}

void gtk_scintilla_style_set_changeable(GtkScintilla *sci, int style, gint changeable)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2099, (gulong) style, (gulong) changeable);
}

void gtk_scintilla_autoc_show(GtkScintilla *sci, int len_entered, const gchar * item_list)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2100, (gulong) len_entered, (gulong) item_list);
}

void gtk_scintilla_autoc_cancel(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2101, 0, 0);
}

gint gtk_scintilla_autoc_active(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2102, 0, 0);
}

glong gtk_scintilla_autoc_pos_start(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2103, 0, 0);
}

void gtk_scintilla_autoc_complete(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2104, 0, 0);
}

void gtk_scintilla_autoc_stops(GtkScintilla *sci, const gchar * character_set)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2105, 0, (gulong) character_set);
}

void gtk_scintilla_autoc_set_separator(GtkScintilla *sci, int separator_character)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2106, (gulong) separator_character, 0);
}

int gtk_scintilla_autoc_get_separator(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2107, 0, 0);
}

void gtk_scintilla_autoc_select(GtkScintilla *sci, const gchar * text)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2108, 0, (gulong) text);
}

void gtk_scintilla_autoc_set_cancel_at_start(GtkScintilla *sci, gint cancel)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2110, (gulong) cancel, 0);
}

gint gtk_scintilla_autoc_get_cancel_at_start(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2111, 0, 0);
}

void gtk_scintilla_autoc_set_fill_ups(GtkScintilla *sci, const gchar * character_set)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2112, 0, (gulong) character_set);
}

void gtk_scintilla_autoc_set_choose_single(GtkScintilla *sci, gint choose_single)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2113, (gulong) choose_single, 0);
}

gint gtk_scintilla_autoc_get_choose_single(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2114, 0, 0);
}

void gtk_scintilla_autoc_set_ignore_case(GtkScintilla *sci, gint ignore_case)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2115, (gulong) ignore_case, 0);
}

gint gtk_scintilla_autoc_get_ignore_case(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2116, 0, 0);
}

void gtk_scintilla_user_list_show(GtkScintilla *sci, int list_type, const gchar * item_list)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2117, (gulong) list_type, (gulong) item_list);
}

void gtk_scintilla_autoc_set_auto_hide(GtkScintilla *sci, gint auto_hide)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2118, (gulong) auto_hide, 0);
}

gint gtk_scintilla_autoc_get_auto_hide(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2119, 0, 0);
}

void gtk_scintilla_autoc_set_drop_rest_of_word(GtkScintilla *sci, gint drop_rest_of_word)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2270, (gulong) drop_rest_of_word, 0);
}

gint gtk_scintilla_autoc_get_drop_rest_of_word(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2271, 0, 0);
}

void gtk_scintilla_register_image(GtkScintilla *sci, int type, const gchar * xpm_data)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2405, (gulong) type, (gulong) xpm_data);
}

void gtk_scintilla_clear_registered_images(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2408, 0, 0);
}

int gtk_scintilla_autoc_get_type_separator(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2285, 0, 0);
}

void gtk_scintilla_autoc_set_type_separator(GtkScintilla *sci, int separator_character)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2286, (gulong) separator_character, 0);
}

void gtk_scintilla_autoc_set_max_width(GtkScintilla *sci, int character_count)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2208, (gulong) character_count, 0);
}

int gtk_scintilla_autoc_get_max_width(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2209, 0, 0);
}

void gtk_scintilla_autoc_set_max_height(GtkScintilla *sci, int row_count)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2210, (gulong) row_count, 0);
}

int gtk_scintilla_autoc_get_max_height(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2211, 0, 0);
}

void gtk_scintilla_set_indent(GtkScintilla *sci, int indent_size)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2122, (gulong) indent_size, 0);
}

int gtk_scintilla_get_indent(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2123, 0, 0);
}

void gtk_scintilla_set_use_tabs(GtkScintilla *sci, gint use_tabs)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2124, (gulong) use_tabs, 0);
}

gint gtk_scintilla_get_use_tabs(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2125, 0, 0);
}

void gtk_scintilla_set_line_indentation(GtkScintilla *sci, int line, int indent_size)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2126, (gulong) line, (gulong) indent_size);
}

int gtk_scintilla_get_line_indentation(GtkScintilla *sci, int line)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2127, (gulong) line, 0);
}

glong gtk_scintilla_get_line_indent_position(GtkScintilla *sci, int line)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2128, (gulong) line, 0);
}

int gtk_scintilla_get_column(GtkScintilla *sci, glong pos)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2129, (gulong) pos, 0);
}

void gtk_scintilla_set_h_scroll_bar(GtkScintilla *sci, gint show)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2130, (gulong) show, 0);
}

gint gtk_scintilla_get_h_scroll_bar(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2131, 0, 0);
}

void gtk_scintilla_set_indentation_guides(GtkScintilla *sci, gint show)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2132, (gulong) show, 0);
}

gint gtk_scintilla_get_indentation_guides(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2133, 0, 0);
}

void gtk_scintilla_set_highlight_guide(GtkScintilla *sci, int column)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2134, (gulong) column, 0);
}

int gtk_scintilla_get_highlight_guide(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2135, 0, 0);
}

int gtk_scintilla_get_line_end_position(GtkScintilla *sci, int line)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2136, (gulong) line, 0);
}

int gtk_scintilla_get_code_page(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2137, 0, 0);
}

glong gtk_scintilla_get_caret_fore(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2138, 0, 0);
}

gint gtk_scintilla_get_use_palette(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2139, 0, 0);
}

gint gtk_scintilla_get_read_only(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2140, 0, 0);
}

void gtk_scintilla_set_current_pos(GtkScintilla *sci, glong pos)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2141, (gulong) pos, 0);
}

void gtk_scintilla_set_selection_start(GtkScintilla *sci, glong pos)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2142, (gulong) pos, 0);
}

glong gtk_scintilla_get_selection_start(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2143, 0, 0);
}

void gtk_scintilla_set_selection_end(GtkScintilla *sci, glong pos)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2144, (gulong) pos, 0);
}

glong gtk_scintilla_get_selection_end(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2145, 0, 0);
}

void gtk_scintilla_set_print_magnification(GtkScintilla *sci, int magnification)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2146, (gulong) magnification, 0);
}

int gtk_scintilla_get_print_magnification(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2147, 0, 0);
}

void gtk_scintilla_set_print_colour_mode(GtkScintilla *sci, int mode)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2148, (gulong) mode, 0);
}

int gtk_scintilla_get_print_colour_mode(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2149, 0, 0);
}

int gtk_scintilla_get_first_visible_line(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2152, 0, 0);
}

int gtk_scintilla_get_line(GtkScintilla *sci, int line, gchar * text)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2153, (gulong) line, (gulong) text);
}

int gtk_scintilla_get_line_count(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2154, 0, 0);
}

void gtk_scintilla_set_margin_left(GtkScintilla *sci, int pixel_width)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2155, 0, (gulong) pixel_width);
}

int gtk_scintilla_get_margin_left(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2156, 0, 0);
}

void gtk_scintilla_set_margin_right(GtkScintilla *sci, int pixel_width)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2157, 0, (gulong) pixel_width);
}

int gtk_scintilla_get_margin_right(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2158, 0, 0);
}

gint gtk_scintilla_get_modify(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2159, 0, 0);
}

void gtk_scintilla_set_sel(GtkScintilla *sci, glong start, glong end)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2160, (gulong) start, (gulong) end);
}

int gtk_scintilla_get_sel_text(GtkScintilla *sci, gchar * text)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2161, 0, (gulong) text);
}

void gtk_scintilla_hide_selection(GtkScintilla *sci, gint normal)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2163, (gulong) normal, 0);
}

int gtk_scintilla_point_x_from_position(GtkScintilla *sci, glong pos)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2164, 0, (gulong) pos);
}

int gtk_scintilla_point_y_from_position(GtkScintilla *sci, glong pos)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2165, 0, (gulong) pos);
}

int gtk_scintilla_line_from_position(GtkScintilla *sci, glong pos)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2166, (gulong) pos, 0);
}

glong gtk_scintilla_position_from_line(GtkScintilla *sci, int line)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2167, (gulong) line, 0);
}

void gtk_scintilla_line_scroll(GtkScintilla *sci, int columns, int lines)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2168, (gulong) columns, (gulong) lines);
}

void gtk_scintilla_scroll_caret(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2169, 0, 0);
}

void gtk_scintilla_replace_sel(GtkScintilla *sci, const gchar * text)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2170, 0, (gulong) text);
}

void gtk_scintilla_set_read_only(GtkScintilla *sci, gint read_only)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2171, (gulong) read_only, 0);
}

void gtk_scintilla_null(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2172, 0, 0);
}

gint gtk_scintilla_can_paste(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2173, 0, 0);
}

gint gtk_scintilla_can_undo(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2174, 0, 0);
}

void gtk_scintilla_empty_undo_buffer(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2175, 0, 0);
}

void gtk_scintilla_undo(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2176, 0, 0);
}

void gtk_scintilla_cut(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2177, 0, 0);
}

void gtk_scintilla_copy(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2178, 0, 0);
}

void gtk_scintilla_paste(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2179, 0, 0);
}

void gtk_scintilla_clear(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2180, 0, 0);
}

void gtk_scintilla_set_text(GtkScintilla *sci, const gchar * text)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2181, 0, (gulong) text);
}

int gtk_scintilla_get_text(GtkScintilla *sci, int length, gchar * text)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2182, (gulong) length, (gulong) text);
}

int gtk_scintilla_get_text_length(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2183, 0, 0);
}

int gtk_scintilla_get_direct_function(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2184, 0, 0);
}

int gtk_scintilla_get_direct_pointer(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2185, 0, 0);
}

void gtk_scintilla_set_overtype(GtkScintilla *sci, gint overtype)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2186, (gulong) overtype, 0);
}

gint gtk_scintilla_get_overtype(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2187, 0, 0);
}

void gtk_scintilla_set_caret_width(GtkScintilla *sci, int pixel_width)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2188, (gulong) pixel_width, 0);
}

int gtk_scintilla_get_caret_width(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2189, 0, 0);
}

void gtk_scintilla_set_target_start(GtkScintilla *sci, glong pos)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2190, (gulong) pos, 0);
}

glong gtk_scintilla_get_target_start(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2191, 0, 0);
}

void gtk_scintilla_set_target_end(GtkScintilla *sci, glong pos)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2192, (gulong) pos, 0);
}

glong gtk_scintilla_get_target_end(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2193, 0, 0);
}

int gtk_scintilla_replace_target(GtkScintilla *sci, int length, const gchar * text)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2194, (gulong) length, (gulong) text);
}

int gtk_scintilla_replace_target_re(GtkScintilla *sci, int length, const gchar * text)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2195, (gulong) length, (gulong) text);
}

int gtk_scintilla_search_in_target(GtkScintilla *sci, int length, const gchar * text)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2197, (gulong) length, (gulong) text);
}

void gtk_scintilla_set_search_flags(GtkScintilla *sci, int flags)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2198, (gulong) flags, 0);
}

int gtk_scintilla_get_search_flags(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2199, 0, 0);
}

void gtk_scintilla_call_tip_show(GtkScintilla *sci, glong pos, const gchar * definition)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2200, (gulong) pos, (gulong) definition);
}

void gtk_scintilla_call_tip_cancel(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2201, 0, 0);
}

gint gtk_scintilla_call_tip_active(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2202, 0, 0);
}

glong gtk_scintilla_call_tip_pos_start(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2203, 0, 0);
}

void gtk_scintilla_call_tip_set_hlt(GtkScintilla *sci, int start, int end)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2204, (gulong) start, (gulong) end);
}

void gtk_scintilla_call_tip_set_back(GtkScintilla *sci, glong back)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2205, (gulong) back, 0);
}

void gtk_scintilla_call_tip_set_fore(GtkScintilla *sci, glong fore)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2206, (gulong) fore, 0);
}

void gtk_scintilla_call_tip_set_fore_hlt(GtkScintilla *sci, glong fore)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2207, (gulong) fore, 0);
}

void gtk_scintilla_call_tip_use_style(GtkScintilla *sci, int tab_size)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2212, (gulong) tab_size, 0);
}

int gtk_scintilla_visible_from_doc_line(GtkScintilla *sci, int line)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2220, (gulong) line, 0);
}

int gtk_scintilla_doc_line_from_visible(GtkScintilla *sci, int line_display)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2221, (gulong) line_display, 0);
}

int gtk_scintilla_wrap_count(GtkScintilla *sci, int line)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2235, (gulong) line, 0);
}

void gtk_scintilla_set_fold_level(GtkScintilla *sci, int line, int level)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2222, (gulong) line, (gulong) level);
}

int gtk_scintilla_get_fold_level(GtkScintilla *sci, int line)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2223, (gulong) line, 0);
}

int gtk_scintilla_get_last_child(GtkScintilla *sci, int line, int level)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2224, (gulong) line, (gulong) level);
}

int gtk_scintilla_get_fold_parent(GtkScintilla *sci, int line)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2225, (gulong) line, 0);
}

void gtk_scintilla_show_lines(GtkScintilla *sci, int line_start, int line_end)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2226, (gulong) line_start, (gulong) line_end);
}

void gtk_scintilla_hide_lines(GtkScintilla *sci, int line_start, int line_end)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2227, (gulong) line_start, (gulong) line_end);
}

gint gtk_scintilla_get_line_visible(GtkScintilla *sci, int line)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2228, (gulong) line, 0);
}

void gtk_scintilla_set_fold_expanded(GtkScintilla *sci, int line, gint expanded)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2229, (gulong) line, (gulong) expanded);
}

gint gtk_scintilla_get_fold_expanded(GtkScintilla *sci, int line)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2230, (gulong) line, 0);
}

void gtk_scintilla_toggle_fold(GtkScintilla *sci, int line)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2231, (gulong) line, 0);
}

void gtk_scintilla_ensure_visible(GtkScintilla *sci, int line)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2232, (gulong) line, 0);
}

void gtk_scintilla_set_fold_flags(GtkScintilla *sci, int flags)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2233, (gulong) flags, 0);
}

void gtk_scintilla_ensure_visible_enforce_policy(GtkScintilla *sci, int line)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2234, (gulong) line, 0);
}

void gtk_scintilla_set_tab_indents(GtkScintilla *sci, gint tab_indents)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2260, (gulong) tab_indents, 0);
}

gint gtk_scintilla_get_tab_indents(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2261, 0, 0);
}

void gtk_scintilla_set_backspace_unindents(GtkScintilla *sci, gint bs_unindents)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2262, (gulong) bs_unindents, 0);
}

gint gtk_scintilla_get_backspace_unindents(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2263, 0, 0);
}

void gtk_scintilla_set_mouse_dwell_time(GtkScintilla *sci, int period_milliseconds)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2264, (gulong) period_milliseconds, 0);
}

int gtk_scintilla_get_mouse_dwell_time(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2265, 0, 0);
}

int gtk_scintilla_word_start_position(GtkScintilla *sci, glong pos, gint only_word_characters)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2266, (gulong) pos, (gulong) only_word_characters);
}

int gtk_scintilla_word_end_position(GtkScintilla *sci, glong pos, gint only_word_characters)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2267, (gulong) pos, (gulong) only_word_characters);
}

void gtk_scintilla_set_wrap_mode(GtkScintilla *sci, int mode)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2268, (gulong) mode, 0);
}

int gtk_scintilla_get_wrap_mode(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2269, 0, 0);
}

void gtk_scintilla_set_wrap_visual_flags(GtkScintilla *sci, int wrap_visual_flags)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2460, (gulong) wrap_visual_flags, 0);
}

int gtk_scintilla_get_wrap_visual_flags(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2461, 0, 0);
}

void gtk_scintilla_set_wrap_visual_flags_location(GtkScintilla *sci, int wrap_visual_flags_location)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2462, (gulong) wrap_visual_flags_location, 0);
}

int gtk_scintilla_get_wrap_visual_flags_location(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2463, 0, 0);
}

void gtk_scintilla_set_wrap_start_indent(GtkScintilla *sci, int indent)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2464, (gulong) indent, 0);
}

int gtk_scintilla_get_wrap_start_indent(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2465, 0, 0);
}

void gtk_scintilla_set_layout_cache(GtkScintilla *sci, int mode)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2272, (gulong) mode, 0);
}

int gtk_scintilla_get_layout_cache(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2273, 0, 0);
}

void gtk_scintilla_set_scroll_width(GtkScintilla *sci, int pixel_width)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2274, (gulong) pixel_width, 0);
}

int gtk_scintilla_get_scroll_width(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2275, 0, 0);
}

int gtk_scintilla_text_width(GtkScintilla *sci, int style, const gchar * text)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2276, (gulong) style, (gulong) text);
}

void gtk_scintilla_set_end_at_last_line(GtkScintilla *sci, gint end_at_last_line)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2277, (gulong) end_at_last_line, 0);
}

gint gtk_scintilla_get_end_at_last_line(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2278, 0, 0);
}

int gtk_scintilla_text_height(GtkScintilla *sci, int line)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2279, (gulong) line, 0);
}

void gtk_scintilla_set_v_scroll_bar(GtkScintilla *sci, gint show)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2280, (gulong) show, 0);
}

gint gtk_scintilla_get_v_scroll_bar(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2281, 0, 0);
}

void gtk_scintilla_append_text(GtkScintilla *sci, int length, const gchar * text)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2282, (gulong) length, (gulong) text);
}

gint gtk_scintilla_get_two_phase_draw(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2283, 0, 0);
}

void gtk_scintilla_set_two_phase_draw(GtkScintilla *sci, gint two_phase)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2284, (gulong) two_phase, 0);
}

void gtk_scintilla_target_from_selection(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2287, 0, 0);
}

void gtk_scintilla_lines_join(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2288, 0, 0);
}

void gtk_scintilla_lines_split(GtkScintilla *sci, int pixel_width)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2289, (gulong) pixel_width, 0);
}

void gtk_scintilla_set_fold_margin_colour(GtkScintilla *sci, gint use_setting, glong back)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2290, (gulong) use_setting, (gulong) back);
}

void gtk_scintilla_set_fold_margin_hi_colour(GtkScintilla *sci, gint use_setting, glong fore)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2291, (gulong) use_setting, (gulong) fore);
}

void gtk_scintilla_line_down(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2300, 0, 0);
}

void gtk_scintilla_line_down_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2301, 0, 0);
}

void gtk_scintilla_line_up(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2302, 0, 0);
}

void gtk_scintilla_line_up_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2303, 0, 0);
}

void gtk_scintilla_char_left(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2304, 0, 0);
}

void gtk_scintilla_char_left_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2305, 0, 0);
}

void gtk_scintilla_char_right(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2306, 0, 0);
}

void gtk_scintilla_char_right_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2307, 0, 0);
}

void gtk_scintilla_word_left(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2308, 0, 0);
}

void gtk_scintilla_word_left_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2309, 0, 0);
}

void gtk_scintilla_word_right(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2310, 0, 0);
}

void gtk_scintilla_word_right_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2311, 0, 0);
}

void gtk_scintilla_home(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2312, 0, 0);
}

void gtk_scintilla_home_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2313, 0, 0);
}

void gtk_scintilla_line_end(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2314, 0, 0);
}

void gtk_scintilla_line_end_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2315, 0, 0);
}

void gtk_scintilla_document_start(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2316, 0, 0);
}

void gtk_scintilla_document_start_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2317, 0, 0);
}

void gtk_scintilla_document_end(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2318, 0, 0);
}

void gtk_scintilla_document_end_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2319, 0, 0);
}

void gtk_scintilla_page_up(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2320, 0, 0);
}

void gtk_scintilla_page_up_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2321, 0, 0);
}

void gtk_scintilla_page_down(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2322, 0, 0);
}

void gtk_scintilla_page_down_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2323, 0, 0);
}

void gtk_scintilla_edit_toggle_overtype(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2324, 0, 0);
}

void gtk_scintilla_cancel(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2325, 0, 0);
}

void gtk_scintilla_delete_back(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2326, 0, 0);
}

void gtk_scintilla_tab(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2327, 0, 0);
}

void gtk_scintilla_back_tab(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2328, 0, 0);
}

void gtk_scintilla_new_line(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2329, 0, 0);
}

void gtk_scintilla_form_feed(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2330, 0, 0);
}

void gtk_scintilla_v_c_home(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2331, 0, 0);
}

void gtk_scintilla_v_c_home_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2332, 0, 0);
}

void gtk_scintilla_zoom_in(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2333, 0, 0);
}

void gtk_scintilla_zoom_out(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2334, 0, 0);
}

void gtk_scintilla_del_word_left(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2335, 0, 0);
}

void gtk_scintilla_del_word_right(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2336, 0, 0);
}

void gtk_scintilla_line_cut(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2337, 0, 0);
}

void gtk_scintilla_line_delete(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2338, 0, 0);
}

void gtk_scintilla_line_transpose(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2339, 0, 0);
}

void gtk_scintilla_line_duplicate(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2404, 0, 0);
}

void gtk_scintilla_lower_case(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2340, 0, 0);
}

void gtk_scintilla_upper_case(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2341, 0, 0);
}

void gtk_scintilla_line_scroll_down(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2342, 0, 0);
}

void gtk_scintilla_line_scroll_up(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2343, 0, 0);
}

void gtk_scintilla_delete_back_not_line(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2344, 0, 0);
}

void gtk_scintilla_home_display(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2345, 0, 0);
}

void gtk_scintilla_home_display_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2346, 0, 0);
}

void gtk_scintilla_line_end_display(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2347, 0, 0);
}

void gtk_scintilla_line_end_display_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2348, 0, 0);
}

void gtk_scintilla_home_wrap(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2349, 0, 0);
}

void gtk_scintilla_home_wrap_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2450, 0, 0);
}

void gtk_scintilla_line_end_wrap(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2451, 0, 0);
}

void gtk_scintilla_line_end_wrap_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2452, 0, 0);
}

void gtk_scintilla_v_c_home_wrap(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2453, 0, 0);
}

void gtk_scintilla_v_c_home_wrap_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2454, 0, 0);
}

void gtk_scintilla_line_copy(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2455, 0, 0);
}

void gtk_scintilla_move_caret_inside_view(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2401, 0, 0);
}

int gtk_scintilla_line_length(GtkScintilla *sci, int line)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2350, (gulong) line, 0);
}

void gtk_scintilla_brace_highlight(GtkScintilla *sci, glong pos1, glong pos2)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2351, (gulong) pos1, (gulong) pos2);
}

void gtk_scintilla_brace_bad_light(GtkScintilla *sci, glong pos)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2352, (gulong) pos, 0);
}

glong gtk_scintilla_brace_match(GtkScintilla *sci, glong pos)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2353, (gulong) pos, 0);
}

gint gtk_scintilla_get_view_eol(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2355, 0, 0);
}

void gtk_scintilla_set_view_eol(GtkScintilla *sci, gint visible)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2356, (gulong) visible, 0);
}

int gtk_scintilla_get_doc_pointer(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2357, 0, 0);
}

void gtk_scintilla_set_doc_pointer(GtkScintilla *sci, int pointer)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2358, 0, (gulong) pointer);
}

void gtk_scintilla_set_mod_event_mask(GtkScintilla *sci, int mask)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2359, (gulong) mask, 0);
}

int gtk_scintilla_get_edge_column(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2360, 0, 0);
}

void gtk_scintilla_set_edge_column(GtkScintilla *sci, int column)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2361, (gulong) column, 0);
}

int gtk_scintilla_get_edge_mode(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2362, 0, 0);
}

void gtk_scintilla_set_edge_mode(GtkScintilla *sci, int mode)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2363, (gulong) mode, 0);
}

glong gtk_scintilla_get_edge_colour(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2364, 0, 0);
}

void gtk_scintilla_set_edge_colour(GtkScintilla *sci, glong edge_colour)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2365, (gulong) edge_colour, 0);
}

void gtk_scintilla_search_anchor(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2366, 0, 0);
}

int gtk_scintilla_search_next(GtkScintilla *sci, int flags, const gchar * text)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2367, (gulong) flags, (gulong) text);
}

int gtk_scintilla_search_prev(GtkScintilla *sci, int flags, const gchar * text)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2368, (gulong) flags, (gulong) text);
}

int gtk_scintilla_lines_on_screen(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2370, 0, 0);
}

void gtk_scintilla_use_pop_up(GtkScintilla *sci, gint allow_pop_up)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2371, (gulong) allow_pop_up, 0);
}

gint gtk_scintilla_selection_is_rectangle(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2372, 0, 0);
}

void gtk_scintilla_set_zoom(GtkScintilla *sci, int zoom)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2373, (gulong) zoom, 0);
}

int gtk_scintilla_get_zoom(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2374, 0, 0);
}

int gtk_scintilla_create_document(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2375, 0, 0);
}

void gtk_scintilla_add_ref_document(GtkScintilla *sci, int doc)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2376, 0, (gulong) doc);
}

void gtk_scintilla_release_document(GtkScintilla *sci, int doc)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2377, 0, (gulong) doc);
}

int gtk_scintilla_get_mod_event_mask(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2378, 0, 0);
}

void gtk_scintilla_set_focus(GtkScintilla *sci, gint focus)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2380, (gulong) focus, 0);
}

gint gtk_scintilla_get_focus(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2381, 0, 0);
}

void gtk_scintilla_set_status(GtkScintilla *sci, int status_code)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2382, (gulong) status_code, 0);
}

int gtk_scintilla_get_status(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2383, 0, 0);
}

void gtk_scintilla_set_mouse_down_captures(GtkScintilla *sci, gint captures)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2384, (gulong) captures, 0);
}

gint gtk_scintilla_get_mouse_down_captures(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2385, 0, 0);
}

void gtk_scintilla_set_cursor(GtkScintilla *sci, int cursor_type)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2386, (gulong) cursor_type, 0);
}

int gtk_scintilla_get_cursor(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2387, 0, 0);
}

void gtk_scintilla_set_control_char_symbol(GtkScintilla *sci, int symbol)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2388, (gulong) symbol, 0);
}

int gtk_scintilla_get_control_char_symbol(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2389, 0, 0);
}

void gtk_scintilla_word_part_left(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2390, 0, 0);
}

void gtk_scintilla_word_part_left_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2391, 0, 0);
}

void gtk_scintilla_word_part_right(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2392, 0, 0);
}

void gtk_scintilla_word_part_right_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2393, 0, 0);
}

void gtk_scintilla_set_visible_policy(GtkScintilla *sci, int visible_policy, int visible_slop)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2394, (gulong) visible_policy, (gulong) visible_slop);
}

void gtk_scintilla_del_line_left(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2395, 0, 0);
}

void gtk_scintilla_del_line_right(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2396, 0, 0);
}

void gtk_scintilla_set_x_offset(GtkScintilla *sci, int new_offset)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2397, (gulong) new_offset, 0);
}

int gtk_scintilla_get_x_offset(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2398, 0, 0);
}

void gtk_scintilla_choose_caret_x(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2399, 0, 0);
}

void gtk_scintilla_grab_focus(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2400, 0, 0);
}

void gtk_scintilla_set_x_caret_policy(GtkScintilla *sci, int caret_policy, int caret_slop)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2402, (gulong) caret_policy, (gulong) caret_slop);
}

void gtk_scintilla_set_y_caret_policy(GtkScintilla *sci, int caret_policy, int caret_slop)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2403, (gulong) caret_policy, (gulong) caret_slop);
}

void gtk_scintilla_set_print_wrap_mode(GtkScintilla *sci, int mode)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2406, (gulong) mode, 0);
}

int gtk_scintilla_get_print_wrap_mode(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2407, 0, 0);
}

void gtk_scintilla_set_hotspot_active_fore(GtkScintilla *sci, gint use_setting, glong fore)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2410, (gulong) use_setting, (gulong) fore);
}

void gtk_scintilla_set_hotspot_active_back(GtkScintilla *sci, gint use_setting, glong back)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2411, (gulong) use_setting, (gulong) back);
}

void gtk_scintilla_set_hotspot_active_underline(GtkScintilla *sci, gint underline)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2412, (gulong) underline, 0);
}

void gtk_scintilla_set_hotspot_single_line(GtkScintilla *sci, gint single_line)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2421, (gulong) single_line, 0);
}

void gtk_scintilla_para_down(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2413, 0, 0);
}

void gtk_scintilla_para_down_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2414, 0, 0);
}

void gtk_scintilla_para_up(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2415, 0, 0);
}

void gtk_scintilla_para_up_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2416, 0, 0);
}

glong gtk_scintilla_position_before(GtkScintilla *sci, glong pos)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2417, (gulong) pos, 0);
}

glong gtk_scintilla_position_after(GtkScintilla *sci, glong pos)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2418, (gulong) pos, 0);
}

void gtk_scintilla_copy_range(GtkScintilla *sci, glong start, glong end)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2419, (gulong) start, (gulong) end);
}

void gtk_scintilla_copy_text(GtkScintilla *sci, int length, const gchar * text)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2420, (gulong) length, (gulong) text);
}

void gtk_scintilla_set_selection_mode(GtkScintilla *sci, int mode)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2422, (gulong) mode, 0);
}

int gtk_scintilla_get_selection_mode(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2423, 0, 0);
}

glong gtk_scintilla_get_line_sel_start_position(GtkScintilla *sci, int line)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2424, (gulong) line, 0);
}

glong gtk_scintilla_get_line_sel_end_position(GtkScintilla *sci, int line)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2425, (gulong) line, 0);
}

void gtk_scintilla_line_down_rect_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2426, 0, 0);
}

void gtk_scintilla_line_up_rect_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2427, 0, 0);
}

void gtk_scintilla_char_left_rect_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2428, 0, 0);
}

void gtk_scintilla_char_right_rect_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2429, 0, 0);
}

void gtk_scintilla_home_rect_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2430, 0, 0);
}

void gtk_scintilla_v_c_home_rect_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2431, 0, 0);
}

void gtk_scintilla_line_end_rect_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2432, 0, 0);
}

void gtk_scintilla_page_up_rect_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2433, 0, 0);
}

void gtk_scintilla_page_down_rect_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2434, 0, 0);
}

void gtk_scintilla_stuttered_page_up(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2435, 0, 0);
}

void gtk_scintilla_stuttered_page_up_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2436, 0, 0);
}

void gtk_scintilla_stuttered_page_down(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2437, 0, 0);
}

void gtk_scintilla_stuttered_page_down_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2438, 0, 0);
}

void gtk_scintilla_word_left_end(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2439, 0, 0);
}

void gtk_scintilla_word_left_end_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2440, 0, 0);
}

void gtk_scintilla_word_right_end(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2441, 0, 0);
}

void gtk_scintilla_word_right_end_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2442, 0, 0);
}

void gtk_scintilla_set_whitespace_chars(GtkScintilla *sci, const gchar * characters)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2443, 0, (gulong) characters);
}

void gtk_scintilla_set_chars_default(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2444, 0, 0);
}

int gtk_scintilla_autoc_get_current(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2445, 0, 0);
}

void gtk_scintilla_allocate(GtkScintilla *sci, int bytes)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2446, (gulong) bytes, 0);
}

int gtk_scintilla_target_as_u_t_f8(GtkScintilla *sci, gchar * s)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2447, 0, (gulong) s);
}

void gtk_scintilla_set_length_for_encode(GtkScintilla *sci, int bytes)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2448, (gulong) bytes, 0);
}

int gtk_scintilla_encoded_from_u_t_f8(GtkScintilla *sci, const gchar * utf8, gchar * encoded)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2449, (gulong) utf8, (gulong) encoded);
}

int gtk_scintilla_find_column(GtkScintilla *sci, int line, int column)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2456, (gulong) line, (gulong) column);
}

gint gtk_scintilla_get_caret_sticky(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2457, 0, 0);
}

void gtk_scintilla_set_caret_sticky(GtkScintilla *sci, gint use_caret_sticky_behaviour)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2458, (gulong) use_caret_sticky_behaviour, 0);
}

void gtk_scintilla_toggle_caret_sticky(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2459, 0, 0);
}

void gtk_scintilla_set_paste_convert_endings(GtkScintilla *sci, gint convert)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2467, (gulong) convert, 0);
}

gint gtk_scintilla_get_paste_convert_endings(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2468, 0, 0);
}

void gtk_scintilla_selection_duplicate(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2469, 0, 0);
}

void gtk_scintilla_set_caret_line_back_alpha(GtkScintilla *sci, int alpha)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2470, (gulong) alpha, 0);
}

int gtk_scintilla_get_caret_line_back_alpha(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2471, 0, 0);
}

void gtk_scintilla_start_record(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        3001, 0, 0);
}

void gtk_scintilla_stop_record(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        3002, 0, 0);
}

void gtk_scintilla_set_lexer(GtkScintilla *sci, int lexer)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        4001, (gulong) lexer, 0);
}

int gtk_scintilla_get_lexer(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        4002, 0, 0);
}

void gtk_scintilla_colourise(GtkScintilla *sci, glong start, glong end)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        4003, (gulong) start, (gulong) end);
}

void gtk_scintilla_set_property(GtkScintilla *sci, const gchar * key, const gchar * value)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        4004, (gulong) key, (gulong) value);
}

void gtk_scintilla_set_keywords(GtkScintilla *sci, int keyword_set, const gchar * key_words)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        4005, (gulong) keyword_set, (gulong) key_words);
}

void gtk_scintilla_set_lexer_language(GtkScintilla *sci, const gchar * language)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        4006, 0, (gulong) language);
}

void gtk_scintilla_load_lexer_library(GtkScintilla *sci, const gchar * path)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        4007, 0, (gulong) path);
}

int gtk_scintilla_get_property(GtkScintilla *sci, const gchar * key, gchar * buf)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        4008, (gulong) key, (gulong) buf);
}

int gtk_scintilla_get_property_expanded(GtkScintilla *sci, const gchar * key, gchar * buf)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        4009, (gulong) key, (gulong) buf);
}

int gtk_scintilla_get_property_int(GtkScintilla *sci, const gchar * key)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        4010, (gulong) key, 0);
}

int gtk_scintilla_get_style_bits_needed(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        4011, 0, 0);
}


/* --- End of autogenerated code --- */

/* === Support functions =================================================== */
static void
notify_cb (GtkWidget *w, gint param, gpointer notif, gpointer data)
{
    struct SCNotification *notification = (struct SCNotification *) notif;
    
    switch (notification->nmhdr.code) {
        case SCN_STYLENEEDED:
            g_signal_emit (G_OBJECT (data),
                           signals[STYLE_NEEDED], 0,
                           (gint) notification->position);
            break;
        case SCN_UPDATEUI:
            g_signal_emit (G_OBJECT (data),
                           signals[UPDATE_UI], 0);
            break;
        case SCN_CHARADDED:
            g_signal_emit (G_OBJECT (data),
                           signals[CHAR_ADDED], 0,
                           (gint) notification->ch);
            break;
        case SCN_SAVEPOINTREACHED:
            g_signal_emit (G_OBJECT (data),
                           signals[SAVE_POINT_REACHED], 0);
            break;
        case SCN_SAVEPOINTLEFT:
            g_signal_emit (G_OBJECT (data),
                           signals[SAVE_POINT_LEFT], 0);
            break;
        case SCN_MODIFYATTEMPTRO:
            g_signal_emit (G_OBJECT (data),
                           signals[MODIFY_ATTEMPT_RO], 0);
            break;
        case SCN_KEY:
            pass_throug_key (GTK_SCINTILLA (data),
                             (gint) notification->ch,
                             (gint) notification->modifiers);
            g_signal_emit (G_OBJECT (data),
                           signals[KEY], 0,
                           (gint) notification->ch,
                           (gint) notification->modifiers);
            break;
        case SCN_DOUBLECLICK:
            g_signal_emit (G_OBJECT (data),
                           signals[DOUBLE_CLICK], 0);
            break;
        case SCN_MODIFIED:
            g_signal_emit (G_OBJECT (data),
                           signals[MODIFIED], 0,
                           (gint) notification->position,
                           (gint) notification->modificationType,
                           (gchar *)notification->text,
                           (gint) notification->length,
                           (gint) notification->linesAdded,
                           (gint) notification->line,
                           (gint) notification->foldLevelNow,
                           (gint) notification->foldLevelPrev);
            break;
        case SCN_MACRORECORD:
            g_signal_emit (G_OBJECT (data),
                           signals[MACRO_RECORD], 0,
                           (gint) notification->message,
                           (gulong) notification->wParam,
                           (glong) notification->lParam);
            break;
        case SCN_MARGINCLICK:
            g_signal_emit (G_OBJECT (data),
                           signals[MARGIN_CLICK], 0,
                           (gint) notification->modifiers,
                           (gint) notification->position,
                           (gint) notification->margin);
            break;
        case SCN_NEEDSHOWN:
            g_signal_emit (G_OBJECT (data),
                           signals[NEED_SHOWN], 0,
                           (gint) notification->position,
                           (gint) notification->length);
            break;
        case SCN_PAINTED:
            g_signal_emit (G_OBJECT (data),
                           signals[PAINTED], 0);
            break;
        case SCN_USERLISTSELECTION:
            g_signal_emit (G_OBJECT (data),
                           signals[USER_LIST_SELECTION], 0,
                           (gint) notification->listType,
                           (gchar *) notification->text);
            break;
        case SCN_URIDROPPED:
            g_signal_emit (G_OBJECT (data),
                           signals[URI_DROPPED], 0,
                           (gchar *) notification->text);
            break;
        case SCN_DWELLSTART:
            g_signal_emit (G_OBJECT (data),
                           signals[DWELL_START], 0,
                           (gint) notification->position);
            break;
        case SCN_DWELLEND:
            g_signal_emit (G_OBJECT (data),
                           signals[DWELL_END], 0,
                           (gint) notification->position);
            break;
        case SCN_ZOOM:
            g_signal_emit (G_OBJECT (data),
                           signals[ZOOM], 0);
            break;
        default:
            //g_warning ("GtkScintilla2: Notification code %d not handled!\n",
            //           (gint) notification->nmhdr.code);
            break;
    }
}

void
pass_throug_key (GtkScintilla *sci, gint ch, gint modifiers)
{
    gint mods = 0;
    
    if (modifiers & SCMOD_SHIFT)
        mods |= GDK_SHIFT_MASK;
    if (modifiers & SCMOD_CTRL)
        mods |= GDK_CONTROL_MASK;
    if (modifiers & SCMOD_ALT)
        mods |= GDK_MOD1_MASK;
    
    if (sci->accel_group) {
        gtk_accel_groups_activate (G_OBJECT (sci->accel_group),
                                   ch, (GdkModifierType) mods);
    }
}
