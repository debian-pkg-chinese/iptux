//
// C++ Implementation: Pal
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "Pal.h"
#include "Control.h"
#include "Command.h"
#include "UdpData.h"
#include "RecvFile.h"
#include "DialogPeer.h"
#include "baling.h"
#include "utils.h"

 Pal::Pal():ipv4(0), version(NULL),
packetn(0), user(NULL), host(NULL),
name(NULL), icon(0), encode(NULL),
flags(0), dialog(NULL), mypacketn(0), reply(true)
{
	extern Control ctr;
	record = gtk_text_buffer_new(ctr.table);
}

Pal::~Pal()
{
	SendExit();

	free(version);
	free(user);
	free(host);
	free(name);
	free(encode);

	g_object_unref(record);
	if (dialog)
		gtk_widget_destroy(((DialogPeer *) dialog)->dialog);
}

// //trans 是否转换编码,true 必须;false 情况而定
void Pal::CreateInfo(in_addr_t ip, const char *msg, size_t size, bool trans)
{
	extern Control ctr;
	char *ptr;

	ipv4 = ip;
	IptuxGetIcon(msg, size);
	IptuxGetEncode(msg, size);

	if (trans || !(flags & BIT1))
		ptr = transfer_encode(msg, ctr.encode, false);
	else
		ptr = Strdup(msg);
	version = iptux_get_section_string(ptr, 0);
	packetn = 0;
	user = iptux_get_section_string(ptr, 2);
	host = iptux_get_section_string(ptr, 3);
	name = ipmsg_get_attach(ptr, 5);
	free(ptr);
	flags |= BIT2;
}

void Pal::UpdateInfo(const char *msg, size_t size, bool trans)
{
	extern Control ctr;
	char *ptr;

	if (!(flags & BIT3)) {
		IptuxGetIcon(msg, size);
		free(encode);
		IptuxGetEncode(msg, size);
	}

	if (trans || !(flags & BIT1))
		ptr = transfer_encode(msg, ctr.encode, false);
	else
		ptr = Strdup(msg);
	free(version);
	version = iptux_get_section_string(ptr, 0);
	packetn = 0;
	free(user);
	user = iptux_get_section_string(ptr, 2);
	free(host);
	host = iptux_get_section_string(ptr, 3);
	if (!(flags & BIT3)) {
		free(name);
		name = ipmsg_get_attach(ptr, 5);
	}
	free(ptr);
	flags |= BIT2;
}

void Pal::SetPalmodelValue(GtkTreeModel * model, GtkTreeIter * iter)
{
	GdkPixbuf *pixbuf;
	char file[MAX_PATH_BUF];

	snprintf(file, MAX_PATH_BUF, __ICON_DIR "/%hhu.png", icon);
	pixbuf = gdk_pixbuf_new_from_file(file, NULL);
	gtk_list_store_set(GTK_LIST_STORE(model), iter, 0, pixbuf, 1, name, 2,
			   this, -1);
	if (pixbuf)
		g_object_unref(pixbuf);
}

bool Pal::CheckReply(uint32_t packetno, bool install)
{
	if (install) {
		mypacketn = packetno;
		reply = false;
		return true;
	}

	if (reply || mypacketn > packetno)
		return true;
	return false;
}

void Pal::BufferInsertText(const char *msg, enum INSERTTYPE type)
{
	switch (type) {
	case PAL:
		gdk_threads_enter();
		BufferInsertPal(msg);
		gdk_threads_leave();
		break;
	case SELF:
		BufferInsertSelf(msg);
		break;
	case ERROR:
		gdk_threads_enter();
		BufferInsertError();
		gdk_threads_leave();
		break;
	default:
		break;
	}
}

//是否已经将消息存放在缓冲区中
bool Pal::RecvMessage(const char *msg)
{
	extern UdpData udt;
	uint32_t packetno;
	GList *tmp;
	char *ptr;

	if (!(flags & BIT2))
		return false;
	packetno = iptux_get_dec_number(msg, 1);
	if (packetno <= packetn)
		return false;
	packetn = packetno;

	ptr = ipmsg_get_attach(msg, 5);
	if (!ptr || !strlen(ptr))
		return true;

	BufferInsertText(ptr, PAL);
	if (!dialog) {
		tmp = (GList *) UdpData::PalGetMsgPos(this);
		if (!tmp) {
			pthread_mutex_lock(&udt.mutex);
			g_queue_push_tail(udt.msgqueue, this);
			pthread_mutex_unlock(&udt.mutex);
		}
	}
	return true;
}

bool Pal::RecvAskShare(const char *msg)
{
	uint32_t packetno;

	if (!(flags & BIT2))
		return false;
	packetno = iptux_get_dec_number(msg, 1);
	if (packetno <= packetn)
		return false;
	packetn = packetno;
	return true;
}

void Pal::RecvReply(const char *msg)
{
	uint32_t oldpacket;

	oldpacket = iptux_get_dec_number(msg, 5);
	if (oldpacket == mypacketn)
		reply = true;
}

void Pal::RecvFile(const char *msg, size_t size)
{
	struct RECVFILE_PARA *para;

	para = (struct RECVFILE_PARA *)Malloc(sizeof(struct RECVFILE_PARA));
	para->data = this;
	para->msg = Strndup(msg, size + 2);
	memcpy(para->msg + size, "\0\0", 2);
	thread_create(ThreadFunc(RecvFile::RecvEntry), para, FALSE);
}

void Pal::SendAnsentry()
{
	Command cmd;
	int sock;

	sock = Socket(PF_INET, SOCK_DGRAM, 0);
	cmd.SendAnsentry(sock, this);
	close(sock);

}

void Pal::SendReply(const char *msg)
{
	Command cmd;
	uint32_t packetno;
	int sock;

	packetno = iptux_get_dec_number(msg, 1);
	sock = Socket(PF_INET, SOCK_DGRAM, 0);
	cmd.SendReply(sock, this, packetno);
	close(sock);
}

void Pal::SendExit()
{
	Command cmd;
	int sock;

	sock = Socket(PF_INET, SOCK_DGRAM, 0);
	cmd.SendExit(sock, this);
	close(sock);
}

void Pal::IptuxGetIcon(const char *msg, size_t size)
{
	extern Control ctr;
	const char *ptr;
	size_t len;
	int result;

	len = strlen(msg) + 1;
	if (len < size) {
		ptr = msg + len;
		result = sscanf(ptr, "%hhu", &icon);
		if (result != 1 || icon >= Control::sum_icon)
			icon = ctr.palicon;
	} else
		icon = ctr.palicon;
}

void Pal::IptuxGetEncode(const char *msg, size_t size)
{
	extern Control ctr;
	const char *ptr;
	size_t len;

	len = strlen(msg) + 1;
	if (len >= size) {
		flags &= ~BIT1;
		encode = Strdup(ctr.encode);
		return;
	}

	len += strlen(msg + len) + 1;
	if (len >= size) {
		flags &= ~BIT1;
		encode = Strdup(ctr.encode);
		return;
	}

	ptr = msg + len;
	while (*ptr && !isalnum(*ptr))
		ptr++;
	if (*ptr) {
		ptr = strchr(getenv("LANG"), '.') + 1;
		if (strcasecmp(ptr, "UTF-8") == 0)
			flags |= BIT1;
		else
			flags &= ~BIT1;
		encode = Strdup(ptr);
	} else {
		flags &= ~BIT1;
		encode = Strdup(ctr.encode);
	}
}

void Pal::BufferInsertPal(const char *msg)
{
	char *ptr, *pptr;
	GtkTextIter iter;

	ptr = (char *)msg;
	if (!(flags & BIT1)) {
		pptr = transfer_encode(ptr, encode, false);
		free(ptr);
	} else
		pptr = ptr;

	ptr = getformattime("%s", name);
	gtk_text_buffer_get_end_iter(record, &iter);
	gtk_text_buffer_insert_with_tags_by_name(record, &iter, ptr,
						 -1, "blue", NULL);
	g_free(ptr);
	ptr = g_strdup_printf("%s\n", pptr);
	gtk_text_buffer_get_end_iter(record, &iter);
	gtk_text_buffer_insert(record, &iter, ptr, -1);
	free(pptr), g_free(ptr);

	ViewScroll();
}

void Pal::BufferInsertSelf(const char *attach)
{
	extern Control ctr;
	GtkTextIter iter;
	char *ptr;

	ptr = getformattime("%s", ctr.myname);
	gtk_text_buffer_get_end_iter(record, &iter);
	gtk_text_buffer_insert_with_tags_by_name(record, &iter, ptr,
						 -1, "green", NULL);
	g_free(ptr);
	ptr = g_strdup_printf("%s\n", attach);
	gtk_text_buffer_get_end_iter(record, &iter);
	gtk_text_buffer_insert(record, &iter, ptr, -1);
	g_free(ptr);

	ViewScroll();
}

void Pal::BufferInsertError()
{
	static char *tips[2] = {
		_("<tips>"),
		_("It isn't online,or not receive the data packet!\n")
	};
	GtkTextIter iter;
	char *ptr;

	ptr = getformattime("%s", tips[0]);
	gtk_text_buffer_get_end_iter(record, &iter);
	gtk_text_buffer_insert_with_tags_by_name(record, &iter, ptr,
						 -1, "red", NULL);
	g_free(ptr);
	gtk_text_buffer_get_end_iter(record, &iter);
	gtk_text_buffer_insert_with_tags_by_name(record, &iter, tips[1],
						 -1, "red", NULL);

	ViewScroll();
}

void Pal::ViewScroll()
{
	GtkTextIter start, end;
	GtkTextMark *mark;

	if (!dialog)
		return;

	gtk_text_buffer_get_bounds(record, &start, &end);
	if (gtk_text_iter_equal(&start, &end))
		return;
	mark = gtk_text_buffer_create_mark(record, NULL, &end, FALSE);
	gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW
				     (((DialogPeer *) dialog)->scroll), mark,
				     0.0, TRUE, 0.0, 0.0);
	gtk_text_buffer_delete_mark(record, mark);
	gtk_window_present(GTK_WINDOW(((DialogPeer *) dialog)->dialog));
}
