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
#include "UdpData.h"
#include "Command.h"
#include "output.h"
#include "baling.h"
#include "utils.h"

GtkWidget *IptuxSetup::setup = NULL;
 IptuxSetup::IptuxSetup():icon_model(NULL), ip_model(NULL),
myname(NULL), myicon(NULL), save_path(NULL), encode(NULL),
palicon(NULL), black(NULL), proof(NULL), entry1(NULL),
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

//头像 2,0 pixbuf,1 icon
GtkTreeModel *IptuxSetup::CreateIconModel()
{
	char buf[MAX_PATH_BUF];
	GtkListStore *model;
	GdkPixbuf *pixbuf;
	GtkTreeIter iter;
	uint8_t count;

	model = gtk_list_store_new(2, GDK_TYPE_PIXBUF, G_TYPE_UINT);
	count = 0;
	while (count < Control::sum_icon) {
		gtk_list_store_append(model, &iter);
		snprintf(buf, MAX_PATH_BUF, __ICON_DIR "/%hhu.png", count);
		pixbuf = gdk_pixbuf_new_from_file(buf, NULL);
		gtk_list_store_set(model, &iter, 0, pixbuf, 1, count, -1);
		if (pixbuf)
			g_object_unref(pixbuf);
		count++;
	}

	return GTK_TREE_MODEL(model);
}

GtkWidget *IptuxSetup::CreateComboBoxWithModel(GtkTreeModel * model,
					       gint active)
{
	GtkWidget *combo;
	GtkCellRenderer *renderer;

	combo = gtk_combo_box_new_with_model(model);
	gtk_combo_box_set_active(GTK_COMBO_BOX(combo), active);
	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo), renderer, FALSE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo), renderer,
				       "pixbuf", 0, NULL);
	gtk_combo_box_set_wrap_width(GTK_COMBO_BOX(combo), 6);
	gtk_widget_show(combo);

	return combo;
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

	setup = create_window(_("Iptux setup"), 132, 79);
	gtk_container_set_border_width(GTK_CONTAINER(setup), 5);
	g_signal_connect_swapped(setup, "destroy", G_CALLBACK(SetupDestroy),
				 this);
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
	CreateButton(hbb);
}

void IptuxSetup::CreatePerson(GtkWidget * note)
{
	extern Control ctr;
	GtkWidget *box, *label;
	GtkWidget *hbox;

	box = create_box();
	label = create_label(_("Personal Setup"));
	gtk_notebook_append_page(GTK_NOTEBOOK(note), box, label);

	hbox = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 5);
	myname = create_label(_("Your nickname:"));
	gtk_box_pack_start(GTK_BOX(hbox), myname, FALSE, FALSE, 0);
	myname =
	    my_entry::create_entry(ctr.myname, _("Please Input your nickname!"),
				   FALSE);
	gtk_box_pack_start(GTK_BOX(hbox), myname, TRUE, TRUE, 0);

	hbox = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 5);
	myicon = create_label(_("Your head portrait:"));
	gtk_box_pack_start(GTK_BOX(hbox), myicon, FALSE, FALSE, 0);
	myicon = CreateComboBoxWithModel(icon_model, ctr.myicon);
	gtk_box_pack_start(GTK_BOX(hbox), myicon, TRUE, TRUE, 0);

	hbox = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 5);
	save_path = create_label(_("Save file to:"));
	gtk_box_pack_start(GTK_BOX(hbox), save_path, FALSE, FALSE, 0);
	save_path = CreateFolderChooser(ctr.path);
	gtk_box_pack_start(GTK_BOX(hbox), save_path, TRUE, TRUE, 0);
}

void IptuxSetup::CreateSystem(GtkWidget * note)
{
	extern Control ctr;
	GtkWidget *box, *label;
	GtkWidget *hbox;

	box = create_box();
	label = create_label(_("System Setup"));
	gtk_notebook_append_page(GTK_NOTEBOOK(note), box, label);

	hbox = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 5);
	encode = create_label(_("Default network encode:"));
	gtk_box_pack_start(GTK_BOX(hbox), encode, FALSE, FALSE, 0);
	encode = my_entry::create_entry(ctr.encode,
					_
					("Default network encode(before modify,you must understand what you are doing)"),
					FALSE);
	gtk_box_pack_start(GTK_BOX(hbox), encode, TRUE, TRUE, 0);

	hbox = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 5);
	palicon = create_label(_("Pal's default head portrait:"));
	gtk_box_pack_start(GTK_BOX(hbox), palicon, FALSE, FALSE, 0);
	palicon = CreateComboBoxWithModel(icon_model, ctr.palicon);
	gtk_box_pack_start(GTK_BOX(hbox), palicon, TRUE, TRUE, 0);

	black =
	    gtk_check_button_new_with_label
	    (_("Use the blacklist(not recommended)"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(black),
				     ctr.flags & BIT2);
	gtk_widget_show(black);
	gtk_box_pack_start(GTK_BOX(box), black, FALSE, FALSE, 5);

	proof =
	    gtk_check_button_new_with_label(_
					    ("Filter the request for shared files"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(proof),
				     ctr.flags & BIT1);
	gtk_widget_show(proof);
	gtk_box_pack_start(GTK_BOX(box), proof, FALSE, FALSE, 5);
}

void IptuxSetup::CreateIpseg(GtkWidget * note)
{
	extern Control ctr;
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
	button = create_button(_("Add"));
	g_signal_connect_swapped(button, "clicked", G_CALLBACK(ClickAddIpseg),
				 this);
	gtk_box_pack_end(GTK_BOX(hbox), button, FALSE, FALSE, 0);
	button = create_button(_("Delete"));
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

void IptuxSetup::CreateButton(GtkWidget * hbb)
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
		gtk_list_store_set(model, &iter, 0, tmp->data, -1);
		tmp = tmp->next;
		gtk_list_store_set(model, &iter, 1, tmp->data, -1);
		tmp = tmp->next;
	}
	pthread_mutex_unlock(&ctr.mutex);

	return GTK_TREE_MODEL(model);
}

GtkWidget *IptuxSetup::CreateFolderChooser(const char *folder)
{
	GtkWidget *chooser;

	chooser = gtk_file_chooser_button_new(_("Save file to"),
					      GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER
						       (chooser), TRUE);
	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(chooser), TRUE);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(chooser), folder);
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

void IptuxSetup::FreshMyInfo()
{
	extern UdpData udt;
	Command cmd;
	GSList *tmp;
	int sock;

	sock = Socket(PF_INET, SOCK_DGRAM, 0);
	pthread_mutex_lock(&udt.mutex);
	tmp = udt.pallist;
	while (tmp) {
		cmd.SendAbsence(sock, tmp->data);
		tmp = tmp->next;
	}
	pthread_mutex_unlock(&udt.mutex);
	close(sock);
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
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	IptuxSetup *ipst;

	ipst = (IptuxSetup *) data;
	selection =
	    gtk_tree_view_get_selection(GTK_TREE_VIEW(ipst->ipseg_view));
	if (gtk_tree_selection_get_selected(selection, NULL, &iter))
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
	GtkTreeIter iter;
	IptuxSetup *ipst;
	const char *text;
	char *ipstr1, *ipstr2;

	ipst = (IptuxSetup *) data;

	text = gtk_entry_get_text(GTK_ENTRY(ipst->myname));
	free(ctr.myname);
	ctr.myname = Strdup(text);

	ctr.myicon = gtk_combo_box_get_active(GTK_COMBO_BOX(ipst->myicon));

	free(ctr.path);
	ctr.path =
	    gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(ipst->save_path));

	text = gtk_entry_get_text(GTK_ENTRY(ipst->encode));
	free(ctr.encode);
	ctr.encode = Strdup(text);

	ctr.palicon = gtk_combo_box_get_active(GTK_COMBO_BOX(ipst->palicon));

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ipst->black)))
		ctr.flags |= BIT2;
	else
		ctr.flags &= ~BIT2;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ipst->proof)))
		ctr.flags |= BIT1;
	else
		ctr.flags &= ~BIT1;

	pthread_mutex_lock(&ctr.mutex);
	g_slist_foreach(ctr.ipseg, remove_each_info, GINT_TO_POINTER(UNKNOWN));
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

	ctr.dirty = true;
	IptuxSetup::FreshMyInfo();
}

void IptuxSetup::SetupDestroy(gpointer data)
{
	setup = NULL;
	delete(IptuxSetup *) data;
}
