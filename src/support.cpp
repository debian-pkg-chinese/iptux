//
// C++ Implementation: support
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "support.h"
#include "Control.h"
#include "UdpData.h"
#include "SendFile.h"
#include "Transport.h"
#include "Log.h"
#include "baling.h"
#include "output.h"
#include "utils.h"

void iptux_init()
{
	extern Control ctr;
	extern UdpData udt;
	extern SendFile sfl;
	extern Transport trans;

	ctr.InitSelf();
	udt.InitSelf();
	sfl.InitSelf();
	trans.InitSelf();

	signal(SIGPIPE, SIG_IGN);
	signal(SIGHUP, (sighandler_t) iptux_quit);
	signal(SIGINT, (sighandler_t) iptux_quit);
	signal(SIGQUIT, (sighandler_t) iptux_quit);
	signal(SIGTERM, (sighandler_t) iptux_quit);
}

void iptux_gui_quit()
{
	extern Transport trans;
	extern struct interactive inter;

	if (trans.TransportActive() && !pop_request_quit(inter.window))
		return;
	gtk_main_quit();
	iptux_quit();
}

void iptux_quit()
{
	pmessage(_("The messenger is quit!\n"));
	exit(0);
}

void update_widget_bg(GtkWidget * widget, const gchar * file)
{
	GtkStyle *style;
	GdkPixbuf *pixbuf;
	GdkPixmap *pixmap;
	gint width, height;

	pixbuf = gdk_pixbuf_new_from_file(file, NULL);
	if (!pixbuf)
		return;

	width = gdk_pixbuf_get_width(pixbuf);
	height = gdk_pixbuf_get_height(pixbuf);
	pixmap = gdk_pixmap_new(NULL, width, height, 8);
	gdk_pixbuf_render_pixmap_and_mask(pixbuf, &pixmap, NULL, 255);
	g_object_unref(pixbuf);

	style = gtk_style_copy(GTK_WIDGET(widget)->style);
	if (style->bg_pixmap[GTK_STATE_NORMAL])
		g_object_unref(style->bg_pixmap[GTK_STATE_NORMAL]);
	style->bg_pixmap[GTK_STATE_NORMAL] = pixmap;
	gtk_widget_set_style(GTK_WIDGET(widget), style);
	g_object_unref(style);
}

void pixbuf_shrink_scale_1(GdkPixbuf **pixbuf, int width, int height)
{
	gdouble scale_x, scale_y, scale;
	gint _width, _height;
	GdkPixbuf *tmp;

	width = (width != -1)?width:G_MAXINT;
	height = (height != -1)?height:G_MAXINT;
	_width = gdk_pixbuf_get_width(*pixbuf);
	_height = gdk_pixbuf_get_height(*pixbuf);
	if (_width > width || _height > height) {
		scale = ((scale_x = (gdouble) width / _width) <
				(scale_y = (gdouble) height / _height))
				? scale_x : scale_y;
		_width = (gint) (_width * scale);
		_height = (gint) (_height * scale);
		tmp = *pixbuf;
		*pixbuf = gdk_pixbuf_scale_simple(tmp, _width, _height, GDK_INTERP_BILINEAR);
		g_object_unref(tmp);
	}
}

void create_iptux_folder()
{
	char path[MAX_PATHBUF];

	snprintf(path, MAX_PATHBUF, "%s/.iptux", getenv("HOME"));
	if (access(path, F_OK) != 0)
		Mkdir(path, 0777);
}

void socket_enable_broadcast(int sock)
{
	socklen_t len;
	int optval;

	optval = 1;
	len = sizeof(optval);
	setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &optval, len);
}

GSList *get_sys_broadcast_addr(int sock)
{
	const uint8_t amount = 5;
	char buf[INET_ADDRSTRLEN];
	uint8_t count, sum;
	struct ifconf ifc;
	struct ifreq *ifr;
	SI *addr;
	GSList *list;
	int status;

	list = g_slist_append(NULL, Strdup("255.255.255.255"));
	ifc.ifc_len = amount * sizeof(struct ifreq);
	ifc.ifc_buf = (caddr_t) Malloc(ifc.ifc_len);
	status = ioctl(sock, SIOCGIFCONF, &ifc);
	if (status == -1)
		return list;
	sum = ifc.ifc_len / sizeof(struct ifreq);
	count = 0;
	while (count < sum) {
		ifr = ifc.ifc_req + count;
		count++;

		status = ioctl(sock, SIOCGIFFLAGS, ifr);
		if (status == -1 || !(ifr->ifr_flags & IFF_BROADCAST))
			continue;
		status = ioctl(sock, SIOCGIFBRDADDR, ifr);
		if (status == -1)
			continue;
		addr = (SI *) & ifr->ifr_broadaddr;
		inet_ntop(AF_INET, &addr->sin_addr, buf, INET_ADDRSTRLEN);
		list = g_slist_append(list, Strdup(buf));
	}

	return list;
}

GSList *get_sys_host_addr(int sock)
{
	const uint8_t amount = 5;
	char buf[INET_ADDRSTRLEN];
	uint8_t count, sum;
	struct ifconf ifc;
	struct ifreq *ifr;
	SI *addr;
	GSList *list;
	int status;

	list = NULL;
	ifc.ifc_len = amount * sizeof(struct ifreq);
	ifc.ifc_buf = (caddr_t) Malloc(ifc.ifc_len);
	status = ioctl(sock, SIOCGIFCONF, &ifc);
	if (status == -1)
		return list;
	sum = ifc.ifc_len / sizeof(struct ifreq);
	count = 0;
	while (count < sum) {
		ifr = ifc.ifc_req + count;
		count++;

		if (strncasecmp(ifr->ifr_name, "lo", 2) == 0)
			continue;
		status = ioctl(sock, SIOCGIFFLAGS, ifr);
		if (status == -1 || !(ifr->ifr_flags & IFF_UP))
			continue;
		status = ioctl(sock, SIOCGIFADDR, ifr);
		if (status == -1)
			continue;
		addr = (SI *) & ifr->ifr_broadaddr;
		inet_ntop(AF_INET, &addr->sin_addr, buf, INET_ADDRSTRLEN);
		list = g_slist_append(list, Strdup(buf));
	}

	return list;
}

char *get_sys_host_addr_string(int sock)
{
	char *ipstr, *ptr;
	GSList *list, *tmp;
	uint8_t sum;

	tmp = list = get_sys_host_addr(sock);
	if (!list)
		return NULL;

	sum = g_slist_length(list);
	ipstr = ptr = (char *)Malloc(sum * INET_ADDRSTRLEN);
	while (tmp) {
		strcpy(ptr, (char *)tmp->data);
		ptr += strlen(ptr) + 1;
		*(ptr - 1) = '\n';
		tmp = tmp->next;
	}
	*(ptr - 1) = '\0';

	g_slist_foreach(list, remove_foreach, GINT_TO_POINTER(UNKNOWN));
	g_slist_free(list);

	return ipstr;
}
