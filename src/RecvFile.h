//
// C++ Interface: RecvFile
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef RECVFILE_H
#define RECVFILE_H

#include "Pal.h"

class RecvFile {
 public:
	RecvFile(gpointer data);
	~RecvFile();

	static void RecvEntry(gpointer data);
 private:
	 bool GetValidData();
	void ParseExtra();
	void CreateRecvWindow();
	gpointer DivideFileinfo(char **ptr);
	GtkTreeModel *CreateRecvModel();
	GtkWidget *CreateRecvView();

	Pal *pal;
	char *msg;
	GSList *file_list;
	uint32_t packetn;
	GtkTreeModel *file_model;
//回调处理部分
 private:
	static void CellEditText(GtkCellRendererText * renderer, gchar * path,
				 gchar * new_text, GtkTreeModel * model);
	static void AddRecvFile(GtkTreeModel * model);
};

#endif
