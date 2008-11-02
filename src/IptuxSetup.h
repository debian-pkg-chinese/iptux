//
// C++ Interface: IptuxSetup
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef IPTUXSETUP_H
#define IPTUXSETUP_H

#include "face.h"

class IptuxSetup {
 public:
	IptuxSetup();
	~IptuxSetup();

	static void SetupEntry();
	static GtkTreeModel *CreateIconModel();
	static GtkWidget *CreateComboBoxWithModel(GtkTreeModel * model,
						  gint active);
 private:
	void InitSetup();
	void CreateSetup();
	void CreatePerson(GtkWidget * note);
	void CreateSystem(GtkWidget * note);
	void CreateIpseg(GtkWidget * note);
	void CreateButton(GtkWidget * hbb);
	GtkTreeModel *CreateIpModel();
	GtkWidget *CreateFolderChooser(const char *folder);
	GtkWidget *CreateIpsegView();
	static bool CheckExist();
	static void FreshMyInfo();

	GtkTreeModel *icon_model, *ip_model;
	GtkWidget *myname, *myicon, *save_path;
	GtkWidget *encode, *palicon, *black, *proof;
	GtkWidget *entry1, *entry2;
	GtkWidget *ipseg_view;
	static GtkWidget *setup;
//回调处理部分
 private:
	static void ClickAddIpseg(gpointer data);
	static void ClickDelIpseg(gpointer data);
	static void ClickOk(gpointer data);
	static void ClickApply(gpointer data);
	static void SetupDestroy(gpointer data);
};

#endif
