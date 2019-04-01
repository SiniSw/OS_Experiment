#include<gtk/gtk.h>
#include<time.h>
#include<glib.h>
gboolean settime(gpointer data)
{
    time_t times;
    struct tm *p_time;
    time(&times);
    p_time = localtime(&times);

    gchar *text_data = g_strdup_printf(\
        "<span size='xx-large'>%04d-%02d-%02d</span>",\
        (1900+p_time->tm_year),(1+p_time->tm_mon),(p_time->tm_mday));

    gchar *text_time = g_strdup_printf(\
    "<span size='x-large'>%02d:%02d:%02d</span>",\
    (p_time->tm_hour), (p_time->tm_min), (p_time->tm_sec));

    gchar *text_markup = g_strdup_printf("\n%s\n\n%s\n", text_time, text_data);

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

    gtk_window_set_title (GTK_WINDOW(window), "time");
    gtk_window_set_default_size(GTK_WINDOW(window),200,100);
    g_signal_connect (G_OBJECT(window), \
    "delete_event", G_CALLBACK(gtk_main_quit), NULL);

    gtk_container_add (GTK_CONTAINER(window), label);

    gint s = g_timeout_add (1000, settime, (void *)label);

    gtk_widget_show_all (window);

    gtk_main();

    return 0;
}
