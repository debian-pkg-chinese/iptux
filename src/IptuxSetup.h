//
// C++ Interface: IptuxSetup
//
// Description:程序功能、数据设置
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef IPTUXSETUP_H
#define IPTUXSETUP_H

#include "udt.h"

class IptuxSetup {
 public:
	IptuxSetup();
	~IptuxSetup();

	static void SetupEntry();
 private:
	void InitSetup();
	void CreateSetup();
	void CreatePerson(GtkWidget * note);
	void CreateSystem(GtkWidget * note);
	void CreateIpseg(GtkWidget * note);
	void CreateFuncButton(GtkWidget * hbb);
	GtkTreeModel *CreateIpModel();
	GtkWidget *CreateFolderChooser();
	GtkWidget *CreateFontChooser();
	GtkWidget *CreateIpsegView();
	static bool CheckExist();

	GtkTreeModel *icon_model, *ip_model;
	GtkWidget *myname, *myicon, *save_path;
	GtkWidget *encode, *palicon, *font, *black, *proof;
	GtkWidget *entry1, *entry2;
	GtkWidget *ipseg_view;
	static GtkWidget *setup;
 public:
	static GtkTreeModel *CreateIconModel();
	static GtkWidget *CreateComboBoxWithModel(GtkTreeModel * model,
						  gchar * iconfile);
	static gint FileGetItemPos(const char *filename, GtkTreeModel * model);
 private:
	static void ObtainPerson(gpointer data);
	static void ObtainSystem(gpointer data);
	static void ObtainIpseg(gpointer data);
	static void UpdateMyInfo();
//回调处理部分
 public:
	static void AddPalIcon(gpointer data);
 private:
	static void ClickAddIpseg(gpointer data);
	static void ClickDelIpseg(gpointer data);
	static void ClickOk(gpointer data);
	static void ClickApply(gpointer data);
	static void SetupDestroy(gpointer data);
};

#endif
