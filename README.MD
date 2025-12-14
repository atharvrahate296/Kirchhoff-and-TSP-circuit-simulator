# Shortest Path Visualizer: Kirchhoff's Laws & TSP Simulator

## 1. Project Overview

This interactive application provides a hands-on environment for visualizing and solving two classic problems rooted in graph theory: **Kirchhoff's Circuit Laws** and the **Traveling Salesman Problem (TSP)**. Built with C and GTK+, it serves as an educational tool to understand complex algorithms and electrical principles through a user-friendly graphical interface with native performance.

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
- **Nodal Analysis**: A systematic method for solving a circuit's voltages and currents by forming a system of linear equations using Gaussian elimination with partial pivoting.

### B. Traveling Salesman Problem (TSP) Simulator
A visual platform for exploring and comparing algorithms that solve the TSP.

**Functionalities:**
- **Create city layouts**: Place cities on a map with a click or generate them randomly.
- **Define connections**: Automatically connect all cities based on Euclidean distance or add custom-weighted edges manually.
- **Solve with multiple algorithms**:
    - **Nearest Neighbor**: A fast, greedy algorithm (O(n²) complexity).
    - **Genetic Algorithm**: An evolutionary approach with tournament selection, order crossover, and swap mutation that finds high-quality solutions.
    - **Dynamic Programming**: The Held-Karp algorithm (O(n² * 2ⁿ) complexity), which guarantees the optimal solution.
- **Visualize the optimal path**: The solution is drawn on the canvas, showing the exact route, step numbers, and total distance.

**Key Concepts Demonstrated:**
- **Graph Theory**: Representing cities as nodes and paths as weighted edges.
- **Algorithmic Complexity**: Observing the trade-offs between speed and optimality (e.g., O(n²) vs. O(n² * 2ⁿ)).
- **Heuristics vs. Optimal Solutions**: Comparing the approximate results of greedy/genetic algorithms with the guaranteed best result from dynamic programming.

---

## 3. File Structure

The project is organized into multiple C source and header files:

-   **`main.c`**: The entry point of the application. It launches the main menu where you can select either the Kirchhoff or TSP simulator.
-   **`kirchoff.c`** & **`kirchhoff.h`**: Contains the entire user interface and logic for the circuit simulator, including the matrix solver for nodal analysis.
-   **`tsp.c`**: Contains the user interface and logic for the TSP simulator, handling city placement, edge management, and visualization.
-   **`tsp_algorithms.c`** & **`tsp_algorithm.h`**: Implements the core TSP-solving algorithms (Nearest Neighbor, Genetic Algorithm with proper crossover/mutation, and Dynamic Programming using Held-Karp), which are called by `tsp.c`.
-   **`makefile`**: Build configuration for the project.
-   **`README.md`**: Project documentation and instructions.

---

## 4. Installation & How to Run

### Prerequisites (Windows 10/11)

**Required Software:**
- **MSYS2**: MinGW compiler environment
- **GTK+ 3.0**: GUI toolkit with Cairo graphics library
- **Make**: Build automation tool

---

### Installation Instructions for Windows

**Step 1: Install MSYS2**
- Download from: https://www.msys2.org/
- Run the installer and install to `C:\msys64`
- Follow the installation wizard

**Step 2: Update MSYS2**
Open **"MSYS2 MinGW 64-bit"** terminal from the Start Menu and run:
```bash
pacman -Syu
```
Close and reopen the terminal, then run:
```bash
pacman -Su
```

**Step 3: Install Development Tools**
In the **MSYS2 MinGW 64-bit terminal**, run:
```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-gtk3 mingw-w64-x86_64-pkg-config base-devel
```

**Step 4: Navigate to Project**
In the **MSYS2 MinGW 64-bit terminal**, run:
```bash
cd {path_to_project_directory}
```

---

### Build & Run

**Step 5: Compile the Application**
In the **MSYS2 MinGW 64-bit terminal**, run:
```bash
make
```

**Step 6: Run the Application**
In the **MSYS2 MinGW 64-bit terminal**, run:
```bash
./run_main
```

**To clean build files:**
In the **MSYS2 MinGW 64-bit terminal**, run:
```bash
make clean
```

---

## 5. Usage Guide

### Kirchhoff's Circuit Simulator

1. **Launch the simulator** from the main menu
2. **Add nodes** by selecting "Add Node" mode and clicking on the canvas
3. **Connect components**:
   - Select "Add Resistor" or "Add Voltage Source"
   - Enter the component value (resistance in Ω or voltage in V)
   - Click on the first node, then click on the second node
4. **Set ground reference**: Enter the node ID (e.g., 0) in the ground node field
5. **Calculate**: Click "Calculate Circuit" to solve for all voltages and currents
6. **View results**: Node voltages, component currents, and power dissipation appear in the results panel

**Tips:**
- Always designate one node as ground (reference = 0V)
- Voltage sources should typically connect between ground and another node
- Current arrows show the magnitude and direction of current flow

### TSP Simulator

1. **Launch the simulator** from the main menu
2. **Add cities**:
   - Select "Add City" mode and click on the canvas, OR
   - Click "Generate Random Cities" for automatic placement
3. **Configure edges**:
   - Enable "Auto-connect all cities" for a complete graph with Euclidean distances, OR
   - Manually add custom-weighted edges
4. **Select algorithm**:
   - **Nearest Neighbor**: Fast approximate solution
   - **Genetic Algorithm**: Balanced speed and quality
   - **Dynamic Programming**: Optimal solution (recommended for ≤15 cities)
5. **Solve**: Click "Solve TSP" to find the shortest tour
6. **View results**: The path is visualized with step numbers, distances, and total cost

**Tips:**
- For more than 15 cities, use Nearest Neighbor or Genetic Algorithm
- Dynamic Programming guarantees the optimal solution but has exponential time complexity
- The START marker indicates the beginning of the tour

---

## 6. Algorithm Details

### Kirchhoff's Circuit Analysis
- **Method**: Nodal analysis with Gaussian elimination
- **Matrix Solver**: Partial pivoting for numerical stability
- **Complexity**: O(n³) where n = number of non-ground nodes
- **Handles**: Resistors and voltage sources with ground reference

### TSP Algorithms

#### Nearest Neighbor (Greedy)
- **Time Complexity**: O(n²)
- **Space Complexity**: O(n)
- **Type**: Heuristic (approximate solution)
- **Best For**: Quick results, large datasets

#### Genetic Algorithm
- **Population Size**: 100 individuals
- **Generations**: 500 iterations
- **Selection**: Tournament selection (size 5)
- **Crossover**: Order crossover (OX)
- **Mutation**: Swap mutation (1% rate)
- **Elitism**: Best individual preserved each generation
- **Type**: Metaheuristic (high-quality approximate solution)
- **Best For**: Balance between speed and solution quality

#### Dynamic Programming (Held-Karp)
- **Time Complexity**: O(n² * 2ⁿ)
- **Space Complexity**: O(n * 2ⁿ)
- **Type**: Exact algorithm (guaranteed optimal solution)
- **Best For**: Small instances (≤15 cities)
- **Implementation**: Bitmask DP with path reconstruction

---

## 7. Building from Source

### Manual Compilation (Linux)
```bash
gcc main.c tsp.c tsp_algorithms.c kirchhoff.c -o shortest_path \
    `pkg-config --cflags --libs gtk+-3.0` -lm -Wall -Wextra -O2
```

### Manual Compilation (Windows with MSYS2)
```bash
gcc main.c tsp.c tsp_algorithms.c kirchhoff.c -o shortest_path.exe \
    `pkg-config --cflags --libs gtk+-3.0` -lm -mwindows -Wall -Wextra -O2
```

### Using Makefile
```bash
# Compile
make

# Clean build artifacts
make clean

# Compile and run
make run
```

---

## 8. Troubleshooting

### Common Issues

**"gtk/gtk.h: No such file or directory"**
- **Solution**: Install GTK+ development libraries
  ```bash
  # Linux
  sudo apt install libgtk-3-dev
  
  # MSYS2
  pacman -S mingw-w64-x86_64-gtk3
  ```

**"Cannot solve circuit - check your connections"**
- Ensure all nodes are properly connected
- Verify at least one voltage source exists
- Check that ground node ID is correct

**Application window is blank**
- Resize the window to trigger a redraw
- Update graphics drivers
- Try software rendering: `GDK_RENDERING=image ./shortest_path`

**Dynamic Programming is very slow**
- This is expected for >15 cities due to O(n² * 2ⁿ) complexity
- Use Nearest Neighbor or Genetic Algorithm for larger instances

For detailed Windows-specific troubleshooting, see `WINDOWS_SETUP.md`.

---

## 9. Performance Considerations

### Memory Usage
- **TSP**: O(MAX_CITIES²) for edge matrix (~1.6 KB for 20 cities)
- **Kirchhoff**: O(MAX_NODES * MAX_COMPONENTS) (~40 KB for 20 nodes, 50 components)
- **DP Algorithm**: O(n * 2ⁿ) - significant memory for n>18

### Recommended Limits
- **TSP Cities**: <20 for Dynamic Programming, unlimited for other algorithms
- **Circuit Nodes**: <100 for smooth performance
- **Components**: <200 for real-time analysis

---

## 10. Technical Stack

- **Language**: C (C99 standard)
- **GUI Framework**: GTK+ 3.0
- **Graphics**: Cairo 2D graphics library
- **Build System**: Make
- **Linear Algebra**: Custom Gaussian elimination implementation
- **Optimization**: Compiled with `-O2` flag for performance

---

## Contribution

Contributions are welcomed! If you'd like to contribute, please follow these steps:

1.  Fork the repository.
2.  Create a new branch for your feature or bugfix (`git checkout -b feature/your-feature-name`).
3.  Commit your changes with clear and descriptive messages.
4.  Push your changes to your fork (`git push origin feature/your-feature-name`).
5.  Submit a pull request to the main repository with a detailed description of your changes.

**Contribution Guidelines:**
- Follow the existing code style and naming conventions
- Add comments for complex algorithms
- Test your changes on both Linux and Windows (if possible)
- Update documentation if adding new features

---

## License

This project is for educational purposes and is not licensed for commercial use. Developed as part of the Fundamentals of Data Structures course project.

---

## Acknowledgments

- **Original Concept**: Python implementation with Tkinter
- **C Implementation**: Complete rewrite using GTK+ and Cairo for native performance
- **Algorithms**: Based on classical graph theory and circuit analysis techniques
- **Course**: Fundamentals of Data Structures

---

## Future Enhancements

Planned improvements:
- Save/load circuit and TSP configurations to files
- Animation of algorithm execution (step-by-step visualization)
- Additional TSP algorithms (Christofides, Branch & Bound, Ant Colony)
- AC circuit analysis with complex impedances and phasors
- Multi-threaded genetic algorithm for faster convergence
- Export results to CSV/PDF format
- Interactive tutorials and help system

---

## Contact & Support

For bug reports, feature requests, or questions:
- **Issues**: Open an issue on GitHub
- **Email**: [atharvdrahate@gmail.com]
---
