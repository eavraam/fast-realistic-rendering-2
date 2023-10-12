# Fast Realistic Rendering (FRR) Lab Project - 2

## Table of contents
* [General info](#general-info)
* [How to run](#how-to-run)
* [Implemented techniques](#implemented-techniques)

## General info
The goal of this project is to analyze the effects of occlusion queries in rendering. The compared methods, are:
- No optimizations.
- View-frustum culling.
- View-frustum culling + occlusion queries.
- Optimization (not implemented).

## How to run
(Linux only, due to Qt dependencies)
Use QtCreator 5.

```
- Go to BaseCode folder, and open a terminal there.
- run: mkdir build
- run: cd build
- run: cmake ..
- run: make
- run the application: ./BaseCode
```

## Implemented techniques
- ImGui for manual variable handling.
- Phong and Gouraud shading.
- Stop-and-Wait occlusion culling
