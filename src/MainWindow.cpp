//
// C++ Implementation: MainWindow
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "MainWindow.h"
#include "DetectPal.h"
#include "Transport.h"
#include "IptuxSetup.h"
#include "ShareFile.h"
#include "DialogGroup.h"
#include "AboutIptux.h"
#include "CoreThread.h"
#include "UdpData.h"
#include "Command.h"
#include "DialogPeer.h"
#include "RevisePal.h"
#include "SendFile.h"
#include "Control.h"
#include "my_entry.h"
#include "output.h"
#include "support.h"
#include "baling.h"
#include "utils.h"

 MainWindow::MainWindow():window(NULL), client_paned(NULL), accel(NULL)
{
}

MainWindow::~MainWindow()
{
	gtk_widget_destroy(window);
	g_object_unref(accel);
}

void MainWindow::CreateWindow()
{
	extern Control ctr;
	extern struct interactive inter;
	GdkGeometry geometry = { 50, 200, 2000, 2000, 0, 0, 1, 10, 0.0, 0.0,
		GDK_GRAVITY_NORTH_WEST
	};
	GdkWindowHints hints =
	    GdkWindowHints(GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE |
			   GDK_HINT_BASE_SIZE | GDK_HINT_RESIZE_INC |
			   GDK_HINT_WIN_GRAVITY | GDK_HINT_USER_POS |
			   GDK_HINT_USER_SIZE);
	GdkPixbuf *pixbuf;

	inter.window = window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	update_widget_bg(window, __BACK_DIR "/back.png");
	gtk_window_set_title(GTK_WINDOW(window), _("iptux"));
	pixbuf =
	    gdk_pixbuf_new_from_file_at_size(__LOGO_DIR "/ip-tux.png", 25, 25,
					     NULL);
	if (pixbuf) {
		gtk_window_set_default_icon(pixbuf);
		g_object_unref(pixbuf);
	} else
		pwarning(Fail, "%s \"" __LOGO_DIR "/ip-tux.png\" %s",
			 _("Icon file"), _("is lost!"));
	gtk_window_set_geometry_hints(GTK_WINDOW(window), window, &geometry,
				      hints);
	gtk_window_set_default_size(GTK_WINDOW(window), GINT(ctr.pix * 70),
				    GINT(ctr.pix * 170));
	accel = gtk_accel_group_new();
	gtk_window_add_accel_group(GTK_WINDOW(window), accel);

	g_signal_connect_swapped(window, "delete-event",
				 G_CALLBACK(SwitchWindowMode), NULL);
	gtk_widget_show(window);
}

void MainWindow::CreateAllArea()
{
	extern struct interactive inter;
	GtkWidget *menu_bar, *label;
	GtkWidget *box, *sw, *paltree;
	gchar *ptr;

	client_paned = create_paned();
	gtk_container_add(GTK_CONTAINER(window), client_paned);
	box = create_box();
	gtk_paned_pack1(GTK_PANED(client_paned), box, true, true);
	menu_bar = CreateMenuBar();
	gtk_box_pack_start(GTK_BOX(box), menu_bar, FALSE, FALSE, 0);
	ptr = g_strdup_printf(_("pals online: %u"), 0);
	inter.online = label = create_label(ptr);
	g_free(ptr);
	gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
	sw = create_scrolled_window();
	gtk_container_set_border_width(GTK_CONTAINER(sw), 4);
	gtk_box_pack_start(GTK_BOX(box), sw, TRUE, TRUE, 0);
	paltree = CreatePalView();
	gtk_container_add(GTK_CONTAINER(sw), paltree);
}

GtkWidget *MainWindow::CreateMenuBar()
{
	GtkWidget *menu_bar;

	menu_bar = gtk_menu_bar_new();
	update_widget_bg(menu_bar, __BACK_DIR "/title.png");
	gtk_widget_show(menu_bar);
	CreateFileMenu(menu_bar);
	CreateToolMenu(menu_bar);
	CreateHelpMenu(menu_bar);

	return menu_bar;
}

GtkWidget *MainWindow::CreatePalView()
{
	extern UdpData udt;
	GdkColor color = { 8, 65535, 65535, 55000 };
	GtkTargetEntry target = { "text/plain", 0, 0 };
	GtkTreeSelection *selection;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;
	GtkWidget *view;

	view = gtk_tree_view_new_with_model(udt.pal_model);
	gtk_widget_modify_base(view, GTK_STATE_NORMAL, &color);
	gtk_drag_dest_set(view, GTK_DEST_DEFAULT_ALL,
			  &target, 1, GDK_ACTION_MOVE);
	gtk_tree_view_set_level_indentation(GTK_TREE_VIEW(view), 10);
	gtk_tree_view_set_show_expanders(GTK_TREE_VIEW(view), FALSE);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), FALSE);
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
	gtk_tree_selection_set_mode(GTK_TREE_SELECTION(selection),
				    GTK_SELECTION_NONE);
	gtk_widget_show(view);

	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
	renderer = gtk_cell_renderer_pixbuf_new();	//
	gtk_tree_view_column_pack_start(GTK_TREE_VIEW_COLUMN(column), renderer,
					FALSE);
	g_object_set(renderer, "follow-state", TRUE, NULL);
	gtk_tree_view_column_set_attributes(GTK_TREE_VIEW_COLUMN(column),
					    renderer, "pixbuf", 0, NULL);
	renderer = gtk_cell_renderer_text_new();	//
	gtk_tree_view_column_pack_start(GTK_TREE_VIEW_COLUMN(column), renderer,
					TRUE);
	g_object_set(renderer, "xalign", 0.0, "wrap-mode", PANGO_WRAP_WORD,
		     "foreground", "#52B838", NULL);
	gtk_tree_view_column_set_attributes(GTK_TREE_VIEW_COLUMN(column),
					    renderer, "text", 2, "font", 3,
					    "visible", 4, NULL);
	renderer = gtk_cell_renderer_text_new();	//
	gtk_tree_view_column_pack_start(GTK_TREE_VIEW_COLUMN(column), renderer,
					TRUE);
	g_object_set(renderer, "xalign", 0.0, "wrap-mode", PANGO_WRAP_WORD,
		     "foreground", "#52B838", NULL);
	gtk_tree_view_column_set_attributes(GTK_TREE_VIEW_COLUMN(column),
					    renderer, "markup", 5, "visible", 6,
					    NULL);

	g_object_set(view, "has-tooltip", TRUE, NULL);
	g_signal_connect(view, "query-tooltip",
			 G_CALLBACK(ViewQueryTooltip), udt.pal_model);
	g_signal_connect(view, "row-activated",
			 G_CALLBACK(ViewRowActivated), udt.pal_model);
	g_signal_connect(view, "button-press-event",
			 G_CALLBACK(PopupPalMenu), udt.pal_model);
	g_signal_connect(view, "button-release-event",
			 G_CALLBACK(ViewChangeStatus), udt.pal_model);
	g_signal_connect(view, "drag-data-received",
			 G_CALLBACK(DragDataReceived), udt.pal_model);

	return view;
}

void MainWindow::CreateFileMenu(GtkWidget * menu_bar)
{
	GtkWidget *image;
	GtkWidget *menu;
	GtkWidget *menu_item;

	menu_item = gtk_menu_item_new_with_mnemonic(_("_File"));
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);

	menu = gtk_menu_new();
	gtk_widget_show(menu);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_Detect"));
	image = gtk_image_new_from_file(__TIP_DIR "/detect.png");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), image);
	g_signal_connect(menu_item, "activate",
			 G_CALLBACK(DetectPal::DetectEntry), NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_Find"));
	image = gtk_image_new_from_file(__TIP_DIR "/find.png");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), image);
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(FindSpecifyPal), this);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_separator_menu_item_new();
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_Quit"));
	image = gtk_image_new_from_file(__TIP_DIR "/out.png");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), image);
	g_signal_connect(menu_item, "activate", G_CALLBACK(iptux_gui_quit),
			 NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
}

void MainWindow::CreateToolMenu(GtkWidget * menu_bar)
{
	GtkWidget *image;
	GtkWidget *menu;
	GtkWidget *menu_item;

	menu_item = gtk_menu_item_new_with_mnemonic(_("_Tools"));
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);

	menu = gtk_menu_new();
	gtk_widget_show(menu);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_Transport"));
	image = gtk_image_new_from_file(__TIP_DIR "/trans.png");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), image);
	g_signal_connect(menu_item, "activate",
			 G_CALLBACK(Transport::TransportEntry), NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_Setup"));
	image = gtk_image_new_from_file(__TIP_DIR "/setup.png");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), image);
	g_signal_connect(menu_item, "activate",
			 G_CALLBACK(IptuxSetup::SetupEntry), NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_Public"));
	image = gtk_image_new_from_file(__TIP_DIR "/share.png");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), image);
	g_signal_connect(menu_item, "activate",
			 G_CALLBACK(ShareFile::ShareEntry), NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_Group"));
	image = gtk_image_new_from_file(__TIP_DIR "/net.png");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), image);
	g_signal_connect(menu_item, "activate",
			 G_CALLBACK(DialogGroup::DialogEntry), NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_Update"));
	image = gtk_image_new_from_file(__TIP_DIR "/fresh.png");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), image);
	g_signal_connect(menu_item, "activate", G_CALLBACK(UpdatePalList),
			 NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
}

void MainWindow::CreateHelpMenu(GtkWidget * menu_bar)
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

void MainWindow::UpdateTips()
{
	extern struct interactive inter;
	extern UdpData udt;
	char buf[MAX_BUF];
	uint32_t sum;
	GSList *tmp;

	pthread_mutex_lock(&udt.mutex);
	sum = 0, tmp = udt.pallist;
	while (tmp) {
		if (FLAG_ISSET(((Pal *) tmp->data)->flags, 1))
			sum++;
		tmp = tmp->next;
	}
	pthread_mutex_unlock(&udt.mutex);
	snprintf(buf, MAX_BUF, _("pals online: %u"), sum);
	gtk_label_set_text(GTK_LABEL(inter.online), buf);
}

GtkWidget *MainWindow::CreatePopupMenu(gpointer data)
{
	GtkWidget *menu, *menu_item;

	menu = gtk_menu_new();
	gtk_widget_show(menu);

	menu_item = gtk_menu_item_new_with_label(_("Send Message"));
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(DialogPeer::DialogEntry), data);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_menu_item_new_with_label(_("Send File"));
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(SendFile::SendRegular), data);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_menu_item_new_with_label(_("Send Folder"));
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(SendFile::SendFolder), data);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_menu_item_new_with_label(_("Ask For Shared Files"));
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(AskSharedFiles), data);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_menu_item_new_with_label(_("Change Information"));
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(RevisePal::ReviseEntry), data);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_menu_item_new_with_label(_("Delete Pal"));
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(DeletePal), data);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	return menu;
}

//find_model 6,0 pixbuf,1 name,2 ipstr,3 user,4 host,5 pal
GtkTreeModel *MainWindow::CreateFindModel()
{
	GtkListStore *model;

	model = gtk_list_store_new(6, GDK_TYPE_PIXBUF,
				   G_TYPE_STRING, G_TYPE_STRING,
				   G_TYPE_STRING, G_TYPE_STRING,
				   G_TYPE_POINTER);

	return GTK_TREE_MODEL(model);
}

GtkWidget *MainWindow::CreateFindView()
{
	GdkColor color = { 8, 65535, 65535, 55000 };
	GtkTargetEntry target = { "text/plain", 0, 0 };
	GtkWidget *view;
	GtkTreeModel *model;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;
	GtkTreeSelection *selection;

	model = CreateFindModel();
	view = gtk_tree_view_new_with_model(model);
	gtk_widget_modify_base(view, GTK_STATE_NORMAL, &color);
	gtk_drag_dest_set(view, GTK_DEST_DEFAULT_ALL,
			  &target, 1, GDK_ACTION_MOVE);
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
	gtk_tree_selection_set_mode(GTK_TREE_SELECTION(selection),
				    GTK_SELECTION_NONE);
	gtk_widget_show(view);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_title(column, _("nickname"));
	renderer = gtk_cell_renderer_pixbuf_new();
	g_object_set(renderer, "follow-state", TRUE, NULL);
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "pixbuf", 0,
					    NULL);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", 1, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_title(column, _("IPv4"));
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", 2, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_title(column, _("user"));
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", 3, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_title(column, _("host"));
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", 4, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

	g_signal_connect(view, "row-activated",
			 G_CALLBACK(FindViewRowActivated), model);
	g_signal_connect(view, "button-press-event",
			 G_CALLBACK(FindPopupPalMenu), model);
	g_signal_connect(view, "drag-data-received",
			 G_CALLBACK(FindDragDataReceived), model);

	return view;
}

void MainWindow::SwitchWindowMode()
{
	extern struct interactive inter;
	GdkPixbuf *pixbuf;

	if (GTK_WIDGET_VISIBLE(inter.window)) {
		gtk_widget_hide(inter.window);
		pixbuf =
		    gdk_pixbuf_new_from_file_at_size(__LOGO_DIR
						     "/ip-penguin.png", 20, 20,
						     NULL);
		if (pixbuf) {
			gtk_status_icon_set_from_pixbuf(inter.status_icon,
							pixbuf);
			g_object_unref(pixbuf);
		} else
			pwarning(Fail,
				 "%s \"" __LOGO_DIR "/ip-penguin.png\" %s",
				 _("The notify icon"), _("is lost!"));
	} else {
		gtk_widget_show(inter.window);
		pixbuf =
		    gdk_pixbuf_new_from_file_at_size(__LOGO_DIR "/ip-tux.png",
						     20, 20, NULL);
		if (pixbuf) {
			gtk_status_icon_set_from_pixbuf(inter.status_icon,
							pixbuf);
			g_object_unref(pixbuf);
		} else
			pwarning(Fail, "%s \"" __LOGO_DIR "/ip-tux.png\" %s",
				 _("The notify icon"), _("is lost!"));
	}
}

void MainWindow::AskSharedFiles(gpointer data)
{
	extern struct interactive inter;
	Command cmd;

	cmd.SendAskShared(inter.udpsock, data);
}

void MainWindow::UpdatePalList()
{
	extern UdpData udt;
	GtkTreeIter iter;
	uint8_t count;
	GSList *tmp;

	pthread_mutex_lock(&udt.mutex);
	tmp = udt.pallist;
	while (tmp) {
		FLAG_CLR(((Pal *) tmp->data)->flags, 1);
		tmp = tmp->next;
	}
	g_queue_clear(udt.msgqueue);
	pthread_mutex_unlock(&udt.mutex);

	gtk_tree_store_clear(GTK_TREE_STORE(udt.pal_model));
	udt.InitPalModel();

	thread_create(ThreadFunc(CoreThread::NotifyAll), NULL, false);
}

void MainWindow::DeletePal(gpointer data)
{
	extern UdpData udt;
	GtkTreeIter iter, parent;
	GList *tmp;
	Pal *pal;

	pal = (Pal *) data;
	if (!udt.Ipv4GetPalPos(pal->ipv4))
		return;

	udt.Ipv4GetParent(pal->ipv4, &parent);
	if (udt.PalGetModelIter(pal, &parent, &iter))
		gtk_tree_store_remove(GTK_TREE_STORE(udt.pal_model), &iter);
	tmp = (GList *) udt.PalGetMsgPos(pal);
	if (tmp) {
		pthread_mutex_lock(&udt.mutex);
		g_queue_delete_link(udt.msgqueue, tmp);
		pthread_mutex_unlock(&udt.mutex);
	}
	FLAG_CLR(pal->flags, 1);
	FLAG_SET(pal->flags, 3);
}

gboolean MainWindow::ViewQueryTooltip(GtkWidget * view, gint x, gint y,
				      gboolean key, GtkTooltip * tooltip,
				      GtkTreeModel * model)
{
	extern Control ctr;
	GdkColor color = { 8, 65535, 65535, 55000 };
	GtkWidget *text_view;
	GtkTextBuffer *buffer;
	GtkTreePath *path;
	GtkTreeIter iter;
	Pal *pal;

	if (key || !gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(view),
						  x, y, &path, NULL, NULL,
						  NULL))
		return FALSE;

	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_path_free(path);
	gtk_tree_model_get(model, &iter, 7, &pal, -1);
	if (!pal)
		return FALSE;

	text_view = create_text_view();
	gtk_widget_modify_base(text_view, GTK_STATE_NORMAL, &color);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), FALSE);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_NONE);
	buffer = gtk_text_buffer_new(ctr.table);
	DialogPeer::FillPalInfoToBuffer(pal, buffer, false);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(text_view), buffer);
	gtk_tooltip_set_custom(tooltip, text_view);
	g_signal_connect_swapped(text_view, "destroy",
				 G_CALLBACK(g_object_unref), buffer);

	return TRUE;
}

void MainWindow::ViewRowActivated(GtkWidget * view, GtkTreePath * path,
				  GtkTreeViewColumn * column,
				  GtkTreeModel * model)
{
	GtkTreeIter iter;
	gpointer data;

	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, 7, &data, -1);
	if (data)
		DialogPeer::DialogEntry(data);
}

gboolean MainWindow::PopupPalMenu(GtkWidget * view, GdkEventButton * event,
				  GtkTreeModel * model)
{
	GtkTreePath *path;
	GtkTreeIter iter;
	gpointer data;

	if (event->button != 3
	    || !gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(view),
					      GINT(event->x), GINT(event->y),
					      &path, NULL, NULL, NULL))
		return FALSE;

	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_path_free(path);
	gtk_tree_model_get(model, &iter, 7, &data, -1);
	if (!data)
		return FALSE;

	gtk_menu_popup(GTK_MENU(CreatePopupMenu(data)), NULL, NULL, NULL, NULL,
		       event->button, event->time);

	return TRUE;
}

gboolean MainWindow::ViewChangeStatus(GtkWidget * view, GdkEventButton * event,
				      GtkTreeModel * model)
{
	GtkTreePath *path;
	GdkPixbuf *pixbuf;
	GtkTreeIter iter;
	gboolean expend;

	if (event->button != 1
	    || !gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(view),
					      GINT(event->x), GINT(event->y),
					      &path, NULL, NULL, NULL)
	    || gtk_tree_path_get_depth(path) != 1)
		return FALSE;

	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, 1, &expend, -1);
	if (expend) {
		pixbuf = gdk_pixbuf_new_from_file(__TIP_DIR "/hide.png", NULL);
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 0, pixbuf, 1,
				   FALSE, -1);
		gtk_tree_view_collapse_row(GTK_TREE_VIEW(view), path);
	} else {
		pixbuf = gdk_pixbuf_new_from_file(__TIP_DIR "/show.png", NULL);
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 0, pixbuf, 1,
				   TRUE, -1);
		gtk_tree_view_expand_row(GTK_TREE_VIEW(view), path, FALSE);
	}
	if (pixbuf)
		g_object_unref(pixbuf);
	gtk_tree_path_free(path);
}

void MainWindow::DragDataReceived(GtkWidget * view, GdkDragContext * context,
				  gint x, gint y, GtkSelectionData * select,
				  guint info, guint time, GtkTreeModel * model)
{
	GtkTreePath *path;
	GtkTreeIter iter;
	gpointer data;

	if (!gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(view),
					   x, y, &path, NULL, NULL, NULL))
		return;
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_path_free(path);
	gtk_tree_model_get(model, &iter, 7, &data, -1);
	if (data)
		DialogPeer::DragDataReceived(data, context, x, y, select, info,
					     time);
}

void MainWindow::FindSpecifyPal(gpointer data)
{
	MainWindow *mw;
	GtkWidget *sw, *view, *button, *image, *entry;
	GtkWidget *box, *hbox;

	mw = (MainWindow *) data;
	if (gtk_paned_get_child2(GTK_PANED(mw->client_paned)))
		return;
	box = create_box();
	gtk_paned_pack2(GTK_PANED(mw->client_paned), box, TRUE, TRUE);

	sw = create_scrolled_window();
	gtk_box_pack_start(GTK_BOX(box), sw, TRUE, TRUE, 0);
	view = CreateFindView();
	gtk_container_add(GTK_CONTAINER(sw), view);

	hbox = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
	button = gtk_button_new();
	g_object_set(button, "relief", GTK_RELIEF_NONE, NULL);
	image = gtk_image_new_from_file(__TIP_DIR "/cancel.png");
	gtk_button_set_image(GTK_BUTTON(button), image);
	gtk_widget_show(button);
	g_signal_connect_swapped(button, "clicked", G_CALLBACK(g_object_unref),
				 gtk_tree_view_get_model(GTK_TREE_VIEW(view)));
	g_signal_connect_swapped(button, "clicked",
				 G_CALLBACK(gtk_widget_destroy), box);
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
	entry = my_entry::create_entry(NULL, _("search pals"));
	gtk_widget_add_events(entry, GDK_KEY_PRESS_MASK);
	g_signal_connect(entry, "key-press-event", G_CALLBACK(FindClearEntry),
			 NULL);
	g_signal_connect(entry, "changed", G_CALLBACK(FindEntryChanged), view);
	gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);

	FindEntryChanged(entry, view);
	gtk_widget_grab_focus(entry);
}

gboolean MainWindow::FindClearEntry(GtkWidget * entry, GdkEventKey * event)
{
	if (event->keyval != GDK_Escape)
		return FALSE;
	gtk_entry_set_text(GTK_ENTRY(entry), "");
	return TRUE;
}

void MainWindow::FindEntryChanged(GtkWidget * entry, GtkWidget * view)
{
	extern UdpData udt;
	char ipstr[INET_ADDRSTRLEN];
	const gchar *text;
	GdkPixbuf *pixbuf;
	GtkTreeModel *model;
	GtkTreeIter iter;
	GSList *tmp;
	Pal *pal;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(view));
	gtk_list_store_clear(GTK_LIST_STORE(model));
	text = gtk_entry_get_text(GTK_ENTRY(entry));
	pthread_mutex_lock(&udt.mutex);
	tmp = udt.pallist;
	while (tmp) {
		pal = (Pal *) tmp->data;
		tmp = tmp->next;
		inet_ntop(AF_INET, &pal->ipv4, ipstr, INET_ADDRSTRLEN);
		if (FLAG_ISSET(pal->flags, 1) && (*text == '\0'
						  || strstr(pal->name, text)
						  || strstr(ipstr, text)
						  || strstr(pal->user, text)
						  || strstr(pal->host, text))) {
			pixbuf =
			    gdk_pixbuf_new_from_file_at_size(pal->iconfile,
							     MAX_ICONSIZE,
							     MAX_ICONSIZE,
							     NULL);
			gtk_list_store_append(GTK_LIST_STORE(model), &iter);
			gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0,
					   pixbuf, 1, pal->name, 2, ipstr, 3,
					   pal->user, 4, pal->host, 5, pal, -1);
			if (pixbuf)
				g_object_unref(pixbuf);
		}
	}
	pthread_mutex_unlock(&udt.mutex);
	gtk_tree_view_columns_autosize(GTK_TREE_VIEW(view));
}

void MainWindow::FindViewRowActivated(GtkWidget * view, GtkTreePath * path,
				      GtkTreeViewColumn * column,
				      GtkTreeModel * model)
{
	GtkTreeIter iter;
	gpointer data;

	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, 5, &data, -1);
	DialogPeer::DialogEntry(data);
}

gboolean MainWindow::FindPopupPalMenu(GtkWidget * view, GdkEventButton * event,
				      GtkTreeModel * model)
{
	GtkTreePath *path;
	GtkTreeIter iter;
	gpointer data;

	if (event->button != 3
	    || !gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(view),
					      GINT(event->x), GINT(event->y),
					      &path, NULL, NULL, NULL))
		return FALSE;

	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_path_free(path);
	gtk_tree_model_get(model, &iter, 5, &data, -1);
	gtk_menu_popup(GTK_MENU(CreatePopupMenu(data)), NULL, NULL, NULL, NULL,
		       event->button, event->time);

	return TRUE;
}

void MainWindow::FindDragDataReceived(GtkWidget * view,
				      GdkDragContext * context, gint x, gint y,
				      GtkSelectionData * select, guint info,
				      guint time, GtkTreeModel * model)
{
	GtkTreePath *path;
	GtkTreeIter iter;
	gpointer data;

	if (!gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(view),
					   x, y, &path, NULL, NULL, NULL))
		return;
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_path_free(path);
	gtk_tree_model_get(model, &iter, 5, &data, -1);
	DialogPeer::DragDataReceived(data, context, x, y, select, info, time);
}
