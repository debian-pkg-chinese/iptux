//
// C++ Implementation: StatusiconPopmenu
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "StatusiconPopmenu.h"
#include "MainWindow.h"
#include "Transport.h"
#include "IptuxSetup.h"
#include "DialogGroup.h"
#include "DetectPal.h"
#include "ShareFile.h"
#include "udt.h"
#include "support.h"

 StatusiconPopmenu::StatusiconPopmenu():menu(NULL)
{
}

StatusiconPopmenu::~StatusiconPopmenu()
{
}

void StatusiconPopmenu::CreatePopMenu()
{
	extern struct interactive inter;
	GtkWidget *menu_item;
	GtkWidget *image;

	menu = gtk_menu_new();
	gtk_widget_show(menu);

	if (GTK_WIDGET_VISIBLE(inter.window))
		menu_item =
		    gtk_image_menu_item_new_with_mnemonic(_("Hide(_H)"));
	else
		menu_item =
		    gtk_image_menu_item_new_with_mnemonic(_("Show(_S)"));
	image = gtk_image_new_from_file(__TIP_DIR "/desk.png");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), image);
	g_signal_connect(menu_item, "activate",
			 G_CALLBACK(MainWindow::SwitchWindowMode), NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
	gtk_widget_show(menu_item);

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("Transport(_T)"));
	image = gtk_image_new_from_file(__TIP_DIR "/trans.png");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), image);
	g_signal_connect(menu_item, "activate",
			 G_CALLBACK(Transport::TransportEntry), NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("Setup(_S)"));
	image = gtk_image_new_from_file(__TIP_DIR "/setup.png");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), image);
	g_signal_connect(menu_item, "activate",
			 G_CALLBACK(IptuxSetup::SetupEntry), NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("Public(_P)"));
	image = gtk_image_new_from_file(__TIP_DIR "/share.png");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), image);
	g_signal_connect(menu_item, "activate",
			 G_CALLBACK(ShareFile::ShareEntry), NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("Group(_G)"));
	image = gtk_image_new_from_file(__TIP_DIR "/net.png");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), image);
	g_signal_connect(menu_item, "activate",
			 G_CALLBACK(DialogGroup::DialogEntry), NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
	gtk_widget_show(menu_item);

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("Detect(_D)"));
	image = gtk_image_new_from_file(__TIP_DIR "/detect.png");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), image);
	g_signal_connect(menu_item, "activate",
			 G_CALLBACK(DetectPal::DetectEntry), NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("Quit(_Q)"));
	image = gtk_image_new_from_file(__TIP_DIR "/out.png");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), image);
	g_signal_connect(menu_item, "activate", G_CALLBACK(iptux_quit), NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
}
