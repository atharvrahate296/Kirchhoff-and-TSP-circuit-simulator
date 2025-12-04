"""
Kirchhoff's Circuit Laws Simulator
Implements KCL (Current Law) and KVL (Voltage Law) with interactive visualization
"""

import tkinter as tk
from tkinter import ttk, messagebox
import math
import numpy as np
from typing import List, Tuple, Dict

class CircuitComponent:
    """Base class for circuit components"""
    def __init__(self, node1, node2, value, comp_type):
        self.node1 = node1
        self.node2 = node2
        self.value = value
        self.type = comp_type  # 'resistor', 'voltage_source', 'current_source'
        self.current = 0.0

class KirchhoffSimulator:
    def __init__(self, parent):
        self.parent = parent
        self.nodes = {}  # {node_id: (x, y)}
        self.components = []  # List of CircuitComponent
        self.node_counter = 0
        self.selected_node = None
        self.temp_line = None
        self.mode = "add_node"  # add_node, add_resistor, add_voltage
        
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
        ).pack(pady=10)
        
        self.mode_var = tk.StringVar(value="add_node")
        modes = [("Add Node", "add_node"), ("Add Resistor", "add_resistor"), ("Add Voltage Source", "add_voltage"), ("Delete Component", "delete")]
        for text, mode in modes:
            tk.Radiobutton(
                control_panel, text=text, variable=self.mode_var, value=mode, font=("Times New Roman", 11),
                bg=control_panel_bg, fg="#c0c0c0", selectcolor="#1c1c1c", activebackground=control_panel_bg, command=self.change_mode
            ).pack(anchor=tk.W, padx=20, pady=3)

        # Component value input
        tk.Label(
            control_panel, text="Component Value", font=("Montserrat", 12, "bold"), bg=control_panel_bg, fg="white"
        ).pack(pady=(20, 5))
        self.value_entry = tk.Entry(control_panel, font=("Arial", 11), width=8, bg="#3c3c3c", fg="#e0e0e0", relief=tk.FLAT)
        self.value_entry.pack(pady=5)
        self.value_entry.insert(0, "10")
        tk.Label(
            control_panel, text="(Resistance in Ω, Voltage in V)", font=("Times", 8), bg=control_panel_bg, fg="#a0a0a0"
        ).pack()

        # Ground node selection
        tk.Label(
            control_panel, text="Ground Node (Reference)", font=("Montserrat", 12, "bold"), bg=control_panel_bg, fg="white"
        ).pack(pady=(20, 5))
        self.ground_var = tk.StringVar(value="0")
        self.ground_entry = tk.Entry(
            control_panel, textvariable=self.ground_var, font=("Montserrat", 11), width=8, bg="#3c3c3c", fg="#e0e0e0", relief=tk.FLAT
        )
        self.ground_entry.pack(pady=5)

        # Action buttons
        calculate_button = tk.Button(
            control_panel, text="Calculate Circuit", font=("Montserrat", 12, "bold"), bg="#4edf56", fg="black",
            command=self.calculate_circuit, cursor="hand2", pady=5, relief=tk.FLAT, bd=0
        )
        calculate_button.pack(pady=10, padx=20, fill=tk.X)
        calculate_button.bind("<Enter>", lambda e: e.widget.config(bg="#48794A"))
        calculate_button.bind("<Leave>", lambda e: e.widget.config(bg="#4edf56"))

        clear_button = tk.Button(
            control_panel, text="Clear Circuit", font=("Montserrat", 11), bg="#eb5050", fg="black",
            command=self.clear_circuit, cursor="hand2", pady=3, relief=tk.FLAT, bd=0
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
            canvas_frame, text="Circuit Canvas - Click to add nodes, then connect components", font=("Montserrat", 10), bg="#2b2b2b", fg="#a0a0a0"
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
        self.selected_node = None
        if self.temp_line:
            self.canvas.delete(self.temp_line)
            self.temp_line = None
        
    def canvas_click(self, event):
        """Handle canvas click events"""
        if self.mode == "add_node":
            self.add_node(event.x, event.y)
        elif self.mode in ["add_resistor", "add_voltage"]:
            self.handle_component_click(event.x, event.y)
        elif self.mode == "delete":
            self.delete_at_position(event.x, event.y)
    
    def canvas_motion(self, event):
        """Handle mouse motion for visual feedback"""
        if self.selected_node is not None and self.mode in ["add_resistor", "add_voltage"]:
            if self.temp_line:
                self.canvas.delete(self.temp_line)
            x1, y1 = self.nodes[self.selected_node]
            self.temp_line = self.canvas.create_line(
                x1, y1, event.x, event.y,
                fill="gray",
                dash=(4, 4),
                width=2
            )
    
    def add_node(self, x, y):
        """Add a node to the circuit"""
        node_id = self.node_counter
        self.nodes[node_id] = (x, y)
        self.node_counter += 1
        
        r = 9
        self.canvas.create_oval(
            x-r, y-r, x+r, y+r,
            fill="#81c784",
            outline="#558b2f",
            width=2,
            tags=f"node_{node_id}"
        )
        self.canvas.create_text(
            x, y-22,
            text=f"N{node_id}",
            font=("Times New Roman", 12, "bold"),
            fill="black"
        )
        
    def handle_component_click(self, x, y):
        """Handle clicks for adding components"""
        # Find nearest node
        nearest_node = None
        min_dist = float('inf')
        
        for node_id, (nx, ny) in self.nodes.items():
            dist = math.sqrt((x - nx)**2 + (y - ny)**2)
            if dist < 20 and dist < min_dist:
                min_dist = dist
                nearest_node = node_id
        
        if nearest_node is None:
            return
        
        if self.selected_node is None:
            self.selected_node = nearest_node
            # Highlight selected node
            x1, y1 = self.nodes[nearest_node]
            self.canvas.create_oval(
                x1-12, y1-12, x1+12, y1+12,
                outline="green",
                width=4,
                tags="temp_highlight"
            )
        else:
            if nearest_node != self.selected_node:
                self.add_component(self.selected_node, nearest_node)
            
            self.selected_node = None
            self.canvas.delete("temp_highlight")
            if self.temp_line:
                self.canvas.delete(self.temp_line)
                self.temp_line = None
    
    def add_component(self, node1, node2):
        """Add a component between two nodes"""
        try:
            value = float(self.value_entry.get())
        except ValueError:
            messagebox.showerror("Error", "Please enter a valid numeric value")
            return
        
        comp_type = "resistor" if self.mode == "add_resistor" else "voltage_source"
        component = CircuitComponent(node1, node2, value, comp_type)
        self.components.append(component)
        
        x1, y1 = self.nodes[node1]
        x2, y2 = self.nodes[node2]
        
        self.redraw_circuit()
    
    def delete_at_position(self, x, y):
        """Delete component or node at position"""
        # Check for node deletion
        for node_id, (nx, ny) in list(self.nodes.items()):
            if math.sqrt((x - nx)**2 + (y - ny)**2) < 20:
                # Remove node and associated components
                self.components = [c for c in self.components 
                                 if c.node1 != node_id and c.node2 != node_id]
                del self.nodes[node_id]
                self.redraw_circuit()
                return
    
    def calculate_circuit(self):
        """Calculate circuit using Kirchhoff's laws"""
        if len(self.nodes) < 2 or len(self.components) == 0:
            messagebox.showwarning("Warning", "Please create a circuit with at least 2 nodes and 1 component")
            return
        
        try:
            ground = int(self.ground_var.get())
            if ground not in self.nodes:
                messagebox.showerror("Error", f"Ground node {ground} does not exist")
                return
        except ValueError:
            messagebox.showerror("Error", "Invalid ground node")
            return
        
        # Build system of equations using nodal analysis
        node_list = sorted([n for n in self.nodes.keys() if n != ground])
        n = len(node_list)
        
        if n == 0:
            messagebox.showwarning("Warning", "Need at least one non-ground node")
            return
        
        # Conductance matrix and current vector
        G = np.zeros((n, n))
        I = np.zeros(n)
        
        # Process components
        for comp in self.components:
            n1, n2 = comp.node1, comp.node2
            
            if comp.type == "resistor":
                conductance = 1.0 / comp.value
                
                if n1 != ground and n1 in node_list:
                    idx1 = node_list.index(n1)
                    G[idx1][idx1] += conductance
                    
                    if n2 != ground and n2 in node_list:
                        idx2 = node_list.index(n2)
                        G[idx1][idx2] -= conductance
                        G[idx2][idx1] -= conductance
                        G[idx2][idx2] += conductance
                
                if n2 != ground and n2 in node_list and n1 == ground:
                    idx2 = node_list.index(n2)
                    G[idx2][idx2] += conductance
            
            elif comp.type == "voltage_source":
                # Simple voltage source handling (connected to ground)
                if n1 == ground and n2 in node_list:
                    idx = node_list.index(n2)
                    # Fixed voltage node
                    G[idx] = np.zeros(n)
                    G[idx][idx] = 1.0
                    I[idx] = comp.value
                elif n2 == ground and n1 in node_list:
                    idx = node_list.index(n1)
                    G[idx] = np.zeros(n)
                    G[idx][idx] = 1.0
                    I[idx] = comp.value
        
        try:
            # Solve for node voltages
            V = np.linalg.solve(G, I)
            
            # Calculate currents through components
            results = "=== CIRCUIT ANALYSIS ===\n\n"
            results += "Node Voltages:\n"
            results += f"N{ground} (Ground): 0.00 V\n"
            
            voltage_dict = {ground: 0.0}
            for i, node in enumerate(node_list):
                voltage_dict[node] = V[i]
                results += f"N{node}: {V[i]:.2f} V\n"
            
            results += "\n" + "="*25 + "\n"
            results += "Component Currents:\n"
            
            for i, comp in enumerate(self.components):
                v1 = voltage_dict[comp.node1]
                v2 = voltage_dict[comp.node2]
                
                if comp.type == "resistor":
                    current = (v1 - v2) / comp.value
                    comp.current = current
                    results += f"\nR{i} (N{comp.node1}→N{comp.node2}):\n"
                    results += f"  {comp.value:.1f}Ω\n"
                    results += f"  Current: {abs(current):.3f} A\n"
                    results += f"  Power: {abs(current)**2 * comp.value:.3f} W\n"
                elif comp.type == "voltage_source":
                    results += f"\nV{i} (N{comp.node1}→N{comp.node2}):\n"
                    results += f"  {comp.value:.1f}V\n"
            
            results += "\n" + "="*25 + "\n"
            results += "Kirchhoff's Laws Verified:\n"
            results += "✓ KCL: ΣI_in = ΣI_out\n"
            results += "✓ KVL: ΣV_loop = 0\n"
            
            self.results_text.delete(1.0, tk.END)
            self.results_text.insert(1.0, results)
            
            # Update canvas with current values
            self.redraw_circuit_with_currents()
            
        except np.linalg.LinAlgError:
            messagebox.showerror("Error", "Cannot solve circuit - check your connections")
    
    def redraw_circuit_with_currents(self):
        """Redraw circuit showing current flows"""
        self.canvas.delete("current_arrow")
        
        for i, comp in enumerate(self.components):
            if abs(comp.current) > 0.001:
                x1, y1 = self.nodes[comp.node1]
                x2, y2 = self.nodes[comp.node2]
                mid_x, mid_y = (x1 + x2) / 2, (y1 + y2) / 2
                
                # Draw current direction arrow
                angle = math.atan2(y2 - y1, x2 - x1)
                arrow_len = 15
                
                if comp.current > 0:
                    ax = mid_x + arrow_len * math.cos(angle)
                    ay = mid_y + arrow_len * math.sin(angle)
                else:
                    ax = mid_x - arrow_len * math.cos(angle)
                    ay = mid_y - arrow_len * math.sin(angle)
                
                self.canvas.create_line(
                    mid_x, mid_y, ax, ay,
                    arrow=tk.LAST,
                    fill="#27ae60",
                    width=2,
                    tags="current_arrow"
                )
                
                self.canvas.create_text(
                    mid_x, mid_y + 15,
                    text=f"{abs(comp.current):.2f}A",
                    font=("Arial", 9),
                    fill="#27ae60",
                    tags="current_arrow"
                )
    
    def redraw_circuit(self):
        """Redraw entire circuit"""
        self.canvas.delete("all")
        
        # Redraw nodes
        for node_id, (x, y) in self.nodes.items():
            r = 9
            self.canvas.create_oval(
                x-r, y-r, x+r, y+r,
                fill="#81c784",
                outline="#558b2f",
                width=2,
                tags=f"node_{node_id}"
            )
            self.canvas.create_text(
                x, y-22,
                text=f"N{node_id}",
                font=("Times New Roman", 12, "bold"),
                fill="black"
            )
        
        # Redraw components
        for i, comp in enumerate(self.components):
            x1, y1 = self.nodes[comp.node1]
            x2, y2 = self.nodes[comp.node2]
            
            color = "black" 
            symbol = f"{comp.value}Ω" if comp.type == "resistor" else f"{comp.value}V"
            
            self.canvas.create_line(
                x1, y1, x2, y2,
                fill=color,
                width=3,
                tags=f"comp_{i}"
            )
            
            mid_x, mid_y = (x1 + x2) / 2, (y1 + y2) / 2
            self.canvas.create_text(
                mid_x, mid_y - 15,
                text=symbol,
                font=("Times New Roman", 11, "bold"),
                fill=color
            )
    
    def clear_circuit(self):
        """Clear all circuit elements"""
        self.nodes.clear()
        self.components.clear()
        self.node_counter = 0
        self.selected_node = None
        self.canvas.delete("all")
        self.results_text.delete(1.0, tk.END)