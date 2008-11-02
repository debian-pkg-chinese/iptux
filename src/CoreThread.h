//
// C++ Interface: CoreThread
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CORETHREAD_H
#define CORETHREAD_H

class CoreThread {
 public:
	CoreThread();
	~CoreThread();

	static void CoreThreadEntry();
	static void NotifyAll();
 private:
	static void RecvUdp();
	static void RecvTcp();
	static void WatchIptux();
	static void Synchronism();

	int tcpsock;
	int udpsock;
	static bool udp_server;
};

#endif
