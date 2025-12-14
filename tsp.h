/*
 * tsp.h - TSP Module Header File
 * Contains data structures and function declarations for the TSP Simulator
 */

#ifndef TSP_H
#define TSP_H

#include <gtk/gtk.h>
#include <cairo.h>

// Maximum limits
#define MAX_CITIES 50
#define MAX_EDGES 2500  // MAX_CITIES * MAX_CITIES

// City structure - represents a single city in the TSP
typedef struct {
    double x;           // X coordinate on canvas
    double y;           // Y coordinate on canvas
    char name[20];      // City name (e.g., "C0", "C1")
} City;

// Main TSP Data Structure
typedef struct {
    // City data
    City cities[MAX_CITIES];
    int city_count;
    int city_counter;  // For naming cities sequentially
    
    // Edge data (adjacency matrix)
    double edges[MAX_CITIES][MAX_CITIES];
    int edge_exists[MAX_CITIES][MAX_CITIES];
    
    // Solution data
    City *solution_path[MAX_CITIES];
    int solution_length;
    double total_cost;
    
    // UI state
    int selected_city_idx;
    char mode[30];          // "add_city", "add_edge", "delete"
    int auto_connect;       // Boolean for auto-connecting cities
    double custom_weight;   // Weight for custom edges
    char algorithm[30];     // "nearest_neighbor", "genetic", "dynamic"
    
    // GTK widgets
    GtkWidget *canvas;
    GtkWidget *results_text;
    GtkWidget *weight_entry;
    GtkWidget *mode_var;
    GtkWidget *algo_var;
    GtkWidget *auto_connect_check;
    
    // Cairo surface for drawing
    cairo_surface_t *surface;
    
    // Temporary drawing state
    cairo_t *temp_cr;
    int temp_line_active;
} TSPData;

// Function declarations

/**
 * Initialize the TSP simulator
 * Creates UI and sets up initial state
 * @param parent_box The GTK box to pack the UI into
 */
void tsp_init(GtkWidget *parent_box);

/**
 * Solve the TSP using the selected algorithm
 * @param data Pointer to TSPData structure
 */
void tsp_solve(TSPData *data);

/**
 * Clean up and free TSP data
 * @param data Pointer to TSPData structure
 */
void tsp_cleanup(TSPData *data);

#endif // TSP_H