//
// C++ Interface: common
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef COMMON_H
#define COMMON_H

#include "udt.h"
#include "Control.h"
#include "CoreThread.h"
#include "UdpData.h"
#include "SendFile.h"
#include "Transport.h"

Control ctr;
CoreThread ctd;
UdpData udt;
SendFile sfl;
Transport trans;
struct interactive inter;

#endif
