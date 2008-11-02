//
// C++ Implementation: TransportPopMenu
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "TransportPopMenu.h"
#include "Transport.h"

GtkTreePath *TransportPopMenu::path = NULL;
 TransportPopMenu::TransportPopMenu():menu(NULL)
{
	if (path)
		gtk_tree_path_free(path);
}

TransportPopMenu::~TransportPopMenu()
{
}

void TransportPopMenu::CreatePopMenu(GtkTreeModel * model)
{
	GtkWidget *menu_item;

	menu = gtk_menu_new();
	gtk_widget_show(menu);

	menu_item = gtk_menu_item_new_with_label(_("Terminate Job"));
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(StopTask), model);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_menu_item_new_with_label(_("Terminate All"));
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(StopAllTask), model);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_menu_item_new_with_label(_("Clear Tasklist"));
	g_signal_connect(menu_item, "activate",
			 G_CALLBACK(Transport::TidyTask), NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
}

void TransportPopMenu::StopTask(GtkTreeModel * model)
{
	GtkTreeIter iter;

	if (!path)
		return;
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 8, TRUE, -1);
}

void TransportPopMenu::StopAllTask(GtkTreeModel * model)
{
	GtkTreeIter iter;

	if (!gtk_tree_model_get_iter_first(model, &iter))
		return;
	do {
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, 8, TRUE, -1);
	} while (gtk_tree_model_iter_next(model, &iter));
}
