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

 MainWindow::MainWindow():window(NULL), mbox(NULL), accel(NULL)
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
	inter.window = window;
	gtk_window_set_title(GTK_WINDOW(window), _("IpTux"));
	pixbuf =
	    gdk_pixbuf_new_from_file_at_size(__LOGO_DIR "/tux.png", 25, 25,
					     NULL);
	if (pixbuf) {
		gtk_window_set_default_icon(pixbuf);
		g_object_unref(pixbuf);
	} else
		pwarning(Fail, "%s \"" __LOGO_DIR "/tux.png %s",
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
	GtkWidget *box;
	GtkWidget *sw;
	MainMenuBar bar(accel);

	box = create_box();
	gtk_container_add(GTK_CONTAINER(window), box);
	bar.CreateMenuBar();
	gtk_box_pack_start(GTK_BOX(box), bar.menu_bar, FALSE, FALSE, 0);
	sw = create_scrolled_window();
	gtk_box_pack_start(GTK_BOX(box), sw, TRUE, TRUE, 0);
	mbox = create_box();
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(sw), mbox);
	update_widget_bg(gtk_bin_get_child(GTK_BIN(sw)),
			 __BACK_DIR "/back.png");
}

void MainWindow::CreateExpanders()
{
	extern UdpData udt;
	GtkWidget *expander;
	GtkWidget *view;
	uint8_t count;
	gchar ipstr[INET_ADDRSTRLEN << 1];

	count = 0;
	while (count < UdpData::sumseg) {
		snprintf(ipstr, INET_ADDRSTRLEN << 1, "%s~%s",
			 UdpData::localip[count << 1],
			 UdpData::localip[(count << 1) + 1]);
		expander = CreateExpander(ipstr);
		gtk_box_pack_start(GTK_BOX(mbox), expander, FALSE, FALSE, 0);
		view = CreatePalView(udt.pal_model[count]);
		gtk_container_add(GTK_CONTAINER(expander), view);
		count++;
	}
	expander = CreateExpander(UdpData::localip[count << 1]);
	gtk_box_pack_start(GTK_BOX(mbox), expander, FALSE, FALSE, 0);
	view = CreatePalView(udt.pal_model[count]);
	gtk_container_add(GTK_CONTAINER(expander), view);
}

GtkWidget *MainWindow::CreateExpander(const gchar * title)
{
	gchar *markup;
	GtkWidget *expander;

	markup =
	    g_markup_printf_escaped
	    ("<span style=\"italic\" underline=\"single\" "
	     "foreground=\"red\" weight=\"bold\" size=\"small\">%s</span>",
	     title);
	expander = gtk_expander_new(markup);
	gtk_expander_set_use_markup(GTK_EXPANDER(expander), TRUE);
	g_free(markup);
	gtk_widget_show(expander);

	return expander;
}

GtkWidget *MainWindow::CreatePalView(GtkTreeModel * model)
{
	GtkTargetEntry target = { "text/plain", 0, 0 };
	GtkCellRenderer *renderer;
	GtkWidget *view;

	view = gtk_icon_view_new();
	gtk_drag_dest_set(view, GTK_DEST_DEFAULT_ALL,
			  &target, 1, GDK_ACTION_MOVE);
	gtk_icon_view_set_model(GTK_ICON_VIEW(view), model);
	gtk_icon_view_set_orientation(GTK_ICON_VIEW(view),
				      GTK_ORIENTATION_HORIZONTAL);
	gtk_icon_view_set_columns(GTK_ICON_VIEW(view), 1);
	gtk_icon_view_set_spacing(GTK_ICON_VIEW(view), 4);
	gtk_icon_view_set_column_spacing(GTK_ICON_VIEW(view), 10);
	gtk_widget_show(view);

	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(view), renderer, FALSE);
	g_object_set(renderer, "follow-state", TRUE, NULL);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(view), renderer,
				       "pixbuf", 0, NULL);
	renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer, "xalign", 0.5, "wrap-mode", PANGO_WRAP_WORD_CHAR,
		     "foreground", "red", "style", PANGO_STYLE_OBLIQUE, NULL);
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(view), renderer, FALSE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(view), renderer,
				       "text", 1, NULL);

	g_object_set(view, "has-tooltip", TRUE, NULL);
	g_signal_connect(view, "query-tooltip",
			 G_CALLBACK(ViewQueryTooltip), model);
	g_signal_connect(view, "item-activated",
			 G_CALLBACK(ViewItemActivated), model);
	g_signal_connect(view, "button-press-event",
			 G_CALLBACK(ViewPopMenu), model);
	g_signal_connect(view, "drag-data-received",
			 G_CALLBACK(DragDataReceived), model);

	return view;
}

void MainWindow::SwitchWindowMode()
{
	extern struct interactive inter;
	GdkPixbuf *pixbuf;

	if (GTK_WIDGET_VISIBLE(inter.window)) {
		gtk_widget_hide(inter.window);
		pixbuf =
		    gdk_pixbuf_new_from_file_at_size(__LOGO_DIR "/penguin.png",
						     20, 20, NULL);
		if (pixbuf) {
			gtk_status_icon_set_from_pixbuf(inter.status_icon,
							pixbuf);
			g_object_unref(pixbuf);
		} else
			pwarning(Fail, "%s \"" __LOGO_DIR "/penguin.png\" %s",
				 _("The notify icon"), _("is lost!"));
	} else {
		gtk_widget_show(inter.window);
		pixbuf =
		    gdk_pixbuf_new_from_file_at_size(__LOGO_DIR "/tux.png", 20,
						     20, NULL);
		if (pixbuf) {
			gtk_status_icon_set_from_pixbuf(inter.status_icon,
							pixbuf);
			g_object_unref(pixbuf);
		} else
			pwarning(Fail, "%s \"" __LOGO_DIR "/tux.png\" %s",
				 _("The notify icon"), _("is lost!"));
	}
}

gboolean MainWindow::ViewQueryTooltip(GtkWidget * view, gint x, gint y,
				      gboolean key, GtkTooltip * tooltip,
				      GtkTreeModel * model)
{
	char ipstr[INET_ADDRSTRLEN];
	GtkTreePath *path;
	GtkTreeIter iter;
	GtkWidget *label;
	Pal *pal;

	if (key)
		return FALSE;
	path = gtk_icon_view_get_path_at_pos(GTK_ICON_VIEW(view), x, y);
	if (!path)
		return FALSE;

	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, 2, &pal, -1);
	gtk_tree_path_free(path);

	inet_ntop(AF_INET, &pal->ipv4, ipstr, INET_ADDRSTRLEN);
	label = create_label(ipstr);
	gtk_tooltip_set_custom(tooltip, label);

	return TRUE;
}

void MainWindow::ViewItemActivated(GtkWidget * view, GtkTreePath * path,
				   GtkTreeModel * model)
{
	GtkTreeIter iter;
	gpointer data;

	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, 2, &data, -1);
	DialogPeer::DialogEntry(data);
}

gboolean MainWindow::ViewPopMenu(GtkWidget * view, GdkEventButton * event,
				 GtkTreeModel * model)
{
	PalviewPopmenu pp;
	GtkTreePath *path;
	GtkTreeIter iter;
	gpointer data;

	if (event->button != 3)
		return FALSE;
	path = gtk_icon_view_get_path_at_pos(GTK_ICON_VIEW(view),
					     GINT(event->x), GINT(event->y));
	if (!path)
		return FALSE;

	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_path_free(path);
	gtk_tree_model_get(model, &iter, 2, &data, -1);
	pp.CreatePopMenu(data);
	gtk_menu_popup(GTK_MENU(pp.menu), NULL, NULL, NULL, NULL,
		       event->button, event->time);

	return TRUE;
}

void MainWindow::DragDataReceived(GtkWidget * view, GdkDragContext * context,
				  gint x, gint y, GtkSelectionData * select,
				  guint info, guint time, GtkTreeModel * model)
{
	GtkTreePath *path;
	GtkTreeIter iter;
	gpointer data;

	path = gtk_icon_view_get_path_at_pos(GTK_ICON_VIEW(view), x, y);
	if (!path)
		return;
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, 2, &data, -1);
	gtk_tree_path_free(path);
	DialogPeer::DragDataReceived(data, context, x, y, select, info, time);
}
