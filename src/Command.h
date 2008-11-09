//
// C++ Interface: Command
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef COMMAND_H
#define COMMAND_H

#include "udt.h"
#include "net.h"

class Command {
 public:
	Command();
	~Command();

	void BroadCast(int sock);
	void DialUp(int sock);
	void SendAnsentry(int sock, pointer data);
	void SendExit(int sock, pointer data);
	void SendAbsence(int sock, pointer data);
	void SendDetectPacket(int sock, in_addr_t ipv4);
	void SendMessage(int sock, pointer data, const char *msg);
	void SendReply(int sock, pointer data, uint32_t packetno);
	void SendGroupMsg(int sock, pointer data, const char *msg);

	bool SendAskData(int sock, pointer data, uint32_t packetno,
			 uint32_t fileid, uint32_t offset);
	bool SendAskFiles(int sock, pointer data, uint32_t packetno,
			  uint32_t fileid);
	void SendAskShared(int sock, pointer data);
	void SendSharedInfo(int sock, pointer data, const char *extra);
	void SendMyIcon(int sock, pointer data);
 private:
	void CreateCommand(uint32_t command, const char *attach);
	void TransferEncode(const char *encode);
	void CreateIptuxExtra();
	void CreateIpmsgExtra(const char *extra);
	void CreateIconExtra();

	char buf[MAX_UDPBUF];
	size_t size;
	static uint32_t packetn;
};

#endif
