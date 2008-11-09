//
// C++ Implementation: RevisePal
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "RevisePal.h"
#include "IptuxSetup.h"
#include "UdpData.h"
#include "my_entry.h"
#include "baling.h"
#include "utils.h"

 RevisePal::RevisePal(gpointer data):pal((Pal *) data),
revise(NULL), icon_model(NULL), name(NULL),
encode(NULL), icon(NULL)
{
}

RevisePal::~RevisePal()
{
	g_object_unref(icon_model);
}

void RevisePal::ReviseEntry(gpointer data)
{
	RevisePal rp(data);

	rp.InitRevise();
	rp.CreateRevise();
	rp.RunRevise();
}

void RevisePal::InitRevise()
{
	icon_model = IptuxSetup::CreateIconModel();
}

void RevisePal::CreateRevise()
{
	extern struct interactive inter;
	GtkWidget *box, *button;

	revise = gtk_dialog_new_with_buttons(_("Change pal's information"),
					     GTK_WINDOW(inter.window),
					     GTK_DIALOG_MODAL,
					     _("OK"), GTK_RESPONSE_OK,
					     _("Cancel"), GTK_RESPONSE_CANCEL,
					     NULL);
	gtk_dialog_set_default_response(GTK_DIALOG(revise), GTK_RESPONSE_OK);

	box = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(revise)->vbox),
			   box, FALSE, FALSE, 0);
	name = create_label(_("Pal's nickname:"));
	gtk_box_pack_start(GTK_BOX(box), name, FALSE, FALSE, 0);
	name =
	    my_entry::create_entry(pal->name,
				   _("Please input pal's new nickname!"),
				   FALSE);
	gtk_box_pack_start(GTK_BOX(box), name, TRUE, TRUE, 0);

	box = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(revise)->vbox),
			   box, FALSE, FALSE, 0);
	encode = create_label(_("System encode:"));
	gtk_box_pack_start(GTK_BOX(box), encode, FALSE, FALSE, 0);
	encode = my_entry::create_entry(pal->encode,
					_
					("you must understand what you are doing!"),
					FALSE);
	gtk_box_pack_start(GTK_BOX(box), encode, TRUE, TRUE, 0);

	box = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(revise)->vbox),
			   box, FALSE, FALSE, 0);
	icon = create_label(_("Head portrait:"));
	gtk_box_pack_start(GTK_BOX(box), icon, FALSE, FALSE, 0);
	icon = IptuxSetup::CreateComboBoxWithModel(icon_model, pal->iconfile);
	gtk_box_pack_start(GTK_BOX(box), icon, TRUE, TRUE, 0);
	button = create_button("...");
	g_signal_connect_swapped(button, "clicked", G_CALLBACK(IptuxSetup::AddPalIcon), icon);
	gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);
}

void RevisePal::RunRevise()
{
	int result;

	result = gtk_dialog_run(GTK_DIALOG(revise));
	if (result == GTK_RESPONSE_OK)
		ApplyRevise();
	gtk_widget_destroy(revise);
}

void RevisePal::ApplyRevise()
{
	extern UdpData udt;
	char buf[MAX_BUF];
	GtkTreeIter iter,parent;
	gint active;

	free(pal->name), free(pal->encode);
	pal->name = gtk_editable_get_chars(GTK_EDITABLE(name), 0, -1);
	pal->encode = gtk_editable_get_chars(GTK_EDITABLE(encode), 0, -1);
	active = gtk_combo_box_get_active(GTK_COMBO_BOX(icon));
	snprintf(buf, MAX_BUF, "%d", active);
	gtk_tree_model_get_iter_from_string(icon_model, &iter, buf);
	free(pal->iconfile);
	gtk_tree_model_get(icon_model, &iter, 1, &pal->iconfile, -1);
	udt.Ipv4GetParent(pal->ipv4, &parent);
	if (udt.PalGetModelIter(pal, &parent, &iter))
		pal->SetPalmodelValue(udt.pal_model, &iter);
	FLAG_SET(pal->flags, 2);
}
