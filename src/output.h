//
// C++ Interface: output
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef OUTPUT_H
#define OUTPUT_H

#include "udt.h"

void pwarning(enum RESULT rst, const char *format, ...);
void pmessage(const char *format, ...);
void pdebug(const char *format, ...);

void pop_warning(GtkWidget * parent, GtkWidget * fw, const gchar * format, ...);
void pop_info(GtkWidget * parent, GtkWidget * fw, const gchar * format, ...);

#endif
