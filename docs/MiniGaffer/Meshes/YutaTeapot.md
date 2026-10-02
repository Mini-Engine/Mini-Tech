---
layout: default
title: Yuta Teapot
parent: Meshes
grand_parent: Mini Tech
nav_order: 70
---

# Yuta Teapot

{: .note-title }
> Example Scene
>
> Help > Examples > MiniGaffer > Yuta Teapot Example
>

Creates a Utah teapot mesh, tessellated from Martin Newell's original 32 bicubic Bezier patches. Handy as a test object for shading, deformers and lookdev.

The teapot sits on the ground with **Y up** and its **spout pointing down +X**. The patch seams are welded, so it is a single connected mesh with smooth vertex normals (`N`) and facevarying `uv`s taken from each patch.

| Plug | Description |
| --- | --- |
| Size | Uniform scale. At `1` the teapot is 3.15 units tall. |
| Divisions | Quads per patch along each direction. The mesh has roughly 32 x divisions x divisions faces. |
| Lid | Includes the lid. Turn off to see inside the pot. |
| Bottom | Includes the bottom. The original 1975 teapot had no bottom. |

* Find it in the node menu under **Mini > Scene > Primitive > YutaTeapot**.
* Faces that collapse at the lid knob and the centre of the bottom are output as triangles.
