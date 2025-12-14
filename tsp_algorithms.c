/*
 * TSP Algorithm Implementations
 * Contains Nearest Neighbor, Genetic Algorithm, and Dynamic Programming solutions
 */

#include "tsp_algorithms.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#define POPULATION_SIZE 100
#define GENERATIONS 500
#define MUTATION_RATE 0.01
#define TOURNAMENT_SIZE 5

static double get_distance(TSPData *data, int i, int j) {
    if (data->edge_exists[i][j]) {
        return data->edges[i][j];
    }
    return INFINITY;
}

static double calculate_path_cost(TSPData *data, int *path, int length) {
    double cost = 0;
    for (int i = 0; i < length; i++) {
        int next = (i + 1) % length;
        cost += get_distance(data, path[i], path[next]);
    }
    return cost;
}

// Nearest Neighbor Algorithm - O(n^2)
void tsp_nearest_neighbor(TSPData *data) {
    if (data->city_count == 0) return;
    
    int visited[MAX_CITIES] = {0};
    int path[MAX_CITIES];
    int path_len = 0;
    
    // Start from city 0
    int current = 0;
    path[path_len++] = current;
    visited[current] = 1;
    
    while (path_len < data->city_count) {
        int nearest = -1;
        double min_dist = INFINITY;
        
        for (int i = 0; i < data->city_count; i++) {
            if (!visited[i]) {
                double dist = get_distance(data, current, i);
                if (dist < min_dist) {
                    min_dist = dist;
                    nearest = i;
                }
            }
        }
        
        if (nearest == -1) break;
        
        path[path_len++] = nearest;
        visited[nearest] = 1;
        current = nearest;
    }
    
    // Store solution
    data->solution_length = path_len;
    for (int i = 0; i < path_len; i++) {
        data->solution_path[i] = &data->cities[path[i]];
    }
    data->total_cost = calculate_path_cost(data, path, path_len);
}

// Genetic Algorithm Implementation
typedef struct {
    int tour[MAX_CITIES];
    double fitness;
} Individual;

static void shuffle_tour(int *tour, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = tour[i];
        tour[i] = tour[j];
        tour[j] = temp;
    }
}

static double fitness_function(TSPData *data, int *tour, int n) {
    double cost = calculate_path_cost(data, tour, n);
    return 1.0 / (cost + 1.0);
}

static void select_parent(Individual *population, int pop_size, int *selected) {
    int best_idx = rand() % pop_size;
    double best_fitness = population[best_idx].fitness;
    
    for (int i = 1; i < TOURNAMENT_SIZE && i < pop_size; i++) {
        int idx = rand() % pop_size;
        if (population[idx].fitness > best_fitness) {
            best_fitness = population[idx].fitness;
            best_idx = idx;
        }
    }
    
    memcpy(selected, population[best_idx].tour, MAX_CITIES * sizeof(int));
}

static void crossover(int *parent1, int *parent2, int *child, int n) {
    int start = rand() % n;
    int end = start + 1 + rand() % (n - start);
    
    int used[MAX_CITIES] = {0};
    
    // Copy segment from parent1
    for (int i = start; i < end; i++) {
        child[i] = parent1[i];
        used[parent1[i]] = 1;
    }
    
    // Fill remaining from parent2
    int pos = end % n;
    for (int i = 0; i < n; i++) {
        int idx = (end + i) % n;
        if (!used[parent2[idx]]) {
            child[pos] = parent2[idx];
            used[parent2[idx]] = 1;
            pos = (pos + 1) % n;
        }
    }
}

static void mutate(int *tour, int n) {
    if ((double)rand() / RAND_MAX < MUTATION_RATE) {
        int i = rand() % n;
        int j = rand() % n;
        int temp = tour[i];
        tour[i] = tour[j];
        tour[j] = temp;
    }
}

void tsp_genetic_algorithm(TSPData *data) {
    if (data->city_count < 2) return;
    
    srand(time(NULL));
    int n = data->city_count;
    
    // Initialize population
    Individual population[POPULATION_SIZE];
    Individual new_population[POPULATION_SIZE];
    
    for (int i = 0; i < POPULATION_SIZE; i++) {
        for (int j = 0; j < n; j++) {
            population[i].tour[j] = j;
        }
        shuffle_tour(population[i].tour, n);
        population[i].fitness = fitness_function(data, population[i].tour, n);
    }
    
    // Find initial best
    int best_idx = 0;
    for (int i = 1; i < POPULATION_SIZE; i++) {
        if (population[i].fitness > population[best_idx].fitness) {
            best_idx = i;
        }
    }
    
    Individual best = population[best_idx];
    
    // Evolution
    for (int gen = 0; gen < GENERATIONS; gen++) {
        // Elitism - keep best
        new_population[0] = best;
        
        // Create new population
        for (int i = 1; i < POPULATION_SIZE; i++) {
            int parent1[MAX_CITIES], parent2[MAX_CITIES];
            select_parent(population, POPULATION_SIZE, parent1);
            select_parent(population, POPULATION_SIZE, parent2);
            
            crossover(parent1, parent2, new_population[i].tour, n);
            mutate(new_population[i].tour, n);
            
            new_population[i].fitness = fitness_function(data, 
                                                         new_population[i].tour, n);
        }
        
        // Update population
        memcpy(population, new_population, sizeof(population));
        
        // Track best
        for (int i = 0; i < POPULATION_SIZE; i++) {
            if (population[i].fitness > best.fitness) {
                best = population[i];
            }
        }
    }
    
    // Store solution
    data->solution_length = n;
    for (int i = 0; i < n; i++) {
        data->solution_path[i] = &data->cities[best.tour[i]];
    }
    data->total_cost = calculate_path_cost(data, best.tour, n);
}

// Dynamic Programming - Held-Karp Algorithm O(n^2 * 2^n)
void tsp_dynamic_programming(TSPData *data) {
    if (data->city_count < 2) return;
    
    int n = data->city_count;
    
    // For large n, fall back to nearest neighbor
    if (n > 20) {
        tsp_nearest_neighbor(data);
        return;
    }
    
    int max_mask = 1 << n;
    
    // Allocate DP table
    double **dp = malloc(max_mask * sizeof(double *));
    int **parent = malloc(max_mask * sizeof(int *));
    
    for (int i = 0; i < max_mask; i++) {
        dp[i] = malloc(n * sizeof(double));
        parent[i] = malloc(n * sizeof(int));
        for (int j = 0; j < n; j++) {
            dp[i][j] = INFINITY;
            parent[i][j] = -1;
        }
    }
    
    // Base case - start from city 0
    dp[1][0] = 0;
    
    // Fill DP table
    for (int mask = 1; mask < max_mask; mask++) {
        for (int last = 0; last < n; last++) {
            if (!(mask & (1 << last))) continue;
            if (dp[mask][last] == INFINITY) continue;
            
            // Try extending to next city
            for (int next = 0; next < n; next++) {
                if (mask & (1 << next)) continue;
                
                int next_mask = mask | (1 << next);
                double dist = get_distance(data, last, next);
                double new_cost = dp[mask][last] + dist;
                
                if (new_cost < dp[next_mask][next]) {
                    dp[next_mask][next] = new_cost;
                    parent[next_mask][next] = last;
                }
            }
        }
    }
    
    // Find best ending city
    int full_mask = (1 << n) - 1;
    double best_cost = INFINITY;
    int best_last = -1;
    
    for (int last = 0; last < n; last++) {
        double cost = dp[full_mask][last] + get_distance(data, last, 0);
        if (cost < best_cost) {
            best_cost = cost;
            best_last = last;
        }
    }
    
    // Reconstruct path
    if (best_last != -1) {
        int path[MAX_CITIES];
        int path_len = 0;
        int mask = full_mask;
        int current = best_last;
        
        while (current != 0) {
            path[path_len++] = current;
            int prev_mask = mask ^ (1 << current);
            current = parent[mask][current];
            mask = prev_mask;
        }
        path[path_len++] = 0;
        
        // Reverse path
        data->solution_length = path_len;
        for (int i = 0; i < path_len; i++) {
            data->solution_path[i] = &data->cities[path[path_len - 1 - i]];
        }
        data->total_cost = best_cost;
    }
    
    // Free memory
    for (int i = 0; i < max_mask; i++) {
        free(dp[i]);
        free(parent[i]);
    }
    free(dp);
    free(parent);
}