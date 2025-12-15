/*
 * Shortest Path Applications - Main Entry Point
 * A visualization tool for Kirchhoff's Circuit Laws and Traveling Salesman Problem
 * 
 * Required libraries:
 * - GTK+ 3.0 (sudo apt-get install libgtk-3-dev)
 * - Cairo (included with GTK+)
 * - Math library (-lm)
 * 
 * Compile with:
 * gcc main.c kirchhoff.c tsp.c tsp_algorithms.c -o shortest_path `pkg-config --cflags --libs gtk+-3.0` -lm
 */

#include <gtk/gtk.h>
#include <stdlib.h>
#include "kirchhoff.h"
#include "tsp.h"

typedef struct {
    GtkWidget *window;
    GtkWidget *main_box;
} AppData;

// Forward declarations
static void launch_kirchhoff(GtkWidget *widget, gpointer data);
static void launch_tsp(GtkWidget *widget, gpointer data);
static void create_main_menu(AppData *app);

// Main menu creation
static void create_main_menu(AppData *app) {
    // Clear existing children
    GList *children = gtk_container_get_children(GTK_CONTAINER(app->main_box));
    for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);
    
    // Set window background and Global Button Styles
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        "window { background-color: #1c1c1c; }"
        ".title-label { color: #e0e0e0; font-size: 32px; font-weight: bold; }"
        ".subtitle-label { color: #a0a0a0; font-size: 14px; }"
        ".section-frame { background-color: #2b2b2b; border-radius: 10px; padding: 30px; }"
        ".section-title { color: #ffffff; font-size: 22px; font-weight: bold; }"
        ".section-text { color: #c0c0c0; font-size: 14px; }"
        ".kirchhoff-title { color: #e57373; }"
        ".tsp-title { color: #81c784; }"
        
        /* GLOBAL BUTTON STYLING - Applies to ALL buttons */
        "button, button:hover, button:active, button:checked, button:focus, button:backdrop {"
        "   background-color: #ffffff;"  /* CONSTANT WHITE BACKGROUND */
        "   color: #000000;"             /* CONSTANT BLACK TEXT */
        "   font-weight: bold;"          /* BOLD TEXT */
        "   border-radius: 5px;"
        "   padding: 10px 20px;"
        
        /* These remove the default gray/blue theme effects */
        "   background-image: none;"
        "   box-shadow: none;"
        "   text-shadow: none;"
        "   border-color: #888888;"
        "}"
        
        ".footer-label { color: #707070; font-size: 10px; }",
        -1, NULL);
    
    GdkScreen *screen = gdk_screen_get_default();
    gtk_style_context_add_provider_for_screen(screen,
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    
    // Title section
    GtkWidget *title_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(app->main_box), title_box, FALSE, FALSE, 20);
    
    GtkWidget *title_label = gtk_label_new("Shortest Path Visualizer");
    gtk_style_context_add_class(gtk_widget_get_style_context(title_label), "title-label");
    gtk_box_pack_start(GTK_BOX(title_box), title_label, FALSE, FALSE, 0);
    
    GtkWidget *subtitle_label = gtk_label_new("An Interactive Toolkit for Kirchhoff's Laws & TSP");
    gtk_style_context_add_class(gtk_widget_get_style_context(subtitle_label), "subtitle-label");
    gtk_box_pack_start(GTK_BOX(title_box), subtitle_label, FALSE, FALSE, 0);
    
    // Content section
    GtkWidget *content_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 40);
    gtk_box_pack_start(GTK_BOX(app->main_box), content_box, TRUE, TRUE, 50);
    
    // Kirchhoff's Laws Section
    GtkWidget *kirchhoff_frame = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_style_context_add_class(gtk_widget_get_style_context(kirchhoff_frame), "section-frame");
    gtk_box_pack_start(GTK_BOX(content_box), kirchhoff_frame, TRUE, TRUE, 20);
    
    GtkWidget *kirchhoff_title = gtk_label_new("Kirchhoff's Circuit Laws");
    gtk_style_context_add_class(gtk_widget_get_style_context(kirchhoff_title), "section-title");
    gtk_style_context_add_class(gtk_widget_get_style_context(kirchhoff_title), "kirchhoff-title");
    gtk_box_pack_start(GTK_BOX(kirchhoff_frame), kirchhoff_title, FALSE, FALSE, 0);
    
    GtkWidget *kirchhoff_text = gtk_label_new(
        "Visualize and solve electrical circuits.\n\n"
        "• Kirchhoff's Current Law (KCL)\n"
        "• Kirchhoff's Voltage Law (KVL)\n\n"
        "Create custom circuits, compute currents and voltages.");
    gtk_label_set_justify(GTK_LABEL(kirchhoff_text), GTK_JUSTIFY_LEFT);
    gtk_style_context_add_class(gtk_widget_get_style_context(kirchhoff_text), "section-text");
    gtk_box_pack_start(GTK_BOX(kirchhoff_frame), kirchhoff_text, FALSE, FALSE, 20);
    
    GtkWidget *kirchhoff_button = gtk_button_new_with_label("Launch Kirchhoff Simulator");
    /* No specific class needed anymore as global button style covers it, 
       but keeping class add just in case you add specific logic later */
    gtk_style_context_add_class(gtk_widget_get_style_context(kirchhoff_button), "kirchhoff-button");
    g_signal_connect(kirchhoff_button, "clicked", G_CALLBACK(launch_kirchhoff), app);
    gtk_box_pack_start(GTK_BOX(kirchhoff_frame), kirchhoff_button, FALSE, FALSE, 30);
    
    // TSP Section
    GtkWidget *tsp_frame = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_style_context_add_class(gtk_widget_get_style_context(tsp_frame), "section-frame");
    gtk_box_pack_start(GTK_BOX(content_box), tsp_frame, TRUE, TRUE, 20);
    
    GtkWidget *tsp_title = gtk_label_new("Traveling Salesman Problem");
    gtk_style_context_add_class(gtk_widget_get_style_context(tsp_title), "section-title");
    gtk_style_context_add_class(gtk_widget_get_style_context(tsp_title), "tsp-title");
    gtk_box_pack_start(GTK_BOX(tsp_frame), tsp_title, FALSE, FALSE, 0);
    
    GtkWidget *tsp_text = gtk_label_new(
        "Solve TSP using multiple algorithms.\n\n"
        "• Nearest Neighbor\n"
        "• Genetic Algorithm\n"
        "• Dynamic Programming\n\n"
        "Design custom city layouts and find the optimal route.");
    gtk_label_set_justify(GTK_LABEL(tsp_text), GTK_JUSTIFY_LEFT);
    gtk_style_context_add_class(gtk_widget_get_style_context(tsp_text), "section-text");
    gtk_box_pack_start(GTK_BOX(tsp_frame), tsp_text, FALSE, FALSE, 20);
    
    GtkWidget *tsp_button = gtk_button_new_with_label("Launch TSP Simulator");
    gtk_style_context_add_class(gtk_widget_get_style_context(tsp_button), "tsp-button");
    g_signal_connect(tsp_button, "clicked", G_CALLBACK(launch_tsp), app);
    gtk_box_pack_start(GTK_BOX(tsp_frame), tsp_button, FALSE, FALSE, 30);
    
    // Footer
    GtkWidget *footer = gtk_label_new("Fundamentals of Data Structures - Course Project");
    gtk_style_context_add_class(gtk_widget_get_style_context(footer), "footer-label");
    gtk_box_pack_end(GTK_BOX(app->main_box), footer, FALSE, FALSE, 10);
    
    gtk_widget_show_all(app->window);
}

static void launch_kirchhoff(GtkWidget *widget, gpointer data) {
    AppData *app = (AppData *)data;
    
    // Clear main box
    GList *children = gtk_container_get_children(GTK_CONTAINER(app->main_box));
    for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);
    
    // Create back button frame
    GtkWidget *back_frame = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(app->main_box), back_frame, FALSE, FALSE, 0);
    
    GtkWidget *back_button = gtk_button_new_with_label("← Main Menu");
    g_signal_connect_swapped(back_button, "clicked", G_CALLBACK(create_main_menu), app);
    gtk_box_pack_start(GTK_BOX(back_frame), back_button, FALSE, FALSE, 10);
    
    GtkWidget *title = gtk_label_new("Circuit Simulator");
    gtk_box_pack_start(GTK_BOX(back_frame), title, FALSE, FALSE, 10);
    
    // Launch Kirchhoff simulator
    kirchhoff_init(app->main_box);
    
    gtk_widget_show_all(app->window);
}

static void launch_tsp(GtkWidget *widget, gpointer data) {
    AppData *app = (AppData *)data;
    
    // Clear main box
    GList *children = gtk_container_get_children(GTK_CONTAINER(app->main_box));
    for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);
    
    // Create back button frame
    GtkWidget *back_frame = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(app->main_box), back_frame, FALSE, FALSE, 0);
    
    GtkWidget *back_button = gtk_button_new_with_label("← Main Menu");
    g_signal_connect_swapped(back_button, "clicked", G_CALLBACK(create_main_menu), app);
    gtk_box_pack_start(GTK_BOX(back_frame), back_button, FALSE, FALSE, 10);
    
    GtkWidget *title = gtk_label_new("TSP Simulator");
    gtk_box_pack_start(GTK_BOX(back_frame), title, FALSE, FALSE, 10);
    
    // Launch TSP simulator
    tsp_init(app->main_box);
    
    gtk_widget_show_all(app->window);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    
    AppData app;
    app.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(app.window), 
        "Shortest Path Applications - Kirchhoff's Laws & TSP");
    gtk_window_set_default_size(GTK_WINDOW(app.window), 1300, 750);
    gtk_window_set_position(GTK_WINDOW(app.window), GTK_WIN_POS_CENTER);
    
    g_signal_connect(app.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    app.main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(app.window), app.main_box);
    
    create_main_menu(&app);
    
    gtk_main();
    
    return 0;
}