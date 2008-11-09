//
// C++ Interface: support
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef SUPPORT_H
#define SUPPORT_H

#include "face.h"

void iptux_init();
void iptux_quit();

void update_widget_bg(GtkWidget * widget, const gchar * file);
GSList *get_sys_broadcast_addr();
GSList *get_sys_host_addr();
char *get_sys_host_addr_string(GSList *ip_list);

#endif
