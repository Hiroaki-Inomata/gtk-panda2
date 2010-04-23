/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
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
#include <sys/types.h>
#include <unistd.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#define MOZILLA_CLIENT

#include <nsCOMPtr.h>
#include <nsIPrefService.h>
#define MOZILLA_INTERNAL_API
#include <nsIServiceManager.h>
#undef MOZILLA_INTERNAL_API

#include "gtkpandaintl.h"
#include "gtkpandahtml.h"

enum
{   
  ACTIVATE,
  LAST_SIGNAL
};

static GtkMozEmbedClass *parent_class = NULL;
static guint signals [LAST_SIGNAL] = { 0 };

static void gtk_panda_html_class_init(GtkPandaHTMLClass *klass);
static void gtk_panda_html_init(GtkPandaHTML     *html);
static void gtk_panda_html_set_proxy(void);
static gboolean gtk_panda_html_open_uri(GtkMozEmbed *embed,
  const char *uri, gpointer data);
static void gtk_panda_html_new_window(GtkMozEmbed *embed,
  GtkMozEmbed **retval, guint chromemask, gpointer data);
#if 0
static void gtk_panda_html_reload(GtkWidget *widget,
  GdkEventExpose *event, gpointer user_data);
#endif

GType
gtk_panda_html_get_type (void)
{
  static GType type = 0;

  if (!type)
    {
      static const GTypeInfo info =
      {
        sizeof (GtkPandaHTMLClass),
        NULL, /* base_init */
        NULL, /* base_finalize */
        (GClassInitFunc) gtk_panda_html_class_init,
        NULL, /* class_finalize */
        NULL, /* class_data */
        sizeof (GtkPandaHTML),
        0,
        (GInstanceInitFunc) gtk_panda_html_init
      };

      type = g_type_register_static( GTK_TYPE_MOZ_EMBED,
                                     "GtkPandaHTML",
                                     &info,
                                     (GTypeFlags)0);
    }

  return type;
}

static void 
gtk_panda_html_class_init (GtkPandaHTMLClass *klass)
{
  GObjectClass *object_class;
  GtkObjectClass *gtk_object_class;
  GtkWidgetClass *widget_class;

  object_class = G_OBJECT_CLASS(klass);
  gtk_object_class = (GtkObjectClass*) klass;
  widget_class = (GtkWidgetClass*) klass;
  parent_class = (GtkMozEmbedClass *)gtk_type_class (GTK_TYPE_MOZ_EMBED);

  signals[ACTIVATE] =
  g_signal_new ("activate",
        G_TYPE_FROM_CLASS (object_class),
        G_SIGNAL_RUN_LAST,
        G_STRUCT_OFFSET (GtkPandaHTMLClass, activate),
        NULL, NULL,
        gtk_marshal_VOID__POINTER,
        G_TYPE_NONE, 1,
        G_TYPE_POINTER);
}

static void
gtk_panda_html_init (GtkPandaHTML *html)
{
//  gtk_panda_html_set_proxy();
  g_signal_connect(GTK_MOZ_EMBED(html), "open-uri",
    G_CALLBACK(gtk_panda_html_open_uri), NULL);
  g_signal_connect(GTK_MOZ_EMBED(html), "new-window",
    G_CALLBACK(gtk_panda_html_new_window), NULL);
#if 0
  g_signal_connect(GTK_WIDGET(html), "expose-event",
    G_CALLBACK(gtk_panda_html_reload), NULL);
#endif
}

GtkWidget*
gtk_panda_html_new (void)
{
  return GTK_WIDGET (g_object_new (GTK_PANDA_TYPE_HTML, NULL));
}

#if 0
gboolean
mozilla_prefs_set_string(const char *preference_name, 
  const char *new_value)
{
	g_return_val_if_fail(preference_name != NULL, FALSE);
	if (!new_value) return FALSE;

	nsCOMPtr<nsIPrefService> prefService = 
				do_GetService(NS_PREFSERVICE_CONTRACTID);
	nsCOMPtr<nsIPrefBranch> pref;
	prefService->GetBranch("", getter_AddRefs(pref));

	if (pref)
	{
		nsresult rv = pref->SetCharPref(preference_name, new_value);
		return NS_SUCCEEDED(rv) ? TRUE : FALSE;
	}
	return FALSE;
}


gboolean
mozilla_prefs_set_int (const char *preference_name, 
  int new_int_value)
{
	g_return_val_if_fail(preference_name != NULL, FALSE);

	nsCOMPtr<nsIPrefService> prefService = 
				do_GetService(NS_PREFSERVICE_CONTRACTID);
	nsCOMPtr<nsIPrefBranch> pref;
	prefService->GetBranch("", getter_AddRefs(pref));

	if (pref)
	{
		nsresult rv = pref->SetIntPref(preference_name, new_int_value);
		return NS_SUCCEEDED(rv) ? TRUE : FALSE;
	}

	return FALSE;
}


void
mozilla_prefs_set_use_proxy (gboolean use)
{
	if (use)
		mozilla_prefs_set_int("network.proxy.type", 1);
	else
		mozilla_prefs_set_int("network.proxy.type", 0);
}

static void
gtk_panda_html_set_proxy(void)
{
  char *env;
  char *head;
  char *tail;
  char *p;
  char *host;
  char *port;

  if ((env = head = getenv("http_proxy")) != NULL) {
    tail = strstr(head, "://");
    if (tail == NULL) return;
    head = tail + 3;
    tail = strstr(head, ":");
    if (tail == NULL) {
      tail = strstr(head, "/");
      if (tail != NULL) *tail = '\0';
      host = strdup(head);
      port = strdup("80");
    } else {
      host = strndup(head, tail - head);
      head = tail + 1;
      port = strdup(head);
    }
    mozilla_prefs_set_int("network.proxy.type", 1);
    mozilla_prefs_set_int("network.proxy.http_port", atoi(port));
    mozilla_prefs_set_string("network.proxy.http", host);
    free(host);
    free(port);
  }
}
#endif

static gboolean
gtk_panda_html_open_uri(GtkMozEmbed *embed,
  const char *uri,
  gpointer data)
{
  gtk_moz_embed_set_chrome_mask(embed, 
    GTK_MOZ_EMBED_FLAG_ALLCHROME
    );
  return FALSE;
}

static void
gtk_panda_html_new_window(GtkMozEmbed *embed,
  GtkMozEmbed **retval,
  guint chromemask,
  gpointer data)
{
  char *argv[3];
  int pid;
  extern char **environ;

  *retval = NULL;
  if ((pid = fork()) == 0) {
    if (strlen(OPEN_BROWSER_COMMAND) > 0) {
      argv[0] = (char *)OPEN_BROWSER_COMMAND;
      argv[1] = gtk_moz_embed_get_link_message(embed);
      argv[2] = NULL;
      execve(argv[0], argv, environ);
    } else {
      g_warning("cannot open external browser.");
      exit(0);
    }
  } else if (pid < 0) {
    g_warning("fork failed");
  } 
  return;
}

#if 0
gtk_panda_html_reload(GtkWidget *widget,
  GdkEventExpose *event,
  gpointer user_data)
{
  gtk_moz_embed_reload(GTK_MOZ_EMBED(widget), GTK_MOZ_EMBED_FLAG_RELOADNORMAL);
  fprintf(stderr,"reloaded %s\n", gtk_moz_embed_get_location(GTK_MOZ_EMBED(widget)));
  //gtk_widget_show_all(widget);
  return;
}
#endif

// public API

char *
gtk_panda_html_get_uri (GtkPandaHTML *html)
{
  return gtk_moz_embed_get_location(GTK_MOZ_EMBED(html));
}

void
gtk_panda_html_set_uri (GtkPandaHTML *html, const char *uri)
{
  gtk_moz_embed_load_url(GTK_MOZ_EMBED(html), uri);
}
