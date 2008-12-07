//
// C++ Implementation: Control
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "Control.h"
#include "my_file.h"
#include "utils.h"
#include "baling.h"

 Control::Control():ipseg(NULL), palicon(NULL), myicon(NULL),
myname(NULL), encode(NULL), path(NULL), font(NULL), flags(0),
dirty(false), table(NULL), iconlist(NULL), pix(3.4)
{
	pthread_mutex_init(&mutex, NULL);
}

Control::~Control()
{
	pthread_mutex_lock(&mutex);
	g_slist_foreach(ipseg, remove_foreach, GINT_TO_POINTER(UNKNOWN));
	g_slist_free(ipseg);
	g_slist_foreach(iconlist, remove_foreach, GINT_TO_POINTER(UNKNOWN));
	g_slist_free(iconlist);
	pthread_mutex_unlock(&mutex);
	pthread_mutex_destroy(&mutex);

	free(palicon);
	free(myicon);
	free(myname);
	free(encode);
	free(path);
	free(font);

	g_object_unref(table);
}

void Control::InitSelf()
{
	ReadControl();
	CreateTagTable();
	GetSysIcon();
	GetRatio_PixMm();
}

void Control::WriteControl()
{
	GConfClient *client;

	client = gconf_client_get_default();
	gconf_client_set_list(client, GCONF_PATH "/scan_ip_section",
			      GCONF_VALUE_STRING, ipseg, NULL);
	gconf_client_set_string(client, GCONF_PATH "/pal_icon", palicon, NULL);
	gconf_client_set_string(client, GCONF_PATH "/self_icon", myicon, NULL);
	gconf_client_set_string(client, GCONF_PATH "/nick_name", myname, NULL);
	gconf_client_set_string(client, GCONF_PATH "/net_encode", encode, NULL);
	gconf_client_set_string(client, GCONF_PATH "/save_path", path, NULL);
	gconf_client_set_string(client, GCONF_PATH "/panel_font", font, NULL);
	gconf_client_set_bool(client, GCONF_PATH "/open_blacklist",
			      FLAG_ISSET(flags, 1) ? TRUE : FALSE, NULL);
	gconf_client_set_bool(client, GCONF_PATH "/proof_shared",
			      FLAG_ISSET(flags, 0) ? TRUE : FALSE, NULL);
	g_object_unref(client);

	dirty = false;
}

void Control::ReadControl()
{
	GConfClient *client;

	client = gconf_client_get_default();
	if (!
	    (ipseg =
	     gconf_client_get_list(client, GCONF_PATH "/scan_ip_section",
				   GCONF_VALUE_STRING, NULL))) {
		pthread_mutex_lock(&mutex);
		ipseg = g_slist_append(ipseg, Strdup("10.10.0.0"));
		ipseg = g_slist_append(ipseg, Strdup("10.10.3.255"));
		pthread_mutex_unlock(&mutex);
	}
	if (!
	    (palicon =
	     gconf_client_get_string(client, GCONF_PATH "/pal_icon", NULL)))
		palicon = Strdup(__ICON_DIR "/qq.png");
	if (!
	    (myicon =
	     gconf_client_get_string(client, GCONF_PATH "/self_icon", NULL)))
		myicon = Strdup(__ICON_DIR "/tux.png");
	if (!
	    (myname =
	     gconf_client_get_string(client, GCONF_PATH "/nick_name", NULL)))
		myname = Strdup(getenv("USER"));
	if (!
	    (encode =
	     gconf_client_get_string(client, GCONF_PATH "/net_encode", NULL)))
		encode = Strdup(_("UTF-8"));
	if (!
	    (path =
	     gconf_client_get_string(client, GCONF_PATH "/save_path", NULL)))
		path = Strdup(getenv("HOME"));
	if (!
	    (font =
	     gconf_client_get_string(client, GCONF_PATH "/panel_font", NULL)))
		font = Strdup("Sans Italic 10");
	if (gconf_client_get_bool(client, GCONF_PATH "/open_blacklist", NULL))
		FLAG_SET(flags, 1);
	if (gconf_client_get_bool(client, GCONF_PATH "/proof_shared", NULL))
		FLAG_SET(flags, 0);
	g_object_unref(client);

	dirty = true;
}

void Control::CreateTagTable()
{
	GtkTextTag *tag;

	table = gtk_text_tag_table_new();
	tag = gtk_text_tag_new("blue");
	g_object_set(tag, "foreground", "blue", NULL);
	gtk_text_tag_table_add(table, tag);
	tag = gtk_text_tag_new("green");
	g_object_set(tag, "foreground", "green", NULL);
	gtk_text_tag_table_add(table, tag);
	tag = gtk_text_tag_new("red");
	g_object_set(tag, "foreground", "red", NULL);
	gtk_text_tag_table_add(table, tag);
}

void Control::GetSysIcon()
{
	my_file mf(false);
	struct dirent *dirt;
	char path[MAX_PATHBUF];
	DIR *dir;

	mf.chdir(__ICON_DIR);
	dir = mf.opendir();
	while (dirt = readdir(dir)) {
		if (strcmp(dirt->d_name, ".") == 0
		    || strcmp(dirt->d_name, "..") == 0)
			continue;
		snprintf(path, MAX_PATHBUF, __ICON_DIR "/%s", dirt->d_name);
		iconlist = g_slist_append(iconlist, Strdup(path));
	}
	closedir(dir);
}

void Control::GetRatio_PixMm()
{
	GdkScreen *screen;
	gint width, widthmm;

	screen = gdk_screen_get_default();
	width = gdk_screen_get_width(screen);
	widthmm = gdk_screen_get_width_mm(screen);

	pix = (float)width / widthmm;
}
