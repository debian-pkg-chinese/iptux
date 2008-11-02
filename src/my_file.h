//
// C++ Interface: my_file
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef MY_FILE_H
#define MY_FILE_H

#include "udt.h"

class my_file {
 public:
	my_file(bool fg);	//r false,w true
	~my_file();

	void chdir(const char *dir);
	int open(const char *filename, int flags, ...);
	int stat(const char *filename, struct stat64 *st);
	DIR *opendir();
 private:
	char path[MAX_PATH_BUF];
	bool flag;
};

#endif
