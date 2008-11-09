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
#include "MainMenuBar.h"
#include "PalviewPopmenu.h"
#include "UdpData.h"
#include "DialogPeer.h"
#include "Control.h"
#include "output.h"
#include "support.h"
#include "baling.h"
#include "utils.h"

 MainWindow::MainWindow():window(NULL), accel(NULL)
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
	GdkGeometry geometry = { 210, 0, 210, 2000, 0, 0, 1, 10, 0.0, 0.0,
		GDK_GRAVITY_NORTH_WEST
	};
	GdkWindowHints hints =
	    GdkWindowHints(GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE |
			   GDK_HINT_BASE_SIZE | GDK_HINT_RESIZE_INC |
			   GDK_HINT_WIN_GRAVITY | GDK_HINT_USER_POS |
			   GDK_HINT_USER_SIZE);
	GdkPixbuf *pixbuf;

	geometry.min_width = GINT(ctr.pix * 30);
	geometry.max_width = GINT(ctr.pix * 90);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	update_widget_bg(window, __BACK_DIR"/back.png");
	inter.window = window;
	gtk_window_set_title(GTK_WINDOW(window), _("IpTux"));
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
	gtk_window_set_default_size(GTK_WINDOW(window), GINT(ctr.pix * 60),
				    GINT(ctr.pix * 180));
	accel = gtk_accel_group_new();
	gtk_window_add_accel_group(GTK_WINDOW(window), accel);

	g_signal_connect_swapped(window, "delete-event",
				 G_CALLBACK(SwitchWindowMode), NULL);
	gtk_widget_show(window);
}

void MainWindow::CreateAllArea()
{
	extern UdpData udt;
	extern struct interactive inter;
	GtkWidget *box, *label, *sw, *paltree;
	gchar *ptr;
	MainMenuBar bar(accel);

	box = create_box();
	gtk_container_add(GTK_CONTAINER(window), box);
	bar.CreateMenuBar();
	gtk_box_pack_start(GTK_BOX(box), bar.menu_bar, FALSE, FALSE, 0);
	ptr = g_strdup_printf(_("pals online: %u"), 0);
	inter.palsum = label = create_label(ptr);
	free(ptr);
	gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
	sw = create_scrolled_window();
	gtk_container_set_border_width(GTK_CONTAINER(sw), 4);
	gtk_box_pack_start(GTK_BOX(box), sw, TRUE, TRUE, 0);
	paltree = CreatePalView(udt.pal_model);
	gtk_container_add(GTK_CONTAINER(sw), paltree);
}

GtkWidget *MainWindow::CreatePalView(GtkTreeModel * model)
{
	GdkColor color = { 8, 65535, 65535, 55000 };
	GtkTargetEntry target = { "text/plain", 0, 0 };
	GtkTreeSelection *selection;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;
	GtkWidget *view;

	view = gtk_tree_view_new();
	gtk_widget_modify_base(view, GTK_STATE_NORMAL, &color);
	gtk_drag_dest_set(view, GTK_DEST_DEFAULT_ALL,
			  &target, 1, GDK_ACTION_MOVE);
	gtk_tree_view_set_model(GTK_TREE_VIEW(view), model);
	gtk_tree_view_set_level_indentation(GTK_TREE_VIEW(view), 10);
	gtk_tree_view_set_show_expanders(GTK_TREE_VIEW(view), FALSE);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), FALSE);
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
	gtk_tree_selection_set_mode(GTK_TREE_SELECTION(selection), GTK_SELECTION_NONE);
	gtk_widget_show(view);

	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(GTK_TREE_VIEW_COLUMN(column), renderer, FALSE);
	g_object_set(renderer, "follow-state", TRUE, NULL);
	gtk_tree_view_column_set_attributes(GTK_TREE_VIEW_COLUMN(column),
					    renderer, "pixbuf", 0, NULL);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(GTK_TREE_VIEW_COLUMN(column), renderer, TRUE);
	g_object_set(renderer, "xalign", 0.0, "wrap-mode", PANGO_WRAP_WORD, NULL);
	gtk_tree_view_column_set_attributes(GTK_TREE_VIEW_COLUMN(column),
					    renderer, "markup", 1, NULL);

	g_object_set(view, "has-tooltip", TRUE, NULL);
	g_signal_connect(view, "query-tooltip",
			 G_CALLBACK(ViewQueryTooltip), model);
	g_signal_connect(view, "row-activated",
			 G_CALLBACK(ViewRowActivated), model);
	g_signal_connect(view, "button-press-event",
			 G_CALLBACK(ViewPopMenu), model);
	g_signal_connect(view, "button-release-event",
			 G_CALLBACK(ViewChangeStatus), model);
	g_signal_connect(view, "drag-data-received",
			 G_CALLBACK(DragDataReceived), model);

	return view;
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
		if (FLAG_ISSET(((Pal*)tmp->data)->flags, 1))
			sum++;
		tmp = tmp->next;
	}
	pthread_mutex_unlock(&udt.mutex);
	snprintf(buf, MAX_BUF, _("pals online: %u"), sum);
	gtk_label_set_text(GTK_LABEL(inter.palsum), buf);
}

void MainWindow::SwitchWindowMode()
{
	extern struct interactive inter;
	GdkPixbuf *pixbuf;

	if (GTK_WIDGET_VISIBLE(inter.window)) {
		gtk_widget_hide(inter.window);
		pixbuf =
		    gdk_pixbuf_new_from_file_at_size(__LOGO_DIR "/ip-penguin.png",
						     20, 20, NULL);
		if (pixbuf) {
			gtk_status_icon_set_from_pixbuf(inter.status_icon,
							pixbuf);
			g_object_unref(pixbuf);
		} else
			pwarning(Fail, "%s \"" __LOGO_DIR "/ip-penguin.png\" %s",
				 _("The notify icon"), _("is lost!"));
	} else {
		gtk_widget_show(inter.window);
		pixbuf =
		    gdk_pixbuf_new_from_file_at_size(__LOGO_DIR "/ip-tux.png", 20,
						     20, NULL);
		if (pixbuf) {
			gtk_status_icon_set_from_pixbuf(inter.status_icon,
							pixbuf);
			g_object_unref(pixbuf);
		} else
			pwarning(Fail, "%s \"" __LOGO_DIR "/ip-tux.png\" %s",
				 _("The notify icon"), _("is lost!"));
	}
}

gboolean MainWindow::ViewQueryTooltip(GtkWidget * view, gint x, gint y,
				      gboolean key, GtkTooltip * tooltip,
				      GtkTreeModel * model)
{
	char ipstr[INET_ADDRSTRLEN], buf[MAX_BUF];
	GtkTreePath *path;
	GtkTreeIter iter;
	GtkWidget *label;
	Pal *pal;

	if (key || !gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(view),
			x, y, &path, NULL, NULL, NULL))
		return FALSE;

	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_path_free(path);
	gtk_tree_model_get(model, &iter, 2, &pal, -1);
	if (!pal)
		return FALSE;

	inet_ntop(AF_INET, &pal->ipv4, ipstr, INET_ADDRSTRLEN);
	snprintf(buf, MAX_BUF, "%s\n%s", ipstr, pal->name);
	label = create_label(buf);
	gtk_tooltip_set_custom(tooltip, label);

	return TRUE;
}

void MainWindow::ViewRowActivated(GtkWidget * view, GtkTreePath * path,
				  GtkTreeViewColumn *column, GtkTreeModel * model)
{
	GtkTreeIter iter;
	gpointer data;

	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, 2, &data, -1);
	if (data)
		DialogPeer::DialogEntry(data);
}

gboolean MainWindow::ViewPopMenu(GtkWidget * view, GdkEventButton * event,
				 GtkTreeModel * model)
{
	PalviewPopmenu pp;
	GtkTreePath *path;
	GtkTreeIter iter;
	gpointer data;

	if (event->button != 3 || !gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(view),
	    GINT(event->x), GINT(event->y), &path, NULL, NULL, NULL))
		return FALSE;

	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_path_free(path);
	gtk_tree_model_get(model, &iter, 2, &data, -1);
	if (!data)
		return FALSE;

	pp.CreatePopMenu(data);
	gtk_menu_popup(GTK_MENU(pp.menu), NULL, NULL, NULL, NULL,
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

	if (event->button != 1 || !gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(view),
	    GINT(event->x), GINT(event->y), &path, NULL, NULL, NULL) ||
		   gtk_tree_path_get_depth(path) != 1)
		return FALSE;

	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, 3, &expend, -1);
	if (expend) {
		pixbuf = gdk_pixbuf_new_from_file(__TIP_DIR"/hide.png", NULL);
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 0, pixbuf, 3, FALSE, -1);
		gtk_tree_view_collapse_row(GTK_TREE_VIEW(view), path);
	} else {
		pixbuf = gdk_pixbuf_new_from_file(__TIP_DIR"/show.png", NULL);
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 0, pixbuf, 3, TRUE, -1);
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
	gtk_tree_model_get(model, &iter, 2, &data, -1);
	if (data)
		DialogPeer::DragDataReceived(data, context, x, y, select, info, time);
}
