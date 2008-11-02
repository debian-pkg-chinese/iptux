//
// C++ Interface: MainMenuBar
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef MAINMENUBAR_H
#define MAINMENUBAR_H

#include "face.h"

class MainMenuBar {
 public:
	MainMenuBar(GtkAccelGroup * acl);
	~MainMenuBar();

	void CreateMenuBar();

	GtkWidget *menu_bar;
 private:
	void CreateFileMenu();
	void CreateToolMenu();
	void CreateHelpMenu();

	GtkAccelGroup *accel;
//回调处理部分
 private:
	static void FreshPalList();
};

#endif
