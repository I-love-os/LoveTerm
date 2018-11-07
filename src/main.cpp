#include <iostream>
#include <vte/vte.h>
#include <gtk/gtk.h>

using namespace std;

const gchar TERM_NAME[9] = "LoveTerm";

struct Terminal {
    int id;
    GtkWidget *vte;
    VteTerminal *terminal;
    char *shell[2];
};

static gboolean key_is_pressed(GtkWidget *widget, GdkEvent *event);
static void term_new(struct Terminal *term);
static void setup();

static GtkWidget *window;
static GtkWidget *tabs;
static GtkWidget *container;
static int id = 0;
static struct Terminal *terms;

static void term_new(struct Terminal *term) {
    term->id = id++;
    term->vte = vte_terminal_new();
    term->terminal = VTE_TERMINAL(term->vte);
    term->shell[0] = vte_get_user_shell();
    GtkWidget *tab_title = gtk_label_new(to_string(term->id + 1).c_str());

    GtkWidget *scroll_container = gtk_scrolled_window_new(nullptr, nullptr);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_container), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_notebook_append_page(GTK_NOTEBOOK(tabs), scroll_container, tab_title);

    vte_terminal_set_scroll_on_output(term->terminal, FALSE);
    vte_terminal_set_rewrap_on_resize(term->terminal, TRUE);
    vte_terminal_set_scroll_on_keystroke(term->terminal, TRUE);

    vte_terminal_spawn_async(
            term->terminal,
            VTE_PTY_DEFAULT,
            nullptr,
            term->shell,
            nullptr,
            G_SPAWN_SEARCH_PATH,
            nullptr,
            nullptr,
            nullptr,
            60,
            nullptr,
            nullptr,
            nullptr
    );

    g_signal_connect(term->terminal, "child-exited", gtk_main_quit, nullptr);
    g_signal_connect(term->terminal, "key-press-event", G_CALLBACK(key_is_pressed), window);

    gtk_container_add(GTK_CONTAINER(scroll_container), term->vte);

    terms[term->id] = *term;
}

static gboolean key_is_pressed(GtkWidget *widget, GdkEvent *event) {
     VteTerminal *term = VTE_TERMINAL(widget);

     if (((GdkEventKey *)event)->state & GDK_CONTROL_MASK) {
         switch (((GdkEventKey *)event)->keyval) {
             case GDK_KEY_C: {
                 vte_terminal_copy_clipboard_format(term, VTE_FORMAT_TEXT);
                 return TRUE;
             }
             case GDK_KEY_V: {
                 vte_terminal_paste_clipboard(term);
                 return TRUE;
             }
             case GDK_KEY_T: {
                 struct Terminal term2 = {0};
                 term_new(&term2);
                 gtk_widget_show_all(window);
                 gtk_notebook_set_current_page(GTK_NOTEBOOK(tabs), id - 1);
                 return TRUE;
             }
             case GDK_KEY_X: {
//                 gtk_notebook_remove_page(GTK_NOTEBOOK(tabs), gtk_notebook_get_current_page(GTK_NOTEBOOK(tabs)));
//                 id--;
//                 terms[gtk_notebook_get_current_page(GTK_NOTEBOOK(tabs))] = {0};

                 gtk_widget_show_all(window);
                 return TRUE;
             }
         }
    }
    return FALSE;
}

static void setup() {
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    tabs = gtk_notebook_new();
    container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    gtk_widget_set_size_request(window, 900, 420);

    gtk_container_add(GTK_CONTAINER(window), container);
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(tabs), GTK_POS_LEFT);
    gtk_box_pack_start(GTK_BOX(container), tabs, TRUE, TRUE, 0);
    gtk_window_set_title(GTK_WINDOW(window), TERM_NAME);
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(tabs), TRUE);

    struct Terminal term = {0};
    term_new(&term);
}

int main(int argc, char **argv) {
    gtk_init(&argc, &argv);
    terms = new struct Terminal[1000];

    setup();

    g_signal_connect(window, "delete-event", gtk_main_quit, nullptr);

    gtk_widget_show_all(window);
    gtk_main();

    delete[] terms;
    return 0;
}