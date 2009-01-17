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
class Pal;
class DialogGroup;
class IptuxSetting;
class DialogPeer;

class UdpData {
 public:
	 UdpData();
	~UdpData();

	void InitSelf();
	void AdjustMemory();
	void UdpDataEntry(in_addr_t ipv4, char *msg, size_t size);
	void SublayerEntry(gpointer data, uint32_t command, const char *path);	//Pal

	gpointer Ipv4GetPal(in_addr_t ipv4);
	gpointer Ipv4GetPalPos(in_addr_t ipv4);
	gpointer PalGetMsgPos(gpointer data);
 private:
	void SomeoneLost(in_addr_t ipv4, char *msg, size_t size);
	void SomeoneEntry(in_addr_t ipv4, char *msg, size_t size);
	void SomeoneExit(in_addr_t ipv4, char *msg, size_t size);
	void SomeoneAnsentry(in_addr_t ipv4, char *msg, size_t size);
	void SomeoneAbsence(in_addr_t ipv4, char *msg, size_t size);
	void SomeoneSendmsg(in_addr_t ipv4, char *msg, size_t size);
	void SomeoneRecvmsg(in_addr_t ipv4, char *msg, size_t size);
	void SomeoneAskShared(in_addr_t ipv4, char *msg, size_t size);
	void SomeoneSendIcon(in_addr_t ipv4, char *msg, size_t size);
	void SomeoneSendSign(in_addr_t ipv4, char *msg, size_t size);

	static void ThreadAskShared(gpointer data);	//
	static bool AllowAskShared(gpointer data);	//

	GSList *pallist;		//好友链表，只能添加，不能删除
	GQueue *msgqueue;	//消息队列
	pthread_mutex_t mutex;
 public:
	 friend class StatusIcon;
	friend class MainWindow;
	friend class Pal;
	friend class DialogGroup;
	friend class IptuxSetting;
	friend class DialogPeer;
};

#endif
