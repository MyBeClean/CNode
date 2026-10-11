# CNode

CNode is an experimental, lightweight C++20 game-engine foundation centered on a node-based programming workflow. Its long-term direction is a modular engine with a visual graph editor, scene tools, 3D rendering, and optional editor modules.

## Current status

This repository is an early foundation, not yet a complete game engine.

Implemented foundations:
- CMake C++20 build configuration.
- Basic logging.
- Node base class, typed ports, graph connections, and cycle detection.
- Float constant and addition node declarations.
- Basic scene entities and transforms.
- Initial engine lifecycle coordinator.
- Lightweight physics integration without collision detection.
- Renderer lifecycle placeholder without a graphics API.
- Native callback registry, not a scripting-language VM.
- Headless editor and node-canvas model, not a visual GUI.
- In-process module lifecycle management; dynamic DLL loading is not implemented yet.

Known limitations:
- Connected values are not yet pulled into input nodes during graph evaluation. `FloatAdd` currently evaluates to a placeholder result.
- No graphical node editor or viewport.
- No rendering backend, asset pipeline, collision solver, or embedded scripting language.
- Graph serialization and reusable node groups are not implemented.

## Requirements

- CMake 3.20 or newer.
- A C++20 compiler. On Windows, Visual Studio 2022 with the Desktop development with C++ workload is recommended.

## Build on Windows

Open a Developer PowerShell or a terminal where CMake and the Visual Studio C++ compiler are available, from the repository root:

```powershell
cmake -S . -B build
cmake --build build --config Debug
.\build\Debug\CNode.exe
```

If using a single-configuration generator such as Ninja, the executable may instead be at `build\CNode.exe`.

## Project layout

- `src/core`: engine lifecycle and logging.
- `src/nodes`: node types, ports, and graph structure.
- `src/scene`: entities and transforms.
- `src/physics`: initial movement integration.
- `src/rendering`: renderer abstraction and lifecycle.
- `src/scripting`: native callback registry for future scripting integration.
- `src/modules`: in-process module lifecycle.
- `src/editor`: editor lifecycle and headless node-canvas model.

## Development priorities

1. Make node evaluation read connected values and implement `FloatAdd`.
2. Add unit tests for graph validation, evaluation, and scene operations.
3. Add graph serialization.
4. Select a lightweight GUI framework and implement the visual node editor.
5. Select and integrate a rendering backend.
6. Add optional editor modules only when the core interfaces are stable.

CNode should grow through small, testable milestones. Features should be described as implemented only after they are actually working and verified.

## Visual Node Editor (Windows prototype)

On Windows, CMake also builds `CNodeEditor.exe`, a lightweight native Win32/GDI editor prototype with no additional GUI dependencies.

Build with Visual Studio's CMake tools:

```powershell
cmake -S . -B build
cmake --build build --config Debug
.\build\Debug\CNodeEditor.exe
```

Editor controls:
- **+ Float** adds a Float Constant node.
- **+ Add** adds an Add node.
- Click a node to select it and drag its body to move it.
- Drag a blue output port to a compatible input port to create a graph connection.
- Select a node and press **Delete**, or click **Remove Selected**, to remove it and its connections.

This is an early visual-editor prototype. Graph connections are stored in the core graph, but connected-value propagation and actual 3D rendering are not implemented yet. Nodes and layout are not saved to disk yet.
