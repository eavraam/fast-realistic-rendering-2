<div align="center">

  <h3 align="center">Occlusion Queries in Rendering</h3>

  <div align="center">
    Analyzing the effects of occlusion queries in rendering.
  </div>
  
  <br />

  <div>
    <img src="https://img.shields.io/badge/OpenGL-black?style=flat" alt="opengl" />
    <img src="https://img.shields.io/badge/C%2B%2B-blue?style=flat" alt="cpp" />
    <img src="https://img.shields.io/badge/Ubuntu-orange?style=flat" alt="ubuntu" />
  </div>
</div>


## 📋 <a name="table">Table of Contents</a>
1. 🤖 [Introduction](#introduction)
2. ⚙️ [How to run](#how-to-run)
3. 📸 [Implemented Techniques](#implemented-techniques)

## <a name="introduction">🤖 Introduction</a>
In this project, I will be exploring the significance of visibility techniques in OpenGL, which are
vital for achieving efficient rendering in real-time computer graphics applications. The scene is
limited by the amount of geometry processed in the GPU. The utilization of default rendering
(no optimization), frustum culling, occlusion queries, and the evaluation of shading models such as
Gouraud and Phong shed some light on their impact on rendering performance.

The compared techniques are:
- No optimizations. (**DONE**)
- View-frustum culling. (**DONE**)
- View-frustum culling + occlusion queries. (**DONE**)
- Optimization (**NOT DONE**).

**<ins>Disclaimer</ins>:** In this README.md file you can find the overview of the project. For more details on the theory and results, you can look into the attached "*FRR_Lab_Assignment_2.pdf*" file. It is not a full-fledged report (as should be), but provides useful information on the assignment, the results of the compared techniques, some conclusions, as well as issues encountered during development.

## <a name="how-to-run">⚙️ How to run</a>

**Prerequisites**

Make sure you have the following installed on your machine:
- [Ubuntu Linux](https://ubuntu.com/) (Operating System)

**Running the project**

- Download the repository and move into the `./BaseCode/` directory.
- Then, open a terminal there and run the following commands:

```
- mkdir build
- cd build
- cmake ..
- make
```

To run the application, inside the `/build/` directory, run: `./BaseCode`

**<ins>NOTE</ins>:** In case the project loads more meshes than the computer can handle, change the
following lines of the Scene.h file:

```
- int modelCopies = [some-acceptable-number];
- The 4 float arrays right after "modelCopies" to contain [3 * some-acceptable-number] elements.
```

## <a name="implemented-techniques">📸 Implemented Techniques</a>

**ImGui**

In order to manually handle the variables of the application and monitor the performance, I have implemented an `ImGui` UI interface.


**Shading Models**

The two methods used for shading the 3D models are `Phong` and `Gouraud`. Gouraud shading computes the color intensity of a polygon at a vertex level (vertex shader) and interpolates those values across the polygon's surface, while Phong computes the shading at a pixel level (fragment shader) on the surface of the polygon.


**Frustum Culling**

Camera frustum represents the zone of vision of a camera. `Frustum culling` is a visibility optimization technique, which sorts visible and invisible elements, and renders only the visible ones. To apply frustum culling, the world-space camera frustum planes are computed. With these, we can check if an object is inside or outside the frustum by testing the frustum planes against the scene's models' Axis-Aligned Bounding Box's (AABB) corners. We typically use a bounding volume to test for frustum culling, since it is a time-efficient rough approximation of our model's mesh.


**Occlusion Culling**

`Occlusion Culling` is a feature that disables rendering of objects, when they are not currently seen
by the camera, because they are obscured (occluded) by other objects. In this project, I apply a simple occlusion query method , following these steps:

1. Initiate an occlusion query.
2. Turn off writing to the frame and depth buffer.
3. Render a simple but conservative approximation of the complex object (AABB).
4. Terminate the occlusion query.
5. Ask for the result of the query (that is, the number of visible pixels of the approximate geometry).
6. If the number of pixels drawn is greater than some threshold (typically zero), render the complex
object.

This method works well if the tested object is really complex, but step 5 involves waiting until
the result of the query actually becomes available, resulting in potentially large delays.

More information about occlusion queries can be found in [GPU Gems 2, Chapter 6](https://developer.nvidia.com/gpugems/gpugems2/part-i-geometric-complexity/chapter-6-hardware-occlusion-queries-made-useful).
