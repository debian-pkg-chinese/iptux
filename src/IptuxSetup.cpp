//
// C++ Implementation: IptuxSetup
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "IptuxSetup.h"
#include "Control.h"
#include "my_entry.h"
#include "my_chooser.h"
#include "UdpData.h"
#include "Command.h"
#include "Pal.h"
#include "support.h"
#include "output.h"
#include "baling.h"
#include "utils.h"

GtkWidget *IptuxSetup::setup = NULL;
 IptuxSetup::IptuxSetup():icon_model(NULL), ip_model(NULL),
myname(NULL), myicon(NULL), save_path(NULL), ad(NULL),
sign(NULL), encode(NULL), palicon(NULL), font(NULL),
tidy(NULL), log(NULL), black(NULL), proof(NULL), entry1(NULL),
     entry2(NULL), ipseg_view(NULL)
{
}

IptuxSetup::~IptuxSetup()
{
	g_object_unref(icon_model);
	g_object_unref(ip_model);
}

void IptuxSetup::SetupEntry()
{
	IptuxSetup *ipst;

	if (IptuxSetup::CheckExist())
		return;
	ipst = new IptuxSetup;
	ipst->InitSetup();
	ipst->CreateSetup();
}

void IptuxSetup::InitSetup()
{
	icon_model = CreateIconModel();
	ip_model = CreateIpModel();
}

void IptuxSetup::CreateSetup()
{
	GtkWidget *box;
	GtkWidget *notebook, *hbb;

	setup = create_window(_("Iptux setup"), 132, 100);
	gtk_container_set_border_width(GTK_CONTAINER(setup), 5);
	g_signal_connect_swapped(setup, "destroy",
				 G_CALLBACK(SetupDestroy), this);
	box = create_box();
	gtk_container_add(GTK_CONTAINER(setup), box);

	notebook = gtk_notebook_new();
	gtk_widget_show(notebook);
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_LEFT);
	gtk_box_pack_start(GTK_BOX(box), notebook, TRUE, TRUE, 0);
	CreatePerson(notebook);
	CreateSystem(notebook);
	CreateIpseg(notebook);

	hbb = create_button_box(FALSE);
	gtk_box_pack_start(GTK_BOX(box), hbb, FALSE, FALSE, 0);
	CreateFuncButton(hbb);
}

void IptuxSetup::CreatePerson(GtkWidget * note)
{
	extern Control ctr;
	char path[MAX_PATHBUF];
	GtkWidget *label, *frame, *sw, *button;
	GtkWidget *box, *hbox;
	GdkPixbuf *pixbuf;

	box = create_box();
	label = create_label(_("Personal Setup"));
	gtk_notebook_append_page(GTK_NOTEBOOK(note), box, label);

	hbox = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 5);
	myname = create_label(_("Your nickname:"));
	gtk_box_pack_start(GTK_BOX(hbox), myname, FALSE, FALSE, 0);
	myname = my_entry::create_entry(ctr.myname,
					_("Please Input your nickname!"),
					FALSE);
	gtk_box_pack_start(GTK_BOX(hbox), myname, TRUE, TRUE, 0);

	hbox = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 5);
	myicon = create_label(_("Your head portrait:"));
	gtk_box_pack_start(GTK_BOX(hbox), myicon, FALSE, FALSE, 0);
	myicon = CreateComboBoxWithModel(icon_model, ctr.myicon);
	gtk_box_pack_start(GTK_BOX(hbox), myicon, TRUE, TRUE, 0);
	button = create_button("...");
	g_signal_connect_swapped(button, "clicked", G_CALLBACK(AddPalIcon),
				 myicon);
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

	hbox = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 5);
	save_path = create_label(_("Save file to:"));
	gtk_box_pack_start(GTK_BOX(hbox), save_path, FALSE, FALSE, 0);
	save_path = CreateFolderChooser();
	gtk_box_pack_start(GTK_BOX(hbox), save_path, TRUE, TRUE, 0);

	/***************************/
	hbox = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 5);
	/***************************/
	frame = create_frame(_("Personal portrait"));
	gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 3);
	button = gtk_button_new();
	gtk_widget_show(button);
	gtk_widget_set_size_request(button, MAX_PREVIEWSIZE, MAX_PREVIEWSIZE);
	gtk_container_add(GTK_CONTAINER(frame), button);
	ad = gtk_image_new();
	gtk_widget_show(ad);
	gtk_container_add(GTK_CONTAINER(button), ad);
	snprintf(path, MAX_PATHBUF, "%s/.iptux/myad", getenv("HOME"));
	if (pixbuf = gdk_pixbuf_new_from_file(path, NULL)) {
		pixbuf_shrink_scale_1(&pixbuf, MAX_PREVIEWSIZE, MAX_PREVIEWSIZE);
		gtk_image_set_from_pixbuf(GTK_IMAGE(ad), pixbuf);
		g_object_unref(pixbuf);
	}
	g_signal_connect_swapped(button, "clicked", G_CALLBACK(ChoosePortrait), ad);

	frame = create_frame(_("Personal signature"));
	gtk_box_pack_end(GTK_BOX(hbox), frame, TRUE, TRUE, 3);
	sw = create_scrolled_window();
	gtk_container_add(GTK_CONTAINER(frame), sw);
	sign = create_text_view();
	gtk_container_add(GTK_CONTAINER(sw), sign);
	gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(sign)),
					 ctr.sign?ctr.sign:"", -1);
}

void IptuxSetup::CreateSystem(GtkWidget * note)
{
	extern Control ctr;
	GtkWidget *label, *button;
	GtkWidget *box, *hbox;

	box = create_box();
	label = create_label(_("System Setup"));
	gtk_notebook_append_page(GTK_NOTEBOOK(note), box, label);

	hbox = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 5);
	encode = create_label(_("Default network encode:"));
	gtk_box_pack_start(GTK_BOX(hbox), encode, FALSE, FALSE, 0);
	encode = my_entry::create_entry(ctr.encode,
		_("Default network encode(before modify,you must understand what you are doing)"),
		FALSE);
	gtk_box_pack_start(GTK_BOX(hbox), encode, TRUE, TRUE, 0);

	hbox = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 5);
	palicon = create_label(_("Pal's default head portrait:"));
	gtk_box_pack_start(GTK_BOX(hbox), palicon, FALSE, FALSE, 0);
	palicon = CreateComboBoxWithModel(icon_model, ctr.palicon);
	gtk_box_pack_start(GTK_BOX(hbox), palicon, TRUE, TRUE, 0);
	button = create_button("...");
	g_signal_connect_swapped(button, "clicked", G_CALLBACK(AddPalIcon),
				 palicon);
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

	hbox = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 5);
	font = create_label(_("Font to show for panel:"));
	gtk_box_pack_start(GTK_BOX(hbox), font, FALSE, FALSE, 0);
	font = CreateFontChooser();
	gtk_box_pack_start(GTK_BOX(hbox), font, TRUE, TRUE, 0);

	tidy = gtk_check_button_new_with_label(_
					    ("Automatic clear up chat history"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tidy),
				     FLAG_ISSET(ctr.flags, 3));
	gtk_widget_show(tidy);
	gtk_box_pack_start(GTK_BOX(box), tidy, FALSE, FALSE, 0);

	log = gtk_check_button_new_with_label(_
					    ("Run the log of the message record"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(log),
				     FLAG_ISSET(ctr.flags, 2));
	gtk_widget_show(log);
	gtk_box_pack_start(GTK_BOX(box), log, FALSE, FALSE, 0);

	black = gtk_check_button_new_with_label(_
					    ("Use the blacklist(not recommended)"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(black),
				     FLAG_ISSET(ctr.flags, 1));
	gtk_widget_show(black);
	gtk_box_pack_start(GTK_BOX(box), black, FALSE, FALSE, 0);

	proof = gtk_check_button_new_with_label(_
					    ("Filter the request for shared files"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(proof),
				     FLAG_ISSET(ctr.flags, 0));
	gtk_widget_show(proof);
	gtk_box_pack_start(GTK_BOX(box), proof, FALSE, FALSE, 0);
}

void IptuxSetup::CreateIpseg(GtkWidget * note)
{
	extern Control ctr;
	char buf[MAX_BUF];
	GtkWidget *box, *label;
	GtkWidget *hbox, *button;
	GtkWidget *frame, *sw;

	box = create_box();
	label = create_label(_("IP Section Setup"));
	gtk_notebook_append_page(GTK_NOTEBOOK(note), box, label);

	hbox = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 5);
	entry1 = create_label(_("Begin:"));
	gtk_box_pack_start(GTK_BOX(hbox), entry1, FALSE, FALSE, 0);
	entry1 = my_entry::create_entry(NULL, _("begin IPv4"), TRUE);
	gtk_box_pack_start(GTK_BOX(hbox), entry1, TRUE, TRUE, 0);
	entry2 = create_label(_("End:"));
	gtk_box_pack_start(GTK_BOX(hbox), entry2, FALSE, FALSE, 0);
	entry2 = my_entry::create_entry(NULL, _("end IPv4"), TRUE);
	gtk_box_pack_start(GTK_BOX(hbox), entry2, TRUE, TRUE, 0);

	hbox = create_button_box(FALSE);
	gtk_button_box_set_layout(GTK_BUTTON_BOX(hbox), GTK_BUTTONBOX_SPREAD);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 5);
	snprintf(buf, MAX_BUF, "%s↓↓", _("Add"));
	button = create_button(buf);
	g_signal_connect_swapped(button, "clicked", G_CALLBACK(ClickAddIpseg),
				 this);
	gtk_box_pack_end(GTK_BOX(hbox), button, FALSE, FALSE, 0);
	snprintf(buf, MAX_BUF, "%s↑↑", _("Delete"));
	button = create_button(buf);
	g_signal_connect_swapped(button, "clicked", G_CALLBACK(ClickDelIpseg),
				 this);
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

	frame = create_frame(_("Added IPv4 section:"));
	gtk_box_pack_start(GTK_BOX(box), frame, TRUE, TRUE, 5);
	sw = create_scrolled_window();
	gtk_container_add(GTK_CONTAINER(frame), sw);
	ipseg_view = CreateIpsegView();
	gtk_container_add(GTK_CONTAINER(sw), ipseg_view);
}

void IptuxSetup::CreateFuncButton(GtkWidget * hbb)
{
	GtkWidget *button;

	button = create_button(_("OK"));
	g_signal_connect_swapped(button, "clicked", G_CALLBACK(ClickOk), this);
	gtk_box_pack_end(GTK_BOX(hbb), button, FALSE, FALSE, 1);
	button = create_button(_("Apply"));
	g_signal_connect_swapped(button, "clicked", G_CALLBACK(ClickApply),
				 this);
	gtk_box_pack_end(GTK_BOX(hbb), button, FALSE, FALSE, 1);
	button = create_button(_("Cancel"));
	g_signal_connect_swapped(button, "clicked",
				 G_CALLBACK(gtk_widget_destroy), setup);
	gtk_box_pack_end(GTK_BOX(hbb), button, FALSE, FALSE, 1);
}

//IP 2,0 ip,1 ip
GtkTreeModel *IptuxSetup::CreateIpModel()
{
	extern Control ctr;
	GtkListStore *model;
	GtkTreeIter iter;
	GSList *tmp;

	model = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
	pthread_mutex_lock(&ctr.mutex);
	tmp = ctr.ipseg;
	while (tmp) {
		gtk_list_store_append(model, &iter);
		gtk_list_store_set(model, &iter, 0, (gchar *) tmp->data, -1);
		tmp = tmp->next;
		gtk_list_store_set(model, &iter, 1, (gchar *) tmp->data, -1);
		tmp = tmp->next;
	}
	pthread_mutex_unlock(&ctr.mutex);

	return GTK_TREE_MODEL(model);
}

GtkWidget *IptuxSetup::CreateFolderChooser()
{
	extern Control ctr;
	GtkWidget *chooser;

	chooser = gtk_file_chooser_button_new(_("Save file to"),
					      GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER
						       (chooser), TRUE);
	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(chooser), TRUE);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(chooser),
					    ctr.path);
	gtk_widget_show(chooser);

	return chooser;
}

GtkWidget *IptuxSetup::CreateFontChooser()
{
	extern Control ctr;
	GtkWidget *chooser;

	chooser = gtk_font_button_new_with_font(ctr.font);
	gtk_font_button_set_show_style(GTK_FONT_BUTTON(chooser), TRUE);
	gtk_font_button_set_show_size(GTK_FONT_BUTTON(chooser), TRUE);
	gtk_font_button_set_use_font(GTK_FONT_BUTTON(chooser), TRUE);
	gtk_font_button_set_use_size(GTK_FONT_BUTTON(chooser), TRUE);
	gtk_font_button_set_title(GTK_FONT_BUTTON(chooser),
				  _("Please choose a font"));
	gtk_widget_show(chooser);

	return chooser;
}

GtkWidget *IptuxSetup::CreateIpsegView()
{
	GtkWidget *view;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	view = gtk_tree_view_new_with_model(ip_model);
	gtk_widget_show(view);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_expand(column, TRUE);
	gtk_tree_view_column_set_title(column, _("begin IPv4"));
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_expand(column, TRUE);
	gtk_tree_view_column_set_title(column, _("end IPv4"));
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", 1, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

	return view;
}

bool IptuxSetup::CheckExist()
{
	if (!setup)
		return false;
	gtk_window_present(GTK_WINDOW(setup));
	return true;
}

//头像 2,0 pixbuf,1 iconfile
GtkTreeModel *IptuxSetup::CreateIconModel()
{
	extern Control ctr;
	GtkListStore *model;
	GdkPixbuf *pixbuf;
	GtkTreeIter iter;
	GSList *tmp;

	model = gtk_list_store_new(2, GDK_TYPE_PIXBUF, G_TYPE_STRING);
	pthread_mutex_lock(&ctr.mutex);
	tmp = ctr.iconlist;
	while (tmp) {
		pixbuf = gdk_pixbuf_new_from_file_at_size((char *)tmp->data,
							  MAX_ICONSIZE,
							  MAX_ICONSIZE, NULL);
		if (pixbuf) {
			gtk_list_store_append(model, &iter);
			gtk_list_store_set(model, &iter, 0, pixbuf, 1,
					   (char *)tmp->data, -1);
			g_object_unref(pixbuf);
		}
		tmp = tmp->next;
	}
	pthread_mutex_unlock(&ctr.mutex);

	return GTK_TREE_MODEL(model);
}

GtkWidget *IptuxSetup::CreateComboBoxWithModel(GtkTreeModel * model,
					       gchar * iconfile)
{
	GtkWidget *combo;
	GtkCellRenderer *renderer;
	gint active;

	combo = gtk_combo_box_new_with_model(model);
	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo), renderer, FALSE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo), renderer,
				       "pixbuf", 0, NULL);
	gtk_combo_box_set_wrap_width(GTK_COMBO_BOX(combo), 6);
	active = FileGetItemPos(iconfile, model);
	gtk_combo_box_set_active(GTK_COMBO_BOX(combo), active);
	gtk_widget_show(combo);

	return combo;
}

gint IptuxSetup::FileGetItemPos(const char *filename, GtkTreeModel * model)
{
	GdkPixbuf *pixbuf;
	GtkTreeIter iter;
	gchar *tmp;
	gint pos;

	pos = 0;
	if (gtk_tree_model_get_iter_first(model, &iter)) {
		do {
			gtk_tree_model_get(model, &iter, 1, &tmp, -1);
			if (strcmp(filename, tmp) == 0) {
				g_free(tmp);
				return pos;
			}
			g_free(tmp);
			pos++;
		} while (gtk_tree_model_iter_next(model, &iter));
	}
	if (access(filename, F_OK) != 0 ||
	    !(pixbuf =
	      gdk_pixbuf_new_from_file_at_size(filename, MAX_ICONSIZE,
					       MAX_ICONSIZE, NULL)))
		return -1;
	gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, pixbuf, 1, filename,
			   -1);
	g_object_unref(pixbuf);

	return pos;
}

void IptuxSetup::ObtainPerson(gpointer data)
{
	extern Control ctr;
	GtkTextIter start, end;
	GtkTextBuffer *buffer;
	GdkPixbuf *pixbuf;
	GtkTreeIter iter;
	IptuxSetup *ipst;
	char buf[MAX_PATHBUF];
	const char *text;
	gint active;

	ipst = (IptuxSetup *) data;
	text = gtk_entry_get_text(GTK_ENTRY(ipst->myname));
	free(ctr.myname);
	ctr.myname = Strdup(text);

	active = gtk_combo_box_get_active(GTK_COMBO_BOX(ipst->myicon));
	snprintf(buf, MAX_PATHBUF, "%d", active);
	gtk_tree_model_get_iter_from_string(ipst->icon_model, &iter, buf);
	free(ctr.myicon);
	gtk_tree_model_get(ipst->icon_model, &iter, 1, &ctr.myicon, -1);
	if (strncmp(ctr.myicon, __ICON_DIR, strlen(__ICON_DIR))) {
		snprintf(buf, MAX_PATHBUF, "%s/.iptux/myicon", getenv("HOME"));
		pixbuf =
		    gdk_pixbuf_new_from_file_at_size(ctr.myicon, MAX_ICONSIZE,
						     MAX_ICONSIZE, NULL);
		gdk_pixbuf_save(pixbuf, buf, "png", NULL, NULL);
		g_object_unref(pixbuf);
	}

	free(ctr.path);
	ctr.path =
	    gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(ipst->save_path));

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ipst->sign));
	gtk_text_buffer_get_bounds(buffer, &start, &end);
	free(ctr.sign);
	ctr.sign = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
}

void IptuxSetup::ObtainSystem(gpointer data)
{
	extern Control ctr;
	GtkTreeIter iter;
	IptuxSetup *ipst;
	char buf[MAX_BUF];
	const char *text;
	gint active;

	ipst = (IptuxSetup *) data;
	text = gtk_entry_get_text(GTK_ENTRY(ipst->encode));
	free(ctr.encode);
	ctr.encode = Strdup(text);

	active = gtk_combo_box_get_active(GTK_COMBO_BOX(ipst->palicon));
	snprintf(buf, MAX_BUF, "%d", active);
	gtk_tree_model_get_iter_from_string(ipst->icon_model, &iter, buf);
	free(ctr.palicon);
	gtk_tree_model_get(ipst->icon_model, &iter, 1, &ctr.palicon, -1);

	text = gtk_font_button_get_font_name(GTK_FONT_BUTTON(ipst->font));
	free(ctr.font);
	ctr.font = Strdup(text);

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ipst->tidy)))
		FLAG_SET(ctr.flags, 3);
	else
		FLAG_CLR(ctr.flags, 3);

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ipst->log)))
		FLAG_SET(ctr.flags, 2);
	else
		FLAG_CLR(ctr.flags, 2);

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ipst->black)))
		FLAG_SET(ctr.flags, 1);
	else
		FLAG_CLR(ctr.flags, 1);

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ipst->proof)))
		FLAG_SET(ctr.flags, 0);
	else
		FLAG_CLR(ctr.flags, 0);
}

void IptuxSetup::ObtainIpseg(gpointer data)
{
	extern Control ctr;
	GtkTreeIter iter;
	IptuxSetup *ipst;
	gchar *ipstr1, *ipstr2;

	ipst = (IptuxSetup *) data;
	pthread_mutex_lock(&ctr.mutex);
	g_slist_foreach(ctr.ipseg, remove_foreach, GINT_TO_POINTER(UNKNOWN));
	g_slist_free(ctr.ipseg), ctr.ipseg = NULL;
	if (gtk_tree_model_get_iter_first(ipst->ip_model, &iter)) {
		do {
			gtk_tree_model_get(ipst->ip_model, &iter,
					   0, &ipstr1, 1, &ipstr2, -1);
			ctr.ipseg = g_slist_append(ctr.ipseg, ipstr1);
			ctr.ipseg = g_slist_append(ctr.ipseg, ipstr2);
		} while (gtk_tree_model_iter_next(ipst->ip_model, &iter));
	}
	pthread_mutex_unlock(&ctr.mutex);
}

void IptuxSetup::UpdateMyInfo()
{
	extern struct interactive inter;
	extern UdpData udt;
	Command cmd;
	GSList *tmp;

	pthread_mutex_lock(&udt.mutex);
	tmp = udt.pallist;
	while (tmp) {
		cmd.SendAbsence(inter.udpsock, tmp->data);
		if (FLAG_ISSET(((Pal*)tmp->data)->flags, 0))
			thread_create(ThreadFunc(Pal::SendFeature), tmp->data, false);
		tmp = tmp->next;
	}
	pthread_mutex_unlock(&udt.mutex);
}

void IptuxSetup::AddPalIcon(GtkWidget *combo)
{
	GtkWidget *chooser;
	GtkTreeModel *model;
	gchar *filename;
	gint active;

	chooser =
	    my_chooser::create_chooser(_("Please choose a head portrait"),
				       setup);
	filename = my_chooser::run_chooser(chooser);
	if (!filename)
		return;
	model = gtk_combo_box_get_model(GTK_COMBO_BOX(combo));
	active = FileGetItemPos(filename, model);
	gtk_combo_box_set_active(GTK_COMBO_BOX(combo), active);
	g_free(filename);
}

void IptuxSetup::ClickAddIpseg(gpointer data)
{
	const gchar *text1, *text2, *text;
	IptuxSetup *ipst;
	in_addr_t ip1, ip2;
	GtkTreeIter iter;
	int status;

	ipst = (IptuxSetup *) data;
	text1 = gtk_entry_get_text(GTK_ENTRY(ipst->entry1));
	status = inet_pton(AF_INET, text1, &ip1);
	if (status <= 0) {
		pop_warning(setup, NULL, _("\nThe address %s is illegal!"),
			    text1);
		return;
	}
	text2 = gtk_entry_get_text(GTK_ENTRY(ipst->entry2));
	status = inet_pton(AF_INET, text2, &ip2);
	if (status <= 0) {
		pop_warning(setup, NULL, _("\nThe address %s is illegal!"),
			    text2);
		return;
	}

	ip1 = ntohl(ip1), ip2 = ntohl(ip2);
	if (ip1 > ip2) {
		text = text1;
		text1 = text2;
		text2 = text;
	}

	gtk_list_store_append(GTK_LIST_STORE(ipst->ip_model), &iter);
	gtk_list_store_set(GTK_LIST_STORE(ipst->ip_model), &iter,
			   0, text1, 1, text2, -1);
	gtk_entry_set_text(GTK_ENTRY(ipst->entry1), "\0");
	gtk_entry_set_text(GTK_ENTRY(ipst->entry2), "\0");
}

void IptuxSetup::ClickDelIpseg(gpointer data)
{
	gchar *ipstr1, *ipstr2;
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	IptuxSetup *ipst;

	ipst = (IptuxSetup *) data;
	selection =
	    gtk_tree_view_get_selection(GTK_TREE_VIEW(ipst->ipseg_view));
	if (!gtk_tree_selection_get_selected(selection, NULL, &iter))
		return;
	gtk_tree_model_get(ipst->ip_model, &iter, 0, &ipstr1, 1, &ipstr2, -1);
	gtk_entry_set_text(GTK_ENTRY(ipst->entry1), ipstr1);
	gtk_entry_set_text(GTK_ENTRY(ipst->entry2), ipstr2);
	gtk_list_store_remove(GTK_LIST_STORE(ipst->ip_model), &iter);
}

void IptuxSetup::ClickOk(gpointer data)
{
	IptuxSetup::ClickApply(data);
	gtk_widget_destroy(setup);
}

void IptuxSetup::ClickApply(gpointer data)
{
	extern Control ctr;

	ObtainPerson(data);
	ObtainSystem(data);
	ObtainIpseg(data);
	ctr.dirty = true;
	IptuxSetup::UpdateMyInfo();
}

void IptuxSetup::SetupDestroy(gpointer data)
{
	setup = NULL;
	delete(IptuxSetup *) data;
}

void IptuxSetup::ChoosePortrait(GtkWidget *image)
{
	gchar path[MAX_PATHBUF], *filename;
	GtkWidget *chooser;
	GdkPixbuf *pixbuf;

	chooser = my_chooser::create_chooser(
			     _("Please choose a personal portrait"), setup);
	filename = my_chooser::run_chooser(chooser);
	if (!filename)
		return;
	pixbuf = gdk_pixbuf_new_from_file(filename, NULL);
	g_free(filename);
	if (pixbuf) {
		snprintf(path, MAX_PATHBUF, "%s/.iptux/myad", getenv("HOME"));
		pixbuf_shrink_scale_1(&pixbuf, MAX_ADSIZE, MAX_ADSIZE);
		gdk_pixbuf_save(pixbuf, path, "png", NULL, NULL);	//命中率极高，不妨直接保存
		pixbuf_shrink_scale_1(&pixbuf, MAX_PREVIEWSIZE, MAX_PREVIEWSIZE);
		gtk_image_set_from_pixbuf(GTK_IMAGE(image), pixbuf);
		g_object_unref(pixbuf);
	}
}
