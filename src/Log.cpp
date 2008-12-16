//
// C++ Implementation: Log
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "Log.h"
#include "Control.h"
#include "Pal.h"
#include "support.h"
#include "baling.h"
#include "utils.h"
#include "udt.h"

Log::Log():communicate(NULL), system(NULL)
{
	char path[MAX_PATHBUF], *env;

	create_iptux_folder();
	env = getenv("HOME");
	snprintf(path, MAX_PATHBUF, "%s/.iptux/communicate.log", env);
	communicate = Fopen(path, "a");
	snprintf(path, MAX_PATHBUF, "%s/.iptux/system.log", env);
	system = Fopen(path, "a");
}

Log::~Log()
{
	fclose(communicate);
	fclose(system);
}

void Log::flush()
{
	fflush(communicate);
	fflush(system);
}

void Log::CommunicateLog(pointer data, const char *fmt, ...)
{
	extern Control ctr;
	gchar *msg, *ptr;
	va_list ap;
	Pal *pal;

	if (!FLAG_ISSET(ctr.flags, 2))
		return;
	if (data) {
		pal = (Pal*)data;
		ptr = getformattime("%s{%s@%s}", pal->name, pal->user, pal->host);
	} else
		ptr = getformattime(_("Me"));
	va_start(ap, fmt);
	msg = g_strdup_vprintf(fmt, ap);
	va_end(ap);
	fprintf(communicate, "%s%s\n", ptr, msg);
	g_free(ptr), g_free(msg);
}

void Log::SystemLog(const char *fmt, ...)
{
	extern Control ctr;
	gchar *msg, *ptr;
	va_list ap;

	if (!FLAG_ISSET(ctr.flags, 2))
		return;
	ptr = getformattime("%s@%s", getenv("USERNAME"), getenv("HOSTNAME"));
	va_start(ap, fmt);
	msg = g_strdup_vprintf(fmt, ap);
	va_end(ap);
	fprintf(system, "%s%s\n\n", ptr, msg);
	g_free(ptr), g_free(msg);
}
