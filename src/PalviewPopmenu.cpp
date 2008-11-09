//
// C++ Implementation: PalviewPopmenu
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "PalviewPopmenu.h"
#include "UdpData.h"
#include "DialogPeer.h"
#include "RevisePal.h"
#include "SendFile.h"
#include "Command.h"
#include "baling.h"
#include "utils.h"

 PalviewPopmenu::PalviewPopmenu():menu(NULL)
{
}

PalviewPopmenu::~PalviewPopmenu()
{
}

void PalviewPopmenu::CreatePopMenu(gpointer data)
{
	GtkWidget *menu_item;

	menu = gtk_menu_new();
	gtk_widget_show(menu);

	menu_item = gtk_menu_item_new_with_label(_("Send Message"));
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(DialogPeer::DialogEntry), data);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_menu_item_new_with_label(_("Send File"));
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(SendFile::SendRegular), data);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_menu_item_new_with_label(_("Send Folder"));
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(SendFile::SendFolder), data);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_menu_item_new_with_label(_("Ask For Shared Files"));
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(AskSharedFiles), data);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_menu_item_new_with_label(_("Change Information"));
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(RevisePal::ReviseEntry), data);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_menu_item_new_with_label(_("Delete Pal"));
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(DeletePal), data);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
}

void PalviewPopmenu::AskSharedFiles(gpointer data)
{
	Command cmd;
	int sock;

	sock = Socket(PF_INET, SOCK_DGRAM, 0);
	cmd.SendAskShared(sock, data);
	close(sock);
}

void PalviewPopmenu::DeletePal(gpointer data)
{
	extern UdpData udt;
	GtkTreeIter iter,parent;
	GList *tmp;
	Pal *pal;

	pal = (Pal *) data;
	if (!udt.Ipv4GetPalPos(pal->ipv4))
		return;

	udt.Ipv4GetParent(pal->ipv4, &parent);
	if (udt.PalGetModelIter(pal, &parent, &iter))
		gtk_tree_store_remove(GTK_TREE_STORE(udt.pal_model), &iter);
	tmp = (GList *) udt.PalGetMsgPos(pal);
	if (tmp) {
		pthread_mutex_lock(&udt.mutex);
		g_queue_delete_link(udt.msgqueue, tmp);
		pthread_mutex_unlock(&udt.mutex);
	}
	FLAG_CLR(pal->flags, 1);
	FLAG_SET(pal->flags, 3);
}
