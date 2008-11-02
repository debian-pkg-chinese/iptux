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
#include "udt.h"
#include "utils.h"
#include "baling.h"

 Control::Control():ipseg(NULL), palicon(0), myicon(0),
myname(NULL), encode(NULL), path(NULL), flags(0),
dirty(false), table(NULL), pix(3.4)
{
	pthread_mutex_init(&mutex, NULL);
}

Control::~Control()
{
	pthread_mutex_lock(&mutex);
	g_slist_foreach(ipseg, remove_each_info, GINT_TO_POINTER(UNKNOWN));
	g_slist_free(ipseg);
	pthread_mutex_unlock(&mutex);
	pthread_mutex_destroy(&mutex);

	free(myname);
	free(encode);
	free(path);

	g_object_unref(table);
}

void Control::InitSelf()
{
	char file[MAX_PATH_BUF], *ev;
	int status;

	ev = getenv("HOME");
	snprintf(file, MAX_PATH_BUF, "%s/.iptux/info", ev);
	status = access(file, F_OK);
	if (status == 0)
		ReadControl();
	else
		CreateControl();

	CreateTagTable();
	GetRatio_PixMm();
}

void Control::WriteControl()
{
	char filebak[MAX_PATH_BUF], file[MAX_PATH_BUF];
	FILE *stream;
	GSList *tmp;
	char *ev;
	int status;

	ev = getenv("HOME");
	snprintf(file, MAX_PATH_BUF, "%s/.iptux", ev);
	status = access(file, F_OK);
	if (status != 0)
		Mkdir(file, 0777);

	snprintf(filebak, MAX_PATH_BUF, "%s/.iptux/info~", ev);
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
	fprintf(stream, "pal icon = %hhu\n", palicon);
	fprintf(stream, "self icon = %hhu\n", myicon);
	fprintf(stream, "nick name = %s\n", myname);
	fprintf(stream, "net encode = %s\n", encode);
	fprintf(stream, "save path = %s\n", path);
	if (flags & BIT2)
		fprintf(stream, "open blacklist = true\n");
	else
		fprintf(stream, "open blacklist = false\n");
	if (flags & BIT1)
		fprintf(stream, "proof share = true\n");
	else
		fprintf(stream, "proof share = false\n");
	fclose(stream);

	snprintf(file, MAX_PATH_BUF, "%s/.iptux/info", ev);
	rename(filebak, file);
	dirty = false;
}

void Control::CreateControl()
{
	pthread_mutex_lock(&mutex);
	ipseg = g_slist_append(ipseg, Strdup("10.10.0.0"));
	ipseg = g_slist_append(ipseg, Strdup("10.10.3.255"));
	ipseg = g_slist_append(ipseg, Strdup("192.168.0.0"));
	ipseg = g_slist_append(ipseg, Strdup("192.168.1.255"));
	pthread_mutex_unlock(&mutex);
	palicon = 1;
	myicon = 0;
	myname = Strdup(getenv("USER"));
	encode = Strdup(_("UTF-8"));
	path = Strdup(getenv("HOME"));
	flags = 0;

	dirty = true;
}

void Control::ReadControl()
{
	char file[MAX_PATH_BUF];
	char *ev, *buf, *tmp;
	FILE *stream;
	guint count, sum;
	size_t n;

	ev = getenv("HOME");
	snprintf(file, MAX_PATH_BUF, "%s/.iptux/info", ev);
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
	sscanf(tmp, "%hhu", &palicon);
	getline(&buf, &n, stream);
	tmp = strchr(buf, '=') + 1;
	sscanf(tmp, "%hhu", &myicon);
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
		flags |= BIT2;
	free(tmp);
	getline(&buf, &n, stream);
	tmp = my_getline(strchr(buf, '=') + 1);
	if (strcasecmp(tmp, "true") == 0)
		flags |= BIT1;
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

void Control::GetRatio_PixMm()
{
	GdkScreen *screen;
	gint width, widthmm;

	screen = gdk_screen_get_default();
	width = gdk_screen_get_width(screen);
	widthmm = gdk_screen_get_width_mm(screen);

	pix = (float)width / widthmm;
}
