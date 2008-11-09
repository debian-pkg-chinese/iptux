//
// C++ Interface: Pal
//
// Description:
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
#include "net.h"
class MainWindow;
class UdpData;
class Cammand;
class RevisePal;
class DialogGroup;
class RecvFile;
class SendFile;
class Transport;
class DialogPeer;
class MainMenuBar;
class PalviewPopmenu;

//对自己而言操作
class Pal {
 public:
	Pal();
	~Pal();

	void CreateInfo(in_addr_t ip, const char *msg, size_t size, bool trans);
	void UpdateInfo(const char *msg, size_t size, bool trans);
	void SetPalmodelValue(GtkTreeModel *model, GtkTreeIter * iter);

	bool CheckReply(uint32_t packetno, bool install);
	void BufferInsertText(const char *str, enum INSERTTYPE type);
	bool RecvMessage(const char *msg);
	bool RecvAskShared(const char *msg);
	bool RecvIcon(const char *msg, size_t size);
	void RecvReply(const char *msg);
	void RecvFile(const char *msg, size_t size);

	void SendAnsentry();
	void SendMyIcon();
	void SendReply(const char *msg);
	void SendExit();
 private:
	void IptuxGetIcon(const char *msg, size_t size);
	void IptuxGetEncode(const char *msg, size_t size);
	void BufferInsertPal(const char *msg);
	void BufferInsertSelf(const char *attach);
	void BufferInsertError();
	void ViewScroll();

	in_addr_t ipv4;		//用户IP
	char *version;		//版本
	uint32_t packetn;	//已接受包编号
	char *user;		//用户名
	char *host;		//用户主机
	char *name;		//昵称
	char *iconfile;		//好友头像
	char *encode;		//用户编码
	uint8_t flags;		//3 黑名单:2 更改:1 在线:0 兼容

	GtkTextBuffer *record;
	pointer dialog;
	uint32_t mypacketn;
	bool reply;
 public:
	 friend class MainWindow;
	friend class UdpData;
	friend class Command;
	friend class RevisePal;
	friend class DialogGroup;
	friend class RecvFile;
	friend class SendFile;
	friend class Transport;
	friend class DialogPeer;
	friend class MainMenuBar;
	friend class PalviewPopmenu;
};

#endif
