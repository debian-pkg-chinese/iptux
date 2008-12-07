//
// C++ Interface: UdpData
//
// Description:处理接收到的UDP数据
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef UDPDATA_H
#define UDPDATA_H

#include "face.h"
#include "net.h"
#include "sys.h"
class StatusIcon;
class MainWindow;
class RevisePal;
class Pal;
class DialogGroup;
class IptuxSetup;
class DialogPeer;

class UdpData {
 private:
	static const char *localip[];
 public:
	 UdpData();
	~UdpData();

	void InitSelf();
	void UdpDataEntry(in_addr_t ipv4, char *msg, size_t size);

	gpointer Ipv4GetPal(in_addr_t ipv4);
	gpointer Ipv4GetPalPos(in_addr_t ipv4);
	gpointer PalGetMsgPos(gpointer data);
	void Ipv4GetParent(in_addr_t ipv4, GtkTreeIter * iter);
	bool PalGetModelIter(gpointer pal, GtkTreeIter * parent,
			     GtkTreeIter * iter);
 private:
	 GtkTreeModel * CreatePalModel();
	void InitPalModel();

	void SomeoneLost(in_addr_t ipv4, char *msg, size_t size);
	void SomeoneEntry(in_addr_t ipv4, char *msg, size_t size);
	void SomeoneExit(in_addr_t ipv4, char *msg, size_t size);
	void SomeoneAnsentry(in_addr_t ipv4, char *msg, size_t size);
	void SomeoneAbsence(in_addr_t ipv4, char *msg, size_t size);
	void SomeoneSendmsg(in_addr_t ipv4, char *msg, size_t size);
	void SomeoneRecvmsg(in_addr_t ipv4, char *msg, size_t size);
	void SomeoneAskShared(in_addr_t ipv4, char *msg, size_t size);
	void SomeoneSendIcon(in_addr_t ipv4, char *msg, size_t size);

	static void ThreadAskShared(gpointer data);
	static bool AllowAskShared(gpointer data);

	GSList *pallist;
	GQueue *msgqueue;
	pthread_mutex_t mutex;
	GtkTreeModel *pal_model;
 public:
	 friend class StatusIcon;
	friend class MainWindow;
	friend class RevisePal;
	friend class Pal;
	friend class DialogGroup;
	friend class IptuxSetup;
	friend class DialogPeer;
};

#endif
