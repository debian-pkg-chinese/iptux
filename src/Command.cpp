//
// C++ Implementation: Command
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "Command.h"
#include "Control.h"
#include "Pal.h"
#include "utils.h"
#include "support.h"
#include "baling.h"

uint32_t Command::packetn = 0;
 Command::Command():size(0)
{
}

Command::~Command()
{
}

//广播
void Command::BroadCast(int sock)
{
	extern Control ctr;
	GSList *list, *tmp;
	SI addr;

	CreateCommand(IPMSG_ABSENCEOPT | IPMSG_BR_ENTRY, ctr.myname);
	TransferEncode(ctr.encode);
	CreateIptuxExtra();

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(IPMSG_DEFAULT_PORT);
	list = tmp = get_sys_broadcast_addr(sock);
	while (tmp) {
		inet_pton(AF_INET, (char *)tmp->data, &addr.sin_addr);
		sendto(sock, buf, size, 0, (SA *) & addr, sizeof(addr));
		delay(0, 9999999);
		tmp = tmp->next;
	}
	g_slist_foreach(list, remove_each_info, GINT_TO_POINTER(UNKNOWN));
	g_slist_free(list);
}

//单点广播
void Command::DialUp(int sock)
{
	extern Control ctr;
	in_addr_t ip1, ip2, ip;
	GSList *tmp;
	SI addr;

	CreateCommand(IPMSG_DIALUPOPT | IPMSG_ABSENCEOPT | IPMSG_BR_ENTRY,
		      ctr.myname);
	TransferEncode(ctr.encode);
	CreateIptuxExtra();

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(IPMSG_DEFAULT_PORT);
	pthread_mutex_lock(&ctr.mutex);
	tmp = ctr.ipseg;
	while (tmp) {
		inet_pton(AF_INET, (char *)tmp->data, &ip1), ip1 = ntohl(ip1);
		tmp = tmp->next;
		inet_pton(AF_INET, (char *)tmp->data, &ip2), ip2 = ntohl(ip2);
		tmp = tmp->next;
		little_endian(&ip1, &ip2);
		ip = ip1;
		while (ip <= ip2) {
			addr.sin_addr.s_addr = htonl(ip);
			sendto(sock, buf, size, 0, (SA *) & addr, sizeof(addr));
			delay(0, 999999);
			ip++;
		}
	}
	pthread_mutex_unlock(&ctr.mutex);
}

//通知在线
void Command::SendAnsentry(int sock, pointer data)
{
	extern Control ctr;
	Pal *pal;
	SI addr;

	pal = (Pal *) data;
	CreateCommand(IPMSG_ABSENCEOPT | IPMSG_ANSENTRY, ctr.myname);
	TransferEncode(pal->encode);
	CreateIptuxExtra();

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(IPMSG_DEFAULT_PORT);
	addr.sin_addr.s_addr = pal->ipv4;

	sendto(sock, buf, size, 0, (SA *) & addr, sizeof(addr));
}

//通告退出
void Command::SendExit(int sock, pointer data)
{
	Pal *pal;
	SI addr;

	pal = (Pal *) data;
	CreateCommand(IPMSG_DIALUPOPT | IPMSG_BR_EXIT, NULL);
	TransferEncode(pal->encode);

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(IPMSG_DEFAULT_PORT);
	addr.sin_addr.s_addr = pal->ipv4;

	sendto(sock, buf, size, 0, (SA *) & addr, sizeof(addr));
}

//更改我的信息
void Command::SendAbsence(int sock, pointer data)
{
	extern Control ctr;
	Pal *pal;
	SI addr;

	pal = (Pal *) data;
	CreateCommand(IPMSG_ABSENCEOPT | IPMSG_BR_ABSENCE, ctr.myname);
	TransferEncode(pal->encode);
	CreateIptuxExtra();

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(IPMSG_DEFAULT_PORT);
	addr.sin_addr.s_addr = pal->ipv4;

	sendto(sock, buf, size, 0, (SA *) & addr, sizeof(addr));
}

//探测好友
void Command::SendDetectPacket(int sock, in_addr_t ipv4)
{
	extern Control ctr;
	uint8_t count;
	SI addr;

	CreateCommand(IPMSG_DIALUPOPT | IPMSG_ABSENCEOPT | IPMSG_BR_ENTRY,
		      ctr.myname);
	TransferEncode(ctr.encode);
	CreateIptuxExtra();

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(IPMSG_DEFAULT_PORT);
	addr.sin_addr.s_addr = ipv4;

	count = 0;
	while (count < IPMSG_RETRY_TIMES) {
		sendto(sock, buf, size, 0, (SA *) & addr, sizeof(addr));
		count++;
	}
}

//发送消息
void Command::SendMessage(int sock, pointer data, const char *msg)
{
	uint32_t packetno;
	uint8_t count;
	Pal *pal;
	SI addr;

	pal = (Pal *) data, packetno = packetn;
	pal->CheckReply(packetno, true);
	CreateCommand(IPMSG_SENDCHECKOPT | IPMSG_SENDMSG, msg);
	TransferEncode(pal->encode);

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(IPMSG_DEFAULT_PORT);
	addr.sin_addr.s_addr = pal->ipv4;

	count = 0;
	do {
		sendto(sock, buf, size, 0, (SA *) & addr, sizeof(addr));
		delay(1, 0);
		count++;
	} while (!pal->CheckReply(packetno, false) &&
		 count < IPMSG_RETRY_TIMES);
	if (!pal->CheckReply(packetno, false))
		pal->BufferInsertText(NULL, ERROR);
}

//回复消息
void Command::SendReply(int sock, pointer data, uint32_t packetno)
{
	char packetstr[5];
	Pal *pal;
	SI addr;

	pal = (Pal *) data;
	snprintf(packetstr, 5, "%u", packetno);
	CreateCommand(IPMSG_SENDCHECKOPT | IPMSG_RECVMSG, packetstr);
	TransferEncode(pal->encode);

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(IPMSG_DEFAULT_PORT);
	addr.sin_addr.s_addr = pal->ipv4;

	sendto(sock, buf, size, 0, (SA *) & addr, sizeof(addr));
}

//群发消息
void Command::SendGroupMsg(int sock, pointer data, const char *msg)
{
	Pal *pal;
	SI addr;

	pal = (Pal *) data;
	CreateCommand(IPMSG_BROADCASTOPT | IPMSG_SENDMSG, msg);
	TransferEncode(pal->encode);

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(IPMSG_DEFAULT_PORT);
	addr.sin_addr.s_addr = pal->ipv4;

	sendto(sock, buf, size, 0, (SA *) & addr, sizeof(addr));
}

bool Command::SendAskData(int sock, pointer data, uint32_t packetno,
			  uint32_t fileid, uint32_t offset)
{
	char attrstr[15];
	Pal *pal;
	SI addr;

	pal = (Pal *) data;
	snprintf(attrstr, 15, "%x:%x:%x", packetno, fileid, offset);
	CreateCommand(IPMSG_FILEATTACHOPT | IPMSG_GETFILEDATA, attrstr);
	TransferEncode(pal->encode);

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(IPMSG_DEFAULT_PORT);
	addr.sin_addr.s_addr = pal->ipv4;

	if (Connect(sock, (SA *) & addr, sizeof(addr)) == -1)
		return false;
	if (Write(sock, buf, size) == -1)
		return false;

	return true;
}

bool Command::SendAskFiles(int sock, pointer data, uint32_t packetno,
			   uint32_t fileid)
{
	char attrstr[15];
	Pal *pal;
	SI addr;

	pal = (Pal *) data;
	snprintf(attrstr, 15, "%x:%x:0", packetno, fileid);	//兼容 LanQQ 软件
	CreateCommand(IPMSG_FILEATTACHOPT | IPMSG_GETDIRFILES, attrstr);
	TransferEncode(pal->encode);

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(IPMSG_DEFAULT_PORT);
	addr.sin_addr.s_addr = pal->ipv4;

	if (Connect(sock, (SA *) & addr, sizeof(addr)) == -1)
		return false;
	if (Write(sock, buf, size) == -1)
		return false;

	return true;
}

void Command::SendAskShare(int sock, pointer data)
{
	uint8_t count;
	Pal *pal;
	SI addr;

	pal = (Pal *) data;
	CreateCommand(IPTUX_ASKSHARE, NULL);
	TransferEncode(pal->encode);

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(IPMSG_DEFAULT_PORT);
	addr.sin_addr.s_addr = pal->ipv4;

	sendto(sock, buf, size, 0, (SA *) & addr, sizeof(addr));
}

void Command::SendShareInfo(int sock, pointer data, const char *extra)
{
	uint8_t count;
	char *ptr;
	Pal *pal;
	SI addr;

	pal = (Pal *) data;
	CreateCommand(IPMSG_FILEATTACHOPT | IPMSG_SENDMSG, NULL);
	TransferEncode(pal->encode);
	ptr = transfer_encode(extra, pal->encode, true);
	CreateIpmsgExtra(ptr);
	free(ptr);

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(IPMSG_DEFAULT_PORT);
	addr.sin_addr.s_addr = pal->ipv4;

	sendto(sock, buf, size, 0, (SA *) & addr, sizeof(addr));
}

void Command::CreateCommand(uint32_t command, const char *attach)
{
	char *ptr, *ev;

	snprintf(buf, MAX_UDPBUF, "%u", IPMSG_VERSION);
	size = strlen(buf);
	ptr = buf + size;

	snprintf(ptr, MAX_UDPBUF - size, ":%u", packetn);
	packetn++;
	size += strlen(ptr);
	ptr = buf + size;

	ev = getenv("USERNAME");
	snprintf(ptr, MAX_UDPBUF - size, ":%s", ev);
	size += strlen(ptr);
	ptr = buf + size;

	ev = getenv("HOSTNAME");
	snprintf(ptr, MAX_UDPBUF - size, ":%s", ev);
	size += strlen(ptr);
	ptr = buf + size;

	snprintf(ptr, MAX_UDPBUF - size, ":%u", command);
	size += strlen(ptr);
	ptr = buf + size;

	if (attach)
		snprintf(ptr, MAX_UDPBUF - size, ":%s", attach);
	else
		snprintf(ptr, MAX_UDPBUF - size, ":");
	size += strlen(ptr) + 1;
}

void Command::TransferEncode(const char *encode)
{
	char *ptr;

	ptr = transfer_encode(buf, encode, true);
	size = strlen(ptr) + 1;
	memcpy(buf, ptr, size);
	free(ptr);
}

void Command::CreateIptuxExtra()
{
	extern Control ctr;
	char *ptr, *ev;

	ptr = buf + size;
	snprintf(ptr, MAX_UDPBUF - size, "%hhu", ctr.myicon);
	size += strlen(ptr) + 1;
	ptr = buf + size;
	ev = getenv("LANG");
	snprintf(ptr, MAX_UDPBUF - size, "%s", strchr(ev, '.') + 1);
	size += strlen(ptr) + 1;
}

void Command::CreateIpmsgExtra(const char *extra)
{
	char *ptr, *tmp;

	ptr = buf + size;
	snprintf(ptr, MAX_UDPBUF - size, "%s", extra);
	if (tmp = strrchr(ptr, '\a'))
		*(tmp + 1) = '\0';
	size += strlen(ptr) + 1;
}