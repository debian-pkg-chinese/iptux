//
// C++ Implementation: my_chooser
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "my_chooser.h"
#include "udt.h"

my_chooser::my_chooser()
{
}

my_chooser::~my_chooser()
{
}

GtkWidget *my_chooser::create_chooser(const gchar * title, GtkWidget * parent)
{
	GtkWidget *chooser;
	GtkWidget *preview;

	chooser = gtk_file_chooser_dialog_new(title,
					      GTK_WINDOW(parent),
					      GTK_FILE_CHOOSER_ACTION_OPEN,
					      GTK_STOCK_OPEN, GTK_RESPONSE_OK,
					      GTK_STOCK_CANCEL,
					      GTK_RESPONSE_CANCEL, NULL);
	gtk_dialog_set_default_response(GTK_DIALOG(chooser), GTK_RESPONSE_OK);

	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(chooser), TRUE);
	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(chooser), FALSE);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER
						       (chooser), TRUE);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(chooser),
					    getenv("HOME"));

	preview = gtk_image_new();
	gtk_widget_set_size_request(preview, MAX_PREVIEWSIZE, MAX_PREVIEWSIZE);
	gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(chooser), preview);
	gtk_file_chooser_set_preview_widget_active(GTK_FILE_CHOOSER(chooser),
						   FALSE);
	g_signal_connect(chooser, "update-preview", G_CALLBACK(UpdatePreview),
			 preview);

	return chooser;
}

gchar *my_chooser::run_chooser(GtkWidget * chooser)
{
	int result;
	gchar *filename;

	result = gtk_dialog_run(GTK_DIALOG(chooser));
	if (result == GTK_RESPONSE_OK)
		filename =
		    gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser));
	else
		filename = NULL;
	gtk_widget_destroy(chooser);

	return filename;
}

void my_chooser::UpdatePreview(GtkFileChooser * chooser, GtkWidget * preview)
{
	gchar *filename;
	GdkPixbuf *pixbuf, *tmp;
	gdouble scale_x, scale_y, scale;
	gint width, height;

	filename = gtk_file_chooser_get_preview_filename(chooser);
	if (!filename) {
		gtk_file_chooser_set_preview_widget_active(chooser, FALSE);
		return;
	}

	pixbuf = gdk_pixbuf_new_from_file(filename, NULL);
	g_free(filename);
	if (!pixbuf) {
		gtk_file_chooser_set_preview_widget_active(chooser, FALSE);
		return;
	}

	width = gdk_pixbuf_get_width(pixbuf);
	height = gdk_pixbuf_get_height(pixbuf);
	if (width > MAX_PREVIEWSIZE || height > MAX_PREVIEWSIZE) {
		scale = (scale_x = (gdouble) MAX_PREVIEWSIZE / width) <
		    (scale_y =
		     (gdouble) MAX_PREVIEWSIZE / height) ? scale_x : scale_y;
		width = (gint) (width * scale), height =
		    (gint) (height * scale);
		tmp = pixbuf;
		pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8,
					width, height);
		gdk_pixbuf_scale(tmp, pixbuf, 0, 0, width, height, 0.0, 0.0,
				 scale, scale, GDK_INTERP_BILINEAR);
		g_object_unref(tmp);
	}

	gtk_image_set_from_pixbuf(GTK_IMAGE(preview), pixbuf);
	g_object_unref(pixbuf);
	gtk_file_chooser_set_preview_widget_active(chooser, TRUE);
}
