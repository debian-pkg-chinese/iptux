//
// C++ Interface: MainWindow
//
// Description:
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
	void CreateExpanders();
 private:
	 GtkWidget * CreateExpander(const gchar * title);
	GtkWidget *CreatePalView(GtkTreeModel * model);

	GtkWidget *window;
	GtkWidget *mbox;
	GtkAccelGroup *accel;
//回调处理部分
 public:
	static void SwitchWindowMode();
 private:
	static gboolean ViewQueryTooltip(GtkWidget * view, gint x, gint y,
					 gboolean key, GtkTooltip * tooltip,
					 GtkTreeModel * model);
	static void ViewItemActivated(GtkWidget * view, GtkTreePath * path,
				      GtkTreeModel * model);
	static gboolean ViewPopMenu(GtkWidget * view, GdkEventButton * event,
				    GtkTreeModel * model);
	static void DragDataReceived(GtkWidget * view, GdkDragContext * context,
				     gint x, gint y, GtkSelectionData * select,
				     guint info, guint time,
				     GtkTreeModel * model);
};

#endif