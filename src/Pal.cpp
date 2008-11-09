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
name(NULL), iconfile(NULL),encode(NULL),
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
	free(iconfile);
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

	if (trans || !FLAG_ISSET(flags, 0))
		ptr = transfer_encode(msg, ctr.encode, false);
	else
		ptr = Strdup(msg);
	version = iptux_get_section_string(ptr, 0);
	packetn = 0;
	user = iptux_get_section_string(ptr, 2);
	host = iptux_get_section_string(ptr, 3);
	name = ipmsg_get_attach(ptr, 5);
	free(ptr);
	FLAG_SET(flags, 1);
}

void Pal::UpdateInfo(const char *msg, size_t size, bool trans)
{
	extern Control ctr;
	char *ptr;

	if (!FLAG_ISSET(flags, 2)) {
		free(iconfile);
		IptuxGetIcon(msg, size);
		free(encode);
		IptuxGetEncode(msg, size);
	}

	if (trans || !FLAG_ISSET(flags, 0))
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
	if (!FLAG_ISSET(flags, 2)) {
		free(name);
		name = ipmsg_get_attach(ptr, 5);
	}
	free(ptr);
	FLAG_SET(flags, 1);
}

void Pal::SetPalmodelValue(GtkTreeModel *model, GtkTreeIter * iter)
{
	GdkPixbuf *pixbuf;
	char buf[MAX_BUF];

	pixbuf = gdk_pixbuf_new_from_file_at_size(iconfile,
			MAX_ICONSIZE, MAX_ICONSIZE, NULL);
	snprintf(buf, MAX_BUF, "<span font_family=\"PakTypeNaqsh\" "
			"foreground=\"#52B838\">%s</span>", name);
	gtk_tree_store_set(GTK_TREE_STORE(model), iter, 0, pixbuf,
			   1, buf, 2, this, -1);
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

	if (!FLAG_ISSET(flags, 1))
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
		tmp = (GList *) udt.PalGetMsgPos(this);
		if (!tmp) {
			pthread_mutex_lock(&udt.mutex);
			g_queue_push_tail(udt.msgqueue, this);
			pthread_mutex_unlock(&udt.mutex);
		}
	}
	return true;
}

bool Pal::RecvAskShared(const char *msg)
{
	uint32_t packetno;

	if (!FLAG_ISSET(flags, 1))
		return false;
	packetno = iptux_get_dec_number(msg, 1);
	if (packetno <= packetn)
		return false;
	packetn = packetno;
	return true;
}

bool Pal::RecvIcon(const char *msg, size_t size)
{
	char file[MAX_PATHBUF], *env;
	size_t len;
	int fd;

	if (FLAG_ISSET(flags, 2) || (len = strlen(msg) + 1) >= size)
		return FALSE;
	env = getenv("HOME");
	snprintf(file, MAX_PATHBUF, "%s/.iptux/%u", env, ipv4);
	if ((fd = Open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1)
			return FALSE;
	Write(fd, msg + len, size - len);
	close(fd);
	free(iconfile);
	iconfile = Strdup(file);
	return TRUE;
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
	struct recvfile_para *para;

	para = (struct recvfile_para *)Malloc(sizeof(struct recvfile_para));
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

void Pal::SendMyIcon()
{
	extern Control ctr;
	Command cmd;
	int sock;

	sock = Socket(PF_INET, SOCK_DGRAM, 0);
	cmd.SendMyIcon(sock, this);
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
	char file[MAX_PATHBUF], *ptr;
	size_t len;

	len = strlen(msg) + 1;
	if (len < size) {
		ptr = my_getline(msg + len);
		snprintf(file, MAX_PATHBUF, __ICON_DIR"/%s", ptr);
		free(ptr);
		if (access(file, F_OK) == 0) {
			iconfile = Strdup(file);
			return;
		}
	}

	iconfile = Strdup(ctr.palicon);
}

void Pal::IptuxGetEncode(const char *msg, size_t size)
{
	extern Control ctr;
	const char *ptr;
	size_t len;

	if ((len = strlen(msg) + 1) >= size ||
	    (len += strlen(msg+len) + 1) >= size) {
		FLAG_CLR(flags, 0);
		encode = Strdup(ctr.encode);
		return;
	}

	ptr = msg + len;
	while (*ptr && !isalnum(*ptr))
		ptr++;
	if (*ptr) {
		FLAG_SET(flags, 0);
		encode = Strdup(ptr);
	} else {
		FLAG_CLR(flags, 0);
		encode = Strdup(ctr.encode);
	}
}

void Pal::BufferInsertPal(const char *msg)
{
	char *ptr, *pptr;
	GtkTextIter iter;

	ptr = (char *)msg;
	if (!FLAG_ISSET(flags, 0)) {
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
