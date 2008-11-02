//
// C++ Interface: TreeviewPopmenu
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef TREEVIEWPOPMENU_H
#define TREEVIEWPOPMENU_H

#include "face.h"

class TreeviewPopmenu {
 public:
	TreeviewPopmenu();
	~TreeviewPopmenu();

	void CreatePopMenu(GtkTreeModel * model);

	GtkWidget *menu;
//回调处理部分
 private:
	static void SelectAll(GtkTreeModel * model);
	static void TurnSelect(GtkTreeModel * model);
	static void ClearAll(GtkTreeModel * model);
};

#endif
