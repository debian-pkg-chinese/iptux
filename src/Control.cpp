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
#include "udt.h"
#include "utils.h"
#include "baling.h"

 Control::Control():ipseg(NULL), palicon(NULL), myicon(NULL),
myname(NULL), encode(NULL), path(NULL), flags(0),
dirty(false), table(NULL), iconlist(NULL), pix(3.4)
{
	pthread_mutex_init(&mutex, NULL);
}

Control::~Control()
{
	pthread_mutex_lock(&mutex);
	g_slist_foreach(ipseg, remove_each_info, GINT_TO_POINTER(UNKNOWN));
	g_slist_free(ipseg);
	g_slist_foreach(iconlist, remove_each_info, GINT_TO_POINTER(UNKNOWN));
	g_slist_free(iconlist);
	pthread_mutex_unlock(&mutex);
	pthread_mutex_destroy(&mutex);

	free(palicon);
	free(myicon);
	free(myname);
	free(encode);
	free(path);

	g_object_unref(table);
}

void Control::InitSelf()
{
	char file[MAX_PATHBUF], *env;

	env = getenv("HOME");
	snprintf(file, MAX_PATHBUF, "%s/.iptux/info", env);
	if (access(file, F_OK) == 0)
		ReadControl();
	else
		CreateControl();

	CreateTagTable();
	GetSysIcon();
	GetRatio_PixMm();
}

void Control::WriteControl()
{
	char filebak[MAX_PATHBUF], file[MAX_PATHBUF];
	FILE *stream;
	GSList *tmp;
	char *env;

	env = getenv("HOME");
	snprintf(file, MAX_PATHBUF, "%s/.iptux", env);
	if (access(file, F_OK) != 0)
		Mkdir(file, 0777);

	snprintf(filebak, MAX_PATHBUF, "%s/.iptux/info~", env);
	if (!(stream = Fopen(filebak, "w")))
		return;
	pthread_mutex_lock(&mutex);
	fprintf(stream, "sum of ip = %u\n", g_slist_length(ipseg));
	tmp = ipseg;
	while (tmp) {
		fprintf(stream, "ip = %s\n", (char *)tmp->data);
		tmp = tmp->next;
	}
	pthread_mutex_unlock(&mutex);
	fprintf(stream, "pal icon = %s\n", palicon);
	fprintf(stream, "self icon = %s\n", myicon);
	fprintf(stream, "nick name = %s\n", myname);
	fprintf(stream, "net encode = %s\n", encode);
	fprintf(stream, "save path = %s\n", path);
	fprintf(stream, "open blacklist = %s\n", FLAG_ISSET(flags,1)?"true":"false");
	fprintf(stream, "proof shared = %s\n", FLAG_ISSET(flags,0)?"true":"false");
	fclose(stream);

	snprintf(file, MAX_PATHBUF, "%s/.iptux/info", env);
	rename(filebak, file);
	dirty = false;
}

void Control::CreateControl()
{
	pthread_mutex_lock(&mutex);
	ipseg = g_slist_append(ipseg, Strdup("10.10.0.0"));
	ipseg = g_slist_append(ipseg, Strdup("10.10.3.255"));
	pthread_mutex_unlock(&mutex);
	palicon = Strdup(__ICON_DIR"/qq.png");
	myicon = Strdup(__ICON_DIR"/tux.png");
	myname = Strdup(getenv("USER"));
	encode = Strdup(_("UTF-8"));
	path = Strdup(getenv("HOME"));
	flags = 0;

	dirty = true;
}

void Control::ReadControl()
{
	char file[MAX_PATHBUF];
	char *buf, *tmp;
	FILE *stream;
	guint count, sum;
	size_t n;

	snprintf(file, MAX_PATHBUF, "%s/.iptux/info", getenv("HOME"));
	if (!(stream = Fopen(file, "r"))) {
		CreateControl();
		return;
	}
	buf = NULL, n = 0;

	getline(&buf, &n, stream);
	tmp = strchr(buf, '=') + 1;
	sscanf(tmp, "%u", &sum);
	pthread_mutex_lock(&mutex);
	count = 0;
	while (count < sum) {
		getline(&buf, &n, stream);
		tmp = strchr(buf, '=') + 1;
		ipseg = g_slist_append(ipseg, my_getline(tmp));
		count++;
	}
	pthread_mutex_unlock(&mutex);

	getline(&buf, &n, stream);
	tmp = strchr(buf, '=') + 1;
	palicon = my_getline(tmp);

	getline(&buf, &n, stream);
	tmp = strchr(buf, '=') + 1;
	myicon = my_getline(tmp);

	getline(&buf, &n, stream);
	tmp = strchr(buf, '=') + 1;
	myname = my_getline(tmp);

	getline(&buf, &n, stream);
	tmp = strchr(buf, '=') + 1;
	encode = my_getline(tmp);

	getline(&buf, &n, stream);
	tmp = strchr(buf, '=') + 1;
	path = my_getline(tmp);

	getline(&buf, &n, stream);
	tmp = my_getline(strchr(buf, '=') + 1);
	if (strcasecmp(tmp, "true") == 0)
		FLAG_SET(flags, 1);
	free(tmp);

	getline(&buf, &n, stream);
	tmp = my_getline(strchr(buf, '=') + 1);
	if (strcasecmp(tmp, "true") == 0)
		FLAG_SET(flags, 0);
	free(tmp);

	free(buf);
	fclose(stream);
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
		if (strcmp(dirt->d_name, ".") == 0 || strcmp(dirt->d_name, "..") == 0)
			continue;
		snprintf(path, MAX_PATHBUF, __ICON_DIR"/%s", dirt->d_name);
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
