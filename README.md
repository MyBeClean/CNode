# CNode Engine

**A Node-Based Game Engine Built with C++**

CNode is an independent game engine project built with C++, focused on visual node-based programming, modular architecture, and customizable development workflows.

The engine is designed to provide a visual approach to game development while preserving the flexibility and control of native C++. By combining interconnected node graphs with an extensible engine architecture, CNode aims to make game logic easier to construct, organize, and expand.

CNode is being developed as a long-term project with the goal of creating a flexible game development environment where visual programming, rendering systems, and specialized editing tools can work together within a unified application.

---

## Overview

CNode explores an alternative approach to game development by placing visual programming at the center of its design.

Rather than relying entirely on traditional code-based scripting, developers will be able to construct game logic through nodes that represent inputs, operations, transformations, and behaviors. These nodes can be connected to define how information flows through a system and how different operations interact.

The engine is also intended to support native C++ development, allowing custom functionality to be implemented when visual nodes alone are insufficient. This combination is designed to provide both an accessible visual workflow and the flexibility required for more advanced game systems.

## Core Features

### Node-Based Visual Programming

CNode is built around a node graph system intended to represent game logic through visual connections.

Each node serves a specific function and communicates with other nodes through defined inputs and outputs. This approach is intended to make relationships between operations more visible, helping developers understand, modify, and organize their game systems.

The node system is planned to support reusable node types, flexible connections, and more complex workflows as development progresses.

### Modular Architecture

CNode aims to maintain a modular architecture in which engine systems and editor components have clearly defined responsibilities.

This design allows individual components to be developed, maintained, and expanded without unnecessarily coupling the entire application. It also provides a foundation for introducing additional functionality as the engine grows.

### Native C++ Extensibility

Although visual programming is central to CNode, native C++ remains an important part of its design.

Developers are intended to be able to implement custom node types and specialized functionality using C++. This allows the engine to move beyond its built-in capabilities and accommodate more advanced or project-specific requirements.

### Reusable Node Groups

Node groups are intended to provide a way to organize related operations into reusable units.

By grouping connected nodes, developers can simplify complex graphs, reduce visual clutter, and create more manageable workflows. This also provides a foundation for reusing logic across different systems and projects.

### Integrated Development Environment

CNode is intended to bring multiple development tools into a unified editor.

In addition to node-based programming, its long-term direction includes specialized workspaces for tasks such as scene editing, UV editing, and graphics-oriented workflows. Each workspace is intended to provide tools suited to its purpose while remaining part of the same development environment.

### Extensible Editor Design

The editor is designed with future expansion in mind, allowing additional panels, windows, and specialized tools to become part of the application.

This approach aims to keep the development environment adaptable without requiring the entire editor to be redesigned whenever new functionality is introduced.

---

## Technology Stack

CNode uses native application technologies to establish its core development environment.

- **C++** — Core engine and application development.
- **CMake** — Build configuration and project compilation.
- **GLFW** — Window creation, input handling, and event management.
- **Dear ImGui** — Editor interface and interactive development tools.
- **OpenGL** — Graphics rendering and visual output.

Additional technologies may be integrated as the engine's requirements evolve.

## Design Philosophy

CNode is guided by the idea that visual programming and traditional programming should complement each other rather than compete.

Visual node graphs provide a way to represent logic and relationships clearly, while native C++ offers the control needed to implement specialized systems and extend the engine beyond its default functionality.

Modularity is equally important to the project's direction. By separating systems according to their responsibilities, CNode aims to remain maintainable as new features and development tools are introduced.

The overall objective is to build an engine that prioritizes flexibility, extensibility, and a cohesive development workflow.

## Long-Term Vision

The long-term vision for CNode is to evolve into a comprehensive, node-driven game development environment.

Its intended direction includes a more capable visual programming system, custom native extensions, reusable logic, integrated editing workspaces, and expanded graphics tools. These capabilities are intended to operate within a consistent architecture rather than as disconnected features.

CNode is an independent project that will continue to evolve according to its technical needs and development goals.

## Development Status

CNode is currently a work in progress. Its architecture, editor functionality, and engine systems are being developed incrementally.

The features and concepts described in this document represent a combination of the project's current direction and its long-term goals. Not every described capability is necessarily implemented or ready for use.

As development continues, the engine's design and functionality will be refined to support its broader vision.
