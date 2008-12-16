//
// C++ Interface: Control
//
// Description:与iptux相关的程序数据
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CONTROL_H
#define CONTROL_H

#include "face.h"
#include "sys.h"

class Control {
 public:
	Control();
	~Control();

	void InitSelf();
	void WriteControl();

	GSList *ipseg;		//通知登录IP段
	char *palicon;		//默认头像
	char *myicon;		//自身头像
	char *myname;		//昵称
	char *encode;		//默认网络编码
	char *path;		//文件存放路径
	char *font;		//字体
	char *sign;		//个性签名
	uint8_t flags;		//2 日志记录:1 黑名单:0 共享过滤
	bool dirty;		//重写标记

	GtkTextTagTable *table;
	GSList *iconlist;
	gfloat pix;

	pthread_mutex_t mutex;
 private:
	void ReadControl();
	void CreateTagTable();
	void GetSysIcon();
	void GetRatio_PixMm();
};

#endif
