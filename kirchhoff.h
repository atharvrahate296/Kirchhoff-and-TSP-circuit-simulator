#ifndef KIRCHHOFF_H
#define KIRCHHOFF_H

#include <gtk/gtk.h>

#define MAX_NODES 20
#define MAX_COMPONENTS 50

typedef enum {
    COMP_RESISTOR,
    COMP_VOLTAGE_SOURCE,
    COMP_CURRENT_SOURCE
} ComponentType;

typedef struct {
    int node1;
    int node2;
    double value;
    ComponentType type;
    double current;
} Component;

typedef struct {
    double x;
    double y;
} Node;

typedef struct {
    Node nodes[MAX_NODES];
    int node_exists[MAX_NODES];
    int node_count;
    
    Component components[MAX_COMPONENTS];
    int component_count;
    
    int selected_node;
    char mode[20];
    int ground_node;
    double component_value;
    
    GtkWidget *canvas;
    GtkWidget *results_text;
    GtkWidget *value_entry;
    GtkWidget *ground_entry;
    cairo_surface_t *surface;
} KirchhoffData;

// Function declarations
void kirchhoff_init(GtkWidget *parent_box);
void kirchhoff_calculate(KirchhoffData *data);

#endif // KIRCHHOFF_H