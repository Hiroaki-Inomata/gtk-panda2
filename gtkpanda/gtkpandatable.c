 /* 
 * Copyright 2011 NaCl
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
 * Boston, MA 02111-1307, USA.
 *
 * Based on gtk/gtktreeview.c
 */

/*
 * Modified by the GTK+ Team and others 1997-1999.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib-object.h>
#include <gtk/gtk.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <gdk/gdkkeysyms.h>
#include "gtkpandaintl.h"
#include "gtkpandatable.h"

enum {
PROP_0,
PROP_COLUMNS,
PROP_TYPES,
PROP_TITLES
};

enum
{
  CELL_EDITED,
  LAST_SIGNAL 
}; 

static GtkTreeViewClass *parent_class = NULL;
static guint signals [LAST_SIGNAL] = { 0 };

static void gtk_panda_table_class_init (GtkPandaTableClass *klass);
static void gtk_panda_table_init (GtkPandaTable *table);

static void  gtk_panda_table_set_property       (GObject         *object,
                       guint            prop_id,
                       const GValue    *value,
                       GParamSpec      *pspec);
static void  gtk_panda_table_get_property       (GObject         *object,
                       guint            prop_id,
                       GValue          *value,
                       GParamSpec      *pspec);

static void
gtk_panda_table_class_init ( GtkPandaTableClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GtkObjectClass *gtk_object_class;
  GtkWidgetClass *widget_class;

  gtk_object_class = (GtkObjectClass *) klass;
  widget_class = (GtkWidgetClass *) klass;

  parent_class = gtk_type_class (GTK_TYPE_TREE_VIEW);

  signals[CELL_EDITED] =
  g_signal_new ("cell-edited",
        G_TYPE_FROM_CLASS (gobject_class),
        G_SIGNAL_RUN_FIRST,
        G_STRUCT_OFFSET (GtkPandaTableClass, cell_edited),
        NULL, NULL,
        gtk_marshal_VOID__INT_INT_POINTER,
        G_TYPE_NONE, 3,
        G_TYPE_INT,
        G_TYPE_INT,
        G_TYPE_STRING);

  gobject_class->set_property = gtk_panda_table_set_property; 
  gobject_class->get_property = gtk_panda_table_get_property; 

  g_object_class_install_property (gobject_class,
    PROP_COLUMNS,
    g_param_spec_int ("columns",
      _("Number of columns"),
      _("Number of columns"),
      0,
      99,
      0,
      G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
    PROP_TYPES,
    g_param_spec_string ("column_types",
      _("The list of column type"),
      _("The list of column type(comma separated string)"),
      "text,text,text,text,text,text,text,text,text,text",
      G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
    PROP_TITLES,
    g_param_spec_string ("column_titles",
      _("The list of column titles"),
      _("The list of column titles(comma separated string)"),
      "",
      G_PARAM_READWRITE));
}

static void
gtk_panda_table_init ( GtkPandaTable * table)
{
  table->types = g_strdup("");
  table->titles = g_strdup("");
  gtk_panda_table_set_columns(table,1);
  gtk_tree_view_set_enable_search(GTK_TREE_VIEW(table), FALSE );
  gtk_tree_view_set_rubber_banding(GTK_TREE_VIEW(table), FALSE );
  GTK_WIDGET_SET_FLAGS(GTK_WIDGET(table), GTK_CAN_FOCUS);
}

GType
gtk_panda_table_get_type (void)
{
  static GType table_type = 0;

  if (!table_type)
    {
      static const GTypeInfo table_info =
      {
        sizeof (GtkPandaTableClass),
        NULL, /* base_init */
        NULL, /* base_finalize */
        (GClassInitFunc) gtk_panda_table_class_init,           
        NULL, /* class_finalize */
        NULL, /* class_data */
        sizeof (GtkPandaTable),
        0,
        (GInstanceInitFunc) gtk_panda_table_init
      };

      table_type = g_type_register_static( GTK_TYPE_TREE_VIEW,
                                           "GtkPandaTable",
                                           &table_info,
                                           0);
    }
  return table_type;
}

GtkWidget *
gtk_panda_table_new ()
{
  return (GtkWidget*)g_object_new(GTK_PANDA_TYPE_TABLE, NULL);
}


static void
parse_prop_types(GtkPandaTable *table)
{
  int i;
  gchar **splits;

  for (i = 0; i < GTK_PANDA_TABLE_MAX_COLS; i++) {
    table->renderer_types[i] = GTK_PANDA_TABLE_RENDERER_TEXT;
    table->model_types[i] = G_TYPE_STRING;
  }
  table->color_column = -1;

  if (table->titles == NULL) {
    return;
  }

  splits = g_strsplit(table->types,",",GTK_PANDA_TABLE_MAX_COLS);
  for(i = 0; splits[i] != NULL; i++) {
    if (!strcmp(splits[i], "text")) {
      table->renderer_types[i] = GTK_PANDA_TABLE_RENDERER_TEXT;
      table->model_types[i] = G_TYPE_STRING;
    } else if (!strcmp(splits[i], "label")) {
      table->renderer_types[i] = GTK_PANDA_TABLE_RENDERER_LABEL;
      table->model_types[i] = G_TYPE_STRING;
    } else if (!strcmp(splits[i], "icon")) {
      table->renderer_types[i] = GTK_PANDA_TABLE_RENDERER_ICON;
      table->model_types[i] = G_TYPE_STRING;
    } else if (!strcmp(splits[i], "check")) {
      table->renderer_types[i] = GTK_PANDA_TABLE_RENDERER_CHECK;
      table->model_types[i] = G_TYPE_BOOLEAN;
    } else if (!strcmp(splits[i], "color")) {
      table->renderer_types[i] = GTK_PANDA_TABLE_RENDERER_COLOR;
      if (i < table->columns) {
        table->color_column = i;
      }
    } else {
      table->renderer_types[i] = GTK_PANDA_TABLE_RENDERER_TEXT;
      table->model_types[i] = G_TYPE_STRING;
    }
  }
  g_strfreev(splits);
}

static void
cb_text_renderer_edited(GtkCellRendererText *renderer,
	gchar *path,
	gchar *new_text,
	gpointer user_data)
{
  GtkPandaTable *table = GTK_PANDA_TABLE(user_data);
  guint row,column;
  gpointer data = g_object_get_data(G_OBJECT(renderer),"column_num");
  
  if (data == NULL) { return; }
  column = (guint)(data);
  row = atoi(path);
  
  g_signal_emit(table, signals[CELL_EDITED],0,row,column,new_text);
}

static void
cb_toggle_renderer_toggled(GtkCellRendererToggle *renderer,
	gchar *path,
	gpointer user_data)
{
  GtkPandaTable *table = GTK_PANDA_TABLE(user_data);
  guint row,column;
  gpointer data = g_object_get_data(G_OBJECT(renderer),"column_num");
  
  if (data == NULL) { return; }
  column = (guint)(data);
  row = atoi(path);

  g_signal_emit(table, signals[CELL_EDITED],0,row,column,
    !gtk_cell_renderer_toggle_get_active(renderer)?"T":"F");
}

static void
apply_prop_types(GtkPandaTable *table)
{
  int i;
  GtkTreeViewColumn *col;
  GtkCellRenderer *renderer;

/* FIXFIX */

  table->color_column = -1;
  for(i = 0; i < table->columns; i++) {
    if (table->renderer_types[i] == GTK_PANDA_TABLE_RENDERER_COLOR) {
      table->color_column = i;
    }
  }

  for(i = 0; i < table->columns; i++) {
    if (table->renderer_types[i] != GTK_PANDA_TABLE_RENDERER_COLOR) {
      col = gtk_tree_view_column_new();
      gtk_tree_view_column_set_title(col, "");
      gtk_tree_view_append_column(GTK_TREE_VIEW(table), col);

      switch(table->renderer_types[i]) {
      case GTK_PANDA_TABLE_RENDERER_TEXT:
        renderer = gtk_cell_renderer_text_new();
        gtk_tree_view_column_pack_start(col, renderer, TRUE);
        g_object_set(G_OBJECT(renderer),
          "editable",TRUE,
          NULL);
        gtk_tree_view_column_set_attributes(col, renderer, 
          "text",i,
          NULL);
	    g_object_set_data(G_OBJECT(renderer),
	  	"column_num", GUINT_TO_POINTER(i));
        g_signal_connect(G_OBJECT(renderer),"edited",
          G_CALLBACK(cb_text_renderer_edited),table);
        break;
      case GTK_PANDA_TABLE_RENDERER_LABEL:
        renderer = gtk_cell_renderer_text_new();
        gtk_tree_view_column_pack_start(col, renderer, TRUE);
        g_object_set(G_OBJECT(renderer),
          "editable",FALSE,
          NULL);
        gtk_tree_view_column_set_attributes(col, renderer, 
          "text",i,
          NULL);
        break;
      case GTK_PANDA_TABLE_RENDERER_CHECK:
        renderer = gtk_cell_renderer_toggle_new();
        gtk_tree_view_column_pack_start(col, renderer, TRUE);
        g_object_set(G_OBJECT(renderer),
          "activatable",TRUE,
          NULL);
        gtk_tree_view_column_set_attributes(col, renderer, 
          "active", i,
          NULL);
	    g_object_set_data(G_OBJECT(renderer),
	  	"column_num", GUINT_TO_POINTER(i));
        g_signal_connect(G_OBJECT(renderer),"toggled",
          G_CALLBACK(cb_toggle_renderer_toggled),table);
        break;
      case GTK_PANDA_TABLE_RENDERER_ICON:
        renderer = gtk_cell_renderer_pixbuf_new();
        gtk_tree_view_column_pack_start(col, renderer, TRUE);
        gtk_tree_view_column_set_attributes(col, renderer, 
          "stock-id",i,
          NULL);
        break;
      }

      if (table->color_column != -1) {
        gtk_tree_view_column_add_attribute(col, renderer, 
          "cell-background",table->color_column);
      }
    }
  }
}

static void
apply_prop_titles(GtkPandaTable *table)
{
  int i, len;
  gchar **splits;
  GList *list;

  g_return_if_fail(table != NULL);

  if (table->titles == NULL) {
    return;
  }

  list = gtk_tree_view_get_columns(GTK_TREE_VIEW(table));
  len = g_list_length(list);
  for (i=0; i < len; i++) {
    gtk_tree_view_column_set_title(
      GTK_TREE_VIEW_COLUMN(g_list_nth_data(list,i)),"");
  }

  splits = g_strsplit(table->types,",",GTK_PANDA_TABLE_MAX_COLS);
  for(i = 0; splits[i] != NULL && i < len; i++) {
    gtk_tree_view_column_set_title(
      GTK_TREE_VIEW_COLUMN(g_list_nth_data(list,i)),splits[i]);
  }
  g_strfreev(splits);
  g_list_free(list);
}

void
gtk_panda_table_set_columns (
  GtkPandaTable *table,
  gint new_columns) 
{
  GtkListStore *store;
  GList *list;
  int i;

  g_return_if_fail(table != NULL);
  g_return_if_fail(new_columns >= 0);

  list = gtk_tree_view_get_columns(GTK_TREE_VIEW(table));
  for (i=0; i < g_list_length(list); i++) {
    gtk_tree_view_remove_column(GTK_TREE_VIEW(table),g_list_nth_data(list,i));
  }
  gtk_tree_view_set_model(GTK_TREE_VIEW(table), NULL);
  g_list_free(list);
  table->columns = new_columns;

  parse_prop_types(table);
  apply_prop_types(table);
  apply_prop_titles(table);

  store = gtk_list_store_newv(table->columns, table->model_types);
  gtk_tree_view_set_model(GTK_TREE_VIEW(table), GTK_TREE_MODEL(store));
}

void
gtk_panda_table_set_types(
  GtkPandaTable *table,
  const gchar *types)
{
  g_return_if_fail(table != NULL);
  g_return_if_fail(types != NULL);

  if (table->types != NULL) {
    g_free(table->types);
  }
  table->types = g_strdup(types);
  gtk_panda_table_set_columns(table,table->columns);
}

void
gtk_panda_table_set_titles(
  GtkPandaTable *table,
  const gchar *titles)
{
  g_return_if_fail(table != NULL);
  g_return_if_fail(titles != NULL);

  if (table->titles != NULL) {
    g_free(table->titles);
  }
  table->titles = g_strdup(titles);
  gtk_panda_table_set_columns(table,table->columns);
}

void 
gtk_panda_table_clear (GtkPandaTable *table)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  g_return_if_fail(table != NULL);
  g_return_if_fail(GTK_IS_PANDA_TABLE(table));

  model = gtk_tree_view_get_model(GTK_TREE_VIEW(table));
  if (gtk_tree_model_get_iter_first(model, &iter)) {
    gtk_list_store_clear(GTK_LIST_STORE(model));
  }
}

void
gtk_panda_table_append  (
  GtkPandaTable *table,
  gchar *data[])
{
  GtkListStore *store;
  GtkTreeIter iter;
  int i;
  GValue *value;
  
  g_return_if_fail(table != NULL);
  g_return_if_fail(GTK_IS_PANDA_TABLE(table));

  store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(table)));
  gtk_list_store_append (store, &iter);

  for(i = 0; i < table->columns; i++) {
    switch(table->renderer_types[i]) {
    case GTK_PANDA_TABLE_RENDERER_TEXT:
    case GTK_PANDA_TABLE_RENDERER_LABEL:
    case GTK_PANDA_TABLE_RENDERER_ICON:
    case GTK_PANDA_TABLE_RENDERER_COLOR:
	  value = g_new0(GValue, 1);
	  g_value_init(value, G_TYPE_STRING);
	  g_value_set_string(value, data[i]);
	  gtk_list_store_set_value(store, &iter, i, value);
      break;
    case GTK_PANDA_TABLE_RENDERER_CHECK:
	  value = g_new0(GValue, 1);
	  g_value_init(value, G_TYPE_BOOLEAN);
	  g_value_set_boolean(value, *data[i] == 'T' ? TRUE : FALSE);
	  gtk_list_store_set_value(store, &iter, i, value);
      break;
    }
  }
}

gint
gtk_panda_table_get_n_rows(
  GtkPandaTable *table)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  gint nrows;

  g_return_val_if_fail (table != NULL, 0);
  g_return_val_if_fail (GTK_IS_PANDA_TABLE (table), 0);

  nrows = 0; 
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(table));
  if (gtk_tree_model_get_iter_first(model, &iter)) {
    nrows = 1;
    while(gtk_tree_model_iter_next(model, &iter)) {
      nrows++;
    }
  }
  return nrows;
}

void 
gtk_panda_table_moveto (
  GtkPandaTable *table,
  gint      row,
  gint      column,
  gfloat    row_align,
  gfloat    col_align)
{
  GtkTreePath *path;
  GtkTreeViewColumn *col;

  g_return_if_fail (table != NULL);
  g_return_if_fail (GTK_IS_PANDA_TABLE (table));

  path = gtk_tree_path_new_from_indices(row, -1);
  col = gtk_tree_view_get_column(GTK_TREE_VIEW(table), column);

  gtk_tree_view_scroll_to_cell(
	GTK_TREE_VIEW(table), path, col, TRUE, row_align, col_align);
  gtk_tree_path_free(path);
}

static void 
gtk_panda_table_set_property (GObject *object,
  guint prop_id,
  const GValue *value,
  GParamSpec *pspec)
{
  GtkPandaTable *table;

  g_return_if_fail(GTK_IS_PANDA_TABLE(object));
  table = GTK_PANDA_TABLE (object);

  switch (prop_id)
    {
    case PROP_COLUMNS:
      gtk_panda_table_set_columns(table,g_value_get_int(value));
	  break;
    case PROP_TYPES:
      gtk_panda_table_set_types(table,g_value_get_string(value));
      break;
    case PROP_TITLES:
      gtk_panda_table_set_titles(table,g_value_get_string(value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void 
gtk_panda_table_get_property (GObject         *object,
				  guint            prop_id,
				  GValue          *value,
				  GParamSpec      *pspec)
{
  GtkPandaTable *table;

  g_return_if_fail(GTK_IS_PANDA_TABLE(object));
  table = GTK_PANDA_TABLE (object);

  switch (prop_id)
    {
    case PROP_COLUMNS:
      g_value_set_int (value, table->columns);
      break;
    case PROP_TYPES:
      g_value_set_string (value, g_strdup(table->types));
      break;
    case PROP_TITLES:
      g_value_set_string (value, g_strdup(table->titles));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}