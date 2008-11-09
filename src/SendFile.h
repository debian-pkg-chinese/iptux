//
// C++ Interface: SendFile
//
// Description:
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
class ShareFile;

class SendFile {
 public:
	SendFile();
	~SendFile();

	void InitSelf();
	void WriteShared();

	void SendSharedFiles(gpointer data);
	void AddSendFile(GSList * list, gpointer data);

	static void TcpDataEntry(int sock);	//线程入口
	static void SendRegular(gpointer data);	//回调入口
	static void SendFolder(gpointer data);	//回调入口

	bool dirty;
 private:
	void SendFileData(int sock, char *buf);
	void SendDirFiles(int sock, char *buf);
	pointer FindSendFileinfo(uint32_t fileid);

	uint32_t pbn;
	uint32_t prn;
	GSList *pblist;
	GSList *prlist;
	pthread_mutex_t mutex;
 public:
	 friend class ShareFile;
};

#endif
