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
#include "SendFile.h"
#include "Control.h"
#include "Pal.h"
#include "output.h"
#include "baling.h"
#include "utils.h"

const char *UdpData::localip[] = {
	"10.0.0.0",
	"10.255.255.255",
	"172.16.0.0",
	"172.31.255.255",
	"192.168.0.0",
	"192.168.255.255",
	"Others",
	NULL
};

 UdpData::UdpData():pallist(NULL), msgqueue(NULL)
{
	pthread_mutex_init(&mutex, NULL);
}

UdpData::~UdpData()
{
	uint8_t count;

	pthread_mutex_lock(&mutex);
	g_slist_foreach(pallist, remove_each_info, GINT_TO_POINTER(PALINFO));
	g_slist_free(pallist);
	g_queue_clear(msgqueue);
	g_queue_free(msgqueue);
	pthread_mutex_unlock(&mutex);
	pthread_mutex_destroy(&mutex);

	count = 0;
	while (count <= sumseg) {
		g_object_unref(pal_model[count]);
		count++;
	}
}

void UdpData::InitSelf()
{
	uint8_t count;

	msgqueue = g_queue_new();

	count = 0;
	while (count <= sumseg) {
		pal_model[count] = CreatePalModel();
		count++;
	}
}

void UdpData::UdpDataEntry(in_addr_t ipv4, char *msg, size_t size)
{
	extern Control ctr;
	uint32_t commandno;
	Pal *pal;

	if (ctr.flags & BIT2 && (pal = (Pal *) Ipv4GetPal(ipv4))
	    && pal->flags & BIT4)
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
	case IPTUX_ASKSHARE:
		SomeoneAskShare(ipv4, msg, size);
		break;
	default:
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
		if (ipv4 == pal->ipv4) {
			pthread_mutex_unlock(&udt.mutex);
			return pal;
		}
		tmp = tmp->next;
	}
	pthread_mutex_unlock(&udt.mutex);

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
		if (ipv4 == pal->ipv4) {
			pthread_mutex_unlock(&udt.mutex);
			return tmp;
		}
		tmp = tmp->next;
	}
	pthread_mutex_unlock(&udt.mutex);

	return NULL;
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
		if (tmp->data == data) {
			pthread_mutex_unlock(&udt.mutex);
			return tmp;
		}
		tmp = tmp->next;
		count++;
	}
	pthread_mutex_unlock(&udt.mutex);

	return NULL;
}

GtkTreeModel *UdpData::Ipv4GetPalModel(in_addr_t ipv4)
{
	extern UdpData udt;
	in_addr_t ip1, ip2;
	uint8_t count;

	count = 0, ipv4 = ntohl(ipv4);
	while (count < udt.sumseg) {
		inet_pton(AF_INET, udt.localip[count << 1], &ip1);
		ip1 = ntohl(ip1);
		inet_pton(AF_INET, udt.localip[(count << 1) + 1], &ip2);
		ip2 = ntohl(ip2);
		little_endian(&ip1, &ip2);
		if (ip1 <= ipv4 && ip2 >= ipv4)
			return udt.pal_model[count];
		count++;
	}
	return udt.pal_model[count];
}

bool UdpData::PalGetModelIter(gpointer pal, GtkTreeModel * model,
			      GtkTreeIter * iter)
{
	gpointer data;

	if (!gtk_tree_model_get_iter_first(model, iter))
		return false;

	do {
		gtk_tree_model_get(model, iter, 2, &data, -1);
		if (pal == data)
			return true;
	} while (gtk_tree_model_iter_next(model, iter));

	return false;
}

//面板 3,0 pixbuf,1 nickname,2 data
GtkTreeModel *UdpData::CreatePalModel()
{
	GtkListStore *model;

	model = gtk_list_store_new(3, GDK_TYPE_PIXBUF, G_TYPE_STRING,
				   G_TYPE_POINTER);

	return GTK_TREE_MODEL(model);
}

void UdpData::SomeoneEntry(in_addr_t ipv4, char *msg, size_t size)
{
	Pal *pal;
	GtkTreeIter iter;
	GtkTreeModel *model;
	bool flag;

	pal = (Pal *) Ipv4GetPal(ipv4);
	gdk_threads_enter();
	if (!pal) {
		pal = new Pal;
		pthread_mutex_lock(&mutex);
		pallist = g_slist_append(pallist, pal);
		pthread_mutex_unlock(&mutex);
		pal->CreateInfo(ipv4, msg, size, true);
		model = Ipv4GetPalModel(ipv4);
		gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	} else {
		pal->UpdateInfo(msg, size, true);
		model = Ipv4GetPalModel(ipv4);
		flag = PalGetModelIter(pal, model, &iter);
		if (!flag)
			gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	}
	pal->SetPalmodelValue(model, &iter);
	gdk_threads_leave();
	pal->SendAnsentry();
}

void UdpData::SomeoneExit(in_addr_t ipv4, char *msg, size_t size)
{
	GSList *tmp1;
	GList *tmp2;
	GtkTreeModel *model;
	GtkTreeIter iter;
	bool flag;

	tmp1 = (GSList *) Ipv4GetPalPos(ipv4);
	if (!tmp1)
		return;

	model = Ipv4GetPalModel(ipv4);
	gdk_threads_enter();
	flag = PalGetModelIter(tmp1->data, model, &iter);
	if (flag)
		gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
	gdk_threads_leave();
	tmp2 = (GList *) PalGetMsgPos(tmp1->data);
	if (tmp2) {
		pthread_mutex_lock(&mutex);
		g_queue_delete_link(msgqueue, tmp2);
		pthread_mutex_unlock(&mutex);
	}
	((Pal *) tmp1->data)->flags &= ~BIT2;
}

void UdpData::SomeoneAnsentry(in_addr_t ipv4, char *msg, size_t size)
{
	Pal *pal;
	GtkTreeIter iter;
	GtkTreeModel *model;
	bool flag;

	pal = (Pal *) Ipv4GetPal(ipv4);
	gdk_threads_enter();
	if (!pal) {
		pal = new Pal;
		pthread_mutex_lock(&mutex);
		pallist = g_slist_append(pallist, pal);
		pthread_mutex_unlock(&mutex);
		pal->CreateInfo(ipv4, msg, size, false);
		model = Ipv4GetPalModel(ipv4);
		gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	} else {
		pal->UpdateInfo(msg, size, false);
		model = Ipv4GetPalModel(ipv4);
		flag = PalGetModelIter(pal, model, &iter);
		if (!flag)
			gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	}
	pal->SetPalmodelValue(model, &iter);
	gdk_threads_leave();
}

void UdpData::SomeoneAbsence(in_addr_t ipv4, char *msg, size_t size)
{
	SomeoneAnsentry(ipv4, msg, size);
}

void UdpData::SomeoneSendmsg(in_addr_t ipv4, char *msg, size_t size)
{
	uint32_t commandno;
	bool flag;
	Pal *pal;

 mark:	pal = (Pal *) Ipv4GetPal(ipv4);
	if (!pal) {
		SomeoneAnsentry(ipv4, msg, size);
		delay(1, 0);
		goto mark;
	}

	flag = pal->RecvMessage(msg);

	commandno = iptux_get_dec_number(msg, 4);
	if (GET_OPT(commandno) == IPMSG_SENDCHECKOPT)
		pal->SendReply(msg);
	if (flag && GET_OPT(commandno) == IPMSG_FILEATTACHOPT)
		pal->RecvFile(msg, size);
}

void UdpData::SomeoneRecvmsg(in_addr_t ipv4, char *msg, size_t size)
{
	Pal *pal;

 mark:	pal = (Pal *) Ipv4GetPal(ipv4);
	if (!pal) {
		SomeoneAnsentry(ipv4, msg, size);
		delay(1, 0);
		goto mark;
	}

	pal->RecvReply(msg);
}

void UdpData::SomeoneAskShare(in_addr_t ipv4, char *msg, size_t size)
{
	Pal *pal;

 mark:	pal = (Pal *) Ipv4GetPal(ipv4);
	if (!pal) {
		SomeoneAnsentry(ipv4, msg, size);
		delay(1, 0);
		goto mark;
	}

	if (!pal->RecvAskShare(msg))
		return;
	thread_create(ThreadFunc(ThreadAskShare), pal, FALSE);
}

void UdpData::ThreadAskShare(gpointer data)
{
	extern Control ctr;
	extern SendFile sfl;

	if (!(ctr.flags & BIT1) || AllowAskShare(data))
		sfl.SendShareFiles(data);
}

bool UdpData::AllowAskShare(gpointer data)
{
	extern interactive inter;
	GtkWidget *dialog, *box;
	GtkWidget *label, *image;
	char ipstr[INET_ADDRSTRLEN], *ptr;
	bool result;

	gdk_threads_enter();
	dialog = gtk_dialog_new_with_buttons(_("Request for shared resources"),
					     GTK_WINDOW(inter.window),
					     GTK_DIALOG_MODAL, _("Agree"),
					     GTK_RESPONSE_ACCEPT, _("Refuse"),
					     GTK_RESPONSE_CANCEL, NULL);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog),
					GTK_RESPONSE_ACCEPT);

	box = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), box, TRUE, TRUE,
			   0);

	image = gtk_image_new_from_stock(GTK_STOCK_DIALOG_AUTHENTICATION,
					 GTK_ICON_SIZE_DIALOG);
	gtk_widget_show(image);
	gtk_box_pack_start(GTK_BOX(box), image, FALSE, FALSE, 0);

	inet_ntop(AF_INET, &((Pal *) data)->ipv4, ipstr, INET_ADDRSTRLEN);
	ptr =
	    g_strdup_printf
	    (_
	     ("The pal (%s)[%s]\nis requesting for your shared resources,\nagree or not?"),
	     ((Pal *) data)->name, ipstr);
	label = create_label(ptr);
	free(ptr);
	gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
	gtk_label_set_line_wrap_mode(GTK_LABEL(label), PANGO_WRAP_WORD_CHAR);
	gtk_box_pack_start(GTK_BOX(box), label, TRUE, TRUE, 4);

	result = gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT;
	gtk_widget_destroy(dialog);
	gdk_threads_leave();

	return result;
}
