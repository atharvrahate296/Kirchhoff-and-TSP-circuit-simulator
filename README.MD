# Shortest Path Visualizer: Kirchhoff's Laws & TSP Simulator

## 1. Project Overview

This interactive application provides a hands-on environment for visualizing and solving two classic problems rooted in graph theory: **Kirchhoff's Circuit Laws** and the **Traveling Salesman Problem (TSP)**. Built with Python and Tkinter, it serves as an educational tool to understand complex algorithms and electrical principles through a user-friendly graphical interface.

---

## 2. Functionalities & Concepts

The application is split into two main modules:

### A. Kirchhoff's Circuit Simulator
An intuitive tool for designing and analyzing electrical circuits.

**Functionalities:**
- **Build interactively**: Add nodes, resistors, and voltage sources directly on a canvas.
- **Set a ground reference**: Designate any node as the 0V reference point.
- **Solve the circuit**: Automatically calculates all node voltages and the current through each component using nodal analysis.
- **Visualize results**: Displays currents, voltages, and power dissipation, with arrows indicating the direction of current flow.

**Key Concepts Demonstrated:**
- **Kirchhoff's Current Law (KCL)**: The sum of currents entering a node is zero.
- **Kirchhoff's Voltage Law (KVL)**: The sum of voltages in any closed loop is zero.
- **Nodal Analysis**: A systematic method for solving a circuit's voltages and currents by forming a system of linear equations.

### B. Traveling Salesman Problem (TSP) Simulator
A visual platform for exploring and comparing algorithms that solve the TSP.

**Functionalities:**
- **Create city layouts**: Place cities on a map with a click or generate them randomly.
- **Define connections**: Automatically connect all cities based on Euclidean distance or add custom-weighted edges manually.
- **Solve with multiple algorithms**:
    - **Nearest Neighbor**: A fast, greedy algorithm.
    - **Genetic Algorithm**: An evolutionary approach that finds high-quality solutions.
    - **Dynamic Programming**: The Held-Karp algorithm, which guarantees the optimal solution.
- **Visualize the optimal path**: The solution is drawn on the canvas, showing the exact route and total distance.

**Key Concepts Demonstrated:**
- **Graph Theory**: Representing cities as nodes and paths as weighted edges.
- **Algorithmic Complexity**: Observing the trade-offs between speed and optimality (e.g., O(n²) vs. O(n² * 2ⁿ)).
- **Heuristics vs. Optimal Solutions**: Comparing the approximate results of greedy/genetic algorithms with the guaranteed best result from dynamic programming.

---

## 3. File Structure

The project is organized into four main Python files:

-   `Main.py`: The entry point of the application. It launches the main menu where you can select either the Kirchhoff or TSP simulator.
-   `kirchhoff_module.py`: Contains the entire user interface and logic for the circuit simulator.
-   `tsp_module.py`: Contains the user interface and logic for the TSP simulator.
-   `tsp_algorithms.py`: Implements the core TSP-solving algorithms (Nearest Neighbor, Genetic, Dynamic Programming), which are called by `tsp_module.py`.

---

## 4. Installation & How to Run

Follow these simple steps to get the application running on your local machine.

**Step 1: Prerequisites**
- Ensure you have **Python 3.7** or a newer version installed.
- You will need the **NumPy** library for the circuit calculations.

**Step 2: Download the Code**
Clone the repository to your local machine:
```bash
git clone https://github.com/atharvrahate2d6/Kirchhoff-and-TSP-circuit-simulator.git
cd Kirchhoff-and-TSP-circuit-simulator
```

**Step 3: Install Dependencies**
Install the dependencies using pip:
```bash
pip install -r requirements.txt
```
*(Note: Tkinter is included with most standard Python installations and does not require a separate install.)*

**Step 4: Run the Application**
Execute the main script from your terminal:
```bash
python Main.py
```

This will open the main menu, allowing you to launch either of the simulators.

## Contribution

Contributions are welcomed! If you'd like to contribute, please follow these steps:

1.  Fork the repository.
2.  Create a new branch for your feature or bugfix (`git checkout -b feature/your-feature-name`).
3.  Commit your changes with clear and descriptive messages.
4.  Push your changes to your fork.
5.  Submit a pull request to the main repository.

## License

This project is for educational purposes and is not licensed for commercial use.