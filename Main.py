"""
Shortest Path Applications - Main Entry Point
A visualization tool for Kirchhoff's Circuit Laws and Traveling Salesman Problem
"""

import tkinter as tk
from tkinter import ttk
from kirchhoff_module import KirchhoffSimulator
from tsp_module import TSPSimulator

class ShortestPathApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Shortest Path Applications - Kirchhoff's Laws & TSP")
        self.root.geometry("1300x725+120+30")
        self.root.configure(bg="#f0f0f0")
        
        # Create main container
        self.create_main_menu()
        
    def create_main_menu(self):
        """Create the main selection menu"""
        for widget in self.root.winfo_children():
            widget.destroy()

        self.root.configure(bg="#1c1c1c")

        # Title Frame
        title_frame = tk.Frame(self.root, bg="#1c1c1c", height=100)
        title_frame.pack(fill=tk.X, pady=(20, 10))

        title_label = tk.Label(
            title_frame,
            text="Shortest Path Visualizer",
            font=("Montserrat", 32, "bold"),
            fg="#e0e0e0",
            bg="#1c1c1c"
        )
        title_label.pack()

        subtitle_label = tk.Label(
            title_frame,
            text="An Interactive Toolkit for Kirchhoff's Laws & TSP",
            font=("Montserrat", 14),
            fg="#a0a0a0",
            bg="#1c1c1c"
        )
        subtitle_label.pack(pady=5)

        # Main content frame
        content_frame = tk.Frame(self.root, bg="#1c1c1c")
        content_frame.pack(expand=True, fill=tk.BOTH, padx=50, pady=20)

        # Kirchhoff's Laws Section
        kirchhoff_frame = tk.Frame(content_frame, bg="#2b2b2b", relief=tk.FLAT, bd=0)
        kirchhoff_frame.pack(side=tk.LEFT, expand=True, fill=tk.BOTH, padx=20)
        
        tk.Label(
            kirchhoff_frame,
            text="Kirchhoff's Circuit Laws",
            font=("Montserrat", 22, "bold"),
            bg="#2b2b2b",
            fg="#e57373"
        ).pack(pady=(30, 15))

        tk.Label(
            kirchhoff_frame,
            text="Visualize and solve electrical circuits.\n\n• Kirchhoff's Current Law (KCL)\n• Kirchhoff's Voltage Law (KVL)\n\nCreate custom circuits, compute currents and voltages.",
            font=("Times New Roman", 14),
            bg="#2b2b2b",
            fg="#c0c0c0",
            justify=tk.LEFT
        ).pack(pady=20, padx=30, anchor="w")

        kirchhoff_button = tk.Button(
            kirchhoff_frame,
            text="Launch Kirchhoff Simulator",
            font=("Montserrat", 14, "bold"),
            bg="#e57373",
            fg="#1c1c1c",
            activebackground="#ef9a9a",
            activeforeground="#1c1c1c",
            command=self.launch_kirchhoff,
            cursor="hand2",
            relief=tk.FLAT,
            padx=20,
            pady=10,
            bd=0
        )
        kirchhoff_button.pack(pady=30)
        kirchhoff_button.bind("<Enter>", lambda e: e.widget.config(bg="#ef9a9a"))
        kirchhoff_button.bind("<Leave>", lambda e: e.widget.config(bg="#e57373"))
        
        # TSP Section
        tsp_frame = tk.Frame(content_frame, bg="#2b2b2b", relief=tk.FLAT, bd=0)
        tsp_frame.pack(side=tk.RIGHT, expand=True, fill=tk.BOTH, padx=20)

        tk.Label(
            tsp_frame,
            text="Traveling Salesman Problem",
            font=("Montserrat", 22, "bold"),
            bg="#2b2b2b",
            fg="#81c784"
        ).pack(pady=(30, 15))

        tk.Label(
            tsp_frame,
            text="Solve TSP using multiple algorithms.\n\n• Nearest Neighbor\n• Genetic Algorithm\n• Dynamic Programming\n\nDesign custom city layouts and find the optimal route.",
            font=("Times New Roman", 14),
            bg="#2b2b2b",
            fg="#c0c0c0",
            justify=tk.LEFT
        ).pack(pady=20, padx=30, anchor="w")

        tsp_button = tk.Button(
            tsp_frame,
            text="Launch TSP Simulator",
            font=("Montserrat", 14, "bold"),
            bg="#81c784",
            fg="#1c1c1c",
            activebackground="#a5d6a7",
            activeforeground="#1c1c1c",
            command=self.launch_tsp,
            cursor="hand2",
            relief=tk.FLAT,
            padx=20,
            pady=10,
            bd=0
        )
        tsp_button.pack(pady=30)
        tsp_button.bind("<Enter>", lambda e: e.widget.config(bg="#a5d6a7"))
        tsp_button.bind("<Leave>", lambda e: e.widget.config(bg="#81c784"))

        # Footer
        footer = tk.Label(
            self.root,
            text="Fundamentals of Data Structures - Course Project",
            font=("Montserrat", 10),
            bg="#1c1c1c",
            fg="#707070"
        )
        footer.pack(side=tk.BOTTOM, pady=10)
        
    def launch_kirchhoff(self):
        """Launch Kirchhoff's Circuit Simulator"""
        for widget in self.root.winfo_children():
            widget.destroy()

        back_frame = tk.Frame(self.root, bg="#2b2b2b")
        back_frame.pack(fill=tk.X)

        back_button = tk.Button(
            back_frame,
            text="← Main Menu",
            font=("Times New Roman", 10,"bold"),
            bg="#2b2b2b",
            fg="white",
            command=self.create_main_menu,
            cursor="hand2",
            padx=15,
            pady=5,
            relief=tk.SUNKEN,
            bd=0
        )
        back_button.pack(side=tk.LEFT, padx=10, pady=5)
        back_button.bind("<Enter>", lambda e: e.widget.config(bg="#3c3c3c"))
        back_button.bind("<Leave>", lambda e: e.widget.config(bg="#2b2b2b"))

        fr_title = tk.Label(
            back_frame,
            text="Circuit Simulator",
            font=("Montserrat", 18, "bold"),
            bg="#2b2b2b",
            fg="#e57373"
        )
        fr_title.pack(pady=10)
        
        KirchhoffSimulator(self.root)
        
    def launch_tsp(self):
        """Launch TSP Simulator"""
        for widget in self.root.winfo_children():
            widget.destroy()

        back_frame = tk.Frame(self.root, bg="#2b2b2b")
        back_frame.pack(fill=tk.X)

        back_button = tk.Button(
            back_frame,
            text="← Main Menu",
            font=("Times New Roman", 10,"bold"),
            bg="#2b2b2b",
            fg="white",
            command=self.create_main_menu,
            cursor="hand2",
            padx=15,
            pady=5,
            relief=tk.SUNKEN,
            bd=0
        )
        back_button.pack(side=tk.LEFT, padx=10, pady=5)
        back_button.bind("<Enter>", lambda e: e.widget.config(bg="#3c3c3c"))
        back_button.bind("<Leave>", lambda e: e.widget.config(bg="#2b2b2b"))

        fr_title = tk.Label(
            back_frame,
            text="TSP Simulator",
            font=("Montserrat", 18, "bold"),
            bg="#2b2b2b",
            fg="#81c784"
        )
        fr_title.pack(pady=10)
        
        TSPSimulator(self.root)

def main():
    root = tk.Tk()
    app = ShortestPathApp(root)
    root.mainloop()

if __name__ == "__main__":
    main()