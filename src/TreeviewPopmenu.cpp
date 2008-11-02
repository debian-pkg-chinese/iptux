//
// C++ Implementation: TreeviewPopmenu
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "TreeviewPopmenu.h"

 TreeviewPopmenu::TreeviewPopmenu():menu(NULL)
{
}

TreeviewPopmenu::~TreeviewPopmenu()
{
}

void TreeviewPopmenu::CreatePopMenu(GtkTreeModel * model)
{
	GtkWidget *menu_item;

	menu = gtk_menu_new();
	gtk_widget_show(menu);

	menu_item = gtk_menu_item_new_with_label(_("Choose All"));
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(SelectAll), model);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_menu_item_new_with_label(_("Reverse All"));
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(TurnSelect), model);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_menu_item_new_with_label(_("Clear Up"));
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(ClearAll), model);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
}

void TreeviewPopmenu::SelectAll(GtkTreeModel * model)
{
	GtkTreeIter iter;

	if (!gtk_tree_model_get_iter_first(model, &iter))
		return;
	do {
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, TRUE, -1);
	} while (gtk_tree_model_iter_next(model, &iter));
}

void TreeviewPopmenu::TurnSelect(GtkTreeModel * model)
{
	GtkTreeIter iter;
	gboolean flag;

	if (!gtk_tree_model_get_iter_first(model, &iter))
		return;
	do {
		gtk_tree_model_get(model, &iter, 0, &flag, -1);
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, !flag, -1);
	} while (gtk_tree_model_iter_next(model, &iter));
}

void TreeviewPopmenu::ClearAll(GtkTreeModel * model)
{
	GtkTreeIter iter;

	if (!gtk_tree_model_get_iter_first(model, &iter))
		return;
	do {
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, FALSE, -1);
	} while (gtk_tree_model_iter_next(model, &iter));
}
