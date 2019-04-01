#include<gtk/gtk.h>
#include<time.h>
#include<glib.h>

int a=0;
gboolean settime(gpointer data)
{
    a++;
    a=a%10;
    gchar *text_markup = g_strdup_printf("\n%d\n", a);
    gtk_label_set_markup(GTK_LABEL(data), text_markup);

    return TRUE;
}

int main(int argc, char *argv[])
{
    gtk_init (&argc, &argv);
    GtkWidget *window;
    GtkWidget *label;

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    label = gtk_label_new (NULL);

    gtk_window_set_title (GTK_WINDOW(window), "num");
    gtk_window_set_default_size(GTK_WINDOW(window),200,100);
    g_signal_connect (G_OBJECT(window), \
    "delete_event", G_CALLBACK(gtk_main_quit), NULL);

    gtk_container_add (GTK_CONTAINER(window), label);

    gint s = g_timeout_add (1000, settime, (void *)label);

    gtk_widget_show_all (window);

    gtk_main();

    return 0;
}
