#include <iostream>
#include <vte/vte.h>
#include <gtk/gtk.h>

using namespace std;

const string TERM_NAME = "LoveTerm";

int main(int argc, char **argv) {

    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    GtkWidget *vte = vte_terminal_new();
    VteTerminal *terminal = VTE_TERMINAL(vte);

    char *default_shell[2];
    default_shell[0] = vte_get_user_shell();

    vte_terminal_spawn_async(
            terminal,
            VTE_PTY_DEFAULT,
            NULL,
            default_shell,
            NULL,
            G_SPAWN_DEFAULT,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL
            );

    return 0;
}