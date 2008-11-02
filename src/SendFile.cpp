//
// C++ Implementation: SendFile
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "SendFile.h"
#include "Command.h"
#include "UdpData.h"
#include "Transport.h"
#include "DialogPeer.h"
#include "output.h"
#include "baling.h"
#include "utils.h"

 SendFile::SendFile():dirty(false), pbn(0),
prn(MAX_SHARE_FILE), pblist(NULL), prlist(NULL)
{
	pthread_mutex_init(&mutex, NULL);
}

SendFile::~SendFile()
{
	pthread_mutex_lock(&mutex);
	g_slist_foreach(pblist, remove_each_info, GINT_TO_POINTER(FILEINFO));
	g_slist_free(pblist);
	g_slist_foreach(prlist, remove_each_info, GINT_TO_POINTER(FILEINFO));
	g_slist_free(prlist);
	pthread_mutex_unlock(&mutex);
	pthread_mutex_destroy(&mutex);
}

void SendFile::InitSelf()
{
	char buf[MAX_PATH_BUF];
	struct stat64 st;
	char *ptr, *tmp, *ev;
	FileInfo *file;
	FILE *stream;
	size_t n;

	dirty = false;
	ev = getenv("HOME");
	snprintf(buf, MAX_PATH_BUF, "%s/.iptux/share", ev);
	if (access(buf, F_OK) != 0 || !(stream = Fopen(buf, "r")))
		return;
	ptr = NULL, n = 0;
	pthread_mutex_lock(&mutex);
	while (getline(&ptr, &n, stream) > 0) {
		tmp = my_getline(ptr);
		if (Stat(tmp, &st) == -1 ||
		    !S_ISREG(st.st_mode) && !S_ISDIR(st.st_mode)) {
			dirty = true;
			continue;
		}
		file = new FileInfo(pbn, tmp, (uint32_t) st.st_size,
				    S_ISREG(st.st_mode) ? IPMSG_FILE_REGULAR :
				    IPMSG_FILE_DIR);
		pblist = g_slist_append(pblist, file);
		pbn++;
	}
	pthread_mutex_unlock(&mutex);
	free(ptr);
	fclose(stream);
}

void SendFile::WriteShare()
{
	char buf[MAX_PATH_BUF], bufbak[MAX_PATH_BUF];
	FILE *stream;
	GSList *tmp;
	char *ev;

	ev = getenv("HOME");
	snprintf(buf, MAX_PATH_BUF, "%s/.iptux", ev);
	if (access(buf, F_OK) != 0)
		Mkdir(buf, 0777);

	snprintf(bufbak, MAX_PATH_BUF, "%s/.iptux/share~", ev);
	if (!(stream = Fopen(bufbak, "w")))
		return;
	pthread_mutex_lock(&mutex);
	tmp = pblist;
	while (tmp) {
		fprintf(stream, "%s\n", ((FileInfo *) tmp->data)->filename);
		tmp = tmp->next;
	}
	pthread_mutex_unlock(&mutex);
	fclose(stream);

	snprintf(buf, MAX_PATH_BUF, "%s/.iptux/share", ev);
	rename(bufbak, buf);

	dirty = false;
}

void SendFile::SendShareFiles(gpointer data)
{
	extern SendFile sfl;
	char buf[MAX_UDPBUF], *ptr, *filename;
	GSList *tmp, *tmp1;
	Command cmd;
	FileInfo *file;
	size_t len;
	int sock;

	ptr = buf, len = 0, buf[0] = '\0';
	pthread_mutex_lock(&sfl.mutex);
	tmp = sfl.pblist;
	while (tmp) {
		file = (FileInfo *) tmp->data;
		if (access(file->filename, F_OK) == -1) {
			delete file, tmp1 = tmp->next;
			sfl.pblist = g_slist_delete_link(sfl.pblist, tmp);
			tmp = tmp1;
			continue;
		}
		filename = ipmsg_set_filename_pal(file->filename);
		snprintf(ptr, MAX_UDPBUF - len, "%u:%s:%x:%x:%x\a:",
			 file->fileid, filename, file->filesize, 0,
			 file->fileattr);
		free(filename), len += strlen(ptr), ptr = buf + len;
		tmp = tmp->next;
	}
	pthread_mutex_unlock(&sfl.mutex);

	sock = Socket(PF_INET, SOCK_DGRAM, 0);
	cmd.SendShareInfo(sock, data, buf);
	close(sock);
}

void SendFile::AddSendFile(GSList * list, gpointer data)
{
	char buf[MAX_UDPBUF], *ptr, *filename;
	struct stat64 st;
	Command cmd;
	FileInfo *file;
	size_t len;
	int sock;

	ptr = buf, len = 0, buf[0] = '\0';
	while (list) {
		if (Stat((char *)list->data, &st) == -1) {
			list = list->next;
			continue;
		}
		filename = ipmsg_set_filename_pal((char *)list->data);
		snprintf(ptr, MAX_UDPBUF - len, "%u:%s:%x:%x:%x\a:", prn,
			 filename, (uint32_t) st.st_size, st.st_mtime,
			 S_ISREG(st.
				 st_mode) ? IPMSG_FILE_REGULAR :
			 IPMSG_FILE_DIR);
		free(filename), len += strlen(ptr), ptr = buf + len;
		file = new FileInfo(prn, Strdup((char *)list->data), st.st_size,
				    S_ISREG(st.
					    st_mode) ? IPMSG_FILE_REGULAR :
				    IPMSG_FILE_DIR);
		pthread_mutex_lock(&mutex);
		prlist = g_slist_prepend(prlist, file);
		pthread_mutex_unlock(&mutex);
		prn++, list = list->next;
	}

	sock = Socket(PF_INET, SOCK_DGRAM, 0);
	cmd.SendShareInfo(sock, data, buf);
	close(sock);
}

void SendFile::TcpDataEntry(int sock)
{
	extern SendFile sfl;
	char buf[MAX_SOCKBUF];
	uint32_t commandno;
	ssize_t size;

	//不处理偏移量
	size = my_read1(sock, buf, MAX_SOCKBUF, 6);
	if (size <= 0) {
		close(sock);
		return;
	}
	commandno = iptux_get_dec_number(buf, 4);
	switch (GET_MODE(commandno)) {
	case IPMSG_GETFILEDATA:
		sfl.SendFileData(sock, buf);
		break;
	case IPMSG_GETDIRFILES:
		sfl.SendDirFiles(sock, buf);
		break;
	default:
		break;
	}

	close(sock);
}

void SendFile::SendRegular(gpointer data)
{
	extern SendFile sfl;
	extern interactive inter;
	GtkWidget *dialog, *widget;
	GSList *list;
	char *ev;
	Pal *pal;

	pal = (Pal *) data;
	widget = pal->dialog ? ((DialogPeer *) pal->dialog)->dialog :
	    inter.window;
	dialog =
	    gtk_file_chooser_dialog_new(_("Choose sending files"),
					GTK_WINDOW(widget),
					GTK_FILE_CHOOSER_ACTION_OPEN,
					GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					NULL);
	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);
	ev = getenv("HOME");
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), ev);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		list = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(dialog));
		sfl.AddSendFile(list, data);
		g_slist_foreach(list, remove_each_info,
				GINT_TO_POINTER(UNKNOWN));
		g_slist_free(list);
	}
	gtk_widget_destroy(dialog);
}

void SendFile::SendFolder(gpointer data)
{
	extern SendFile sfl;
	extern interactive inter;
	GtkWidget *dialog, *widget;
	GSList *list;
	char *ev;
	Pal *pal;

	pal = (Pal *) data;
	widget = pal->dialog ? ((DialogPeer *) pal->dialog)->dialog :
	    inter.window;
	dialog =
	    gtk_file_chooser_dialog_new(_("Choose sending folders"),
					GTK_WINDOW(widget),
					GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
					GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					NULL);
	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);
	ev = getenv("HOME");
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), ev);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		list = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(dialog));
		sfl.AddSendFile(list, data);
		g_slist_foreach(list, remove_each_info,
				GINT_TO_POINTER(UNKNOWN));
		g_slist_free(list);
	}
	gtk_widget_destroy(dialog);
}

void SendFile::SendFileData(int sock, char *buf)
{
	extern Transport trans;
	const char *filename;
	GdkPixbuf *pixbuf;
	GtkTreeIter iter;
	uint32_t fileid;
	socklen_t len;
	FileInfo *file;
	char *ptr;
	Pal *pal;
	SI addr;

	fileid = iptux_get_hex_number(buf, 6);
	file = (FileInfo *) FindSendFileinfo(fileid);
	if (!file || GET_MODE(file->fileattr) != IPMSG_FILE_REGULAR)
		return;

	len = sizeof(addr);
	getpeername(sock, (SA *) & addr, &len);
	pal = (Pal *) UdpData::Ipv4GetPal(addr.sin_addr.s_addr);
	if (!pal)
		return;

	ptr = number_to_string(file->filesize);
	filename = ipmsg_set_filename_self(file->filename);
	gdk_threads_enter();
	pixbuf = gdk_pixbuf_new_from_file(__TIP_DIR "/send.png", NULL);
	gtk_list_store_append(GTK_LIST_STORE(trans.trans_model), &iter);
	gtk_list_store_set(GTK_LIST_STORE(trans.trans_model), &iter,
			   0, pixbuf, 1, _("send"), 2, filename, 3, pal->name,
			   4, "0B", 5, ptr, 6, "0B/s", 7, 0, 8, FALSE, 9, 0,
			   10, file->fileid, 11, file->filesize, 12,
			   file->fileattr, 13, file->filename, 14, pal, -1);
	if (pixbuf)
		g_object_unref(pixbuf);
	gdk_threads_leave();
	if (filename != file->filename)
		*(file->filename + strlen(file->filename)) = '/';
	free(ptr);

	Transport::SendFileEntry(sock, &iter, file->fileattr);
}

void SendFile::SendDirFiles(int sock, char *buf)
{
	extern Transport trans;
	const char *filename;
	GdkPixbuf *pixbuf;
	GtkTreeIter iter;
	uint32_t fileid;
	socklen_t len;
	FileInfo *file;
	char *ptr;
	Pal *pal;
	SI addr;

	fileid = iptux_get_hex_number(buf, 6);
	file = (FileInfo *) FindSendFileinfo(fileid);
	if (!file || GET_MODE(file->fileattr) != IPMSG_FILE_DIR)
		return;

	len = sizeof(addr);
	getpeername(sock, (SA *) & addr, &len);
	pal = (Pal *) UdpData::Ipv4GetPal(addr.sin_addr.s_addr);
	if (!pal)
		return;

	ptr = number_to_string(file->filesize);
	filename = ipmsg_set_filename_self(file->filename);
	gdk_threads_enter();
	pixbuf = gdk_pixbuf_new_from_file(__TIP_DIR "/send.png", NULL);
	gtk_list_store_append(GTK_LIST_STORE(trans.trans_model), &iter);
	gtk_list_store_set(GTK_LIST_STORE(trans.trans_model), &iter,
			   0, pixbuf, 1, _("send"), 2, filename, 3, pal->name,
			   4, "0B", 5, ptr, 6, "0B/s", 7, 0, 8, FALSE, 9, 0,
			   10, file->fileid, 11, file->filesize, 12,
			   file->fileattr, 13, file->filename, 14, pal, -1);
	if (pixbuf)
		g_object_unref(pixbuf);
	gdk_threads_leave();
	if (filename != file->filename)
		*(file->filename + strlen(file->filename)) = '/';
	free(ptr);

	Transport::SendFileEntry(sock, &iter, file->fileattr);
}

pointer SendFile::FindSendFileinfo(uint32_t fileid)
{
	extern SendFile sfl;
	GSList *tmp;

	pthread_mutex_lock(&mutex);
	if (fileid >= MAX_SHARE_FILE)
		tmp = sfl.prlist;
	else
		tmp = sfl.pblist;
	while (tmp) {
		if (((FileInfo *) tmp->data)->fileid == fileid) {
			pthread_mutex_unlock(&mutex);
			return tmp->data;
		}
		tmp = tmp->next;
	}
	pthread_mutex_unlock(&mutex);

	return NULL;
}
