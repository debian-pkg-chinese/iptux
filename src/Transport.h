//
// C++ Interface: Transport
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef TRANSPORT_H
#define TRANSPORT_H

#include "face.h"
#include "sys.h"
class RecvFile;
class SendFile;

class Transport {
 public:
	Transport();
	~Transport();

	void InitSelf();
	static void TransportEntry();
	static void RecvFileEntry(GtkTreePath * path);
	static void SendFileEntry(int sock, GtkTreeIter * iter,
				  uint32_t fileattr);
 private:
	 GtkTreeModel * CreateTransModel();
	bool CheckExist();
	void CreateTransView();
	void CreateTransDialog();
	void RecvFileData(GtkTreeIter * iter);
	void RecvDirFiles(GtkTreeIter * iter);
	uint32_t RecvData(int sock, int fd, GtkTreeIter * iter,
			  uint32_t filesize, char *buf, uint32_t offset);
	void SendFileData(int sock, GtkTreeIter * iter);
	void SendDirFiles(int sock, GtkTreeIter * iter);
	uint32_t SendData(int sock, int fd, GtkTreeIter * iter,
			  uint32_t filesize, char *buf);
	void EndTransportData(int sock, int fd, GtkTreeIter * iter,
			      const char *pathname);
	void EndTransportDirFiles(GtkTreeIter * iter, char *filename);

	GtkWidget *transport;
	GtkWidget *trans_view;
	GtkTreeModel *trans_model;
 public:
	 friend class RecvFile;
	friend class SendFile;
//回调处理部分
 public:
	static void TidyTask();
 private:
	static void DestroyDialog();
	static gboolean ViewPopMenu(GtkWidget * view, GdkEventButton * event,
				    GtkTreeModel * model);
};

#endif
