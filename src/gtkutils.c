#include <gtk/gtk.h>
#include "gtkutils.h"
#include "account.h"
#include "util.h"
/**
 * We copied the following two functions from pidgin,
 * hoping pidgin wouldn't angry,for we follow GPL...
 */
gboolean 
pixbuf_is_opaque(GdkPixbuf *pixbuf) {
	int height, rowstride, i;
	unsigned char *pixels;
	unsigned char *row;

	if (!gdk_pixbuf_get_has_alpha(pixbuf))
		return TRUE;

	height = gdk_pixbuf_get_height (pixbuf);
	rowstride = gdk_pixbuf_get_rowstride (pixbuf);
	pixels = gdk_pixbuf_get_pixels (pixbuf);

	row = pixels;
	for (i = 3; i < rowstride; i+=4) {
		if (row[i] < 0xfe)
			return FALSE;
	}

	for (i = 1; i < height - 1; i++) {
		row = pixels + (i * rowstride);
		if (row[3] < 0xfe || row[rowstride - 1] < 0xfe) {
			return FALSE;
	    }
	}

	row = pixels + ((height - 1) * rowstride);
	for (i = 3; i < rowstride; i += 4) {
		if (row[i] < 0xfe)
			return FALSE;
	}

	return TRUE;
}

void
pixbuf_make_round(GdkPixbuf *pixbuf) {
	int width, height, rowstride;
	guchar *pixels;
	if (!gdk_pixbuf_get_has_alpha(pixbuf))
		return;
	width = gdk_pixbuf_get_width(pixbuf);
	height = gdk_pixbuf_get_height(pixbuf);
	rowstride = gdk_pixbuf_get_rowstride(pixbuf);
	pixels = gdk_pixbuf_get_pixels(pixbuf);

	if (width < 6 || height < 6)
		return;
	/* Top left */
	pixels[3] = 0;
	pixels[7] = 0x80;
	pixels[11] = 0xC0;
	pixels[rowstride + 3] = 0x80;
	pixels[rowstride * 2 + 3] = 0xC0;

	/* Top right */
	pixels[width * 4 - 1] = 0;
	pixels[width * 4 - 5] = 0x80;
	pixels[width * 4 - 9] = 0xC0;
	pixels[rowstride + (width * 4) - 1] = 0x80;
	pixels[(2 * rowstride) + (width * 4) - 1] = 0xC0;

	/* Bottom left */
	pixels[(height - 1) * rowstride + 3] = 0;
	pixels[(height - 1) * rowstride + 7] = 0x80;
	pixels[(height - 1) * rowstride + 11] = 0xC0;
	pixels[(height - 2) * rowstride + 3] = 0x80;
	pixels[(height - 3) * rowstride + 3] = 0xC0;

	/* Bottom right */
	pixels[height * rowstride - 1] = 0;
	pixels[(height - 1) * rowstride - 1] = 0x80;
	pixels[(height - 2) * rowstride - 1] = 0xC0;
	pixels[height * rowstride - 5] = 0x80;
	pixels[height * rowstride - 9] = 0xC0;
}

GdkPixbuf*
create_pixbuf(const guchar *pixbuf_data, gint pixbuf_len)
{
	GdkPixbufLoader *loader;
	GdkPixbuf *pixbuf;

	g_return_val_if_fail(pixbuf_data != NULL, NULL);
	g_return_val_if_fail(pixbuf_len != 0, NULL);

	loader = gdk_pixbuf_loader_new();
	gdk_pixbuf_loader_write(loader, pixbuf_data, pixbuf_len, NULL);
	gdk_pixbuf_loader_close(loader, NULL);

	pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);

	return pixbuf;
}

GdkPixbuf*
create_pixbuf_at_size(const guchar *pixbuf_data, gint pixbuf_len,
		gint scale_width, gint scale_height)
{
	GdkPixbuf *pixbuf;
	GdkPixbuf *res;

	g_return_val_if_fail(pixbuf_data != NULL, NULL);
	g_return_val_if_fail(pixbuf_len != 0, NULL);

	pixbuf = create_pixbuf(pixbuf_data, pixbuf_len);

	res = gdk_pixbuf_scale_simple(pixbuf,
			scale_width, scale_height, GDK_INTERP_BILINEAR);

	g_object_unref(pixbuf);

	return res;
}

GdkPixbuf*
create_round_pixbuf(const guchar *pixbuf_data, gint pixbuf_len,
		gint scale_size)
{
	GdkPixbufLoader *loader;
	GdkPixbuf *pixbuf;
	GdkPixbuf *newpixbuf;
	gint orig_width;
	gint orig_height;

	loader = gdk_pixbuf_loader_new();
	gdk_pixbuf_loader_write(loader, pixbuf_data, pixbuf_len, NULL);
	gdk_pixbuf_loader_close(loader, NULL);

	pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);
	if (!pixbuf) {
		im_debug_error("blist", "get pixbuf from loader");
		return NULL;
	}

	orig_width = gdk_pixbuf_get_width(pixbuf);
	orig_height = gdk_pixbuf_get_height(pixbuf);

	newpixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8,	32, 32);
	gdk_pixbuf_fill(newpixbuf, 0x00000000);

	gdk_pixbuf_scale(pixbuf, newpixbuf, 0, 0, scale_size, scale_size, 0, 0,
			(double)scale_size/(double)orig_width,
			(double)scale_size/(double)orig_height, GDK_INTERP_BILINEAR);

	g_object_unref(pixbuf);

	if (pixbuf_is_opaque(newpixbuf)) {
		pixbuf_make_round(newpixbuf);
	}

	pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, scale_size, scale_size);
	gdk_pixbuf_fill(pixbuf, 0x00000000);
	gdk_pixbuf_copy_area(newpixbuf, 0, 0, scale_size, scale_size,
			pixbuf, 0, 0);

	g_object_unref(newpixbuf);

	return pixbuf;
}

GdkPixbuf*
create_presence_pixbuf(gint presence, gint scale_size)
{
	gchar *name;

	switch (presence) {

		case IM_STATE_OFFLINE:
			name = DATA_DIR"/offline.png";
			break;
		case IM_STATE_INVISIBLE:
			name = DATA_DIR"/invisible.png";
			break;
		case IM_STATE_BUSY:
			name = DATA_DIR"/busy.png";
			break;
		case IM_STATE_AWAY:
			name = DATA_DIR"/away.png";
			break;
		case IM_STATE_ONLINE:
			name = DATA_DIR"/available.png";
			break;
		default:
			name = DATA_DIR"/offline.png";
			break;
	};

	return gdk_pixbuf_new_from_file_at_size(name,
			scale_size, scale_size, NULL);
}