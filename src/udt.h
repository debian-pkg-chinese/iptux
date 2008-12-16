//
// C++ Interface: udt
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef UDT_H
#define UDT_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "sys.h"
#include "net.h"
#include "face.h"
#include "ipmsg.h"

enum BELONG_TYPE {
	PAL,
	SELF,
	ERROR,
};

enum INFO_TYPE {
	PALINFO,
	CHIPDATA,
	FILEINFO,
	UNKNOWN
};

enum STATE_TYPE {
	Success,
	Fail,
	Quit
};

enum MSG_TYPE {
	STRING,
	PICTURE
};

struct interactive {
	GtkWidget *window;
	GtkWidget *online;
	GtkStatusIcon *status_icon;
	int udpsock, tcpsock;
};

struct recvfile_para {
	gpointer data;
	char *msg;
};

struct sendmsg_para {
	gpointer data;
	GSList *chiplist;
};

class FileInfo {
 public:
	FileInfo(uint32_t id, char *name, uint32_t size,
		 uint32_t attr):fileid(id), filename(name),
		 filesize(size), fileattr(attr) {
	} ~FileInfo() {
		free(filename);
	}

	uint32_t fileid;
	char *filename;
	uint32_t filesize;
	uint32_t fileattr;
};

class ChipData {
public:
	ChipData(enum MSG_TYPE tp, char *dt):
		type(tp), data(dt) {
	} ~ChipData() {
		free(data);
	}

	enum MSG_TYPE type;
	char *data;
};

typedef struct sockaddr SA;
typedef struct sockaddr_in SI;

#endif
