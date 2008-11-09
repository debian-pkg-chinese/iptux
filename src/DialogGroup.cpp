//
// C++ Implementation: DialogGroup
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "DialogGroup.h"
#include "TreeviewPopmenu.h"
#include "GroupMenuBar.h"
#include "UdpData.h"
#include "Control.h"
#include "Command.h"
#include "Pal.h"
#include "baling.h"
#include "output.h"
#include "utils.h"

GtkWidget *DialogGroup::dialog = NULL;
 DialogGroup::DialogGroup():pal_view(NULL), record(NULL),
input(NULL), accel(NULL), group_model(NULL)
{
}

DialogGroup::~DialogGroup()
{
	g_object_unref(accel);
	g_object_unref(group_model);
}

void DialogGroup::DialogEntry()
{
	DialogGroup *dg;

	if (DialogGroup::CheckExist())
		return;
	dg = new DialogGroup;
	dg->InitDialog();
	dg->CreateDialog();
}

void DialogGroup::InitDialog()
{
	group_model = CreateGroupModel();
}

void DialogGroup::CreateDialog()
{
	extern Control ctr;
	GdkColor color = { 8, 39321, 41634, 65535 };
	GroupMenuBar bar(this);
	GtkWidget *hpaned, *vpaned;
	GtkWidget *vbox;

	dialog = create_window(_("Group message"), 141, 138);
	gtk_widget_modify_bg(dialog, GTK_STATE_NORMAL, &color);
	accel = gtk_accel_group_new();
	gtk_window_add_accel_group(GTK_WINDOW(dialog), accel);
	g_signal_connect_swapped(dialog, "destroy", G_CALLBACK(DialogDestroy),
				 this);

	vbox = create_box();
	gtk_container_add(GTK_CONTAINER(dialog), vbox);
	bar.CreateMenuBar();
	gtk_box_pack_start(GTK_BOX(vbox), bar.menu_bar, FALSE, FALSE, 0);

	hpaned = create_paned(FALSE);
	gtk_paned_set_position(GTK_PANED(hpaned), GINT(ctr.pix * 40));
	gtk_box_pack_start(GTK_BOX(vbox), hpaned, TRUE, TRUE, 0);
	CreateChooseArea(hpaned);

	vpaned = create_paned();
	gtk_paned_set_position(GTK_PANED(vpaned), GINT(ctr.pix * 91));
	gtk_paned_pack2(GTK_PANED(hpaned), vpaned, TRUE, TRUE);
	CreateRecordArea(vpaned);
	CreateInputArea(vpaned);
}

void DialogGroup::CreateChooseArea(GtkWidget * paned)
{
	GtkWidget *frame, *sw;

	pal_view = CreateGroupView();
	g_signal_connect_swapped(pal_view, "button-press-event",
				 G_CALLBACK(ViewPopMenu), group_model);
	frame = create_frame(_("Choose Pals"));
	gtk_paned_pack1(GTK_PANED(paned), frame, FALSE, TRUE);
	sw = create_scrolled_window();
	gtk_container_add(GTK_CONTAINER(frame), sw);
	gtk_container_add(GTK_CONTAINER(sw), pal_view);
}

void DialogGroup::CreateRecordArea(GtkWidget * paned)
{
	GtkWidget *frame, *sw;

	record = create_text_view();
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(record), FALSE);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(record), FALSE);
	frame = create_frame(_("Send History"));
	gtk_paned_pack1(GTK_PANED(paned), frame, TRUE, TRUE);
	sw = create_scrolled_window();
	gtk_container_add(GTK_CONTAINER(frame), sw);
	gtk_container_add(GTK_CONTAINER(sw), record);
}

void DialogGroup::CreateInputArea(GtkWidget * paned)
{
	extern Control ctr;
	GtkWidget *box, *frame, *sw;
	GtkWidget *hbb, *button;

	frame = create_frame(_("Input Your Message"));
	gtk_paned_pack2(GTK_PANED(paned), frame, FALSE, TRUE);
	box = create_box();
	gtk_container_add(GTK_CONTAINER(frame), box);

	input = create_text_view();
	sw = create_scrolled_window();
	gtk_box_pack_start(GTK_BOX(box), sw, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(sw), input);

	hbb = create_button_box(FALSE);
	gtk_box_pack_start(GTK_BOX(box), hbb, FALSE, FALSE, 0);
	button = create_button(_("Close"));
	g_signal_connect_swapped(button, "clicked",
				 G_CALLBACK(gtk_widget_destroy), dialog);
	gtk_box_pack_end(GTK_BOX(hbb), button, FALSE, FALSE, 0);
	button = create_button(_("Send"));
	g_signal_connect_swapped(button, "clicked",
				 G_CALLBACK(SendMessage), this);
	gtk_box_pack_end(GTK_BOX(hbb), button, FALSE, FALSE, 0);
	gtk_widget_add_accelerator(button, "clicked", accel, GDK_Return,
				   GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

	gtk_widget_grab_focus(input);
}

void DialogGroup::BufferInsertText(const gchar * msg)
{
	extern Control ctr;
	GtkTextBuffer *buffer;
	GtkTextIter iter;
	char *ptr;

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(record));
	ptr = getformattime("%s", ctr.myname);
	gtk_text_buffer_get_end_iter(buffer, &iter);
	gtk_text_buffer_insert(buffer, &iter, ptr, -1);
	free(ptr);
	ptr = g_strdup_printf("%s\n", msg);
	gtk_text_buffer_get_end_iter(buffer, &iter);
	gtk_text_buffer_insert(buffer, &iter, ptr, -1);
	g_free(ptr);
}

void DialogGroup::SendGroupMsg(const gchar * msg)
{
	Command cmd;
	GtkTreeIter iter;
	gboolean active;
	gpointer pal;
	int sock;

	if (!gtk_tree_model_get_iter_first(group_model, &iter))
		return;

	sock = Socket(PF_INET, SOCK_DGRAM, 0);
	do {
		gtk_tree_model_get(group_model, &iter, 0, &active, 3, &pal, -1);
		if (active)
			cmd.SendGroupMsg(sock, pal, msg);
	} while (gtk_tree_model_iter_next(group_model, &iter));
	close(sock);
}

void DialogGroup::ViewScroll()
{
	GtkTextBuffer *buffer;
	GtkTextIter start, end;
	GtkTextMark *mark;

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(record));
	gtk_text_buffer_get_bounds(buffer, &start, &end);
	if (gtk_text_iter_equal(&start, &end))
		return;
	mark = gtk_text_buffer_create_mark(buffer, NULL, &end, FALSE);
	gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(record), mark, 0.0, TRUE,
				     0.0, 0.0);
	gtk_text_buffer_delete_mark(buffer, mark);
}

//群发 4,0 flag,1 pixbuf,2 nickname,3 pointer
GtkTreeModel *DialogGroup::CreateGroupModel()
{
	extern UdpData udt;
	GtkListStore *model;
	GtkTreeIter iter;
	GdkPixbuf *pixbuf;
	GSList *tmp;
	Pal *pal;

	model = gtk_list_store_new(4, G_TYPE_BOOLEAN, GDK_TYPE_PIXBUF,
				   G_TYPE_STRING, G_TYPE_POINTER);
	pthread_mutex_lock(&udt.mutex);
	tmp = udt.pallist;
	while (tmp) {
		pal = (Pal *) tmp->data;
		tmp = tmp->next;
		if (!FLAG_ISSET(pal->flags, 1))
			continue;
		pixbuf = gdk_pixbuf_new_from_file_at_size(pal->iconfile,
				MAX_ICONSIZE, MAX_ICONSIZE, NULL);
		gtk_list_store_append(model, &iter);
		gtk_list_store_set(model, &iter, 0, TRUE, 1, pixbuf, 2,
				   pal->name, 3, pal, -1);
		if (pixbuf)
			g_object_unref(pixbuf);
	}
	pthread_mutex_unlock(&udt.mutex);

	return GTK_TREE_MODEL(model);
}

GtkWidget *DialogGroup::CreateGroupView()
{
	GtkWidget *view;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	view = gtk_tree_view_new_with_model(group_model);
	gtk_widget_show(view);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_title(column, _("send"));
	renderer = gtk_cell_renderer_toggle_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "active", 0,
					    NULL);
	g_signal_connect_swapped(renderer, "toggled",
				 G_CALLBACK(ViewToggleChange), group_model);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_title(column, _("pals"));
	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "pixbuf", 1,
					    NULL);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", 2, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

	return view;
}

bool DialogGroup::CheckExist()
{
	if (dialog == NULL)
		return false;
	gtk_window_present(GTK_WINDOW(dialog));
	return true;
}

void DialogGroup::ViewToggleChange(GtkTreeModel * model, gchar * path)
{
	GtkTreePath *treepath;
	GtkTreeIter iter;
	gboolean active;

	treepath = gtk_tree_path_new_from_string(path);
	gtk_tree_model_get_iter(model, &iter, treepath);
	gtk_tree_model_get(model, &iter, 0, &active, -1);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, !active, -1);
}

gboolean DialogGroup::ViewPopMenu(GtkTreeModel * model, GdkEventButton * event)
{
	TreeviewPopmenu tp;

	if (event->button != 3)
		return FALSE;
	tp.CreatePopMenu(model);
	gtk_menu_popup(GTK_MENU(tp.menu), NULL, NULL, NULL, NULL,
		       event->button, event->time);

	return TRUE;
}

void DialogGroup::SendMessage(gpointer data)
{
	GtkTextBuffer *buffer;
	GtkTextIter start, end;
	DialogGroup *dg;
	char *msg;

	dg = (DialogGroup *) data;
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(dg->input));
	gtk_text_buffer_get_bounds(buffer, &start, &end);
	if (gtk_text_iter_equal(&start, &end)) {
		pop_warning(dialog, dg->input,
			    _("<span weight=\"heavy\" underline=\"error\">"
			      "\nCan't send an empty message!!</span>"));
		return;
	}
	msg = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
	gtk_text_buffer_delete(buffer, &start, &end);

	dg->BufferInsertText(msg);
	dg->SendGroupMsg(msg);
	dg->ViewScroll();

	g_free(msg);
	gtk_widget_grab_focus(dg->input);
}

void DialogGroup::DialogDestroy(gpointer data)
{
	dialog = NULL;
	delete(DialogGroup *) data;
}
