//
// C++ Interface: GroupMenuBar
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef GROUPMENUBAR_H
#define GROUPMENUBAR_H

#include "DialogGroup.h"

class GroupMenuBar {
 public:
	GroupMenuBar(gpointer data);
	~GroupMenuBar();

	void CreateMenuBar();

	GtkWidget *menu_bar;
 private:
	void CreateFileMenu();
	void CreateHelpMenu();

	DialogGroup *group;
//回调处理部分
 private:
	static void FreshPalList(gpointer data);
};

#endif
