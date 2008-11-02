//
// C++ Implementation: StatusIcon
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "StatusIcon.h"
#include "StatusiconPopmenu.h"
#include "UdpData.h"
#include "DialogPeer.h"
#include "MainWindow.h"
#include "output.h"

 StatusIcon::StatusIcon():status_icon(NULL)
{
}

StatusIcon::~StatusIcon()
{
	g_object_unref(status_icon);
}

void StatusIcon::CreateStatusIcon()
{
	extern interactive inter;
	GdkPixbuf *pixbuf;
	GdkScreen *screen;

	pixbuf = gdk_pixbuf_new_from_file_at_size(__LOGO_DIR "/tux.png",
						  20, 20, NULL);
	if (!pixbuf) {
		pop_warning(NULL, NULL, "\n%s \"" __LOGO_DIR "/tux.png\" %s",
			    _("The notify icon"), _("is lost!"));
		exit(1);
	}

	status_icon = gtk_status_icon_new_from_pixbuf(pixbuf);
	inter.status_icon = status_icon;
	g_object_unref(pixbuf);
	screen = gdk_screen_get_default();
	gtk_status_icon_set_screen(status_icon, screen);
	gtk_status_icon_set_tooltip(status_icon, _("IpTux"));

	g_signal_connect(status_icon, "activate",
			 G_CALLBACK(StatusIconActivate), NULL);
	g_signal_connect(status_icon, "popup-menu",
			 G_CALLBACK(StatusIconPopMenu), NULL);
}

void StatusIcon::StatusIconActivate()
{
	extern UdpData udt;
	Pal *pal;

	pthread_mutex_lock(&udt.mutex);
	pal = (Pal *) g_queue_peek_head(udt.msgqueue);
	pthread_mutex_unlock(&udt.mutex);
	if (pal)
		DialogPeer::DialogEntry(pal);
	else
		MainWindow::SwitchWindowMode();
}

void StatusIcon::StatusIconPopMenu(GtkStatusIcon * status_icon, guint button,
				   guint activate_time)
{
	StatusiconPopmenu pop_menu;

	pop_menu.CreatePopMenu();
	gtk_menu_popup(GTK_MENU(pop_menu.menu), NULL, NULL,
		       NULL, NULL, button, activate_time);
}
