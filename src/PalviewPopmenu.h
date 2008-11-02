//
// C++ Interface: PalviewPopmenu
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef PALVIEWPOPMENU_H
#define PALVIEWPOPMENU_H

#include "face.h"

class PalviewPopmenu {
 public:
	PalviewPopmenu();
	~PalviewPopmenu();

	void CreatePopMenu(gpointer data);

	GtkWidget *menu;
//回调处理部分
 public:
	static void AskShareFiles(gpointer data);
 private:
	static void DeletePal(gpointer data);
};

#endif
