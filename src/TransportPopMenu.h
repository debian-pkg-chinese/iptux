//
// C++ Interface: TransportPopMenu
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef TRANSPORTPOPMENU_H
#define TRANSPORTPOPMENU_H

#include "face.h"

class TransportPopMenu {
 public:
	TransportPopMenu();
	~TransportPopMenu();

	void CreatePopMenu(GtkTreeModel * model);

	GtkWidget *menu;
	static GtkTreePath *path;
//回调处理部分
 private:
	static void StopTask(GtkTreeModel * model);
	static void StopAllTask(GtkTreeModel * model);
};

#endif
