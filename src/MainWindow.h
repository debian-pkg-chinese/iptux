//
// C++ Interface: MainWindow
//
// Description:创建主面板
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "face.h"

class MainWindow {
 public:
	MainWindow();
	~MainWindow();

	void CreateWindow();
	void CreateAllArea();
 private:
	 GtkWidget *CreateMenuBar();
	 GtkWidget *CreatePalView();

	void CreateFileMenu(GtkWidget *menu_bar);
	void CreateToolMenu(GtkWidget *menu_bar);
	void CreateHelpMenu(GtkWidget *menu_bar);

	GtkWidget *window;
	GtkWidget *client_box;
	GtkAccelGroup *accel;
 public:
	 static void UpdateTips();
 private:
	 static GtkWidget *CreatePopupMenu(gpointer data);	//pal
//回调处理部分
 public:
	static void SwitchWindowMode();
	static void AskSharedFiles(gpointer data);
 private:
	 static void UpdatePalList();
	 static void DeletePal(gpointer data);
	static gboolean ViewQueryTooltip(GtkWidget * view, gint x, gint y,
					 gboolean key, GtkTooltip * tooltip,
					 GtkTreeModel * model);
	static void ViewRowActivated(GtkWidget * view, GtkTreePath * path,
				      GtkTreeViewColumn *column, GtkTreeModel * model);
	static gboolean PopupPalMenu(GtkWidget * view, GdkEventButton * event,
				    GtkTreeModel * model);
	static gboolean ViewChangeStatus(GtkWidget * view, GdkEventButton * event,
				   GtkTreeModel * model);
	static void DragDataReceived(GtkWidget * view, GdkDragContext * context,
				     gint x, gint y, GtkSelectionData * select,
				     guint info, guint time,
				     GtkTreeModel * model);
};

#endif
