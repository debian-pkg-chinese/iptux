//
// C++ Interface: UdpData
//
// Description:
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
class CoreThread;
class Pal;
class DialogGroup;
class IptuxSetup;
class MainMenuBar;
class DialogPeer;
class PalviewPopmenu;

class UdpData {
 public:
	static const uint8_t sumseg = 3;
	static const char *localip[];
 public:
	 UdpData();
	~UdpData();

	void InitSelf();
	void UdpDataEntry(in_addr_t ipv4, char *msg, size_t size);

	static gpointer Ipv4GetPal(in_addr_t ipv4);
	static gpointer Ipv4GetPalPos(in_addr_t ipv4);
	static gpointer PalGetMsgPos(gpointer data);
	static GtkTreeModel *Ipv4GetPalModel(in_addr_t ipv4);
	static bool PalGetModelIter(gpointer pal, GtkTreeModel * model,
				    GtkTreeIter * iter);
 private:
	 GtkTreeModel * CreatePalModel();

	void SomeoneEntry(in_addr_t ipv4, char *msg, size_t size);
	void SomeoneExit(in_addr_t ipv4, char *msg, size_t size);
	void SomeoneAnsentry(in_addr_t ipv4, char *msg, size_t size);
	void SomeoneAbsence(in_addr_t ipv4, char *msg, size_t size);
	void SomeoneSendmsg(in_addr_t ipv4, char *msg, size_t size);
	void SomeoneRecvmsg(in_addr_t ipv4, char *msg, size_t size);
	void SomeoneAskShare(in_addr_t ipv4, char *msg, size_t size);

	static void ThreadAskShare(gpointer data);
	static bool AllowAskShare(gpointer data);

	GSList *pallist;
	GQueue *msgqueue;
	pthread_mutex_t mutex;
	GtkTreeModel *pal_model[sumseg + 1];
 public:
	 friend class StatusIcon;
	friend class MainWindow;
	friend class CoreThread;
	friend class Pal;
	friend class DialogGroup;
	friend class IptuxSetup;
	friend class MainMenuBar;
	friend class DialogPeer;
	friend class PalviewPopmenu;
};

#endif
