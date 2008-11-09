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

 UdpData::UdpData():pallist(NULL), msgqueue(NULL), pal_model(NULL)
{
	pthread_mutex_init(&mutex, NULL);
}

UdpData::~UdpData()
{
	pthread_mutex_lock(&mutex);
	g_slist_foreach(pallist, remove_each_info, GINT_TO_POINTER(PALINFO));
	g_slist_free(pallist);
	g_queue_clear(msgqueue);
	g_queue_free(msgqueue);
	pthread_mutex_unlock(&mutex);
	pthread_mutex_destroy(&mutex);

	g_object_unref(pal_model);
}

void UdpData::InitSelf()
{
	msgqueue = g_queue_new();
	pal_model = CreatePalModel();
	InitPalModel();
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

void UdpData::Ipv4GetParent(in_addr_t ipv4, GtkTreeIter *iter)
{
	extern UdpData udt;
	in_addr_t ip1, ip2;
	uint8_t count;

	gtk_tree_model_get_iter_first(udt.pal_model, iter);
	count = 0, ipv4 = ntohl(ipv4);
	do {
		if (!udt.localip[count << 1] || !udt.localip[(count << 1) + 1])
			break;
		inet_pton(AF_INET, udt.localip[count << 1], &ip1);
		ip1 = ntohl(ip1);
		inet_pton(AF_INET, udt.localip[(count << 1) + 1], &ip2);
		ip2 = ntohl(ip2);
		little_endian(&ip1, &ip2);
		if (ip1 <= ipv4 && ip2 >= ipv4)
			break;
		count++;
	} while (gtk_tree_model_iter_next(udt.pal_model, iter));
}

bool UdpData::PalGetModelIter(gpointer pal, GtkTreeIter *parent,
			      GtkTreeIter * iter)
{
	extern UdpData udt;
	gpointer data;

	if (!gtk_tree_model_iter_children(udt.pal_model, iter, parent))
		return false;

	do {
		gtk_tree_model_get(udt.pal_model, iter, 2, &data, -1);
		if (pal == data)
			return true;
	} while (gtk_tree_model_iter_next(udt.pal_model, iter));

	return false;
}

//面板 4,0 pixbuf,1 nickname,2 data,3 expand
GtkTreeModel *UdpData::CreatePalModel()
{
	GtkTreeStore *model;

	model = gtk_tree_store_new(4, GDK_TYPE_PIXBUF, G_TYPE_STRING,
		   G_TYPE_POINTER, G_TYPE_BOOLEAN);

	return GTK_TREE_MODEL(model);
}

void UdpData::InitPalModel()
{
	extern Control ctr;
	gchar ipstr[32], buf[MAX_BUF];
	GtkTreeIter iter;
	GdkPixbuf *pixbuf;
	uint8_t count;

	pixbuf = gdk_pixbuf_new_from_file(__TIP_DIR"/hide.png", NULL);
	count = 0;
	while (localip[count << 1]) {
		if (localip[(count << 1) + 1])
			snprintf(ipstr, 32, "%s~%s", localip[count << 1], localip[(count << 1) + 1]);
		else
			snprintf(ipstr, 32, "%s", localip[count << 1]);
		snprintf(buf, MAX_BUF, "<span style=\"italic\" underline=\"single\" size=\"small\" "
				"foreground=\"#52B838\" weight=\"bold\">%s</span>", ipstr);
		gtk_tree_store_append(GTK_TREE_STORE(pal_model), &iter, NULL);
		gtk_tree_store_set(GTK_TREE_STORE(pal_model), &iter, 0, pixbuf,
				   1, buf, 2, NULL, 3, FALSE, -1);
		if (!localip[(count << 1) + 1])
			break;
		count++;
	}
}

void UdpData::SomeoneEntry(in_addr_t ipv4, char *msg, size_t size)
{
	extern Control ctr;
	GtkTreeIter iter, parent;
	Pal *pal;

	pal = (Pal *) Ipv4GetPal(ipv4);
	gdk_threads_enter();
	if (!pal) {
		pal = new Pal;
		pthread_mutex_lock(&mutex);
		pallist = g_slist_append(pallist, pal);
		pthread_mutex_unlock(&mutex);
		pal->CreateInfo(ipv4, msg, size, true);
		Ipv4GetParent(ipv4, &parent);
		gtk_tree_store_append(GTK_TREE_STORE(pal_model), &iter, &parent);
	} else {
		pal->UpdateInfo(msg, size, true);
		Ipv4GetParent(ipv4, &parent);
		if (!PalGetModelIter(pal, &parent, &iter))
			gtk_tree_store_append(GTK_TREE_STORE(pal_model), &iter, &parent);
	}
	pal->SetPalmodelValue(pal_model, &iter);
	gdk_threads_leave();
	pal->SendAnsentry();
	if (strncmp(ctr.myicon, __ICON_DIR, strlen(__ICON_DIR)))
		pal->SendMyIcon();
}

void UdpData::SomeoneExit(in_addr_t ipv4, char *msg, size_t size)
{
	GtkTreeIter iter, parent;
	GSList *tmp1;
	GList *tmp2;

	tmp1 = (GSList *) Ipv4GetPalPos(ipv4);
	if (!tmp1)
		return;

	Ipv4GetParent(ipv4, &parent);
	gdk_threads_enter();
	if (PalGetModelIter(tmp1->data, &parent, &iter))
		gtk_tree_store_remove(GTK_TREE_STORE(pal_model), &iter);
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
	extern Control ctr;
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
		Ipv4GetParent(ipv4, &parent);
		gtk_tree_store_append(GTK_TREE_STORE(pal_model), &iter, &parent);
	} else {
		pal->UpdateInfo(msg, size, false);
		Ipv4GetParent(ipv4, &parent);
		if (!PalGetModelIter(pal, &parent, &iter))
			gtk_tree_store_append(GTK_TREE_STORE(pal_model), &iter, &parent);
	}
	pal->SetPalmodelValue(pal_model, &iter);
	gdk_threads_leave();
	if (strncmp(ctr.myicon, __ICON_DIR, strlen(__ICON_DIR)))
		pal->SendMyIcon();
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
		goto mark;
	}

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
	thread_create(ThreadFunc(ThreadAskShared), pal, FALSE);
}

void UdpData::SomeoneSendIcon(in_addr_t ipv4, char *msg, size_t size)
{
	GtkTreeIter iter, parent;
	Pal *pal;
	bool flag;

	pal = (Pal *) Ipv4GetPal(ipv4);
	if (!pal)
		return;

	gdk_threads_enter();
	flag = pal->RecvIcon(msg, size);
	if (flag) {
		Ipv4GetParent(ipv4, &parent);
		PalGetModelIter(pal, &parent, &iter);
		pal->SetPalmodelValue(pal_model, &iter);
	}
	gdk_threads_leave();
}

void UdpData::ThreadAskShared(gpointer data)
{
	extern Control ctr;
	extern SendFile sfl;

	if (!FLAG_ISSET(ctr.flags, 0) || AllowAskShared(data))
		sfl.SendSharedFiles(data);
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
