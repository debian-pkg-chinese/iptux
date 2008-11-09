//
// C++ Interface: my_chooser
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef MY_CHOOSER_H
#define MY_CHOOSER_H

#include "face.h"

class my_chooser {
public:
	my_chooser();
        ~my_chooser();

	static GtkWidget *create_chooser(const gchar *title, GtkWidget *parent);
	static gchar *run_chooser(GtkWidget *chooser);
//回调处理部分
private:
	static void UpdatePreview(GtkFileChooser *chooser, GtkWidget *preview);
};

#endif
