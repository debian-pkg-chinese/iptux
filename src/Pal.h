//
// C++ Interface: Pal
//
// Description:好友相关数据
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef PAL_H
#define PAL_H

#include "udt.h"
class MainWindow;
class IptuxSetup;
class UdpData;
class TcpData;
class Cammand;
class RevisePal;
class DialogGroup;
class RecvFile;
class SendFile;
class Transport;
class DialogPeer;
class Log;

//对自己而言操作
class Pal {
 public:
	Pal();
	~Pal();

	void CreateInfo(in_addr_t ip, const char *msg, size_t size, bool trans);
	void UpdateInfo(const char *msg, size_t size, bool trans);
	void SetPalmodelValue(GtkTreeModel * model, GtkTreeIter * iter);

	bool CheckReply(uint32_t packetno, bool install);
	void BufferInsertData(GSList *chiplist, enum BELONG_TYPE type);
	bool RecvMessage(const char *msg);
	bool RecvAskShared(const char *msg);
	bool RecvIcon(const char *msg, size_t size);
	void RecvReply(const char *msg);
	void RecvFile(const char *msg, size_t size);
	void RecvSign(const char *msg);
	void RecvAdPic(const char *path);
	void RecvMsgPic(const char *path);

	void SendAnsentry();
	void SendReply(const char *msg);
	void SendExit();
 private:
	 bool IptuxGetIcon(const char *msg, size_t size);
	bool IptuxGetEncode(const char *msg, size_t size);
	void BufferInsertPal(GSList *chiplist);
	void BufferInsertSelf(GSList *chiplist);
	void BufferInsertError();
	void ViewScroll();

	in_addr_t ipv4;		//用户IP
	char *version;		//版本
	uint32_t packetn;	//已接受包编号
	char *user;		//用户名
	char *host;		//用户主机
	char *name;		//昵称
	char *ad;		//广告
	char *sign;		//个性签名
	char *iconfile;		//好友头像
	char *encode;		//用户编码
	uint8_t flags;		//3 黑名单:2 更改:1 在线:0 兼容

	GtkTextBuffer *record;
	pointer dialog;
	uint32_t mypacketn;
	bool reply;
 public:
	 static void SendFeature(gpointer data);

	 friend class MainWindow;
	 friend class IptuxSetup;
	friend class UdpData;
	friend class TcpData;
	friend class Command;
	friend class RevisePal;
	friend class DialogGroup;
	friend class RecvFile;
	friend class SendFile;
	friend class Transport;
	friend class DialogPeer;
	friend class Log;
};

#endif
