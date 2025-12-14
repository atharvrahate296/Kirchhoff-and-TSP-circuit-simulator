#ifndef TSP_ALGORITHMS_H
#define TSP_ALGORITHMS_H

#include "tsp.h"

// Algorithm implementations
void tsp_nearest_neighbor(TSPData *data);
void tsp_genetic_algorithm(TSPData *data);
void tsp_dynamic_programming(TSPData *data);

#endif // TSP_ALGORITHMS_H