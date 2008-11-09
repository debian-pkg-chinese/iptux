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

#include "sys.h"
#include "face.h"
#include "ipmsg.h"

class FileInfo {
 public:
	FileInfo(uint32_t id, char *name, uint32_t size,
		 uint32_t attr):fileid(id), filename(name), filesize(size),
	    fileattr(attr) {
	} FileInfo():filename(NULL) {
	} ~FileInfo() {
		free(filename);
	}
	uint32_t fileid;
	char *filename;
	uint32_t filesize;
	uint32_t fileattr;
};

enum INSERTTYPE {
	PAL,
	SELF,
	ERROR,
};

enum INFO {
	PALINFO,
	FILEINFO,
	UNKNOWN
};

enum RESULT {
	Success,
	Fail,
	Quit
};

struct interactive {
	GtkWidget *window;
	GtkWidget *palsum;
	GtkStatusIcon *status_icon;
	int sock;
};

struct recvfile_para {
	gpointer data;
	char *msg;
};

typedef struct sockaddr SA;
typedef struct sockaddr_in SI;
typedef struct sockaddr_in6 SI6;
typedef struct sockaddr_storage SS;

#endif
