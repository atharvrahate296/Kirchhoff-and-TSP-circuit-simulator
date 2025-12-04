"""
TSP Algorithm Implementations
Contains Nearest Neighbor, Genetic Algorithm, and Dynamic Programming solutions
"""

import random
import math
from typing import List, Tuple, Dict

class TSPSolver:
    """Solver for Traveling Salesman Problem"""
    
    def __init__(self, cities, edges):
        """
        Initialize solver
        cities: List of City objects
        edges: Dict of (city1, city2): weight
        """
        self.cities = cities
        self.edges = edges
    
    def get_distance(self, city1, city2):
        """Get distance between two cities"""
        return self.edges.get((city1, city2), float('inf'))
    
    def calculate_path_cost(self, path):
        """Calculate total cost of a path"""
        cost = 0
        for i in range(len(path)):
            city1 = path[i]
            city2 = path[(i + 1) % len(path)]
            cost += self.get_distance(city1, city2)
        return cost
    
    def nearest_neighbor(self, start_city=None):
        """
        Nearest Neighbor Algorithm (Greedy Approach)
        Time Complexity: O(n^2)
        """
        if not self.cities:
            return [], 0
        
        if start_city is None:
            start_city = self.cities[0]
        
        unvisited = set(self.cities)
        path = [start_city]
        current = start_city
        unvisited.remove(current)
        
        while unvisited:
            nearest = None
            min_dist = float('inf')
            
            for city in unvisited:
                dist = self.get_distance(current, city)
                if dist < min_dist:
                    min_dist = dist
                    nearest = city
            
            if nearest is None:
                break
            
            path.append(nearest)
            current = nearest
            unvisited.remove(current)
        
        total_cost = self.calculate_path_cost(path)
        return path, total_cost
    
    def genetic_algorithm(self, population_size=100, generations=500, mutation_rate=0.01):
        """
        Genetic Algorithm for TSP
        Uses selection, crossover, and mutation
        """
        if len(self.cities) < 2:
            return [], 0
        
        def create_individual():
            """Create a random tour"""
            tour = self.cities.copy()
            random.shuffle(tour)
            return tour
        
        def fitness(tour):
            """Fitness is inverse of tour cost"""
            cost = self.calculate_path_cost(tour)
            return 1 / (cost + 1)
        
        def select_parent(population, fitnesses):
            """Tournament selection"""
            tournament_size = 5
            tournament = random.sample(list(zip(population, fitnesses)), 
                                     min(tournament_size, len(population)))
            return max(tournament, key=lambda x: x[1])[0]
        
        def crossover(parent1, parent2):
            """Order crossover (OX)"""
            size = len(parent1)
            start, end = sorted(random.sample(range(size), 2))
            
            child = [None] * size
            child[start:end] = parent1[start:end]
            
            pointer = end
            for city in parent2[end:] + parent2[:end]:
                if city not in child:
                    if pointer >= size:
                        pointer = 0
                    child[pointer] = city
                    pointer += 1
            
            return child
        
        def mutate(tour):
            """Swap mutation"""
            if random.random() < mutation_rate:
                i, j = random.sample(range(len(tour)), 2)
                tour[i], tour[j] = tour[j], tour[i]
            return tour
        
        # Initialize population
        population = [create_individual() for _ in range(population_size)]
        
        best_tour = None
        best_fitness = 0
        
        for generation in range(generations):
            # Calculate fitness
            fitnesses = [fitness(tour) for tour in population]
            
            # Track best
            max_fitness_idx = fitnesses.index(max(fitnesses))
            if fitnesses[max_fitness_idx] > best_fitness:
                best_fitness = fitnesses[max_fitness_idx]
                best_tour = population[max_fitness_idx].copy()
            
            # Create new population
            new_population = []
            
            # Elitism - keep best individual
            new_population.append(best_tour.copy())
            
            while len(new_population) < population_size:
                parent1 = select_parent(population, fitnesses)
                parent2 = select_parent(population, fitnesses)
                
                child = crossover(parent1, parent2)
                child = mutate(child)
                
                new_population.append(child)
            
            population = new_population
        
        total_cost = self.calculate_path_cost(best_tour)
        return best_tour, total_cost
    
    def dynamic_programming(self):
        """
        Dynamic Programming solution using Held-Karp algorithm
        Time Complexity: O(n^2 * 2^n)
        Space Complexity: O(n * 2^n)
        Only practical for small numbers of cities (< 20)
        """
        if len(self.cities) < 2:
            return [], 0
        
        n = len(self.cities)
        
        # For large n, fall back to nearest neighbor
        if n > 20:
            return self.nearest_neighbor()
        
        # Create city index mapping
        city_to_idx = {city: i for i, city in enumerate(self.cities)}
        
        # dp[mask][i] = minimum cost to visit cities in mask ending at city i
        dp = [[float('inf')] * n for _ in range(1 << n)]
        parent = [[None] * n for _ in range(1 << n)]
        
        # Start from city 0
        dp[1][0] = 0
        
        # Iterate through all subsets
        for mask in range(1, 1 << n):
            for last in range(n):
                if not (mask & (1 << last)):
                    continue
                
                if dp[mask][last] == float('inf'):
                    continue
                
                # Try to extend to next city
                for next_city in range(n):
                    if mask & (1 << next_city):
                        continue
                    
                    next_mask = mask | (1 << next_city)
                    dist = self.get_distance(self.cities[last], self.cities[next_city])
                    new_cost = dp[mask][last] + dist
                    
                    if new_cost < dp[next_mask][next_city]:
                        dp[next_mask][next_city] = new_cost
                        parent[next_mask][next_city] = last
        
        # Find best ending city
        full_mask = (1 << n) - 1
        best_cost = float('inf')
        best_last = -1
        
        for last in range(n):
            # Add cost to return to start
            cost = dp[full_mask][last] + self.get_distance(self.cities[last], self.cities[0])
            if cost < best_cost:
                best_cost = cost
                best_last = last
        
        # Reconstruct path
        path = []
        if best_last != -1:
            mask = full_mask
            current = best_last
            
            while current != 0:
                path.append(self.cities[current])
                prev_mask = mask ^ (1 << current)
                current = parent[mask][current]
                mask = prev_mask
            
            path.append(self.cities[0])
            path.reverse()

        return path, best_cost
    
    def two_opt(self, tour):
        """
        2-opt improvement heuristic
        Can be applied to improve any tour
        """
        improved = True
        best_tour = tour.copy()
        best_cost = self.calculate_path_cost(best_tour)
        
        while improved:
            improved = False
            
            for i in range(1, len(tour) - 2):
                for j in range(i + 1, len(tour)):
                    if j - i == 1:
                        continue
                    
                    new_tour = best_tour.copy()
                    new_tour[i:j] = reversed(new_tour[i:j])
                    
                    new_cost = self.calculate_path_cost(new_tour)
                    
                    if new_cost < best_cost:
                        best_tour = new_tour
                        best_cost = new_cost
                        improved = True
                        break
                
                if improved:
                    break
        
        return best_tour, best_cost