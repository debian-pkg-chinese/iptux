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
#include "Log.h"
#include "support.h"
#include "baling.h"
#include "utils.h"

 Pal::Pal():ipv4(0), version(NULL), packetn(0), user(NULL),
host(NULL), name(NULL), ad(NULL), sign(NULL), iconfile(NULL),
encode(NULL), flags(0), dialog(NULL), mypacketn(0), reply(true)
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
	free(ad);
	free(sign);
	free(iconfile);
	free(encode);

	if (dialog)
		gtk_widget_destroy(((DialogPeer *) dialog)->dialog);
	g_object_unref(record);
}

// //trans 是否转换编码,true 必须;false 情况而定
void Pal::CreateInfo(in_addr_t ip, const char *msg, size_t size, bool trans)
{
	extern Control ctr;
	char *ptr;

	ipv4 = ip;
	if (!IptuxGetIcon(msg, size))
		iconfile = Strdup(ctr.palicon);
	if (!IptuxGetEncode(msg, size))
		encode = Strdup(ctr.encode);

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
	bool cpt;

	if (!FLAG_ISSET(flags, 2)) {
		cpt = FLAG_ISSET(flags, 0);
		ptr = iconfile;
		if (IptuxGetIcon(msg, size))
			free(ptr);
		ptr = encode;
		if (IptuxGetEncode(msg, size))
			free(ptr);
		if (cpt)	//如果以前兼容，则此次也兼容
			FLAG_SET(flags, 0);
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

void Pal::SetPalmodelValue(GtkTreeModel * model, GtkTreeIter * iter)
{
	extern Control ctr;
	GdkPixbuf *pixbuf;
	char buf[MAX_BUF], ipstr[INET_ADDRSTRLEN];

	pixbuf = gdk_pixbuf_new_from_file_at_size(iconfile,
						  MAX_ICONSIZE, MAX_ICONSIZE,
						  NULL);
	if (!pixbuf)
		pixbuf = gdk_pixbuf_new_from_file_at_size(ctr.palicon,
							  MAX_ICONSIZE,
							  MAX_ICONSIZE, NULL);
	inet_ntop(AF_INET, &ipv4, ipstr, INET_ADDRSTRLEN);
	snprintf(buf, MAX_BUF, "%s\n%s", name, ipstr);
	gtk_tree_store_set(GTK_TREE_STORE(model), iter, 0, pixbuf, 2, buf, 3,
			   ctr.font, 4, TRUE, 6, FALSE, 7, this, -1);
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

void Pal::BufferInsertData(GSList *chiplist, enum BELONG_TYPE type)
{
	switch (type) {
	case PAL:
		gdk_threads_enter();
		BufferInsertPal(chiplist);
		gdk_threads_leave();
		break;
	case SELF:
		BufferInsertSelf(chiplist);
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
	GSList *chiplist;
	GList *tmp;
	char *ptr;

	packetno = iptux_get_dec_number(msg, 1);
	if (packetno <= packetn)
		return false;
	packetn = packetno;

	ptr = ipmsg_get_attach(msg, 5);
	if (!ptr)
		return true;

	chiplist = g_slist_append(NULL, new ChipData(STRING, ptr));
	BufferInsertData(chiplist, PAL);
	g_slist_foreach(chiplist, remove_foreach, GINT_TO_POINTER(CHIPDATA));
	g_slist_free(chiplist);

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

	packetno = iptux_get_dec_number(msg, 1);
	if (packetno <= packetn)
		return false;
	packetn = packetno;
	return true;
}

bool Pal::RecvIcon(const char *msg, size_t size)
{
	char file[MAX_PATHBUF];
	size_t len;
	int fd;

	if (FLAG_ISSET(flags, 2) || (len = strlen(msg) + 1) >= size)
		return false;
	snprintf(file, MAX_PATHBUF, "%s/.iptux/%x.ic", getenv("HOME"), ipv4);
	if ((fd = Open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1)
		return false;
	Write(fd, msg + len, size - len);
	close(fd);
	free(iconfile);
	iconfile = Strdup(file);
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
	struct recvfile_para *para;

	para = (struct recvfile_para *)Malloc(sizeof(struct recvfile_para));
	para->data = this;
	para->msg = Strndup(msg, size + 2);
	memcpy(para->msg + size, "\0\0", 2);	//防止搜索超越合法区域
	thread_create(ThreadFunc(RecvFile::RecvEntry), para, false);
}

void Pal::RecvSign(const char *msg)
{
	char *ptr;

	ptr = ipmsg_get_attach(msg, 5);
	if (!FLAG_ISSET(flags, 0)) {
		sign = transfer_encode(ptr, encode, false);
		free(ptr);
	} else
		sign = ptr;
}

void Pal::RecvAdPic(const char *path)
{
	ad = Strdup(path);
}

void Pal::RecvMsgPic(const char *path)
{
	GSList *chiplist;

	chiplist = g_slist_append(NULL, new ChipData(PICTURE, Strdup(path)));
	BufferInsertData(chiplist, PAL);
	g_slist_foreach(chiplist, remove_foreach, GINT_TO_POINTER(CHIPDATA));
	g_slist_free(chiplist);
}

void Pal::SendAnsentry()
{
	extern struct interactive inter;
	Command cmd;

	cmd.SendAnsentry(inter.udpsock, this);
}

void Pal::SendReply(const char *msg)
{
	extern struct interactive inter;
	Command cmd;
	uint32_t packetno;

	packetno = iptux_get_dec_number(msg, 1);
	cmd.SendReply(inter.udpsock, this, packetno);
}

void Pal::SendExit()
{
	extern struct interactive inter;
	Command cmd;

	cmd.SendExit(inter.udpsock, this);
}

//是否成功获得头像文件
bool Pal::IptuxGetIcon(const char *msg, size_t size)
{
	char file[MAX_PATHBUF], *ptr;
	size_t len;

	len = strlen(msg) + 1;
	if (len >= size)
		return false;
	ptr = my_getline(msg + len);
	snprintf(file, MAX_PATHBUF, __ICON_DIR "/%s", ptr);
	free(ptr);
	if (access(file, F_OK) != 0)
		return false;
	iconfile = Strdup(file);
	return true;
}

//是否成功获得系统编码
bool Pal::IptuxGetEncode(const char *msg, size_t size)
{
	const char *ptr;
	size_t len;

	FLAG_CLR(flags, 0);
	if ((len = strlen(msg) + 1) >= size ||
	    (len += strlen(msg + len) + 1) >= size)
		return false;

	ptr = msg + len;
	while (*ptr && !isalnum(*ptr))
		ptr++;
	if (*ptr == '\0')
		return false;

	FLAG_SET(flags, 0);
	encode = Strdup(ptr);
	return true;
}

void Pal::BufferInsertPal(GSList *chiplist)
{
	extern Log mylog;
	char *ptr, *pptr;
	GSList *tmp;
	GdkPixbuf *pixbuf;
	GtkTextIter iter;

	ptr = getformattime("%s", name);
	gtk_text_buffer_get_end_iter(record, &iter);
	gtk_text_buffer_insert_with_tags_by_name(record, &iter, ptr,
				-1, "blue", NULL);
	g_free(ptr);

	tmp = chiplist;
	while (tmp) {
		gtk_text_buffer_get_end_iter(record, &iter);
		ptr = ((ChipData*)tmp->data)->data;
		switch (((ChipData*)tmp->data)->type) {
		case STRING:
			if (!FLAG_ISSET(flags, 0))
				pptr = transfer_encode(ptr, encode, false);
			else
				pptr = Strdup(ptr);
			gtk_text_buffer_insert(record, &iter, pptr, -1);
			mylog.CommunicateLog(this, pptr);
			free(pptr);
			break;
		case PICTURE:
			pixbuf = gdk_pixbuf_new_from_file(ptr, NULL);
			if (pixbuf) {
				gtk_text_buffer_insert_pixbuf(record, &iter, pixbuf);
				g_object_unref(pixbuf);
			}
			break;
		default:
			break;
		}
		tmp = tmp->next;
	}

	gtk_text_buffer_get_end_iter(record, &iter);
	gtk_text_buffer_insert(record, &iter, "\n", -1);
	ViewScroll();
}

void Pal::BufferInsertSelf(GSList *chiplist)
{
	extern Control ctr;
	extern Log mylog;
	GdkPixbuf *pixbuf;
	GtkTextIter iter;
	GSList *tmp;
	char *ptr;

	ptr = getformattime("%s", ctr.myname);
	gtk_text_buffer_get_end_iter(record, &iter);
	gtk_text_buffer_insert_with_tags_by_name(record, &iter, ptr,
						 -1, "green", NULL);
	g_free(ptr);

	tmp = chiplist;
	while (tmp) {
		gtk_text_buffer_get_end_iter(record, &iter);
		ptr = ((ChipData*)tmp->data)->data;
		switch (((ChipData*)tmp->data)->type) {
		case STRING:
			gtk_text_buffer_insert(record, &iter, ptr, -1);
			mylog.CommunicateLog(NULL, ptr);
			break;
		case PICTURE:
			pixbuf = gdk_pixbuf_new_from_file(ptr, NULL);
			if (pixbuf) {
				gtk_text_buffer_insert_pixbuf(record, &iter, pixbuf);
				g_object_unref(pixbuf);
			}
			break;
		default:
			break;
		}
		tmp = tmp->next;
	}

	gtk_text_buffer_get_end_iter(record, &iter);
	gtk_text_buffer_insert(record, &iter, "\n", -1);
	ViewScroll();
}

void Pal::BufferInsertError()
{
	static char *tips[2] = {
		_("<tips>"),
		_("It isn't online,or not receive the data packet!\n")
	};
	extern Log mylog;
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
	mylog.SystemLog(_("send data packet fail!"));

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

void Pal::SendFeature(gpointer data)
{
	extern struct interactive inter;
	extern Control ctr;
	char path[MAX_PATHBUF];
	Command cmd;
	int sock;

	if (strncmp(ctr.myicon, __ICON_DIR, strlen(__ICON_DIR)))
		cmd.SendMyIcon(inter.udpsock, data);
	if (ctr.sign && *ctr.sign != '\0')
		cmd.SendMySign(inter.udpsock, data);
	snprintf(path, MAX_PATHBUF, "%s/.iptux/myad", getenv("HOME"));
	if (access(path, F_OK) == 0) {
		sock = Socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
		cmd.SendSublayer(sock, data, IPTUX_ADPICOPT, path);
		close(sock);
	}
}
