//
// C++ Implementation: DialogPeer
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "DialogPeer.h"
#include "SendFile.h"
#include "AboutIptux.h"
#include "MainWindow.h"
#include "UdpData.h"
#include "Control.h"
#include "Command.h"
#include "baling.h"
#include "support.h"
#include "output.h"
#include "utils.h"

 DialogPeer::DialogPeer(gpointer data):pal((Pal *) data),
dialog(NULL), focus(NULL), scroll(NULL),
accel(NULL)
{
	pal->dialog = this;
}

DialogPeer::~DialogPeer()
{
	GtkTextIter start, end;

	pal->dialog = NULL;
	g_object_unref(accel);
	gtk_text_buffer_get_bounds(pal->record, &start, &end);
	gtk_text_buffer_delete(pal->record, &start, &end);
}

void DialogPeer::DialogEntry(gpointer data)
{
	DialogPeer *peer;

	if (DialogPeer::CheckExist(data))
		return;
	peer = new DialogPeer(data);
	peer->CreateDialog();
	peer->CreateAllArea();
}

void DialogPeer::CreateDialog()
{
	GdkColor color = { 8, 39321, 41634, 65535 };
	GtkTargetEntry target = { "text/plain", 0, 0 };
	gchar *title;

	title = g_strdup_printf(_("Communicate with %s"), pal->name);
	dialog = create_window(title, 162, 111);
	g_free(title);
	gtk_widget_modify_bg(dialog, GTK_STATE_NORMAL, &color);
	gtk_drag_dest_set(dialog, GTK_DEST_DEFAULT_ALL,
			  &target, 1, GDK_ACTION_MOVE);
	g_signal_connect_swapped(dialog, "drag-data-received",
				 G_CALLBACK(DragDataReceived), pal);
	accel = gtk_accel_group_new();
	gtk_window_add_accel_group(GTK_WINDOW(dialog), accel);
	g_signal_connect_swapped(dialog, "destroy", G_CALLBACK(DialogDestroy),
				 this);
}

void DialogPeer::CreateAllArea()
{
	extern Control ctr;
	GtkWidget *box;
	GtkWidget *hpaned, *vpaned;

	box = create_box();
	gtk_container_add(GTK_CONTAINER(dialog), box);
	gtk_box_pack_start(GTK_BOX(box), CreateMenuBar(), FALSE, FALSE, 0);
	hpaned = create_paned(FALSE);
	gtk_paned_set_position(GTK_PANED(hpaned), GINT(ctr.pix * 107));
	gtk_box_pack_end(GTK_BOX(box), hpaned, TRUE, TRUE, 0);
	CreateInfoArea(hpaned);

	vpaned = create_paned();
	gtk_paned_set_position(GTK_PANED(vpaned), GINT(ctr.pix * 67));
	gtk_paned_pack1(GTK_PANED(hpaned), vpaned, TRUE, TRUE);
	CreateRecordArea(vpaned);
	CreateInputArea(vpaned);
}

void DialogPeer::CreateInfoArea(GtkWidget * paned)
{
	GdkColor color = { 8, 65535, 65535, 55000 };
	GtkWidget *view, *frame, *sw;
	GtkTextBuffer *info;

	view = create_text_view();
	gtk_widget_modify_base(view, GTK_STATE_NORMAL, &color);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(view), FALSE);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view), GTK_WRAP_NONE);
	info = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
	FillInfoBuffer(info);
	frame = create_frame(_("Pal's Infomation"));
	gtk_paned_pack2(GTK_PANED(paned), frame, FALSE, TRUE);
	sw = create_scrolled_window();
	gtk_container_add(GTK_CONTAINER(frame), sw);
	gtk_container_add(GTK_CONTAINER(sw), view);
}

void DialogPeer::FillInfoBuffer(GtkTextBuffer * info)
{
	char buf[MAX_BUF], ipstr[INET_ADDRSTRLEN];
	GdkPixbuf *pixbuf;
	GtkTextIter iter;

	pixbuf = gdk_pixbuf_new_from_file_at_size(pal->iconfile,
						  MAX_ICONSIZE, MAX_ICONSIZE,
						  NULL);
	if (pixbuf) {
		gtk_text_buffer_get_end_iter(info, &iter);
		gtk_text_buffer_insert_pixbuf(info, &iter, pixbuf);
		g_object_unref(pixbuf);
	}

	snprintf(buf, MAX_BUF, _("\nVersion: %s\n"), pal->version);
	gtk_text_buffer_get_end_iter(info, &iter);
	gtk_text_buffer_insert(info, &iter, buf, -1);

	snprintf(buf, MAX_BUF, _("Nickname: %s\n"), pal->name);
	gtk_text_buffer_get_end_iter(info, &iter);
	gtk_text_buffer_insert(info, &iter, buf, -1);

	snprintf(buf, MAX_BUF, _("User: %s\n"), pal->user);
	gtk_text_buffer_get_end_iter(info, &iter);
	gtk_text_buffer_insert(info, &iter, buf, -1);

	snprintf(buf, MAX_BUF, _("Host: %s\n"), pal->host);
	gtk_text_buffer_get_end_iter(info, &iter);
	gtk_text_buffer_insert(info, &iter, buf, -1);

	inet_ntop(AF_INET, &pal->ipv4, ipstr, INET_ADDRSTRLEN);
	snprintf(buf, MAX_BUF, _("Host IP: %s\n"), ipstr);
	gtk_text_buffer_get_end_iter(info, &iter);
	gtk_text_buffer_insert(info, &iter, buf, -1);

	if (!FLAG_ISSET(pal->flags, 0))
		snprintf(buf, MAX_BUF, _("Compatibility: Microsoft\n"));
	else
		snprintf(buf, MAX_BUF, _("Compatibility: GNU/Linux\n"));
	gtk_text_buffer_get_end_iter(info, &iter);
	gtk_text_buffer_insert(info, &iter, buf, -1);

	snprintf(buf, MAX_BUF, _("System Encode: %s\n"), pal->encode);
	gtk_text_buffer_get_end_iter(info, &iter);
	gtk_text_buffer_insert(info, &iter, buf, -1);
}

void DialogPeer::CreateRecordArea(GtkWidget * paned)
{
	GtkWidget *frame, *sw;

	scroll = create_text_view();
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(scroll), FALSE);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(scroll), FALSE);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(scroll), pal->record);
	frame = create_frame(_("Chat History"));
	gtk_paned_pack1(GTK_PANED(paned), frame, TRUE, TRUE);
	sw = create_scrolled_window();
	gtk_container_add(GTK_CONTAINER(frame), sw);
	gtk_container_add(GTK_CONTAINER(sw), scroll);
}

void DialogPeer::CreateInputArea(GtkWidget * paned)
{
	GtkWidget *frame, *sw;
	GtkWidget *vbox, *hbb, *button;

	frame = create_frame(_("Input Your Message"));
	gtk_paned_pack2(GTK_PANED(paned), frame, FALSE, TRUE);
	vbox = create_box();
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	focus = create_text_view();
	sw = create_scrolled_window();
	gtk_box_pack_start(GTK_BOX(vbox), sw, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(sw), focus);

	hbb = create_button_box(FALSE);
	gtk_box_pack_start(GTK_BOX(vbox), hbb, FALSE, FALSE, 0);

	button = create_button(_("Close"));
	gtk_box_pack_end(GTK_BOX(hbb), button, FALSE, FALSE, 0);
	g_signal_connect_swapped(button, "clicked",
				 G_CALLBACK(gtk_widget_destroy), dialog);
	button = create_button(_("Send"));
	gtk_box_pack_end(GTK_BOX(hbb), button, FALSE, FALSE, 0);
	g_signal_connect_swapped(button, "clicked", G_CALLBACK(SendMessage),
				 this);
	gtk_widget_add_accelerator(button, "clicked", accel, GDK_Return,
				   GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_grab_focus(focus);
}

GtkWidget *DialogPeer::CreateMenuBar()
{
	GtkWidget *menu_bar;

	menu_bar = gtk_menu_bar_new();
	update_widget_bg(menu_bar, __BACK_DIR "/title.png");
	gtk_widget_show(menu_bar);
	CreateFileMenu(menu_bar);
	CreateHelpMenu(menu_bar);

	return menu_bar;
}

void DialogPeer::CreateFileMenu(GtkWidget * menu_bar)
{
	GtkWidget *menu;
	GtkWidget *menu_item;

	menu_item = gtk_menu_item_new_with_mnemonic(_("_File"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
	gtk_widget_show(menu_item);

	menu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
	gtk_widget_show(menu);

	menu_item = gtk_menu_item_new_with_label(_("Send File"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(SendFile::SendRegular), pal);
	gtk_widget_show(menu_item);

	menu_item = gtk_menu_item_new_with_label(_("Send Folder"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(SendFile::SendFolder), pal);
	gtk_widget_show(menu_item);

	menu_item = gtk_menu_item_new_with_label(_("Ask For Shared Files"));
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(MainWindow::AskSharedFiles), pal);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_tearoff_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
	gtk_widget_show(menu_item);

	menu_item = gtk_menu_item_new_with_label(_("Close"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(gtk_widget_destroy), dialog);
	gtk_widget_show(menu_item);
}

void DialogPeer::CreateHelpMenu(GtkWidget * menu_bar)
{
	GtkWidget *menu;
	GtkWidget *menu_item;

	menu_item = gtk_menu_item_new_with_mnemonic(_("_Help"));
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);

	menu = gtk_menu_new();
	gtk_widget_show(menu);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);

	menu_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, accel);
	g_signal_connect(menu_item, "activate",
			 G_CALLBACK(AboutIptux::AboutEntry), NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
}

bool DialogPeer::CheckExist(gpointer data)
{
	extern UdpData udt;
	GList *tmp;
	Pal *pal;

	pal = (Pal *) data;
	if (pal->dialog) {
		gtk_window_present(GTK_WINDOW
				   (((DialogPeer *) pal->dialog)->dialog));
		return true;
	}
	tmp = (GList *) udt.PalGetMsgPos(data);
	if (tmp) {
		pthread_mutex_lock(&udt.mutex);
		g_queue_delete_link(udt.msgqueue, tmp);
		pthread_mutex_unlock(&udt.mutex);
	}

	return false;
}

void DialogPeer::DragDataReceived(gpointer data, GdkDragContext * context,
				  gint x, gint y, GtkSelectionData * select,
				  guint info, guint time)
{
	extern SendFile sfl;
	extern struct interactive inter;
	const char *prl = "file://";
	char ipstr[INET_ADDRSTRLEN], *tmp, *file;
	GSList *list;
	Pal *pal;

	if (select->length <= 0 || select->format != 8 ||
	    strcasestr((char *)select->data, prl) == NULL) {
		gtk_drag_finish(context, FALSE, FALSE, time);
		return;
	}

	list = NULL, tmp = (char *)select->data;
	while (tmp = strcasestr(tmp, prl)) {
		file = my_getline(tmp + strlen(prl));
		list = g_slist_append(list, file);
		tmp += strlen(prl) + strlen(file);
	}
	pal = (Pal *) data;
	sfl.SendFileInfo(list, pal);
	g_slist_free(list);	//他处释放

	inet_ntop(AF_INET, &pal->ipv4, ipstr, INET_ADDRSTRLEN);
	pop_info(pal->dialog ? ((DialogPeer *) pal->dialog)->dialog :
		 inter.window,
		 pal->dialog ? ((DialogPeer *) pal->dialog)->focus : NULL,
		 _("Sending the files' infomation to \n%s[%s] is done!"),
		 pal->name, ipstr);

	gtk_drag_finish(context, TRUE, FALSE, time);
}

void DialogPeer::DialogDestroy(gpointer data)
{
	delete(DialogPeer *) data;
}

void DialogPeer::SendMessage(gpointer data)
{
	GtkTextBuffer *buffer;
	GtkTextIter start, end;
	DialogPeer *peer;
	gchar *ptr;

	peer = (DialogPeer *) data;
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(peer->focus));
	gtk_text_buffer_get_bounds(buffer, &start, &end);
	if (gtk_text_iter_equal(&start, &end)) {
		pop_warning(peer->dialog, peer->focus,
			    _("<span weight=\"heavy\" underline=\"error\">"
			      "\nCan't send an empty message!!</span>"));
		return;
	}
	ptr = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
	peer->pal->BufferInsertText(ptr, SELF);
	g_free(ptr);
	thread_create(ThreadFunc(ThreadSendMessage), data, FALSE);
	gtk_widget_grab_focus(peer->focus);
}

void DialogPeer::ThreadSendMessage(gpointer data)
{
	extern struct interactive inter;
	GtkTextBuffer *buffer;
	GtkTextIter start, end;
	DialogPeer *peer;
	Command cmd;
	gchar *ptr;

	peer = (DialogPeer *) data;
	gdk_threads_enter();
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(peer->focus));
	gtk_text_buffer_get_bounds(buffer, &start, &end);
	ptr = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
	gtk_text_buffer_delete(buffer, &start, &end);
	gdk_threads_leave();

	cmd.SendMessage(inter.udpsock, peer->pal, ptr);
	g_free(ptr);
}
