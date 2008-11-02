//
// C++ Interface: RevisePal
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef REVISEPAL_H
#define REVISEPAL_H

#include "Pal.h"

class RevisePal {
 public:
	RevisePal(gpointer data);
	~RevisePal();

	static void ReviseEntry(gpointer data);
 private:
	void InitRevise();
	void CreateRevise();
	void RunRevise();
	void ApplyRevise();

	GtkWidget *revise;
	GtkTreeModel *icon_model;
	GtkWidget *name, *encode, *icon;
	Pal *pal;
};

#endif