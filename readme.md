# LOD-Mesh-Simplifier

This is a museum-like scene defined by a map file, processes 3D models for Level of Detail (LOD) using simplification techniques, pre-calculates visibility, and renders the scene. It features dynamic LOD switching and a simple GUI for interaction.

## Prerequisites

- **Linux:**
    - `cmake`
    - A C++ compiler (e.g., g++)
    - `make`
    - `OpenGL`
    - `glfw`
    - `glm`
- **Windows:**
    - Visual Studio (with C++ development)

## Building the Project

### Linux

1.  Create a build directory and navigate into it:
    ```bash
    mkdir build
    cd build
    ```
2.  Run CMake to configure the project and then build it:
    ```bash
    cmake -DCMAKE_BUILD_TYPE=Release .. && make -j
    ```
    (The executable `lodmesh` will be in the `build` directory).

### Windows

1.  Open the solution file `windows/LOD-Mesh-Simplifier/LOD-Mesh-Simplifier.sln` with Visual Studio.
2.  Build the solution.

## Running the Project

The application requires a `map.txt` file to define the scene.

- **Linux (from the `build` directory):**
    ```bash
    ./lodmesh ../map.txt
    ```
    (Assuming `map.txt` is in the project's root directory, one level above `build`.)

- **Windows:**
    Provide the path to `map.txt` as a command-line argument, or configure the solution to open the executable with the argument with the path of the `map.txt`

### `map.txt` File Format

The `map.txt` file has a specific structure:

1.  **Map Dimensions:**
    `width height`
    (e.g., `10 10`)

2.  **Map Layout:**
    `width * height` characters (0 or 1), representing the museum structure. `0` indicates floor, `1` indicates a wall. These are listed row by row.
    Example for a 3x3 map:
    ```
    111
    101
    101
    ```

3.  **Number of Models:**
    `num_models` (integer indicating how many unique 3D models to load)

4.  **Model Definitions:**
    `num_models` lines, each with the following format:
    `path/to/model.ply lod_res1 lod_res2 ... lod_resN`
    - `path/to/model.ply`: Path to the model file in .ply format.
    - `lod_resX`: Integer resolutions for Levels of Detail (LODs). These should ideally be in decreasing order.
    Example: `../models/bunny.ply 128 64 32`

5.  **Number of Instances:**
    `num_instances` (integer indicating how many model instances will be placed in the scene)

6.  **Instance Definitions:**
    `num_instances` lines, each with the following format:
    `model_id pos_x pos_y pos_z scale_x scale_y scale_z`
    - `model_id`: 0-indexed ID, based on the order models were defined above.
    - `pos_x pos_y pos_z`: Floating-point numbers for the instance's position.
    - `scale_x scale_y scale_z`: Floating-point numbers for the instance's scale.
    Example: `0 1.5 0.0 2.0 1.0 1.0 1.0`

You can see `map.txt` as an example


## Core Functionality Details

### LOD Generation

When reading `map.txt`, simplified models are generated using the resolutions specified for each model. The simplification technique is Vertex Clustering, implemented with four variations:
1.  Average
2.  Average with Normals
3.  QEM (Quadric Error Metrics)
4.  QEM with Normals

These generated LOD models are saved to disk in the `models/LODs/` directory (relative to the model's path) to avoid re-computation on subsequent runs. For example, if a model is `../models/bunny.ply`, its LODs might be saved in `../models/LODs/bunny_128_0.ply`, etc.

### Visibility Pre-calculation

A visibility file named `visibility.txt` is calculated using Random Visibility Sampling. This file is used during rendering to determine which model instances are potentially visible from the current camera position.
- **Recalculation:** `visibility.txt` is automatically recalculated if the map dimensions (width/height specified in `map.txt`) change.
- **Manual Deletion:** If you modify the map layout *without* changing its dimensions, you **must manually delete** the `visibility.txt` file to force its recalculation with the new layout.

Once all data is loaded and processed, the 3D scene will be displayed.

## Controls

- **`F1`**: Toggle mouse camera control. (When active, the mouse controls camera orientation).
- **`WASD` / `Arrow Keys`**: Move the camera (forward, left, backward, right relative to camera view).
- **`Q`**: Move camera up (fly).
- **`E`**: Move camera down (fly).
- **`F2`**: Toggle LOD coloring. This visually highlights which LOD is being rendered for each model.
- **`F5`**: Toggle Full Screen mode.
- **`1`, `2`, `3`, `4`**: Switch the active simplification method used for displaying models:
    - **`1`**: Average
    - **`2`**: Average with Normals
    - **`3`**: QEM
    - **`4`**: QEM with Normals

## GUI

A small on-screen User Interface (UI) is provided, which shows:
- **FPS Counter**: Displays the current frames per second.
- **Max Cost**: Allows adjustment of the 'max cost' parameter for the Time-critical rendering algorithm, influencing the trade-off between visual quality (higher detail LODs) and performance.
- **Simplification Method**: Allows changing the active simplification method (same functionality as the `1`, `2`, `3`, `4` keys).
- **Total Cost**: The total cost of the render (in triangles) is displayed.