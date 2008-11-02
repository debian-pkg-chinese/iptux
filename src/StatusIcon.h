//
// C++ Interface: StatusIcon
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef STATUSICON_H
#define STATUSICON_H

#include "face.h"

class StatusIcon {
 public:
	StatusIcon();
	~StatusIcon();

	void CreateStatusIcon();
 private:
	 GtkStatusIcon * status_icon;
//回调处理部分
 private:
	static void StatusIconActivate();
	static void StatusIconPopMenu(GtkStatusIcon * status_icon, guint button,
				      guint activate_time);
};

#endif
