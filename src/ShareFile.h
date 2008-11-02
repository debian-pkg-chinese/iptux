//
// C++ Interface: ShareFile
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef SHAREFILE_H
#define SHAREFILE_H

#include "face.h"
#include "sys.h"

class ShareFile {
 public:
	ShareFile();
	~ShareFile();

	static void ShareEntry();
 private:
	void InitShare();
	void CreateShare();
	void AddShareFiles(GSList * list, uint32_t fileattr);
	void FindInsertPosition(const gchar * path, uint32_t fileattr,
				GtkTreeIter * iter);
	GtkTreeModel *CreateShareModel();
	GtkWidget *CreateShareView();
	static bool CheckExist();

	GtkWidget *share_view;
	GtkTreeModel *share_model;
	static GtkWidget *share;
//回调处理部分
 private:
	static void AddRegular(gpointer data);
	static void AddFolder(gpointer data);
	static void DeleteFiles(gpointer data);
	static void ClickOk(gpointer data);
	static void ClickApply(gpointer data);
	static void ShareDestroy(gpointer data);
};

#endif
