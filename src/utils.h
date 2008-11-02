//
// C++ Interface: utils
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef UTILS_H
#define UTILS_H

#include "sys.h"

#define difftimeval(val2,val1) \
	((((val2).tv_sec-(val1).tv_sec)*1000000 \
	+ (val2).tv_usec-(val1).tv_usec) \
	/ 1000000.0f)
#define percent(num1,num2) (100.0f*(num1)/(num2))

void delay(time_t sec, long nsec);
void little_endian(uint32_t * digit1, uint32_t * digit2);
char *_iconv(const char *instr, const char *tocode, const char *fromcode);
char *transfer_encode(const char *instr, const char *encode, bool direc);

typedef void *(*ThreadFunc) (void *);
pthread_t thread_create(ThreadFunc func, pointer data, bool joinable);
char *my_getline(const char *str);
int strnchr(const char *str, char chr);
void remove_each_info(pointer data, pointer data1);
char *getformattime(const char *format, ...);
char *number_to_string(uint32_t number, bool rate = false);

uint32_t iptux_get_dec_number(const char *msg, uint8_t times);
uint32_t iptux_get_hex_number(const char *msg, uint8_t times);
char *iptux_get_section_string(const char *msg, uint8_t times);
char *ipmsg_get_filename(const char *msg, uint8_t times);
char *ipmsg_get_attach(const char *msg, uint8_t times);
char *ipmsg_get_extra(const char *msg);
char *ipmsg_set_filename_pal(const char *pathname);
const char *ipmsg_set_filename_self(char *pathname);

#endif