#include <glib.h>
#include <gtk/gtk.h>
#include "module.h"
#include "blist.h"

gint 
main(gint argc, gchar **argv)
{
	gtk_init(&argc, &argv);

	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_usize(window, 250, 500);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

	im_module_init();

	IMBlist *blist = im_blist_create();
	IMGroup *group;
	IMContact *contact;

	im_blist_init(blist);
	
	group = im_blist_add_group(blist, "ocean university");
	contact = im_blist_add_contact(blist, group, "125559923", "levin108");
	contact = im_blist_add_contact(blist, group, "125559923", "soulpower");
	contact = im_blist_add_contact(blist, group, "125559923", "ricky");
	group = im_blist_add_group(blist, "beijing university");

	gtk_container_add(GTK_CONTAINER(window), blist->treeview);

	gtk_widget_show_all(window);

	gtk_main();
	return 0;
}