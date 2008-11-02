//
// C++ Interface: StatusiconPopmenu
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef STATUSICONPOPMENU_H
#define STATUSICONPOPMENU_H

#include "face.h"

class StatusiconPopmenu {
 public:
	StatusiconPopmenu();
	~StatusiconPopmenu();

	void CreatePopMenu();

	GtkWidget *menu;
};

#endif
