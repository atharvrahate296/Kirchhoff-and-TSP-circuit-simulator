
"""
Traveling Salesman Problem Simulator
Implements multiple algorithms: Nearest Neighbor, Genetic Algorithm, Dynamic Programming
"""

import tkinter as tk
from tkinter import ttk, messagebox
import math
import random
from typing import List, Tuple
from tsp_algorithms import TSPSolver

class City:
    """Represents a city in TSP"""
    def __init__(self, x, y, name):
        self.x = x
        self.y = y
        self.name = name
    
    def distance_to(self, other):
        """Calculate Euclidean distance to another city"""
        return math.sqrt((self.x - other.x)**2 + (self.y - other.y)**2)

class TSPSimulator:
    def __init__(self, parent):
        self.parent = parent
        self.cities = []
        self.city_counter = 0
        self.edges = {}  # (city1, city2): weight
        self.mode = "add_city"
        self.selected_city = None
        self.temp_line = None
        self.solution_path = []
        self.total_cost = 0
        
        self.setup_ui()
    
    def setup_ui(self):
        """Setup the user interface"""
        self.parent.configure(bg="#1c1c1c")

        # Main container
        main_container = tk.Frame(self.parent, bg="#1c1c1c")
        main_container.pack(fill=tk.BOTH, expand=True)

        # Left panel - Controls
        control_panel_bg = "#2b2b2b"
        control_panel = tk.Frame(main_container, bg=control_panel_bg, width=350, relief=tk.FLAT, bd=0)
        control_panel.pack(side=tk.LEFT, fill=tk.Y, padx=10, pady=10)
        control_panel.pack_propagate(False)

        # Mode selection
        tk.Label(
            control_panel, text="Mode Selection", font=("Montserrat", 14, "bold"), bg=control_panel_bg, fg="white"
        ).pack(pady=5)
        self.mode_var = tk.StringVar(value="add_city")
        modes = [("Add City", "add_city"), ("Add Edge (Custom)", "add_edge"), ("Delete City", "delete")]
        for text, mode in modes:
            tk.Radiobutton(
                control_panel, text=text, variable=self.mode_var, value=mode, font=("Montserrat", 10),
                bg=control_panel_bg, fg="#c0c0c0", selectcolor="#1c1c1c", activebackground=control_panel_bg, command=self.change_mode
            ).pack(anchor=tk.W, padx=20, pady=2)

        # Edge weight input
        tk.Label(
            control_panel, text="Edge Weight (for custom edges)", font=("Montserrat", 11, "bold"), bg=control_panel_bg, fg="white"
        ).pack(pady=(10, 2))
        self.weight_entry = tk.Entry(control_panel, font=("Montserrat", 10), width=10, bg="#3c3c3c", fg="#e0e0e0", relief=tk.FLAT)
        self.weight_entry.pack(pady=2)
        self.weight_entry.insert(0, "10")

        # Auto-connect option
        self.auto_connect_var = tk.BooleanVar(value=True)
        tk.Checkbutton(
            control_panel, text="Auto-connect all cities (Euclidean)", variable=self.auto_connect_var,
            font=("Arial", 8), bg=control_panel_bg, fg="#c0c0c0", selectcolor="#1c1c1c", activebackground=control_panel_bg
        ).pack(pady=2)

        # Algorithm selection
        tk.Label(
            control_panel, text="Algorithm Selection", font=("Montserrat", 12, "bold"), bg=control_panel_bg, fg="white"
        ).pack(pady=(15, 3))
        self.algo_var = tk.StringVar(value="nearest_neighbor")
        algorithms = [("Nearest Neighbor", "nearest_neighbor"), ("Genetic Algorithm", "genetic"), ("Dynamic Programming", "dynamic")]
        for text, algo in algorithms:
            tk.Radiobutton(
                control_panel, text=text, variable=self.algo_var, value=algo, font=("Montserrat", 9),
                bg=control_panel_bg, fg="#c0c0c0", selectcolor="#1c1c1c", activebackground=control_panel_bg
            ).pack(anchor=tk.W, padx=20, pady=2)

        # Action buttons
        solve_button = tk.Button(
            control_panel, text="Solve TSP", font=("Montserrat", 12, "bold"), bg="#4edf56", fg="black",
            command=self.solve_tsp, cursor="hand2", pady=4, relief=tk.FLAT, bd=0
        )
        solve_button.pack(pady=10, padx=20, fill=tk.X)
        solve_button.bind("<Enter>", lambda e: e.widget.config(bg="#48794A"))
        solve_button.bind("<Leave>", lambda e: e.widget.config(bg="#4edf56"))
        
        random_button = tk.Button(
            control_panel, text="Generate Random Cities", font=("Montserrat", 9), bg="#49a8d8", fg="#1c1c1c",
            command=self.generate_random_cities, cursor="hand2", pady=3, relief=tk.FLAT, bd=0
        )
        random_button.pack(padx=20, fill=tk.X, pady=3)
        random_button.bind("<Enter>", lambda e: e.widget.config(bg="#3A657B"))
        random_button.bind("<Leave>", lambda e: e.widget.config(bg="#49a8d8"))

        clear_button = tk.Button(
            control_panel, text="Clear All", font=("Montserrat", 9), bg="#eb5050", fg="#1c1c1c",
            command=self.clear_all, cursor="hand2", pady=3, relief=tk.FLAT, bd=0
        )
        clear_button.pack(padx=20, fill=tk.X)
        clear_button.bind("<Enter>", lambda e: e.widget.config(bg="#9B4C4C"))
        clear_button.bind("<Leave>", lambda e: e.widget.config(bg="#eb5050"))

        # Results display
        results_frame = tk.LabelFrame(
            control_panel, text="RESULTS", font=("Montserrat", 12, "bold"), bg=control_panel_bg, fg="white", relief=tk.FLAT
        )
        results_frame.pack(pady=15, padx=10, fill=tk.BOTH, expand=True)
        
        self.results_text = tk.Text(
            results_frame, font=("Arial", 10), height=30, width=35, wrap=tk.WORD,
            bg="#1c1c1c", fg="white", relief=tk.FLAT, bd=0
        )
        scrollbar = tk.Scrollbar(results_frame, command=self.results_text.yview, relief=tk.FLAT)
        self.results_text.configure(yscrollcommand=scrollbar.set)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        self.results_text.pack(padx=5, pady=5, fill=tk.BOTH, expand=True)

        # Right panel - Canvas
        canvas_frame = tk.Frame(main_container, bg="#2b2b2b", relief=tk.FLAT, bd=0)
        canvas_frame.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        tk.Label(
            canvas_frame, text="TSP Canvas - Click to add cities", font=("Montserrat", 10), bg="#2b2b2b", fg="#d4d2d2"
        ).pack(pady=5)
        
        self.canvas = tk.Canvas(
            canvas_frame, bg="white", highlightthickness=0
        )
        self.canvas.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        self.canvas.bind("<Button-1>", self.canvas_click)
        self.canvas.bind("<Motion>", self.canvas_motion)
    
    def change_mode(self):
        """Change interaction mode"""
        self.mode = self.mode_var.get()
        self.selected_city = None
        if self.temp_line:
            self.canvas.delete(self.temp_line)
            self.temp_line = None
    
    def canvas_click(self, event):
        """Handle canvas click events"""
        if self.mode == "add_city":
            self.add_city(event.x, event.y)
        elif self.mode == "add_edge":
            self.handle_edge_click(event.x, event.y)
        elif self.mode == "delete":
            self.delete_at_position(event.x, event.y)
    
    def canvas_motion(self, event):
        """Handle mouse motion for visual feedback"""
        if self.selected_city is not None and self.mode == "add_edge":
            if self.temp_line:
                self.canvas.delete(self.temp_line)
            self.temp_line = self.canvas.create_line(
                self.selected_city.x, self.selected_city.y,
                event.x, event.y,
                fill="gray",
                dash=(4, 4),
                width=2
            )
    
    def add_city(self, x, y):
        """Add a city to the map"""
        city = City(x, y, f"C{self.city_counter}")
        self.cities.append(city)
        self.city_counter += 1
        
        # Auto-connect to existing cities if enabled
        if self.auto_connect_var.get() and len(self.cities) > 1:
            for other in self.cities[:-1]:
                dist = city.distance_to(other)
                self.edges[(city, other)] = dist
                self.edges[(other, city)] = dist
        
        self.redraw_map()
    
    def handle_edge_click(self, x, y):
        """Handle clicks for adding custom edges"""
        nearest = self.find_nearest_city(x, y)
        
        if nearest is None:
            return
        
        if self.selected_city is None:
            self.selected_city = nearest
            self.highlight_city(nearest, "green")
        else:
            if nearest != self.selected_city:
                try:
                    weight = float(self.weight_entry.get())
                    self.edges[(self.selected_city, nearest)] = weight
                    self.edges[(nearest, self.selected_city)] = weight
                    self.redraw_map()
                except ValueError:
                    messagebox.showerror("Error", "Invalid weight value")
            
            self.canvas.delete("temp_highlight")
            self.selected_city = None
            if self.temp_line:
                self.canvas.delete(self.temp_line)
                self.temp_line = None
    
    def find_nearest_city(self, x, y):
        """Find nearest city to coordinates"""
        nearest = None
        min_dist = float('inf')
        
        for city in self.cities:
            dist = math.sqrt((x - city.x)**2 + (y - city.y)**2)
            if dist < 25 and dist < min_dist:
                min_dist = dist
                nearest = city
        
        return nearest
    
    def highlight_city(self, city, color):
        """Highlight a city"""
        self.canvas.create_oval(
            city.x-15, city.y-15, city.x+15, city.y+15,
            outline=color,
            width=4,
            tags="temp_highlight"
        )
    
    def delete_at_position(self, x, y):
        """Delete city at position"""
        city = self.find_nearest_city(x, y)
        if city:
            # Remove edges
            self.edges = {k: v for k, v in self.edges.items() 
                         if city not in k}
            self.cities.remove(city)
            self.redraw_map()
    
    def generate_random_cities(self):
        """Generate random cities"""
        self.clear_all()
        
        width = self.canvas.winfo_width()
        height = self.canvas.winfo_height()
        
        if width <= 1:
            width = 800
        if height <= 1:
            height = 600
        
        num_cities = random.randint(3, 8)
        
        for _ in range(num_cities):
            x = random.randint(50, width - 50)
            y = random.randint(50, height - 50)
            self.add_city(x, y)
    
    def solve_tsp(self):
        """Solve TSP using selected algorithm"""
        if len(self.cities) < 2:
            messagebox.showwarning("Warning", "Add at least 2 cities")
            return
        
        if len(self.edges) == 0:
            messagebox.showwarning("Warning", "No edges defined. Enable auto-connect or add edges manually")
            return
        
        algorithm = self.algo_var.get()
        solver = TSPSolver(self.cities, self.edges)
        
        self.results_text.delete(1.0, tk.END)
        self.results_text.insert(tk.END, f"Solving with {algorithm}...\n\n")
        self.parent.update()
        
        if algorithm == "nearest_neighbor":
            path, cost = solver.nearest_neighbor()
            algo_name = "Nearest Neighbor"
        elif algorithm == "genetic":
            path, cost = solver.genetic_algorithm()
            algo_name = "Genetic Algorithm"
        else:  # dynamic
            if len(self.cities) > 15:
                messagebox.showwarning(
                    "Warning", 
                    "Dynamic Programming may be slow for >15 cities. Consider using another algorithm."
                )
            path, cost = solver.dynamic_programming()
            algo_name = "Dynamic Programming"
        
        self.solution_path = path
        self.total_cost = cost
        
        # Display results
        results = f"=== {algo_name} ===\n\n"
        results += f"Total Distance: {cost:.2f}\n\n"
        results += "Path Sequence:\n"
        
        for i, city in enumerate(path):
            results += f"{i+1}. {city.name} "
            if i < len(path) - 1:
                edge_dist = self.edges.get((city, path[i+1]), 0)
                results += f"→ ({edge_dist:.1f}) →\n"
        
        results += f"\nReturn to start: {path[-1].name} → {path[0].name}"
        results += f" ({self.edges.get((path[-1], path[0]), 0):.1f})\n"
        
        results += "\n" + "="*28 + "\n"
        results += f"Cities Visited: {len(path)}\n"
        results += f"Total Edges: {len(path)}\n"
        
        self.results_text.delete(1.0, tk.END)
        self.results_text.insert(1.0, results)
        
        # Draw solution
        self.draw_solution()
    
    def draw_solution(self):
        """Draw the solution path on canvas"""
        self.canvas.delete("solution")
        
        if len(self.solution_path) < 2:
            return
        
        # Draw path
        for i in range(len(self.solution_path)):
            city1 = self.solution_path[i]
            city2 = self.solution_path[(i + 1) % len(self.solution_path)]
            
            self.canvas.create_line(
                city1.x, city1.y, city2.x, city2.y,
                fill="#e57373",
                width=2,
                arrow=tk.LAST,
                tags="solution"
            )
            
            # Draw step number
            mid_x = (city1.x + city2.x) / 2
            mid_y = (city1.y + city2.y) / 2
            
            self.canvas.create_text(
                mid_x, mid_y,
                text=str(i + 1),
                font=("Times New Roman", 10, "bold"),
                fill="#1c1c1c",
                tags="solution"
            )
        
        # Highlight start city
        start = self.solution_path[0]
        self.canvas.create_oval(
            start.x-20, start.y-20, start.x+20, start.y+20,
            outline="#81c784",
            width=4,
            tags="solution"
        )
        self.canvas.create_text(
            start.x, start.y + 35,
            text="START",
            font=("Times New Roman", 11, "bold"),
            fill="#81c784",
            tags="solution"
        )
    
    def redraw_map(self):
        """Redraw entire map"""
        self.canvas.delete("all")
        
        # Draw edges first
        drawn_edges = set()
        for (city1, city2), weight in self.edges.items():
            edge = tuple(sorted([id(city1), id(city2)]))
            if edge not in drawn_edges:
                self.canvas.create_line(
                    city1.x, city1.y, city2.x, city2.y,
                    fill="#000000",
                    width=2
                )
                
                # Draw weight
                mid_x = (city1.x + city2.x) / 2
                mid_y = (city1.y + city2.y) / 2
                self.canvas.create_text(
                    mid_x, mid_y,
                    text=f"{weight:.0f}",
                    font=("Times New Roman", 11),
                    fill="#000000"
                )
                
                drawn_edges.add(edge)
        
        # Draw cities
        for city in self.cities:
            r = 12
            self.canvas.create_oval(
                city.x-r, city.y-r, city.x+r, city.y+r,
                fill="#81c784",
                outline="#558b2f",
                width=2
            )
            self.canvas.create_text(
                city.x, city.y,
                text=city.name,
                font=("Times New Roman", 12, "bold"),
                fill="#1c1c1c"
            )
    
    def clear_all(self):
        """Clear all cities and edges"""
        self.cities.clear()
        self.edges.clear()
        self.city_counter = 0
        self.selected_city = None
        self.solution_path = []
        self.canvas.delete("all")
        self.results_text.delete(1.0, tk.END)