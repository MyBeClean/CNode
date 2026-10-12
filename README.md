# CNode Engine

**A Node-Based Game Engine Built with C++**

CNode is an independent game engine project focused on node-based visual programming, modular architecture, and customizable development workflows. Its goal is to provide a flexible environment where game logic can be designed through interconnected nodes rather than relying entirely on traditional code-based scripting.

Built with C++, CNode aims to give developers greater control over how game systems are structured, connected, and extended.

---

## Overview

CNode is designed around the idea that game development should be both visual and programmable.

Instead of requiring developers to write every behavior manually, CNode aims to let them construct game logic by connecting nodes that represent inputs, operations, transformations, and behaviors. Each node serves a specific purpose and communicates with other nodes through defined connections.

This approach is intended to make game logic easier to understand, modify, reuse, and expand while preserving the flexibility needed for more advanced development.

CNode is envisioned as a modular environment where visual workflows and native C++ functionality can work together.

## Core Philosophy

### 1. Node-Based Development

Visual node graphs are intended to be a central part of CNode.

Developers will be able to construct logic by connecting nodes through their inputs and outputs. Each connection represents a relationship between operations, allowing complex behaviors to be organized into readable visual workflows.

The objective is to make game logic more intuitive to build and easier to maintain without eliminating the option of working directly with code.

### 2. Modularity

CNode is intended to use a modular architecture in which different systems can be developed and maintained independently.

The editor, node system, rendering systems, and other engine components should have clearly defined responsibilities. This separation is intended to simplify maintenance, support future expansion, and reduce unnecessary dependencies between components.

### 3. Extensibility

An important design goal is to allow developers to extend the engine beyond its built-in functionality.

CNode is intended to support custom node types and native C++ extensions, allowing developers to introduce new operations, behaviors, and specialized workflows when the default functionality is insufficient.

### 4. Developer Control

CNode aims to balance visual convenience with low-level control.

Visual programming should help developers organize logic, while C++ should remain available for implementing custom systems and functionality that require greater control over execution and behavior.

### 5. Long-Term Flexibility

CNode is being designed with future expansion in mind. Its architecture is intended to accommodate additional editing tools and development workflows without requiring the entire engine to be redesigned whenever a new system is introduced.

---

## Planned Design Concepts

The following concepts describe the intended direction of CNode.

### Visual Node Graph

The node graph is intended to serve as a visual workspace for constructing game logic.

Its design is centered around:

- Nodes with defined inputs and outputs.
- Connections between compatible node ports.
- Reusable node types for common operations.
- Multiple connections where a node's design permits them.
- Organized graphs for building complex behaviors.
- Custom node types implemented through C++.

A possible workflow could connect keyboard input to a transformation operation, then to a movement system, and finally to an object in the game world.

This structure is intended to help developers understand how data and behavior flow through a game system.

### Custom C++ Nodes

CNode aims to make native C++ a part of its extensibility model.

Developers should be able to implement specialized logic through custom node types, enabling functionality that cannot be adequately represented by built-in nodes alone.

The long-term objective is to combine the accessibility of visual programming with the flexibility of native code.

### Node Groups

Node groups are intended to help developers organize related operations into reusable units.

Instead of keeping every operation visible in a large graph, developers could group related logic into a more manageable structure. This would help keep complex systems readable and support reuse across different parts of a project.

### Integrated Editing Workspaces

CNode is envisioned as an editor with multiple workspaces for different development tasks.

Potential workspaces include a node editor, a UV editor, and graphics-oriented editing tools. These workspaces are intended to provide specialized interfaces while remaining part of a cohesive development environment.

### Modular Editor Architecture

The editor is intended to separate its windows, tabs, panels, and specialized tools into manageable components.

This approach is intended to make the interface easier to extend and maintain as additional editing capabilities are introduced.

---

## Technology

CNode is being developed with a focus on native application development.

| Technology | Intended Role |
|---|---|
| C++ | Core programming language |
| CMake | Build configuration and project organization |
| GLFW | Window creation and input/event handling |

Additional technologies may be introduced as the engine's architecture and requirements evolve.

## Design Goals

CNode's long-term design goals include:

- A flexible visual programming environment.
- A modular and maintainable C++ codebase.
- Support for custom node types and native extensions.
- Clear separation between editor functionality and engine systems.
- Reusable logic through node groups and structured graphs.
- Specialized workspaces for different development tasks.
- An architecture that can evolve alongside the project's needs.

These goals describe the intended direction of the project rather than a guarantee that every feature is already available.

## Project Structure

CNode is intended to keep source code, resources, and generated build files organized separately.

```text
CNode/
├── src/       # Source code
├── assets/    # Project resources
├── build/     # Local build output
├── CMakeLists.txt
├── .gitignore
└── README.md
