//
// C++ Interface: DialogPeer
//
// Description:好友对话框
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef DIALOGPEER_H
#define DIALOGPEER_H

#include "face.h"
#include "Pal.h"
class Pal;
class SendFile;

class DialogPeer {
 public:
	DialogPeer(gpointer data);
	~DialogPeer();

	static void DialogEntry(gpointer data);
 private:
	void CreateDialog();
	void CreateAllArea();
	void CreateInfoArea(GtkWidget * paned);
	void FillInfoBuffer(GtkTextBuffer * info);
	void CreateRecordArea(GtkWidget * paned);
	void CreateInputArea(GtkWidget * paned);
	GtkWidget *CreateMenuBar();
	void CreateFileMenu(GtkWidget * menu_bar);
	void CreateHelpMenu(GtkWidget * menu_bar);
	static bool CheckExist(gpointer data);

	GtkWidget *dialog;	//主窗口
	GtkWidget *focus;	//焦点
	GtkWidget *scroll;	//滚动
	GtkAccelGroup *accel;
	Pal *pal;
 public:
	 friend class Pal;
	friend class SendFile;
//回调处理部分
 public:
	static void DragDataReceived(gpointer data, GdkDragContext * context,
				     gint x, gint y, GtkSelectionData * select,
				     guint info, guint time);
 private:
	static void DialogDestroy(gpointer data);
	static void SendMessage(gpointer data);
//线程处理
 private:
	static void ThreadSendMessage(gpointer data);
};

#endif
