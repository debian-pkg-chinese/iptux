//
// C++ Implementation: PeerMenuBar
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "PeerMenuBar.h"
#include "PalviewPopmenu.h"
#include "SendFile.h"
#include "AboutIptux.h"
#include "support.h"

 PeerMenuBar::PeerMenuBar(gpointer data):
menu_bar(NULL), peer((DialogPeer *) data)
{
}

PeerMenuBar::~PeerMenuBar()
{
}

void PeerMenuBar::CreateMenuBar()
{
	menu_bar = gtk_menu_bar_new();
	update_widget_bg(menu_bar, __BACK_DIR "/title.png");
	CreateFileMenu();
	CreateHelpMenu();
	gtk_widget_show(menu_bar);
}

void PeerMenuBar::CreateFileMenu()
{
	GtkWidget *menu;
	GtkWidget *menu_item;

	menu_item = gtk_menu_item_new_with_mnemonic(_("_File"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
	gtk_widget_show(menu_item);

	menu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
	gtk_widget_show(menu);

	menu_item = gtk_menu_item_new_with_label(_("Send File"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(SendFile::SendRegular), peer->pal);
	gtk_widget_show(menu_item);

	menu_item = gtk_menu_item_new_with_label(_("Send Folder"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(SendFile::SendFolder), peer->pal);
	gtk_widget_show(menu_item);

	menu_item = gtk_menu_item_new_with_label(_("Ask For Shared Files"));
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(PalviewPopmenu::AskSharedFiles),
				 peer->pal);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_tearoff_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
	gtk_widget_show(menu_item);

	menu_item = gtk_menu_item_new_with_label(_("Close"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(gtk_widget_destroy), peer->dialog);
	gtk_widget_show(menu_item);
}

void PeerMenuBar::CreateHelpMenu()
{
	GtkWidget *menu;
	GtkWidget *menu_item;

	menu_item = gtk_menu_item_new_with_mnemonic(_("_Help"));
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);

	menu = gtk_menu_new();
	gtk_widget_show(menu);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);

	menu_item =
	    gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, peer->accel);
	g_signal_connect(menu_item, "activate",
			 G_CALLBACK(AboutIptux::AboutEntry), NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
}
