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
				 G_CALLBACK(AskShareFiles), data);
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

void PalviewPopmenu::AskShareFiles(gpointer data)
{
	Command cmd;
	int sock;

	sock = Socket(PF_INET, SOCK_DGRAM, 0);
	cmd.SendAskShare(sock, data);
	close(sock);
}

void PalviewPopmenu::DeletePal(gpointer data)
{
	extern UdpData udt;
	GtkTreeModel *model;
	GtkTreeIter iter;
	GList *tmp;
	Pal *pal;

	pal = (Pal *) data;
	if (!UdpData::Ipv4GetPalPos(pal->ipv4))
		return;

	model = UdpData::Ipv4GetPalModel(pal->ipv4);
	if (UdpData::PalGetModelIter(pal, model, &iter))
		gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
	tmp = (GList *) UdpData::PalGetMsgPos(pal);
	if (tmp) {
		pthread_mutex_lock(&udt.mutex);
		g_queue_delete_link(udt.msgqueue, tmp);
		pthread_mutex_unlock(&udt.mutex);
	}
	pal->flags &= ~BIT2;
	pal->flags |= BIT4;
}
