//
// C++ Implementation: UdpData
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "UdpData.h"
#include "MainWindow.h"
#include "SendFile.h"
#include "Control.h"
#include "Pal.h"
#include "output.h"
#include "baling.h"
#include "utils.h"

 UdpData::UdpData():pallist(NULL), msgqueue(NULL)
{
	pthread_mutex_init(&mutex, NULL);
}

UdpData::~UdpData()
{
	pthread_mutex_lock(&mutex);
	g_slist_foreach(pallist, GFunc(remove_foreach),
			GINT_TO_POINTER(PALINFO));
	g_slist_free(pallist);
	g_queue_clear(msgqueue);
	g_queue_free(msgqueue);
	pthread_mutex_unlock(&mutex);
	pthread_mutex_destroy(&mutex);
}

void UdpData::InitSelf()
{
	msgqueue = g_queue_new();
}

void UdpData::AdjustMemory()
{
	extern Control ctr;
	GSList *tmp, *list;
	Pal *pal;

	//防止申请锁的方向出现错误
	pthread_mutex_lock(&mutex);
	tmp = list = g_slist_copy(pallist);	//根据其只增不减的特性，可以采用浅拷贝方案
	pthread_mutex_unlock(&mutex);

	while (tmp) {
		pal = (Pal *) tmp->data;
		if (FLAG_ISSET(ctr.flags, 5)) {	//最小化内存使用
			if (pal->iconpix) {
				gdk_threads_enter();
				g_object_unref(pal->iconpix);
				pal->iconpix = NULL;
				gdk_threads_leave();
			}
		} else {	//以内存换取运行效率
			if (!pal->iconpix) {
				gdk_threads_enter();
				pal->iconpix = pal->GetIconPixbuf();	//返回对象必须被释放
				g_object_unref(pal->iconpix);
				gdk_threads_leave();
			}
		}
		tmp = tmp->next;
	}
	g_slist_free(list);
}

void UdpData::UdpDataEntry(in_addr_t ipv4, char *msg, size_t size)
{
	extern Control ctr;
	uint32_t commandno;
	Pal *pal;

	if (FLAG_ISSET(ctr.flags, 1) && (pal = (Pal *) Ipv4GetPal(ipv4))
	    && FLAG_ISSET(pal->flags, 3))
		return;
	commandno = iptux_get_dec_number(msg, 4);
	switch (GET_MODE(commandno)) {
	case IPMSG_BR_ENTRY:
		SomeoneEntry(ipv4, msg, size);
		break;
	case IPMSG_BR_EXIT:
		SomeoneExit(ipv4, msg, size);
		break;
	case IPMSG_ANSENTRY:
		SomeoneAnsentry(ipv4, msg, size);
		break;
	case IPMSG_BR_ABSENCE:
		SomeoneAbsence(ipv4, msg, size);
		break;
	case IPMSG_SENDMSG:
		SomeoneSendmsg(ipv4, msg, size);
		break;
	case IPMSG_RECVMSG:
		SomeoneRecvmsg(ipv4, msg, size);
		break;
	case IPTUX_ASKSHARED:
		SomeoneAskShared(ipv4, msg, size);
		break;
	case IPTUX_SENDICON:
		SomeoneSendIcon(ipv4, msg, size);
		break;
	case IPTUX_SENDSIGN:
		SomeoneSendSign(ipv4, msg, size);
		break;
	default:
		break;
	}
}

void UdpData::SublayerEntry(gpointer data, uint32_t command, const char *path)
{
	Pal *pal;

	pal = (Pal *) data;
	switch (GET_OPT(command)) {
	case IPTUX_ADPICOPT:
		pal->RecvAdPic(path);
		break;
	case IPTUX_MSGPICOPT:
		pal->RecvMsgPic(path);
		break;
	}
}

gpointer UdpData::Ipv4GetPal(in_addr_t ipv4)
{
	extern UdpData udt;
	GSList *tmp;
	Pal *pal;

	pthread_mutex_lock(&udt.mutex);
	tmp = udt.pallist;
	while (tmp) {
		pal = (Pal *) tmp->data;
		if (ipv4 == pal->ipv4)
			break;
		tmp = tmp->next;
	}
	pthread_mutex_unlock(&udt.mutex);

	if (tmp)
		return pal;
	return NULL;
}

gpointer UdpData::Ipv4GetPalPos(in_addr_t ipv4)
{
	extern UdpData udt;
	GSList *tmp;
	Pal *pal;

	pthread_mutex_lock(&udt.mutex);
	tmp = udt.pallist;
	while (tmp) {
		pal = (Pal *) tmp->data;
		if (ipv4 == pal->ipv4)
			break;
		tmp = tmp->next;
	}
	pthread_mutex_unlock(&udt.mutex);

	return tmp;
}

gpointer UdpData::PalGetMsgPos(pointer data)
{
	extern UdpData udt;
	GList *tmp;
	guint count, length;

	pthread_mutex_lock(&udt.mutex);
	tmp = udt.msgqueue->head;
	length = udt.msgqueue->length;
	count = 0;
	while (count < length) {
		if (tmp->data == data)
			break;
		tmp = tmp->next;
		count++;
	}
	pthread_mutex_unlock(&udt.mutex);

	if (count < length)
		return tmp;
	return NULL;
}

void UdpData::SomeoneLost(in_addr_t ipv4, char *msg, size_t size)
{
	extern Control ctr;
	Pal *pal;

	pal = new Pal;
	pthread_mutex_lock(&mutex);
	pallist = g_slist_append(pallist, pal);
	pthread_mutex_unlock(&mutex);

	//对底层调用函数中可能不会被初始化的数据进行预初始化
	pal->ipv4 = ipv4;
	pal->name = Strdup(_("mysterious"));
	pal->group = Strdup(_("mysterious"));
	pal->iconfile = Strdup(ctr.palicon);
	pal->encode = Strdup(ctr.encode);
	FLAG_SET(pal->flags, 2);

	SomeoneAbsence(ipv4, msg, size);
}

void UdpData::SomeoneEntry(in_addr_t ipv4, char *msg, size_t size)
{
	extern MainWindow *mwp;
	GtkTreeIter iter;
	Pal *pal;

	pal = (Pal *) Ipv4GetPal(ipv4);
	gdk_threads_enter();
	if (!pal) {
		pal = new Pal;
		pthread_mutex_lock(&mutex);
		pallist = g_slist_append(pallist, pal);
		pthread_mutex_unlock(&mutex);
		pal->CreateInfo(ipv4, msg, size, true);
		mwp->AttachItemToModel(ipv4, &iter);
	} else {
		pal->UpdateInfo(msg, size, true);
		if (!mwp->PalGetModelIter(pal, &iter))
			mwp->AttachItemToModel(ipv4, &iter);
	}
	mwp->SetValueToModel(pal, &iter);
	gdk_threads_leave();
	pal->SendAnsentry();
	if (FLAG_ISSET(pal->flags, 0))
		thread_create(ThreadFunc(Pal::SendFeature), pal, false);
}

void UdpData::SomeoneExit(in_addr_t ipv4, char *msg, size_t size)
{
	extern MainWindow *mwp;
	GSList *tmp1;
	GList *tmp2;

	tmp1 = (GSList *) Ipv4GetPalPos(ipv4);
	if (!tmp1)
		return;

	gdk_threads_enter();
	mwp->DelItemFromModel(tmp1->data);
	gdk_threads_leave();
	tmp2 = (GList *) PalGetMsgPos(tmp1->data);
	if (tmp2) {
		pthread_mutex_lock(&mutex);
		g_queue_delete_link(msgqueue, tmp2);
		pthread_mutex_unlock(&mutex);
	}
	FLAG_CLR(((Pal *) tmp1->data)->flags, 1);
}

void UdpData::SomeoneAnsentry(in_addr_t ipv4, char *msg, size_t size)
{
	Pal *pal;

	SomeoneAbsence(ipv4, msg, size);
	if (!(pal = (Pal *) Ipv4GetPal(ipv4)))
		return;
	if (FLAG_ISSET(pal->flags, 0))
		thread_create(ThreadFunc(Pal::SendFeature), pal, false);
}

void UdpData::SomeoneAbsence(in_addr_t ipv4, char *msg, size_t size)
{
	extern MainWindow *mwp;
	GtkTreeIter iter, parent;
	Pal *pal;

	pal = (Pal *) Ipv4GetPal(ipv4);
	gdk_threads_enter();
	if (!pal) {
		pal = new Pal;
		pthread_mutex_lock(&mutex);
		pallist = g_slist_append(pallist, pal);
		pthread_mutex_unlock(&mutex);
		pal->CreateInfo(ipv4, msg, size, false);
		mwp->AttachItemToModel(ipv4, &iter);
	} else {
		pal->UpdateInfo(msg, size, false);
		if (!mwp->PalGetModelIter(pal, &iter))
			mwp->AttachItemToModel(ipv4, &iter);
	}
	mwp->SetValueToModel(pal, &iter);
	gdk_threads_leave();
}

void UdpData::SomeoneSendmsg(in_addr_t ipv4, char *msg, size_t size)
{
	uint32_t commandno;
	bool flag;
	Pal *pal;

	pal = (Pal *) Ipv4GetPal(ipv4);
	if (!pal) {
		SomeoneLost(ipv4, msg, size);
		if (!(pal = (Pal *) Ipv4GetPal(ipv4)))
			return;
	}

	flag = pal->RecvMessage(msg);

	commandno = iptux_get_dec_number(msg, 4);
	if (commandno & IPMSG_SENDCHECKOPT)
		pal->SendReply(msg);
	if (flag && (commandno & IPMSG_FILEATTACHOPT))
		pal->RecvFile(msg, size);
}

void UdpData::SomeoneRecvmsg(in_addr_t ipv4, char *msg, size_t size)
{
	Pal *pal;

	pal = (Pal *) Ipv4GetPal(ipv4);
	if (!pal)
		return;

	pal->RecvReply(msg);
}

void UdpData::SomeoneAskShared(in_addr_t ipv4, char *msg, size_t size)
{
	Pal *pal;

	pal = (Pal *) Ipv4GetPal(ipv4);
	if (!pal)
		return;

	if (!pal->RecvAskShared(msg))
		return;
	thread_create(ThreadFunc(ThreadAskShared), pal, false);
}

void UdpData::SomeoneSendIcon(in_addr_t ipv4, char *msg, size_t size)
{
	extern MainWindow *mwp;
	GtkTreeIter iter, parent;
	Pal *pal;
	bool flag;

	pal = (Pal *) Ipv4GetPal(ipv4);
	if (!pal)
		return;

	flag = pal->RecvIcon(msg, size);
	if (flag) {
		gdk_threads_enter();
		mwp->PalGetModelIter(pal, &iter);
		mwp->SetValueToModel(pal, &iter);
		gdk_threads_leave();
	}
}

void UdpData::SomeoneSendSign(in_addr_t ipv4, char *msg, size_t size)
{
	Pal *pal;

	pal = (Pal *) Ipv4GetPal(ipv4);
	if (!pal) {
		SomeoneLost(ipv4, msg, size);
		if (!(pal = (Pal *) Ipv4GetPal(ipv4)))
			return;
	}
	pal->RecvSign(msg);
}

void UdpData::ThreadAskShared(gpointer data)
{
	extern Control ctr;
	extern SendFile sfl;

	if (!FLAG_ISSET(ctr.flags, 0) || AllowAskShared(data))
		sfl.SendSharedInfo(data);
}

bool UdpData::AllowAskShared(gpointer data)
{
	extern struct interactive inter;
	GtkWidget *dialog, *box;
	GtkWidget *label, *image;
	char ipstr[INET_ADDRSTRLEN], *ptr;
	bool result;

	gdk_threads_enter();
	dialog = gtk_dialog_new_with_buttons(_("Request for shared resources"),
					     GTK_WINDOW(inter.window),
					     GTK_DIALOG_MODAL, _("Refuse"),
					     GTK_RESPONSE_CANCEL, _("Agree"),
					     GTK_RESPONSE_ACCEPT, NULL);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog),
					GTK_RESPONSE_ACCEPT);

	box = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), box, TRUE, TRUE, 0);

	image = gtk_image_new_from_stock(GTK_STOCK_DIALOG_AUTHENTICATION,
							 GTK_ICON_SIZE_DIALOG);
	gtk_widget_show(image);
	gtk_box_pack_start(GTK_BOX(box), image, FALSE, FALSE, 0);

	inet_ntop(AF_INET, &((Pal *) data)->ipv4, ipstr, INET_ADDRSTRLEN);
	ptr = g_strdup_printf(
			        _("The pal (%s)[%s]\nis requesting for "
				"your shared resources,\nagree or not?"),
			       ((Pal *) data)->name, ipstr);
	label = create_label(ptr);
	free(ptr);
	gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
	gtk_label_set_line_wrap_mode(GTK_LABEL(label), PANGO_WRAP_WORD_CHAR);
	gtk_box_pack_start(GTK_BOX(box), label, TRUE, TRUE, 4);

	result = (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT);
	gtk_widget_destroy(dialog);
	gdk_threads_leave();

	return result;
}
