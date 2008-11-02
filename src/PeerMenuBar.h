//
// C++ Interface: PeerMenuBar
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef PEERMENUBAR_H
#define PEERMENUBAR_H

#include "DialogPeer.h"

class PeerMenuBar {
 public:
	PeerMenuBar(gpointer data);
	~PeerMenuBar();

	void CreateMenuBar();

	GtkWidget *menu_bar;
 private:
	void CreateFileMenu();
	void CreateHelpMenu();

	DialogPeer *peer;
};

#endif
