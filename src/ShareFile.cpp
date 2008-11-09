//
// C++ Implementation: ShareFile
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "ShareFile.h"
#include "SendFile.h"
#include "output.h"
#include "baling.h"
#include "utils.h"

GtkWidget *ShareFile::share = NULL;
 ShareFile::ShareFile():share_view(NULL), share_model(NULL)
{
}

ShareFile::~ShareFile()
{
	g_object_unref(share_model);
}

void ShareFile::ShareEntry()
{
	ShareFile *sf;

	if (ShareFile::CheckExist())
		return;
	sf = new ShareFile;
	sf->InitShare();
	sf->CreateShare();
}

void ShareFile::InitShare()
{
	share_model = CreateSharedModel();
}

void ShareFile::CreateShare()
{
	GtkTargetEntry target = { "text/plain", 0, 0 };
	GtkWidget *vbox, *hbox;
	GtkWidget *bb, *button;
	GtkWidget *sw;

	share = create_window(_("Shared files management"), 132, 79);
	gtk_container_set_border_width(GTK_CONTAINER(share), 5);
	gtk_drag_dest_set(share, GTK_DEST_DEFAULT_ALL,
			  &target, 1, GDK_ACTION_MOVE);
	g_signal_connect_swapped(share, "drag-data-received",
				 G_CALLBACK(DragDataReceived), this);
	g_signal_connect_swapped(share, "destroy", G_CALLBACK(ShareDestroy),
				 this);
	gtk_container_set_border_width(GTK_CONTAINER(share), 5);
	vbox = create_box();
	gtk_container_add(GTK_CONTAINER(share), vbox);

	hbox = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
	sw = create_scrolled_window();
	gtk_box_pack_end(GTK_BOX(hbox), sw, TRUE, TRUE, 0);
	share_view = CreateSharedView();
	gtk_container_add(GTK_CONTAINER(sw), share_view);
	bb = create_button_box();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(bb), GTK_BUTTONBOX_START);
	gtk_box_pack_start(GTK_BOX(hbox), bb, FALSE, FALSE, 0);
	button = create_button(_("Add Files"));
	g_signal_connect_swapped(button, "clicked",
				 G_CALLBACK(AddRegular), this);
	gtk_box_pack_start(GTK_BOX(bb), button, FALSE, FALSE, 0);
	button = create_button(_("Add Folders"));
	g_signal_connect_swapped(button, "clicked",
				 G_CALLBACK(AddFolder), this);
	gtk_box_pack_start(GTK_BOX(bb), button, FALSE, FALSE, 0);
	button = create_button(_("Delete Shared"));
	g_signal_connect_swapped(button, "clicked",
				 G_CALLBACK(DeleteFiles), this);
	gtk_box_pack_start(GTK_BOX(bb), button, FALSE, FALSE, 0);

	bb = create_button_box(FALSE);
	gtk_box_pack_start(GTK_BOX(vbox), bb, FALSE, FALSE, 0);
	button = create_button(_("OK"));
	g_signal_connect_swapped(button, "clicked", G_CALLBACK(ClickOk), this);
	gtk_box_pack_end(GTK_BOX(bb), button, FALSE, FALSE, 0);
	button = create_button(_("Apply"));
	g_signal_connect_swapped(button, "clicked",
				 G_CALLBACK(ClickApply), this);
	gtk_box_pack_end(GTK_BOX(bb), button, FALSE, FALSE, 0);
	button = create_button(_("Cancel"));
	g_signal_connect_swapped(button, "clicked",
				 G_CALLBACK(gtk_widget_destroy), share);
	gtk_box_pack_end(GTK_BOX(bb), button, FALSE, FALSE, 0);
}

void ShareFile::AddSharedFiles(GSList * list)
{
	GdkPixbuf *pixbuf1, *pixbuf2;
	GdkPixbuf *pixbuf;
	GtkTreeIter iter;
	struct stat64 st;
	char *ptr;

	pixbuf1 = gdk_pixbuf_new_from_file(__TIP_DIR "/regular.png", NULL);
	pixbuf2 = gdk_pixbuf_new_from_file(__TIP_DIR "/folder.png", NULL);
	while (list) {
		if (Stat((char *)list->data, &st) == -1)
			continue;
		ptr = number_to_string(st.st_size);
		FindInsertPosition((char *)list->data,
			    S_ISREG(st.st_mode)?IPMSG_FILE_REGULAR:IPMSG_FILE_DIR, &iter);
		gtk_list_store_set(GTK_LIST_STORE(share_model), &iter,
				   1, (char *)list->data, 2, ptr,
				       4, (uint32_t) st.st_size, 5,
			   S_ISREG(st.st_mode)?IPMSG_FILE_REGULAR:IPMSG_FILE_DIR, -1);
		if (S_ISREG(st.st_mode))
			gtk_list_store_set(GTK_LIST_STORE(share_model), &iter,
					   0, pixbuf1, 3, _("regular"), -1);
		else
			gtk_list_store_set(GTK_LIST_STORE(share_model), &iter,
					   0, pixbuf2, 3, _("directory"), -1);
		free(ptr);
		list = list->next;
	}
	if (pixbuf1)
		g_object_unref(pixbuf1);
	if (pixbuf2)
		g_object_unref(pixbuf2);
}

void ShareFile::FindInsertPosition(const gchar * path, uint32_t fileattr,
				   GtkTreeIter * iter)
{
	GtkTreeIter sibling;
	gchar *tmp;
	guint attr;

	if (!gtk_tree_model_get_iter_first(share_model, iter)) {
		gtk_list_store_append(GTK_LIST_STORE(share_model), iter);
		return;
	}
	do {
		gtk_tree_model_get(share_model, iter, 1, &tmp, 5, &attr, -1);
		if (GET_MODE(fileattr) == IPMSG_FILE_DIR &&
			  GET_MODE(attr) != IPMSG_FILE_DIR ||
		  GET_MODE(fileattr) == attr && strcmp(tmp, path) > 0) {
			g_free(tmp), sibling = *iter;
			gtk_list_store_insert_before(GTK_LIST_STORE
						     (share_model), iter,
						     &sibling);
			return;
		}
		g_free(tmp);
	} while (gtk_tree_model_iter_next(share_model, iter));
	gtk_list_store_append(GTK_LIST_STORE(share_model), iter);
}

// 6,0 icon,1 path,2 size,3 type,4 size,5 type
GtkTreeModel *ShareFile::CreateSharedModel()
{
	extern SendFile sfl;
	GdkPixbuf *pixbuf1, *pixbuf2;
	GtkListStore *model;
	GtkTreeIter iter;
	FileInfo *file;
	GSList *tmp;
	char *ptr;

	model = gtk_list_store_new(6, GDK_TYPE_PIXBUF, G_TYPE_STRING,
				   G_TYPE_STRING, G_TYPE_STRING,
				   G_TYPE_UINT, G_TYPE_UINT);
	pixbuf1 = gdk_pixbuf_new_from_file(__TIP_DIR "/regular.png", NULL);
	pixbuf2 = gdk_pixbuf_new_from_file(__TIP_DIR "/folder.png", NULL);
	pthread_mutex_lock(&sfl.mutex);
	tmp = sfl.pblist;
	while (tmp) {
		file = (FileInfo *) tmp->data;
		ptr = number_to_string(file->filesize);
		gtk_list_store_append(model, &iter);
		gtk_list_store_set(model, &iter, 1, file->filename, 2, ptr,
				   4, file->filesize, 5, file->fileattr, -1);
		if (GET_MODE(file->fileattr) == IPMSG_FILE_REGULAR)
			gtk_list_store_set(model, &iter, 0, pixbuf1, 3,
					   _("regular"), -1);
		else
			gtk_list_store_set(model, &iter, 0, pixbuf2, 3,
					   _("directory"), -1);
		free(ptr);
		tmp = tmp->next;
	}
	pthread_mutex_unlock(&sfl.mutex);
	if (pixbuf1)
		g_object_unref(pixbuf1);
	if (pixbuf2)
		g_object_unref(pixbuf2);

	return GTK_TREE_MODEL(model);
}

GtkWidget *ShareFile::CreateSharedView()
{
	GtkWidget *view;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;
	GtkTreeSelection *selection;

	view = gtk_tree_view_new_with_model(share_model);
	gtk_widget_show(view);
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_title(column, _("file"));
	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "pixbuf", 0,
					    NULL);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", 1, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_title(column, _("length"));
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", 2, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_title(column, _("type"));
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", 3, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

	return view;
}

bool ShareFile::CheckExist()
{
	if (!share)
		return false;
	gtk_window_present(GTK_WINDOW(share));
	return true;
}

void ShareFile::AddRegular(gpointer data)
{
	GtkWidget *dialog;
	GSList *list;

	dialog =
	    gtk_file_chooser_dialog_new(_("Choose shared files"),
					GTK_WINDOW(share),
					GTK_FILE_CHOOSER_ACTION_OPEN,
					GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					NULL);
	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), getenv("HOME"));

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		list = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(dialog));
		((ShareFile *) data)->AddSharedFiles(list);
		g_slist_foreach(list, remove_each_info,
				GINT_TO_POINTER(UNKNOWN));
		g_slist_free(list);
	}
	gtk_widget_destroy(dialog);
}

void ShareFile::AddFolder(gpointer data)
{
	GtkWidget *dialog;
	GSList *list;

	dialog =
	    gtk_file_chooser_dialog_new(_("Choose shared folders"),
					GTK_WINDOW(share),
					GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
					GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					NULL);
	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), getenv("HOME"));

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		list = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(dialog));
		((ShareFile *) data)->AddSharedFiles(list);
		g_slist_foreach(list, remove_each_info,
				GINT_TO_POINTER(UNKNOWN));
		g_slist_free(list);
	}
	gtk_widget_destroy(dialog);
}

void ShareFile::DeleteFiles(gpointer data)
{
	GtkTreeSelection *selection;
	gboolean status, result;
	GtkTreeIter iter;
	ShareFile *sf;

	sf = (ShareFile *) data;
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(sf->share_view));
	if (!gtk_tree_model_get_iter_first(sf->share_model, &iter))
		return;
	do {
 mark:		status =
		    gtk_tree_selection_iter_is_selected(selection,
							&iter);
		if (status) {
			result =
			    gtk_list_store_remove(GTK_LIST_STORE
						  (sf->share_model), &iter);
			if (result)
				goto mark;
			break;
		}
	} while (gtk_tree_model_iter_next(sf->share_model, &iter));
}

void ShareFile::ClickOk(gpointer data)
{
	ClickApply(data);
	gtk_widget_destroy(share);
}

void ShareFile::ClickApply(gpointer data)
{
	extern SendFile sfl;
	uint32_t filesize, fileattr;
	gchar *pathname;
	GtkTreeIter iter;
	ShareFile *sf;
	FileInfo *file;

	pthread_mutex_lock(&sfl.mutex);
	g_slist_foreach(sfl.pblist, remove_each_info,
			GINT_TO_POINTER(FILEINFO));
	g_slist_free(sfl.pblist);
	sfl.pblist = NULL;
	pthread_mutex_unlock(&sfl.mutex);
	sfl.dirty = true;

	sf = (ShareFile *) data;
	if (!gtk_tree_model_get_iter_first(sf->share_model, &iter))
		return;
	sfl.pbn = 0;
	pthread_mutex_lock(&sfl.mutex);
	do {
		gtk_tree_model_get(sf->share_model, &iter, 1, &pathname,
				   4, &filesize, 5, &fileattr, -1);
		file = new FileInfo(sfl.pbn, pathname, filesize, fileattr);
		sfl.pblist = g_slist_append(sfl.pblist, file);
		sfl.pbn++;
	} while (gtk_tree_model_iter_next(sf->share_model, &iter));
	pthread_mutex_unlock(&sfl.mutex);
}

void ShareFile::ShareDestroy(gpointer data)
{
	delete(ShareFile *) data;
	share = NULL;
}

void ShareFile::DragDataReceived(gpointer data, GdkDragContext * context,
				  gint x, gint y, GtkSelectionData * select,
				  guint info, guint time)
{
	const char *prl = "file://";
	char *tmp, *file;
	ShareFile *sf;
	GSList *list;

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
	sf = (ShareFile *) data;
	sf->AddSharedFiles(list);
	g_slist_foreach(list, remove_each_info, GINT_TO_POINTER(UNKNOWN));
	g_slist_free(list);
	gtk_drag_finish(context, TRUE, FALSE, time);
}
