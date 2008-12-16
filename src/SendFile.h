//
// C++ Interface: SendFile
//
// Description:发送相关的文件信息,不包含文件数据
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef SENDFILE_H
#define SENDFILE_H

#include "sys.h"
#include "face.h"
class DialogPeer;
class ShareFile;
class UdpData;
class TcpData;

class SendFile {
 public:
	SendFile();
	~SendFile();

	void InitSelf();
	void WriteShared();

	static void SendRegular(gpointer data);	//回调入口
	static void SendFolder(gpointer data);	//回调入口

	bool dirty;
 private:
	void RequestData(int sock, uint32_t fileattr, char *buf);
	void PickFile(uint32_t fileattr, gpointer data);
	void SendFileInfo(GSList * list, gpointer data);
	void SendSharedInfo(gpointer data);
	pointer FindFileinfo(uint32_t fileid);

	uint32_t pbn;
	uint32_t prn;
	GSList *pblist;
	GSList *prlist;
	pthread_mutex_t mutex;
 public:
	 friend class DialogPeer;
	friend class ShareFile;
	friend class UdpData;
	friend class TcpData;
};

#endif
