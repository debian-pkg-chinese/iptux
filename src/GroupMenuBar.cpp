//
// C++ Implementation: GroupMenuBar
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "GroupMenuBar.h"
#include "AboutIptux.h"
#include "support.h"

 GroupMenuBar::GroupMenuBar(gpointer data):
menu_bar(NULL), group((DialogGroup *) data)
{
}

GroupMenuBar::~GroupMenuBar()
{
}

void GroupMenuBar::CreateMenuBar()
{
	menu_bar = gtk_menu_bar_new();
	update_widget_bg(menu_bar, __BACK_DIR "/title.png");
	CreateFileMenu();
	CreateHelpMenu();
	gtk_widget_show(menu_bar);
}

void GroupMenuBar::CreateFileMenu()
{
	GtkWidget *menu;
	GtkWidget *menu_item;

	menu_item = gtk_menu_item_new_with_mnemonic(_("File(_F)"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
	gtk_widget_show(menu_item);

	menu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
	gtk_widget_show(menu);

	menu_item = gtk_menu_item_new_with_label(_("Update List"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(FreshPalList), group);
	gtk_widget_show(menu_item);

	menu_item = gtk_tearoff_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
	gtk_widget_show(menu_item);

	menu_item = gtk_menu_item_new_with_label(_("Close"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(gtk_widget_destroy), group->dialog);
	gtk_widget_show(menu_item);
}

void GroupMenuBar::CreateHelpMenu()
{
	GtkWidget *menu;
	GtkWidget *menu_item;

	menu_item = gtk_menu_item_new_with_mnemonic(_("Help(_H)"));
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);

	menu = gtk_menu_new();
	gtk_widget_show(menu);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);

	menu_item =
	    gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, group->accel);
	g_signal_connect(menu_item, "activate",
			 G_CALLBACK(AboutIptux::AboutEntry), NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
}

void GroupMenuBar::FreshPalList(gpointer data)
{
	DialogGroup *group;
	GtkTreeModel *model;

	group = (DialogGroup *) data;
	model = group->group_model;
	group->group_model = group->CreateGroupModel();
	gtk_tree_view_set_model(GTK_TREE_VIEW(group->pal_view),
				group->group_model);
	g_object_unref(model);
}
