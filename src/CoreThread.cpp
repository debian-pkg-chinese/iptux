//
// C++ Implementation: CoreThread
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "CoreThread.h"
#include "MainWindow.h"
#include "StatusIcon.h"
#include "Control.h"
#include "SendFile.h"
#include "UdpData.h"
#include "TcpData.h"
#include "Command.h"
#include "baling.h"
#include "output.h"
#include "support.h"
#include "utils.h"

bool CoreThread::server = false;
CoreThread::CoreThread()
{
}

CoreThread::~CoreThread()
{
	extern struct interactive inter;

	server = false;
	shutdown(inter.udpsock, SHUT_RDWR);
	shutdown(inter.tcpsock, SHUT_RDWR);
}

void CoreThread::CoreThreadEntry()
{
	thread_create(ThreadFunc(RecvUdp), NULL, false);
	thread_create(ThreadFunc(RecvTcp), NULL, false);
	thread_create(ThreadFunc(WatchIptux), NULL, false);
	Synchronism();
	NotifyAll();
}

void CoreThread::NotifyAll()
{
	extern struct interactive inter;
	Command cmd;

	cmd.BroadCast(inter.udpsock);
	cmd.DialUp(inter.udpsock);
}

void CoreThread::RecvUdp()
{
	extern struct interactive inter;
	extern UdpData udt;
	char buf[MAX_UDPBUF];
	socklen_t len;
	ssize_t size;
	SI addr;
	int sock;

	inter.udpsock = sock = Socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	socket_enable_broadcast(sock);
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(IPTUX_DEFAULT_PORT);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sock, (SA *) & addr, sizeof(addr)) == -1) {
		gdk_threads_enter();
		pop_error(_("act: bind the UDP port(2425) !\nerror: %s !"),
			 strerror(errno));
		gdk_threads_leave();
	}
	server = true;

	while (server) {
		len = sizeof(addr);
		if ((size = recvfrom(sock, buf, MAX_UDPBUF, 0,
				     (SA *) & addr, &len)) == -1)
			continue;
		if (size != MAX_UDPBUF)
			buf[size] = '\0';
		udt.UdpDataEntry(addr.sin_addr.s_addr, buf, size);
		pmessage(_("accept %s> %s\n"), inet_ntoa(addr.sin_addr), buf);
	}
}

void CoreThread::RecvTcp()
{
	extern struct interactive inter;
	int sock, subsock;
	SI addr;

	inter.tcpsock = sock = Socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(IPTUX_DEFAULT_PORT);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sock, (SA *) & addr, sizeof(addr)) == -1) {
		gdk_threads_enter();
		pop_error(_("act: bind the TCP port(2425) !\nerror: %s !"),
			 strerror(errno));
		gdk_threads_leave();
	}
	listen(sock, 5);
	Synchronism();

	while (server) {
		if ((subsock = Accept(sock, NULL, NULL)) == -1)
			continue;
		thread_create(ThreadFunc(TcpData::TcpDataEntry),
			      GINT_TO_POINTER(subsock), false);
	}
}

void CoreThread::WatchIptux()
{
	extern Control ctr;
	extern SendFile sfl;

	Synchronism();
	while (server) {
		my_delay(1, 0);
		gdk_threads_enter();
		StatusIcon::UpdateTips();
		MainWindow::UpdateTips();
		gdk_threads_leave();
		if (ctr.dirty)
			ctr.WriteControl();
		if (sfl.dirty)
			sfl.WriteShared();
	}
}

void CoreThread::Synchronism()
{
	while (!server)
		my_delay(0, 999999);
}
