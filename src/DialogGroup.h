//
// C++ Interface: DialogGroup
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef DIALOGGROUP_H
#define DIALOGGROUP_H

#include "face.h"
class GroupMenuBar;

class DialogGroup {
 public:
	DialogGroup();
	~DialogGroup();

	static void DialogEntry();
 private:
	void InitDialog();
	void CreateDialog();
	void CreateChooseArea(GtkWidget * paned);
	void CreateRecordArea(GtkWidget * paned);
	void CreateInputArea(GtkWidget * paned);
	void BufferInsertText(const gchar * msg);
	void SendGroupMsg(const gchar * msg);
	void ViewScroll();
	GtkTreeModel *CreateGroupModel();
	GtkWidget *CreateGroupView();
	static bool CheckExist();

	GtkWidget *pal_view;
	GtkWidget *record, *input;
	GtkAccelGroup *accel;
	GtkTreeModel *group_model;
	static GtkWidget *dialog;
 public:
	 friend class GroupMenuBar;
//回调处理部分
 public:
	static void ViewToggleChange(GtkTreeModel * model, gchar * path);
	static gboolean ViewPopMenu(GtkTreeModel * model,
				    GdkEventButton * event);
 private:
	static void SendMessage(gpointer data);
	static void DialogDestroy(gpointer data);
};

#endif
