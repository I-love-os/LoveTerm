#include <iostream>
#include <vte/vte.h>
#include <gtk/gtk.h>

using namespace std;

const gchar TERM_NAME[9] = "LoveTerm";

gboolean key_is_pressed(GtkWidget *widget, GdkEvent *event, gpointer data) {
     VteTerminal *term = VTE_TERMINAL(widget);
     GtkWidget *win = (GtkWidget *)data;

     if (((GdkEventKey *)event)->state & GDK_CONTROL_MASK) {
         switch (((GdkEventKey *)event)->keyval) {
             case GDK_KEY_C:
                 vte_terminal_copy_clipboard_format(term, VTE_FORMAT_TEXT);
                 return TRUE;
             case GDK_KEY_V:
                 vte_terminal_paste_clipboard(term);
                 return TRUE;
             default:
                 return FALSE;
         }
    }
}
int main(int argc, char **argv) {

    gtk_init(&argc, &argv);

    //TODO: To struct
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    GtkWidget *vte = vte_terminal_new();
    VteTerminal *terminal = VTE_TERMINAL(vte);
    gtk_window_set_title(GTK_WINDOW(window), TERM_NAME);

    static char *default_shell[2];
    default_shell[0] = vte_get_user_shell();

    vte_terminal_spawn_async(
            terminal,
            VTE_PTY_DEFAULT,
            NULL,
            default_shell,
            NULL,
            G_SPAWN_SEARCH_PATH,
            NULL,
            NULL,
            NULL,
            60,
            NULL,
            NULL,
            NULL
            );

    g_signal_connect(window, "delete-event", gtk_main_quit, NULL);
    g_signal_connect(terminal, "child-exited", gtk_main_quit, NULL);
    g_signal_connect(terminal, "key-press_event", G_CALLBACK(key_is_pressed), vte);

    gtk_container_add(GTK_CONTAINER(window), vte);
    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}