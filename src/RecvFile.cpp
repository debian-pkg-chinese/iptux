//
// C++ Implementation: RecvFile
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "RecvFile.h"
#include "DialogGroup.h"
#include "Transport.h"
#include "Control.h"
#include "utils.h"
#include "output.h"
#include "baling.h"

 RecvFile::RecvFile(gpointer data):file_list(NULL),
packetn(0), file_model(NULL)
{
	pal = (Pal *) ((struct RECVFILE_PARA *)data)->data;
	msg = ((struct RECVFILE_PARA *)data)->msg;
	free(data);
}

RecvFile::~RecvFile()
{
	free(msg);
	g_slist_foreach(file_list, remove_each_info, GINT_TO_POINTER(FILEINFO));
	g_slist_free(file_list);
	//g_object_unref(file_model); //他处释放
}

void RecvFile::RecvEntry(gpointer data)
{
	RecvFile rf(data);

	if (!rf.GetValidData())
		return;
	rf.ParseExtra();

	gdk_threads_enter();
	rf.CreateRecvWindow();
	gdk_threads_leave();
}

bool RecvFile::GetValidData()
{
	char *ptr;

	packetn = iptux_get_dec_number(msg, 1);
	ptr = ipmsg_get_extra(msg);
	if (!ptr)
		return false;

	free(msg);
	if (!(pal->flags & BIT1)) {
		msg = transfer_encode(ptr, pal->encode, false);
		free(ptr);
	} else
		msg = ptr;

	return true;
}

void RecvFile::ParseExtra()
{
	char *ptr;

	ptr = msg;
	while (ptr && *ptr)
		file_list = g_slist_append(file_list, DivideFileinfo(&ptr));
}

void RecvFile::CreateRecvWindow()
{
	GtkWidget *window, *sw, *view;
	GtkWidget *box, *hbb, *button;

	file_model = CreateRecvModel();
	window = create_window(_("File receive management"), 132, 79);
	gtk_container_set_border_width(GTK_CONTAINER(window), 5);
	g_signal_connect_swapped(window, "destroy", G_CALLBACK(g_object_unref),
				 file_model);
	box = create_box();
	gtk_container_add(GTK_CONTAINER(window), box);

	view = CreateRecvView();
	sw = create_scrolled_window();
	gtk_box_pack_start(GTK_BOX(box), sw, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(sw), view);

	hbb = create_button_box(FALSE);
	gtk_box_pack_start(GTK_BOX(box), hbb, FALSE, FALSE, 0);
	button = create_button(_("Receive"));
	g_signal_connect_swapped(button, "clicked",
				 G_CALLBACK(AddRecvFile), file_model);
	g_signal_connect_swapped(button, "clicked",
				 G_CALLBACK(gtk_widget_destroy), window);
	gtk_box_pack_end(GTK_BOX(hbb), button, FALSE, FALSE, 0);
	button = create_button(_("Refuse"));
	g_signal_connect_swapped(button, "clicked",
				 G_CALLBACK(gtk_widget_destroy), window);
	gtk_box_pack_end(GTK_BOX(hbb), button, FALSE, FALSE, 0);
}

gpointer RecvFile::DivideFileinfo(char **ptr)
{
	FileInfo *file;

	file = new FileInfo;
	file->fileid = iptux_get_dec_number(*ptr, 0);
	file->filename = ipmsg_get_filename(*ptr, 1);
	file->filesize = iptux_get_hex_number(*ptr, 2);
	file->fileattr = iptux_get_hex_number(*ptr, 4);

	//格式1 ... ;格式2 ...\a ;格式3 ...\a:
	*ptr = strstr(*ptr, "\a:");
	if (*ptr)
		*ptr += 2;

	return file;
}

//10,0 bool,1 filename,2 owner,3 size,4 type,5 packetn, 6 fileid,7 size,8 type 9 data
GtkTreeModel *RecvFile::CreateRecvModel()
{
	GtkListStore *model;
	GtkTreeIter iter;
	FileInfo *file;
	GSList *tmp;
	char *ptr;

	model = gtk_list_store_new(10, G_TYPE_BOOLEAN,
				   G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
				   G_TYPE_STRING, G_TYPE_UINT, G_TYPE_UINT,
				   G_TYPE_UINT, G_TYPE_UINT, G_TYPE_POINTER);
	tmp = file_list;
	while (tmp) {
		file = (FileInfo *) tmp->data;
		ptr = number_to_string(file->filesize);
		gtk_list_store_append(model, &iter);
		gtk_list_store_set(model, &iter, 0, TRUE, 1, file->filename,
				   2, pal->name, 3, ptr, 5, packetn, 6,
				   file->fileid, 7, file->filesize, 8,
				   file->fileattr, 9, pal, -1);
		free(ptr);
		if (GET_MODE(file->fileattr) == IPMSG_FILE_REGULAR)
			gtk_list_store_set(model, &iter, 4, _("regular"), -1);
		else if (GET_MODE(file->fileattr) == IPMSG_FILE_DIR)
			gtk_list_store_set(model, &iter, 4, _("directory"), -1);
		else
			gtk_list_store_set(model, &iter, 4, _("unknown"), -1);
		tmp = tmp->next;
	}

	return GTK_TREE_MODEL(model);
}

GtkWidget *RecvFile::CreateRecvView()
{
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;
	GtkWidget *view;

	view = gtk_tree_view_new_with_model(file_model);
	g_signal_connect_swapped(view, "button-press-event",
				 G_CALLBACK(DialogGroup::ViewPopMenu),
				 file_model);
	gtk_widget_show(view);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_title(column, _("receive"));
	renderer = gtk_cell_renderer_toggle_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "active", 0,
					    NULL);
	g_signal_connect_swapped(renderer, "toggled",
				 G_CALLBACK(DialogGroup::ViewToggleChange),
				 file_model);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_title(column, _("filename"));
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", 1, NULL);
	g_object_set(renderer, "editable", TRUE, NULL);
	g_signal_connect(renderer, "edited", G_CALLBACK(CellEditText),
			 file_model);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_title(column, _("belong"));
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", 2, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_title(column, _("length"));
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", 3, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_title(column, _("type"));
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", 4, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

	return view;
}

void RecvFile::CellEditText(GtkCellRendererText * renderer, gchar * path,
			    gchar * new_text, GtkTreeModel * model)
{
	GtkTreeIter iter;

	gtk_tree_model_get_iter_from_string(model, &iter, path);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 1, new_text, -1);
}

void RecvFile::AddRecvFile(GtkTreeModel * model)
{
	extern Control ctr;
	extern Transport trans;
	uint32_t packetn, fileid, filesize, fileattr;
	gchar *filename, *filestr;
	GtkTreeIter iter1, iter2;
	GtkTreePath *path;
	GdkPixbuf *pixbuf;
	gboolean active;
	bool demand;
	Pal *pal;

	demand = false;
	if (!gtk_tree_model_get_iter_first(model, &iter1))
		return;
	pixbuf = gdk_pixbuf_new_from_file(__TIP_DIR "/recv.png", NULL);
	do {
		gtk_tree_model_get(model, &iter1, 0, &active, 1, &filename,
				   3, &filestr, 5, &packetn, 6, &fileid, 7,
				   &filesize, 8, &fileattr, 9, &pal, -1);
		if (!active) {
			g_free(filename), g_free(filestr);
			continue;
		}
		demand = true;

		gtk_list_store_append(GTK_LIST_STORE(trans.trans_model),
				      &iter2);
		gtk_list_store_set(GTK_LIST_STORE(trans.trans_model), &iter2, 0,
				   pixbuf, 1, _("receive"), 2, filename, 3,
				   pal->name, 4, "0B", 5, filestr, 6, "0B/s", 7,
				   0, 8, FALSE, 9, packetn, 10, fileid, 11,
				   filesize, 12, fileattr, 13, ctr.path, 14,
				   pal, -1);
		g_free(filename), g_free(filestr);

		path = gtk_tree_model_get_path(trans.trans_model, &iter2);
		thread_create(ThreadFunc(Transport::RecvFileEntry), path,
			      false);
	} while (gtk_tree_model_iter_next(model, &iter1));
	if (pixbuf)
		g_object_unref(pixbuf);
	if (demand)
		Transport::TransportEntry();
}