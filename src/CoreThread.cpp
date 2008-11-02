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
#include "Control.h"
#include "UdpData.h"
#include "SendFile.h"
#include "Command.h"
#include "utils.h"
#include "baling.h"
#include "output.h"

bool CoreThread::udp_server = false;
 CoreThread::CoreThread():tcpsock(-1), udpsock(-1)
{
}

CoreThread::~CoreThread()
{
	udp_server = false;
	shutdown(tcpsock, SHUT_RDWR);
	shutdown(udpsock, SHUT_RDWR);
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
	Command cmd;
	int sock, optval;
	socklen_t len;

	sock = Socket(PF_INET, SOCK_DGRAM, 0);
	optval = 1;
	len = sizeof(optval);
	setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &optval, len);

	cmd.BroadCast(sock);
	cmd.DialUp(sock);

	close(sock);
}

void CoreThread::RecvUdp()
{
	extern CoreThread ctd;
	extern UdpData udt;
	char buf[MAX_UDPBUF];
	int sock, status;
	ssize_t size;
	socklen_t len;
	SI addr;

	sock = Socket(PF_INET, SOCK_DGRAM, 0);
	ctd.udpsock = sock;
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(IPMSG_DEFAULT_PORT);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	status = bind(sock, (SA *) & addr, sizeof(addr));
	if (status == -1)
		pwarning(Quit, _("act: bind the UDP port(2425),warning: %s\n"),
			 strerror(errno));
	udp_server = true;

	while (udp_server) {
		len = sizeof(addr);
		if ((size =
		     recvfrom(sock, buf, MAX_UDPBUF, 0, (SA *) & addr,
			      &len)) == -1)
			continue;
		buf[size] = '\0';
		udt.UdpDataEntry(addr.sin_addr.s_addr, buf, size);
		pmessage(_("accept %s> %s\n"), inet_ntoa(addr.sin_addr), buf);
	}
}

void CoreThread::RecvTcp()
{
	extern CoreThread ctd;
	int sock, subsock, status;
	SI addr;

	sock = Socket(PF_INET, SOCK_STREAM, 0);
	ctd.tcpsock = sock;
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(IPMSG_DEFAULT_PORT);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	status = bind(sock, (SA *) & addr, sizeof(addr));
	if (status == -1)
		pwarning(Quit, _("act: bind the TCP port(2425),warning: %s\n"),
			 strerror(errno));
	listen(sock, 5);
	Synchronism();

	while (udp_server) {
		if ((subsock = Accept(sock, NULL, NULL)) == -1)
			continue;
		thread_create(ThreadFunc(SendFile::TcpDataEntry),
			      GINT_TO_POINTER(subsock), FALSE);
	}
}

void CoreThread::WatchIptux()
{
	extern Control ctr;
	extern UdpData udt;
	extern SendFile sfl;
	extern struct interactive inter;

	Synchronism();
	while (udp_server) {
		delay(1, 0);
		pthread_mutex_lock(&udt.mutex);
		gdk_threads_enter();
		if (g_queue_get_length(udt.msgqueue)) {
			gtk_status_icon_set_blinking(inter.status_icon, TRUE);
			gtk_status_icon_set_tooltip(inter.status_icon,
						    _("The message is here!"));
		} else {
			gtk_status_icon_set_blinking(inter.status_icon, FALSE);
			gtk_status_icon_set_tooltip(inter.status_icon,
						    _("IpTux"));
		}
		gdk_threads_leave();
		pthread_mutex_unlock(&udt.mutex);
		if (ctr.dirty)
			ctr.WriteControl();
		if (sfl.dirty)
			sfl.WriteShare();
	}
}

void CoreThread::Synchronism()
{
	while (!udp_server)
		delay(0, 999999);
}
