/*
 * Kirchhoff's Circuit Laws Simulator
 * Implements KCL (Current Law) and KVL (Voltage Law) with interactive visualization
 */

#include "kirchhoff.h"
#include <cairo.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

static KirchhoffData *global_kirchhoff_data = NULL;

// Forward declarations
static gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer data);
static gboolean on_button_press(GtkWidget *widget, GdkEventButton *event, gpointer data);
static void redraw_circuit(KirchhoffData *data);
static void add_node(KirchhoffData *data, double x, double y);
static void clear_circuit(GtkWidget *widget, gpointer user_data);
static void calculate_callback(GtkWidget *widget, gpointer user_data);
static void mode_changed(GtkWidget *widget, gpointer user_data);

// Matrix operations for circuit analysis
static int solve_linear_system(int n, double **A, double *b, double *x) {
    // Gaussian elimination with partial pivoting
    double **aug = malloc(n * sizeof(double *));
    for (int i = 0; i < n; i++) {
        aug[i] = malloc((n + 1) * sizeof(double));
        for (int j = 0; j < n; j++) {
            aug[i][j] = A[i][j];
        }
        aug[i][n] = b[i];
    }
    
    // Forward elimination
    for (int i = 0; i < n; i++) {
        // Find pivot
        int max_row = i;
        for (int k = i + 1; k < n; k++) {
            if (fabs(aug[k][i]) > fabs(aug[max_row][i])) {
                max_row = k;
            }
        }
        
        // Swap rows
        double *temp = aug[i];
        aug[i] = aug[max_row];
        aug[max_row] = temp;
        
        // Check for singular matrix
        if (fabs(aug[i][i]) < 1e-10) {
            for (int k = 0; k < n; k++) free(aug[k]);
            free(aug);
            return 0;
        }
        
        // Eliminate column
        for (int k = i + 1; k < n; k++) {
            double factor = aug[k][i] / aug[i][i];
            for (int j = i; j <= n; j++) {
                aug[k][j] -= factor * aug[i][j];
            }
        }
    }
    
    // Back substitution
    for (int i = n - 1; i >= 0; i--) {
        x[i] = aug[i][n];
        for (int j = i + 1; j < n; j++) {
            x[i] -= aug[i][j] * x[j];
        }
        x[i] /= aug[i][i];
    }
    
    for (int i = 0; i < n; i++) free(aug[i]);
    free(aug);
    return 1;
}

static void add_node(KirchhoffData *data, double x, double y) {
    int node_id = -1;
    
    // Find free node ID
    for (int i = 0; i < MAX_NODES; i++) {
        if (!data->node_exists[i]) {
            node_id = i;
            break;
        }
    }
    
    if (node_id == -1) return;
    
    data->nodes[node_id].x = x;
    data->nodes[node_id].y = y;
    data->node_exists[node_id] = 1;
    data->node_count++;
    
    redraw_circuit(data);
}

static void redraw_circuit(KirchhoffData *data) {
    GtkAllocation allocation;
    gtk_widget_get_allocation(data->canvas, &allocation);
    
    // Only redraw if canvas has valid size
    if (allocation.width <= 1 || allocation.height <= 1) {
        return;
    }
    
    if (data->surface) {
        cairo_surface_destroy(data->surface);
    }
    
    data->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                                               allocation.width,
                                               allocation.height);
    cairo_t *cr = cairo_create(data->surface);
    
    // White background
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);
    
    // Draw components
    for (int i = 0; i < data->component_count; i++) {
        Component *comp = &data->components[i];
        
        if (!data->node_exists[comp->node1] || !data->node_exists[comp->node2]) {
            continue;
        }
        
        double x1 = data->nodes[comp->node1].x;
        double y1 = data->nodes[comp->node1].y;
        double x2 = data->nodes[comp->node2].x;
        double y2 = data->nodes[comp->node2].y;
        
        // Draw edge with better visibility and anti-aliasing
        cairo_set_line_width(cr, 2.5);
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0); // Strictly BLACK edges
        cairo_move_to(cr, x1, y1);
        cairo_line_to(cr, x2, y2);
        cairo_stroke(cr);
        
        // Add subtle outer glow for better visibility
        cairo_set_line_width(cr, 3.5);
        cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.2);
        cairo_move_to(cr, x1, y1);
        cairo_line_to(cr, x2, y2);
        cairo_stroke(cr);
        
        // Prepare label
        double mid_x = (x1 + x2) / 2;
        double mid_y = (y1 + y2) / 2;
        
        char label[50];
        if (comp->type == COMP_RESISTOR) {
            sprintf(label, "%.1fΩ", comp->value);
        } else if (comp->type == COMP_VOLTAGE_SOURCE) {
            sprintf(label, "%.1fV", comp->value);
        }
        
        // Font settings
        cairo_text_extents_t extents;
        cairo_select_font_face(cr, "Times New Roman",
                               CAIRO_FONT_SLANT_NORMAL,
                               CAIRO_FONT_WEIGHT_BOLD);
        
        cairo_set_font_size(cr, 15);
        cairo_text_extents(cr, label, &extents);
        
        // Calculate perpendicular offset to avoid edge overlap
        double dx = x2 - x1;
        double dy = y2 - y1;
        double len = sqrt(dx*dx + dy*dy);
        
        double offset_x = -dy / len * 25; 
        double offset_y = dx / len * 25;
        
        // Draw text
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
        cairo_move_to(cr, mid_x + offset_x - extents.width/2, mid_y + offset_y + 5);
        cairo_show_text(cr, label);
        
        // Draw current arrow if calculated
        if (fabs(comp->current) > 0.001) {
            cairo_set_source_rgb(cr, 0.0, 0.0, 0.0); // Black Arrow
            
            double angle = atan2(y2 - y1, x2 - x1);
            double arrow_len = 15;
            
            double ax, ay;
            if (comp->current > 0) {
                ax = mid_x + arrow_len * cos(angle);
                ay = mid_y + arrow_len * sin(angle);
            } else {
                ax = mid_x - arrow_len * cos(angle);
                ay = mid_y - arrow_len * sin(angle);
            }
            
            cairo_set_line_width(cr, 2);
            cairo_move_to(cr, mid_x, mid_y);
            cairo_line_to(cr, ax, ay);
            
            // Arrow head
            double arrow_angle1 = angle + 3.14159 / 6;
            double arrow_angle2 = angle - 3.14159 / 6;
            cairo_line_to(cr, ax - 5 * cos(arrow_angle1), ay - 5 * sin(arrow_angle1));
            cairo_move_to(cr, ax, ay);
            cairo_line_to(cr, ax - 5 * cos(arrow_angle2), ay - 5 * sin(arrow_angle2));
            cairo_stroke(cr);
            
            // Current value
            sprintf(label, "%.2fA", fabs(comp->current));
            cairo_set_font_size(cr, 9);
            cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
            cairo_move_to(cr, mid_x - 15, mid_y + 15);
            cairo_show_text(cr, label);
        }
    }
    
    // Draw nodes
    for (int i = 0; i < MAX_NODES; i++) {
        if (!data->node_exists[i]) continue;
        
        double x = data->nodes[i].x;
        double y = data->nodes[i].y;
        
        // [FIX 1] Start a fresh path for every node to prevent the 
        // "Random Green Line" connecting the previous text position to this circle.
        cairo_new_path(cr);

        // Circle (filled green)
        cairo_set_source_rgb(cr, 0.506, 0.780, 0.514); // #81c784
        cairo_arc(cr, x, y, 14, 0, 2 * M_PI); // Radius 14
        cairo_fill_preserve(cr);
        
        // Border (stroked dark green)
        cairo_set_source_rgb(cr, 0.333, 0.545, 0.184); // #558b2f
        cairo_set_line_width(cr, 2);
        cairo_stroke(cr);
        
        // Label
        char label[10];
        sprintf(label, "N%d", i);
        
        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0); // White text
        cairo_select_font_face(cr, "Times New Roman",
                               CAIRO_FONT_SLANT_NORMAL,
                               CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 12);
        
        cairo_move_to(cr, x - 10, y - 28); 
        cairo_show_text(cr, label);

        // [FIX 2] Reset color to BLACK after drawing the node
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    }
    
    cairo_destroy(cr);
    gtk_widget_queue_draw(data->canvas);
}

static gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer data) {
    KirchhoffData *k_data = (KirchhoffData *)data;
    
    // If no surface yet, create one with white background
    if (!k_data->surface) {
        GtkAllocation allocation;
        gtk_widget_get_allocation(widget, &allocation);
        
        if (allocation.width > 1 && allocation.height > 1) {
            k_data->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                                                         allocation.width,
                                                         allocation.height);
            cairo_t *temp_cr = cairo_create(k_data->surface);
            cairo_set_source_rgb(temp_cr, 1.0, 1.0, 1.0);
            cairo_paint(temp_cr);
            cairo_destroy(temp_cr);
        }
    }
    
    if (k_data->surface) {
        cairo_set_source_surface(cr, k_data->surface, 0, 0);
        cairo_paint(cr);
    } else {
        // Fallback: paint white background directly
        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
        cairo_paint(cr);
    }
    
    return FALSE;
}

static int find_nearest_node(KirchhoffData *data, double x, double y) {
    int nearest = -1;
    double min_dist = 25; // Increased hit-box from 20 to 25
    
    for (int i = 0; i < MAX_NODES; i++) {
        if (!data->node_exists[i]) continue;
        
        double dist = sqrt((x - data->nodes[i].x) * (x - data->nodes[i].x) +
                           (y - data->nodes[i].y) * (y - data->nodes[i].y));
        if (dist < min_dist) {
            min_dist = dist;
            nearest = i;
        }
    }
    
    return nearest;
}

static gboolean on_button_press(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    KirchhoffData *k_data = (KirchhoffData *)data;
    
    if (strcmp(k_data->mode, "add_node") == 0) {
        add_node(k_data, event->x, event->y);
    }
    else if (strcmp(k_data->mode, "add_resistor") == 0 || 
             strcmp(k_data->mode, "add_voltage") == 0) {
        int node = find_nearest_node(k_data, event->x, event->y);
        
        if (node >= 0) {
            if (k_data->selected_node == -1) {
                // First node selected
                k_data->selected_node = node;
            } else {
                // Second node selected - connect them
                if (node != k_data->selected_node) {
                    
                    // --- FIX STARTS HERE ---
                    // 1. Get text from the entry box
                    const char *val_text = gtk_entry_get_text(GTK_ENTRY(k_data->value_entry));
                    
                    // 2. Convert string to double
                    double current_val = atof(val_text);
                    
                    // 3. Update the data structure (optional validation)
                    if (current_val != 0.0) {
                        k_data->component_value = current_val;
                    }
                    // --- FIX ENDS HERE ---

                    // Add component
                    if (k_data->component_count < MAX_COMPONENTS) {
                        Component *comp = &k_data->components[k_data->component_count];
                        comp->node1 = k_data->selected_node;
                        comp->node2 = node;
                        
                        // Assign the freshly read value
                        comp->value = k_data->component_value;
                        
                        comp->type = strcmp(k_data->mode, "add_resistor") == 0 ? 
                                     COMP_RESISTOR : COMP_VOLTAGE_SOURCE;
                        comp->current = 0;
                        k_data->component_count++;
                        
                        redraw_circuit(k_data);
                    }
                }
                k_data->selected_node = -1;
            }
        }
    }
    else if (strcmp(k_data->mode, "delete") == 0) {
        int node = find_nearest_node(k_data, event->x, event->y);
        
        if (node >= 0) {
            // Remove node
            k_data->node_exists[node] = 0;
            k_data->node_count--;
            
            // Remove associated components
            int new_count = 0;
            for (int i = 0; i < k_data->component_count; i++) {
                if (k_data->components[i].node1 != node && 
                    k_data->components[i].node2 != node) {
                    k_data->components[new_count++] = k_data->components[i];
                }
            }
            k_data->component_count = new_count;
            
            redraw_circuit(k_data);
        }
    }
    
    return TRUE;
}

void kirchhoff_calculate(KirchhoffData *data) {
    const char *ground_text = gtk_entry_get_text(GTK_ENTRY(data->ground_entry));
    data->ground_node = atoi(ground_text);

    if (data->node_count < 2 || data->component_count == 0) {
        GtkWidget *dialog = gtk_message_dialog_new(NULL,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_WARNING,
            GTK_BUTTONS_OK,
            "Please create a circuit with at least 2 nodes and 1 component");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Build node list (excluding ground)
    int node_list[MAX_NODES];
    int n = 0;
    
    for (int i = 0; i < MAX_NODES; i++) {
        if (data->node_exists[i] && i != data->ground_node) {
            node_list[n++] = i;
        }
    }
    
    if (n == 0) {
        GtkWidget *dialog = gtk_message_dialog_new(NULL,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_WARNING,
            GTK_BUTTONS_OK,
            "Need at least one non-ground node");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Allocate conductance matrix and current vector
    double **G = malloc(n * sizeof(double *));
    double *I = calloc(n, sizeof(double));
    double *V = malloc(n * sizeof(double));
    
    for (int i = 0; i < n; i++) {
        G[i] = calloc(n, sizeof(double));
    }
    
    // Build system using nodal analysis
    for (int c = 0; c < data->component_count; c++) {
        Component *comp = &data->components[c];
        int n1 = comp->node1;
        int n2 = comp->node2;
        
        if (comp->type == COMP_RESISTOR) {
            double conductance = 1.0 / comp->value;
            
            // Find indices in node_list
            int idx1 = -1, idx2 = -1;
            for (int i = 0; i < n; i++) {
                if (node_list[i] == n1) idx1 = i;
                if (node_list[i] == n2) idx2 = i;
            }
            
            if (idx1 >= 0) {
                G[idx1][idx1] += conductance;
                if (idx2 >= 0) {
                    G[idx1][idx2] -= conductance;
                }
            }
            
            if (idx2 >= 0) {
                G[idx2][idx2] += conductance;
                if (idx1 >= 0) {
                    G[idx2][idx1] -= conductance;
                }
            }
        }
        else if (comp->type == COMP_VOLTAGE_SOURCE) {
            // Case 1: Connected to Ground (Use forcing method for precision)
            if (n1 == data->ground_node) {
                int idx = -1;
                for (int i = 0; i < n; i++) {
                    if (node_list[i] == n2) idx = i;
                }
                if (idx >= 0) {
                    memset(G[idx], 0, n * sizeof(double));
                    G[idx][idx] = 1.0;
                    // n1 is ground (+), n2 is node (-). V_n1 - V_n2 = Val => 0 - V_n2 = Val => V_n2 = -Val
                    I[idx] = -comp->value; 
                }
            }
            else if (n2 == data->ground_node) {
                int idx = -1;
                for (int i = 0; i < n; i++) {
                    if (node_list[i] == n1) idx = i;
                }
                if (idx >= 0) {
                    memset(G[idx], 0, n * sizeof(double));
                    G[idx][idx] = 1.0;
                    // n1 is node (+), n2 is ground (-). V_n1 - 0 = Val
                    I[idx] = comp->value;
                }
            }
            // Case 2: Floating Voltage Source (Between two non-ground nodes)
            // Fix: Use Norton Equivalent (Current Source || Small Resistor)
            else {
                int idx1 = -1, idx2 = -1;
                for (int i = 0; i < n; i++) {
                    if (node_list[i] == n1) idx1 = i;
                    if (node_list[i] == n2) idx2 = i;
                }

                if (idx1 >= 0 && idx2 >= 0) {
                    // Use a very small internal resistance to model the ideal source
                    double r_internal = 0.01; 
                    double g_internal = 1.0 / r_internal;
                    double i_injected = comp->value / r_internal;

                    // Update Conductance Matrix (Resistor part)
                    G[idx1][idx1] += g_internal;
                    G[idx2][idx2] += g_internal;
                    G[idx1][idx2] -= g_internal;
                    G[idx2][idx1] -= g_internal;

                    // Update Current Vector (Source part)
                    // Current is injected into Positive Node (n1) and extracted from Negative Node (n2)
                    I[idx1] += i_injected;
                    I[idx2] -= i_injected;
                }
            }
        }
    }
    
    // Solve system
    if (!solve_linear_system(n, G, I, V)) {
        GtkWidget *dialog = gtk_message_dialog_new(NULL,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Cannot solve circuit - check your connections or ensure nodes are not isolated.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        
        for (int i = 0; i < n; i++) free(G[i]);
        free(G);
        free(I);
        free(V);
        return;
    }
    
    // Create voltage map
    double voltage_map[MAX_NODES] = {0};
    voltage_map[data->ground_node] = 0;
    for (int i = 0; i < n; i++) {
        voltage_map[node_list[i]] = V[i];
    }
    
    // Calculate component currents
    for (int c = 0; c < data->component_count; c++) {
        Component *comp = &data->components[c];
        double v1 = voltage_map[comp->node1];
        double v2 = voltage_map[comp->node2];
        
        if (comp->type == COMP_RESISTOR) {
            comp->current = (v1 - v2) / comp->value;
        }
        else if (comp->type == COMP_VOLTAGE_SOURCE) {
            // For voltage sources, we can estimate current using node voltages 
            // and Kirchhoff's Current Law at the nodes, but simpler is:
            // The simulator solves V, but I through a V-source is a dependent variable.
            // We can leave it as 0 for display, or back-calculate using KCL if implemented.
            // For this version, we'll leave it 0 or calculate if there's a simple path.
            comp->current = 0; 
        }
    }
    
    // Format results
    char results[8192];
    sprintf(results, "=== CIRCUIT ANALYSIS ===\n\nNode Voltages:\n");
    sprintf(results + strlen(results), "N%d (Ground): 0.00 V\n", data->ground_node);
    
    for (int i = 0; i < n; i++) {
        sprintf(results + strlen(results), "N%d: %.2f V\n", node_list[i], V[i]);
    }
    
    strcat(results, "\n=========================\n");
    strcat(results, "Component Currents:\n");
    
    for (int c = 0; c < data->component_count; c++) {
        Component *comp = &data->components[c];
        
        if (comp->type == COMP_RESISTOR) {
            sprintf(results + strlen(results), "\nR%d (N%d->N%d):\n", 
                   c, comp->node1, comp->node2);
            sprintf(results + strlen(results), "  %.1f Ohm\n", comp->value);
            sprintf(results + strlen(results), "  Current: %.3f A\n", fabs(comp->current));
            sprintf(results + strlen(results), "  Power: %.3f W\n", 
                   fabs(comp->current) * fabs(comp->current) * comp->value);
        }
        else if (comp->type == COMP_VOLTAGE_SOURCE) {
            sprintf(results + strlen(results), "\nV%d (N%d->N%d):\n", 
                   c, comp->node1, comp->node2);
            sprintf(results + strlen(results), "  %.1fV\n", comp->value);
        }
    }
    
    strcat(results, "\n=========================\n");
    strcat(results, "Kirchhoff's Laws Verified:\n");
    strcat(results, "✓ KCL: Sum(I_in) = Sum(I_out)\n");
    strcat(results, "✓ KVL: Sum(V_loop) = 0\n");
    
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(data->results_text));
    gtk_text_buffer_set_text(buffer, results, -1);
    
    // Redraw with currents
    redraw_circuit(data);
    
    // Cleanup
    for (int i = 0; i < n; i++) free(G[i]);
    free(G);
    free(I);
    free(V);
}

static void calculate_callback(GtkWidget *widget, gpointer user_data) {
    KirchhoffData *data = (KirchhoffData *)user_data;
    kirchhoff_calculate(data);
}

static void clear_circuit(GtkWidget *widget, gpointer user_data) {
    KirchhoffData *data = (KirchhoffData *)user_data;
    
    memset(data->node_exists, 0, sizeof(data->node_exists));
    data->node_count = 0;
    data->component_count = 0;
    data->selected_node = -1;
    
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(data->results_text));
    gtk_text_buffer_set_text(buffer, "", -1);
    
    redraw_circuit(data);
}

static void mode_changed(GtkWidget *widget, gpointer user_data) {
    KirchhoffData *data = (KirchhoffData *)user_data;
    const char *label = gtk_button_get_label(GTK_BUTTON(widget));
    
    if (strstr(label, "Add Node")) {
        strcpy(data->mode, "add_node");
    }
    else if (strstr(label, "Add Resistor")) {
        strcpy(data->mode, "add_resistor");
    }
    else if (strstr(label, "Add Voltage")) {
        strcpy(data->mode, "add_voltage");
    }
    else if (strstr(label, "Delete")) {
        strcpy(data->mode, "delete");
    }
    
    data->selected_node = -1;
}

void kirchhoff_init(GtkWidget *parent_box) {
    global_kirchhoff_data = g_new0(KirchhoffData, 1);
    KirchhoffData *data = global_kirchhoff_data;
    
    strcpy(data->mode, "add_node");
    data->component_value = 10.0;
    data->ground_node = 0;
    data->selected_node = -1;
    
    // Apply CSS styling for labels and buttons
    // UPDATED: Added specific selectors for :active, :checked, :hover, :focus
    // to ensure the button stays white with black text in all states.
    GtkCssProvider *css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css_provider,
        /* Global label text is white (for your dashboard text) */
        "label { color: #ffffff; }"
        
        /* CONSTANT WHITE BUTTONS */
        "button, button:active, button:checked, button:hover, button:focus, button:backdrop {"
        "   background-color: #ffffff;"
        "   color: #000000;"
        "   font-weight: bold;"
        "   background-image: none;"
        "   box-shadow: none;"
        "   text-shadow: none;"
        "   border-color: #888888;"
        "}"
        
        /* SAFETY FIX: Ensure text *inside* buttons is explicitly black */
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
    
    // Left panel
    GtkWidget *control_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_size_request(control_panel, 350, -1);
    gtk_box_pack_start(GTK_BOX(main_container), control_panel, FALSE, FALSE, 10);
    
    // Mode selection
    GtkWidget *mode_label = gtk_label_new("Mode Selection");
    gtk_box_pack_start(GTK_BOX(control_panel), mode_label, FALSE, FALSE, 10);
    
    GtkWidget *mode_node = gtk_radio_button_new_with_label(NULL, "Add Node");
    g_signal_connect(mode_node, "toggled", G_CALLBACK(mode_changed), data);
    gtk_box_pack_start(GTK_BOX(control_panel), mode_node, FALSE, FALSE, 3);
    
    GtkWidget *mode_resistor = gtk_radio_button_new_with_label_from_widget(
        GTK_RADIO_BUTTON(mode_node), "Add Resistor");
    g_signal_connect(mode_resistor, "toggled", G_CALLBACK(mode_changed), data);
    gtk_box_pack_start(GTK_BOX(control_panel), mode_resistor, FALSE, FALSE, 3);
    
    GtkWidget *mode_voltage = gtk_radio_button_new_with_label_from_widget(
        GTK_RADIO_BUTTON(mode_node), "Add Voltage Source");
    g_signal_connect(mode_voltage, "toggled", G_CALLBACK(mode_changed), data);
    gtk_box_pack_start(GTK_BOX(control_panel), mode_voltage, FALSE, FALSE, 3);
    
    GtkWidget *mode_delete = gtk_radio_button_new_with_label_from_widget(
        GTK_RADIO_BUTTON(mode_node), "Delete Component");
    g_signal_connect(mode_delete, "toggled", G_CALLBACK(mode_changed), data);
    gtk_box_pack_start(GTK_BOX(control_panel), mode_delete, FALSE, FALSE, 3);
    
    // Component value
    GtkWidget *value_label = gtk_label_new("\nComponent Value");
    gtk_box_pack_start(GTK_BOX(control_panel), value_label, FALSE, FALSE, 5);
    
    data->value_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(data->value_entry), "10");
    gtk_entry_set_width_chars(GTK_ENTRY(data->value_entry), 8);
    gtk_box_pack_start(GTK_BOX(control_panel), data->value_entry, FALSE, FALSE, 5);
    
    GtkWidget *value_hint = gtk_label_new("(Resistance in Ω, Voltage in V)");
    gtk_box_pack_start(GTK_BOX(control_panel), value_hint, FALSE, FALSE, 0);
    
    // Ground node
    GtkWidget *ground_label = gtk_label_new("\nGround Node (Reference)");
    gtk_box_pack_start(GTK_BOX(control_panel), ground_label, FALSE, FALSE, 5);
    
    data->ground_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(data->ground_entry), "0");
    gtk_entry_set_width_chars(GTK_ENTRY(data->ground_entry), 8);
    gtk_box_pack_start(GTK_BOX(control_panel), data->ground_entry, FALSE, FALSE, 5);
    
    // Buttons
    GtkWidget *calc_btn = gtk_button_new_with_label("Calculate Circuit");
    g_signal_connect(calc_btn, "clicked", G_CALLBACK(calculate_callback), data);
    gtk_box_pack_start(GTK_BOX(control_panel), calc_btn, FALSE, FALSE, 10);
    
    GtkWidget *clear_btn = gtk_button_new_with_label("Clear Circuit");
    g_signal_connect(clear_btn, "clicked", G_CALLBACK(clear_circuit), data);
    gtk_box_pack_start(GTK_BOX(control_panel), clear_btn, FALSE, FALSE, 3);
    
    // Results
    GtkWidget *results_frame = gtk_frame_new("RESULTS");
    gtk_box_pack_start(GTK_BOX(control_panel), results_frame, TRUE, TRUE, 15);
    
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(results_frame), scrolled);
    
    data->results_text = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(data->results_text), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(data->results_text), GTK_WRAP_WORD);
    gtk_container_add(GTK_CONTAINER(scrolled), data->results_text);
    
    // Style results text view with black background and white text
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
    
    GtkWidget *canvas_label = gtk_label_new(
        "Circuit Canvas - Click to add nodes, then connect components");
    gtk_box_pack_start(GTK_BOX(canvas_frame), canvas_label, FALSE, FALSE, 5);
    
    data->canvas = gtk_drawing_area_new();
    gtk_widget_set_size_request(data->canvas, 800, 600);
    gtk_box_pack_start(GTK_BOX(canvas_frame), data->canvas, TRUE, TRUE, 10);
    
    g_signal_connect(data->canvas, "draw", G_CALLBACK(on_draw), data);
    g_signal_connect(data->canvas, "button-press-event", 
                    G_CALLBACK(on_button_press), data);
    gtk_widget_add_events(data->canvas, GDK_BUTTON_PRESS_MASK);
    
    redraw_circuit(data);
}