//
// C++ Implementation: my_file
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "my_file.h"
#include "output.h"
#include "baling.h"

my_file::my_file(bool fg)
{
	if (!getcwd(path, MAX_PATH_BUF))
		path[0] = '\0';
	flag = fg;
}

my_file::~my_file()
{
}

void my_file::chdir(const char *dir)
{
	size_t len;
	char *ptr;

	if (strcmp(dir, ".") == 0)
		return;

	if (strcmp(dir, "..") == 0) {
		ptr = strrchr(path, '/');
		if (ptr && ptr != path)
			*ptr = '\0';
	} else {
		if (*dir != '/') {
			len = strlen(path);
			snprintf(path + len, MAX_PATH_BUF - len, "/%s", dir);
		} else
			snprintf(path, MAX_PATH_BUF, "%s", dir);
		if (flag)
			Mkdir(path, 0777);
	}
}

int my_file::open(const char *filename, int flags, ...)
{
	int fd;
	bool tmp;
	va_list ap;

	tmp = flag, flag = false;
	chdir(filename);
	va_start(ap, flags);
	fd = Open(path, flags, va_arg(ap, mode_t));
	va_end(ap);
	chdir("..");
	flag = tmp;

	return fd;
}

int my_file::stat(const char *filename, struct stat64 *st)
{
	int result;
	bool tmp;

	tmp = flag, flag = false;
	chdir(filename);
	result = Stat(path, st);
	chdir("..");
	flag = tmp;

	return result;
}

DIR *my_file::opendir()
{
	DIR *dir;

	dir =::opendir(path);
	if (!dir)
		pwarning(Fail, _("act: open directory '%s',warning: %s\n"),
			 strerror(errno));

	return dir;
}
