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
#include "baling.h"
#include "output.h"

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

void iptux_quit()
{
	gtk_main_quit();
	pmessage("IpTux quit!\n");
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

GSList *get_sys_broadcast_addr()
{
	const uint8_t amount = 5;
	extern struct interactive inter;
	char buf[INET_ADDRSTRLEN];
	struct ifconf ifc;
	struct ifreq *ifr;
	SI *addr;
	GSList *list;
	uint8_t count, sum;
	int status;

	list = g_slist_append(NULL, Strdup("255.255.255.255"));
	ifc.ifc_len = amount * sizeof(struct ifreq);
	ifc.ifc_buf = (caddr_t) Malloc(ifc.ifc_len);
	status = ioctl(inter.sock, SIOCGIFCONF, &ifc);
	if (status == -1)
		return list;
	sum = ifc.ifc_len / sizeof(struct ifreq);
	count = 0;
	while (count < sum) {
		ifr = ifc.ifc_req + count;
		count++;

		status = ioctl(inter.sock, SIOCGIFFLAGS, ifr);
		if (status == -1 || !(ifr->ifr_flags & IFF_BROADCAST))
			continue;
		status = ioctl(inter.sock, SIOCGIFBRDADDR, ifr);
		if (status == -1)
			continue;
		addr = (SI *) & ifr->ifr_broadaddr;
		inet_ntop(AF_INET, &addr->sin_addr, buf, INET_ADDRSTRLEN);
		list = g_slist_append(list, Strdup(buf));
	}

	return list;
}

GSList *get_sys_host_addr()
{
	const uint8_t amount = 5;
	extern struct interactive inter;
	char buf[INET_ADDRSTRLEN];
	struct ifconf ifc;
	struct ifreq *ifr;
	SI *addr;
	GSList *list;
	uint8_t count, sum;
	int status;

	list = NULL;
	ifc.ifc_len = amount * sizeof(struct ifreq);
	ifc.ifc_buf = (caddr_t) Malloc(ifc.ifc_len);
	status = ioctl(inter.sock, SIOCGIFCONF, &ifc);
	if (status == -1)
		return list;
	sum = ifc.ifc_len / sizeof(struct ifreq);
	count = 0;
	while (count < sum) {
		ifr = ifc.ifc_req + count;
		count++;

		if (strncasecmp(ifr->ifr_name,"lo",2) == 0)
			continue;
		status = ioctl(inter.sock, SIOCGIFFLAGS, ifr);
		if (status == -1 || !(ifr->ifr_flags & IFF_UP))
			continue;
		status = ioctl(inter.sock, SIOCGIFADDR, ifr);
		if (status == -1)
			continue;
		addr = (SI *) & ifr->ifr_broadaddr;
		inet_ntop(AF_INET, &addr->sin_addr, buf, INET_ADDRSTRLEN);
		list = g_slist_append(list, Strdup(buf));
	}

	return list;
}

char *get_sys_host_addr_string(GSList *ip_list)
{
	char *ipstr,*ptr;
	GSList *tmp;
	uint8_t sum;

	if (!ip_list)
		return NULL;

	sum = g_slist_length(ip_list);
	ipstr = ptr = (char*)Malloc(sum*INET_ADDRSTRLEN);
	tmp = ip_list;
	while (tmp) {
		strcpy(ptr, (char*)tmp->data);
		ptr += strlen(ptr) + 1;
		*(ptr-1) = '\n';
		tmp = tmp->next;
	}
	*(ptr-1) = '\0';

	return ipstr;
}
