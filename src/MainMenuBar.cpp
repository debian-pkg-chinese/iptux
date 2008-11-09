//
// C++ Implementation: MainMenuBar
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "MainMenuBar.h"
#include "DetectPal.h"
#include "UdpData.h"
#include "CoreThread.h"
#include "AboutIptux.h"
#include "IptuxSetup.h"
#include "DialogGroup.h"
#include "Transport.h"
#include "ShareFile.h"
#include "Control.h"
#include "Pal.h"
#include "support.h"
#include "utils.h"

 MainMenuBar::MainMenuBar(GtkAccelGroup * acl):
menu_bar(NULL), accel(acl)
{
}

MainMenuBar::~MainMenuBar()
{
}

void MainMenuBar::CreateMenuBar()
{
	menu_bar = gtk_menu_bar_new();
	update_widget_bg(menu_bar, __BACK_DIR "/title.png");
	CreateFileMenu();
	CreateToolMenu();
	CreateHelpMenu();
	gtk_widget_show(menu_bar);
}

void MainMenuBar::CreateFileMenu()
{
	GtkWidget *image;
	GtkWidget *menu;
	GtkWidget *menu_item;

	menu_item = gtk_menu_item_new_with_mnemonic(_("_File"));
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);

	menu = gtk_menu_new();
	gtk_widget_show(menu);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_Detect"));
	image = gtk_image_new_from_file(__TIP_DIR "/detect.png");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), image);
	g_signal_connect(menu_item, "activate",
			 G_CALLBACK(DetectPal::DetectEntry), NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_separator_menu_item_new();
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_Quit"));
	image = gtk_image_new_from_file(__TIP_DIR "/out.png");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), image);
	g_signal_connect(menu_item, "activate", G_CALLBACK(iptux_quit), NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
}

void MainMenuBar::CreateToolMenu()
{
	GtkWidget *image;
	GtkWidget *menu;
	GtkWidget *menu_item;

	menu_item = gtk_menu_item_new_with_mnemonic(_("_Tools"));
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);

	menu = gtk_menu_new();
	gtk_widget_show(menu);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_Transport"));
	image = gtk_image_new_from_file(__TIP_DIR "/trans.png");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), image);
	g_signal_connect(menu_item, "activate",
			 G_CALLBACK(Transport::TransportEntry), NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_Setup"));
	image = gtk_image_new_from_file(__TIP_DIR "/setup.png");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), image);
	g_signal_connect(menu_item, "activate",
			 G_CALLBACK(IptuxSetup::SetupEntry), NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_Public"));
	image = gtk_image_new_from_file(__TIP_DIR "/share.png");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), image);
	g_signal_connect(menu_item, "activate",
			 G_CALLBACK(ShareFile::ShareEntry), NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_Group"));
	image = gtk_image_new_from_file(__TIP_DIR "/net.png");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), image);
	g_signal_connect(menu_item, "activate",
			 G_CALLBACK(DialogGroup::DialogEntry), NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_Update"));
	image = gtk_image_new_from_file(__TIP_DIR "/fresh.png");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), image);
	g_signal_connect(menu_item, "activate", G_CALLBACK(FreshPalList), NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
}

void MainMenuBar::CreateHelpMenu()
{
	extern Control ctr;
	GtkWidget *menu;
	GtkWidget *menu_item;

	menu_item = gtk_menu_item_new_with_mnemonic(_("_Help"));
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);

	menu = gtk_menu_new();
	gtk_widget_show(menu);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);

	menu_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, accel);
	g_signal_connect(menu_item, "activate",
			 G_CALLBACK(AboutIptux::AboutEntry), NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
}

void MainMenuBar::FreshPalList()
{
	extern UdpData udt;
	GtkTreeIter iter;
	uint8_t count;
	GSList *tmp;

	pthread_mutex_lock(&udt.mutex);
	tmp = udt.pallist;
	while (tmp) {
		FLAG_CLR(((Pal *) tmp->data)->flags, 1);
		tmp = tmp->next;
	}
	g_queue_clear(udt.msgqueue);
	pthread_mutex_unlock(&udt.mutex);

	gtk_tree_store_clear(GTK_TREE_STORE(udt.pal_model));
	udt.InitPalModel();

	thread_create(ThreadFunc(CoreThread::NotifyAll), NULL, FALSE);
}
