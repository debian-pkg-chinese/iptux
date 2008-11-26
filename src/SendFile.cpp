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
#include "baling.h"
#include "utils.h"

 SendFile::SendFile():dirty(false), pbn(0),
prn(MAX_SHAREDFILE), pblist(NULL), prlist(NULL)
{
	pthread_mutex_init(&mutex, NULL);
}

SendFile::~SendFile()
{
	pthread_mutex_lock(&mutex);
	g_slist_foreach(pblist, remove_foreach, GINT_TO_POINTER(FILEINFO));
	g_slist_free(pblist);
	g_slist_foreach(prlist, remove_foreach, GINT_TO_POINTER(FILEINFO));
	g_slist_free(prlist);
	pthread_mutex_unlock(&mutex);
	pthread_mutex_destroy(&mutex);
}

void SendFile::InitSelf()
{
	GConfClient *client;
	GSList *filelist, *tmp;
	FileInfo *file;
	struct stat64 st;

	client = gconf_client_get_default();
	filelist = gconf_client_get_list(client, GCONF_PATH"/shared_file_list",
			      GCONF_VALUE_STRING, NULL);
	pthread_mutex_lock(&mutex);
	pblist = NULL, tmp = filelist;
	while (tmp) {
		if (Stat((const char*)tmp->data, &st) == -1 ||
			  !S_ISREG(st.st_mode) && !S_ISDIR(st.st_mode)) {
			free(tmp->data);
			continue;
		}
		file = new FileInfo(pbn, (char*)tmp->data, (uint32_t) st.st_size,
				    S_ISREG(st.st_mode) ? IPMSG_FILE_REGULAR :
						    IPMSG_FILE_DIR);
		pblist = g_slist_append(pblist, file);
		tmp = tmp->next, pbn++;
	}
	pthread_mutex_unlock(&mutex);
	g_slist_free(filelist);

	dirty = true;
}

void SendFile::WriteShared()
{
	GConfClient *client;
	GSList *filelist, *tmp;

	pthread_mutex_lock(&mutex);
	filelist = NULL, tmp = pblist;
	while (tmp) {
		filelist = g_slist_append(filelist, ((FileInfo*)tmp->data)->filename);
		tmp = tmp->next;
	}
	pthread_mutex_unlock(&mutex);
	client = gconf_client_get_default();
	gconf_client_set_list(client,GCONF_PATH"/shared_file_list",
			      GCONF_VALUE_STRING, filelist, NULL);
	g_slist_free(filelist);

	dirty = false;
}

void SendFile::RequestEntry(int sock)
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
		sfl.RequestData(sock, IPMSG_FILE_REGULAR, buf);
		break;
	case IPMSG_GETDIRFILES:
		sfl.RequestData(sock, IPMSG_FILE_DIR, buf);
		break;
	default:
		break;
	}

	close(sock);
}

void SendFile::SendRegular(gpointer data)
{
	extern SendFile sfl;

	sfl.PickFile(IPMSG_FILE_REGULAR, data);
}

void SendFile::SendFolder(gpointer data)
{
	extern SendFile sfl;

	sfl.PickFile(IPMSG_FILE_DIR, data);
}

void SendFile::SendShared(gpointer data)
{
	extern struct interactive inter;
	extern SendFile sfl;
	char buf[MAX_UDPBUF], *ptr, *filename;
	GSList *tmp, *tmp1;
	Command cmd;
	FileInfo *file;
	size_t len;

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
			 file->fileid, filename, file->filesize, 0, file->fileattr);
		free(filename), len += strlen(ptr), ptr = buf + len;
		tmp = tmp->next;
	}
	pthread_mutex_unlock(&sfl.mutex);

	cmd.SendSharedInfo(inter.udpsock, data, buf);
}

void SendFile::RequestData(int sock, uint32_t fileattr, char *buf)
{
	extern Transport trans;
	extern UdpData udt;
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
	file = (FileInfo *) FindFileinfo(fileid);
	if (!file || GET_MODE(file->fileattr) != GET_MODE(fileattr))
		return;

	len = sizeof(addr);
	getpeername(sock, (SA *) & addr, &len);
	if (!(pal = (Pal *) udt.Ipv4GetPal(addr.sin_addr.s_addr)))
		return;

	ptr = number_to_string(file->filesize);
	filename = ipmsg_set_filename_self(file->filename);
	gdk_threads_enter();
	pixbuf = gdk_pixbuf_new_from_file(__TIP_DIR "/send.png", NULL);
	gtk_list_store_append(GTK_LIST_STORE(trans.trans_model), &iter);
	gtk_list_store_set(GTK_LIST_STORE(trans.trans_model), &iter,
			   0, pixbuf, 1, _("send"), 2, filename, 3, pal->name,
			   4, "0B", 5, ptr, 6, "0B/s", 7, 0, 8, FALSE, 9, 0,
			   10, file->fileid, 11, file->filesize, 12, file->fileattr,
			   13, file->filename, 14, pal, -1);
	if (pixbuf)
		g_object_unref(pixbuf);
	gdk_threads_leave();
	if (filename != file->filename)
		*(file->filename + strlen(file->filename)) = '/';
	free(ptr);

	Transport::SendFileEntry(sock, &iter, fileattr);
}

void SendFile::PickFile(uint32_t fileattr, gpointer data)
{
	extern struct interactive inter;
	GtkFileChooserAction action;
	GtkWidget *dialog, *parent;
	gchar *title;
	GSList *list;
	Pal *pal;

	pal = (Pal *) data;
	parent = pal->dialog ? ((DialogPeer *) pal->dialog)->dialog :
			inter.window;
	title = (GET_MODE(fileattr) == IPMSG_FILE_REGULAR) ?
			_("Choose sending files"):
			_("Choose sending folders");
	action = (GET_MODE(fileattr) == IPMSG_FILE_REGULAR) ?
			GTK_FILE_CHOOSER_ACTION_OPEN:
			GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;

	dialog = gtk_file_chooser_dialog_new(title, GTK_WINDOW(parent), action,
				   GTK_STOCK_OK, GTK_RESPONSE_OK,
				   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				   NULL);
	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), getenv("HOME"));

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		list = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(dialog));
		SendFileInfo(list, data);
		g_slist_free(list);	//他处释放
	}
	gtk_widget_destroy(dialog);
}

void SendFile::SendFileInfo(GSList * list, gpointer data)
{
	extern struct interactive inter;
	char buf[MAX_UDPBUF], *ptr, *filename;
	struct stat64 st;
	Command cmd;
	FileInfo *file;
	size_t len;

	ptr = buf, len = 0, buf[0] = '\0';
	pthread_mutex_lock(&mutex);
	while (list) {
		if (Stat((char *)list->data, &st) == -1) {
			list = list->next, free(list->data);
			continue;
		}
		filename = ipmsg_set_filename_pal((char *)list->data);
		snprintf(ptr, MAX_UDPBUF - len, "%u:%s:%x:%x:%x\a:",
			 prn, filename, (uint32_t) st.st_size, st.st_mtime,
			 S_ISREG(st.st_mode) ? IPMSG_FILE_REGULAR :
						    IPMSG_FILE_DIR);
		free(filename), len += strlen(ptr), ptr = buf + len;
		file = new FileInfo(prn, (char *)list->data, (uint32_t)st.st_size,
			    S_ISREG(st.st_mode) ? IPMSG_FILE_REGULAR :
						    IPMSG_FILE_DIR);
		prlist = g_slist_prepend(prlist, file);
		list = list->next, prn++;
	}
	pthread_mutex_unlock(&mutex);

	cmd.SendSharedInfo(inter.udpsock, data, buf);
}

pointer SendFile::FindFileinfo(uint32_t fileid)
{
	GSList *tmp;

	pthread_mutex_lock(&mutex);
	tmp = (fileid < MAX_SHAREDFILE)?pblist:prlist;
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
