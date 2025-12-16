/*
 * Traveling Salesman Problem Simulator
 * Implements multiple algorithms: Nearest Neighbor, Genetic Algorithm, Dynamic Programming
 */

#include "tsp.h"
#include "tsp_algorithms.h"
#include <cairo.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

static TSPData *global_tsp_data = NULL;

// Forward declarations
static gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer data);
static gboolean on_button_press(GtkWidget *widget, GdkEventButton *event, gpointer data);
static void redraw_map(TSPData *data);
static void draw_solution(TSPData *data);
static void add_city(TSPData *data, double x, double y);
static void generate_random_cities(GtkWidget *widget, gpointer user_data);
static void clear_all(GtkWidget *widget, gpointer user_data);
static void solve_tsp_callback(GtkWidget *widget, gpointer user_data);
static void mode_changed(GtkWidget *widget, gpointer user_data);
static void auto_connect_toggled(GtkWidget *widget, gpointer user_data);
static void algo_changed(GtkWidget *widget, gpointer user_data);

static double distance_between(City *c1, City *c2) {
    return sqrt((c1->x - c2->x) * (c1->x - c2->x) + 
                (c1->y - c2->y) * (c1->y - c2->y));
}

static void add_city(TSPData *data, double x, double y) {
    if (data->city_count >= MAX_CITIES) return;
    
    int idx = data->city_count;
    data->cities[idx].x = x;
    data->cities[idx].y = y;
    sprintf(data->cities[idx].name, "C%d", idx);
    
    // Auto-connect to existing cities if enabled
    if (data->auto_connect && data->city_count > 0) {
        for (int i = 0; i < data->city_count; i++) {
            double dist = distance_between(&data->cities[idx], &data->cities[i]);
            data->edges[idx][i] = dist;
            data->edges[i][idx] = dist;
            data->edge_exists[idx][i] = 1;
            data->edge_exists[i][idx] = 1;
        }
    }
    
    data->city_count++;
    redraw_map(data);
}

static void redraw_map(TSPData *data) {
    GtkAllocation allocation;
    gtk_widget_get_allocation(data->canvas, &allocation);
    
    // Only redraw if canvas has valid size
    if (allocation.width <= 1 || allocation.height <= 1) {
        return;
    }
    
    // Reset surface
    if (data->surface) {
        cairo_surface_destroy(data->surface);
    }
    
    data->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                                               allocation.width,
                                               allocation.height);
    cairo_t *cr = cairo_create(data->surface);
    
    // 1. Paint clean White background
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);
    
    // 2. Draw edges
    for (int i = 0; i < data->city_count; i++) {
        for (int j = i + 1; j < data->city_count; j++) {
            if (data->edge_exists[i][j]) {
                
                // A. Draw the solid Black line first
                cairo_set_line_width(cr, 2.0);
                cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);  // Pure Black
                cairo_move_to(cr, data->cities[i].x, data->cities[i].y);
                cairo_line_to(cr, data->cities[j].x, data->cities[j].y);
                cairo_stroke(cr);
                
                // B. Draw Weight Text (No background box)
                double mid_x = (data->cities[i].x + data->cities[j].x) / 2;
                double mid_y = (data->cities[i].y + data->cities[j].y) / 2;
                
                char weight_str[20];
                sprintf(weight_str, "%.0f", data->edges[i][j]);
                
                cairo_select_font_face(cr, "Times New Roman", 
                                       CAIRO_FONT_SLANT_NORMAL,
                                       CAIRO_FONT_WEIGHT_NORMAL);
                cairo_set_font_size(cr, 15);
                
                cairo_text_extents_t extents;
                cairo_text_extents(cr, weight_str, &extents);
                
                // Calculate offset to move text away from the line
                double dx = data->cities[j].x - data->cities[i].x;
                double dy = data->cities[j].y - data->cities[i].y;
                double len = sqrt(dx*dx + dy*dy);
                
                if (len > 0) {
                    // Increased offset factor from 18 to 22 for better spacing
                    double offset_x = -dy / len * 22;
                    double offset_y = dx / len * 22;
                    
                    // Draw text in Black
                    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
                    cairo_move_to(cr, 
                                  mid_x + offset_x - extents.width/2, 
                                  mid_y + offset_y + extents.height/2); // Adjustment for baseline
                    cairo_show_text(cr, weight_str);
                }
            }
        }
    }
    
    // 3. Draw cities (Nodes)
    for (int i = 0; i < data->city_count; i++) {
        // [FIX 1] Start a fresh path to prevent "Green Line" connecting previous text to this circle
        cairo_new_path(cr);

        // Green Circle Fill
        cairo_set_source_rgb(cr, 0.506, 0.780, 0.514); // #81c784
        cairo_arc(cr, data->cities[i].x, data->cities[i].y, 18, 0, 2 * M_PI);
        cairo_fill_preserve(cr);
        
        // Dark Green Border
        cairo_set_source_rgb(cr, 0.333, 0.545, 0.184); // #558b2f
        cairo_set_line_width(cr, 2);
        cairo_stroke(cr);
        
        // City Name (White)
        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
        cairo_select_font_face(cr, "Times New Roman",
                              CAIRO_FONT_SLANT_NORMAL,
                              CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 12);
        
        cairo_text_extents_t extents;
        cairo_text_extents(cr, data->cities[i].name, &extents);
        cairo_move_to(cr, 
                      data->cities[i].x - extents.width / 2,
                      data->cities[i].y + extents.height / 2);
        cairo_show_text(cr, data->cities[i].name);

        // [FIX 2] Reset color to BLACK after drawing the node to prevent color bleeding
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    }
    
    cairo_destroy(cr);
    gtk_widget_queue_draw(data->canvas);
}

static void draw_solution(TSPData *data) {
    if (data->solution_length < 2) return;
    
    GtkAllocation allocation;
    gtk_widget_get_allocation(data->canvas, &allocation);
    
    cairo_t *cr = cairo_create(data->surface);
    
    // Draw solution path
    cairo_set_line_width(cr, 2);
    cairo_set_source_rgb(cr, 0.898, 0.451, 0.451); // #e57373
    
    for (int i = 0; i < data->solution_length; i++) {
        City *c1 = data->solution_path[i];
        City *c2 = data->solution_path[(i + 1) % data->solution_length];
        
        cairo_move_to(cr, c1->x, c1->y);
        cairo_line_to(cr, c2->x, c2->y);
        cairo_stroke(cr);
        
        // Draw arrow
        double angle = atan2(c2->y - c1->y, c2->x - c1->x);
        double arrow_x = c2->x - 15 * cos(angle);
        double arrow_y = c2->y - 15 * sin(angle);
        
        cairo_move_to(cr, c2->x, c2->y);
        cairo_line_to(cr, arrow_x - 5 * cos(angle + M_PI/6), 
                          arrow_y - 5 * sin(angle + M_PI/6));
        cairo_move_to(cr, c2->x, c2->y);
        cairo_line_to(cr, arrow_x - 5 * cos(angle - M_PI/6), 
                          arrow_y - 5 * sin(angle - M_PI/6));
        cairo_stroke(cr);
        
        // Draw step number
        double mid_x = (c1->x + c2->x) / 2;
        double mid_y = (c1->y + c2->y) / 2;
        
        char step[10];
        sprintf(step, "%d", i + 1);
        
        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
        cairo_select_font_face(cr, "Times New Roman",
                              CAIRO_FONT_SLANT_NORMAL,
                              CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 10);
        cairo_move_to(cr, mid_x - 5, mid_y + 5);
        cairo_show_text(cr, step);
        cairo_set_source_rgb(cr, 0.898, 0.451, 0.451);
    }
    
    // Highlight start city
    City *start = data->solution_path[0];
    cairo_set_source_rgb(cr, 0.506, 0.780, 0.514); // #81c784
    cairo_set_line_width(cr, 4);
    cairo_arc(cr, start->x, start->y, 20, 0, 2 * M_PI);
    cairo_stroke(cr);
    
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0); // White
    cairo_select_font_face(cr, "Times New Roman",
                           CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 11);
    cairo_move_to(cr, start->x - 20, start->y + 35);
    cairo_show_text(cr, "START");
    
    cairo_destroy(cr);
    gtk_widget_queue_draw(data->canvas);
}

static gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer data) {
    TSPData *tsp_data = (TSPData *)data;
    
    // If no surface yet, create one with white background
    if (!tsp_data->surface) {
        GtkAllocation allocation;
        gtk_widget_get_allocation(widget, &allocation);
        
        if (allocation.width > 1 && allocation.height > 1) {
            tsp_data->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                                                         allocation.width,
                                                         allocation.height);
            cairo_t *temp_cr = cairo_create(tsp_data->surface);
            cairo_set_source_rgb(temp_cr, 1.0, 1.0, 1.0);
            cairo_paint(temp_cr);
            cairo_destroy(temp_cr);
        }
    }
    
    if (tsp_data->surface) {
        cairo_set_source_surface(cr, tsp_data->surface, 0, 0);
        cairo_paint(cr);
    } else {
        // Fallback: paint white background directly
        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
        cairo_paint(cr);
    }
    
    return FALSE;
}

static gboolean on_button_press(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    TSPData *tsp_data = (TSPData *)data;
    
    if (strcmp(tsp_data->mode, "add_city") == 0) {
        add_city(tsp_data, event->x, event->y);
    }
    else if (strcmp(tsp_data->mode, "delete") == 0) {
        // Find nearest city
        int nearest = -1;
        double min_dist = 25;
        
        for (int i = 0; i < tsp_data->city_count; i++) {
            double dist = sqrt((event->x - tsp_data->cities[i].x) * (event->x - tsp_data->cities[i].x) +
                             (event->y - tsp_data->cities[i].y) * (event->y - tsp_data->cities[i].y));
            if (dist < min_dist) {
                min_dist = dist;
                nearest = i;
            }
        }
        
        if (nearest >= 0) {
            // Remove city
            for (int i = nearest; i < tsp_data->city_count - 1; i++) {
                tsp_data->cities[i] = tsp_data->cities[i + 1];
            }
            
            // Remove edges
            for (int i = 0; i < MAX_CITIES; i++) {
                for (int j = 0; j < MAX_CITIES; j++) {
                    if (i == nearest || j == nearest) {
                        tsp_data->edge_exists[i][j] = 0;
                    }
                }
            }
            
            tsp_data->city_count--;
            redraw_map(tsp_data);
        }
    }
    
    return TRUE;
}

static void generate_random_cities(GtkWidget *widget, gpointer user_data) {
    TSPData *data = (TSPData *)user_data;
    
    // Clear existing
    data->city_count = 0;
    data->solution_length = 0;
    memset(data->edge_exists, 0, sizeof(data->edge_exists));
    
    GtkAllocation allocation;
    gtk_widget_get_allocation(data->canvas, &allocation);
    
    int width = allocation.width > 1 ? allocation.width : 800;
    int height = allocation.height > 1 ? allocation.height : 600;
    
    srand(time(NULL));
    int num_cities = 3 + rand() % 6; // 3-8 cities
    
    for (int i = 0; i < num_cities; i++) {
        double x = 50 + rand() % (width - 100);
        double y = 50 + rand() % (height - 100);
        add_city(data, x, y);
    }
}

static void clear_all(GtkWidget *widget, gpointer user_data) {
    TSPData *data = (TSPData *)user_data;
    
    data->city_count = 0;
    data->solution_length = 0;
    memset(data->edge_exists, 0, sizeof(data->edge_exists));
    
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(data->results_text));
    gtk_text_buffer_set_text(buffer, "", -1);
    
    redraw_map(data);
}

static void solve_tsp_callback(GtkWidget *widget, gpointer user_data) {
    TSPData *data = (TSPData *)user_data;
    tsp_solve(data);
}

void tsp_solve(TSPData *data) {
    if (data->city_count < 2) {
        GtkWidget *dialog = gtk_message_dialog_new(NULL,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_WARNING,
            GTK_BUTTONS_OK,
            "Add at least 2 cities");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Check if edges exist
    int has_edges = 0;
    for (int i = 0; i < data->city_count && !has_edges; i++) {
        for (int j = 0; j < data->city_count; j++) {
            if (data->edge_exists[i][j]) {
                has_edges = 1;
                break;
            }
        }
    }
    
    if (!has_edges) {
        GtkWidget *dialog = gtk_message_dialog_new(NULL,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_WARNING,
            GTK_BUTTONS_OK,
            "No edges defined. Enable auto-connect or add edges manually");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Solve based on algorithm
    char results[4096];
    const char *algo_name;
    
    if (strcmp(data->algorithm, "nearest_neighbor") == 0) {
        tsp_nearest_neighbor(data);
        algo_name = "Nearest Neighbor";
    }
    else if (strcmp(data->algorithm, "genetic") == 0) {
        tsp_genetic_algorithm(data);
        algo_name = "Genetic Algorithm";
    }
    else { // dynamic
        if (data->city_count > 15) {
            GtkWidget *dialog = gtk_message_dialog_new(NULL,
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_WARNING,
                GTK_BUTTONS_OK,
                "Dynamic Programming may be slow for >15 cities.\nConsider using another algorithm.");
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
        }
        tsp_dynamic_programming(data);
        algo_name = "Dynamic Programming";
    }
    
    // Format results
    sprintf(results, "=== %s ===\n\nTotal Distance: %.2f\n\nPath Sequence:\n",
            algo_name, data->total_cost);
    
    for (int i = 0; i < data->solution_length; i++) {
        char line[100];
        sprintf(line, "%d. %s ", i + 1, data->solution_path[i]->name);
        strcat(results, line);
        
        if (i < data->solution_length - 1) {
            City *c1 = data->solution_path[i];
            City *c2 = data->solution_path[i + 1];
            int idx1 = c1 - data->cities;
            int idx2 = c2 - data->cities;
            sprintf(line, "→ (%.1f) →\n", data->edges[idx1][idx2]);
            strcat(results, line);
        }
    }
    
    // Return to start
    City *last = data->solution_path[data->solution_length - 1];
    City *first = data->solution_path[0];
    int idx_last = last - data->cities;
    int idx_first = first - data->cities;
    
    char line[200];
    sprintf(line, "\nReturn to start: %s → %s (%.1f)\n\n",
            last->name, first->name, data->edges[idx_last][idx_first]);
    strcat(results, line);
    
    strcat(results, "============================\n");
    sprintf(line, "Cities Visited: %d\nTotal Edges: %d\n",
            data->solution_length, data->solution_length);
    strcat(results, line);
    
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(data->results_text));
    gtk_text_buffer_set_text(buffer, results, -1);
    
    draw_solution(data);
}

static void mode_changed(GtkWidget *widget, gpointer user_data) {
    TSPData *data = (TSPData *)user_data;
    const char *label = gtk_button_get_label(GTK_BUTTON(widget));
    
    if (strstr(label, "Add City")) {
        strcpy(data->mode, "add_city");
    }
    else if (strstr(label, "Delete")) {
        strcpy(data->mode, "delete");
    }
}

static void auto_connect_toggled(GtkWidget *widget, gpointer user_data) {
    TSPData *data = (TSPData *)user_data;
    data->auto_connect = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}

static void algo_changed(GtkWidget *widget, gpointer user_data) {
    TSPData *data = (TSPData *)user_data;
    const char *label = gtk_button_get_label(GTK_BUTTON(widget));
    
    if (strstr(label, "Nearest")) {
        strcpy(data->algorithm, "nearest_neighbor");
    }
    else if (strstr(label, "Genetic")) {
        strcpy(data->algorithm, "genetic");
    }
    else if (strstr(label, "Dynamic")) {
        strcpy(data->algorithm, "dynamic");
    }
}

void tsp_init(GtkWidget *parent_box) {
    global_tsp_data = g_new0(TSPData, 1);
    TSPData *data = global_tsp_data;
    
    strcpy(data->mode, "add_city");
    strcpy(data->algorithm, "nearest_neighbor");
    data->auto_connect = 1;
    data->custom_weight = 10.0;
    data->selected_city_idx = -1;
    
    // Apply CSS styling
    GtkCssProvider *css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css_provider,
        "label { color: #ffffff; }"
        "button, button:active, button:checked, button:hover, button:focus, button:backdrop {"
        "   background-color: #ffffff;"  
        "   color: #000000;"             
        "   font-weight: bold;"          
        "   background-image: none;"    
        "   box-shadow: none;"           
        "   text-shadow: none;"          
        "   border-color: #888888;"      
        "}"
        "button label { color: #000000; }"
        "textview, text { background-color: #000000; color: #ffffff; }",
        -1, NULL);
    
    GdkScreen *screen = gdk_screen_get_default();
    gtk_style_context_add_provider_for_screen(screen,
        GTK_STYLE_PROVIDER(css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    
    // Main container
    GtkWidget *main_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(parent_box), main_container, TRUE, TRUE, 10);
    
    // Left panel - Controls
    GtkWidget *control_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_size_request(control_panel, 350, -1);
    gtk_box_pack_start(GTK_BOX(main_container), control_panel, FALSE, FALSE, 10);
    
    // Mode selection
    GtkWidget *mode_label = gtk_label_new("Mode Selection");
    gtk_box_pack_start(GTK_BOX(control_panel), mode_label, FALSE, FALSE, 5);
    
    GtkWidget *mode_add = gtk_radio_button_new_with_label(NULL, "Add City");
    g_signal_connect(mode_add, "toggled", G_CALLBACK(mode_changed), data);
    gtk_box_pack_start(GTK_BOX(control_panel), mode_add, FALSE, FALSE, 2);
    
    GtkWidget *mode_delete = gtk_radio_button_new_with_label_from_widget(
        GTK_RADIO_BUTTON(mode_add), "Delete City");
    g_signal_connect(mode_delete, "toggled", G_CALLBACK(mode_changed), data);
    gtk_box_pack_start(GTK_BOX(control_panel), mode_delete, FALSE, FALSE, 2);
    
    // Auto-connect option
    GtkWidget *auto_check = gtk_check_button_new_with_label(
        "Auto-connect all cities (Euclidean)");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(auto_check), TRUE);
    g_signal_connect(auto_check, "toggled", G_CALLBACK(auto_connect_toggled), data);
    gtk_box_pack_start(GTK_BOX(control_panel), auto_check, FALSE, FALSE, 2);
    
    // Algorithm selection
    GtkWidget *algo_label = gtk_label_new("\nAlgorithm Selection");
    gtk_box_pack_start(GTK_BOX(control_panel), algo_label, FALSE, FALSE, 5);
    
    GtkWidget *algo_nn = gtk_radio_button_new_with_label(NULL, "Nearest Neighbor");
    g_signal_connect(algo_nn, "toggled", G_CALLBACK(algo_changed), data);
    gtk_box_pack_start(GTK_BOX(control_panel), algo_nn, FALSE, FALSE, 2);
    
    GtkWidget *algo_genetic = gtk_radio_button_new_with_label_from_widget(
        GTK_RADIO_BUTTON(algo_nn), "Genetic Algorithm");
    g_signal_connect(algo_genetic, "toggled", G_CALLBACK(algo_changed), data);
    gtk_box_pack_start(GTK_BOX(control_panel), algo_genetic, FALSE, FALSE, 2);
    
    GtkWidget *algo_dynamic = gtk_radio_button_new_with_label_from_widget(
        GTK_RADIO_BUTTON(algo_nn), "Dynamic Programming");
    g_signal_connect(algo_dynamic, "toggled", G_CALLBACK(algo_changed), data);
    gtk_box_pack_start(GTK_BOX(control_panel), algo_dynamic, FALSE, FALSE, 2);
    
    // Action buttons
    GtkWidget *solve_btn = gtk_button_new_with_label("Solve TSP");
    g_signal_connect(solve_btn, "clicked", G_CALLBACK(solve_tsp_callback), data);
    gtk_box_pack_start(GTK_BOX(control_panel), solve_btn, FALSE, FALSE, 10);
    
    GtkWidget *random_btn = gtk_button_new_with_label("Generate Random Cities");
    g_signal_connect(random_btn, "clicked", G_CALLBACK(generate_random_cities), data);
    gtk_box_pack_start(GTK_BOX(control_panel), random_btn, FALSE, FALSE, 3);
    
    GtkWidget *clear_btn = gtk_button_new_with_label("Clear All");
    g_signal_connect(clear_btn, "clicked", G_CALLBACK(clear_all), data);
    gtk_box_pack_start(GTK_BOX(control_panel), clear_btn, FALSE, FALSE, 3);
    
    // Results display
    GtkWidget *results_frame = gtk_frame_new("RESULTS");
    gtk_box_pack_start(GTK_BOX(control_panel), results_frame, TRUE, TRUE, 15);
    
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(results_frame), scrolled);
    
    data->results_text = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(data->results_text), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(data->results_text), GTK_WRAP_WORD);
    gtk_container_add(GTK_CONTAINER(scrolled), data->results_text);
    
    GtkCssProvider *text_css = gtk_css_provider_new();
    gtk_css_provider_load_from_data(text_css,
        "textview text { background-color: #000000; color: #ffffff; }",
        -1, NULL);
    gtk_style_context_add_provider(
        gtk_widget_get_style_context(data->results_text),
        GTK_STYLE_PROVIDER(text_css),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    
    // Right panel - Canvas
    GtkWidget *canvas_frame = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(main_container), canvas_frame, TRUE, TRUE, 10);
    
    // Corrected Label Name and Centering
    GtkWidget *canvas_label = gtk_label_new("Circuit Canvas - Click to add nodes, then connect components");
    gtk_widget_set_halign(canvas_label, GTK_ALIGN_CENTER);
    gtk_label_set_justify(GTK_LABEL(canvas_label), GTK_JUSTIFY_CENTER);
    
    gtk_box_pack_start(GTK_BOX(canvas_frame), canvas_label, FALSE, FALSE, 5);
    
    data->canvas = gtk_drawing_area_new();
    gtk_widget_set_size_request(data->canvas, 800, 600);
    gtk_box_pack_start(GTK_BOX(canvas_frame), data->canvas, TRUE, TRUE, 10);
    
    g_signal_connect(data->canvas, "draw", G_CALLBACK(on_draw), data);
    g_signal_connect(data->canvas, "button-press-event", 
                    G_CALLBACK(on_button_press), data);
    gtk_widget_add_events(data->canvas, GDK_BUTTON_PRESS_MASK);
    
    redraw_map(data);
}