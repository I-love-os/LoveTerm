#include <iostream>
#include <vte/vte.h>
#include <gtk/gtk.h>
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#include <memory>

using namespace std;

const gchar TERM_NAME[9] = "LoveTerm";

struct Terminal {
    int t_index;
    GtkWidget *vte;
    VteTerminal *terminal;
    char *shell[2];
    GtkWidget *container;
    int tag;
};

struct Terms {
    struct Terminal *t;
    int size = 0;
};

static gboolean key_is_pressed(GtkWidget *widget, GdkEvent *event);
static gboolean key_is_released(GtkWidget *widget, GdkEvent *event);
static void term_new(struct Terminal *term);
static void on_child_exited(VteTerminal *, int status);
static void delete_current_tab();
static bool terms_remove(int terms_i);
static void tabs_update();
static gboolean handle_urls(VteTerminal *vte, GdkEvent *event);
static void setup();

static GtkWidget *window;
static GtkWidget *tabs;
static GtkWidget *container;
static struct Terms terms;
static bool control_pressed = false;

static void term_new(struct Terminal *term) {
    term->t_index = terms.size;
    term->vte = vte_terminal_new();
    term->terminal = VTE_TERMINAL(term->vte);
    term->shell[0] = vte_get_user_shell();
    term->container = gtk_scrolled_window_new(nullptr, nullptr);
    GtkWidget *tab_title = gtk_label_new(to_string(term->t_index + 1).c_str());
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(term->container), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_notebook_append_page(GTK_NOTEBOOK(tabs), term->container, tab_title);

    vte_terminal_set_scroll_on_output(term->terminal, FALSE);
    vte_terminal_set_rewrap_on_resize(term->terminal, TRUE);
    vte_terminal_set_scroll_on_keystroke(term->terminal, TRUE);
    vte_terminal_set_audible_bell(term->terminal, TRUE);
    vte_terminal_set_allow_hyperlink(term->terminal, TRUE);

    auto *reg = vte_regex_new_for_match("http[A-Za-z0-9-\\--:]*", -1, PCRE2_MULTILINE | PCRE2_NOTEMPTY, nullptr);
    term->tag = vte_terminal_match_add_regex(term->terminal, reg, 0);
    vte_terminal_match_set_cursor_type(term->terminal, term->tag, GDK_HAND2);

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

    g_signal_connect(term->terminal, "child-exited", G_CALLBACK(on_child_exited), nullptr);
    g_signal_connect(term->terminal, "key-press-event", G_CALLBACK(key_is_pressed), window);
    g_signal_connect(term->terminal, "key-release-event", G_CALLBACK(key_is_released), window);
    g_signal_connect(term->terminal, "button-press-event", G_CALLBACK(handle_urls),  window);

    gtk_container_add(GTK_CONTAINER(term->container), term->vte);
    terms.t[term->t_index] = *term;
    terms.size++;
}

static gboolean key_is_released(GtkWidget *widget, GdkEvent *event) {
    if (((GdkEventKey *)event)->keyval == GDK_KEY_Control_L) {
        control_pressed = false;
        return TRUE;
    }

    return FALSE;
}

static gboolean key_is_pressed(GtkWidget *widget, GdkEvent *event) {
     VteTerminal *term = VTE_TERMINAL(widget);

     if (((GdkEventKey *)event)->keyval == GDK_KEY_Control_L) {
         control_pressed = true;
     }

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
                 gtk_notebook_set_current_page(GTK_NOTEBOOK(tabs), terms.size - 1);
                 return TRUE;
             }
             case GDK_KEY_X: {
                 delete_current_tab();
                 return TRUE;
             }
             case GDK_KEY_Left: {
                 gtk_notebook_prev_page(GTK_NOTEBOOK(tabs));
                 return TRUE;
             }
             case GDK_KEY_Right: {
                 gtk_notebook_next_page(GTK_NOTEBOOK(tabs));
                 return TRUE;
             }
         }
    }
    return FALSE;
}

static void delete_current_tab() {
    gint page;
    page = gtk_notebook_get_current_page(GTK_NOTEBOOK(tabs));

    terms_remove(page);
    gtk_notebook_remove_page(GTK_NOTEBOOK(tabs),page);

    tabs_update();
}

static void on_child_exited(VteTerminal *, int status) {
    if (terms.size <= 0) {
        gtk_main_quit();
    } else if (terms.size <= 1 && status == 0) {
        gtk_main_quit();
    } else if (status == 0) {
        delete_current_tab();
    }
}


static void setup() {
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    tabs = gtk_notebook_new();
    container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    gtk_widget_set_size_request(window, 1000, 510);

    gtk_container_add(GTK_CONTAINER(window), container);
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(tabs), GTK_POS_LEFT);
    gtk_box_pack_start(GTK_BOX(container), tabs, TRUE, TRUE, 0);
    gtk_window_set_title(GTK_WINDOW(window), TERM_NAME);
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(tabs), TRUE);

    struct Terminal term = {0};
    term_new(&term);
}

static bool terms_remove(int terms_i) {
    for (int i = 0; i < terms.size; i++) {
        if (terms.t[i].t_index == terms_i) {
            for (; i < terms.size - 1; i++) {
                terms.t[i] = terms.t[i + 1];
                terms.t[i].t_index = i;
            }

            terms.t[terms.size - 1] = {0};
            terms.size--;

            return true;
        }
    }
    return false;
}

static void tabs_update() {
    for (int i = 0; i < gtk_notebook_get_n_pages(GTK_NOTEBOOK(tabs)); i++) {
        auto page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(tabs), i);
        gtk_notebook_set_tab_label_text(GTK_NOTEBOOK(tabs), page, to_string(i + 1).c_str());
    }
}

static gboolean handle_urls(VteTerminal *vte, GdkEvent *event) {
    if (event->type == GDK_BUTTON_PRESS  &&  event->button.button == 1 && control_pressed) {
        auto url = vte_terminal_hyperlink_check_event(vte, event);

        if (url == NULL) {
            g_free(url);

            int tab;
            url = vte_terminal_match_check_event(vte, event, &tab);
        }

        g_print(url);

        return TRUE;
    }

    return FALSE;
}

int main(int argc, char **argv) {
    gtk_init(&argc, &argv);
    terms = {nullptr};
    terms.t = new struct Terminal[1000];

    setup();

    g_signal_connect(window, "delete-event", gtk_main_quit, nullptr);
    //TODO: app gtk
    gtk_widget_show_all(window);

    g_print("Terminal is running\n");

    gtk_main();

    delete[] terms.t;

    return 0;
}