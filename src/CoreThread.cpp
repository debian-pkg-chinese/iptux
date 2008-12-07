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
	int sock, status;
	socklen_t len;
	ssize_t size;
	SI addr;

	inter.udpsock = sock = Socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	socket_enable_broadcast(sock);
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(IPTUX_DEFAULT_PORT);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	status = bind(sock, (SA *) & addr, sizeof(addr));
	if (status == -1)
		pwarning(Quit, _("act: bind the UDP port(2425),warning: %s\n"),
			 strerror(errno));
	server = true;

	while (server) {
		len = sizeof(addr);
		if ((size = recvfrom(sock, buf, MAX_UDPBUF, 0,
				     (SA *) & addr, &len)) == -1)
			continue;
		buf[size] = '\0';
		udt.UdpDataEntry(addr.sin_addr.s_addr, buf, size);
		pmessage(_("accept %s> %s\n"), inet_ntoa(addr.sin_addr), buf);
	}
}

void CoreThread::RecvTcp()
{
	extern struct interactive inter;
	int sock, subsock, status;
	SI addr;

	inter.tcpsock = sock = Socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(IPTUX_DEFAULT_PORT);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	status = bind(sock, (SA *) & addr, sizeof(addr));
	if (status == -1)
		pwarning(Quit, _("act: bind the TCP port(2425),warning: %s\n"),
			 strerror(errno));
	listen(sock, 5);
	Synchronism();

	while (server) {
		if ((subsock = Accept(sock, NULL, NULL)) == -1)
			continue;
		thread_create(ThreadFunc(SendFile::RequestEntry),
			      GINT_TO_POINTER(subsock), FALSE);
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
